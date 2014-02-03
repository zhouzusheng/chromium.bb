/*
 * Copyright (C) 2014 Bloomberg Finance L.P.
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

#ifndef INCLUDED_BLPWTK2_BROWSERCONTEXTIMPLMANAGER_H
#define INCLUDED_BLPWTK2_BROWSERCONTEXTIMPLMANAGER_H

#include <blpwtk2_config.h>

#include <base/basictypes.h>

#include <list>
#include <string>

namespace blpwtk2 {

class BrowserContextImpl;

// This object is responsible for creating BrowserContextImpl and keeping them
// alive until 'deleteBrowserContexts' is called.  This is necessary because
// there are objects that have pointers to the BrowserContext, but are not yet
// cleaned up when BrowserContextImpl is destroyed.  An example of this is the
// in-process renderer host.  Once we find all these references to
// BrowserContext, and ensure that they are cleaned up properly, then we can
// safely delete the BrowserContextImpl inside its 'destroy()' method, and we
// can get rid of this manager class.
class BrowserContextImplManager {
  public:
    BrowserContextImplManager();
    ~BrowserContextImplManager();

    // Create a BrowserContextImpl.  The returned object will be kept alive
    // until 'deleteBrowserContexts' is called.  This can only be called on the
    // browser-main thread.
    BrowserContextImpl* createBrowserContextImpl(const std::string& dataDir,
                                                 bool diskCacheEnabled);

    // Delete all the browser contexts.  This invalidates all the pointers that
    // had previously been returned from 'createBrowserContextImpl'.  This can
    // only be called on the browser-main thread.
    void deleteBrowserContexts();

  private:
    std::list<BrowserContextImpl*> d_browserContexts;

    DISALLOW_COPY_AND_ASSIGN(BrowserContextImplManager);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_BROWSERCONTEXTIMPLMANAGER_H

