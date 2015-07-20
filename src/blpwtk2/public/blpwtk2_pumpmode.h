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

#ifndef INCLUDED_BLPWTK2_PUMPMODE_H
#define INCLUDED_BLPWTK2_PUMPMODE_H

#include <blpwtk2_config.h>

namespace blpwtk2 {

// Enum to configure how the main message loop pump works.
struct PumpMode {

    enum Value {
        // In this mode, the application is responsible for calling
        // Toolkit::preHandleMessage and Toolkit::postHandleMessage, as
        // documented in 'blpwtk2_toolkit.h'.  This is the "preferred" mode,
        // since it more closely resembles the Chromium pump, where Chromium is
        // given an opportunity to do work at each message-loop iteration.
        // This is the default pump mode.
        MANUAL,

        // This is an optional mode provided for embedders that don't have the
        // ability to modify the application's main message loop (for example,
        // if blpwtk2 is being used inside a deeply nested library).  In this
        // mode, blpwtk2 will automatically give Chromium an opportunity to do
        // work using WM_TIMER events.  Note that WM_TIMER has a very low
        // priority in the Windows message queue, so Chromium work will also be
        // done in a lower priority.  However, this only applies to work done
        // on the application's main thread, so in practice, there isn't any
        // noticable lag.
        AUTOMATIC
    };
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PUMPMODE_H

