/*
 * Copyright (C) 2014 Bloomberg Finance L.P.
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

#ifndef INCLUDED_BLPWTK2_PROCESSHOSTLISTENER_H
#define INCLUDED_BLPWTK2_PROCESSHOSTLISTENER_H

#include <blpwtk2_config.h>

#include <ipc/ipc_listener.h>

namespace blpwtk2 {

// This interface is implemented by each "listener" object on the ProcessHost
// (i.e. WebViewHost and ProfileHost).  Right now, all it does is expose the
// protected destructor of IPC::Listener, but in the future we can add more
// stuff in here that is common among all such listeners.
class ProcessHostListener : public IPC::Listener {
  public:
    virtual ~ProcessHostListener();
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROCESSHOSTLISTENER_H
