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
// will post back to the render thread to setup the WebKit::WebView, otherwise
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
//     for the first didNavigateMainFramePostCommit() callback on your
//     WebViewDelegate, before attempting to access WebView::mainFrame().

#include <blpwtk2_config.h>

#include <blpwtk2_createparams.h>
#include <blpwtk2_threadmode.h>
#include <blpwtk2_pumpmode.h>
#include <blpwtk2_constants.h>

namespace blpwtk2 {

class HttpTransactionHandler;
class WebView;
class WebViewDelegate;

// This struct defines a bunch of static functions and enums for controlling
// the behavior of the blpwtk2 module.
struct BLPWTK2_EXPORT Toolkit {

    // Set the thread mode for the application.  This function can only be
    // called before creating any WebViews.  If this function is not called,
    // 'ThreadMode::ORIGINAL' will be used when creating the first WebView.
    // The behavior is undefined if attempting to change the thread mode after
    // a WebView has been created (even if it has been destroyed).
    static void setThreadMode(ThreadMode::Value mode);

    // Set the pump mode for the application.  This function can only be called
    // before creating any WebViews.  If this function is not called,
    // 'PumpMode::MANUAL' will be used when creating the first WebView, which
    // means that the application must call 'preHandleMessage' and
    // 'postHandleMessage' within the application's main message loop.  Set
    // this to 'PumpMode::AUTOMATIC' only if the embedding code doesn't have
    // access to the application's main message loop.  The behavior is
    // undefined if attempting to change the pump mode after a WebView has been
    // created (even if it has been destroyed).
    static void setPumpMode(PumpMode::Value mode);

    // Install a custom HttpTransactionHandler.  This function can only be
    // called before creating any WebViews, otherwise it will not have any
    // effect.  The http handler should remain alive until shutdown() is
    // called.
    static void setHttpTransactionHandler(HttpTransactionHandler* handler);

    // Shutdown all threads.  This will block until all threads have joined.
    // Note that this function is a no-op if no WebViews have been created
    // (since threads are only initialized on the creation of the first
    // WebView), or if shutdown() has already been called.  The behavior is
    // undefined if there are WebViews that haven't been destroyed.
    static void shutdown();

    // Create a WebView that will be hosted in the specified 'parent' window.
    // If a 'delegate' is provided, the delegate will receive callbacks from
    // this WebView.  The 'rendererAffinity' property in the specified 'params'
    // can be used to specify whether this WebView will run in-process or
    // out-of-process.  The behavior is undefined if Toolkit::shutdown() has
    // been called.
    static WebView* createWebView(NativeView parent,
                                  WebViewDelegate* delegate = 0,
                                  const CreateParams& params = CreateParams());

    // This function must be called by the application whenever any root
    // window containing a WebView receives WM_WINDOWPOSCHANGED.  Chromium
    // internally uses this notification to update its screen configuration
    // for the WebViews contained inside.  Note that this is only necessary
    // for 'ThreadMode::RENDERER_MAIN'.  This function is a no-op for other
    // thread modes (Chromium automatically hooks to the root window's WndProc
    // if thread mode is 'ThreadMode::ORIGINAL').
    static void onRootWindowPositionChanged(NativeView root);

    // This function must be called by the application whenever any root
    // window containing a WebView receives WM_SETTINGCHANGE.  Chromium
    // internally uses this notification to update its screen configuration
    // for the WebViews contained inside.  Note that this is only necessary
    // for 'ThreadMode::RENDERER_MAIN'.  This function is a no-op for other
    // thread modes (Chromium automatically hooks to the root window's WndProc
    // if thread mode is 'ThreadMode::ORIGINAL').
    static void onRootWindowSettingChange(NativeView root);

    // Do extra chromium work needed at each message-loop iteration.  These
    // functions must be called on each message within the application's
    // message loop, like:
    ///<code>
    //     MSG msg;
    //     while(GetMessage(&msg, NULL, 0, 0) > 0) {
    //         if (!blpwtk2::Toolkit::preHandleMessage(&msg)) {
    //             TranslateMessage(&msg);
    //             DispatchMessage(&msg);
    //         }
    //         blpwtk2::Toolkit::postHandleMessage(&msg);
    //     }
    ///</code>
    // The behavior is undefined if these functions are not invoked as shown
    // in the code snippet above.  If 'preHandleMessage' returns true, this
    // means that blpwtk2 has consumed the message, and it should not be
    // dispatched through the normal Windows mechanism.  However, the
    // application must still call 'postHandleMessage'.  The behavior is also
    // undefined if these functions are called when 'PumpMode::AUTOMATIC' has
    // been set using 'Toolkit::setPumpMode'.
    static bool preHandleMessage(const NativeMsg* msg);
    static void postHandleMessage(const NativeMsg* msg);

    // This is blpwtk2_subprocess.exe's entry point into the blpwtk2.dll -- do
    // not use this from other apps!!
    static int subProcessMain(HINSTANCE hInstance);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_TOOLKIT_H

