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

#include <blpwtk2_httptransactionfactoryimpl.h>
#include <blpwtk2_networkdelegateimpl.h>

#include <base/command_line.h>
#include <base/logging.h>  // for DCHECK
#include <base/string_util.h>
#include <base/threading/worker_pool.h>
#include <content/public/browser/browser_thread.h>
#include <content/public/common/content_switches.h>
#include <content/public/common/url_constants.h>
#include <net/base/cert_verifier.h>
#include <net/cookies/cookie_monster.h>
#include <net/dns/mapped_host_resolver.h>
#include <net/http/http_auth_handler_factory.h>
#include <net/http/http_cache.h>
#include <net/http/http_network_layer.h>
#include <net/http/http_network_session.h>
#include <net/http/http_server_properties_impl.h>
#include <net/proxy/proxy_service.h>
#include <net/ssl/default_server_bound_cert_store.h>
#include <net/ssl/server_bound_cert_service.h>
#include <net/ssl/ssl_config_service_defaults.h>
#include <net/url_request/protocol_intercept_job_factory.h>
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
        bool ignoreCertificateErrors,
        const base::FilePath& basePath,
        MessageLoop* ioLoop,
        MessageLoop* fileLoop,
        content::ProtocolHandlerMap* protocolHandlers)
: d_ignoreCertificateErrors(ignoreCertificateErrors)
, d_basePath(basePath)
, d_ioLoop(ioLoop)
, d_fileLoop(fileLoop)
{
    // Must first be created on the UI thread.
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

    std::swap(d_protocolHandlers, *protocolHandlers);

    // We must create the proxy config service on the UI loop on Linux because
    // it must synchronously run on the glib message loop.  This will be passed
    // to the URLRequestContextStorage on the IO thread in initialize().
    d_proxyConfigService.reset(
        net::ProxyService::CreateSystemProxyConfigService(
            d_ioLoop->message_loop_proxy(), d_fileLoop));
}

URLRequestContextGetterImpl::~URLRequestContextGetterImpl()
{
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
    const CommandLine& cmdline = *CommandLine::ForCurrentProcess();

    d_urlRequestContext.reset(new net::URLRequestContext());
    d_networkDelegate.reset(new NetworkDelegateImpl());
    d_urlRequestContext->set_network_delegate(d_networkDelegate.get());
    d_storage.reset(
        new net::URLRequestContextStorage(d_urlRequestContext.get()));
    d_storage->set_cookie_store(new net::CookieMonster(0, 0));
    d_storage->set_server_bound_cert_service(new net::ServerBoundCertService(
        new net::DefaultServerBoundCertStore(0),
        base::WorkerPool::GetTaskRunner(true)));
    d_storage->set_http_user_agent_settings(
        new net::StaticHttpUserAgentSettings(
        "en-us,en", EmptyString()));

    scoped_ptr<net::HostResolver> hostResolver
        = net::HostResolver::CreateDefaultResolver(0);

    d_storage->set_cert_verifier(net::CertVerifier::CreateDefault());
    {
        // TODO(jam): use v8 if possible, look at chrome code.
        d_storage->set_proxy_service(
            net::ProxyService::CreateUsingSystemProxyResolver(
                d_proxyConfigService.release(), 0, 0));
    }
    d_storage->set_ssl_config_service(new net::SSLConfigServiceDefaults);
    d_storage->set_http_auth_handler_factory(
        net::HttpAuthHandlerFactory::CreateDefault(hostResolver.get()));
    d_storage->set_http_server_properties(new net::HttpServerPropertiesImpl);

    net::HttpNetworkSession::Params networkSessionParams;
    networkSessionParams.cert_verifier =
        d_urlRequestContext->cert_verifier();
    networkSessionParams.server_bound_cert_service =
        d_urlRequestContext->server_bound_cert_service();
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
    networkSessionParams.ignore_certificate_errors = d_ignoreCertificateErrors;
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

    bool useCache = true;  // TODO: make this configurable
    net::HttpCache::BackendFactory* backendFactory =
        useCache ? new net::HttpCache::DefaultBackend(net::DISK_CACHE,
                                                      d_basePath.Append(FILE_PATH_LITERAL("Cache")),
                                                      0,
                                                      content::BrowserThread::GetMessageLoopProxyForThread(content::BrowserThread::CACHE))
                 : net::HttpCache::DefaultBackend::InMemory(0);

    net::HttpNetworkLayer* defaultNetworkLayer
        = new net::HttpNetworkLayer(new net::HttpNetworkSession(networkSessionParams));

    // our own network layer that has hooks to blpwtk2::TransactionHandler
    blpwtk2::HttpTransactionFactoryImpl* hookedNetworkLayer
        = new blpwtk2::HttpTransactionFactoryImpl(defaultNetworkLayer);

    net::HttpCache* mainCache = new net::HttpCache(hookedNetworkLayer,
                                                   networkSessionParams.net_log,
                                                   backendFactory);
    d_storage->set_http_transaction_factory(mainCache);

    scoped_ptr<net::URLRequestJobFactoryImpl> jobFactory(
        new net::URLRequestJobFactoryImpl());
    installProtocolHandlers(jobFactory.get(), &d_protocolHandlers);
    d_storage->set_job_factory(jobFactory.release());
}

}  // close namespace blpwtk2




