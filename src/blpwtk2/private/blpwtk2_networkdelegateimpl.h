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

#ifndef INCLUDED_BLPWTK2_NETWORKDELEGATEIMPL_H
#define INCLUDED_BLPWTK2_NETWORKDELEGATEIMPL_H

#include <blpwtk2_config.h>

#include <net/base/network_delegate.h>

namespace blpwtk2 {

// This is our implementation of chromium's NetworkDelegate interface.  We need
// this in order to allow access to the file system using "file://" urls by overriding
// the OnCanAccessFile method.
class NetworkDelegateImpl : public net::NetworkDelegate {
public:
    NetworkDelegateImpl();
    virtual ~NetworkDelegateImpl();

private:
    // Called before a request is sent. Allows the delegate to rewrite the URL
    // being fetched by modifying |new_url|. |callback| and |new_url| are valid
    // only until OnURLRequestDestroyed is called for this request. Returns a net
    // status code, generally either OK to continue with the request or
    // ERR_IO_PENDING if the result is not ready yet. A status code other than OK
    // and ERR_IO_PENDING will cancel the request and report the status code as
    // the reason.
    virtual int OnBeforeURLRequest(net::URLRequest* request,
                                   const net::CompletionCallback& callback,
                                   GURL* new_url) OVERRIDE;

    // Called right before the HTTP headers are sent. Allows the delegate to
    // read/write |headers| before they get sent out. |callback| and |headers| are
    // valid only until OnCompleted or OnURLRequestDestroyed is called for this
    // request.
    // Returns a net status code.
    virtual int OnBeforeSendHeaders(net::URLRequest* request,
                                    const net::CompletionCallback& callback,
                                    net::HttpRequestHeaders* headers) OVERRIDE;

    // Called right before the HTTP request(s) are being sent to the network.
    // |headers| is only valid until OnCompleted or OnURLRequestDestroyed is
    // called for this request.
    virtual void OnSendHeaders(net::URLRequest* request,
                               const net::HttpRequestHeaders& headers) OVERRIDE;

    // Called for HTTP requests when the headers have been received. Returns a net
    // status code, generally either OK to continue with the request or
    // ERR_IO_PENDING if the result is not ready yet.  A status code other than OK
    // and ERR_IO_PENDING will cancel the request and report the status code as
    // the reason.
    // |original_response_headers| contains the headers as received over the
    // network, these must not be modified. |override_response_headers| can be set
    // to new values, that should be considered as overriding
    // |original_response_headers|.
    // |callback|, |original_response_headers|, and |override_response_headers|
    // are only valid until OnURLRequestDestroyed is called for this request.
    virtual int OnHeadersReceived(
        net::URLRequest* request,
        const net::CompletionCallback& callback,
        const net::HttpResponseHeaders* original_response_headers,
        scoped_refptr<net::HttpResponseHeaders>* override_response_headers) OVERRIDE;

    // Called right after a redirect response code was received.
    // |new_location| is only valid until OnURLRequestDestroyed is called for this
    // request.
    virtual void OnBeforeRedirect(net::URLRequest* request,
                                  const GURL& new_location) OVERRIDE;

    // This corresponds to URLRequestDelegate::OnResponseStarted.
    virtual void OnResponseStarted(net::URLRequest* request) OVERRIDE;

    // Called every time we read raw bytes.
    virtual void OnRawBytesRead(const net::URLRequest& request, int bytes_read) OVERRIDE;

    // Indicates that the URL request has been completed or failed.
    // |started| indicates whether the request has been started. If false,
    // some information like the socket address is not available.
    virtual void OnCompleted(net::URLRequest* request, bool started) OVERRIDE;

    // Called when an URLRequest is being destroyed. Note that the request is
    // being deleted, so it's not safe to call any methods that may result in
    // a virtual method call.
    virtual void OnURLRequestDestroyed(net::URLRequest* request) OVERRIDE;

    // Corresponds to ProxyResolverJSBindings::OnError.
    virtual void OnPACScriptError(int line_number, const base::string16& error) OVERRIDE;

    // Called when a request receives an authentication challenge
    // specified by |auth_info|, and is unable to respond using cached
    // credentials. |callback| and |credentials| must be non-NULL, and must
    // be valid until OnURLRequestDestroyed is called for |request|.
    //
    // The following return values are allowed:
    //  - AUTH_REQUIRED_RESPONSE_NO_ACTION: |auth_info| is observed, but
    //    no action is being taken on it.
    //  - AUTH_REQUIRED_RESPONSE_SET_AUTH: |credentials| is filled in with
    //    a username and password, which should be used in a response to
    //    |auth_info|.
    //  - AUTH_REQUIRED_RESPONSE_CANCEL_AUTH: The authentication challenge
    //    should not be attempted.
    //  - AUTH_REQUIRED_RESPONSE_IO_PENDING: The action will be decided
    //    asynchronously. |callback| will be invoked when the decision is made,
    //    and one of the other AuthRequiredResponse values will be passed in with
    //    the same semantics as described above.
    virtual AuthRequiredResponse OnAuthRequired(
        net::URLRequest* request,
        const net::AuthChallengeInfo& auth_info,
        const AuthCallback& callback,
        net::AuthCredentials* credentials) OVERRIDE;

    // Called when reading cookies to allow the network delegate to block access
    // to the cookie. This method will never be invoked when
    // LOAD_DO_NOT_SEND_COOKIES is specified.
    virtual bool OnCanGetCookies(const net::URLRequest& request,
                                 const net::CookieList& cookie_list) OVERRIDE;

    // Called when a cookie is set to allow the network delegate to block access
    // to the cookie. This method will never be invoked when
    // LOAD_DO_NOT_SAVE_COOKIES is specified.
    virtual bool OnCanSetCookie(const net::URLRequest& request,
                                const std::string& cookie_line,
                                net::CookieOptions* options) OVERRIDE;

    // Called when a file access is attempted to allow the network delegate to
    // allow or block access to the given file path.  Returns true if access is
    // allowed.
    virtual bool OnCanAccessFile(const net::URLRequest& request,
                                 const base::FilePath& path) const OVERRIDE;

    // Returns true if the given request may be rejected when the
    // URLRequestThrottlerManager believes the server servicing the
    // request is overloaded or down.
    virtual bool OnCanThrottleRequest(const net::URLRequest& request) const OVERRIDE;

    // Called before a SocketStream tries to connect.
    virtual int OnBeforeSocketStreamConnect(
        net::SocketStream* socket,
        const net::CompletionCallback& callback) OVERRIDE;

    DISALLOW_COPY_AND_ASSIGN(NetworkDelegateImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_NETWORKDELEGATEIMPL_H

