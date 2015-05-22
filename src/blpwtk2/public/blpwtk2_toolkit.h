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

#ifndef INCLUDED_BLPWTK2_TOOLKIT_H
#define INCLUDED_BLPWTK2_TOOLKIT_H

// The original thread model used in upstream chromium is:
//   * browser-main-thread: used to control the browser UI.  This is
//                          responsible for creating browser tabs, windows,
//                          etc.  It is also responsible for creating the
//                          windows in which webkit renders content.  This is
//                          known as the BrowserMain thread.  However, it does
//                          not run webkit/v8.  This is the main application
//                          thread.
//
//   * browser-background-threads: used to control all kinds of things like IO,
//                                 sub-process management, db, caching, gpu
//                                 coordination, etc.
//                                 See browser_thread.h in the content module
//                                 for more details.
//
//   * render-thread: normally this thread runs in a separate process.
//                    However, if "--single-process" is specified on the
//                    command-line, this thread is run as a background thread
//                    in the same process as the browser.  However, the
//                    single-process mode is not officially supported by
//                    chromium and only exists for debugging purposes.  This
//                    thread runs webkit/v8.
//
// The render-thread makes sync calls to the browser threads.  However, browser
// threads must *not* make sync calls to the render-thread.  See
// https://sites.google.com/a/chromium.org/dev/developers/design-documents/inter-process-communication
//
// Since we want to use webkit/v8 in our application's main thread, we added a
// new thread model called 'RENDERER_MAIN', where the render-thread is on the
// application's main thread.
//
// In this mode, we create a second thread to run browser-main, which spawns
// all the other browser background threads.  When we create a blpwtk2::WebView
// (from the application's main thread, i.e. the render-thread), we post a
// message to the secondary browser-main thread, which sets up the window for
// the web content.  If it is an in-process WebView, then the browser thread
// will post back to the render thread to setup the blink::WebView, otherwise
// it will post to an external renderer process.
//
// Some caveats of this design:
//   * the window that hosts web contents must never be exposed to users of
//     blpwtk2, because it belongs to a different thread (and is created and
//     destroyed asynchronously).  Any window-ey operation we need on the
//     WebView (like show/hide/etc) should be a method on blpwtk2::WebView,
//     which posts to the browser-main thread.  See blpwtk2_webviewproxy where
//     this is done.
//
//   * our WebView<->WebViewDelegate interfaces must be completely asynchronous
//     (i.e. there can be no return values).  Anything that needs to be sent
//     back must be done via callbacks.
//
//   * the WebView::mainFrame() is not available immediately after creating the
//     WebView, because you have to wait for the post to the browser-main
//     thread, and post back to the render thread.  In general, you should wait
//     for the first didFinishLoad() callback on your WebViewDelegate, before
//     attempting to access WebView::mainFrame().

#include <blpwtk2_config.h>

#include <blpwtk2_webviewcreateparams.h>

namespace blpwtk2 {

class Profile;
class ProfileCreateParams;
class String;
class WebView;
class WebViewDelegate;

// This interface can be used to create profiles and WebViews.  An single
// instance of this class can be created using the 'ToolkitFactory'.
class Toolkit {
  public:
    // Create a Profile using the specified 'params'.  A browser will typically
    // create a profile for each user on the system.  The data (cookies, local
    // storage, cache, etc) for any WebViews created using this profile will be
    // stored within the 'dataDir' path in the 'params' object.  An incognito
    // profile will be returned if the 'dataDir' is empty.
    // For non-incognito profiles, the behavior is undefined if a Profile has
    // already been created in the 'dataDir'.  The only exception to this rule
    // is when you have multiple blpwtk2 client processes connected to the same
    // host process, and each client process creates a Profile with the same
    // 'dataDir' (in this case, the same underlying profile in the host process
    // will be used for all client processes).
    virtual Profile* createProfile(const ProfileCreateParams& params) = 0;

    // Return true if the blpwtk2_devtools pak file was detected and has been
    // loaded.  This method determines whether blpwtk2::WebView::loadInspector
    // can be used.  Note that applications should not cache this value because
    // the pak file is only loaded when a WebView has been created, which may
    // happen asynchronously if using the RENDERER_MAIN thread mode.
    virtual bool hasDevTools() = 0;

    // Destroy this Toolkit object.  This will shutdown all threads and block
    // until all threads have joined.  The behavior is undefined if this
    // Toolkit object is used after 'destroy' has been called.  The behavior is
    // also undefined if there are any WebViews or Profiles that haven't been
    // destroyed.
    virtual void destroy() = 0;

    // Create a WebView that will be hosted in the specified 'parent' window.
    // If a 'delegate' is provided, the delegate will receive callbacks from
    // this WebView.  The 'rendererAffinity' property in the specified 'params'
    // can be used to specify whether this WebView will run in-process or
    // out-of-process.
    virtual WebView* createWebView(
        NativeView parent,
        WebViewDelegate* delegate = 0,
        const WebViewCreateParams& params = WebViewCreateParams()) = 0;

    // Create a new host channel.  The returned string can be passed to
    // 'ToolkitCreateParams::setHostChannel' in another process.  This will
    // allow this process and the other process to share the same browser
    // process resources.  The 'timeoutInMilliseconds' parameter specifies the
    // amount of time that blpwtk2 will wait for a connection to be
    // established.  If the other process does not connect within this timeout,
    // then the channel will be destroyed and will no longer be usable.  Note
    // that only one process may connect to a single host channel.
    virtual String createHostChannel(int timeoutInMilliseconds) = 0;

    // Do extra chromium work needed at each message-loop iteration.  These
    // functions must be called on each message within the application's
    // message loop, like:
    ///<code>
    //     MSG msg;
    //     while(GetMessage(&msg, NULL, 0, 0) > 0) {
    //         if (!toolkit->preHandleMessage(&msg)) {
    //             TranslateMessage(&msg);
    //             DispatchMessage(&msg);
    //         }
    //         toolkit->postHandleMessage(&msg);
    //     }
    ///</code>
    // The behavior is undefined if these functions are not invoked as shown
    // in the code snippet above.  If 'preHandleMessage' returns true, this
    // means that blpwtk2 has consumed the message, and it should not be
    // dispatched through the normal Windows mechanism.  However, the
    // application must still call 'postHandleMessage'.  The behavior is also
    // undefined if these functions are called when 'PumpMode::AUTOMATIC' is
    // being used.
    virtual bool preHandleMessage(const NativeMsg* msg) = 0;
    virtual void postHandleMessage(const NativeMsg* msg) = 0;

    // Clears unused resources from the global web cache on all renderer
    // processes.
    virtual void clearWebCache() = 0;

    // By default, timers on hidden pages are aligned so that they fire once per
    // second at most.  This API changes that alignment interval.
    virtual void setTimerHiddenPageAlignmentInterval(double) = 0;

  protected:
    // Destroy this Toolkit object.  Note that clients of blpwtk2 should use
    // the 'destroy()' method, instead of deleting the object directly.
    virtual ~Toolkit();
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_TOOLKIT_H

