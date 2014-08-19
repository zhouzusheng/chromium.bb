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

#ifndef INCLUDED_BLPWTK2_WEBVIEWHOST_H
#define INCLUDED_BLPWTK2_WEBVIEWHOST_H

#include <blpwtk2_config.h>

#include <blpwtk2_processhostlistener.h>
#include <blpwtk2_webviewdelegate.h>
#include <blpwtk2_webviewimplclient.h>

#include <base/compiler_specific.h>
#include <ipc/ipc_sender.h>
#include <ui/gfx/rect.h>

#include <string>

namespace gfx {
class Point;
}  // close namespace gfx

namespace blpwtk2 {

class BrowserContextImpl;
struct FindOnPageRequest;
class ProcessHost;
class WebViewImpl;

// This is the peer of the WebViewProxy.  This object lives in the browser-main
// thread.  It is created in response to a 'BlpWebViewHostMsg_New' message, and
// destroyed in response to a 'BlpWebViewHostMsg_Destroy' message.  It creates
// and hosts the WebViewImpl object for the WebViewProxy, and handles all the
// browser-side IPC between WebViewProxy and WebViewImpl.
class WebViewHost : public ProcessHostListener,
                    private IPC::Sender,
                    private WebViewImplClient,
                    private WebViewDelegate {
  public:
    WebViewHost(ProcessHost* processHost,
                BrowserContextImpl* browserContext,
                int routingId,
                bool isInProcess,
                NativeView parent,
                int hostAffinity,
                bool initiallyVisible,
                bool takeFocusOnMouseDown,
                bool domPasteEnabled,
                bool javascriptCanAccessClipboard);
    WebViewHost(ProcessHost* processHost,
                WebViewImpl* webView,
                int routingId,
                bool isInProcess);
    virtual ~WebViewHost();

    // IPC::Listener overrides
    virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  private:
    // Message handlers
    void onLoadUrl(const std::string& url);
    void onLoadInspector(int inspectedViewId);
    void onInspectElementAt(const gfx::Point& point);
    void onReload(bool ignoreCache);
    void onGoBack();
    void onGoForward();
    void onStop();
    void onFocus();
    void onShow();
    void onHide();
    void onSetParent(NativeViewForTransit parent);
    void onMove(const gfx::Rect& rect);
    void onSyncMove(const gfx::Rect& rect);
    void onCutSelection();
    void onCopySelection();
    void onPaste();
    void onDeleteSelection();
    void onEnableFocusBefore(bool enabled);
    void onEnableFocusAfter(bool enabled);
    void onEnableNCHitTest(bool enabled);
    void onOnNCHitTestResult(int x, int y, int result);
    void onNCDragMoveAck(const gfx::Point& movePoint);
    void onNCDragEndAck();
    void onPerformCustomContextMenuAction(int actionId);
    void onEnableAltDragRubberbanding(bool enabled);
    void onEnableCustomTooltip(bool enabled);
    void onSetZoomPercent(int value);
    void onFind(const FindOnPageRequest& value);
    void onReplaceMisspelledRange(const std::string& text);
    void onRootWindowPositionChanged();
    void onRootWindowSettingsChanged();
    void onPrint();

    // IPC::Sender override
    virtual bool Send(IPC::Message* message) OVERRIDE;

    // WebViewImplClient overrides
    virtual void updateNativeViews(blpwtk2::NativeView webview,
                                   blpwtk2::NativeView hiddenView) OVERRIDE;
    virtual bool shouldDisableBrowserSideResize() OVERRIDE;
    virtual void aboutToNativateRenderView(int routingId) OVERRIDE;
    virtual void didUpdatedBackingStore(const gfx::Size& size) OVERRIDE;
    virtual void findStateWithReqId(int reqId,
                                    int numberOfMatches,
                                    int activeMatchOrdinal,
                                    bool finalUpdate) OVERRIDE;

    // WebViewDelegate overrides
    virtual void updateTargetURL(WebView* source, const StringRef& url) OVERRIDE;
    virtual void updateNavigationState(WebView* source,
                                       const NavigationState& state) OVERRIDE;
    virtual void didNavigateMainFramePostCommit(WebView* source, const StringRef& url) OVERRIDE;
    virtual void didFinishLoad(WebView* source, const StringRef& url) OVERRIDE;
    virtual void didFailLoad(WebView* source, const StringRef& url) OVERRIDE;
    virtual void didCreateNewView(WebView* source,
                                  WebView* newView,
                                  const NewViewParams& params,
                                  WebViewDelegate** newViewDelegate) OVERRIDE;
    virtual void destroyView(WebView* source) OVERRIDE;
    virtual void focusBefore(WebView* source) OVERRIDE;
    virtual void focusAfter(WebView* source) OVERRIDE;
    virtual void focused(WebView* source) OVERRIDE;
    virtual void blurred(WebView* source) OVERRIDE;
    virtual void showContextMenu(WebView* source, const ContextMenuParams& params) OVERRIDE;
    virtual void handleExternalProtocol(WebView* source, const StringRef& url) OVERRIDE;
    virtual void moveView(WebView* source, int x, int y, int width, int height) OVERRIDE;
    virtual void requestNCHitTest(WebView* source) OVERRIDE;
    virtual void ncDragBegin(WebView* source,
                             int hitTestCode,
                             const POINT& startPoint) OVERRIDE;
    virtual void ncDragMove(WebView* source, const POINT& movePoint) OVERRIDE;
    virtual void ncDragEnd(WebView* source, const POINT& endPoint) OVERRIDE;
    virtual void showTooltip(WebView* source,
                             const StringRef& tooltipText,
                             TextDirection::Value direction) OVERRIDE;
    virtual void findState(WebView* source,
                           int numberOfMatches,
                           int activeMatchOrdinal,
                           bool finalUpdate) OVERRIDE;

    ProcessHost* d_processHost;
    WebViewImpl* d_webView;
    int d_routingId;
    gfx::Rect d_implRect;
    gfx::Point d_ncDragEndPoint;
    bool d_implMoveAckPending;
    bool d_ncDragAckPending;
    bool d_ncDragNeedsAck;
    bool d_ncDragging;
    bool d_isInProcess;

    DISALLOW_COPY_AND_ASSIGN(WebViewHost);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBVIEWHOST_H

