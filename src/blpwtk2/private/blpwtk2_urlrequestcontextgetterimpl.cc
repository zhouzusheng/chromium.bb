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

#include <blpwtk2_urlrequestcontextgetterimpl.h>

#include <blpwtk2_networkdelegateimpl.h>

#include <base/bind.h>
#include <base/command_line.h>
#include <base/logging.h>  // for DCHECK
#include <base/strings/string_util.h>
#include <base/threading/sequenced_worker_pool.h>
#include <base/threading/worker_pool.h>
#include <content/browser/net/sqlite_persistent_cookie_store.h>
#include <content/public/browser/browser_thread.h>
#include <content/public/browser/cookie_crypto_delegate.h>
#include <content/public/common/content_switches.h>
#include <content/public/common/url_constants.h>
#include <net/cert/cert_verifier.h>
#include <net/cookies/cookie_monster.h>
#include <net/dns/mapped_host_resolver.h>
#include <net/http/http_auth_handler_factory.h>
#include <net/http/http_cache.h>
#include <net/http/http_network_layer.h>
#include <net/http/http_network_session.h>
#include <net/http/http_server_properties_impl.h>
#include <net/proxy/proxy_service.h>
#include <net/proxy/proxy_config_service.h>
#include <net/proxy/proxy_config_service_fixed.h>
#include <net/ssl/channel_id_service.h>
#include <net/ssl/default_channel_id_store.h>
#include <net/ssl/ssl_config_service_defaults.h>
#include <net/url_request/data_protocol_handler.h>
#include <net/url_request/file_protocol_handler.h>
#include <net/url_request/static_http_user_agent_settings.h>
#include <net/url_request/url_request_context.h>
#include <net/url_request/url_request_context_storage.h>
#include <net/url_request/url_request_job_factory_impl.h>

namespace blpwtk2 {

namespace {

void installProtocolHandlers(net::URLRequestJobFactoryImpl* jobFactory,
                             content::ProtocolHandlerMap* protocolHandlers) {
    for (content::ProtocolHandlerMap::iterator it = protocolHandlers->begin();
         it != protocolHandlers->end(); ++it) {
        bool setProtocol = jobFactory->SetProtocolHandler(
            it->first, it->second.release());
        DCHECK(setProtocol);
    }
    protocolHandlers->clear();
}

}  // close unnamed namespace

URLRequestContextGetterImpl::URLRequestContextGetterImpl(
    const base::FilePath& path,
    bool diskCacheEnabled,
    bool cookiePersistenceEnabled)
: d_gotProtocolHandlers(false)
, d_path(path)
, d_diskCacheEnabled(diskCacheEnabled)
, d_cookiePersistenceEnabled(cookiePersistenceEnabled)
, d_wasProxyInitialized(false)
{
}

URLRequestContextGetterImpl::~URLRequestContextGetterImpl()
{
}

void URLRequestContextGetterImpl::setProxyConfig(const net::ProxyConfig& config)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    d_wasProxyInitialized = true;

    scoped_ptr<net::ProxyConfigService> proxyConfigService(
        new net::ProxyConfigServiceFixed(config));

    GetNetworkTaskRunner()->PostTask(
        FROM_HERE,
        base::Bind(&URLRequestContextGetterImpl::updateProxyConfig,
                   this,
                   base::Passed(&proxyConfigService)));
}

void URLRequestContextGetterImpl::useSystemProxyConfig()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    d_wasProxyInitialized = true;

    base::MessageLoop* ioLoop =
        content::BrowserThread::UnsafeGetMessageLoopForThread(
            content::BrowserThread::IO);

    base::MessageLoop* fileLoop =
        content::BrowserThread::UnsafeGetMessageLoopForThread(
            content::BrowserThread::FILE);

    // We must create the proxy config service on the UI loop on Linux
    // because it must synchronously run on the glib message loop.  This
    // will be passed to the ProxyServer on the IO thread.
    scoped_ptr<net::ProxyConfigService> proxyConfigService(
        net::ProxyService::CreateSystemProxyConfigService(
            ioLoop->message_loop_proxy(),
            fileLoop));

    GetNetworkTaskRunner()->PostTask(
        FROM_HERE,
        base::Bind(&URLRequestContextGetterImpl::updateProxyConfig,
                   this,
                   base::Passed(&proxyConfigService)));
}

void URLRequestContextGetterImpl::setProtocolHandlers(
    content::ProtocolHandlerMap* protocolHandlers,
    content::URLRequestInterceptorScopedVector requestInterceptors)
{
    // Note: It is guaranteed that this is only called once, and it happens
    //       before GetURLRequestContext() is called on the IO thread.
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    // If we haven't got a proxy configuration at this point, just initialize
    // to use the system proxy settings.  Note that proxy configuration must be
    // setup before the IO thread starts using this URLRequestContextGetter.
    if (!d_wasProxyInitialized) {
        useSystemProxyConfig();
        DCHECK(d_wasProxyInitialized);
    }

    base::AutoLock guard(d_protocolHandlersLock);
    DCHECK(!d_gotProtocolHandlers);
    std::swap(d_protocolHandlers, *protocolHandlers);
    d_requestInterceptors = requestInterceptors.Pass();
    d_gotProtocolHandlers = true;
}


// net::URLRequestContextGetter implementation.

net::URLRequestContext* URLRequestContextGetterImpl::GetURLRequestContext()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (!d_urlRequestContext.get())
        initialize();
    return d_urlRequestContext.get();
}

scoped_refptr<base::SingleThreadTaskRunner>
URLRequestContextGetterImpl::GetNetworkTaskRunner() const
{
    return content::BrowserThread::GetMessageLoopProxyForThread(
            content::BrowserThread::IO);
}

