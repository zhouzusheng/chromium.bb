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

#include <blpwtk2_webviewhost.h>

#include <blpwtk2_processhost.h>
#include <blpwtk2_string.h>
#include <blpwtk2_webviewimpl.h>
#include <blpwtk2_webview_messages.h>

#include <ipc/ipc_message_macros.h>

namespace blpwtk2 {

WebViewHost::WebViewHost(ProcessHost* processHost,
                         BrowserContextImpl* browserContext,
                         int routingId,
                         bool isInProcess,
                         NativeView parent,
                         int hostAffinity,
                         bool initiallyVisible,
                         bool takeFocusOnMouseDown,
                         bool domPasteEnabled,
                         bool javascriptCanAccessClipboard)
: d_processHost(processHost)
, d_routingId(routingId)
, d_implMoveAckPending(false)
, d_ncDragAckPending(false)
, d_ncDragNeedsAck(false)
, d_ncDragging(false)
, d_isInProcess(isInProcess)
{
    d_processHost->addRoute(d_routingId, this);

    d_webView = new WebViewImpl(this,
                                parent,
                                browserContext,
                                hostAffinity,
                                initiallyVisible,
                                takeFocusOnMouseDown,
                                domPasteEnabled,
                                javascriptCanAccessClipboard);
    d_webView->setImplClient(this);
}

WebViewHost::WebViewHost(ProcessHost* processHost,
                         WebViewImpl* webView,
                         int routingId,
                         bool isInProcess)
: d_processHost(processHost)
, d_webView(webView)
, d_routingId(routingId)
, d_implMoveAckPending(false)
, d_ncDragAckPending(false)
, d_ncDragNeedsAck(false)
, d_ncDragging(false)
, d_isInProcess(isInProcess)
{
    d_processHost->addRoute(d_routingId, this);
    d_webView->setImplClient(this);
}

WebViewHost::~WebViewHost()
{
    d_webView->destroy();
    d_processHost->removeRoute(d_routingId);
}

// IPC::Listener overrides

bool WebViewHost::OnMessageReceived(const IPC::Message& message)
{
    bool handled = true;
    bool msgIsOk = true;
    IPC_BEGIN_MESSAGE_MAP_EX(WebViewHost, message, msgIsOk)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_LoadUrl, onLoadUrl)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_LoadInspector, onLoadInspector)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_InspectElementAt, onInspectElementAt)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_Reload, onReload)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_GoBack, onGoBack)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_GoForward, onGoForward)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_Stop, onStop)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_Focus, onFocus)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_Show, onShow)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_Hide, onHide)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_SetParent, onSetParent)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_Move, onMove)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_SyncMove, onSyncMove)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_CutSelection, onCutSelection)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_CopySelection, onCopySelection)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_Paste, onPaste)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_DeleteSelection, onDeleteSelection)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_EnableFocusBefore, onEnableFocusBefore)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_EnableFocusAfter, onEnableFocusAfter)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_EnableNCHitTest, onEnableNCHitTest)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_OnNCHitTestResult, onOnNCHitTestResult)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_NCDragMoveAck, onNCDragMoveAck)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_NCDragEndAck, onNCDragEndAck)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_PerformContextMenuAction, onPerformCustomContextMenuAction)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_EnableAltDragRubberbanding, onEnableAltDragRubberbanding)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_EnableCustomTooltip, onEnableCustomTooltip)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_SetZoomPercent, onSetZoomPercent)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_Find, onFind)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_ReplaceMisspelledRange, onReplaceMisspelledRange)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_RootWindowPositionChanged, onRootWindowPositionChanged)
        IPC_MESSAGE_HANDLER(BlpWebViewHostMsg_RootWindowSettingsChanged, onRootWindowSettingsChanged)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP_EX()

    if (!msgIsOk) {
        LOG(ERROR) << "bad message " << message.type();
    }

    return handled;
}

// Message handlers

void WebViewHost::onLoadUrl(const std::string& url)
{
    d_webView->loadUrl(url);
}

