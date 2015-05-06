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
#include <blpwtk2_proxyconfig.h>
#include <blpwtk2_proxyconfigimpl.h>
#include <blpwtk2_resourcecontextimpl.h>
#include <blpwtk2_spellcheckconfig.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>
#include <blpwtk2_urlrequestcontextgetterimpl.h>

#include <base/files/file_path.h>
#include <base/files/file_util.h>
#include <base/logging.h>  // for CHECK
#include <base/prefs/pref_service_factory.h>
#include <base/prefs/pref_service.h>
#include <base/threading/thread_restrictions.h>
#include <chrome/browser/spellchecker/spellcheck_factory.h>
#include <chrome/common/pref_names.h>
#include <chrome/common/spellcheck_common.h>
#include <content/public/browser/browser_thread.h>
#include <content/public/browser/render_process_host.h>
#include <content/public/browser/spellcheck_data.h>
#include <content/public/browser/storage_partition.h>
#include <components/keyed_service/content/browser_context_dependency_manager.h>
#include <components/pref_registry/pref_registry_syncable.h>
#include <components/user_prefs/user_prefs.h>

namespace blpwtk2 {

BrowserContextImpl::BrowserContextImpl(const std::string& dataDir,
                                       bool diskCacheEnabled,
                                       bool cookiePersistenceEnabled)
: d_numWebViews(0)
, d_isIncognito(dataDir.empty())
, d_isDestroyed(false)
{
    // If disk-cache/cookie-persistence is enabled, then it must not be
    // incognito.
    DCHECK(!diskCacheEnabled || !d_isIncognito);
    DCHECK(!cookiePersistenceEnabled || !d_isIncognito);

    if (Statics::isOriginalThreadMode()) {
        ++Statics::numProfiles;
    }

    base::FilePath path;

    if (!d_isIncognito) {
        // allow IO during creation of data directory
        base::ThreadRestrictions::ScopedAllowIO allowIO;

        path = base::FilePath::FromUTF8Unsafe(dataDir);
        if (!base::PathExists(path))
            base::CreateDirectory(path);
    }
    else {
        // It seems that even incognito browser contexts need to return a valid
        // data path.  Not providing one causes a DCHECK failure in
        // fileapi::DeviceMediaAsyncFileUtil::Create.
        // For now, just create a temporary directory for this.
        // TODO: see if we can remove this requirement.

        // allow IO during creation of temporary directory
        base::ThreadRestrictions::ScopedAllowIO allowIO;
        base::CreateNewTempDirectory(L"blpwtk2_", &path);
    }

    d_requestContextGetter =
        new URLRequestContextGetterImpl(path,
                                        diskCacheEnabled,
                                        cookiePersistenceEnabled);

    {
        // Initialize prefs for this context.
        d_prefRegistry = new user_prefs::PrefRegistrySyncable();
        d_userPrefs = new PrefStore();

        base::PrefServiceFactory factory;
        factory.set_user_prefs(d_userPrefs);
        d_prefService = factory.Create(d_prefRegistry.get());
        user_prefs::UserPrefs::Set(this, d_prefService.get());
        d_prefRegistry->RegisterBooleanPref(
            prefs::kPrintingEnabled,
            true,
            user_prefs::PrefRegistrySyncable::UNSYNCABLE_PREF);
    }

    content::SpellcheckData::CreateForContext(this);

    SpellcheckServiceFactory::GetInstance();  // This needs to be initialized before
                                              // calling CreateBrowserContextServices.

    BrowserContextDependencyManager::GetInstance()->CreateBrowserContextServices(this);
}

BrowserContextImpl::~BrowserContextImpl()
{
    DCHECK(d_isDestroyed);
}

URLRequestContextGetterImpl* BrowserContextImpl::requestContextGetter() const
{
    DCHECK(!d_isDestroyed);
    return d_requestContextGetter.get();
}

void BrowserContextImpl::incrementWebViewCount()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    DCHECK(!d_isDestroyed);
    ++d_numWebViews;
}

