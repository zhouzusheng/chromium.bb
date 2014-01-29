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

#include <blpwtk2_browsercontextimpl.h>

#include <blpwtk2_prefstore.h>
#include <blpwtk2_resourcecontextimpl.h>
#include <blpwtk2_spellcheckconfig.h>
#include <blpwtk2_stringref.h>

#include <base/files/file_path.h>
#include <base/file_util.h>
#include <base/logging.h>  // for CHECK
#include <base/prefs/pref_service_builder.h>
#include <base/prefs/pref_service.h>
#include <base/threading/thread_restrictions.h>
#include <chrome/browser/spellchecker/spellcheck_factory.h>
#include <chrome/common/pref_names.h>
#include <content/public/browser/browser_thread.h>
#include <content/public/browser/storage_partition.h>
#include <components/browser_context_keyed_service/browser_context_dependency_manager.h>
#include <components/user_prefs/pref_registry_syncable.h>
#include <components/user_prefs/user_prefs.h>
#include <net/proxy/proxy_service.h>
#include <net/proxy/proxy_config_service.h>
#include <net/proxy/proxy_config_service_fixed.h>
#include <net/url_request/url_request_context_getter.h>

namespace blpwtk2 {

BrowserContextImpl::BrowserContextImpl(const std::string& dataDir,
                                       bool diskCacheEnabled)
: d_proxyService(0)
, d_diskCacheEnabled(diskCacheEnabled)
, d_isIncognito(dataDir.empty())
{
    // If disk cache is enabled, then it must not be incognito.
    DCHECK(!d_diskCacheEnabled || !d_isIncognito);

    if (!d_isIncognito) {
        // allow IO during creation of data directory
        base::ThreadRestrictions::ScopedAllowIO allowIO;

        d_path = base::FilePath::FromUTF8Unsafe(dataDir);
        if (!file_util::PathExists(d_path))
            file_util::CreateDirectory(d_path);
    }
    else {
        // It seems that even incognito browser contexts need to return a valid
        // data path.  Not providing one causes a DCHECK failure in
        // fileapi::DeviceMediaAsyncFileUtil::Create.
        // For now, just create a temporary directory for this.
        // TODO: see if we can remove this requirement.

        // allow IO during creation of temporary directory
        base::ThreadRestrictions::ScopedAllowIO allowIO;
        file_util::CreateNewTempDirectory(L"blpwtk2_", &d_path);
    }

    {
        // Initialize prefs for this context.
        d_prefRegistry = new user_prefs::PrefRegistrySyncable();
        d_userPrefs = new PrefStore();

        d_prefRegistry->RegisterListPref(prefs::kSpellCheckCustomWords,
                                         new base::ListValue(),
                                         user_prefs::PrefRegistrySyncable::UNSYNCABLE_PREF);

        PrefServiceBuilder builder;
        builder.WithUserPrefs(d_userPrefs);
        d_prefService.reset(builder.Create(d_prefRegistry.get()));
        user_prefs::UserPrefs::Set(this, d_prefService.get());
    }

    SpellcheckServiceFactory::GetInstance();  // This needs to be initialized before
                                              // calling CreateBrowserContextServices.

    BrowserContextDependencyManager::GetInstance()->CreateBrowserContextServices(this, false);
}

BrowserContextImpl::~BrowserContextImpl()
{
    if (d_isIncognito) {
        // Delete the temporary directory that we created in the constructor.

        // allow IO during deletion of temporary directory
        base::ThreadRestrictions::ScopedAllowIO allowIO;
        DCHECK(file_util::PathExists(d_path));
        file_util::Delete(d_path, true);
    }

    BrowserContextDependencyManager::GetInstance()->DestroyBrowserContextServices(this);

    if (d_resourceContext.get()) {
        content::BrowserThread::DeleteSoon(content::BrowserThread::IO,
                                           FROM_HERE,
                                           d_resourceContext.release());
    }
}

void BrowserContextImpl::setRequestContextGetter(
    net::URLRequestContextGetter* getter)
{
    d_requestContextGetter = getter;

    if (d_resourceContext) {
        d_resourceContext->setRequestContextGetter(d_requestContextGetter);
    }
}

net::URLRequestContextGetter* BrowserContextImpl::requestContextGetter() const
{
    return d_requestContextGetter.get();
}

void BrowserContextImpl::setProxyConfig(const net::ProxyConfig& config)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    {
        base::AutoLock guard(d_proxyConfigServiceLock);
        d_proxyConfigService.reset(new net::ProxyConfigServiceFixed(config));
    }

    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&BrowserContextImpl::ioOnUpdateProxyConfig,
                   base::Unretained(this)));
}

