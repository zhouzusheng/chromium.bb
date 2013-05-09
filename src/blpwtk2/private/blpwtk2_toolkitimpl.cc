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

#include <blpwtk2_statics.h>
#include <blpwtk2_constants.h>
#include <blpwtk2_createparams.h>
#include <blpwtk2_browserthread.h>
#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_inprocessrendererhost.h>
#include <blpwtk2_browsermainrunner.h>
#include <blpwtk2_mainmessagepump.h>
#include <blpwtk2_webviewimpl.h>
#include <blpwtk2_webviewproxy.h>

#include <base/command_line.h>
#include <base/message_loop.h>
#include <base/synchronization/waitable_event.h>
#include <content/public/app/content_main_runner.h>
#include <content/public/app/startup_helper_win.h>  // for InitializeSandboxInfo
#include <content/public/browser/render_process_host.h>
#include <content/public/browser/site_instance.h>
#include <content/public/common/content_switches.h>
#include <content/public/renderer/render_thread.h>
#include <content/browser/web_contents/web_contents_view_win.h>
#include <content/browser/renderer_host/render_process_host_impl.h>

extern HANDLE g_instDLL;  // set in DllMain

namespace blpwtk2 {

static base::MessagePump* messagePumpForUIFactory()
{
    if (Statics::isInApplicationMainThread()) {
        return new MainMessagePump();
    }

    return new base::MessagePumpForUI();
}

static ToolkitImpl* g_instance = 0;

ToolkitImpl* ToolkitImpl::instance()
{
    return g_instance;
}

ToolkitImpl::ToolkitImpl()
{
    DCHECK(!g_instance);
    g_instance = this;

    MessageLoop::InitMessagePumpForUIFactory(&messagePumpForUIFactory);

    content::InitializeSandboxInfo(&d_sandboxInfo);
    d_mainRunner.reset(content::ContentMainRunner::Create());
    int rc = d_mainRunner->Initialize((HINSTANCE)g_instDLL, &d_sandboxInfo, &d_mainDelegate);
    DCHECK(-1 == rc);  // it returns -1 for success!!

    if (Statics::isRendererMainThreadMode()) {
        Statics::rendererMessageLoop = new MessageLoop(MessageLoop::TYPE_UI);

        content::WebContentsViewWin::disableHookOnRoot();
        base::WaitableEvent initializeEvent(true, false);
        d_browserThread.reset(new BrowserThread(&initializeEvent, &d_sandboxInfo));

        // We must wait for the browser thread to finish initializing before we
        // return to the application.  Part of the initialization of the
        // BrowserThread is the creation of the InProcessRendererHost, which
        // must exist before the application can create any in-process
        // WebViews.
        initializeEvent.Wait();
    }
    else {
        DCHECK(Statics::isOriginalThreadMode());
        d_browserMainRunner.reset(new BrowserMainRunner(&d_sandboxInfo));
    }

    MainMessagePump::current()->init();
}

ToolkitImpl::~ToolkitImpl()
{
    DCHECK(g_instance);

    if (Statics::isRendererMainThreadMode()) {
        d_browserThread->sync();  // make sure any WebView::destroy has been
                                  // handled by the browser-main thread
        MainMessagePump::current()->cleanup();
        content::RenderThread::CleanUpInProcessRenderer();
        delete Statics::rendererMessageLoop;
        Statics::rendererMessageLoop = 0;
        d_browserThread.reset();
    }
    else {
        MainMessagePump::current()->cleanup();
        d_browserMainRunner.reset();
    }

    d_mainRunner->Shutdown();
    d_mainRunner.reset();

    g_instance = 0;
}

WebView* ToolkitImpl::createWebView(NativeView parent,
                                    WebViewDelegate* delegate,
                                    const CreateParams& params)
{
    DCHECK(Statics::isRendererMainThreadMode()
        || Statics::isOriginalThreadMode());

    int hostAffinity;
    // Enforce in-process renderer if "--single-process" is specified on the
    // command line.  This is useful for debugging.
    if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kSingleProcess)
        || params.rendererAffinity() == Constants::IN_PROCESS_RENDERER) {
        hostAffinity = Statics::isRendererMainThreadMode()
            ? d_browserThread->mainRunner()->inProcessRendererHost()->id()
            : d_browserMainRunner->createInProcessRendererHost()->id();
    }
    else if (params.rendererAffinity() == Constants::ANY_OUT_OF_PROCESS_RENDERER) {
        hostAffinity = content::SiteInstance::kNoProcessAffinity;
    }
    else {
        DCHECK(0 <= params.rendererAffinity());
        RendererToHostIdMap::iterator it =
            d_rendererToHostIdMap.find(params.rendererAffinity());
        if (it == d_rendererToHostIdMap.end()) {
            hostAffinity = content::RenderProcessHostImpl::GenerateUniqueId();
            d_rendererToHostIdMap[params.rendererAffinity()] = hostAffinity;
        }
        else
            hostAffinity = it->second;
    }

    if (Statics::isRendererMainThreadMode()) {
        DCHECK(d_browserThread.get());

        content::BrowserContext* browserContext
            = d_browserThread->mainRunner()->browserContext();

        return new WebViewProxy(delegate,
                                parent,
                                d_browserThread->messageLoop(),
                                browserContext,
                                hostAffinity,
                                params.initiallyVisible());
    }
    else if (Statics::isOriginalThreadMode()) {
        content::BrowserContext* browserContext
            = d_browserMainRunner->browserContext();

        return new WebViewImpl(delegate,
                               parent,
                               browserContext,
                               hostAffinity,
                               params.initiallyVisible());
    }

    NOTREACHED();
    return 0;
}

void ToolkitImpl::onRootWindowPositionChanged(gfx::NativeView root)
{
    if (Statics::isRendererMainThreadMode()) {
        DCHECK(d_browserThread);
        d_browserThread->messageLoop()->PostTask(FROM_HERE,
            base::Bind(&content::WebContentsViewWin::onRootWindowPositionChanged, root));
    }
}

void ToolkitImpl::onRootWindowSettingChange(gfx::NativeView root)
{
    if (Statics::isRendererMainThreadMode()) {
        DCHECK(d_browserThread);
        d_browserThread->messageLoop()->PostTask(FROM_HERE,
            base::Bind(&content::WebContentsViewWin::onRootWindowSettingChange, root));
    }
}

bool ToolkitImpl::preHandleMessage(const NativeMsg* msg)
{
    return MainMessagePump::current()->preHandleMessage(*msg);
}

void ToolkitImpl::postHandleMessage(const NativeMsg* msg)
{
    MainMessagePump::current()->postHandleMessage(*msg);
}

}  // close namespace blpwtk2

