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

#ifndef INCLUDED_BLPWTK2_PROFILEHOST_H
#define INCLUDED_BLPWTK2_PROFILEHOST_H

#include <blpwtk2_config.h>

#include <base/compiler_specific.h>
#include <ipc/ipc_listener.h>

#include <string>

namespace blpwtk2 {

class BrowserContextImpl;
class ProcessHost;
class ProxyConfig;
class SpellCheckConfig;

// This is the peer of the ProfileProxy.  This object lives in the browser-main
// thread.  It is created in response to a 'BlpProfileHostMsg_New' message, and
// destroyed in response to a 'BlpProfileHostMsg_Destroy' message.  It creates
// and hosts the BrowserContextImpl object for the ProfileProxy, and handles
// all the browser-side IPC between ProfileProxy and BrowserContextImpl.
class ProfileHost : public IPC::Listener {
  public:
    ProfileHost(ProcessHost* processHost,
                int routingId,
                const std::string& dataDir,
                bool diskCacheEnabled,
                bool cookiePersistenceEnabled);
    virtual ~ProfileHost();

    BrowserContextImpl* browserContext() const;

    // IPC::Listener overrides
    virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  private:
    // Message handlers
    void onSetProxyConfig(const ProxyConfig& config);
    void onUseSystemProxyConfig();
    void onSetSpellCheckConfig(const SpellCheckConfig& config);

    BrowserContextImpl* d_browserContext;
    ProcessHost* d_processHost;
    int d_routingId;

    DISALLOW_COPY_AND_ASSIGN(ProfileHost);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROFILEHOST_H

