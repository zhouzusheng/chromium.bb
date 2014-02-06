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

#include <blpwtk2_profile_messages.h>
#include <blpwtk2_profilehost.h>

#include <content/public/browser/browser_thread.h>
#include <ipc/ipc_channel_proxy.h>

namespace blpwtk2 {

ProcessHostImpl::ProcessHostImpl(const std::string& channelId)
{
    scoped_refptr<base::SingleThreadTaskRunner> ioTaskRunner =
        content::BrowserThread::GetMessageLoopProxyForThread(
            content::BrowserThread::IO);

    d_channel.reset(new IPC::ChannelProxy(channelId,
                                          IPC::Channel::MODE_SERVER,
                                          this,
                                          ioTaskRunner));
}

ProcessHostImpl::~ProcessHostImpl()
{
    d_channel.reset();
}

// ProcessHost overrides

void ProcessHostImpl::addRoute(int routingId, IPC::Listener* listener)
{
    DLOG(INFO) << "Added route: routingId(" << routingId << ")";
    d_routes.AddWithID(listener, routingId);
}

void ProcessHostImpl::removeRoute(int routingId)
{
    d_routes.Remove(routingId);
    DLOG(INFO) << "Removed route: routingId(" << routingId << ")";
}

IPC::Listener* ProcessHostImpl::findListener(int routingId)
{
    return d_routes.Lookup(routingId);
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
            IPC_MESSAGE_HANDLER(BlpProfileHostMsg_New, onProfileNew)
            IPC_MESSAGE_HANDLER(BlpProfileHostMsg_Destroy, onProfileDestroy)
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
}

void ProcessHostImpl::OnChannelError()
{
    LOG(ERROR) << "channel error!";
}

// Control message handlers

void ProcessHostImpl::onProfileNew(int routingId,
                                   const std::string& dataDir,
                                   bool diskCacheEnabled,
                                   void** browserContext)
{
    ProfileHost* profileHost = new ProfileHost(this,
                                               routingId,
                                               dataDir,
                                               diskCacheEnabled);
    *browserContext = profileHost->browserContext();
}

void ProcessHostImpl::onProfileDestroy(int routingId)
{
    ProfileHost* profileHost =
        static_cast<ProfileHost*>(findListener(routingId));
    DCHECK(profileHost);
    delete profileHost;
}

}  // close namespace blpwtk2
