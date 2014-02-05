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

#ifndef INCLUDED_BLPWTK2_PROCESSCLIENTIMPL_H
#define INCLUDED_BLPWTK2_PROCESSCLIENTIMPL_H

#include <blpwtk2_config.h>

#include <blpwtk2_processclient.h>

#include <base/compiler_specific.h>
#include <base/id_map.h>
#include <base/memory/scoped_ptr.h>
#include <base/synchronization/waitable_event.h>
#include <ipc/ipc_listener.h>

#include <string>

namespace base {
class SingleThreadTaskRunner;
}  // close namespace base

namespace IPC {
class SyncChannel;
}  // close namespace IPC

namespace blpwtk2 {

class ProcessClientImpl : public ProcessClient,
                            private IPC::Listener {
  public:
    // Verifies the version embedded in the channelId
    static bool isValidVersion(const std::string& channelId);

    ProcessClientImpl(const std::string& channelId,
                      base::SingleThreadTaskRunner* ipcTaskRunner);
    ~ProcessClientImpl();

    // ProcessClient overrides
    virtual void addRoute(int routingId, IPC::Listener* listener) OVERRIDE;
    virtual void removeRoute(int routingId) OVERRIDE;
    virtual IPC::Listener* findListener(int routingId) OVERRIDE;

    // IPC::Sender overrides
    virtual bool Send(IPC::Message* message) OVERRIDE;

  private:
    // IPC::Listener overrides
    virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
    virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;
    virtual void OnChannelError() OVERRIDE;

    base::WaitableEvent d_shutdownEvent;
    scoped_ptr<IPC::SyncChannel> d_channel;
    IDMap<IPC::Listener> d_routes;

    DISALLOW_COPY_AND_ASSIGN(ProcessClientImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROCESSCLIENTIMPL_H

