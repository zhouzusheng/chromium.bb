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

#ifndef INCLUDED_BLPWTK2_PROFILEPROXY_H
#define INCLUDED_BLPWTK2_PROFILEPROXY_H

#include <blpwtk2_config.h>

#include <blpwtk2_profile.h>

#include <base/memory/ref_counted.h>
#include <base/basictypes.h>
#include <base/compiler_specific.h>

#include <string>

namespace base {
class MessageLoop;
}  // close namespace base

namespace blpwtk2 {

class BrowserContextImpl;
class ProxyConfig;
class SpellCheckConfig;

// Concrete implementation of the Profile interface when using the
// 'RENDERER_MAIN' thread-mode.
//
// This class is responsible for forwarding the creation of
// 'BrowserContextImpl' to the secondary browser-main thread, and forwarding
// all the 'Profile' methods to the 'BrowserContextImpl' in the browser-main
// thread.
//
// See blpwtk2_toolkit.h for an explanation about the threads.
class ProfileProxy : public base::RefCountedThreadSafe<ProfileProxy>,
                        public Profile {
  public:
    ProfileProxy(const std::string& dataDir,
                 bool diskCacheEnabled,
                 base::MessageLoop* uiLoop);
    virtual ~ProfileProxy();

    // Only called from the application-main thread.
    void incrementWebViewCount();
    void decrementWebViewCount();

    // Only called from the browser-main thread.
    BrowserContextImpl* browserContext() const;

    // Profile overrides, must only be called on the application-main thread.
    virtual void destroy() OVERRIDE;
    virtual void setProxyConfig(const ProxyConfig& config) OVERRIDE;
    virtual void useSystemProxyConfig() OVERRIDE;
    virtual void setSpellCheckConfig(const SpellCheckConfig& config) OVERRIDE;

  private:
    // methods that get invoked in the UI thread
    void uiInit(const std::string& dataDir,
                bool diskCacheEnabled);
    void uiDestroy();
    void uiSetProxyConfig(const ProxyConfig& config);
    void uiUseSystemProxyConfig();
    void uiSetSpellCheckConfig(const SpellCheckConfig& config);

    BrowserContextImpl* d_browserContext;  // only touched in the UI thread
    base::MessageLoop* d_uiLoop;
    int d_numWebViews;

    DISALLOW_COPY_AND_ASSIGN(ProfileProxy);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROFILE_H

