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

#include <blpwtk2_httptransactionimpl.h>

#include <blpwtk2_string.h>
#include <blpwtk2_httptransactionhandler.h>
#include <blpwtk2_statics.h>

#include <content/public/browser/browser_thread.h>
#include <net/base/io_buffer.h>
#include <net/base/net_errors.h>
#include <net/http/http_request_info.h>
#include <net/http/http_response_info.h>
#include <net/http/http_response_headers.h>
#include <net/http/http_transaction_factory.h>

namespace blpwtk2 {

// static
void HttpTransactionImpl::invokeDataAvailableCallbackRef(scoped_refptr<RefForCallback> refForCallback)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    HttpTransactionImpl* transaction = refForCallback->transaction();
    if (transaction)
        transaction->invokeDataAvailableCallback();
}

void HttpTransactionImpl::invokeDataAvailableCallback()
{
    DCHECK(!d_fallbackTransaction.get());

    if (d_callback.is_null())
        return;

    // Invoking the callback could result in Read being called immediately,
    // so let's clear out our previous state first.
    scoped_refptr<net::IOBuffer> lastBuffer = d_lastBuffer;
    net::CompletionCallback callback = d_callback;
    d_lastBuffer = 0;
    d_callback.Reset();

    int arg = lastBuffer.get() ? d_lastReadSize : net::OK;
    callback.Run(arg);
}

void HttpTransactionImpl::initResponseInfoIfNecessary()
{
    if (d_responseInfo.get())
        return;

    d_responseInfo.reset(new net::HttpResponseInfo());
    d_responseInfo->socket_address = net::HostPortPair::FromURL(d_requestInfo->url);
    d_responseInfo->headers = new net::HttpResponseHeaders("HTTP/1.1 200 OK\0\0");
}

HttpTransactionImpl::HttpTransactionImpl(net::HttpTransactionFactory* fallbackFactory,
                                         net::HttpTransactionDelegate* fallbackDelegate)
: d_fallbackFactory(fallbackFactory)
, d_fallbackDelegate(fallbackDelegate)
, d_loadState(net::LOAD_STATE_WAITING_FOR_RESPONSE)
, d_priority(net::DEFAULT_PRIORITY)
{
    DCHECK(d_fallbackFactory);
    d_refForCallback = new RefForCallback(this);
}

HttpTransactionImpl::~HttpTransactionImpl()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    d_refForCallback->reset();
    if (!d_fallbackTransaction.get()) {
        Statics::httpTransactionHandler->endTransaction(this, d_userData);
    }
}

// =========== blpwtk2::HttpTransaction overrides ===============

String HttpTransactionImpl::url() const
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    return String(d_requestInfo->url.spec());
}

String HttpTransactionImpl::method() const
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    return String(d_requestInfo->method);
}

void HttpTransactionImpl::replaceStatusLine(const StringRef& newStatus)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    DCHECK(!d_fallbackTransaction.get());
    DCHECK(net::LOAD_STATE_WAITING_FOR_RESPONSE == d_loadState);

    initResponseInfoIfNecessary();

    // TODO: avoid copy
    std::string statusStr(newStatus.data(), newStatus.length());
    d_responseInfo->headers->ReplaceStatusLine(statusStr);
}

void HttpTransactionImpl::addResponseHeader(const StringRef& header)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
    DCHECK(!d_fallbackTransaction.get());
    DCHECK(net::LOAD_STATE_WAITING_FOR_RESPONSE == d_loadState);

    initResponseInfoIfNecessary();

    // TODO: avoid copy
    std::string headerStr(header.data(), header.length());
    d_responseInfo->headers->AddHeader(headerStr);
}

void HttpTransactionImpl::notifyDataAvailable()
{
    DCHECK(!d_fallbackTransaction.get());
    content::BrowserThread::PostTask(content::BrowserThread::IO,
                                     FROM_HERE,
                                     base::Bind(&HttpTransactionImpl::invokeDataAvailableCallbackRef,
                                                d_refForCallback));
}

// =========== net::HttpTransaction overrides ===============

