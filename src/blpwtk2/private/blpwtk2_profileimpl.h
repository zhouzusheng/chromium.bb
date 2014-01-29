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

#ifndef INCLUDED_BLPWTK2_PROFILEIMPL_H
#define INCLUDED_BLPWTK2_PROFILEIMPL_H

#include <blpwtk2_config.h>

#include <blpwtk2_profile.h>
#include <blpwtk2_spellcheckconfig.h>

#include <base/basictypes.h>
#include <base/compiler_specific.h>
#include <base/synchronization/lock.h>
#include <net/proxy/proxy_config.h>

#include <string>

namespace base {
class MessageLoop;
}  // close namespace base

namespace blpwtk2 {

class BrowserContextImpl;
class ProxyConfig;

// Concrete implementation of the Profile interface.  There is a 1-to-1
// relationship between ProfileImpl and BrowserContextImpl.
//
// The main difference between the two objects is that ProfileImpl lives on the
// application's main thread, whereas BrowserContextImpl lives on the
// browser-main thread.  In upstream chromium, these are both the same thread,
// however in blpwtk2, these would be different threads whenever the
// RENDERER_MAIN thread-mode is used.  See blpwtk2_toolkit.h for an explanation
// about thread-modes.
//
// The ProfileImpl objects are stored in ProfileManager, which contains a
// mapping from data-directory to ProfileImpl, and also a list of incognito
// ProfileImpls.
//
// When a profile setting is modified, the update is posted to the browser-main
// thread so that the BrowserContextImpl settings can be updated.
//
// The ProfileImpl will always out-live the BrowserContextImpl.  blpwtk2's
// shutdown procedure destroys the BrowserMainRunner (which causes all the
// BrowserContextImpl objects to be deleted), then joins the browser-main
// thread, then deletes all the ProfileImpl objects.
class ProfileImpl : public Profile {
  public:
    ProfileImpl(const std::string& dataDir,
                bool diskCacheEnabled,
                base::MessageLoop* uiLoop);
    virtual ~ProfileImpl();

    bool isIncognito() const { return d_dataDir.empty(); }

    // Only called from the UI thread.
    BrowserContextImpl* browserContext() const;
    BrowserContextImpl* createBrowserContext();

    // Profile overrides, must only be called on the application-main thread.
    virtual void destroy() OVERRIDE;
    virtual void setProxyConfig(const ProxyConfig& config) OVERRIDE;
    virtual void useSystemProxyConfig() OVERRIDE;
    virtual void setSpellCheckConfig(const SpellCheckConfig& config) OVERRIDE;

  private:
    // methods that get invoked in the UI thread
    void uiUpdateProxyConfig();
    void uiUpdateSpellCheckConfig();

    BrowserContextImpl* d_browserContext;  // only touched in the UI thread

    base::Lock d_lock;
    std::string d_dataDir;
    base::MessageLoop* d_uiLoop;
    net::ProxyConfig d_appProxyConfig;
    SpellCheckConfig d_spellCheckConfig;
    bool d_useAppProxyConfig;
    bool d_diskCacheEnabled;
    bool d_isDestroyed;

    DISALLOW_COPY_AND_ASSIGN(ProfileImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROFILE_H

