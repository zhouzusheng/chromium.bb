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

#ifndef INCLUDED_MINIKIT_WEBVIEWCREATEPARAMS_H
#define INCLUDED_MINIKIT_WEBVIEWCREATEPARAMS_H

#include <minikit_config.h>

namespace minikit {

class Profile;

// This class contains parameters that are passed to minikit whenever the
// application wants to create a new WebView.
class MINIKIT_EXPORT WebViewCreateParams {
  public:
    WebViewCreateParams();

    // By default, WebViews are visible.  Set this to false to make it
    // invisible.  The WebView::show() method can be used to show it
    // afterwards.
    void setInitiallyVisible(bool visible);

    // By default, WebViews will take keyboard focus on mouse down (mimicking
    // upstream chromium behavior).  However, setting this flag to false will
    // disable that behavior.
    void setTakeKeyboardFocusOnMouseDown(bool enable);

    // By default, WebViews will take logical focus on mouse down (mimicking
    // upstream chromium behavior).  However, setting this flag to false will
    // disable that behavior.
    void setTakeLogicalFocusOnMouseDown(bool enable);

    // By default, the root owner of the WebView will activate on mouse down.
    // (mimicking upstream chromium behavior). However, setting this flag to
    // false will disable that behavior.
    void setActivateWindowOnMouseDown(bool enable);

    // By default, Javascript will not be able to paste into the DOM.  However,
    // setting this flag will enable that behavior.  Note that this will only
    // work if "setJavascriptCanAccessClipboard(true)" is also set.
    void setDOMPasteEnabled(bool enable);

    // By default, Javascript will not be able to access the clipboard.
    // However, setting this flag will enable that behavior.
    void setJavascriptCanAccessClipboard(bool enable);

    // By default, WebViews dynamically instantiate renderer processes each
    // time a new navigation request is made.  This is known as the
    // "process-per-site-instance" model.  For more information, please refer
    // to this document:
    // https://sites.google.com/a/chromium.org/dev/developers/design-documents/process-models
    // The command-line arguments described on that page will also work on
    // minikit-based applications.
    //
    // However, in order to provide better control to some applications, the
    // 'affinity' parameter allows each WebView to be constructed to have
    // affinity to a particular renderer process.
    //
    // If 'affinity' is 'minikit::Constants::ANY_OUT_OF_PROCESS_RENDERER'
    // (which is the default if 'setRendererAffinity' is not called), then the
    // upstream chromium logic will be used.
    //
    // If 'affinity' is 'minikit::Constants::IN_PROCESS_RENDERER', then the
    // WebView will be rendered in-process (either on the application's main
    // thread, or a background thread, depending on the ThreadMode selected
    // during startup - see minikit_toolkit.h for an explanation about thread
    // modes).
    //
    // If 'affinity' is any positive number, then the WebView will use that
    // particular (out-of-process) renderer process.
    //
    // Note that if 'affinity' is not 'ANY_OUT_OF_PROCESS_RENDERER', then the
    // application is responsible for ensuring that WebViews that need to talk
    // to each other via script are routed to the same renderer process.
    //
    // Also note that if '--single-process' is specified on the windows command
    // line, then all WebViews will be routed to 'IN_PROCESS_RENDERER',
    // regardless of the 'affinity' setting.  This is useful for debugging.
    void setRendererAffinity(int affinity);

    // Set the profile that will be used for the newly created WebView.  By
    // default, the profile is null.  This makes minikit use the default
    // profile, which is incognito and uses the system proxy configuration.
    // Note that right now, WebViews with affinity to a particular process
    // (i.e. if 'rendererAffinity()' is not 'ANY_OUT_OF_PROCESS_RENDERER') must
    // use the same profile.  In other words, it is undefined behavior to
    // create two WebViews with different profiles but with affinity to the
    // same renderer process.
    void setProfile(Profile* profile);

    // By default, WebViews render their content on a black background with an opaque
    // alpha channel. Setting this flag to true will cause the alpha channel to be
    // populated with values specified by the content's HTML and CSS.
    void setIsTransparent(bool isTransparent);

    // Setting this flag will cause the WebView's HWND to not receive input event
    // messages; application code can then use WebView::handleInputEvents() to
    // push such events into the WebView.
    void setInputEventsDisabled(bool inputEventsDisabled);

    bool initiallyVisible() const { return d_initiallyVisible; }
    bool takeKeyboardFocusOnMouseDown() const { return d_takeKeyboardFocusOnMouseDown; }
    bool takeLogicalFocusOnMouseDown() const { return d_takeLogicalFocusOnMouseDown; }
    bool activateWindowOnMouseDown() const { return d_activateWindowOnMouseDown; }
    bool domPasteEnabled() const { return d_domPasteEnabled; }
    bool javascriptCanAccessClipboard() const { return d_javascriptCanAccessClipboard; }
    int rendererAffinity() const { return d_rendererAffinity; }
    Profile* profile() const { return d_profile; }
    bool isTransparent() const { return d_isTransparent; }
    bool inputEventsDisabled() const { return d_inputEventsDisabled; }

  private:
    bool d_initiallyVisible;
    bool d_takeKeyboardFocusOnMouseDown;
    bool d_takeLogicalFocusOnMouseDown;
    bool d_activateWindowOnMouseDown;
    bool d_domPasteEnabled;
    bool d_javascriptCanAccessClipboard;
    int d_rendererAffinity;
    Profile* d_profile;
    bool d_isTransparent;
    bool d_inputEventsDisabled;
};


}  // close namespace minikit

#endif  // INCLUDED_MINIKIT_CREATEPARAMS_H

