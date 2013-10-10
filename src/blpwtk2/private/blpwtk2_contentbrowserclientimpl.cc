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

#include <blpwtk2_contentbrowserclientimpl.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_mediaobserverimpl.h>
#include <blpwtk2_urlrequestcontextgetterimpl.h>
#include <blpwtk2_webcontentsviewdelegateimpl.h>

#include <base/message_loop.h>
#include <base/threading/thread.h>
#include <base/threading/platform_thread.h>
#include <content/public/browser/render_view_host.h>
#include <content/public/browser/render_process_host.h>
#include <content/public/renderer/render_thread.h>
#include <content/public/browser/resource_dispatcher_host.h>
#include <content/public/browser/resource_dispatcher_host_delegate.h>

namespace blpwtk2 {

namespace {

class InProcessRendererThread : public base::Thread {
public:
    InProcessRendererThread(const std::string& channelId)
    : base::Thread("BlpInProcRenderer")
    , d_channelId(channelId)
    {
        base::Thread::Options options;
        options.message_loop_type = base::MessageLoop::TYPE_UI;
        StartWithOptions(options);
    }
    ~InProcessRendererThread()
    {
        Stop();
    }

private:
    // Called just prior to starting the message loop
    virtual void Init() OVERRIDE
    {
        Statics::rendererMessageLoop = message_loop();
        content::RenderThread::InitInProcessRenderer(d_channelId);
    }

    // Called just after the message loop ends
    virtual void CleanUp()
    {
        content::RenderThread::CleanUpInProcessRenderer();
        Statics::rendererMessageLoop = 0;
    }

    std::string d_channelId;

    DISALLOW_COPY_AND_ASSIGN(InProcessRendererThread);
};
InProcessRendererThread* g_inProcessRendererThread = 0;

class ResourceDispatcherHostDelegate : public content::ResourceDispatcherHostDelegate
{
public:
    static ResourceDispatcherHostDelegate& Get()
    {
        static ResourceDispatcherHostDelegate instance;
        return instance;
    }

    virtual bool HandleExternalProtocol(const GURL& url,
                                        int child_id,
                                        int route_id);

private:
    ResourceDispatcherHostDelegate() {}
    DISALLOW_COPY_AND_ASSIGN(ResourceDispatcherHostDelegate);

    static void doHandleExternalProtocol(const GURL& url,
                                         int child_id,
                                         int route_id);
};

}  // close unnamed namespace

bool ResourceDispatcherHostDelegate::HandleExternalProtocol(const GURL& url,
                                                            int child_id,
                                                            int route_id)
{
    if (Statics::isInBrowserMainThread()) {
        doHandleExternalProtocol(url, child_id, route_id);
    }
    else {
        Statics::browserMainMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&doHandleExternalProtocol, url, child_id, route_id));
    }
    return true;
}

void ResourceDispatcherHostDelegate::doHandleExternalProtocol(const GURL& url,
                                                              int child_id,
                                                              int route_id)
{
    DCHECK(Statics::isInBrowserMainThread());

    content::RenderViewHost* viewHost = 
        content::RenderViewHost::FromID(child_id, route_id);

    viewHost->HandleExternalProtocol(url);
}

ContentBrowserClientImpl::ContentBrowserClientImpl()
{
}

ContentBrowserClientImpl::~ContentBrowserClientImpl()
{
}

void ContentBrowserClientImpl::RenderProcessHostCreated(
    content::RenderProcessHost* host)
{
    DCHECK(Statics::isInBrowserMainThread());
    int renderer = Statics::hostIdToRenderer(host->GetID());
    if (Statics::rendererUsesInProcessPlugins(renderer)) {
        host->SetUsesInProcessPlugins();
    }
}

bool ContentBrowserClientImpl::SupportsInProcessRenderer()
{
    return true;
}

void ContentBrowserClientImpl::StartInProcessRendererThread(
    const std::string& channel_id)
{
    DCHECK(Statics::isInBrowserMainThread());

    if (Statics::isRendererMainThreadMode()) {
        DCHECK(Statics::rendererMessageLoop);
        Statics::rendererMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&content::RenderThread::InitInProcessRenderer,
                       channel_id));
    }
    else {
        DCHECK(Statics::isOriginalThreadMode());
        DCHECK(!g_inProcessRendererThread);
        g_inProcessRendererThread = new InProcessRendererThread(channel_id);
    }
}

void ContentBrowserClientImpl::ResourceDispatcherHostCreated() {
    content::ResourceDispatcherHost::Get()->SetDelegate(
        &ResourceDispatcherHostDelegate::Get());
}

void ContentBrowserClientImpl::StopInProcessRendererThread()
{
    DCHECK(Statics::isInBrowserMainThread());

    if (Statics::isRendererMainThreadMode()) {
        // Don't do anything here, ToolkitImpl will directly call
        // content::RenderThread::CleanUpInProcessRenderer when it shuts down,
        // because the order of destruction needs to be very specific,
        // otherwise all kinds of crashes occur.
        // TODO: see if we can make RenderThread::CleanUpInProcessRenderer
        // TODO: callable from here.
    }
    else {
        DCHECK(Statics::isOriginalThreadMode());
        DCHECK(g_inProcessRendererThread);
        delete g_inProcessRendererThread;
        g_inProcessRendererThread = 0;
    }
}

content::WebContentsViewDelegate*
ContentBrowserClientImpl::GetWebContentsViewDelegate(content::WebContents* webContents)
{
    return new WebContentsViewDelegateImpl(webContents);
}

net::URLRequestContextGetter* ContentBrowserClientImpl::CreateRequestContext(
    content::BrowserContext* browserContext,
    content::ProtocolHandlerMap* protocolHandlers)
{
    BrowserContextImpl* contextImpl
        = static_cast<BrowserContextImpl*>(browserContext);

    if (!contextImpl->requestContextGetter()) {
        new URLRequestContextGetterImpl(contextImpl, protocolHandlers);
        DCHECK(contextImpl->requestContextGetter());
    }

    return contextImpl->requestContextGetter();
}

content::MediaObserver* ContentBrowserClientImpl::GetMediaObserver()
{
    if (!d_mediaObserver.get()) {
        d_mediaObserver.reset(new MediaObserverImpl());
    }
    return d_mediaObserver.get();
}

}  // close namespace blpwtk2

