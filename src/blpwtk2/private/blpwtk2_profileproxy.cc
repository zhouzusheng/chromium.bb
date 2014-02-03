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

#include <blpwtk2_profileproxy.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_browsercontextimplmanager.h>
#include <blpwtk2_proxyconfig.h>
#include <blpwtk2_proxyconfigimpl.h>
#include <blpwtk2_spellcheckconfig.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>

#include <base/bind.h>
#include <base/message_loop/message_loop.h>

namespace blpwtk2 {

ProfileProxy::ProfileProxy(const std::string& dataDir,
                           bool diskCacheEnabled,
                           base::MessageLoop* uiLoop)
: d_browserContext(0)
, d_uiLoop(uiLoop)
, d_numWebViews(0)
{
    // If disk cache is enabled, then it must not be incognito.
    DCHECK(!diskCacheEnabled || dataDir.empty());
    DCHECK(d_uiLoop);

    ++Statics::numProfiles;

    AddRef();  // this is balanced in destroy()

    d_uiLoop->PostTask(
        FROM_HERE,
        base::Bind(&ProfileProxy::uiInit, this, dataDir, diskCacheEnabled));
}

ProfileProxy::~ProfileProxy()
{
    DCHECK(!d_browserContext);
}

BrowserContextImpl* ProfileProxy::browserContext() const
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(d_browserContext);
    return d_browserContext;
}

void ProfileProxy::incrementWebViewCount()
{
    DCHECK(Statics::isInApplicationMainThread());
    ++d_numWebViews;
}

void ProfileProxy::decrementWebViewCount()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(0 < d_numWebViews);
    --d_numWebViews;
}

// Profile overrides

void ProfileProxy::destroy()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(0 == d_numWebViews);

    DCHECK(0 < Statics::numProfiles);
    --Statics::numProfiles;

    d_uiLoop->PostTask(
        FROM_HERE,
        base::Bind(&ProfileProxy::uiDestroy, this));

    Release();  // balance the AddRef() from the constructor
}

void ProfileProxy::setProxyConfig(const ProxyConfig& config)
{
    DCHECK(Statics::isInApplicationMainThread());
    d_uiLoop->PostTask(
        FROM_HERE,
        base::Bind(&ProfileProxy::uiSetProxyConfig, this, config));
}

void ProfileProxy::useSystemProxyConfig()
{
    DCHECK(Statics::isInApplicationMainThread());
    d_uiLoop->PostTask(
        FROM_HERE,
        base::Bind(&ProfileProxy::uiUseSystemProxyConfig, this));
}

void ProfileProxy::setSpellCheckConfig(const SpellCheckConfig& config)
{
    DCHECK(Statics::isInApplicationMainThread());

    // Auto-correct cannot be enabled if spellcheck is disabled.
    DCHECK(!config.isAutoCorrectEnabled() || config.isSpellCheckEnabled());

    d_uiLoop->PostTask(
        FROM_HERE,
        base::Bind(&ProfileProxy::uiSetSpellCheckConfig, this, config));
}

void ProfileProxy::uiInit(const std::string& dataDir,
                          bool diskCacheEnabled)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_browserContext);
    DCHECK(Statics::browserContextImplManager);
    d_browserContext =
        Statics::browserContextImplManager->createBrowserContextImpl(
            dataDir,
            diskCacheEnabled);
}

void ProfileProxy::uiDestroy()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(d_browserContext);
    d_browserContext->destroy();
    d_browserContext = 0;
}

void ProfileProxy::uiSetProxyConfig(const ProxyConfig& config)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(d_browserContext);
    d_browserContext->setProxyConfig(config);
}

void ProfileProxy::uiUseSystemProxyConfig()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(d_browserContext);
    d_browserContext->useSystemProxyConfig();
}

void ProfileProxy::uiSetSpellCheckConfig(const SpellCheckConfig& config)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(d_browserContext);
    d_browserContext->setSpellCheckConfig(config);
}

}  // close namespace blpwtk2

