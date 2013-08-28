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

#ifndef INCLUDED_BLPWTK2_URLREQUESTCONTEXTGETTERIMPL_H
#define INCLUDED_BLPWTK2_URLREQUESTCONTEXTGETTERIMPL_H

#include <blpwtk2_config.h>

#include <base/files/file_path.h>
#include <base/memory/ref_counted.h>
#include <base/memory/scoped_ptr.h>
#include <content/public/browser/content_browser_client.h>  // for ProtocolHandlerMap
#include <net/url_request/url_request_context_getter.h>
#include <net/url_request/url_request_job_factory.h>

namespace net {
    class ProxyConfigService;
    class NetworkDelegate;
    class URLRequestContext;
    class URLRequestContextStorage;
}  // close namespace net

namespace blpwtk2 {

class BrowserContextImpl;

// An instance of this is created for each BrowserContext by the
// ContentBrowserClient.  This implementation is mostly a copy of
// ShellURLRequestContextGetter from content_shell, but cleaned up and modified
// a bit to make it do what we need.
class URLRequestContextGetterImpl : public net::URLRequestContextGetter {
public:
    URLRequestContextGetterImpl(
        BrowserContextImpl* browserContext,
        content::ProtocolHandlerMap* protocolHandlers);
    virtual ~URLRequestContextGetterImpl();

    // net::URLRequestContextGetter implementation.
    virtual net::URLRequestContext* GetURLRequestContext() OVERRIDE;

    virtual scoped_refptr<base::SingleThreadTaskRunner>
    GetNetworkTaskRunner() const OVERRIDE;

private:
    void initialize();

    BrowserContextImpl* d_browserContext;
    scoped_ptr<net::NetworkDelegate> d_networkDelegate;
    scoped_ptr<net::URLRequestContextStorage> d_storage;
    scoped_ptr<net::URLRequestContext> d_urlRequestContext;
    content::ProtocolHandlerMap d_protocolHandlers;

    DISALLOW_COPY_AND_ASSIGN(URLRequestContextGetterImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_URLREQUESTCONTEXTGETTERIMPL_H

