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

#include <blpwtk2_proxyconfig.h>
#include <blpwtk2_proxyconfigimpl.h>
#include <blpwtk2_stringref.h>

#include <base/logging.h>

#include <string>

namespace blpwtk2 {

static net::ProxyServer makeServer(ProxyConfig::ProxyType type,
                                   const StringRef& host,
                                   int port)
{
    net::ProxyServer::Scheme scheme = net::ProxyServer::SCHEME_INVALID;
    switch (type) {
    case ProxyConfig::DIRECT:
        scheme = net::ProxyServer::SCHEME_DIRECT;
        break;
    case ProxyConfig::HTTP:
        scheme = net::ProxyServer::SCHEME_HTTP;
        break;
    case ProxyConfig::SOCKS4:
        scheme = net::ProxyServer::SCHEME_SOCKS4;
        break;
    case ProxyConfig::SOCKS5:
        scheme = net::ProxyServer::SCHEME_SOCKS5;
        break;
    case ProxyConfig::HTTPS:
        scheme = net::ProxyServer::SCHEME_HTTPS;
        break;
    }

    DCHECK(net::ProxyServer::SCHEME_INVALID != scheme);
    DCHECK(0 < port);

    std::string shost(host.data(), host.length());
    return net::ProxyServer(scheme, net::HostPortPair(shost, port));
}

ProxyConfig::ProxyConfig()
: d_impl(new ProxyConfigImpl())
{
    d_impl->d_config.proxy_rules().type
        = net::ProxyConfig::ProxyRules::TYPE_PROXY_PER_SCHEME;
}

ProxyConfig::ProxyConfig(const ProxyConfig& other)
: d_impl(new ProxyConfigImpl(*other.d_impl))
{
}

ProxyConfig::~ProxyConfig()
{
    delete d_impl;
}

ProxyConfig& ProxyConfig::operator=(const ProxyConfig& rhs)
{
    if (this != &rhs) {
        ProxyConfigImpl* impl = new ProxyConfigImpl(*rhs.d_impl);
        delete d_impl;
        d_impl = impl;
    }
    return *this;
}

void ProxyConfig::setPacUrl(const StringRef& url)
{
    std::string surl(url.data(), url.length());
    GURL gurl(surl);
    d_impl->d_config.set_pac_url(gurl);
}

void ProxyConfig::addHttpProxy(ProxyType type,
                               const StringRef& host,
                               int port)
{
    d_impl->d_config.proxy_rules().proxies_for_http.AddProxyServer(
        makeServer(type, host, port));
}

void ProxyConfig::addHttpsProxy(ProxyType type,
                                const StringRef& host,
                                int port)
{
    d_impl->d_config.proxy_rules().proxies_for_https.AddProxyServer(
        makeServer(type, host, port));
}

void ProxyConfig::addFtpProxy(ProxyType type,
                              const StringRef& host,
                              int port)
{
    d_impl->d_config.proxy_rules().proxies_for_ftp.AddProxyServer(
        makeServer(type, host, port));
}

void ProxyConfig::addFallbackProxy(ProxyType type,
                                   const StringRef& host,
                                   int port)
{
    d_impl->d_config.proxy_rules().fallback_proxies.AddProxyServer(
        makeServer(type, host, port));
}

void ProxyConfig::addBypassRule(const StringRef& rule)
{
    std::string srule(rule.data(), rule.length());
    d_impl->d_config.proxy_rules().bypass_rules.AddRuleFromString(srule);
}

const ProxyConfigImpl* getProxyConfigImpl(const ProxyConfig& config)
{
    return config.d_impl;
}

}  // close namespace blpwtk2
