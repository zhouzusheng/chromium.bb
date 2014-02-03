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

#include <base/memory/ref_counted.h>
#include <base/memory/scoped_ptr.h>
#include <content/public/browser/browser_context.h>

namespace net {
class ProxyConfig;
}  // close namespace net

namespace user_prefs {
class PrefRegistrySyncable;
}

class PrefService;

namespace blpwtk2 {

class PrefStore;
class ResourceContextImpl;
class SpellCheckConfig;
class URLRequestContextGetterImpl;

// This is our implementation of the content::BrowserContext interface.  A
// browser context represents a user's "Profile" in chromium.  In blpwtk2, we
// create a BrowserContext for each blpwtk2::Profile.
//
// There is a 1-to-1 relationship between BrowserContextImpl and ProfileImpl.
// See blpwtk2_profileimpl.h for more details about this relationship.
//
// This class is responsible for providing net::URLRequestContextGetter
// objects.  URLRequestContextGetter are used to create a "request context" for
// each URL request.  The request context contains the necessary mechanisms to
// control caching/proxying etc.
class BrowserContextImpl : public content::BrowserContext {
  public:
    BrowserContextImpl(const std::string& dataDir,
                       bool diskCacheEnabled);
    virtual ~BrowserContextImpl();

    URLRequestContextGetterImpl* requestContextGetter() const;

    // Only called from the UI thread.
    void setProxyConfig(const net::ProxyConfig& config);
    void useSystemProxyConfig();
    void setSpellCheckConfig(const SpellCheckConfig& config);


    // ======== content::BrowserContext implementation =============

    virtual base::FilePath GetPath() OVERRIDE;
    virtual bool IsOffTheRecord() const OVERRIDE;
    virtual net::URLRequestContextGetter* GetRequestContext() OVERRIDE;
    virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
        int rendererChildId) OVERRIDE;
    virtual net::URLRequestContextGetter* GetMediaRequestContext() OVERRIDE;
    virtual net::URLRequestContextGetter*
    GetMediaRequestContextForRenderProcess(int rendererChildId) OVERRIDE;
    virtual net::URLRequestContextGetter*
    GetMediaRequestContextForStoragePartition(
        const base::FilePath& partitionPath, bool inMemory) OVERRIDE;
    virtual content::ResourceContext* GetResourceContext() OVERRIDE;
    virtual content::DownloadManagerDelegate*
    GetDownloadManagerDelegate() OVERRIDE;
    virtual content::GeolocationPermissionContext*
    GetGeolocationPermissionContext() OVERRIDE;
    virtual content::SpeechRecognitionPreferences*
    GetSpeechRecognitionPreferences() OVERRIDE;
    virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() OVERRIDE;
    virtual bool AllowDictionaryDownloads() OVERRIDE;

  private:
    scoped_ptr<ResourceContextImpl> d_resourceContext;
    scoped_refptr<URLRequestContextGetterImpl> d_requestContextGetter;
    scoped_refptr<user_prefs::PrefRegistrySyncable> d_prefRegistry;
    scoped_ptr<PrefService> d_prefService;
    scoped_refptr<PrefStore> d_userPrefs;
    bool d_isIncognito;

    DISALLOW_COPY_AND_ASSIGN(BrowserContextImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_BROWSERCONTEXTIMPL_H

