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

#include <blpwtk2_webviewproxy.h>

#include <blpwtk2_contextmenuparams.h>
#include <blpwtk2_newviewparams.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>
#include <blpwtk2_webviewimpl.h>
#include <blpwtk2_mediarequestimpl.h>

#include <base/bind.h>
#include <base/message_loop.h>

namespace blpwtk2 {

WebViewProxy::WebViewProxy(WebViewDelegate* delegate,
                           gfx::NativeView parent,
                           MessageLoop* implDispatcher,
                           content::BrowserContext* browserContext,
                           int hostAffinity,
                           bool initiallyVisible)
: d_impl(0)
, d_implDispatcher(implDispatcher)
, d_proxyDispatcher(MessageLoop::current())
, d_delegate(delegate)
, d_wasDestroyed(false)
, d_isMainFrameAccessible(false)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(browserContext);

    AddRef();  // this is balanced in destroy()

    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implInit, this, parent, browserContext,
                   hostAffinity, initiallyVisible));
}

WebViewProxy::WebViewProxy(WebViewImpl* impl,
                           MessageLoop* implDispatcher,
                           MessageLoop* proxyDispatcher)
: d_impl(impl)
, d_implDispatcher(implDispatcher)
, d_proxyDispatcher(proxyDispatcher)
, d_delegate(0)
, d_wasDestroyed(false)
, d_isMainFrameAccessible(false)
{
    DCHECK(MessageLoop::current() == implDispatcher);

    AddRef();  // this is balanced in destroy()
}

WebViewProxy::~WebViewProxy()
{
    DCHECK(d_wasDestroyed);
}

void WebViewProxy::destroy()
{
    DCHECK(Statics::isInApplicationMainThread());

    DCHECK(!d_wasDestroyed);
    d_wasDestroyed = true;
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implDestroy, this));

    Release();  // balance the AddRef() from the constructor
}

WebFrame* WebViewProxy::mainFrame()
{
    DCHECK(!d_wasDestroyed);
    DCHECK(Statics::isRendererMainThreadMode());
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(d_isMainFrameAccessible)
        << "You should wait for didNavigateMainFramePostCommit";

    return d_impl->mainFrame();
}

void WebViewProxy::loadUrl(const StringRef& url)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    std::string surl(url.data(), url.length());
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implLoadUrl, this, surl));
}

void WebViewProxy::loadInspector(WebView* inspectedView)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    DCHECK(inspectedView);
    WebViewProxy* inspectedViewProxy
        = static_cast<WebViewProxy*>(inspectedView);
    DCHECK(!inspectedViewProxy->d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implLoadInspector, this, inspectedView));
}

void WebViewProxy::reload(bool ignoreCache)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implReload, this, ignoreCache));
}

void WebViewProxy::goBack()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implGoBack, this));
}

void WebViewProxy::goForward()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implGoForward, this));
}

void WebViewProxy::stop()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implStop, this));
}

void WebViewProxy::focus()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implFocus, this));
}

void WebViewProxy::show()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implShow, this));
}

void WebViewProxy::hide()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implHide, this));
}

void WebViewProxy::setParent(NativeView parent)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implSetParent, this, parent));
}

void WebViewProxy::move(int left, int top, int width, int height, bool repaint)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implMove, this,
                   left, top, width, height, repaint));
}

void WebViewProxy::cutSelection()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implCutSelection, this));
}

void WebViewProxy::copySelection()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implCopySelection, this));
}

void WebViewProxy::paste()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implPaste, this));
}

void WebViewProxy::deleteSelection()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implDeleteSelection, this));
}

void WebViewProxy::enableFocusBefore(bool enabled)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implEnableFocusBefore, this, enabled));
}

void WebViewProxy::enableFocusAfter(bool enabled)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implEnableFocusAfter, this, enabled));
}

void WebViewProxy::performCustomContextMenuAction(int actionId)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implPerformCustomContextMenuAction, this, actionId));
}

void WebViewProxy::updateTargetURL(WebView* source, const StringRef& url)
{
    DCHECK(source == d_impl);
    std::string surl(url.data(), url.length());
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyUpdateTargetURL, this, surl));
}

void WebViewProxy::updateNavigationState(WebView* source,
                                         const NavigationState& state)
{
    DCHECK(source == d_impl);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyUpdateNavigationState, this, state));
}

void WebViewProxy::didNavigateMainFramePostCommit(WebView* source, const StringRef& url)
{
    DCHECK(source == d_impl);
    std::string surl(url.data(), url.length());
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyDidNavigateMainFramePostCommit, this, surl));
}

void WebViewProxy::didCreateNewView(WebView* source,
                                    WebView* newView,
                                    const NewViewParams& params,
                                    WebViewDelegate** newViewDelegate)
{
    DCHECK(source == d_impl);
    WebViewProxy* newProxy = new WebViewProxy(static_cast<WebViewImpl*>(newView),
                                              d_implDispatcher,
                                              d_proxyDispatcher);
    *newViewDelegate = newProxy;
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyDidCreateNewView, this, newProxy, params));
}

void WebViewProxy::destroyView(WebView* source)
{
    DCHECK(source == d_impl);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyDestroyView, this));
}

void WebViewProxy::focusBefore(WebView* source)
{
    DCHECK(source == d_impl);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyFocusBefore, this));
}

void WebViewProxy::focusAfter(WebView* source)
{
    DCHECK(source == d_impl);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyFocusAfter, this));
}

void WebViewProxy::focused(WebView* source)
{
    DCHECK(source == d_impl);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyFocused, this));
}

