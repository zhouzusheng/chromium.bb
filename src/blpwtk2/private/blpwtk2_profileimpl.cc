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

#include <blpwtk2_profileimpl.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_proxyconfig.h>
#include <blpwtk2_proxyconfigimpl.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>

#include <base/bind.h>
#include <base/message_loop/message_loop.h>

namespace blpwtk2 {

ProfileImpl::ProfileImpl(const std::string& dataDir,
                         bool diskCacheEnabled,
                         base::MessageLoop* uiLoop)
: d_browserContext(0)
, d_dataDir(dataDir)
, d_uiLoop(uiLoop)
, d_useAppProxyConfig(false)
, d_diskCacheEnabled(diskCacheEnabled)
, d_isDestroyed(false)
{
    // If disk cache is enabled, then it must not be incognito.
    DCHECK(!d_diskCacheEnabled || !isIncognito());
    DCHECK(d_uiLoop);
}

ProfileImpl::~ProfileImpl()
{
    DCHECK(d_isDestroyed);
}

BrowserContextImpl* ProfileImpl::browserContext() const
{
    DCHECK(Statics::isInBrowserMainThread());
    return d_browserContext;
}

BrowserContextImpl* ProfileImpl::createBrowserContext()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_browserContext);

    d_browserContext = new BrowserContextImpl(d_dataDir, d_diskCacheEnabled);
    uiUpdateProxyConfig();
    uiUpdateSpellCheckConfig();

    return d_browserContext;
}

// Profile overrides

void ProfileImpl::destroy()
{
    DCHECK(!d_isDestroyed);
    d_isDestroyed = true;
}

void ProfileImpl::setProxyConfig(const ProxyConfig& config)
{
    DCHECK(!d_isDestroyed);

    base::AutoLock guard(d_lock);

    const net::ProxyConfig& impl = getProxyConfigImpl(config)->d_config;
    if (d_useAppProxyConfig && d_appProxyConfig.Equals(impl)) {
        return;
    }

    d_appProxyConfig = impl;
    d_useAppProxyConfig = true;

    d_uiLoop->PostTask(
        FROM_HERE,
        base::Bind(&ProfileImpl::uiUpdateProxyConfig,
                   base::Unretained(this)));
}

void ProfileImpl::useSystemProxyConfig()
{
    DCHECK(!d_isDestroyed);

    base::AutoLock guard(d_lock);

    if (!d_useAppProxyConfig) {
        return;
    }

    d_useAppProxyConfig = false;

    d_uiLoop->PostTask(
        FROM_HERE,
        base::Bind(&ProfileImpl::uiUpdateProxyConfig,
                   base::Unretained(this)));
}

void ProfileImpl::setSpellCheckConfig(const SpellCheckConfig& config)
{
    DCHECK(!d_isDestroyed);

    // Auto-correct cannot be enabled if spellcheck is disabled.
    DCHECK(!config.isAutoCorrectEnabled() || config.isSpellCheckEnabled());

    base::AutoLock guard(d_lock);

    d_spellCheckConfig = config;

    d_uiLoop->PostTask(
        FROM_HERE,
        base::Bind(&ProfileImpl::uiUpdateSpellCheckConfig,
                   base::Unretained(this)));
}

void ProfileImpl::uiUpdateProxyConfig()
{
    DCHECK(Statics::isInBrowserMainThread());
    if (!d_browserContext) {
        return;
    }

    base::AutoLock guard(d_lock);
    if (d_useAppProxyConfig) {
        d_browserContext->setProxyConfig(d_appProxyConfig);
    }
    else {
        d_browserContext->useSystemProxyConfig();
    }
}

void ProfileImpl::uiUpdateSpellCheckConfig()
{
    DCHECK(Statics::isInBrowserMainThread());
    if (!d_browserContext) {
        return;
    }

    base::AutoLock guard(d_lock);
    d_browserContext->setSpellCheckConfig(d_spellCheckConfig);
}

}  // close namespace blpwtk2

