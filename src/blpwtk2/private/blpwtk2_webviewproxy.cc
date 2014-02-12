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

#include <blpwtk2_constants.h>
#include <blpwtk2_contextmenuparams.h>
#include <blpwtk2_mainmessagepump.h>
#include <blpwtk2_newviewparams.h>
#include <blpwtk2_processclient.h>
#include <blpwtk2_profileproxy.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>
#include <blpwtk2_webframeimpl.h>
#include <blpwtk2_webviewdelegate.h>
#include <blpwtk2_webview_messages.h>

#include <base/bind.h>
#include <base/message_loop.h>
#include <content/public/renderer/render_view.h>
#include <third_party/WebKit/public/web/WebView.h>
#include <ui/gfx/size.h>

namespace blpwtk2 {

WebViewProxy::WebViewProxy(ProcessClient* processClient,
                           int routingId,
                           ProfileProxy* profileProxy,
                           WebViewDelegate* delegate,
                           gfx::NativeView parent,
                           int rendererAffinity,
                           bool initiallyVisible,
                           bool takeFocusOnMouseDown)
: d_profileProxy(profileProxy)
, d_processClient(processClient)
, d_delegate(delegate)
, d_routingId(routingId)
, d_rendererRoutingId(0)
, d_moveAckPending(false)
, d_isMainFrameAccessible(false)
, d_gotRendererInfo(false)
, d_ncDragNeedsAck(false)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(profileProxy);
    DCHECK(d_processClient);

    profileProxy->incrementWebViewCount();
    d_processClient->addRoute(d_routingId, this);

    BlpWebViewHostMsg_NewParams params;
    params.routingId = routingId;
    params.profileId = profileProxy->routingId();
    params.initiallyVisible = initiallyVisible;
    params.takeFocusOnMouseDown = takeFocusOnMouseDown;
    params.rendererAffinity = rendererAffinity;
    params.parent = (NativeViewForTransit)parent;
    Send(new BlpWebViewHostMsg_New(params));
}

WebViewProxy::WebViewProxy(ProcessClient* processClient,
                           int routingId,
                           ProfileProxy* profileProxy)
: d_profileProxy(profileProxy)
, d_processClient(processClient)
, d_delegate(0)
, d_routingId(routingId)
, d_rendererRoutingId(0)
, d_moveAckPending(false)
, d_isMainFrameAccessible(false)
, d_gotRendererInfo(false)
, d_ncDragNeedsAck(false)
{
    profileProxy->incrementWebViewCount();
    d_processClient->addRoute(d_routingId, this);
}

WebViewProxy::~WebViewProxy()
{
}

void WebViewProxy::destroy()
{
    DCHECK(Statics::isInApplicationMainThread());

    d_processClient->removeRoute(d_routingId);
    d_profileProxy->decrementWebViewCount();

    Send(new BlpWebViewHostMsg_Destroy(d_routingId));

    delete this;
}

WebFrame* WebViewProxy::mainFrame()
{
    DCHECK(Statics::isRendererMainThreadMode());
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(d_isMainFrameAccessible)
        << "You should wait for didFinishLoad";
    DCHECK(d_gotRendererInfo);

    if (!d_mainFrame.get()) {
        content::RenderView* rv = content::RenderView::FromRoutingID(d_rendererRoutingId);
        DCHECK(rv);

        WebKit::WebFrame* webFrame = rv->GetWebView()->mainFrame();
        d_mainFrame.reset(new WebFrameImpl(webFrame));
    }

    return d_mainFrame.get();
}

void WebViewProxy::loadUrl(const StringRef& url)
{
    DCHECK(Statics::isInApplicationMainThread());
    std::string surl(url.data(), url.length());
    Send(new BlpWebViewHostMsg_LoadUrl(d_routingId, surl));
}

void WebViewProxy::find(const StringRef& text, bool matchCase, bool forward)
{
    DCHECK(Statics::isInApplicationMainThread());

    if (!d_find) d_find.reset(new FindOnPage());

    FindOnPageRequest request = d_find->makeRequest(text, matchCase, forward);
    Send(new BlpWebViewHostMsg_Find(d_routingId, request));
}

void WebViewProxy::loadInspector(WebView* inspectedView)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(inspectedView);
    WebViewProxy* inspectedViewProxy
        = static_cast<WebViewProxy*>(inspectedView);
    Send(new BlpWebViewHostMsg_LoadInspector(d_routingId,
                                             inspectedViewProxy->routingId()));
}

void WebViewProxy::inspectElementAt(const POINT& point)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_InspectElementAt(d_routingId,
                                                gfx::Point(point)));
}

void WebViewProxy::reload(bool ignoreCache)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_Reload(d_routingId, ignoreCache));
}

void WebViewProxy::goBack()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_GoBack(d_routingId));
}

void WebViewProxy::goForward()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_GoForward(d_routingId));
}

void WebViewProxy::stop()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_Stop(d_routingId));
}

