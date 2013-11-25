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
#include <blpwtk2_inprocessrenderer.h>
#include <blpwtk2_mediaobserverimpl.h>
#include <blpwtk2_urlrequestcontextgetterimpl.h>
#include <blpwtk2_webcontentsviewdelegateimpl.h>

#include <base/message_loop.h>
#include <base/threading/thread.h>
#include <base/threading/platform_thread.h>
#include <content/public/browser/render_view_host.h>
#include <content/public/browser/render_process_host.h>
#include <content/public/browser/resource_dispatcher_host.h>
#include <content/public/browser/resource_dispatcher_host_delegate.h>
#include <chrome/browser/spellchecker/spellcheck_message_filter.h>

namespace blpwtk2 {

namespace {

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
    int id = host->GetID();
    int renderer = Statics::hostIdToRenderer(id);
    if (Statics::rendererUsesInProcessPlugins(renderer)) {
        host->SetUsesInProcessPlugins();
    }
    host->GetChannel()->AddFilter(new SpellCheckMessageFilter(id));
}

bool ContentBrowserClientImpl::SupportsInProcessRenderer()
{
    return true;
}

void ContentBrowserClientImpl::StartInProcessRendererThread(
    const std::string& channel_id)
{
    // This does not actually start the thread.  The thread is started during
    // ToolkitImpl startup.  What we do here is set the channel name that is
    // used by the in-process renderer thread.
    InProcessRenderer::setChannelName(channel_id);
}

void ContentBrowserClientImpl::StopInProcessRendererThread()
{
    // Don't actually stop the thread here.  That is done by ToolkitImpl's
    // shutdown procedure.
}

void ContentBrowserClientImpl::ResourceDispatcherHostCreated()
{
    content::ResourceDispatcherHost::Get()->SetDelegate(
        &ResourceDispatcherHostDelegate::Get());
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

