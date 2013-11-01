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

#ifndef INCLUDED_BLPWTK2_NCDRAGUTIL_H
#define INCLUDED_BLPWTK2_NCDRAGUTIL_H

#include <blpwtk2_config.h>

namespace blpwtk2 {

// Utility for performing non-client drag operations across threads.  Windows
// doesn't handle HTTRANSPARENT regions that cross thread boundaries.  This is
// an issue for us when using RENDERER_MAIN thread mode, where the
// application's main HWND is on the application's thread, but the WebView's
// HWND is on a secondary thread (see blpwtk2_toolkit.h for more details about
// the threading model).
// This utility emulates the Windows non-client drag behavior, but does it
// across threads (specifically, from the browser's secondary thread to the
// application's main thread).
// The entry points for this utility must be invoked from the browser's thread.
// It will internally forward the calls to the application's main thread.
struct NCDragUtil {

    // The 'view' hwnd is the WebView's HWND.  The actual window that will be
    // moved will be the root window of 'view'.  The specified hitTestCode must
    // be one:
    //..
    // o HTCAPTION
    // o HTTOP
    // o HTBOTTOM
    // o HTLEFT
    // o HTRIGHT
    // o HTTOPLEFT
    // o HTTOPRIGHT
    // o HTBOTTOMLEFT
    // o HTBOTTOMRIGHT
    //..
    static void onDragBegin(HWND view, int hitTestCode, const POINT& pt);
    static void onDragMove();
    static void onDragEnd();
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_NCDRAGUTIL_H

