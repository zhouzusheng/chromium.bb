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

namespace blpwtk2 {

class ContextMenuParams;
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
    virtual ~WebViewDelegate();

    // Notification that the target URL has changed, for example when the user
    // moves the mouse over links.
    virtual void updateTargetURL(WebView* source, const StringRef& url) {}

    // Invoked when a main frame navigation occurs.  This is the notification
    // that guarantees that the 'mainFrame()' method on the WebView can be used
    // (for in-process WebViews, and in the renderer thread).
    virtual void didNavigateMainFramePostCommit(WebView* source,
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

    // Invoked when the user wants a context menu, for example upon
    // right-clicking inside the WebView.  The specified 'params' contain
    // information about the operations that may be performed on the WebView
    // (for example, Copy/Paste etc).
    virtual void showContextMenu(WebView* source,
                                 const ContextMenuParams& params) {}

    // Invoked when a request has been made to access user's media devices
    // such as the camera or the microphone. The specified 'request' contains
    // information on the devices that are available.
    // Note that users should call addRef() on the specified 'request' in order
    // to obtain ownership on it, and call release() when they don't need it
    // any more.
    virtual void handleMediaRequest(WebView* source,
                                    MediaRequest* request) {}
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBVIEWDELEGATE_H

