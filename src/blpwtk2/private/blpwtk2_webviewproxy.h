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

#include <blpwtk2_textdirection.h>
#include <blpwtk2_webview.h>

#include <base/memory/scoped_ptr.h>
#include <base/memory/weak_ptr.h>
#include <ipc/ipc_listener.h>
#include <ipc/ipc_sender.h>
#include <ui/gfx/rect.h>

#include <string>

namespace base {
class MessageLoop;
}  // close namespace base

namespace blpwtk2 {

class ContextMenuParams;
class FileChooserParams;
class FindOnPage;
class NewViewParams;
class ProcessClient;
class ProfileProxy;
class WebFrameImpl;
class WebViewDelegate;
struct WebViewProperties;

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
class WebViewProxy : public WebView,
                        private IPC::Sender,
                        private IPC::Listener,
                        private base::SupportsWeakPtr<WebViewProxy> {
  public:
    WebViewProxy(ProcessClient* processClient,
                 int routingId,
                 ProfileProxy* profileProxy,
                 WebViewDelegate* delegate,
                 blpwtk2::NativeView parent,
                 int rendererAffinity,
                 bool initiallyVisible,
                 const WebViewProperties& properties);
    WebViewProxy(ProcessClient* processClient,
                 int routingId,
                 ProfileProxy* profileProxy);

    int routingId() const { return d_routingId; }
    void moveImpl(const gfx::Rect& rc);

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
    virtual void takeKeyboardFocus() OVERRIDE;
    virtual void setLogicalFocus(bool focused) OVERRIDE;
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
    virtual void fileChooserCompleted(const StringRef* paths,
                                      size_t numPaths) OVERRIDE;
    virtual void performCustomContextMenuAction(int actionId) OVERRIDE;
    virtual void enableAltDragRubberbanding(bool enabled) OVERRIDE;
    virtual void enableCustomTooltip(bool enabled) OVERRIDE;
    virtual void setZoomPercent(int value) OVERRIDE;
    virtual void find(const StringRef& text, bool matchCase, bool forward) OVERRIDE;
    virtual void replaceMisspelledRange(const StringRef& text) OVERRIDE;
    virtual void rootWindowPositionChanged() OVERRIDE;
    virtual void rootWindowSettingsChanged() OVERRIDE;
    virtual void print() OVERRIDE;
    virtual void handleInputEvents(const InputEvent *events, size_t eventsCount) OVERRIDE;
    virtual void setDelegate(WebViewDelegate* delegate) OVERRIDE;

  private:
    // Destructor is private.  Calling destroy() will delete the object.
    virtual ~WebViewProxy();

  private:
    // IPC::Sender override
    virtual bool Send(IPC::Message* message) OVERRIDE;

    // IPC::Listener overrides
    virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

    // Message handlers
    void onUpdateTargetURL(const std::string& url);
    void onUpdateNavigationState(bool canGoBack,
                                 bool canGoForward,
                                 bool isLoading);
    void onDidNavigateMainFramePostCommit(const std::string& url);
    void onDidFinishLoad(const std::string& url);
    void onDidFailLoad(const std::string& url);
    void onDidCreateNewView(int newRoutingId,
                            const NewViewParams& params);
    void onDestroyView();
    void onFocusBefore();
    void onFocusAfter();
    void onFocused();
    void onBlurred();
    void onRunFileChooser(const FileChooserParams& params);
    void onShowContextMenu(const ContextMenuParams& params);
    void onHandleExternalProtocol(const std::string& url);
    void onMoveView(const gfx::Rect& rect);
    void onRequestNCHitTest();
    void onNCDragBegin(int hitTestCode, const gfx::Point& startPoint);
    void onNCDragMove();
    void onNCDragEnd(const gfx::Point& endPoint);
    void onShowTooltip(const std::string& tooltipText, TextDirection::Value direction);
    void onFindState(int reqId,
                     int numberOfMatches,
                     int activeMatchOrdinal,
                     bool finalUpdate);
    void onMoveAck(const gfx::Rect& lastRect);
    void onUpdateNativeViews(blpwtk2::NativeViewForTransit webview, blpwtk2::NativeViewForTransit hiddenView);
    void onAboutToNavigateRenderView(int rendererRoutingId);


    ProfileProxy* d_profileProxy;
    ProcessClient* d_processClient;
    WebViewDelegate* d_delegate;
    scoped_ptr<FindOnPage> d_find;
    scoped_ptr<WebFrameImpl> d_mainFrame;
    blpwtk2::NativeView d_nativeWebView;
    blpwtk2::NativeView d_nativeHiddenView;
    int d_routingId;
    int d_rendererRoutingId;
    gfx::Rect d_lastMoveRect;
    bool d_moveAckPending;
    bool d_isMainFrameAccessible;
    bool d_gotRendererInfo;
    bool d_ncDragNeedsAck;
};


}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBVIEWPROXY_H

