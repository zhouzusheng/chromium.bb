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

#include <base/basictypes.h>
#include <base/compiler_specific.h>
#include <base/memory/scoped_ptr.h>
#include <base/synchronization/lock.h>
#include <net/proxy/proxy_config.h>

#include <string>

namespace base {
class MessageLoop;
}  // close namespace base

namespace content {
class BrowserContext;
}  // close namespace content

namespace net {
class ProxyConfigService;
class ProxyService;
}  // close namespace net

namespace blpwtk2 {

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
// The ProfileImpl objects are stored in Statics, which contains a mapping from
// data-directory to ProfileImpl, and also a list of incognito ProfileImpls.
//
// Initialization of a ProfileImpl happens in a few stages.  First, it is
// constructed in the application's main thread.  When the browser-main thread
// creates a BrowserContext for this ProfileImpl, it will invoke the
// 'initFromBrowserUIThread' method.  At this point, it also provides the
// MessageLoop for the IO and FILE threads (these are needed by the system
// proxy config service).  Later, when the IO thread needs to obtain the
// ProxyService for this profile, it will invoke the 'initFromBrowserIOThread'
// method.
//
// Whenever a proxy setting change happens, it will need to bounce through
// these different threads before arriving at the ProxyService on the IO
// thread, where the setting is actually made effective.
//
// The ProfileImpl will always out-live the BrowserContextImpl.  blpwtk2's
// shutdown procedure destroys the BrowserMainRunner (which causes all the
// BrowserContextImpl objects to be deleted), then joins the browser-main
// thread, then deletes all the ProfileImpl objects.
class ProfileImpl : public Profile {
  public:
    explicit ProfileImpl(const char* dataDir);
    virtual ~ProfileImpl();

    void initFromBrowserUIThread(content::BrowserContext* browserContext,
                                 base::MessageLoop* ioLoop,
                                 base::MessageLoop* fileLoop);
    net::ProxyService* initFromBrowserIOThread();

    // Occurs on the application-main thread when a WebView using this profile
    // is created.
    void disableDiskCacheChanges() { d_canChangeDiskCacheEnablement = false; }

    // Occurs on the IO thread when setting up the cache.  No lock is required
    // here because setDiskCacheEnabled cannot be called after WebViews have
    // been created.
    bool diskCacheEnabled() const { return d_diskCacheEnabled; }

    const char* dataDir() const
    {
        return d_dataDir.length() != 0 ? d_dataDir.c_str() : 0;
    }
    content::BrowserContext* browserContext() const
    {
        return d_browserContext;
    }

    // Profile overrides, must only be called on the application-main thread.
    virtual void setDiskCacheEnabled(bool enabled) OVERRIDE;
    virtual void setProxyConfig(const ProxyConfig& config) OVERRIDE;
    virtual void useSystemProxyConfig() OVERRIDE;

  private:
    void createProxyConfigService();
    // called on the UI thread when a proxy config change happens
    void uiOnUpdateProxyConfig();
    // called on the IO thread when a proxy config change happens
    void ioOnUpdateProxyConfig();

    base::Lock d_lock;
    std::string d_dataDir;
    content::BrowserContext* d_browserContext;
    base::MessageLoop* d_uiLoop;
    base::MessageLoop* d_ioLoop;
    base::MessageLoop* d_fileLoop;
    net::ProxyService* d_proxyService;
    scoped_ptr<net::ProxyConfigService> d_proxyConfigService;
    net::ProxyConfig d_appProxyConfig;
    bool d_useAppProxyConfig;
    bool d_diskCacheEnabled;
    bool d_canChangeDiskCacheEnablement;

    DISALLOW_COPY_AND_ASSIGN(ProfileImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROFILE_H