void BrowserContextImpl::decrementWebViewCount()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    DCHECK(0 < d_numWebViews);
    DCHECK(!d_isDestroyed);
    --d_numWebViews;
}

bool BrowserContextImpl::diskCacheEnabled() const
{
    DCHECK(!d_isDestroyed);
    return d_requestContextGetter->diskCacheEnabled();
}

bool BrowserContextImpl::cookiePersistenceEnabled() const
{
    DCHECK(!d_isDestroyed);
    return d_requestContextGetter->cookiePersistenceEnabled();
}

void BrowserContextImpl::reallyDestroy()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    DCHECK(0 == d_numWebViews);
    DCHECK(!d_isDestroyed);

    if (Statics::isOriginalThreadMode()) {
        DCHECK(0 < Statics::numProfiles);
        --Statics::numProfiles;
    }

    BrowserContextDependencyManager::GetInstance()->DestroyBrowserContextServices(this);

    d_userPrefs = 0;
    d_prefRegistry = 0;

    content::BrowserThread::DeleteSoon(content::BrowserThread::UI,
                                       FROM_HERE,
                                       d_prefService.release());

    if (d_resourceContext.get()) {
        content::BrowserThread::DeleteSoon(content::BrowserThread::IO,
                                           FROM_HERE,
                                           d_resourceContext.release());
    }

    if (d_isIncognito) {
        // Delete the temporary directory that we created in the constructor.

        // allow IO during deletion of temporary directory
        base::ThreadRestrictions::ScopedAllowIO allowIO;
        DCHECK(base::PathExists(d_requestContextGetter->path()));
        base::DeleteFile(d_requestContextGetter->path(), true);
    }

    d_requestContextGetter = 0;
    d_isDestroyed = true;
}

// Profile overrides

void BrowserContextImpl::destroy()
{
    // This is a no-op because BrowserContextImplManager needs to keep the
    // BrowserContextImpl objects alive until Toolkit is destroyed.
}

void BrowserContextImpl::setProxyConfig(const ProxyConfig& config)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    DCHECK(!d_isDestroyed);
    d_requestContextGetter->setProxyConfig(
        getProxyConfigImpl(config)->d_config);
}

void BrowserContextImpl::useSystemProxyConfig()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    DCHECK(!d_isDestroyed);
    d_requestContextGetter->useSystemProxyConfig();
}

void BrowserContextImpl::setSpellCheckConfig(const SpellCheckConfig& config)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
    DCHECK(!d_isDestroyed);

    // Auto-correct cannot be enabled if spellcheck is disabled.
    DCHECK(SpellCheckConfig::AUTOCORRECT_NONE == config.autocorrectBehavior()
        || config.isSpellCheckEnabled());

    PrefService* prefs = user_prefs::UserPrefs::Get(this);

    bool wasEnabled = prefs->GetBoolean(prefs::kEnableContinuousSpellcheck);

    // Convert from blpwtk2::SpellCheckConfig::AutocorrectFlags to
    // chrome::spellcheck_common::AutocorrectFlags.
    int autocorrectBehavior = chrome::spellcheck_common::AUTOCORRECT_NONE;
    if (config.autocorrectBehavior() & SpellCheckConfig::AUTOCORRECT_WORD_MAP)
        autocorrectBehavior |= chrome::spellcheck_common::AUTOCORRECT_WORD_MAP;
    if (config.autocorrectBehavior() & SpellCheckConfig::AUTOCORRECT_SWAP_ADJACENT_CHARS)
        autocorrectBehavior |= chrome::spellcheck_common::AUTOCORRECT_SWAP_ADJACENT_CHARS;

    std::string languages;
    for (size_t i = 0; i < config.numLanguages(); ++i) {
        StringRef str = config.languageAt(i);
        if (!languages.empty()) {
            languages.append(",");
        }
        languages.append(str.data(), str.length());
    }
    prefs->SetBoolean(prefs::kEnableContinuousSpellcheck,
                      config.isSpellCheckEnabled());
    prefs->SetInteger(prefs::kAutoSpellCorrectBehavior, autocorrectBehavior);
    prefs->SetString(prefs::kSpellCheckDictionary, languages);

    if (!wasEnabled && config.isSpellCheckEnabled()) {
        // Ensure the spellcheck service is created for this context if we just
        // turned it on.
        SpellcheckServiceFactory::GetForContext(this);
    }
}

