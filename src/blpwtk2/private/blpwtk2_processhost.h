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

#ifndef INCLUDED_BLPWTK2_PROCESSHOST_H
#define INCLUDED_BLPWTK2_PROCESSHOST_H

#include <blpwtk2_config.h>

#include <ipc/ipc_sender.h>

namespace IPC {
class Listener;
}  // close namespace IPC

namespace blpwtk2 {

// This interface is used in the browser-main thread to send messages to a
// ProcessClient.  It uses IPC::Sender, so the ProcessClient can potentially be
// in another process on the same machine.
// A ProcessHost object is created for each ProcessClient that the browser-main
// thread services.  IPC::Listeners can subscribe to routed messages using the
// 'addRoute' method.
class ProcessHost : public IPC::Sender {
  public:
    // Add the specified 'listener' to be routed using the specified
    // 'routingId'.
    virtual void addRoute(int routingId, IPC::Listener* listener) = 0;

    // Remove the listener having the specified 'routingId'.
    virtual void removeRoute(int routingId) = 0;

    // Return the listener having the specified 'routingId', or 0 if there is
    // no such listener.
    virtual IPC::Listener* findListener(int routingId) = 0;

    // Get a new host-allocated routingId.  These routing ids start at 0x10000,
    // to make them different from routing ids allocated from the client (which
    // start at 0).
    virtual int getUniqueRoutingId() = 0;

  protected:
    virtual ~ProcessHost();
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROCESSHOST_H

