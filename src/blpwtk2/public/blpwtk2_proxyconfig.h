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

#ifndef INCLUDED_BLPWTK2_PROXYCONFIG_H
#define INCLUDED_BLPWTK2_PROXYCONFIG_H

#include <blpwtk2_config.h>

namespace blpwtk2 {

struct ProxyConfigImpl;
class StringRef;
class ProxyConfig;

ProxyConfigImpl* getProxyConfigImpl(ProxyConfig&);
const ProxyConfigImpl* getProxyConfigImpl(const ProxyConfig&);

// This is a value-semantic type containing proxy configuration settings.
// Different profiles can be setup with different proxy configurations.  The
// ProxyConfig type can specify both automatic settings (via pac url) and
// manual settings.  Automatic settings will override manual settings.  Note
// that using a default-constructed ProxyConfig effectively means "always use
// direct connections".
class BLPWTK2_EXPORT ProxyConfig {
  public:
    enum ProxyType {
        DIRECT,  // no proxy is used (a direct connection is attempted)
        HTTP,
        SOCKS4,
        SOCKS5,
        HTTPS,
    };

    // A default constructed ProxyConfig will always use direct connection.
    ProxyConfig();
    ProxyConfig(const ProxyConfig& other);
    ~ProxyConfig();
    ProxyConfig& operator=(const ProxyConfig& rhs);

    // The url to be used for PAC (proxy-automatic-configuration).  If the pac
    // url is downloaded and parsed successfully, it will override any manual
    // settings.
    void setPacUrl(const StringRef& url);


    // The remaining settings here are only used if the pac url is not set, or
    // if it fails to download or parse.
    // Different lists of proxies can be configured for "http://", "https://",
    // and "ftp://" urls, and a fallback list is used for all other url
    // schemes.  The proxies in the list will be tried in the order specified
    // until one of them succeeds, otherwise a direct connection is attempted.

    // proxies for http urls
    void addHttpProxy(ProxyType type,
                      const StringRef& host,
                      int port);
    void clearHttpProxies();

    // proxies for https urls
    void addHttpsProxy(ProxyType type,
                       const StringRef& host,
                       int port);
    void clearHttpsProxies();

    // proxies for ftp urls
    void addFtpProxy(ProxyType type,
                     const StringRef& host,
                     int port);
    void clearFtpProxies();

    // proxies for all other url schemes
    void addFallbackProxy(ProxyType type,
                          const StringRef& host,
                          int port);
    void clearFallbackProxies();

    // Rules for hostnames that will bypass this proxy configuration.  The
    // format of the rule string is documented here:
    //     http://src.chromium.org/viewvc/chrome/trunk/src/net/proxy/proxy_bypass_rules.h?revision=146163
    // starting at line 94.
    void addBypassRule(const StringRef& rule);
    void clearBypassRules();

  private:
    friend ProxyConfigImpl* getProxyConfigImpl(ProxyConfig&);
    friend const ProxyConfigImpl* getProxyConfigImpl(const ProxyConfig&);
    ProxyConfigImpl *d_impl;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROXYCONFIG_H
