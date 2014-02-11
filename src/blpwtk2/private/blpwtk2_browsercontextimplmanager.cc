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

#include <blpwtk2_browsercontextimplmanager.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_statics.h>

namespace blpwtk2 {

BrowserContextImplManager::BrowserContextImplManager()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!Statics::browserContextImplManager);
    Statics::browserContextImplManager = this;
}

BrowserContextImplManager::~BrowserContextImplManager()
{
    DCHECK(this == Statics::browserContextImplManager);
    DCHECK(d_browserContexts.empty());
    Statics::browserContextImplManager = 0;
}

BrowserContextImpl* BrowserContextImplManager::createBrowserContextImpl(
    const std::string& dataDir,
    bool diskCacheEnabled)
{
    DCHECK(Statics::isInBrowserMainThread());
    BrowserContextImpl* result = new BrowserContextImpl(dataDir,
                                                        diskCacheEnabled);
    d_browserContexts.push_back(result);
    return result;
}

void BrowserContextImplManager::deleteBrowserContexts()
{
    DCHECK(0 == Statics::browserMainMessageLoop);
    typedef std::list<BrowserContextImpl*>::iterator Iterator;
    for (Iterator it = d_browserContexts.begin();
            it != d_browserContexts.end(); ++it) {
        DCHECK((*it)->isDestroyed());
        delete (*it);
    }
    d_browserContexts.clear();
}

}  // close namespace blpwtk2

