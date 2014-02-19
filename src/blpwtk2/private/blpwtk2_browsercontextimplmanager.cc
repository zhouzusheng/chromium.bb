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
    Statics::browserContextImplManager = 0;

    typedef std::map<std::string, BrowserContextImpl*>::iterator Iterator;
    for (Iterator it = d_dataBrowserContexts.begin();
        it != d_dataBrowserContexts.end(); ++it) {
            delete it->second;
    }
    d_dataBrowserContexts.clear();

    for (size_t i = 0; i < d_incognitoBrowserContexts.size(); ++i) {
        delete d_incognitoBrowserContexts[i];
    }
    d_incognitoBrowserContexts.clear();
}

BrowserContextImpl* BrowserContextImplManager::obtainBrowserContextImpl(
    const std::string& dataDir,
    bool diskCacheEnabled)
{
    DCHECK(Statics::isInBrowserMainThread());

    BrowserContextImpl* result;

    if (!dataDir.empty()) {
        typedef std::map<std::string, BrowserContextImpl*>::iterator Iterator;
        Iterator it = d_dataBrowserContexts.find(dataDir);
        if (it == d_dataBrowserContexts.end()) {
            result = new BrowserContextImpl(dataDir, diskCacheEnabled);
            d_dataBrowserContexts[dataDir] = result;
        }
        else {
            result = it->second;
            DCHECK(diskCacheEnabled == result->diskCacheEnabled());
        }
    }
    else {
        result = new BrowserContextImpl(dataDir, diskCacheEnabled);
        d_incognitoBrowserContexts.push_back(result);
    }

    return result;
}

void BrowserContextImplManager::destroyBrowserContexts()
{
    DCHECK(Statics::isInBrowserMainThread());

    typedef std::map<std::string, BrowserContextImpl*>::iterator Iterator;
    for (Iterator it = d_dataBrowserContexts.begin();
                  it != d_dataBrowserContexts.end(); ++it) {
        it->second->reallyDestroy();
    }

    for (size_t i = 0; i < d_incognitoBrowserContexts.size(); ++i) {
        d_incognitoBrowserContexts[i]->reallyDestroy();
    }
}

}  // close namespace blpwtk2

