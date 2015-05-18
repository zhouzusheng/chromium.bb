/*
 * Copyright (C) 2013 Bloomberg Finance L.P.
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

#include <blpwtk2_toolkitimpl.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_browsercontextimplmanager.h>
#include <blpwtk2_browsermainrunner.h>
#include <blpwtk2_browserthread.h>
#include <blpwtk2_channelinfo.h>
#include <blpwtk2_constants.h>
#include <blpwtk2_control_messages.h>
#include <blpwtk2_inprocessrenderer.h>
#include <blpwtk2_mainmessagepump.h>
#include <blpwtk2_managedrenderprocesshost.h>
#include <blpwtk2_processclientimpl.h>
#include <blpwtk2_processhostimpl.h>
#include <blpwtk2_processhostmanager.h>
#include <blpwtk2_products.h>
#include <blpwtk2_profilecreateparams.h>
#include <blpwtk2_profileproxy.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_webviewcreateparams.h>
#include <blpwtk2_webviewimpl.h>
#include <blpwtk2_webviewproxy.h>

#include <base/command_line.h>
#include <base/message_loop/message_loop.h>
#include <base/path_service.h>
#include <base/synchronization/waitable_event.h>
#include <base/threading/thread_restrictions.h>
#include <chrome/common/chrome_paths.h>
#include <content/public/app/content_main.h>
#include <content/public/app/content_main_runner.h>
#include <content/public/app/startup_helper_win.h>  // for InitializeSandboxInfo
#include <content/public/browser/render_process_host.h>
#include <content/public/common/content_switches.h>
#include <sandbox/win/src/win_utils.h>
#include <third_party/WebKit/public/web/WebScriptController.h>

extern HANDLE g_instDLL;  // set in DllMain

namespace blpwtk2 {

static scoped_ptr<base::MessagePump> messagePumpForUIFactory()
{
    if (Statics::isInApplicationMainThread()) {
        return scoped_ptr<base::MessagePump>(new MainMessagePump());
    }

    return scoped_ptr<base::MessagePump>(new base::MessagePumpForUI());
}

static ToolkitImpl* g_instance = 0;

ToolkitImpl* ToolkitImpl::instance()
{
    return g_instance;
}

ToolkitImpl::ToolkitImpl(const StringRef& dictionaryPath,
                         const StringRef& hostChannel)
: d_threadsStarted(false)
, d_threadsStopped(false)
, d_defaultProfile(0)
, d_mainDelegate(false)
, d_dictionaryPath(dictionaryPath.data(), dictionaryPath.length())
, d_hostChannel(hostChannel.data(), hostChannel.length())
{
    DCHECK(!g_instance);
    g_instance = this;

    // If host channel is set, we must not be in ORIGINAL thread mode.
    CHECK(d_hostChannel.empty() || !Statics::isOriginalThreadMode());

    if (!d_hostChannel.empty()) {
        // If another process is our host, then explicitly disable sandbox
        // in *this* process.
        // Since both 'broker_services' and 'target_services' will be null in
        // our SandboxInterfaceInfo, we don't want chromium to touch it.  This
        // flag prevents chromium from trying to use these services.
        d_mainDelegate.appendCommandLineSwitch(switches::kNoSandbox);
    }

    blink::WebScriptController::setStackCaptureControlledByInspector(false);
    d_mainDelegate.setRendererInfoMap(&d_rendererInfoMap);
    base::MessageLoop::InitMessagePumpForUIFactory(&messagePumpForUIFactory);
}

ToolkitImpl::~ToolkitImpl()
{
    DCHECK(!d_threadsStarted || d_threadsStopped);
    DCHECK(g_instance);
    g_instance = 0;
}

void ToolkitImpl::startupThreads()
{
    CHECK(!d_threadsStarted);

    ChannelInfo hostChannelInfo;
    if (d_hostChannel.empty()) {
        content::InitializeSandboxInfo(&d_sandboxInfo);
    }
    else {
        d_sandboxInfo.broker_services = 0;
        d_sandboxInfo.target_services = 0;
        bool check = hostChannelInfo.deserialize(d_hostChannel);
        DCHECK(check);
        for (size_t i = 0; i < hostChannelInfo.d_switches.size(); ++i) {
            const ChannelInfo::Switch& s = hostChannelInfo.d_switches[i];
            if (s.d_value.empty()) {
                appendCommandLineSwitch(s.d_key.c_str());
            }
            else {
                std::string str = s.d_key;
                str.append("=");
                str.append(s.d_value);
                appendCommandLineSwitch(str.c_str());
            }
        }
    }

    d_mainRunner.reset(content::ContentMainRunner::Create());
    content::ContentMainParams mainParams(&d_mainDelegate);
    mainParams.instance = (HINSTANCE)g_instDLL;
    mainParams.sandbox_info = &d_sandboxInfo;
    int rc = d_mainRunner->Initialize(mainParams);
    CHECK(-1 == rc);  // it returns -1 for success!!

    if (!d_dictionaryPath.empty()) {
        LOG(INFO) << "Setting dictionary path: " << d_dictionaryPath;
        base::ThreadRestrictions::ScopedAllowIO allowIO;
        PathService::Override(chrome::DIR_APP_DICTIONARIES, base::FilePath::FromUTF8Unsafe(d_dictionaryPath));
    }

    if (Statics::isRendererMainThreadMode()) {
        LOG(INFO) << "isRendererMainThreadMode";
        new base::MessageLoop(base::MessageLoop::TYPE_UI);
        if (d_hostChannel.empty()) {
            d_browserThread.reset(new BrowserThread(&d_sandboxInfo));
        }
    }
    else {
        DCHECK(Statics::isOriginalThreadMode());
        LOG(INFO) << "isOriginalThreadMode";
        d_browserMainRunner.reset(new BrowserMainRunner(&d_sandboxInfo));
    }

    if (!Statics::isInProcessRendererDisabled) {
        LOG(INFO) << "Initializing InProcessRenderer";
        InProcessRenderer::init();
    }

    LOG(INFO) << "Initializing MainMessagePump";
    MainMessagePump::current()->init();

    if (Statics::isRendererMainThreadMode()) {
        base::MessageLoop::current()->set_ipc_sync_messages_should_peek(true);

        std::string channelId;

        // If the specified hostChannel is empty, we will create an in-process
        // host on the browser-main thread, otherwise, our ProcessClient will
        // connect to the hostChannel provided by the app.
        if (d_hostChannel.empty()) {
            d_browserThread->messageLoop()->PostTask(
                FROM_HERE,
                base::Bind(&ToolkitImpl::createInProcessHost,
                           base::Unretained(this)));
            LOG(INFO) << "Waiting for ProcessHost on the browser thread";
            d_browserThread->sync();
            CHECK(d_inProcessHost.get());
            LOG(INFO) << "ProcessHost on the browser thread has been initialized";
            channelId = d_inProcessHost->channelId();
        }
        else {
            channelId = hostChannelInfo.d_channelId;
        }

        LOG(INFO) << "Creating ProcessClient for in-process renderer";
        d_processClient.reset(
            new ProcessClientImpl(channelId,
                                  InProcessRenderer::ipcTaskRunner()));
    }

    d_threadsStarted = true;

    LOG(INFO) << "Threads have started!";
}

void ToolkitImpl::shutdownThreads()
{
    CHECK(!d_threadsStopped);
    CHECK(d_threadsStarted);

    LOG(INFO) << "Shutting down threads...";

    if (d_defaultProfile) {
        d_defaultProfile->destroy();
        d_defaultProfile = 0;
    }

    if (Statics::isRendererMainThreadMode()) {
        // Make sure any messages posted to the ProcessHost have been handled.
        d_processClient->Send(new BlpControlHostMsg_Sync(true));
        d_processClient.reset();

        if (d_browserThread.get()) {
            d_browserThread->messageLoop()->PostTask(
                FROM_HERE,
                base::Bind(&ToolkitImpl::destroyInProcessHost,
                           base::Unretained(this)));
            d_browserThread->messageLoop()->PostTask(
                FROM_HERE,
                base::Bind(&BrowserMainRunner::destroyProcessHostManager,
                           base::Unretained(d_browserThread->mainRunner())));
            d_browserThread->messageLoop()->PostTask(
                FROM_HERE,
                base::Bind(
                    &BrowserContextImplManager::destroyBrowserContexts,
                    base::Unretained(Statics::browserContextImplManager)));

            // Make sure any tasks posted to the browser-main thread have been
            // handled.
            d_browserThread->sync();
        }
    }
    else {
        DCHECK(Statics::isOriginalThreadMode());
        d_browserMainRunner->destroyProcessHostManager();
        Statics::browserContextImplManager->destroyBrowserContexts();
    }

    DCHECK(0 == Statics::numProfiles);

    MainMessagePump::current()->cleanup();

    if (!Statics::isInProcessRendererDisabled)
        InProcessRenderer::cleanup();

    if (Statics::isRendererMainThreadMode()) {
        delete base::MessageLoop::current();
        d_browserThread.reset();
    }
    else {
        DCHECK(Statics::isOriginalThreadMode());
        d_inProcessRendererHost.reset();
        d_browserMainRunner.reset();
    }

    d_mainRunner->Shutdown();
    d_mainRunner.reset();

    sandbox::CallOnExitHandlers();

    d_threadsStopped = true;
}

void ToolkitImpl::appendCommandLineSwitch(const char* switchString)
{
    d_mainDelegate.appendCommandLineSwitch(switchString);
}

Profile* ToolkitImpl::createProfile(const ProfileCreateParams& params)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(Statics::isRendererMainThreadMode()
        || Statics::isOriginalThreadMode());

    if (!d_threadsStarted) {
        startupThreads();
    }

    std::string dataDir(params.dataDir().data(), params.dataDir().length());
    if (Statics::isRendererMainThreadMode()) {
        return new ProfileProxy(d_processClient.get(),
                                Statics::getUniqueRoutingId(),
                                dataDir,
                                params.diskCacheEnabled(),
                                params.cookiePersistenceEnabled());
    }
    else {
        DCHECK(Statics::isOriginalThreadMode());
        DCHECK(Statics::browserContextImplManager);
        return Statics::browserContextImplManager->obtainBrowserContextImpl(
            dataDir,
            params.diskCacheEnabled(),
            params.cookiePersistenceEnabled());
    }
}

bool ToolkitImpl::hasDevTools()
{
    DCHECK(Statics::isInApplicationMainThread());
    return Statics::hasDevTools;
}

void ToolkitImpl::destroy()
{
    DCHECK(Statics::isInApplicationMainThread());
    if (d_threadsStarted) {
        shutdownThreads();
    }
    delete this;
}

WebView* ToolkitImpl::createWebView(NativeView parent,
                                    WebViewDelegate* delegate,
                                    const WebViewCreateParams& params)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(Statics::isRendererMainThreadMode()
        || Statics::isOriginalThreadMode());

    if (!d_threadsStarted) {
        startupThreads();
    }

    Profile* profile = params.profile();
    if (!profile) {
        if (!d_defaultProfile) {
            d_defaultProfile = createProfile(ProfileCreateParams(""));
        }
        profile = d_defaultProfile;
    }

    bool singleProcess = base::CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kSingleProcess);
    // Enforce in-process renderer if "--single-process" is specified on the
    // command line.  This is useful for debugging.
    int rendererAffinity = singleProcess ? Constants::IN_PROCESS_RENDERER
                                         : params.rendererAffinity();

    DCHECK(rendererAffinity != Constants::IN_PROCESS_RENDERER || !Statics::isInProcessRendererDisabled);
    DCHECK(singleProcess ||
           rendererAffinity == Constants::ANY_OUT_OF_PROCESS_RENDERER ||
           (rendererAffinity == Constants::IN_PROCESS_RENDERER &&
            d_inProcessRendererInfo.dcheckProfile(profile)) ||
           d_rendererInfoMap.dcheckProfileForRenderer(rendererAffinity, profile));

    WebViewProperties properties;
    properties.takeKeyboardFocusOnMouseDown = params.takeKeyboardFocusOnMouseDown();
    properties.takeLogicalFocusOnMouseDown = params.takeLogicalFocusOnMouseDown();
    properties.activateWindowOnMouseDown = params.activateWindowOnMouseDown();
    properties.domPasteEnabled = params.domPasteEnabled();
    properties.javascriptCanAccessClipboard = params.javascriptCanAccessClipboard();

    if (Statics::isRendererMainThreadMode()) {
        ProfileProxy* profileProxy = static_cast<ProfileProxy*>(profile);
        return new WebViewProxy(d_processClient.get(),
                                Statics::getUniqueRoutingId(),
                                profileProxy,
                                delegate,
                                parent,
                                rendererAffinity,
                                params.initiallyVisible(),
                                properties);
    }
    else if (Statics::isOriginalThreadMode()) {
        BrowserContextImpl* browserContext =
            static_cast<BrowserContextImpl*>(profile);

        int hostAffinity;

        if (rendererAffinity == Constants::IN_PROCESS_RENDERER) {
            if (!d_inProcessRendererHost.get()) {
                DCHECK(-1 == d_inProcessRendererInfo.d_hostId);
                d_inProcessRendererHost.reset(
                    new ManagedRenderProcessHost(
                        base::GetCurrentProcessHandle(),
                        browserContext));
                d_inProcessRendererInfo.d_hostId =
                    d_inProcessRendererHost->id();
                InProcessRenderer::setChannelName(
                    d_inProcessRendererHost->channelId());
            }

            DCHECK(-1 != d_inProcessRendererInfo.d_hostId);
            hostAffinity = d_inProcessRendererInfo.d_hostId;
        }
        else {
            hostAffinity =
                d_rendererInfoMap.obtainHostAffinity(rendererAffinity);
        }

        return new WebViewImpl(delegate,
                               parent,
                               browserContext,
                               hostAffinity,
                               params.initiallyVisible(),
                               properties);
    }

    NOTREACHED();
    return 0;
}

String ToolkitImpl::createHostChannel(int timeoutInMilliseconds)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(Statics::isRendererMainThreadMode()
        || Statics::isOriginalThreadMode());

    if (!d_threadsStarted) {
        startupThreads();
    }

    std::string channelInfo;

    if (Statics::isRendererMainThreadMode()) {
        DCHECK(d_processClient.get());
        d_processClient->Send(
            new BlpControlHostMsg_CreateNewHostChannel(timeoutInMilliseconds,
                                                       &channelInfo));
    }
    else {
        DCHECK(Statics::processHostManager);

        ProcessHostImpl* processHost = new ProcessHostImpl(&d_rendererInfoMap);
        channelInfo = processHost->channelInfo();
        Statics::processHostManager->addProcessHost(
            processHost,
            base::TimeDelta::FromMilliseconds(timeoutInMilliseconds));
    }

    return String(channelInfo.data(), channelInfo.length());
}

bool ToolkitImpl::preHandleMessage(const NativeMsg* msg)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(PumpMode::MANUAL == Statics::pumpMode);
    if (d_threadsStarted) {
        return MainMessagePump::current()->preHandleMessage(*msg);
    }
    return false;
}

void ToolkitImpl::postHandleMessage(const NativeMsg* msg)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(PumpMode::MANUAL == Statics::pumpMode);
    if (d_threadsStarted) {
        MainMessagePump::current()->postHandleMessage(*msg);
    }
}

void ToolkitImpl::createInProcessHost()
{
    DCHECK(Statics::isInBrowserMainThread());
    d_inProcessHost.reset(new ProcessHostImpl(&d_rendererInfoMap));
}

void ToolkitImpl::destroyInProcessHost()
{
    DCHECK(Statics::isInBrowserMainThread());
    d_inProcessHost.reset();
}

void ToolkitImpl::clearWebCache()
{
    if (Statics::isRendererMainThreadMode()) {
        DCHECK(d_processClient.get());
        d_processClient->Send(new BlpControlHostMsg_ClearWebCache());
    }
    else {
        content::RenderProcessHost::ClearWebCacheOnAllRenderers();
    }
}

}  // close namespace blpwtk2