void WebViewProxy::focus()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_Focus(d_routingId));
}

void WebViewProxy::show()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_Show(d_routingId));
}

void WebViewProxy::hide()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_Hide(d_routingId));
}

void WebViewProxy::setParent(NativeView parent)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_SetParent(d_routingId,
                                         (NativeViewForTransit)parent));
}

void WebViewProxy::move(int left, int top, int width, int height)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(0 <= width);
    DCHECK(0 <= height);

    gfx::Rect rc(left, top, width, height);

    if (d_rect == rc) {
        return;
    }

    DCHECK(!d_moveAckPending);

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
        Send(new BlpWebViewHostMsg_Move(d_routingId, d_rect));
        return;
    }

    content::RenderView* rv = content::RenderView::FromRoutingID(d_rendererRoutingId);
    DCHECK(rv);

    d_moveAckPending = true;
    Send(new BlpWebViewHostMsg_SyncMove(d_routingId, d_rect));

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
    Send(new BlpWebViewHostMsg_CutSelection(d_routingId));
}

void WebViewProxy::copySelection()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_CopySelection(d_routingId));
}

void WebViewProxy::paste()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_Paste(d_routingId));
}

void WebViewProxy::deleteSelection()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_DeleteSelection(d_routingId));
}

void WebViewProxy::enableFocusBefore(bool enabled)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_EnableFocusBefore(d_routingId, enabled));
}

void WebViewProxy::enableFocusAfter(bool enabled)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_EnableFocusAfter(d_routingId, enabled));
}

void WebViewProxy::enableNCHitTest(bool enabled)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_EnableNCHitTest(d_routingId, enabled));
}

void WebViewProxy::onNCHitTestResult(int x, int y, int result)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_OnNCHitTestResult(d_routingId, x, y, result));
}

void WebViewProxy::performCustomContextMenuAction(int actionId)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_PerformContextMenuAction(d_routingId,
                                                        actionId));
}

void WebViewProxy::enableCustomTooltip(bool enabled)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_EnableCustomTooltip(d_routingId, enabled));
}

void WebViewProxy::setZoomPercent(int value)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_SetZoomPercent(d_routingId, value));
}

void WebViewProxy::replaceMisspelledRange(const StringRef& text)
{
    DCHECK(Statics::isInApplicationMainThread());
    std::string stext(text.data(), text.length());
    Send(new BlpWebViewHostMsg_ReplaceMisspelledRange(d_routingId, stext));
}

void WebViewProxy::rootWindowPositionChanged()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_RootWindowPositionChanged(d_routingId));
}

void WebViewProxy::rootWindowSettingsChanged()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_RootWindowSettingsChanged(d_routingId));
}

// IPC::Sender override

bool WebViewProxy::Send(IPC::Message* message)
{
    DCHECK(d_processClient);
    return d_processClient->Send(message);
}

// IPC::Listener overrides

bool WebViewProxy::OnMessageReceived(const IPC::Message& message)
{
    bool handled = true;
    bool msgIsOk = true;
    IPC_BEGIN_MESSAGE_MAP_EX(WebViewProxy, message, msgIsOk)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_UpdateTargetURL, onUpdateTargetURL)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_UpdateNavigationState, onUpdateNavigationState)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_DidNavigateMainFramePostCommit, onDidNavigateMainFramePostCommit)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_DidFinishLoad, onDidFinishLoad)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_DidFailLoad, onDidFailLoad)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_DidCreateNewView, onDidCreateNewView)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_DestroyView, onDestroyView)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_FocusBefore, onFocusBefore)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_FocusAfter, onFocusAfter)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_Focused, onFocused)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_ShowContextMenu, onShowContextMenu)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_HandleExternalProtocol, onHandleExternalProtocol)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_MoveView, onMoveView)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_RequestNCHitTest, onRequestNCHitTest)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_NCDragBegin, onNCDragBegin)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_NCDragMove, onNCDragMove)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_NCDragEnd, onNCDragEnd)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_ShowTooltip, onShowTooltip)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_FindState, onFindState)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_MoveAck, onMoveAck)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_AboutToNavigateRenderView, onAboutToNavigateRenderView)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP_EX()

    if (!msgIsOk) {
        LOG(ERROR) << "bad message " << message.type();
    }

    return handled;
}

// Message handlers

void WebViewProxy::onUpdateTargetURL(const std::string& url)
{
    if (d_delegate)
        d_delegate->updateTargetURL(this, url);
}

void WebViewProxy::onUpdateNavigationState(bool canGoBack,
                                           bool canGoForward,
                                           bool isLoading)
{
    if (d_delegate) {
        WebViewDelegate::NavigationState state;
        state.canGoBack = canGoBack;
        state.canGoForward = canGoForward;
        state.isLoading = isLoading;
        d_delegate->updateNavigationState(this, state);
    }
}