void BrowserContextImpl::addCustomWords(const StringRef* words,
                                        size_t numWords)
{
    std::vector<base::StringPiece> wordsVector(numWords);
    for (size_t i = 0; i < numWords; ++i) {
        wordsVector[i].set(words[i].data(), words[i].length());
    }
    content::SpellcheckData::FromContext(this)->AdjustCustomWords(
        wordsVector,
        std::vector<base::StringPiece>());
}

void BrowserContextImpl::removeCustomWords(const StringRef* words,
                                           size_t numWords)
{
    std::vector<base::StringPiece> wordsVector(numWords);
    for (size_t i = 0; i < numWords; ++i) {
        wordsVector[i].set(words[i].data(), words[i].length());
    }
    content::SpellcheckData::FromContext(this)->AdjustCustomWords(
        std::vector<base::StringPiece>(),
        wordsVector);
}

void BrowserContextImpl::addAutocorrectWords(const StringRef* badWords,
                                             const StringRef* goodWords,
                                             size_t numWords)
{
    std::map<base::StringPiece, base::StringPiece> wordsMap;
    for (size_t i = 0; i < numWords; ++i) {
        base::StringPiece badWord(badWords[i].data(), badWords[i].length());
        base::StringPiece goodWord(goodWords[i].data(), goodWords[i].length());
        wordsMap[badWord] = goodWord;
    }
    content::SpellcheckData::FromContext(this)->AdjustAutocorrectWords(
        wordsMap,
        std::vector<base::StringPiece>());
}

void BrowserContextImpl::removeAutocorrectWords(const StringRef* badWords,
                                                size_t numWords)
{
    std::vector<base::StringPiece> badWordsVector(numWords);
    for (size_t i = 0; i < numWords; ++i) {
        badWordsVector[i].set(badWords[i].data(), badWords[i].length());
    }
    content::SpellcheckData::FromContext(this)->AdjustAutocorrectWords(
        std::map<base::StringPiece, base::StringPiece>(),
        badWordsVector);
}

// ======== content::BrowserContext implementation =============

scoped_ptr<content::ZoomLevelDelegate>
BrowserContextImpl::CreateZoomLevelDelegate(const base::FilePath& partition_path)
{
    return scoped_ptr<content::ZoomLevelDelegate>();
}

base::FilePath BrowserContextImpl::GetPath() const
{
    DCHECK(!d_isDestroyed);
    return d_requestContextGetter->path();
}

bool BrowserContextImpl::IsOffTheRecord() const
{
    DCHECK(!d_isDestroyed);
    return d_isIncognito;
}

net::URLRequestContextGetter* BrowserContextImpl::GetRequestContext()
{
    DCHECK(!d_isDestroyed);
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
    DCHECK(!d_isDestroyed);

    if (!d_resourceContext.get()) {
        d_resourceContext.reset(
            new ResourceContextImpl(d_requestContextGetter.get()));
    }
    return d_resourceContext.get();
}

content::DownloadManagerDelegate*
BrowserContextImpl::GetDownloadManagerDelegate()
{
    return 0;
}

content::BrowserPluginGuestManager*
BrowserContextImpl::GetGuestManager()
{
    return 0;
}

storage::SpecialStoragePolicy* BrowserContextImpl::GetSpecialStoragePolicy()
{
    return 0;
}

content::PushMessagingService* BrowserContextImpl::GetPushMessagingService()
{
    return 0;
}

content::SSLHostStateDelegate* BrowserContextImpl::GetSSLHostStateDelegate()
{
    return 0;
}

bool BrowserContextImpl::AllowDictionaryDownloads()
{
    return false;
}

}  // close namespace blpwtk2

