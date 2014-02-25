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

#include <blpwtk2_networkdelegateimpl.h>

#include <net/base/net_errors.h>

namespace blpwtk2 {

NetworkDelegateImpl::NetworkDelegateImpl()
{
}

NetworkDelegateImpl::~NetworkDelegateImpl()
{
}

int NetworkDelegateImpl::OnBeforeURLRequest(net::URLRequest* request,
                                            const net::CompletionCallback& callback,
                                            GURL* new_url)
{
    return net::OK;
}

int NetworkDelegateImpl::OnBeforeSendHeaders(net::URLRequest* request,
                                             const net::CompletionCallback& callback,
                                             net::HttpRequestHeaders* headers)
{
    return net::OK;
}

void NetworkDelegateImpl::OnSendHeaders(net::URLRequest* request,
                                        const net::HttpRequestHeaders& headers)
{
}

int NetworkDelegateImpl::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers)
{
    return net::OK;
}

void NetworkDelegateImpl::OnBeforeRedirect(net::URLRequest* request,
                                           const GURL& new_location)
{
}

void NetworkDelegateImpl::OnResponseStarted(net::URLRequest* request)
{
}

void NetworkDelegateImpl::OnRawBytesRead(const net::URLRequest& request, int bytes_read)
{
}

void NetworkDelegateImpl::OnCompleted(net::URLRequest* request, bool started)
{
}

void NetworkDelegateImpl::OnURLRequestDestroyed(net::URLRequest* request)
{
}

void NetworkDelegateImpl::OnPACScriptError(int line_number, const string16& error)
{
}

NetworkDelegateImpl::AuthRequiredResponse
NetworkDelegateImpl::OnAuthRequired(
    net::URLRequest* request,
    const net::AuthChallengeInfo& auth_info,
    const AuthCallback& callback,
    net::AuthCredentials* credentials)
{
    return AUTH_REQUIRED_RESPONSE_NO_ACTION;
}

bool NetworkDelegateImpl::OnCanGetCookies(const net::URLRequest& request,
                                          const net::CookieList& cookie_list)
{
    return true;
}

bool NetworkDelegateImpl::OnCanSetCookie(const net::URLRequest& request,
                                         const std::string& cookie_line,
                                         net::CookieOptions* options)
{
    return true;
}

bool NetworkDelegateImpl::OnCanAccessFile(const net::URLRequest& request,
                                          const base::FilePath& path) const
{
    return true;
}

bool NetworkDelegateImpl::OnCanThrottleRequest(const net::URLRequest& request) const
{
    return false;
}

int NetworkDelegateImpl::OnBeforeSocketStreamConnect(
    net::SocketStream* socket,
    const net::CompletionCallback& callback)
{
    return net::OK;
}

void NetworkDelegateImpl::OnRequestWaitStateChange(const net::URLRequest& request,
                                                   RequestWaitState state)
{
}

}  // close namespace blpwtk2

