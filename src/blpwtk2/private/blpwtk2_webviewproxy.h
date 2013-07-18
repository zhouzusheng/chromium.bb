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

#include <blpwtk2_webview.h>
#include <blpwtk2_webviewdelegate.h>
#include <blpwtk2_webviewimplclient.h>

#include <base/memory/ref_counted.h>
#include <ui/gfx/native_widget_types.h>

#include <string>

class MessageLoop;

namespace content {
    class BrowserContext;
}  // close namespace content

namespace blpwtk2 {

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
                 MessageLoop* implDispatcher,
                 content::BrowserContext* browserContext,
                 int hostAffinity,
                 bool initiallyVisible);
    WebViewProxy(WebViewImpl* impl,
                 MessageLoop* implDispatcher,
                 MessageLoop* proxyDispatcher);

    // ========== WebView overrides ================

    virtual void destroy() OVERRIDE;
    virtual WebFrame* mainFrame() OVERRIDE;
    virtual void loadUrl(const StringRef& url) OVERRIDE;
    virtual void loadInspector(WebView* inspectedView) OVERRIDE;
    virtual void reload(bool ignoreCache) OVERRIDE;
    virtual void goBack() OVERRIDE;
    virtual void goForward() OVERRIDE;
    virtual void stop() OVERRIDE;
    virtual void focus() OVERRIDE;
    virtual void show() OVERRIDE;
    virtual void hide() OVERRIDE;
    virtual void setParent(NativeView parent) OVERRIDE;
    virtual void move(int left, int top, int width, int height, bool repaint) OVERRIDE;
    virtual void cutSelection() OVERRIDE;
    virtual void copySelection() OVERRIDE;
    virtual void paste() OVERRIDE;
    virtual void deleteSelection() OVERRIDE;
    virtual void enableFocusBefore(bool enabled) OVERRIDE;
    virtual void enableFocusAfter(bool enabled) OVERRIDE;
    virtual void performCustomContextMenuAction(int actionId) OVERRIDE;

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

    // ========== WebViewImplClient overrides ================

    virtual void updateRendererInfo(bool isInProcess, int routingId) OVERRIDE;

  private:
    // only RefCountedThreadSafe should be able to delete this object
    friend class base::RefCountedThreadSafe<WebViewProxy>;
    ~WebViewProxy();

  private:
    // methods that get invoked in the impl thread
    void implInit(gfx::NativeView parent, content::BrowserContext* browserContext,
                  int hostAffinity, bool initiallyVisible);
    void implDestroy();
    void implLoadUrl(const std::string& url);
    void implLoadInspector(WebView* inspectedView);
    void implReload(bool ignoreCache);
    void implGoBack();
    void implGoForward();
    void implStop();
    void implFocus();
    void implShow();
    void implHide();
    void implSetParent(NativeView parent);
    void implMove(int left, int top, int width, int height, bool repaint);
    void implCutSelection();
    void implCopySelection();
    void implPaste();
    void implDeleteSelection();
    void implEnableFocusBefore(bool enabled);
    void implEnableFocusAfter(bool enabled);
    void implPerformCustomContextMenuAction(int actionId);

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

    void proxyMoveAck(int left, int top, int width, int height, bool repaint);

    void proxyUpdateRendererInfo(bool isInProcess, int routingId);

    WebViewImpl* d_impl;
    MessageLoop* d_implDispatcher;
    MessageLoop* d_proxyDispatcher;
    WebViewDelegate* d_delegate;
    int d_routingId;
    RECT d_lastMoveRect;
    bool d_lastMoveRepaint;
    bool d_isMoveAckPending;
    bool d_wasDestroyed;
    bool d_isMainFrameAccessible;
    bool d_isInProcess;
    bool d_gotRendererInfo;
};


}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBVIEWPROXY_H

