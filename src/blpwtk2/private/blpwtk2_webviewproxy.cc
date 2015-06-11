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
#include <content/browser/renderer_host/web_input_event_aura.h>
#include <content/public/browser/native_web_keyboard_event.h>
#include <content/public/renderer/render_view.h>
#include <third_party/WebKit/public/web/WebView.h>
#include <third_party/WebKit/public/web/WebFrame.h>
#include <third_party/WebKit/public/web/WebInputEvent.h>
#include <skia/ext/platform_canvas.h>
#include <third_party/skia/include/core/SkDocument.h>
#include <third_party/skia/include/core/SkStream.h>
#include <pdf/pdf.h>
#include <ui/events/event.h>
#include <ui/gfx/geometry/size.h>

namespace blpwtk2 {

WebViewProxy::WebViewProxy(ProcessClient* processClient,
                           int routingId,
                           ProfileProxy* profileProxy,
                           WebViewDelegate* delegate,
                           blpwtk2::NativeView parent,
                           int rendererAffinity,
                           bool initiallyVisible,
                           const WebViewProperties& properties)
: d_profileProxy(profileProxy)
, d_processClient(processClient)
, d_delegate(delegate)
, d_nativeWebView(0)
, d_nativeHiddenView(0)
, d_routingId(routingId)
, d_renderViewRoutingId(0)
, d_moveAckPending(false)
, d_isMainFrameAccessible(false)
, d_gotRenderViewInfo(false)
, d_ncDragNeedsAck(false)
{
    LOG(INFO) << "Creating WebViewProxy, routingId=" << routingId
              << ", initiallyVisible=" << initiallyVisible;

    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(profileProxy);
    DCHECK(d_processClient);

    profileProxy->incrementWebViewCount();
    d_processClient->addRoute(d_routingId, this);

    BlpWebViewHostMsg_NewParams params;
    params.routingId = routingId;
    params.profileId = profileProxy->routingId();
    params.initiallyVisible = initiallyVisible;
    params.properties = properties;
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
, d_renderViewRoutingId(0)
, d_moveAckPending(false)
, d_isMainFrameAccessible(false)
, d_gotRenderViewInfo(false)
, d_ncDragNeedsAck(false)
{
    LOG(INFO) << "Creating WebViewProxy from existing, routingId=" << routingId;

    profileProxy->incrementWebViewCount();
    d_processClient->addRoute(d_routingId, this);
}

WebViewProxy::~WebViewProxy()
{
}

void WebViewProxy::destroy()
{
    DCHECK(Statics::isInApplicationMainThread());

    LOG(INFO) << "Destroying WebViewProxy, routingId=" << d_routingId;

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
    DCHECK(d_gotRenderViewInfo);

    if (!d_mainFrame.get()) {
        content::RenderView* rv = content::RenderView::FromRoutingID(d_renderViewRoutingId);
        DCHECK(rv);

        blink::WebFrame* webFrame = rv->GetWebView()->mainFrame();
        d_mainFrame.reset(new WebFrameImpl(webFrame));
    }

    return d_mainFrame.get();
}

void WebViewProxy::loadUrl(const StringRef& url)
{
    DCHECK(Statics::isInApplicationMainThread());
    std::string surl(url.data(), url.length());
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId << ", loadUrl=" << surl;
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

static inline SkScalar distance(SkScalar x, SkScalar y)
{
    return sqrt(x*x + y*y);
}

String WebViewProxy::getLayoutTreeAsText(int flags) const
{
    DCHECK(Statics::isRendererMainThreadMode());
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(d_isMainFrameAccessible)
        << "You should wait for didFinishLoad";
    DCHECK(d_gotRenderViewInfo);

    content::RenderView* rv = content::RenderView::FromRoutingID(d_renderViewRoutingId);
    blink::WebFrame* webFrame = rv->GetWebView()->mainFrame();
    DCHECK(webFrame->isWebLocalFrame());

    return fromWebString(webFrame->layoutTreeAsText(flags));
}

void WebViewProxy::drawContents(const NativeRect &srcRegion,
                                const NativeRect &destRegion,
                                int dpiMultiplier,
                                const StringRef &styleClass,
                                NativeDeviceContext deviceContext)
{
    DCHECK(Statics::isRendererMainThreadMode());
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(d_isMainFrameAccessible)
        << "You should wait for didFinishLoad";
    DCHECK(d_gotRenderViewInfo);

    content::RenderView* rv = content::RenderView::FromRoutingID(d_renderViewRoutingId);
    blink::WebFrame* webFrame = rv->GetWebView()->mainFrame();
    DCHECK(webFrame->isWebLocalFrame());

    const int dpi =
        25.4 *                                                                                      // millimeters / inch
        distance(GetDeviceCaps(deviceContext, HORZRES), GetDeviceCaps(deviceContext, VERTRES)) /    // resolution
        distance(GetDeviceCaps(deviceContext, HORZSIZE), GetDeviceCaps(deviceContext, VERTSIZE));   // size in millimeters

    const int destWidth = destRegion.right - destRegion.left;
    const int destHeight = destRegion.bottom - destRegion.top;

    std::vector<char> pdf_data;
    size_t pdf_data_size;
    {
        SkDynamicMemoryWStream pdf_stream;
        {
            skia::RefPtr<SkDocument> document = skia::AdoptRef(SkDocument::CreatePDF(&pdf_stream, dpi * dpiMultiplier));
            SkCanvas *canvas = document->beginPage(destWidth, destHeight);
            const int srcWidth = srcRegion.right - srcRegion.left;
            const int srcHeight = srcRegion.bottom - srcRegion.top;

            canvas->scale(static_cast<SkScalar>(destWidth) / srcWidth, static_cast<SkScalar>(destHeight) / srcHeight);

            webFrame->drawInCanvas(blink::WebRect(srcRegion.left, srcRegion.top, srcWidth, srcHeight),
                                    blink::WebString::fromUTF8(styleClass.data(), styleClass.length()),
                                    canvas);
            canvas->flush();
            document->endPage();
        }
        pdf_data_size = pdf_stream.getOffset();
        pdf_data.reserve(pdf_data_size);
        pdf_data.push_back(0);
        pdf_stream.copyTo(&pdf_data[0]);
    }

    chrome_pdf::RenderPDFPageToDC(
        pdf_data.data(),
        pdf_data_size,
        0,
        deviceContext,
        dpi,
        destRegion.left,
        destRegion.top,
        destWidth,
        destHeight,
        false,
        false,
        false,
        false,
        false);
}

void WebViewProxy::handleInputEvents(const InputEvent *events, size_t eventsCount)
{
    DCHECK(Statics::isRendererMainThreadMode());
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(d_isMainFrameAccessible)
        << "You should wait for didFinishLoad";
    DCHECK(d_gotRenderViewInfo);

    content::RenderView* rv = content::RenderView::FromRoutingID(d_renderViewRoutingId);
    DCHECK(rv);

    for (size_t i=0; i < eventsCount; ++i) {
        const InputEvent *event = events + i;
        MSG msg = {
            event->hwnd,
            event->message,
            event->wparam,
            event->lparam,
            GetMessageTime()
        };

        switch (event->message) {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYUP:
        case WM_IME_CHAR:
        case WM_SYSCHAR:
        case WM_CHAR: {
            ui::KeyEvent uiKeyboardEvent(msg);
            content::NativeWebKeyboardEvent blinkKeyboardEvent(&uiKeyboardEvent);

            blinkKeyboardEvent.modifiers &= ~(
                    blink::WebInputEvent::ShiftKey |
                    blink::WebInputEvent::ControlKey |
                    blink::WebInputEvent::AltKey |
                    blink::WebInputEvent::MetaKey |
                    blink::WebInputEvent::IsAutoRepeat |
                    blink::WebInputEvent::IsKeyPad |
                    blink::WebInputEvent::IsLeft |
                    blink::WebInputEvent::IsRight |
                    blink::WebInputEvent::NumLockOn |
                    blink::WebInputEvent::CapsLockOn
                );

            if (event->shiftKey)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::ShiftKey;

            if (event->controlKey)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::ControlKey;

            if (event->altKey)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::AltKey;

            if (event->metaKey)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::MetaKey;

            if (event->isAutoRepeat)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::IsAutoRepeat;

            if (event->isKeyPad)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::IsKeyPad;

            if (event->isLeft)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::IsLeft;

            if (event->isRight)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::IsRight;

            if (event->numLockOn)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::NumLockOn;

            if (event->capsLockOn)
                blinkKeyboardEvent.modifiers |= blink::WebInputEvent::CapsLockOn;

            rv->GetWebView()->handleInputEvent(blinkKeyboardEvent);
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
        case WM_RBUTTONUP: {
            ui::MouseEvent uiMouseEvent(msg);
            blink::WebMouseEvent blinkMouseEvent = content::MakeWebMouseEvent(uiMouseEvent);
            rv->GetWebView()->handleInputEvent(blinkMouseEvent);

        } break;
        }
    }
}

void WebViewProxy::loadInspector(WebView* inspectedView)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(inspectedView);
    WebViewProxy* inspectedViewProxy
        = static_cast<WebViewProxy*>(inspectedView);
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId
              << ", loading inspector for " << inspectedViewProxy->routingId();
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
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId << ", reload";
    Send(new BlpWebViewHostMsg_Reload(d_routingId, ignoreCache));
}

void WebViewProxy::goBack()
{
    DCHECK(Statics::isInApplicationMainThread());
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId << ", goBack";
    Send(new BlpWebViewHostMsg_GoBack(d_routingId));
}

void WebViewProxy::goForward()
{
    DCHECK(Statics::isInApplicationMainThread());
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId << ", goForward";
    Send(new BlpWebViewHostMsg_GoForward(d_routingId));
}

void WebViewProxy::stop()
{
    DCHECK(Statics::isInApplicationMainThread());
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId << ", stop";
    Send(new BlpWebViewHostMsg_Stop(d_routingId));
}

void WebViewProxy::takeKeyboardFocus()
{
    DCHECK(Statics::isInApplicationMainThread());
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId << ", takeKeyboardFocus";
    if (d_nativeWebView) {
        ::SetFocus(d_nativeWebView);
    }
    else {
        Send(new BlpWebViewHostMsg_TakeKeyboardFocus(d_routingId));
    }
}

void WebViewProxy::setLogicalFocus(bool focused)
{
    DCHECK(Statics::isInApplicationMainThread());
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId
              << ", setLogicalFocus " << (focused ? "true" : "false");
    if (d_gotRenderViewInfo) {
        // If we have the renderer in-process, then set the logical focus
        // immediately so that handleInputEvents will work as expected.
        content::RenderView* rv = content::RenderView::FromRoutingID(d_renderViewRoutingId);
        DCHECK(rv);
        rv->SetFocus(focused);
    }

    // Send the message, which will update the browser-side aura::Window focus
    // state.
    Send(new BlpWebViewHostMsg_SetLogicalFocus(d_routingId, focused));
}

void WebViewProxy::show()
{
    DCHECK(Statics::isInApplicationMainThread());
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId << ", show";
    Send(new BlpWebViewHostMsg_Show(d_routingId));
}

void WebViewProxy::hide()
{
    DCHECK(Statics::isInApplicationMainThread());
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId << ", hide";
    Send(new BlpWebViewHostMsg_Hide(d_routingId));
}

void WebViewProxy::setParent(NativeView parent)
{
    DCHECK(Statics::isInApplicationMainThread());

    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId
              << ", setParent=" << (void*)parent;
    if (d_nativeWebView) {
        ::SetParent(d_nativeWebView, parent ? parent : d_nativeHiddenView);
    }
    else {
        Send(new BlpWebViewHostMsg_SetParent(d_routingId,
                                             (NativeViewForTransit)parent));
    }
}

void WebViewProxy::embedChild(NativeView child)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(d_nativeWebView);
    ::SetParent(child, d_nativeWebView);
}

void WebViewProxy::move(int left, int top, int width, int height)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(0 <= width);
    DCHECK(0 <= height);

    gfx::Rect rc(left, top, width, height);
    if (rc == d_lastMoveRect)
        return;

    d_lastMoveRect = rc;
    if (d_moveAckPending)
        return;

    moveImpl(rc);
}

void WebViewProxy::moveImpl(const gfx::Rect& rc)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_moveAckPending);

    if (d_gotRenderViewInfo && !rc.IsEmpty()) {
        // If we have renderer info (only happens if we are in-process), we can
        // start resizing the RenderView while we are in the main thread.  This
        // is to avoid a round-trip delay waiting for the resize to get to the
        // browser thread, and it sending a ViewMsg_Resize back to this thread.
        content::RenderView* rv = content::RenderView::FromRoutingID(d_renderViewRoutingId);
        DCHECK(rv);
        rv->SetSize(rc.size());
    }

    d_moveAckPending = true;
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

