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

#ifndef INCLUDED_BLPWTK2_RESOURCECONTEXTIMPL_H
#define INCLUDED_BLPWTK2_RESOURCECONTEXTIMPL_H

#include <blpwtk2_config.h>

#include <base/memory/scoped_ptr.h>
#include <content/public/browser/resource_context.h>

namespace blpwtk2 {

// An instance of this class is owned by each BrowserContext.  Per
// content::ResourceContext, it is constructed in the UI thread, but is used
// and must be destructed in the IO thread.
class ResourceContextImpl : public content::ResourceContext {
  public:
    ResourceContextImpl();
    virtual ~ResourceContextImpl();

    virtual net::HostResolver* GetHostResolver() OVERRIDE;

    // DEPRECATED: This is no longer a valid given isolated apps/sites and
    // storage partitioning. This getter returns the default context associated
    // with a BrowsingContext.
    virtual net::URLRequestContext* GetRequestContext() OVERRIDE;

  private:
    scoped_ptr<net::HostResolver> d_hostResolver;

    DISALLOW_COPY_AND_ASSIGN(ResourceContextImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_RESOURCECONTEXTIMPL_H


