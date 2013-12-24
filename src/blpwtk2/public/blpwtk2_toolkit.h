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
//     for the first didFinishLoad() callback on your WebViewDelegate, before
//     attempting to access WebView::mainFrame().

#include <blpwtk2_config.h>

#include <blpwtk2_createparams.h>
#include <blpwtk2_threadmode.h>
#include <blpwtk2_pumpmode.h>
#include <blpwtk2_constants.h>

namespace blpwtk2 {

class Profile;
class HttpTransactionHandler;
class StringRef;
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

    // Set the maximum number of sockets per proxy, up to a maximum of 99.
    // Note that each Profile maintains its own pool of connections, so this is
    // actually the maximum number of sockets per proxy *per profile*.  The
    // behavior is undefined if attempting to change the count after a WebView
    // has been created (even if it has been destroyed).  The behavior is also
    // undefined if 'count' is less than 1, or more than 99.
    static void setMaxSocketsPerProxy(int count);

    // Register a plugin at the specified 'pluginPath'.  The 'pluginPath'
    // should point to a DLL that exports the standard NPAPI entry points.
    // This function can only be called before creating any WebViews, otherwise
    // it will not have any effect.
    static void registerPlugin(const char* pluginPath);

    // Enable or disable loading default plugins (e.g. from paths in the
    // Windows registry).  This is enabled by default.  If it is disabled, then
    // only plugins registered via 'registerPlugin' will be enabled.  This
    // function can only be called before creating any WebViews.  The behavior
    // is undefined if attempting to change this setting after a WebView has
    // been created (even if it has been destroyed).
    static void enableDefaultPlugins(bool enabled);

    // Mark the specified 'renderer' to use in-process plugins.  Any WebView
    // created with affinity to the specified 'renderer' will use in-process
    // plugins.  The behavior is undefined if this function is called after any
    // such WebViews have been created.  Note that using in-process plugins
    // will disable the sandbox for that renderer.
    static void setRendererUsesInProcessPlugins(int renderer);

    // Get the profile that stores data in the specified 'dataDir'.  A browser
    // will typically create a profile for each user on the system.  The data
    // (cookies, local storage, cache, etc) for any WebViews created using this
    // profile will be stored within the specified 'dataDir' path.  The profile
    // will be created if it does not already exist.  Note that profile
    // settings (such as proxy configuration) are not persisted in the
    // 'dataDir', so applications are responsible for restoring those settings
    // (if desired) when they first get the profile.
    // The returned Profile object will remain valid until shutdown() is
    // called.  The behavior is undefined if shutdown() has already been
    // called.  The behavior is also undefined if any other process creates a
    // profile in the same 'dataDir'.
    static Profile* getProfile(const char* dataDir);

    // Create a new Profile that will prevent WebViews that are created using
    // it from storing any data (cookies, local storage, cache, etc) on disk.
    // The returned Profile object will remain valid until shutdown() is
    // called.  The behavior is undefined if shutdown() has already been
    // called.
    static Profile* createIncognitoProfile();

    // Return true if the blpwtk2_devtools pak file was detected and has been
    // loaded.  This method determines whether blpwtk2::WebView::loadInspector
    // can be used.  Note that the pak file is only loaded when a WebView has
    // been created, which may happen asynchronously if using the RENDERER_MAIN
    // thread mode.
    static bool hasDevTools();

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

    // Set the path to look for the .bdic files in.  This function can only be
    // called before creating any WebViews.  If this function is not called,
    // the current working directory will be used.  The behavior is undefined
    // if attempting to change the dictionary path after a WebView has been
    // created (even if it has been destroyed).
    static void setDictionaryPath(const StringRef& path);

    // Do not use this function unless you know what you're doing.  It relaxes
    // a bunch of security checks in the V8 binding layer in order to allow non
    // window contexts.  Note that it has been explicitly marked 'Unsafe' in
    // order to discourage its use except in cases where it is absolutely
    // necessary.
    static void allowNonWindowContexts_Unsafe();
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_TOOLKIT_H