void WebViewHost::onLoadInspector(int inspectedViewId)
{
    WebViewHost* inspectedView =
        static_cast<WebViewHost*>(
            d_processHost->findListener(inspectedViewId));
    DCHECK(inspectedView);
    d_webView->loadInspector(inspectedView->d_webView);
}

void WebViewHost::onInspectElementAt(const gfx::Point& point)
{
    d_webView->inspectElementAt(point.ToPOINT());
}

void WebViewHost::onReload(bool ignoreCache)
{
    d_webView->reload(ignoreCache);
}

void WebViewHost::onGoBack()
{
    d_webView->goBack();
}

void WebViewHost::onGoForward()
{
    d_webView->goForward();
}

void WebViewHost::onStop()
{
    d_webView->stop();
}

void WebViewHost::onFocus()
{
    d_webView->focus();
}

void WebViewHost::onShow()
{
    d_webView->show();
}

void WebViewHost::onHide()
{
    d_webView->hide();
}

void WebViewHost::onSetParent(NativeViewForTransit parent)
{
    d_webView->setParent((NativeView)parent);
}

void WebViewHost::onMove(const gfx::Rect& rect)
{
    d_webView->move(rect.x(), rect.y(), rect.width(), rect.height());
}

void WebViewHost::onSyncMove(const gfx::Rect& rect)
{
    DCHECK(d_isInProcess);
    DCHECK(!d_implMoveAckPending);

    // We don't get backing store updates for empty rectangles, so just move
    // the WebView and send an ack back.  Also, if the browser-side renderer
    // already matches the size before we get here, then we already have a
    // backing store, so move the WebView and send an ack.  This can happen
    // if the size didn't actually change, or if the browser thread pulled
    // out the backing store update msg before we reach this point.
    if (rect.IsEmpty() || d_webView->rendererMatchesSize(rect.size())) {
        d_webView->move(rect.x(), rect.y(), rect.width(), rect.height());
        Send(new BlpWebViewMsg_MoveAck(d_routingId));
    }
    else {
        // We need to wait for didUpdatedBackingStore in order to ack the move.
        d_implMoveAckPending = true;
        d_implRect = rect;
    }
}

void WebViewHost::onCutSelection()
{
    d_webView->cutSelection();
}

void WebViewHost::onCopySelection()
{
    d_webView->copySelection();
}

void WebViewHost::onPaste()
{
    d_webView->paste();
}

void WebViewHost::onDeleteSelection()
{
    d_webView->deleteSelection();
}

void WebViewHost::onEnableFocusBefore(bool enabled)
{
    d_webView->enableFocusBefore(enabled);
}

void WebViewHost::onEnableFocusAfter(bool enabled)
{
    d_webView->enableFocusAfter(enabled);
}

void WebViewHost::onEnableNCHitTest(bool enabled)
{
    d_webView->enableNCHitTest(enabled);
}

void WebViewHost::onOnNCHitTestResult(int x, int y, int result)
{
    d_webView->onNCHitTestResult(x, y, result);
}

void WebViewHost::onNCDragMoveAck(const gfx::Point& movePoint)
{
    DCHECK(d_ncDragAckPending);
    d_ncDragAckPending = false;
    if (d_ncDragging) {
        POINT nowPoint;
        ::GetCursorPos(&nowPoint);
        if (gfx::Point(nowPoint) != movePoint) {
            d_ncDragAckPending = d_ncDragNeedsAck;
            Send(new BlpWebViewMsg_NCDragMove(d_routingId));
        }
    }
    else {
        d_ncDragAckPending = d_ncDragNeedsAck;
        Send(new BlpWebViewMsg_NCDragEnd(d_routingId, d_ncDragEndPoint));
    }
}

void WebViewHost::onNCDragEndAck()
{
    DCHECK(!d_ncDragging);
    DCHECK(d_ncDragAckPending);
    d_ncDragAckPending = false;
}

void WebViewHost::onPerformCustomContextMenuAction(int actionId)
{
    d_webView->performCustomContextMenuAction(actionId);
}

void WebViewHost::onEnableAltDragRubberbanding(bool enabled)
{
    d_webView->enableAltDragRubberbanding(enabled);
}

