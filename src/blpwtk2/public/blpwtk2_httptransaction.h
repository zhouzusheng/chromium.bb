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

#ifndef INCLUDED_BLPWTK2_HTTPTRANSACTION_H
#define INCLUDED_BLPWTK2_HTTPTRANSACTION_H

#include <blpwtk2_config.h>

namespace blpwtk2 {

class String;
class StringRef;

// This class represents an HTTP transaction, i.e. a single request/response
// pair.  This is passed to the HttpTransactionHandler, which the application
// can install at startup time.  The application can choose to handle the
// transaction, or let Chromium handle it.  See
// blpwtk2_httptransactionhandler.h for more information about this.
//
// Note: Transactions are handled on an internal IO thread.  Apart from
// notifyDataAvailable(), all methods on HttpTransaction can only be used from
// the IO thread (i.e. within the HttpTransactionHandler callbacks).
class HttpTransaction {
public:
    // Get the request url.
    virtual String url() const = 0;

    // GET, POST etc
    virtual String method() const = 0;

    // Replaces the current status line with the provided one (the specified
    // 'newStatus' should not have any EOL).  For example,
    // "HTTP/1.1 404 Not Found".  If this method is not called, the default
    // "HTTP/1.1 200 OK" is used.  The behavior is undefined if this is called
    // after the first 'readResponseBody' on the HttpTransactionHandler.
    virtual void replaceStatusLine(const StringRef& newStatus) = 0;

    // Adds a particular header to the response.  The specified 'header' has to
    // be a single header without any EOL termination, just
    // "<header-name>: <header-values>".  If a header with the same name is
    // already stored, the two headers are not merged together by this method;
    // the one provided is simply put at the end of the list.  The behavior is
    // undefined if this is called after the first 'readResponseBody' on the
    // HttpTransactionHandler.
    virtual void addResponseHeader(const StringRef& header) = 0;

    // The application's HttpTransactionHandler should call this whenever it
    // has data available.  This will cause readResponseBody to be called for
    // this transaction.  Unlike the other methods on this class, this method
    // can be called from any other thread.
    virtual void notifyDataAvailable() = 0;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_HTTPTRANSACTION_H

