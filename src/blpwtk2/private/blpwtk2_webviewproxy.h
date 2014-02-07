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

#ifndef INCLUDED_BLPWTK2_WEBVIEWPROXY_H
#define INCLUDED_BLPWTK2_WEBVIEWPROXY_H

#include <blpwtk2_config.h>

#include <blpwtk2_findonpage.h>
#include <blpwtk2_webview.h>
#include <blpwtk2_webviewdelegate.h>
#include <blpwtk2_webviewimplclient.h>

#include <base/memory/ref_counted.h>
#include <base/memory/scoped_ptr.h>
#include <ui/gfx/native_widget_types.h>
#include <ui/gfx/rect.h>

#include <string>

namespace base {
class MessageLoop;
}  // close namespace base

namespace blpwtk2 {

class ProfileProxy;
class WebFrameImpl;
class WebViewImpl;

// This is an alternate implementation of the blpwtk2::WebView interface, and
// is only used when we are using 'ThreadMode::RENDERER_MAIN'.  This class is
// responsible for forwarding the creation of 'WebViewImpl' to the secondary
// browser-main thread, and forwarding all the 'WebView' methods to the
// 'WebViewImpl' in the browser-main thread.  It also receives all the
// 'WebViewDelegate' callbacks from the 'WebViewImpl' (in the browser-main
// thread), and forwards them to the application-supplied 'WebViewDelegate' (in
// the application thread).
//
// See blpwtk2_toolkit.h for an explanation about the threads.
class WebViewProxy : public base::RefCountedThreadSafe<WebViewProxy>,
                        public WebView,
                        public WebViewDelegate,
                        public WebViewImplClient {
  public:
    WebViewProxy(WebViewDelegate* delegate,
                 gfx::NativeView parent,
                 base::MessageLoop* implDispatcher,
                 ProfileProxy* profileProxy,
                 int hostAffinity,
                 bool initiallyVisible,
                 bool takeFocusOnMouseDown,
                 bool isInProcess);
    WebViewProxy(WebViewImpl* impl,
                 base::MessageLoop* implDispatcher,
                 base::MessageLoop* proxyDispatcher,
                 ProfileProxy* profileProxy,
                 bool isInProcess);

    bool isMoveAckNotPending() const { return !d_moveAckPending; }

    // ========== WebView overrides ================

    virtual void destroy() OVERRIDE;
    virtual WebFrame* mainFrame() OVERRIDE;
    virtual void loadUrl(const StringRef& url) OVERRIDE;
    virtual void loadInspector(WebView* inspectedView) OVERRIDE;
    virtual void inspectElementAt(const POINT& point) OVERRIDE;
    virtual void reload(bool ignoreCache) OVERRIDE;
    virtual void goBack() OVERRIDE;
    virtual void goForward() OVERRIDE;
    virtual void stop() OVERRIDE;
    virtual void focus() OVERRIDE;
    virtual void show() OVERRIDE;
    virtual void hide() OVERRIDE;
    virtual void setParent(NativeView parent) OVERRIDE;
    virtual void move(int left, int top, int width, int height) OVERRIDE;
    virtual void cutSelection() OVERRIDE;
    virtual void copySelection() OVERRIDE;
    virtual void paste() OVERRIDE;
    virtual void deleteSelection() OVERRIDE;
    virtual void enableFocusBefore(bool enabled) OVERRIDE;
    virtual void enableFocusAfter(bool enabled) OVERRIDE;
    virtual void enableNCHitTest(bool enabled) OVERRIDE;
    virtual void onNCHitTestResult(int x, int y, int result) OVERRIDE;
    virtual void performCustomContextMenuAction(int actionId) OVERRIDE;
    virtual void enableCustomTooltip(bool enabled) OVERRIDE;
    virtual void setZoomPercent(int value) OVERRIDE;
    virtual void find(const StringRef& text, bool matchCase, bool forward) OVERRIDE;
    virtual void replaceMisspelledRange(const StringRef& text) OVERRIDE;

    // ========== WebViewDelegate overrides ================

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
    virtual void showContextMenu(WebView* source, const ContextMenuParams& params) OVERRIDE;
    virtual void handleMediaRequest(WebView* source, MediaRequest* request) OVERRIDE;
    virtual void handleExternalProtocol(WebView* source, const StringRef& url) OVERRIDE;
    virtual void moveView(WebView* source, int x, int y, int width, int height) OVERRIDE;
    virtual void requestNCHitTest(WebView* source) OVERRIDE;
    virtual void showTooltip(WebView* source, const String& tooltipText, TextDirection::Value direction) OVERRIDE;
    virtual void findState(WebView* source,
                           int numberOfMatches,
                           int activeMatchOrdinal,
                           bool finalUpdate) OVERRIDE;

    // ========== WebViewImplClient overrides ================

    virtual bool shouldDisableBrowserSideResize() OVERRIDE;
    virtual void aboutToNativateRenderView(int routingId) OVERRIDE;
    virtual void didUpdatedBackingStore(const gfx::Size& size) OVERRIDE;
    virtual void findStateWithReqId(int reqId,
                                    int numberOfMatches,
                                    int activeMatchOrdinal,
                                    bool finalUpdate) OVERRIDE;

  private:
    // only RefCountedThreadSafe should be able to delete this object
    friend class base::RefCountedThreadSafe<WebViewProxy>;
    ~WebViewProxy();

  private:
    // methods that get invoked in the impl thread
    void implInit(gfx::NativeView parent, ProfileProxy* profileProxy,
                  int hostAffinity, bool initiallyVisible,
                  bool takeFocusOnMouseDown);
    void implDestroy();
    void implLoadUrl(const std::string& url);
    void implFind(const FindOnPageRequest& request);
    void implLoadInspector(WebView* inspectedView);
    void implInspectElementAt(const POINT& point);
    void implReload(bool ignoreCache);
    void implGoBack();
    void implGoForward();
    void implStop();
    void implFocus();
    void implShow();
    void implHide();
    void implSetParent(NativeView parent);
    void implSyncMove(const gfx::Rect& rc);
    void implMove(int left, int top, int width, int height);
    void implCutSelection();
    void implCopySelection();
    void implPaste();
    void implDeleteSelection();
    void implEnableFocusBefore(bool enabled);
    void implEnableFocusAfter(bool enabled);
    void implEnableNCHitTest(bool enabled);
    void implOnNCHitTestResult(int x, int y, int result);
    void implPerformCustomContextMenuAction(int actionId);
    void implEnableCustomTooltip(bool enabled);
    void implSetZoomPercent(int value);
    void implReplaceMisspelledRange(const std::string& text);

    // methods that get invoked in the proxy (main) thread
    void proxyUpdateTargetURL(const std::string& url);
    void proxyUpdateNavigationState(const NavigationState& state);
    void proxyDidNavigateMainFramePostCommit(const std::string& url);
    void proxyDidFinishLoad(const std::string& url);
    void proxyDidFailLoad(const std::string& url);
    void proxyDidCreateNewView(WebViewProxy* newProxy,
                               const NewViewParams& params);
    void proxyDestroyView();
    void proxyFocusBefore();
    void proxyFocusAfter();
    void proxyFocused();
    void proxyShowContextMenu(const ContextMenuParams& params);
    void proxyHandleMediaRequest(MediaRequest* request);
    void proxyHandleExternalProtocol(const std::string& url);
    void proxyMoveView(int x, int y, int width, int height);
    void proxyRequestNCHitTest();
    void proxyShowTooltip(const String& tooltipText, TextDirection::Value direction); 
    void proxyFindState(int reqId,
                        int numberOfMatches,
                        int activeMatchOrdinal,
                        bool finalUpdate);

    void proxyMoveAck();
    void proxyAboutToNavigateRenderView(int routingId);

    ProfileProxy* d_profileProxy;
    WebViewImpl* d_impl;
    base::MessageLoop* d_implDispatcher;
    base::MessageLoop* d_proxyDispatcher;
    WebViewDelegate* d_delegate;
    scoped_ptr<FindOnPage> d_find;
    scoped_ptr<WebFrameImpl> d_mainFrame;
    int d_routingId;
    gfx::Rect d_rect;
    gfx::Rect d_implRect;       // touched only in the impl thread
    bool d_implMoveAckPending;  // touched only in the impl thread
    bool d_moveAckPending;
    bool d_wasDestroyed;
    bool d_isMainFrameAccessible;
    bool d_isInProcess;
    bool d_gotRendererInfo;
};


}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBVIEWPROXY_H