void WebViewHost::onEnableCustomTooltip(bool enabled)
{
    d_webView->enableCustomTooltip(enabled);
}

void WebViewHost::onSetZoomPercent(int value)
{
    d_webView->setZoomPercent(value);
}

void WebViewHost::onFind(const FindOnPageRequest& value)
{
    d_webView->handleFindRequest(value);
}

void WebViewHost::onReplaceMisspelledRange(const std::string& text)
{
    d_webView->replaceMisspelledRange(text);
}

void WebViewHost::onRootWindowPositionChanged()
{
    d_webView->rootWindowPositionChanged();
}

void WebViewHost::onRootWindowSettingsChanged()
{
    d_webView->rootWindowSettingsChanged();
}

// IPC::Sender override

bool WebViewHost::Send(IPC::Message* message)
{
    return d_processHost->Send(message);
}

// WebViewImplClient overrides

bool WebViewHost::shouldDisableBrowserSideResize()
{
    return d_isInProcess;
}

void WebViewHost::aboutToNativateRenderView(int rendererRoutingId)
{
    if (d_isInProcess) {
        Send(new BlpWebViewMsg_AboutToNavigateRenderView(d_routingId,
                                                         rendererRoutingId));
    }
}

void WebViewHost::didUpdatedBackingStore(const gfx::Size& size)
{
    if (d_implMoveAckPending && d_implRect.size() == size) {
        DCHECK(d_isInProcess);
        d_implMoveAckPending = false;
        d_webView->move(d_implRect.x(), d_implRect.y(),
                        d_implRect.width(), d_implRect.height());
        Send(new BlpWebViewMsg_MoveAck(d_routingId));
    }
}

void WebViewHost::findStateWithReqId(int reqId,
                                     int numberOfMatches,
                                     int activeMatchOrdinal,
                                     bool finalUpdate)
{
    Send(new BlpWebViewMsg_FindState(d_routingId,
                                     reqId,
                                     numberOfMatches,
                                     activeMatchOrdinal,
                                     finalUpdate));
}

// WebViewDelegate overrides

void WebViewHost::updateTargetURL(WebView* source, const StringRef& url)
{
    DCHECK(source == d_webView);
    std::string surl(url.data(), url.length());
    Send(new BlpWebViewMsg_UpdateTargetURL(d_routingId, surl));
}

void WebViewHost::updateNavigationState(WebView* source,
                                        const NavigationState& state)
{
    DCHECK(source == d_webView);
    Send(new BlpWebViewMsg_UpdateNavigationState(d_routingId,
                                                 state.canGoBack,
                                                 state.canGoForward,
                                                 state.isLoading));
}

void WebViewHost::didNavigateMainFramePostCommit(WebView* source,
                                                 const StringRef& url)
{
    DCHECK(source == d_webView);
    std::string surl(url.data(), url.length());
    Send(new BlpWebViewMsg_DidNavigateMainFramePostCommit(d_routingId, surl));
}

void WebViewHost::didFinishLoad(WebView* source, const StringRef& url)
{
    DCHECK(source == d_webView);
    std::string surl(url.data(), url.length());
    Send(new BlpWebViewMsg_DidFinishLoad(d_routingId, surl));
}

void WebViewHost::didFailLoad(WebView* source, const StringRef& url)
{
    DCHECK(source == d_webView);
    std::string surl(url.data(), url.length());
    Send(new BlpWebViewMsg_DidFailLoad(d_routingId, surl));
}

void WebViewHost::didCreateNewView(WebView* source,
                                   WebView* newView,
                                   const NewViewParams& params,
                                   WebViewDelegate** newViewDelegate)
{
    DCHECK(source == d_webView);
    int newHostRoutingId = d_processHost->getUniqueRoutingId();
    WebViewHost* newHost =
        new WebViewHost(d_processHost,
                        static_cast<WebViewImpl*>(newView),
                        newHostRoutingId,
                        d_isInProcess);
    *newViewDelegate = newHost;
    Send(new BlpWebViewMsg_DidCreateNewView(d_routingId,
                                            newHostRoutingId,
                                            params));
}