void WebViewProxy::showContextMenu(WebView* source, const ContextMenuParams& params)
{
    DCHECK(source == d_impl);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyShowContextMenu, this, params));
}

void WebViewProxy::handleMediaRequest(WebView* source, MediaRequest* mediaRequest)
{
    DCHECK(source == d_impl);
    DCHECK(mediaRequest);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyHandleMediaRequest, this,
                    make_scoped_refptr(static_cast<MediaRequestImpl*>(mediaRequest))));
}

void WebViewProxy::implInit(gfx::NativeView parent,
                            content::BrowserContext* browserContext,
                            int hostAffinity,
                            bool initiallyVisible)
{
    d_impl = new WebViewImpl(this, parent, browserContext, hostAffinity,
                             initiallyVisible);
}

void WebViewProxy::implDestroy()
{
    DCHECK(d_impl);
    d_impl->destroy();
    d_impl = 0;
}

void WebViewProxy::implLoadUrl(const std::string& url)
{
    DCHECK(d_impl);
    d_impl->loadUrl(url);
}

void WebViewProxy::implLoadInspector(WebView* inspectedView)
{
    DCHECK(d_impl);
    WebViewProxy* inspectedViewProxy
        = static_cast<WebViewProxy*>(inspectedView);
    DCHECK(inspectedViewProxy->d_impl);
    d_impl->loadInspector(inspectedViewProxy->d_impl);
}

void WebViewProxy::implReload(bool ignoreCache)
{
    DCHECK(d_impl);
    d_impl->reload(ignoreCache);
}

void WebViewProxy::implGoBack()
{
    DCHECK(d_impl);
    d_impl->goBack();
}

void WebViewProxy::implGoForward()
{
    DCHECK(d_impl);
    d_impl->goForward();
}

void WebViewProxy::implStop()
{
    DCHECK(d_impl);
    d_impl->stop();
}

void WebViewProxy::implFocus()
{
    DCHECK(d_impl);
    d_impl->focus();
}

void WebViewProxy::implShow()
{
    DCHECK(d_impl);
    d_impl->show();
}

void WebViewProxy::implHide()
{
    DCHECK(d_impl);
    d_impl->hide();
}

void WebViewProxy::implSetParent(NativeView parent)
{
    DCHECK(d_impl);
    d_impl->setParent(parent);
}

void WebViewProxy::implMove(int left, int top, int width, int height, bool repaint)
{
    DCHECK(d_impl);
    d_impl->move(left, top, width, height, repaint);
}

void WebViewProxy::implCutSelection()
{
    DCHECK(d_impl);
    d_impl->cutSelection();
}

void WebViewProxy::implCopySelection()
{
    DCHECK(d_impl);
    d_impl->copySelection();
}

void WebViewProxy::implPaste()
{
    DCHECK(d_impl);
    d_impl->paste();
}

void WebViewProxy::implDeleteSelection()
{
    DCHECK(d_impl);
    d_impl->deleteSelection();
}

void WebViewProxy::implEnableFocusBefore(bool enabled)
{
    DCHECK(d_impl);
    d_impl->enableFocusBefore(enabled);
}

void WebViewProxy::implEnableFocusAfter(bool enabled)
{
    DCHECK(d_impl);
    d_impl->enableFocusAfter(enabled);
}

void WebViewProxy::implPerformCustomContextMenuAction(int actionId)
{
    DCHECK(d_impl);
    d_impl->performCustomContextMenuAction(actionId);
}

void WebViewProxy::proxyUpdateTargetURL(const std::string& url)
{
    if (d_delegate && !d_wasDestroyed)
        d_delegate->updateTargetURL(this, url);
}

void WebViewProxy::proxyUpdateNavigationState(const NavigationState& state)
{
    if (d_delegate && !d_wasDestroyed)
        d_delegate->updateNavigationState(this, state);
}

void WebViewProxy::proxyDidNavigateMainFramePostCommit(const std::string& url)
{
    d_isMainFrameAccessible = true;  // wait until we receive this
                                     // notification before we make the
                                     // mainFrame accessible

    if (d_delegate && !d_wasDestroyed)
        d_delegate->didNavigateMainFramePostCommit(this, url);
}

void WebViewProxy::proxyDidCreateNewView(WebViewProxy* newProxy,
                                         const NewViewParams& params)
{
    if (d_wasDestroyed || !d_delegate) {
        newProxy->destroy();
        return;
    }

    d_delegate->didCreateNewView(this, newProxy, params,
                                 &newProxy->d_delegate);
}

void WebViewProxy::proxyDestroyView()
{
    if (d_wasDestroyed) return;
    if (!d_delegate) {
        destroy();
        return;
    }

    d_delegate->destroyView(this);
}

void WebViewProxy::proxyFocusBefore()
{
    if (d_delegate && !d_wasDestroyed)
        d_delegate->focusBefore(this);
}

void WebViewProxy::proxyFocusAfter()
{
    if (d_delegate && !d_wasDestroyed)
        d_delegate->focusAfter(this);
}

void WebViewProxy::proxyFocused()
{
    if (d_delegate && !d_wasDestroyed)
        d_delegate->focused(this);
}

void WebViewProxy::proxyShowContextMenu(const ContextMenuParams& params)
{
    if (d_delegate && !d_wasDestroyed)
        d_delegate->showContextMenu(this, params);
}

void WebViewProxy::proxyHandleMediaRequest(MediaRequest* request)
{
    if (d_delegate && !d_wasDestroyed){
        d_delegate->handleMediaRequest(this, request);
    }
}

}  // close namespace blpwtk2

