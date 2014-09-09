/*
 * Copyright (C) 2014 Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <blpwtk2_processhostimpl.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_channelinfo.h>
#include <blpwtk2_constants.h>
#include <blpwtk2_control_messages.h>
#include <blpwtk2_managedrenderprocesshost.h>
#include <blpwtk2_processhostmanager.h>
#include <blpwtk2_products.h>
#include <blpwtk2_profile_messages.h>
#include <blpwtk2_profilehost.h>
#include <blpwtk2_rendererinfomap.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_webview_messages.h>
#include <blpwtk2_webviewhost.h>

#include <base/command_line.h>
#include <base/process/process_iterator.h>  // for kProcessAccess* constants
#include <base/process/process_handle.h>
#include <content/public/browser/browser_thread.h>
#include <content/public/browser/render_process_host.h>
#include <ipc/ipc_channel_proxy.h>

namespace blpwtk2 {

ProcessHostImpl::ProcessHostImpl(RendererInfoMap* rendererInfoMap)
: d_processHandle(base::kNullProcessHandle)
, d_rendererInfoMap(rendererInfoMap)
, d_lastRoutingId(0x10000)
{
    DCHECK(d_rendererInfoMap);

    scoped_refptr<base::SingleThreadTaskRunner> ioTaskRunner =
        content::BrowserThread::GetMessageLoopProxyForThread(
            content::BrowserThread::IO);

    std::string channelId = IPC::Channel::GenerateVerifiedChannelID(BLPWTK2_VERSION);
    d_channel.reset(new IPC::ChannelProxy(channelId,
                                          IPC::Channel::MODE_SERVER,
                                          this,
                                          ioTaskRunner));
}

ProcessHostImpl::~ProcessHostImpl()
{
    // Load the listeners into a vector first, because their destructors will
    // remove themselves from the d_routes map.
    std::vector<ProcessHostListener*> listeners;
    listeners.reserve(d_routes.size());
    for (IDMap<ProcessHostListener>::Iterator<ProcessHostListener> it(&d_routes); !it.IsAtEnd(); it.Advance()) {
        listeners.push_back(it.GetCurrentValue());
    }
    for (size_t i = 0; i < listeners.size(); ++i) {
        delete listeners[i];
    }
    DCHECK(0 == d_routes.size());

    d_channel.reset();

    // This needs to use DeleteSoon because WebViewImpl::destroy uses
    // DeleteSoon, and we need to ensure that the render process host outlives
    // the WebViewImpl.
    // TODO(SHEZ): See if we can just delete the WebViewImpl directly
    //             instead of using DeleteSoon.
    base::MessageLoop::current()->DeleteSoon(
        FROM_HERE,
        d_renderProcessHost.release());

    if (d_processHandle != base::kNullProcessHandle) {
        base::CloseProcessHandle(d_processHandle);
    }
}

const std::string& ProcessHostImpl::channelId() const
{
    return d_channel->ChannelId();
}

std::string ProcessHostImpl::channelInfo() const
{
    CommandLine commandLine(CommandLine::NO_PROGRAM);

    // TODO(SHEZ): We are missing kDisableDatabases for incognito profiles
    //             because we don't know yet which profile will be used for the
    //             in-process renderer.  We need to either have the profile
    //             specified upfront, or we need to handle kDisableDatabases
    //             once the profile is known.
    content::RenderProcessHost::AdjustCommandLineForRenderer(&commandLine);

    ChannelInfo channelInfo;
    channelInfo.d_channelId = channelId();
    channelInfo.loadSwitchesFromCommandLine(commandLine);
    return channelInfo.serialize();
}

// ProcessHost overrides

void ProcessHostImpl::addRoute(int routingId, ProcessHostListener* listener)
{
    DLOG(INFO) << "Added route: routingId(" << routingId << ")";
    d_routes.AddWithID(listener, routingId);
}

void ProcessHostImpl::removeRoute(int routingId)
{
    d_routes.Remove(routingId);
    DLOG(INFO) << "Removed route: routingId(" << routingId << ")";
}

ProcessHostListener* ProcessHostImpl::findListener(int routingId)
{
    return d_routes.Lookup(routingId);
}

int ProcessHostImpl::getUniqueRoutingId()
{
    return ++d_lastRoutingId;
}

base::ProcessHandle ProcessHostImpl::processHandle()
{
    return d_processHandle;
}

// IPC::Sender overrides

bool ProcessHostImpl::Send(IPC::Message* message)
{
    return d_channel->Send(message);
}

// IPC::Listener overrides

bool ProcessHostImpl::OnMessageReceived(const IPC::Message& message)
{
    if (message.routing_id() == MSG_ROUTING_CONTROL) {
        // Dispatch control messages
        bool msgIsOk = true;
        IPC_BEGIN_MESSAGE_MAP_EX(ProcessHostImpl, message, msgIsOk)
            IPC_MESSAGE_HANDLER(BlpControlHostMsg_Sync, onSync)
            IPC_MESSAGE_HANDLER(BlpControlHostMsg_CreateNewHostChannel, onCreateNewHostChannel)
            IPC_MESSAGE_HANDLER(BlpControlHostMsg_ClearWebCache, onClearWebCache)
            IPC_MESSAGE_HANDLER(BlpProfileHostMsg_New, onProfileNew)
            IPC_MESSAGE_HANDLER(BlpProfileHostMsg_Destroy, onProfileDestroy)
            IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_New, onWebViewNew)
            IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_Destroy, onWebViewDestroy)
            IPC_MESSAGE_UNHANDLED_ERROR()
        IPC_END_MESSAGE_MAP_EX()

        if (!msgIsOk) {
            LOG(ERROR) << "bad message " << message.type();
        }
        return true;
    }

    // Dispatch incoming messages to the appropriate IPC::Listener.
    IPC::Listener* listener = d_routes.Lookup(message.routing_id());
    if (!listener) {
        if (message.is_sync()) {
            // The listener has gone away, so we must respond or else the
            // caller will hang waiting for a reply.
            IPC::Message* reply = IPC::SyncMessage::GenerateReply(&message);
            reply->set_reply_error();
            Send(reply);
        }

        DLOG(INFO) << "message received, but no listener: routingId("
                   << message.routing_id() << ") type(" << message.type()
                   << ")";
        return true;
    }

    return listener->OnMessageReceived(message);
}

void ProcessHostImpl::OnChannelConnected(int32 peer_pid)
{
    DLOG(INFO) << "channel connected: peer_pid(" << peer_pid << ")";
    if (peer_pid == base::GetCurrentProcId()) {
        d_processHandle = base::GetCurrentProcessHandle();
    }
    else {
        bool success =
            base::OpenProcessHandleWithAccess(
                peer_pid,
                base::kProcessAccessDuplicateHandle |
                base::kProcessAccessQueryInformation |
                base::kProcessAccessWaitForTermination,
                &d_processHandle);
        CHECK(success);
    }
    CHECK(d_processHandle != base::kNullProcessHandle);
}

void ProcessHostImpl::OnChannelError()
{
    LOG(ERROR) << "channel error!";
}

// Control message handlers

void ProcessHostImpl::onSync()
{
    DLOG(INFO) << "sync";
}

void ProcessHostImpl::onCreateNewHostChannel(int timeoutInMilliseconds,
                                             std::string* channelInfo)
{
    DCHECK(Statics::processHostManager);

    ProcessHostImpl* newProcessHost = new ProcessHostImpl(d_rendererInfoMap);
    *channelInfo = newProcessHost->channelInfo();
    Statics::processHostManager->addProcessHost(
        newProcessHost,
        base::TimeDelta::FromMilliseconds(timeoutInMilliseconds));
}

void ProcessHostImpl::onClearWebCache()
{
    content::RenderProcessHost::ClearWebCacheOnAllRenderers();
}

void ProcessHostImpl::onProfileNew(int routingId,
                                   const std::string& dataDir,
                                   bool diskCacheEnabled,
                                   bool cookiePersistenceEnabled)
{
    new ProfileHost(this,
                    routingId,
                    dataDir,
                    diskCacheEnabled,
                    cookiePersistenceEnabled);
}

void ProcessHostImpl::onProfileDestroy(int routingId)
{
    ProfileHost* profileHost =
        static_cast<ProfileHost*>(findListener(routingId));
    DCHECK(profileHost);
    delete profileHost;
}

void ProcessHostImpl::onWebViewNew(const BlpWebViewHostMsg_NewParams& params)
{
    ProfileHost* profileHost =
        static_cast<ProfileHost*>(findListener(params.profileId));
    DCHECK(profileHost);

    int hostAffinity;
    bool isInProcess =
        params.rendererAffinity == blpwtk2::Constants::IN_PROCESS_RENDERER;

    if (isInProcess) {
        if (!d_renderProcessHost.get()) {
            DCHECK(-1 == d_inProcessRendererInfo.d_hostId);
            CHECK(d_processHandle != base::kNullProcessHandle);
            d_renderProcessHost.reset(
                new ManagedRenderProcessHost(
                    d_processHandle,
                    profileHost->browserContext()));
            d_inProcessRendererInfo.d_hostId = d_renderProcessHost->id();
            Send(new BlpControlMsg_SetInProcessRendererChannelName(
                d_renderProcessHost->channelId()));
        }

        DCHECK(-1 != d_inProcessRendererInfo.d_hostId);
        hostAffinity = d_inProcessRendererInfo.d_hostId;
    }
    else {
        hostAffinity =
            d_rendererInfoMap->obtainHostAffinity(params.rendererAffinity);
    }

    new WebViewHost(this,
                    profileHost->browserContext(),
                    params.routingId,
                    isInProcess,
                    (NativeView)params.parent,
                    hostAffinity,
                    params.initiallyVisible,
                    params.properties);
}

void ProcessHostImpl::onWebViewDestroy(int routingId)
{
    WebViewHost* webViewHost =
        static_cast<WebViewHost*>(findListener(routingId));
    DCHECK(webViewHost);
    delete webViewHost;
}

}  // close namespace blpwtk2