void WebViewHost::destroyView(WebView* source)
{
    DCHECK(source == d_webView);
    Send(new BlpWebViewMsg_DestroyView(d_routingId));
}

void WebViewHost::focusBefore(WebView* source)
{
    DCHECK(source == d_webView);
    Send(new BlpWebViewMsg_FocusBefore(d_routingId));
}

void WebViewHost::focusAfter(WebView* source)
{
    DCHECK(source == d_webView);
    Send(new BlpWebViewMsg_FocusAfter(d_routingId));
}

void WebViewHost::focused(WebView* source)
{
    DCHECK(source == d_webView);
    Send(new BlpWebViewMsg_Focused(d_routingId));
}

void WebViewHost::showContextMenu(WebView* source,
                                  const ContextMenuParams& params)
{
    DCHECK(source == d_webView);
    Send(new BlpWebViewMsg_ShowContextMenu(d_routingId, params));
}

void WebViewHost::handleExternalProtocol(WebView* source, const StringRef& url)
{
    DCHECK(source == d_webView);
    std::string surl(url.data(), url.length());
    Send(new BlpWebViewMsg_HandleExternalProtocol(d_routingId, surl));
}

void WebViewHost::moveView(WebView* source,
                           int x, int y, int width, int height)
{
    DCHECK(source == d_webView);
    gfx::Rect rc(x, y, width, height);
    Send(new BlpWebViewMsg_MoveView(d_routingId, rc));
}

void WebViewHost::requestNCHitTest(WebView* source)
{
    DCHECK(source == d_webView);
    Send(new BlpWebViewMsg_RequestNCHitTest(d_routingId));
}

void WebViewHost::ncDragBegin(WebView* source,
                              int hitTestCode,
                              const POINT& startPoint)
{
    DCHECK(source == d_webView);

    if (d_ncDragAckPending) {
        // This could happen if the user stopped dragging, then started
        // dragging again before we received an ack.  We'll just ignore the
        // second drag for now.
        // TODO: fix this
        return;
    }

    // If we are dragging the caption, then we want the drag to be as smooth as
    // possible, so we will sent continuous ncDragMove notifications.  However,
    // other drag types would cause resizes to happen, which would be slow.  In
    // those case, we don't want to send a continuous stream of ncDragMove
    // notifications, so we will need an ack from the WebViewProxy for each
    // move.
    d_ncDragNeedsAck = hitTestCode != HTCAPTION;
    d_ncDragging = true;
    Send(new BlpWebViewMsg_NCDragBegin(d_routingId,
                                       hitTestCode,
                                       gfx::Point(startPoint)));
}

void WebViewHost::ncDragMove(WebView* source, const POINT& movePoint)
{
    DCHECK(source == d_webView);

    if (d_ncDragAckPending)
        return;

    d_ncDragAckPending = d_ncDragNeedsAck;

    // Note that we are ignoring the 'movePoint'.  WebViewProxy will provide
    // its delegate with a more up-to-date mouse position.
    Send(new BlpWebViewMsg_NCDragMove(d_routingId));
}

void WebViewHost::ncDragEnd(WebView* source, const POINT& endPoint)
{
    DCHECK(source == d_webView);

    d_ncDragging = false;

    if (d_ncDragAckPending) {
        d_ncDragEndPoint = endPoint;
        return;
    }

    d_ncDragAckPending = d_ncDragNeedsAck;
    Send(new BlpWebViewMsg_NCDragEnd(d_routingId, gfx::Point(endPoint)));
}

void WebViewHost::showTooltip(WebView* source,
                              const StringRef& tooltipText,
                              TextDirection::Value direction)
{
    DCHECK(source == d_webView);
    std::string stext(tooltipText.data(), tooltipText.length());
    Send(new BlpWebViewMsg_ShowTooltip(d_routingId, stext, direction));
}

void WebViewHost::findState(WebView* source,
                            int numberOfMatches,
                            int activeMatchOrdinal,
                            bool finalUpdate)
{
    NOTREACHED() << "findState should come in via findStateWithReqId";
}

}  // close namespace blpwtk2

