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

#include <blpwtk2_proxyconfig.h>
#include <blpwtk2_proxyconfigimpl.h>

#include <content/public/browser/browser_thread.h>
#include <net/proxy/proxy_service.h>
#include <net/proxy/proxy_config_service.h>
#include <net/proxy/proxy_config_service_fixed.h>

namespace blpwtk2 {

ProfileImpl::ProfileImpl(const char* dataDir)
: d_dataDir(dataDir ? dataDir : "")
, d_browserContext(0)
, d_uiLoop(0)
, d_ioLoop(0)
, d_fileLoop(0)
, d_proxyService(0)
, d_useAppProxyConfig(false)
, d_diskCacheEnabled(d_dataDir.length() != 0)
, d_canChangeDiskCacheEnablement(true)
{
    // dataDir must not be an empty string.  It must be null for
    // incognito profiles.
    DCHECK(!dataDir || d_dataDir.length() != 0);
}

ProfileImpl::~ProfileImpl()
{
}

void ProfileImpl::initFromBrowserUIThread(
    content::BrowserContext* browserContext,
    base::MessageLoop* ioLoop,
    base::MessageLoop* fileLoop)
{
    base::AutoLock guard(d_lock);

    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    DCHECK(!d_browserContext && !d_uiLoop && !d_ioLoop && !d_fileLoop);
    DCHECK(browserContext && base::MessageLoop::current() && ioLoop && fileLoop);

    d_browserContext = browserContext;
    d_uiLoop = base::MessageLoop::current();
    d_ioLoop = ioLoop;
    d_fileLoop = fileLoop;

    createProxyConfigService();
}

net::ProxyService* ProfileImpl::initFromBrowserIOThread()
{
    base::AutoLock guard(d_lock);
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    DCHECK(!d_proxyService);
    DCHECK(d_proxyConfigService.get());

    // TODO(jam): use v8 if possible, look at chrome code.
    d_proxyService = net::ProxyService::CreateUsingSystemProxyResolver(
        d_proxyConfigService.release(), 0, 0);
    return d_proxyService;
}

// Profile overrides

void ProfileImpl::setDiskCacheEnabled(bool enabled)
{
    // No need a lock, because these variables will only be modified in the
    // application-main thread.

    DCHECK(!enabled || d_dataDir.length());
    DCHECK(d_canChangeDiskCacheEnablement || enabled == d_diskCacheEnabled);
    d_diskCacheEnabled = enabled;
}

void ProfileImpl::setProxyConfig(const ProxyConfig& config)
{
    base::AutoLock guard(d_lock);

    const net::ProxyConfig& impl = getProxyConfigImpl(config)->d_config;
    if (d_useAppProxyConfig && d_appProxyConfig.Equals(impl)) {
        return;
    }

    d_appProxyConfig = impl;
    d_useAppProxyConfig = true;

    if (d_uiLoop) {
        d_uiLoop->PostTask(
            FROM_HERE,
            base::Bind(&ProfileImpl::uiOnUpdateProxyConfig,
                       base::Unretained(this)));
    }
}

void ProfileImpl::useSystemProxyConfig()
{
    base::AutoLock guard(d_lock);

    if (!d_useAppProxyConfig) {
        return;
    }

    d_useAppProxyConfig = false;

    if (d_uiLoop) {
        d_uiLoop->PostTask(
            FROM_HERE,
            base::Bind(&ProfileImpl::uiOnUpdateProxyConfig,
                       base::Unretained(this)));
    }
}

void ProfileImpl::createProxyConfigService()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    if (d_useAppProxyConfig) {
        d_proxyConfigService.reset(
            new net::ProxyConfigServiceFixed(d_appProxyConfig));
    }
    else {
        // We must create the proxy config service on the UI loop on Linux
        // because it must synchronously run on the glib message loop.  This
        // will be passed to the ProxyServer on the IO thread.
        d_proxyConfigService.reset(
            net::ProxyService::CreateSystemProxyConfigService(
                d_ioLoop->message_loop_proxy(), d_fileLoop));
    }
}

void ProfileImpl::uiOnUpdateProxyConfig()
{
    base::AutoLock guard(d_lock);
    createProxyConfigService();
    if (d_proxyService) {
        DCHECK(d_ioLoop);
        d_ioLoop->PostTask(
            FROM_HERE,
            base::Bind(&ProfileImpl::ioOnUpdateProxyConfig,
                       base::Unretained(this)));
    }
}

void ProfileImpl::ioOnUpdateProxyConfig()
{
    base::AutoLock guard(d_lock);
    DCHECK(d_proxyService);
    d_proxyService->ResetConfigService(d_proxyConfigService.release());
}

}  // close namespace blpwtk2

