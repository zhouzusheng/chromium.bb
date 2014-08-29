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
#include <blpwtk2_newviewparams.h>
#include <blpwtk2_processclient.h>
#include <blpwtk2_profileproxy.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>
#include <blpwtk2_webframeimpl.h>
#include <blpwtk2_webviewdelegate.h>
#include <blpwtk2_webview_messages.h>

#include <base/bind.h>
#include <base/message_loop/message_loop.h>
#include <content/public/renderer/render_view.h>
#include <third_party/WebKit/public/web/WebView.h>
#include <third_party/WebKit/public/web/WebInputEvent.h>
#include <third_party/WebKit/public/web/win/WebInputEventFactory.h>
#include <ui/gfx/size.h>

namespace blpwtk2 {

WebViewProxy::WebViewProxy(ProcessClient* processClient,
                           int routingId,
                           ProfileProxy* profileProxy,
                           WebViewDelegate* delegate,
                           blpwtk2::NativeView parent,
                           int rendererAffinity,
                           bool initiallyVisible,
                           bool takeFocusOnMouseDown,
                           bool domPasteEnabled,
                           bool javascriptCanAccessClipboard)
: d_profileProxy(profileProxy)
, d_processClient(processClient)
, d_delegate(delegate)
, d_nativeWebView(0)
, d_nativeHiddenView(0)
, d_routingId(routingId)
, d_rendererRoutingId(0)
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
    params.domPasteEnabled = domPasteEnabled;
    params.javascriptCanAccessClipboard = javascriptCanAccessClipboard;
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
, d_nativeWebView(0)
, d_nativeHiddenView(0)
, d_routingId(routingId)
, d_rendererRoutingId(0)
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

    if (d_nativeWebView) {
        ::SetParent(d_nativeWebView, d_nativeHiddenView);
    }

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

void WebViewProxy::print()
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_Print(d_routingId));
}

void WebViewProxy::handleInputEvents(const InputEvent *events, size_t eventsCount)
{
    DCHECK(Statics::isRendererMainThreadMode());
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(d_isMainFrameAccessible)
        << "You should wait for didFinishLoad";
    DCHECK(d_gotRendererInfo);

    content::RenderView* rv = content::RenderView::FromRoutingID(d_rendererRoutingId);
    DCHECK(rv);

    for (size_t i=0; i < eventsCount; ++i) {
        const InputEvent *event = events + i;

        switch (event->message) {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYUP:
        case WM_IME_CHAR:
        case WM_SYSCHAR:
        case WM_CHAR: {
            WebKit::WebKeyboardEvent keyboardEvent = WebKit::WebInputEventFactory::keyboardEvent(
                event->hwnd,
                event->message,
                event->wparam,
                event->lparam);

            keyboardEvent.modifiers &= ~(
                    WebKit::WebInputEvent::ShiftKey |
                    WebKit::WebInputEvent::ControlKey |
                    WebKit::WebInputEvent::AltKey |
                    WebKit::WebInputEvent::MetaKey |
                    WebKit::WebInputEvent::IsAutoRepeat |
                    WebKit::WebInputEvent::IsKeyPad |
                    WebKit::WebInputEvent::IsLeft |
                    WebKit::WebInputEvent::IsRight |
                    WebKit::WebInputEvent::NumLockOn |
                    WebKit::WebInputEvent::CapsLockOn
                );

            if (event->shiftKey)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::ShiftKey;

            if (event->controlKey)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::ControlKey;

            if (event->altKey)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::AltKey;

            if (event->metaKey)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::MetaKey;

            if (event->isAutoRepeat)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::IsAutoRepeat;

            if (event->isKeyPad)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::IsKeyPad;

            if (event->isLeft)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::IsLeft;

            if (event->isRight)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::IsRight;

            if (event->numLockOn)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::NumLockOn;

            if (event->capsLockOn)
                keyboardEvent.modifiers |= WebKit::WebInputEvent::CapsLockOn;

            rv->GetWebView()->handleInputEvent(keyboardEvent);
        } break;

        case WM_MOUSEMOVE:
        case WM_MOUSELEAVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            rv->GetWebView()->handleInputEvent(WebKit::WebInputEventFactory::mouseEvent(
                    event->hwnd,
                    event->message,
                    event->wparam,
                    event->lparam));

            break;
        }
    }
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

    if (d_gotRendererInfo && !rc.IsEmpty()) {
        // If we have renderer info (only happens if we are in-process), we can
        // start resizing the RenderView while we are in the main thread.  This
        // is to avoid a round-trip delay waiting for the resize to get to the
        // browser thread, and it sending a ViewMsg_Resize back to this thread.
        content::RenderView* rv = content::RenderView::FromRoutingID(d_rendererRoutingId);
        DCHECK(rv);
        rv->SetSize(rc.size());
    }

    Send(new BlpWebViewHostMsg_Move(d_routingId, rc));
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

void WebViewProxy::enableAltDragRubberbanding(bool enabled)
{
    DCHECK(Statics::isInApplicationMainThread());
    Send(new BlpWebViewHostMsg_EnableAltDragRubberbanding(d_routingId, enabled));
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
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_Blurred, onBlurred)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_ShowContextMenu, onShowContextMenu)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_HandleExternalProtocol, onHandleExternalProtocol)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_MoveView, onMoveView)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_RequestNCHitTest, onRequestNCHitTest)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_NCDragBegin, onNCDragBegin)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_NCDragMove, onNCDragMove)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_NCDragEnd, onNCDragEnd)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_ShowTooltip, onShowTooltip)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_FindState, onFindState)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_UpdateNativeViews, onUpdateNativeViews)
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

void WebViewProxy::onBlurred()
{
    if (d_delegate)
        d_delegate->blurred(this);
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
        d_delegate->showTooltip(this, tooltipText, direction);
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

void WebViewProxy::onUpdateNativeViews(blpwtk2::NativeViewForTransit webview, blpwtk2::NativeViewForTransit hiddenView)
{
    DCHECK(webview);
    DCHECK(hiddenView);
    d_nativeWebView = (blpwtk2::NativeView)webview;
    d_nativeHiddenView = (blpwtk2::NativeView)hiddenView;
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
}

}  // close namespace blpwtk2

