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

#include <blpwtk2_httptransactionfactoryimpl.h>
#include <blpwtk2_httptransactionimpl.h>

#include <net/base/net_errors.h>

namespace blpwtk2 {

HttpTransactionFactoryImpl::HttpTransactionFactoryImpl(net::HttpTransactionFactory* fallbackFactory)
: d_fallbackFactory(fallbackFactory)
{
    DCHECK(d_fallbackFactory.get());
}

int HttpTransactionFactoryImpl::CreateTransaction(net::RequestPriority priority,
                                                  scoped_ptr<net::HttpTransaction>* trans,
                                                  net::HttpTransactionDelegate* delegate)
{
    trans->reset(new HttpTransactionImpl(d_fallbackFactory.get(), delegate));
    return net::OK;
}

net::HttpCache* HttpTransactionFactoryImpl::GetCache()
{
    return d_fallbackFactory->GetCache();
}

net::HttpNetworkSession* HttpTransactionFactoryImpl::GetSession()
{
    return d_fallbackFactory->GetSession();
}

}  // close namespace blpwtk2

