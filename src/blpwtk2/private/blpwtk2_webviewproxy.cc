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
#include <blpwtk2_mainmessagepump.h>
#include <blpwtk2_newviewparams.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>
#include <blpwtk2_webviewimpl.h>
#include <blpwtk2_mediarequestimpl.h>

#include <base/bind.h>
#include <base/message_loop.h>
#include <content/public/renderer/render_view.h>
#include <ui/gfx/size.h>

namespace blpwtk2 {

WebViewProxy::WebViewProxy(WebViewDelegate* delegate,
                           gfx::NativeView parent,
                           base::MessageLoop* implDispatcher,
                           Profile* profile,
                           int hostAffinity,
                           bool initiallyVisible,
                           bool isInProcess)
: d_impl(0)
, d_implDispatcher(implDispatcher)
, d_proxyDispatcher(base::MessageLoop::current())
, d_delegate(delegate)
, d_routingId(0)
, d_implMoveAckPending(false)
, d_moveAckPending(false)
, d_wasDestroyed(false)
, d_isMainFrameAccessible(false)
, d_isInProcess(isInProcess)
, d_gotRendererInfo(false)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(profile);
    ++Statics::numWebViews;

    AddRef();  // this is balanced in destroy()

    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implInit, this, parent, profile,
                   hostAffinity, initiallyVisible));
}

WebViewProxy::WebViewProxy(WebViewImpl* impl,
                           base::MessageLoop* implDispatcher,
                           base::MessageLoop* proxyDispatcher,
                           bool isInProcess)
: d_impl(impl)
, d_implDispatcher(implDispatcher)
, d_proxyDispatcher(proxyDispatcher)
, d_delegate(0)
, d_routingId(0)
, d_implMoveAckPending(false)
, d_moveAckPending(false)
, d_wasDestroyed(false)
, d_isMainFrameAccessible(false)
, d_isInProcess(isInProcess)
, d_gotRendererInfo(false)
{
    DCHECK(base::MessageLoop::current() == implDispatcher);
    ++Statics::numWebViews;

    AddRef();  // this is balanced in destroy()

    impl->setImplClient(this);
}

WebViewProxy::~WebViewProxy()
{
    DCHECK(d_wasDestroyed);
}

void WebViewProxy::destroy()
{
    DCHECK(Statics::isInApplicationMainThread());

    DCHECK(!d_wasDestroyed);
    DCHECK(0 < Statics::numWebViews);
    --Statics::numWebViews;
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
        << "You should wait for didFinishLoad";

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

void WebViewProxy::find(const StringRef& text, bool matchCase, bool forward)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);

    if (!d_find) d_find.reset(new FindOnPage());

    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implFind, this,
                   d_find->makeRequest(text, matchCase, forward)));
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

void WebViewProxy::inspectElementAt(const POINT& point)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implInspectElementAt, this, point));
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

void WebViewProxy::move(int left, int top, int width, int height)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    DCHECK(0 <= width);
    DCHECK(0 <= height);

    gfx::Rect rc(left, top, width, height);

    if (d_rect == rc) {
        return;
    }

    DCHECK(!d_moveAckPending);
    DCHECK(!d_implMoveAckPending);

    d_rect = rc;

    // The logic here is as follows:
    // * If we haven't got any renderer info (e.g. if the renderer hasn't been
    //   navigated to yet, or if it is out-of-process), then we will just do an
    //   asynchronous move.
    // * If we got the renderer info, that means it is in-process, and we can
    //   resize the RenderView in our thread, and wait for an ack from the
    //   browser thread before returning.  This ensures the move() method is
    //   synchronous.  This prevents the noticeable lag when resizing a window,
    //   and the contents update out-of-sync with the main window.
    // * We get the ack in one of two ways:
    //   * the browser thread sees that it already has a backing store with the
    //     same size.
    //   * the browser thread gets the backing store updated with the new size.

    if (!d_gotRendererInfo) {
        d_implDispatcher->PostTask(
            FROM_HERE,
            base::Bind(&WebViewProxy::implMove, this, left, top, width, height));
        return;
    }

    DCHECK(d_isInProcess);

    content::RenderView* rv = content::RenderView::FromRoutingID(d_routingId);
    DCHECK(rv);

    d_moveAckPending = true;
    d_implDispatcher->PostTask(FROM_HERE,
                               base::Bind(&WebViewProxy::implSyncMove, this, d_rect));

    if (!d_rect.IsEmpty()) {
        rv->SetSize(d_rect.size());
    }

    // Wait for the move ack.
    MainMessagePump::current()->pumpUntilConditionIsTrue(
        base::Bind(&WebViewProxy::isMoveAckNotPending, base::Unretained(this)));

    DCHECK(rv->GetSize() == d_rect.size() || d_rect.IsEmpty());
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

void WebViewProxy::enableNCHitTest(bool enabled)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implEnableNCHitTest, this, enabled));
}