void WebViewProxy::fileChooserCompleted(const StringRef* paths,
                                        size_t numPaths)
{
    DCHECK(Statics::isInApplicationMainThread());
    std::vector<std::string> pathsVector(numPaths);
    for (size_t i = 0; i < numPaths; ++i) {
        pathsVector[i].assign(paths[i].data(), paths[i].length());
    }
    Send(new BlpWebViewHostMsg_FileChooserCompleted(d_routingId, pathsVector));
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

void WebViewProxy::setDelegate(WebViewDelegate *delegate)
{
    DCHECK(Statics::isInApplicationMainThread());
    d_delegate = delegate;
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
    IPC_BEGIN_MESSAGE_MAP(WebViewProxy, message)
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
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_RunFileChooser, onRunFileChooser)
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
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_UpdateNativeViews, onUpdateNativeViews)
        IPC_MESSAGE_HANDLER(BlpWebViewMsg_GotNewRenderViewRoutingId, onGotNewRenderViewRoutingId)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()

    return handled;
}

void WebViewProxy::OnBadMessageReceived(const IPC::Message& message)
{
    LOG(ERROR) << "bad message " << message.type();
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

void WebViewProxy::onRunFileChooser(const FileChooserParams& params)
{
    if (d_delegate)
        d_delegate->runFileChooser(this, params);
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

void WebViewProxy::onMoveAck(const gfx::Rect& lastRect)
{
    DCHECK(d_moveAckPending);
    d_moveAckPending = false;
    if (lastRect != d_lastMoveRect) {
        moveImpl(d_lastMoveRect);
    }
}

void WebViewProxy::onUpdateNativeViews(blpwtk2::NativeViewForTransit webview, blpwtk2::NativeViewForTransit hiddenView)
{
    DCHECK(webview);
    DCHECK(hiddenView);
    d_nativeWebView = (blpwtk2::NativeView)webview;
    d_nativeHiddenView = (blpwtk2::NativeView)hiddenView;
}

void WebViewProxy::onGotNewRenderViewRoutingId(int renderViewRoutingId)
{
    content::RenderView* rv =
        content::RenderView::FromRoutingID(renderViewRoutingId);
    if (!rv) {
        // The RenderView has not been created yet.  Keep reposting this task
        // until the RenderView is available.
        base::MessageLoop::current()->PostTask(
            FROM_HERE,
            base::Bind(&WebViewProxy::onGotNewRenderViewRoutingId,
                       AsWeakPtr(),
                       renderViewRoutingId));
        return;
    }

    d_gotRenderViewInfo = true;
    d_renderViewRoutingId = renderViewRoutingId;
    LOG(INFO) << "WebViewProxy, routingId=" << d_routingId
              << ", gotRenderViewInfo, renderViewRoutingId=" << renderViewRoutingId;
}

}  // close namespace blpwtk2

