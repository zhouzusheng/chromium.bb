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
#include <blpwtk2_control_messages.h>
#include <blpwtk2_devtoolshttphandlerdelegateimpl.h>  // for DevToolsManagerDelegateImpl
                                                      // TODO: move this
#include <blpwtk2_statics.h>
#include <blpwtk2_rendererinfomap.h>
#include <blpwtk2_urlrequestcontextgetterimpl.h>
#include <blpwtk2_webcontentsviewdelegateimpl.h>
#include <blpwtk2_webviewimpl.h>

#include <base/message_loop/message_loop.h>
#include <base/threading/thread.h>
#include <base/threading/platform_thread.h>
#include <content/public/browser/render_view_host.h>
#include <content/public/browser/render_process_host.h>
#include <content/public/browser/resource_dispatcher_host.h>
#include <content/public/browser/resource_dispatcher_host_delegate.h>
#include <content/public/browser/web_contents.h>
#include <content/public/common/url_constants.h>
#include <chrome/browser/spellchecker/spellcheck_message_filter.h>
#include <chrome/browser/printing/printing_message_filter.h>

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

    bool HandleExternalProtocol(const GURL& url,
                                int child_id,
                                int route_id,
                                bool is_main_frame,
                                ui::PageTransition page_transition,
                                bool has_user_gesture) override;

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
                                                            int route_id,
                                                            bool is_main_frame,
                                                            ui::PageTransition page_transition,
                                                            bool has_user_gesture)
{
    if (Statics::isInBrowserMainThread()) {
        doHandleExternalProtocol(url, child_id, route_id);
    }
    else {
        DCHECK(Statics::browserMainMessageLoop);
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
    if (!viewHost) {
        // RenderViewHost has gone away.
        return;
    }

    content::WebContents* webContents =
        content::WebContents::FromRenderViewHost(viewHost);
    DCHECK(webContents);

    WebViewImpl* webViewImpl =
        static_cast<WebViewImpl*>(webContents->GetDelegate());
    DCHECK(webViewImpl);

    webViewImpl->handleExternalProtocol(url);
}

ContentBrowserClientImpl::ContentBrowserClientImpl(RendererInfoMap* rendererInfoMap)
: d_rendererInfoMap(rendererInfoMap)
{
    DCHECK(d_rendererInfoMap);
}

ContentBrowserClientImpl::~ContentBrowserClientImpl()
{
}

void ContentBrowserClientImpl::RenderProcessWillLaunch(
    content::RenderProcessHost* host)
{
    DCHECK(Statics::isInBrowserMainThread());
    int id = host->GetID();
    host->AddFilter(new SpellCheckMessageFilter(id));
    host->AddFilter(new printing::PrintingMessageFilter(id));

    if (!Statics::userAgentFromEmbedder().empty()) {
        host->Send(new BlpControlMsg_SetUserAgentFromEmbedder(Statics::userAgentFromEmbedder()));
    }
}

void ContentBrowserClientImpl::OverrideWebkitPrefs(
    content::RenderViewHost* render_view_host,
    content::WebPreferences* prefs)
{
    content::WebContents* webContents =
        content::WebContents::FromRenderViewHost(render_view_host);
    DCHECK(webContents);

    WebViewImpl* webViewImpl =
        static_cast<WebViewImpl*>(webContents->GetDelegate());
    DCHECK(webViewImpl);

    webViewImpl->overrideWebkitPrefs(prefs);
}

bool ContentBrowserClientImpl::SupportsInProcessRenderer()
{
    return !Statics::isInProcessRendererDisabled;
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
    content::ProtocolHandlerMap* protocolHandlers,
    content::URLRequestInterceptorScopedVector requestInterceptors)
{
    BrowserContextImpl* contextImpl
        = static_cast<BrowserContextImpl*>(browserContext);

    contextImpl->requestContextGetter()->setProtocolHandlers(protocolHandlers,
                                                             requestInterceptors.Pass());
    return contextImpl->requestContextGetter();
}

bool ContentBrowserClientImpl::IsHandledURL(const GURL& url)
{
    if (!url.is_valid())
        return false;
    DCHECK_EQ(url.scheme(), base::StringToLowerASCII(url.scheme()));
    // Keep in sync with ProtocolHandlers added by
    // URLRequestContextGetterImpl::GetURLRequestContext().
    static const char* const kProtocolList[] = {
        url::kBlobScheme,
        url::kFileSystemScheme,
        content::kChromeUIScheme,
        content::kChromeDevToolsScheme,
        url::kDataScheme,
        url::kFileScheme,
    };
    for (size_t i = 0; i < arraysize(kProtocolList); ++i) {
        if (url.scheme() == kProtocolList[i])
            return true;
    }
    return false;
}

content::DevToolsManagerDelegate* ContentBrowserClientImpl::GetDevToolsManagerDelegate()
{
    return new DevToolsManagerDelegateImpl();
}

}  // close namespace blpwtk2

