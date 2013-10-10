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

#ifndef INCLUDED_BLPWTK2_HTTPTRANSACTIONIMPL_H
#define INCLUDED_BLPWTK2_HTTPTRANSACTIONIMPL_H

#include <blpwtk2_config.h>

#include <blpwtk2_httptransaction.h>

#include <net/http/http_transaction.h>

namespace net {
    class HttpTransactionFactory;
    class HttpTransactionDelegate;
}  // close namespace net

namespace blpwtk2 {


// This is our implementation of 'net::HttpTransaction'.
// TODO: document this more
class HttpTransactionImpl : public blpwtk2::HttpTransaction,
                              public net::HttpTransaction {

    class RefForCallback : public base::RefCountedThreadSafe<RefForCallback> {
      public:
        explicit RefForCallback(HttpTransactionImpl* transaction) : d_transaction(transaction) {}
        void reset() { d_transaction = 0; }
        HttpTransactionImpl* transaction() const { return d_transaction; }
      private:
        HttpTransactionImpl* d_transaction;
    };

    static void invokeDataAvailableCallbackRef(scoped_refptr<RefForCallback> refForCallback);
    void invokeDataAvailableCallback();
    void initResponseInfoIfNecessary();

public:
    HttpTransactionImpl(net::HttpTransactionFactory* fallbackFactory,
                        net::HttpTransactionDelegate* fallbackDelegate);
    virtual ~HttpTransactionImpl();

    // =========== blpwtk2::HttpTransaction overrides ===============

    virtual String url() const OVERRIDE;
    virtual String method() const OVERRIDE;
    virtual void replaceStatusLine(const StringRef& newStatus) OVERRIDE;
    virtual void addResponseHeader(const StringRef& header) OVERRIDE;
    virtual void notifyDataAvailable() OVERRIDE;

    // =========== net::HttpTransaction overrides ===============

    // Starts the HTTP transaction (i.e., sends the HTTP request).
    //
    // Returns OK if the transaction could be started synchronously, which means
    // that the request was served from the cache.  ERR_IO_PENDING is returned to
    // indicate that the CompletionCallback will be notified once response info is
    // available or if an IO error occurs.  Any other return value indicates that
    // the transaction could not be started.
    //
    // Regardless of the return value, the caller is expected to keep the
    // request_info object alive until Destroy is called on the transaction.
    //
    // NOTE: The transaction is not responsible for deleting the callback object.
    //
    // Profiling information for the request is saved to |net_log| if non-NULL.
    virtual int Start(const net::HttpRequestInfo* request_info,
                      const net::CompletionCallback& callback,
                      const net::BoundNetLog& net_log) OVERRIDE;

    // Restarts the HTTP transaction, ignoring the last error.  This call can
    // only be made after a call to Start (or RestartIgnoringLastError) failed.
    // Once Read has been called, this method cannot be called.  This method is
    // used, for example, to continue past various SSL related errors.
    //
    // Not all errors can be ignored using this method.  See error code
    // descriptions for details about errors that can be ignored.
    //
    // NOTE: The transaction is not responsible for deleting the callback object.
    //
    virtual int RestartIgnoringLastError(const net::CompletionCallback& callback) OVERRIDE;

    // Restarts the HTTP transaction with a client certificate.
    virtual int RestartWithCertificate(net::X509Certificate* client_cert,
                                       const net::CompletionCallback& callback) OVERRIDE;

    // Restarts the HTTP transaction with authentication credentials.
    virtual int RestartWithAuth(const net::AuthCredentials& credentials,
                                const net::CompletionCallback& callback) OVERRIDE;

    // Returns true if auth is ready to be continued. Callers should check
    // this value anytime Start() completes: if it is true, the transaction
    // can be resumed with RestartWithAuth(L"", L"", callback) to resume
    // the automatic auth exchange. This notification gives the caller a
    // chance to process the response headers from all of the intermediate
    // restarts needed for authentication.
    virtual bool IsReadyToRestartForAuth() OVERRIDE;

    // Once response info is available for the transaction, response data may be
    // read by calling this method.
    //
    // Response data is copied into the given buffer and the number of bytes
    // copied is returned.  ERR_IO_PENDING is returned if response data is not
    // yet available.  The CompletionCallback is notified when the data copy
    // completes, and it is passed the number of bytes that were successfully
    // copied.  Or, if a read error occurs, the CompletionCallback is notified of
    // the error.  Any other negative return value indicates that the transaction
    // could not be read.
    //
    // NOTE: The transaction is not responsible for deleting the callback object.
    // If the operation is not completed immediately, the transaction must acquire
    // a reference to the provided buffer.
    //
    virtual int Read(net::IOBuffer* buf, int buf_len,
                     const net::CompletionCallback& callback) OVERRIDE;

    // Stops further caching of this request by the HTTP cache, if there is any.
    virtual void StopCaching() OVERRIDE;

    // Called to tell the transaction that we have successfully reached the end
    // of the stream. This is equivalent to performing an extra Read() at the end
    // that should return 0 bytes. This method should not be called if the
    // transaction is busy processing a previous operation (like a pending Read).
    virtual void DoneReading() OVERRIDE;

    // Returns the response info for this transaction or NULL if the response
    // info is not available.
    virtual const net::HttpResponseInfo* GetResponseInfo() const OVERRIDE;

    // Returns the load state for this transaction.
    virtual net::LoadState GetLoadState() const OVERRIDE;

    // Returns the upload progress in bytes.  If there is no upload data,
    // zero will be returned.  This does not include the request headers.
    virtual net::UploadProgress GetUploadProgress() const OVERRIDE;

    // Populates all of load timing, except for request start times.
    // |load_timing_info| must have all null times when called.  Returns false and
    // does not modify |load_timing_info| if not currently connected.
    virtual bool GetLoadTimingInfo(net::LoadTimingInfo* load_timing_info) const OVERRIDE;

    // Called when the priority of the parent job changes.
    virtual void SetPriority(net::RequestPriority priority) OVERRIDE;

  private:
      net::HttpTransactionFactory* d_fallbackFactory;
      net::HttpTransactionDelegate* d_fallbackDelegate;
      scoped_ptr<net::HttpTransaction> d_fallbackTransaction;

      scoped_refptr<RefForCallback> d_refForCallback;
      const net::HttpRequestInfo* d_requestInfo;
      scoped_ptr<net::HttpResponseInfo> d_responseInfo;
      scoped_refptr<net::IOBuffer> d_lastBuffer;
      int d_lastReadSize;
      bool d_isCompleted;
      net::CompletionCallback d_callback;
      net::LoadState d_loadState;
      void* d_userData;
      net::RequestPriority d_priority;

      DISALLOW_COPY_AND_ASSIGN(HttpTransactionImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_HTTPTRANSACTIONIMPL_H

