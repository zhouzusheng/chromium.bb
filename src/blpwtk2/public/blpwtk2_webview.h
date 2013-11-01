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

#ifndef INCLUDED_BLPWTK2_WEBVIEW_H
#define INCLUDED_BLPWTK2_WEBVIEW_H

#include <blpwtk2_config.h>

namespace blpwtk2 {

class StringRef;
class WebFrame;

// This class represents a single WebView.  Instances of this class can be
// created using blpwtk2::Toolkit::createWebView().
//
// All methods on this class can only be invoked from the application's main
// thread.
class WebView {
public:
    // Destroy the WebView and release any resources.  Do not use this WebView
    // after calling this method.
    virtual void destroy() = 0;

    // Return the main WebFrame for this WebView.  This can only be used if
    // this is an in-process WebView, and only from the renderer thread.  See
    // blpwtk2_toolkit.h for an explanation about the threads.
    //
    // The main frame will not be available immediately after creating a
    // WebView, or loading a URL.  The application should wait for the
    // 'didFinishLoad()' callback to be invoked on the WebViewDelegate before
    // trying to access the main frame.
    virtual WebFrame* mainFrame() = 0;

    // Load the specified 'url' into this WebView, replacing whatever contents
    // are currently in this WebView.
    virtual void loadUrl(const StringRef& url) = 0;

    // Load an inspector for the specified 'inspectedView' into this WebView,
    // replacing whatever contents are currently in this WebView.  This method
    // depends on the blpwtk2_devtools pak file being present in the same
    // directory as blpwtk2.dll.  This can be checked using the
    // blpwtk2::Toolkit::hasDevTools() method.  The behavior if this method is
    // undefined unless blpwtk2::Toolkit::hasDevTools() returns true.
    virtual void loadInspector(WebView* inspectedView) = 0;

    // The the element at the specified 'point'.  The behavior is undefined
    // unless 'loadInspector' has been called.
    virtual void inspectElementAt(const POINT& point) = 0;

    // Reload the contents of this WebView.  Bypass the memory cache if the
    // specified 'ignoreCache' is 'true'.  The behavior is undefined unless
    // 'loadUrl' or 'loadInspector' has been called.
    virtual void reload(bool ignoreCache = false) = 0;

    // Go back.  This is a no-op if there is nothing to go back to.
    virtual void goBack() = 0;

    // Go forward.  This is a no-op if there is nothing to go forward to.
    virtual void goForward() = 0;

    // Stop loading the current contents.  This is a no-op if the WebView is
    // not loading any content.
    virtual void stop() = 0;

    // Focus this WebView.  If any script has set focus to any editable
    // elements in this WebView, that element will get the input caret.
    virtual void focus() = 0;

    // Show this WebView.
    virtual void show() = 0;

    // Hide this WebView.
    virtual void hide() = 0;

    // Reparent this WebView into the specified 'parent'.
    virtual void setParent(NativeView parent) = 0;

    // Move this WebView to the specified 'left' and the specified 'top'
    // position, and resize it to have the specified 'width' and the specified
    // 'height'.  If the specified 'repaint' is 'true', the contents of the
    // WebView will be painted immediately.
    virtual void move(int left, int top, int width, int height,
                      bool repaint) = 0;

    // Remove the current selection, placing its contents into the system
    // clipboard.  This method has no effect if there is no selected editable
    // content.
    virtual void cutSelection() = 0;

    // Copy the contents of the current selection into the system clipboard.
    // This method has no effect if there is nothing selected.
    virtual void copySelection() = 0;

    // Paste the contents of the clipboard, replacing the current selection, if
    // any.  This method has no effect if the caret is not inside editable
    // content.
    virtual void paste() = 0;

    // Remove the current selection, without placing its contents into the
    // system clipboard.  This method has no effect if there is no selected
    // editable content.
    virtual void deleteSelection() = 0;

    // If set to 'true', the 'focusBefore()' and 'focusAfter()' methods will be
    // invoked on the delegate when the WebView is done tabbing through the
    // controls on the page.  If set to 'false', the WebView will loop around
    // and keep focus within itself.  The default value for both of these is
    // 'false'.
    virtual void enableFocusBefore(bool enabled) = 0;
    virtual void enableFocusAfter(bool enabled) = 0;

    // If set to 'true', the WebViewDelegate will be requested to provide
    // non-client hit testing.  Otherwise, WebViews assume they do not include
    // non-client regions.
    virtual void enableNCHitTest(bool enabled) = 0;

    // When the WebViewDelegate's requestNCHitTest method is called, this
    // method should be invoked to provide the hit test result.  This should
    // only be called in response to 'requestNCHitTest' on the delegate.
    virtual void onNCHitTestResult(int x, int y, int result) = 0;

    // Perform a custom context menu action. This should be called when a custom 
    // item in the conext menu has been selected.
    virtual void performCustomContextMenuAction(int actionId) = 0;

    // If set to 'true', the default tooltip will not be used and the 
    // corresponding WebViewDelegate will be expected to provide a custom 
    // tooltip. Otherwise, the default tooltip will be used.
    virtual void enableCustomTooltip(bool enabled) = 0;

    // Set zoom percent for the WebView. 100 is the "original size" or 100%,
    // 150 is 150% etc. up to default limits of 500% and 25% of original size,
    // respectively.
    virtual void setZoomPercent(int value) = 0;

    // Find text on the current page. To clear highlighted results send an
    // empty search string. Sending the same search string again implies
    // continuing the search forward or backward depending on the value of
    // the third argument.
    virtual void find(const StringRef& text, bool matchCase,
                      bool forward = true) = 0;

protected:
    // Destroy this WebView.  Note that clients of blpwtk2 should use the
    // 'destroy()' method, instead of deleting the object directly.
    virtual ~WebView();
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBVIEW_H