void WebViewProxy::onDidNavigateMainFramePostCommit(const std::string& url)
{
    if (d_delegate)
        d_delegate->didNavigateMainFramePostCommit(this, url);
}

void WebViewProxy::onDidFinishLoad(const std::string& url)
{
    d_isMainFrameAccessible = true;  // wait until we receive this
                                     // notification before we make the
                                     // mainFrame accessible

    if (d_delegate)
        d_delegate->didFinishLoad(this, url);
}

void WebViewProxy::onDidFailLoad(const std::string& url)
{
    if (d_delegate)
        d_delegate->didFailLoad(this, url);
}

void WebViewProxy::onDidCreateNewView(int newRoutingId,
                                      const NewViewParams& params)
{
    WebViewProxy* newProxy =
        new WebViewProxy(d_processClient,
                         newRoutingId,
                         d_profileProxy);

    if (!d_delegate) {
        newProxy->destroy();
        return;
    }

    d_delegate->didCreateNewView(this, newProxy, params,
                                 &newProxy->d_delegate);
}

void WebViewProxy::onDestroyView()
{
    if (!d_delegate) {
        destroy();
        return;
    }

    d_delegate->destroyView(this);
}

void WebViewProxy::onFocusBefore()
{
    if (d_delegate)
        d_delegate->focusBefore(this);
}

void WebViewProxy::onFocusAfter()
{
    if (d_delegate)
        d_delegate->focusAfter(this);
}

void WebViewProxy::onFocused()
{
    if (d_delegate)
        d_delegate->focused(this);
}

void WebViewProxy::onShowContextMenu(const ContextMenuParams& params)
{
    if (d_delegate)
        d_delegate->showContextMenu(this, params);
}

void WebViewProxy::onHandleExternalProtocol(const std::string& url)
{
    if (d_delegate)
        d_delegate->handleExternalProtocol(this, url);
}

void WebViewProxy::onMoveView(const gfx::Rect& rect)
{
    if (d_delegate) {
        d_delegate->moveView(this,
                             rect.x(),
                             rect.y(),
                             rect.width(),
                             rect.height());
    }
}

void WebViewProxy::onRequestNCHitTest()
{
    if (!d_delegate) {
        onNCHitTestResult(0, 0, HTERROR);
    }
    else {
        d_delegate->requestNCHitTest(this);
    }
}

void WebViewProxy::onNCDragBegin(int hitTestCode, const gfx::Point& startPoint)
{
    // Keep this in sync with WebViewHost::d_ncDragNeedsAck
    d_ncDragNeedsAck = hitTestCode != HTCAPTION;

    if (d_delegate) {
        d_delegate->ncDragBegin(this, hitTestCode, startPoint.ToPOINT());
    }
}

void WebViewProxy::onNCDragMove()
{
    POINT point;
    ::GetCursorPos(&point);
    if (d_delegate) {
        d_delegate->ncDragMove(this, point);
    }
    if (d_ncDragNeedsAck) {
        Send(new BlpWebViewHostMsg_NCDragMoveAck(d_routingId,
                                                 gfx::Point(point)));
    }
}

void WebViewProxy::onNCDragEnd(const gfx::Point& endPoint)
{
    if (d_delegate) {
        d_delegate->ncDragEnd(this, endPoint.ToPOINT());
    }
    if (d_ncDragNeedsAck) {
        Send(new BlpWebViewHostMsg_NCDragEndAck(d_routingId));
    }
}

void WebViewProxy::onShowTooltip(const std::string& tooltipText, TextDirection::Value direction)
{
    if (d_delegate) {
        d_delegate->showTooltip(this, String(tooltipText), direction);
    }
}

void WebViewProxy::onFindState(int reqId,
                               int numberOfMatches,
                               int activeMatchOrdinal,
                               bool finalUpdate)
{
    DCHECK(d_find);
    if (d_delegate &&
        d_find->applyUpdate(reqId, numberOfMatches, activeMatchOrdinal)) {
        d_delegate->findState(this,
                              d_find->numberOfMatches(),
                              d_find->activeMatchIndex(),
                              finalUpdate);
    }
}

void WebViewProxy::onMoveAck()
{
    DCHECK(d_moveAckPending);
    d_moveAckPending = false;
}

void WebViewProxy::onAboutToNavigateRenderView(int rendererRoutingId)
{
    content::RenderView* rv =
        content::RenderView::FromRoutingID(rendererRoutingId);
    if (!rv) {
        // The RenderView has not been created yet.  Keep reposting this task
        // until the RenderView is available.
        base::MessageLoop::current()->PostTask(
            FROM_HERE,
            base::Bind(&WebViewProxy::onAboutToNavigateRenderView,
                       AsWeakPtr(),
                       rendererRoutingId));
        return;
    }

    d_gotRendererInfo = true;
    d_rendererRoutingId = rendererRoutingId;

    if (!d_rect.IsEmpty()) {
        rv->SetSize(d_rect.size());
    }
}

}  // close namespace blpwtk2

