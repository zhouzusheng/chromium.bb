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

#ifndef INCLUDED_MINIKIT_THREADMODE_H
#define INCLUDED_MINIKIT_THREADMODE_H

#include <minikit_config.h>

namespace minikit {

// See minikit_toolkit.h for an explanation of the threading model.
struct ThreadMode {

    enum Value {
        // The original chromium threading model, which runs the browser-main
        // in the application's main thread, and creates a background thread if
        // any in-process WebViews are created.  This is the default thread
        // mode.
        ORIGINAL,

        // The new threading model for minikit, which runs the renderer in the
        // application's main thread, and runs the browser-main in a second
        // thread.
        RENDERER_MAIN
    };
};

}  // close namespace minikit

#endif  // INCLUDED_MINIKIT_THREADMODE_H

