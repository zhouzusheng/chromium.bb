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

#ifndef INCLUDED_BLPWTK2_PROCESSHOSTIMPL_H
#define INCLUDED_BLPWTK2_PROCESSHOSTIMPL_H

#include <blpwtk2_config.h>

#include <blpwtk2_processhost.h>
#include <blpwtk2_rendererinfomap.h>

#include <base/compiler_specific.h>
#include <base/id_map.h>
#include <base/memory/scoped_ptr.h>
#include <base/process/process_handle.h>
#include <ipc/ipc_listener.h>

#include <string>

struct BlpWebViewHostMsg_NewParams;

namespace IPC {
class ChannelProxy;
}  // close namespace blpwtk2

namespace blpwtk2 {

class ManagedRenderProcessHost;

// This object lives in the browser process, and on the browser UI thread.  It
// sets up a channel to speak to a blpwtk2 ProcessClientImpl.
class ProcessHostImpl : public ProcessHost,
                        private IPC::Listener {
  public:
    ProcessHostImpl(RendererInfoMap* rendererInfoMap);
    ~ProcessHostImpl();

    const std::string& channelId() const;
    std::string channelInfo() const;

    // ProcessHost overrides
    void addRoute(int routingId, ProcessHostListener* listener) override;
    void removeRoute(int routingId) override;
    ProcessHostListener* findListener(int routingId) override;
    int getUniqueRoutingId() override;
    base::ProcessHandle processHandle() override;

    // IPC::Sender overrides
    bool Send(IPC::Message* message) override;

  private:
    // IPC::Listener overrides
    bool OnMessageReceived(const IPC::Message& message) override;
    void OnBadMessageReceived(const IPC::Message& message) override;
    void OnChannelConnected(int32 peer_pid) override;
    void OnChannelError() override;

    // Control message handlers
    void onSync(bool isFinalSync);
    void onCreateNewHostChannel(int timeoutInMilliseconds,
                                std::string* channelInfo);
    void onClearWebCache();
    void onProfileNew(int routingId,
                      const std::string& dataDir,
                      bool diskCacheEnabled,
                      bool cookiePersistenceEnabled);
    void onProfileDestroy(int routingId);
    void onWebViewNew(const BlpWebViewHostMsg_NewParams& params);
    void onWebViewDestroy(int routingId);

    base::ProcessHandle d_processHandle;
    scoped_ptr<ManagedRenderProcessHost> d_renderProcessHost;
    scoped_ptr<IPC::ChannelProxy> d_channel;
    RendererInfoMap* d_rendererInfoMap;
    RendererInfo d_inProcessRendererInfo;
    IDMap<ProcessHostListener> d_routes;
    int d_lastRoutingId;
    bool d_receivedFinalSync;

    DISALLOW_COPY_AND_ASSIGN(ProcessHostImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROCESSHOSTIMPL_H