void WebViewProxy::onNCHitTestResult(int x, int y, int result)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implOnNCHitTestResult, this, x, y, result));
}

void WebViewProxy::performCustomContextMenuAction(int actionId)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implPerformCustomContextMenuAction, this, actionId));
}

void WebViewProxy::enableCustomTooltip(bool enabled)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implEnableCustomTooltip, this, enabled));
}

void WebViewProxy::setZoomPercent(int value)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implSetZoomPercent, this, value));
}

void WebViewProxy::replaceMisspelledRange(const StringRef& text)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_wasDestroyed);
    std::string stext(text.data(), text.length());
    d_implDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::implReplaceMisspelledRange, this, stext));
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

void WebViewProxy::didFinishLoad(WebView* source, const StringRef& url)
{
    DCHECK(source == d_impl);
    std::string surl(url.data(), url.length());
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyDidFinishLoad, this, surl));
}

void WebViewProxy::didFailLoad(WebView* source, const StringRef& url)
{
    DCHECK(source == d_impl);
    std::string surl(url.data(), url.length());
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyDidFailLoad, this, surl));
}

void WebViewProxy::didCreateNewView(WebView* source,
                                    WebView* newView,
                                    const NewViewParams& params,
                                    WebViewDelegate** newViewDelegate)
{
    DCHECK(source == d_impl);
    WebViewProxy* newProxy = new WebViewProxy(static_cast<WebViewImpl*>(newView),
                                              d_implDispatcher,
                                              d_proxyDispatcher,
                                              d_isInProcess);
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

void WebViewProxy::handleExternalProtocol(WebView* source, const StringRef& url)
{
    DCHECK(source == d_impl);
    std::string surl(url.data(), url.length());
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyHandleExternalProtocol, this, surl));
}

void WebViewProxy::moveView(WebView* source, int x, int y, int width, int height)
{
    DCHECK(source == d_impl);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyMoveView, this, x, y, width, height));
}

void WebViewProxy::requestNCHitTest(WebView* source)
{
    DCHECK(source == d_impl);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyRequestNCHitTest, this));
}

void WebViewProxy::showTooltip(WebView* source, const String& tooltipText, TextDirection::Value direction)
{
    DCHECK(source == d_impl);
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyShowTooltip, this, tooltipText, direction));
}

void WebViewProxy::findState(WebView* source,
                             int numberOfMatches,
                             int activeMatchOrdinal,
                             bool finalUpdate)
{
    NOTREACHED() << "findState should come in via findStateWithReqId";
}

bool WebViewProxy::shouldDisableBrowserSideResize()
{
    return d_isInProcess;
}

void WebViewProxy::aboutToNativateRenderView(int routingId)
{
    if (d_isInProcess) {
        d_proxyDispatcher->PostTask(FROM_HERE,
            base::Bind(&WebViewProxy::proxyAboutToNavigateRenderView,
                       this, routingId));
    }
}

void WebViewProxy::didUpdatedBackingStore(const gfx::Size& size)
{
    if (d_implMoveAckPending && d_implRect.size() == size) {
        DCHECK(d_isInProcess);
        d_implMoveAckPending = false;
        d_impl->move(d_implRect.x(), d_implRect.y(),
                     d_implRect.width(), d_implRect.height());
        d_proxyDispatcher->PostTask(FROM_HERE,
            base::Bind(&WebViewProxy::proxyMoveAck, this));
    }
}

void WebViewProxy::findStateWithReqId(int reqId,
                                      int numberOfMatches,
                                      int activeMatchOrdinal,
                                      bool finalUpdate)
{
    d_proxyDispatcher->PostTask(FROM_HERE,
        base::Bind(&WebViewProxy::proxyFindState,
                   this, reqId,
                   numberOfMatches,
                   activeMatchOrdinal,
                   finalUpdate));
}

void WebViewProxy::implInit(gfx::NativeView parent,
                            Profile* profile,
                            int hostAffinity,
                            bool initiallyVisible)
{
    d_impl = new WebViewImpl(this, parent, profile, hostAffinity,
                             initiallyVisible);
    d_impl->setImplClient(this);
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

void WebViewProxy::implFind(const FindOnPage::Request& request)
{
    DCHECK(d_impl);
    d_impl->handleFindRequest(request);
}

void WebViewProxy::implLoadInspector(WebView* inspectedView)
{
    DCHECK(d_impl);
    WebViewProxy* inspectedViewProxy
        = static_cast<WebViewProxy*>(inspectedView);
    DCHECK(inspectedViewProxy->d_impl);
    d_impl->loadInspector(inspectedViewProxy->d_impl);
}

void WebViewProxy::implInspectElementAt(const POINT& point)
{
    DCHECK(d_impl);
    d_impl->inspectElementAt(point);
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

void WebViewProxy::implSyncMove(const gfx::Rect& rc)
{
    DCHECK(d_impl);
    DCHECK(d_isInProcess);
    DCHECK(!d_implMoveAckPending);

    // We don't get backing store updates for empty rectangles, so just move
    // the WebView and send an ack back.  Also, if the browser-side renderer
    // already matches the size before we get here, then we already have a
    // backing store, so move the WebView and send an ack.  This can happen
    // if the size didn't actually change, or if the browser thread pulled
    // out the backing store update msg before we reach this point.
    if (rc.IsEmpty() || d_impl->rendererMatchesSize(rc.size())) {
        d_impl->move(rc.x(), rc.y(), rc.width(), rc.height());
        d_proxyDispatcher->PostTask(FROM_HERE,
            base::Bind(&WebViewProxy::proxyMoveAck, this));
    }
    else {
        // We need to wait for didUpdatedBackingStore in order to ack the move.
        d_implMoveAckPending = true;
        d_implRect = rc;
    }
}

void WebViewProxy::implMove(int left, int top, int width, int height)
{
    DCHECK(d_impl);
    d_impl->move(left, top, width, height);
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

void WebViewProxy::implEnableNCHitTest(bool enabled)
{
    DCHECK(d_impl);
    d_impl->enableNCHitTest(enabled);
}

void WebViewProxy::implOnNCHitTestResult(int x, int y, int result)
{
    DCHECK(d_impl);
    d_impl->onNCHitTestResult(x, y, result);
}

void WebViewProxy::implPerformCustomContextMenuAction(int actionId)
{
    DCHECK(d_impl);
    d_impl->performCustomContextMenuAction(actionId);
}

void WebViewProxy::implEnableCustomTooltip(bool enabled)
{
    DCHECK(d_impl);
    d_impl->enableCustomTooltip(enabled);
}

void WebViewProxy::implSetZoomPercent(int value)
{
    DCHECK(d_impl);
    d_impl->setZoomPercent(value);
}

void WebViewProxy::implReplaceMisspelledRange(const std::string& text)
{
    DCHECK(d_impl);
    d_impl->replaceMisspelledRange(text);
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
    if (d_delegate && !d_wasDestroyed)
        d_delegate->didNavigateMainFramePostCommit(this, url);
}

void WebViewProxy::proxyDidFinishLoad(const std::string& url)
{
    d_isMainFrameAccessible = true;  // wait until we receive this
                                     // notification before we make the
                                     // mainFrame accessible

    if (d_delegate && !d_wasDestroyed)
        d_delegate->didFinishLoad(this, url);
}

void WebViewProxy::proxyDidFailLoad(const std::string& url)
{
    if (d_delegate && !d_wasDestroyed)
        d_delegate->didFailLoad(this, url);
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

void WebViewProxy::proxyHandleExternalProtocol(const std::string& url)
{
    if (d_delegate && !d_wasDestroyed)
        d_delegate->handleExternalProtocol(this, url);
}

void WebViewProxy::proxyMoveView(int x, int y, int width, int height)
{
    if (d_delegate && !d_wasDestroyed) {
        d_delegate->moveView(this, x, y, width, height);
    }
}

void WebViewProxy::proxyRequestNCHitTest()
{
    if (d_wasDestroyed) return;
    if (!d_delegate) {
        onNCHitTestResult(0, 0, HTERROR);
    }
    else {
        d_delegate->requestNCHitTest(this);
    }
}

void WebViewProxy::proxyShowTooltip(const String& tooltipText, TextDirection::Value direction)
{
    if (d_delegate && !d_wasDestroyed) {
        d_delegate->showTooltip(this, tooltipText, direction);
    }
}

void WebViewProxy::proxyFindState(int reqId,
                                  int numberOfMatches,
                                  int activeMatchOrdinal,
                                  bool finalUpdate)
{
    DCHECK(d_find);
    if (d_delegate && !d_wasDestroyed &&
        d_find->applyUpdate(reqId, numberOfMatches, activeMatchOrdinal)) {
        d_delegate->findState(this,
                              d_find->numberOfMatches(),
                              d_find->activeMatchIndex(),
                              finalUpdate);
    }
}

void WebViewProxy::proxyMoveAck()
{
    DCHECK(!d_wasDestroyed);
    DCHECK(d_moveAckPending);
    d_moveAckPending = false;
}

void WebViewProxy::proxyAboutToNavigateRenderView(int routingId)
{
    DCHECK(d_isInProcess);
    if (d_wasDestroyed) return;

    content::RenderView* rv = content::RenderView::FromRoutingID(routingId);
    if (!rv) {
        // The RenderView has not been created yet.  Keep reposting this task
        // until the RenderView is available.
        d_proxyDispatcher->PostTask(
            FROM_HERE,
            base::Bind(&WebViewProxy::proxyAboutToNavigateRenderView,
                       this,
                       routingId));
        return;
    }

    d_gotRendererInfo = true;
    d_routingId = routingId;

    if (!d_rect.IsEmpty()) {
        rv->SetSize(d_rect.size());
    }
}

}  // close namespace blpwtk2

