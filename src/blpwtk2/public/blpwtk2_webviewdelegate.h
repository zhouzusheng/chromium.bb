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

#ifndef INCLUDED_BLPWTK2_WEBVIEWDELEGATE_H
#define INCLUDED_BLPWTK2_WEBVIEWDELEGATE_H

#include <blpwtk2_config.h>
#include <blpwtk2_textdirection.h>

namespace blpwtk2 {

class ContextMenuParams;
class FileChooserParams;
class NewViewParams;
class StringRef;
class WebView;
class MediaRequest;

// This class can be implemented by the application to receive notifications
// for various events pertaining to a particular WebView.  The delegate is
// provided at the time the WebView is created, and must remain alive until the
// WebView is destroyed.
//
// All methods on the delegate are invoked in the application's main thread.
class BLPWTK2_EXPORT WebViewDelegate {
  public:
    struct NavigationState {
        bool canGoBack;
        bool canGoForward;
        bool isLoading;
    };

    virtual ~WebViewDelegate();

    // Notification that the target URL has changed, for example when the user
    // moves the mouse over links.
    virtual void updateTargetURL(WebView* source, const StringRef& url) {}

    // Notification that the navigation state of the specified 'source' has
    // been updated.  This notification can be used to update the state of UI
    // controls for back/forward actions.
    virtual void updateNavigationState(WebView* source,
                                       const NavigationState& state) {}

    // Invoked when a main frame navigation occurs.
    virtual void didNavigateMainFramePostCommit(WebView* source,
                                                const StringRef& url) {}

    // Invoked when the main frame finished loading the specified 'url'.  This
    // is the notification that guarantees that the 'mainFrame()' method on the
    // WebView can be used (for in-process WebViews, and in the renderer
    // thread).
    virtual void didFinishLoad(WebView* source,
                               const StringRef& url) {}

    // Invoked when the main frame failed loading the specified 'url', or was
    // cancelled (e.g. window.stop() was called).
    virtual void didFailLoad(WebView* source,
                             const StringRef& url) {}

    // Invoked when the WebView creates a new WebView, for example by using
    // 'window.open'.  The default implementation of this method is to simply
    // destroy the specified 'newView'.  The delegate for the 'newView' will
    // initially be null, it can be set by modifying the specified
    // 'newViewDelegate'.
    virtual void didCreateNewView(WebView* source,
                                  WebView* newView,
                                  const NewViewParams& params,
                                  WebViewDelegate** newViewDelegate);

    // Invoked when the WebView needs to be destroyed, for example by using
    // 'window.close'.  The default implementation of this method is to simply
    // destroy the specified 'source'.
    virtual void destroyView(WebView* source);

    // Invoked when the WebView is done tabbing backwards through controls in
    // the page.  This is only invoked if 'enableFocusBefore(true)' was called
    // on the WebView.
    virtual void focusBefore(WebView* source) {}

    // Invoked when the WebView is done tabbing forwards through controls in
    // the page.  This is only invoked if 'enableFocusAfter(true)' was called
    // on the WebView.
    virtual void focusAfter(WebView* source) {}

    // Invoked when the WebView has gained focus.
    virtual void focused(WebView* source) {}

    // Invoked when the WebView has lost focus.
    virtual void blurred(WebView* source) {}

    // Invoked when the user wants a context menu, for example upon
    // right-clicking inside the WebView.  The specified 'params' contain
    // information about the operations that may be performed on the WebView
    // (for example, Copy/Paste etc).
    virtual void showContextMenu(WebView* source,
                                 const ContextMenuParams& params) {}

    // Invoked when the user needs to select files.  A modal file chooser
    // dialog should be displayed, allowing users to select the files.  Once
    // the files are selected, the 'fileChooserCompleted' method must be
    // invoked on the specified 'source' webview.  Note that this completion
    // method must be invoked even if the user cancels the action (just pass
    // an empty file list, in this case).  There will only ever be one
    // outstanding file chooser requests at any time.
    virtual void runFileChooser(WebView* source,
                                const FileChooserParams& params) {}

    // Invoked when an attempt is made to navigate using a protocol
    // that cannot be used withing the browser, such as 'mailto:'.
    virtual void handleExternalProtocol(WebView* source,
                                        const StringRef& url) {}

    // Invoked when a request has been made to move the webview.
    virtual void moveView(WebView* source, int x, int y, int width, int height) {}

    // Invoked when the WebView requests a non-client hit test.  This is called
    // only if non-client hit testing has been enabled via 'enableNCHitTest' on
    // the WebView.  The delegate is expected to invoke 'onNCHitTestResult' on
    // the WebView with the result of the hit test.  Note that there will only
    // be one outstanding hit test per WebView.  The hit test should be
    // performed using the current mouse coordinates.
    virtual void requestNCHitTest(WebView* source) {}

    // Invoked when the user starts dragging inside a non-client region in the
    // WebView.  This is called only if non-client hit testing has been enabled
    // via 'enableNCHitTest' on the WebView.  The specified 'startPoint'
    // contains the mouse position where the drag began, in screen coordinates.
    virtual void ncDragBegin(WebView* source,
                             int hitTestCode,
                             const POINT& startPoint) {}

    // Invoked when the user moves the mouse while dragging inside a non-client
    // region in the WebView.  The specified 'movePoint' contains the current
    // mouse position in screen coordinates.
    virtual void ncDragMove(WebView* source, const POINT& movePoint) {}

    // Invoked when the user releases the mouse after dragging inside a
    // non-client region in the WebView.  The specified 'endPoint' contains the
    // mouse position where the drag ended, in screen coordinates.
    virtual void ncDragEnd(WebView* source, const POINT& endPoint) {}

    // Show custom tooltip.
    virtual void showTooltip(WebView* source,
                             const StringRef& tooltipText,
                             TextDirection::Value direction) {}

    // Invoked response to a WebView::find method call to report find-on-page
    // status update.
    virtual void findState(WebView* source,
                           int numberOfMatches,
                           int activeMatchOrdinal,
                           bool finalUpdate) {}
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBVIEWDELEGATE_H