void BrowserContextImpl::useSystemProxyConfig()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    {
        base::MessageLoop* ioLoop =
            content::BrowserThread::UnsafeGetMessageLoopForThread(
                content::BrowserThread::IO);

        base::MessageLoop* fileLoop =
            content::BrowserThread::UnsafeGetMessageLoopForThread(
                content::BrowserThread::FILE);

        base::AutoLock guard(d_proxyConfigServiceLock);
        // We must create the proxy config service on the UI loop on Linux
        // because it must synchronously run on the glib message loop.  This
        // will be passed to the ProxyServer on the IO thread.
        d_proxyConfigService.reset(
            net::ProxyService::CreateSystemProxyConfigService(
                ioLoop->message_loop_proxy(),
                fileLoop));
    }

    content::BrowserThread::PostTask(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&BrowserContextImpl::ioOnUpdateProxyConfig,
                   base::Unretained(this)));
}

void BrowserContextImpl::setSpellCheckConfig(const SpellCheckConfig& config)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    PrefService* prefs = user_prefs::UserPrefs::Get(this);

    bool wasEnabled = prefs->GetBoolean(prefs::kEnableContinuousSpellcheck);

    std::string languages;
    base::ListValue customWords;
    for (size_t i = 0; i < config.numLanguages(); ++i) {
        StringRef str = config.languageAt(i);
        if (!languages.empty()) {
            languages.append(",");
        }
        languages.append(str.data(), str.length());
    }
    for (size_t i = 0; i < config.numCustomWords(); ++i) {
        StringRef str = config.customWordAt(i);
        customWords.AppendString(std::string(str.data(), str.length()));
    }
    prefs->SetBoolean(prefs::kEnableContinuousSpellcheck,
                      config.isSpellCheckEnabled());
    prefs->SetBoolean(prefs::kEnableAutoSpellCorrect,
                      config.isAutoCorrectEnabled());
    prefs->SetString(prefs::kSpellCheckDictionary, languages);
    prefs->Set(prefs::kSpellCheckCustomWords, customWords);

    if (!wasEnabled && config.isSpellCheckEnabled()) {
        // Ensure the spellcheck service is created for this context if we just
        // turned it on.
        SpellcheckServiceFactory::GetForContext(this);
    }
}

void BrowserContextImpl::ioOnUpdateProxyConfig()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    scoped_ptr<net::ProxyConfigService> proxyConfigService;

    {
        base::AutoLock guard(d_proxyConfigServiceLock);
        DCHECK(d_proxyConfigService);
        proxyConfigService = d_proxyConfigService.Pass();
    }

    if (d_proxyService) {
        d_proxyService->ResetConfigService(proxyConfigService.release());
        return;
    }

    // TODO(jam): use v8 if possible, look at chrome code.
    d_proxyService = net::ProxyService::CreateUsingSystemProxyResolver(
        proxyConfigService.release(), 0, 0);
}

net::ProxyService* BrowserContextImpl::proxyService()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    DCHECK(d_proxyService);
    return d_proxyService;
}

bool BrowserContextImpl::diskCacheEnabled()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    return d_diskCacheEnabled;
}

// ======== content::BrowserContext implementation =============

base::FilePath BrowserContextImpl::GetPath()
{
    return d_path;
}

bool BrowserContextImpl::IsOffTheRecord() const
{
    return d_isIncognito;
}

net::URLRequestContextGetter* BrowserContextImpl::GetRequestContext()
{
    return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter*
BrowserContextImpl::GetRequestContextForRenderProcess(int rendererChildId)
{
    return GetRequestContext();
}

net::URLRequestContextGetter* BrowserContextImpl::GetMediaRequestContext()
{
    return GetRequestContext();
}

net::URLRequestContextGetter*
BrowserContextImpl::GetMediaRequestContextForRenderProcess(int rendererChildId)
{
    return GetRequestContext();
}

net::URLRequestContextGetter*
BrowserContextImpl::GetMediaRequestContextForStoragePartition(
    const base::FilePath& partitionPath,
    bool inMemory)
{
    return GetRequestContext();
}

content::ResourceContext* BrowserContextImpl::GetResourceContext()
{
	DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

	if (!d_resourceContext.get()) {
		d_resourceContext.reset(new ResourceContextImpl());
        d_resourceContext->setRequestContextGetter(d_requestContextGetter.get());
    }
    return d_resourceContext.get();
}

content::DownloadManagerDelegate*
BrowserContextImpl::GetDownloadManagerDelegate()
{
    return 0;
}

content::GeolocationPermissionContext*
BrowserContextImpl::GetGeolocationPermissionContext()
{
    return 0;
}

content::SpeechRecognitionPreferences*
BrowserContextImpl::GetSpeechRecognitionPreferences()
{
    return 0;
}

quota::SpecialStoragePolicy* BrowserContextImpl::GetSpecialStoragePolicy()
{
    return 0;
}

bool BrowserContextImpl::AllowDictionaryDownloads()
{
    return false;
}

}  // close namespace blpwtk2