int HttpTransactionImpl::Start(const net::HttpRequestInfo* request_info,
                               const net::CompletionCallback& callback,
                               const net::BoundNetLog& net_log)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    d_requestInfo = request_info;

    if (Statics::httpTransactionHandler) {
        if (Statics::httpTransactionHandler->startTransaction(this,
                                                              &d_userData)) {
            d_callback = callback;
            d_isCompleted = false;
            return net::ERR_IO_PENDING;
        }
    }

    int rc = d_fallbackFactory->CreateTransaction(d_priority,
                                                  &d_fallbackTransaction,
                                                  d_fallbackDelegate);
    if (rc != net::OK) {
        return net::ERR_FAILED;
    }
    DCHECK(d_fallbackTransaction.get());
    return d_fallbackTransaction->Start(request_info, callback, net_log);
}

int HttpTransactionImpl::RestartIgnoringLastError(const net::CompletionCallback& callback)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        return d_fallbackTransaction->RestartIgnoringLastError(callback);

    NOTIMPLEMENTED();
    return net::ERR_FAILED;
}

int HttpTransactionImpl::RestartWithCertificate(net::X509Certificate* client_cert,
                                                const net::CompletionCallback& callback)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        return d_fallbackTransaction->RestartWithCertificate(client_cert, callback);

    NOTIMPLEMENTED();
    return net::ERR_FAILED;
}

int HttpTransactionImpl::RestartWithAuth(const net::AuthCredentials& credentials,
                                         const net::CompletionCallback& callback)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        return d_fallbackTransaction->RestartWithAuth(credentials, callback);

    NOTIMPLEMENTED();
    return net::ERR_FAILED;
}

bool HttpTransactionImpl::IsReadyToRestartForAuth()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    return d_fallbackTransaction.get()
        ? d_fallbackTransaction->IsReadyToRestartForAuth()
        : false;
}

int HttpTransactionImpl::Read(net::IOBuffer* buf, int buf_len,
                              const net::CompletionCallback& callback)
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        return d_fallbackTransaction->Read(buf, buf_len, callback);

    if (d_isCompleted) {
        DCHECK(d_callback.is_null());
        DCHECK(!d_lastBuffer.get());
        return 0;
    }

    d_lastReadSize = Statics::httpTransactionHandler->readResponseBody(this,
                                                                       d_userData,
                                                                       buf->data(),
                                                                       buf_len,
                                                                       &d_isCompleted);
    if (0 > d_lastReadSize) {
        d_lastBuffer = 0;
        d_callback.Reset();
        return net::ERR_FAILED;
    }

    // If d_lastReadSize == 0, then d_isCompleted must be true.
    DCHECK(0 < d_lastReadSize || d_isCompleted);

    d_loadState = net::LOAD_STATE_READING_RESPONSE;
    d_lastBuffer = buf;
    d_callback = callback;

    if (d_isCompleted) {
        // We don't expect handlers to notifyDataAvailable() once they have
        // completed, but chromium's HttpTransaction interface expects that the
        // callback be invoked one last time so that the next Read() returns 0.
        notifyDataAvailable();
    }

    return net::ERR_IO_PENDING;
}

void HttpTransactionImpl::StopCaching()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        d_fallbackTransaction->StopCaching();
}

void HttpTransactionImpl::DoneReading()
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        d_fallbackTransaction->DoneReading();
}

const net::HttpResponseInfo*
HttpTransactionImpl::GetResponseInfo() const
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        return d_fallbackTransaction->GetResponseInfo();

    const_cast<HttpTransactionImpl*>(this)->initResponseInfoIfNecessary();
    return d_responseInfo.get();
}

net::LoadState HttpTransactionImpl::GetLoadState() const
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        return d_fallbackTransaction->GetLoadState();

    return d_loadState;
}

net::UploadProgress HttpTransactionImpl::GetUploadProgress() const
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        return d_fallbackTransaction->GetUploadProgress();

    NOTIMPLEMENTED();
    return net::UploadProgress();
}

bool HttpTransactionImpl::GetLoadTimingInfo(net::LoadTimingInfo* load_timing_info) const
{
    DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));

    if (d_fallbackTransaction.get())
        return d_fallbackTransaction->GetLoadTimingInfo(load_timing_info);

    NOTIMPLEMENTED();
    return false;
}

void HttpTransactionImpl::SetPriority(net::RequestPriority priority)
{
    // TODO: should we assert that we are on IO thread here?

    if (d_fallbackTransaction.get())
        return d_fallbackTransaction->SetPriority(priority);

    d_priority = priority;

    // TODO: forward this to our transaction handler?
}


}  // close namespace blpwtk2

