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

#ifndef INCLUDED_BLPWTK2_BROWSERCONTEXTIMPL_H
#define INCLUDED_BLPWTK2_BROWSERCONTEXTIMPL_H

#include <blpwtk2_config.h>

#include <blpwtk2_profile.h>

#include <base/memory/ref_counted.h>
#include <base/memory/scoped_ptr.h>
#include <content/public/browser/browser_context.h>

namespace user_prefs {
class PrefRegistrySyncable;
}

class PrefService;

namespace blpwtk2 {

class PrefStore;
class ResourceContextImpl;
class SpellCheckConfig;
class URLRequestContextGetterImpl;

// This is our implementation of the content::BrowserContext interface.  It is
// also the implementation of blpwtk2::Profile that lives on the browser-main
// thread.
//
// This class is responsible for providing net::URLRequestContextGetter
// objects.  URLRequestContextGetter are used to create a "request context" for
// each URL request.  The request context contains the necessary mechanisms to
// control caching/proxying etc.
//
// Do not create BrowserContextImpl objects directly.  Instead, create them
// using BrowserContextImplManager.
class BrowserContextImpl : public content::BrowserContext,
                            public Profile {
  public:
    BrowserContextImpl(const std::string& dataDir,
                       bool diskCacheEnabled,
                       bool cookiePersistenceEnabled);
    virtual ~BrowserContextImpl();

    // Only called from the browser-main thread.
    URLRequestContextGetterImpl* requestContextGetter() const;
    void incrementWebViewCount();
    void decrementWebViewCount();
    bool diskCacheEnabled() const;
    bool cookiePersistenceEnabled() const;
    void reallyDestroy();

    // Profile overrides, must only be called on the browser-main thread.
    void destroy() override;
    void setProxyConfig(const ProxyConfig& config) override;
    void useSystemProxyConfig() override;
    void setSpellCheckConfig(const SpellCheckConfig& config) override;
    void addCustomWords(const StringRef* words,
                        size_t numWords) override;
    void removeCustomWords(const StringRef* words,
                           size_t numWords) override;


    // ======== content::BrowserContext implementation =============

    scoped_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
        const base::FilePath& partition_path) override;
    base::FilePath GetPath() const override;
    bool IsOffTheRecord() const override;
    net::URLRequestContextGetter* GetRequestContext() override;
    net::URLRequestContextGetter* GetRequestContextForRenderProcess(
        int rendererChildId) override;
    net::URLRequestContextGetter* GetMediaRequestContext() override;
    net::URLRequestContextGetter*
    GetMediaRequestContextForRenderProcess(int rendererChildId) override;
    net::URLRequestContextGetter*
    GetMediaRequestContextForStoragePartition(
        const base::FilePath& partitionPath, bool inMemory) override;
    content::ResourceContext* GetResourceContext() override;
    content::DownloadManagerDelegate* GetDownloadManagerDelegate() override;
    content::BrowserPluginGuestManager* GetGuestManager() override;
    storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
    content::PushMessagingService* GetPushMessagingService() override;
    content::SSLHostStateDelegate* GetSSLHostStateDelegate() override;
    bool AllowDictionaryDownloads() override;
    content::PermissionManager* GetPermissionManager() override;
    content::BackgroundSyncController* GetBackgroundSyncController() override;

  private:
    scoped_ptr<ResourceContextImpl> d_resourceContext;
    scoped_refptr<URLRequestContextGetterImpl> d_requestContextGetter;
    scoped_refptr<user_prefs::PrefRegistrySyncable> d_prefRegistry;
    scoped_ptr<PrefService> d_prefService;
    scoped_refptr<PrefStore> d_userPrefs;
    int d_numWebViews;
    bool d_isIncognito;
    bool d_isDestroyed;

    DISALLOW_COPY_AND_ASSIGN(BrowserContextImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_BROWSERCONTEXTIMPL_H