void URLRequestContextGetterImpl::initialize()
{
    DCHECK(d_proxyService.get());

    if (d_cookiePersistenceEnabled) {
        d_cookieStore =
            new content::SQLitePersistentCookieStore(
                d_path.Append(FILE_PATH_LITERAL("Cookies")),
                GetNetworkTaskRunner(),
                content::BrowserThread::GetMessageLoopProxyForThread(
                    content::BrowserThread::FILE),
                true,
                (quota::SpecialStoragePolicy*)0,
                (content::CookieCryptoDelegate*)0);
    }

    const base::CommandLine& cmdline = *base::CommandLine::ForCurrentProcess();

    d_urlRequestContext.reset(new net::URLRequestContext());
    d_urlRequestContext->set_proxy_service(d_proxyService.get());
    d_storage.reset(
        new net::URLRequestContextStorage(d_urlRequestContext.get()));
    d_storage->set_network_delegate(new NetworkDelegateImpl());
    d_storage->set_cookie_store(
        new net::CookieMonster(d_cookieStore.get(), 0));
    d_storage->set_channel_id_service(new net::ChannelIDService(
        new net::DefaultChannelIDStore(0),
        base::WorkerPool::GetTaskRunner(true)));
    d_storage->set_http_user_agent_settings(
        new net::StaticHttpUserAgentSettings(
        "en-us,en", base::EmptyString()));

    scoped_ptr<net::HostResolver> hostResolver
        = net::HostResolver::CreateDefaultResolver(0);

    d_storage->set_cert_verifier(net::CertVerifier::CreateDefault());
    d_storage->set_transport_security_state(new net::TransportSecurityState());
    d_storage->set_ssl_config_service(new net::SSLConfigServiceDefaults);
    d_storage->set_http_auth_handler_factory(
        net::HttpAuthHandlerFactory::CreateDefault(hostResolver.get()));
    d_storage->set_http_server_properties(
        scoped_ptr<net::HttpServerProperties>(
            new net::HttpServerPropertiesImpl()));

    net::HttpNetworkSession::Params networkSessionParams;
    networkSessionParams.cert_verifier =
        d_urlRequestContext->cert_verifier();
    networkSessionParams.transport_security_state =
        d_urlRequestContext->transport_security_state();
    networkSessionParams.channel_id_service =
        d_urlRequestContext->channel_id_service();
    networkSessionParams.proxy_service =
        d_urlRequestContext->proxy_service();
    networkSessionParams.ssl_config_service =
        d_urlRequestContext->ssl_config_service();
    networkSessionParams.http_auth_handler_factory =
        d_urlRequestContext->http_auth_handler_factory();
    networkSessionParams.network_delegate =
        d_urlRequestContext->network_delegate();
    networkSessionParams.http_server_properties =
        d_urlRequestContext->http_server_properties();
    networkSessionParams.ignore_certificate_errors = false;
    if (cmdline.HasSwitch(switches::kHostResolverRules)) {
        scoped_ptr<net::MappedHostResolver> mappedHostResolver(
            new net::MappedHostResolver(hostResolver.Pass()));
        mappedHostResolver->SetRulesFromString(
            cmdline.GetSwitchValueASCII(switches::kHostResolverRules));
        hostResolver = mappedHostResolver.Pass();
    }

    // Give d_storage ownership at the end in case it's mappedHostResolver.
    d_storage->set_host_resolver(hostResolver.Pass());
    networkSessionParams.host_resolver =
        d_urlRequestContext->host_resolver();

    bool useCache = d_diskCacheEnabled;
    net::HttpCache::BackendFactory* backendFactory =
        useCache ? new net::HttpCache::DefaultBackend(net::DISK_CACHE,
                                                      net::CACHE_BACKEND_DEFAULT,
                                                      d_path.Append(FILE_PATH_LITERAL("Cache")),
                                                      0,
                                                      content::BrowserThread::GetMessageLoopProxyForThread(content::BrowserThread::CACHE))
                 : net::HttpCache::DefaultBackend::InMemory(0);

    net::HttpNetworkLayer* networkLayer
        = new net::HttpNetworkLayer(new net::HttpNetworkSession(networkSessionParams));

    net::HttpCache* mainCache = new net::HttpCache(networkLayer,
                                                   networkSessionParams.net_log,
                                                   backendFactory);
    d_storage->set_http_transaction_factory(mainCache);

    scoped_ptr<net::URLRequestJobFactoryImpl> jobFactory(
        new net::URLRequestJobFactoryImpl());
    {
        base::AutoLock guard(d_protocolHandlersLock);
        DCHECK(d_gotProtocolHandlers);
        installProtocolHandlers(jobFactory.get(), &d_protocolHandlers);
    }
    bool setProtocol = jobFactory->SetProtocolHandler(
        url::kDataScheme,
        new net::DataProtocolHandler);
    DCHECK(setProtocol);
    setProtocol = jobFactory->SetProtocolHandler(
        url::kFileScheme,
        new net::FileProtocolHandler(
            content::BrowserThread::GetBlockingPool()->
                GetTaskRunnerWithShutdownBehavior(
                    base::SequencedWorkerPool::SKIP_ON_SHUTDOWN)));
    DCHECK(setProtocol);
    d_storage->set_job_factory(jobFactory.release());
}

void URLRequestContextGetterImpl::updateProxyConfig(
    scoped_ptr<net::ProxyConfigService> proxyConfigService)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    if (d_proxyService) {
        d_proxyService->ResetConfigService(proxyConfigService.release());
        return;
    }

    // TODO(jam): use v8 if possible, look at chrome code.
    d_proxyService.reset(
        net::ProxyService::CreateUsingSystemProxyResolver(
            proxyConfigService.release(), 0, 0));
}

}  // close namespace blpwtk2




