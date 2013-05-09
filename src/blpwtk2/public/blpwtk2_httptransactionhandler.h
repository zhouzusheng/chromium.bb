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

#ifndef INCLUDED_BLPWTK2_HTTPTRANSACTIONHANDLER_H
#define INCLUDED_BLPWTK2_HTTPTRANSACTIONHANDLER_H

#include <blpwtk2_config.h>

namespace blpwtk2 {

class HttpTransaction;

// A HttpTransaction is a single request/response pair.  Chromium has built-in
// mechanisms for handling HttpTransactions, but this class allows applications
// to provide custom handlers by implementing this interface and installing it
// on startup via blpwtk2::Toolkit::setHttpTransactionHandler().
//
// Note that all HTTP transactions are handled in an IO thread.  All methods in
// this interface are invoked from the IO thread.  The application *must*
// *not* post to the renderer thread while a transaction is in progress.  This
// is because, in some cases, the renderer makes *synchronous* requests for
// resources.  If the application posts back to the renderer thread during a
// synchronous request, it will deadlock!
class HttpTransactionHandler {
public:

    // Invoked whenever an HTTP transaction occurs.  Return true if this
    // handler will handle the specified 'transaction', return false to use
    // Chromium's default handler.  The application can store any data
    // pertaining to this transaction in the specified 'userData', which will
    // be provided back in subsequent callbacks for this transaction.
    //
    // If the application handles this transaction, it is expected to call
    // notifyDataAvailable() on the specified 'transaction' when it has data
    // available.  This will cause 'readResponseBody' to be called.
    virtual bool startTransaction(HttpTransaction* transaction,
                                  void** userData) = 0;

    // Populate the specified 'buffer' of the specified 'bufferLen' with as
    // much available data that will fit in the buffer.  Return the number of
    // bytes read, or a negative number if an error occurs, in which case the
    // transaction will be aborted.  Set the specified 'isCompleted' to 'true'
    // when there is no more content to read and the transaction is complete,
    // otherwise the application is expected to call notifyDataAvailable()
    // again when it has more data.  The behavior is undefined if 0 is returned
    // but 'isCompleted' is not set to 'true', because notifyDataAvailable()
    // should not have been called if there was no data available!
    //
    // Note that 'replaceStatusLine' and 'addResponseHeader' cannot be called
    // after the first 'readResponseBody' returns (the application must set the
    // status line and add all the necessary response headers before Chromium
    // starts consuming the response body).
    virtual int readResponseBody(HttpTransaction* transaction,
                                 void* userData,
                                 char* buffer,
                                 int bufferLen,
                                 bool* isCompleted) = 0;

    // Invoked when the specified 'transaction' has completed/aborted, allowing
    // the HttpTransactionHandler to perform any necessary cleanup.  Do not use
    // the specified 'transaction' object after this call.
    virtual void endTransaction(HttpTransaction* transaction,
                                void* userData) = 0;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_HTTPTRANSACTIONHANDLER_H

