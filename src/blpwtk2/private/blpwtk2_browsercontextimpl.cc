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

#include <blpwtk2_browsercontextimpl.h>

#include <blpwtk2_resourcecontextimpl.h>

#include <base/files/file_path.h>
#include <base/file_util.h>
#include <base/logging.h>  // for CHECK
#include <base/path_service.h>
#include <base/threading/thread_restrictions.h>
#include <content/public/browser/browser_thread.h>
#include <content/public/browser/storage_partition.h>
#include <net/url_request/url_request_context_getter.h>

namespace blpwtk2 {

BrowserContextImpl::BrowserContextImpl()
{
    // allow IO during creation of browser context
    base::ThreadRestrictions::ScopedAllowIO allowIO;

    CHECK(PathService::Get(base::DIR_LOCAL_APP_DATA, &d_path));
    d_path = d_path.Append(std::wstring(L"blpwtk2"));
    if (!file_util::PathExists(d_path))
        file_util::CreateDirectory(d_path);
}

BrowserContextImpl::~BrowserContextImpl()
{
    if (d_resourceContext.get()) {
        content::BrowserThread::DeleteSoon(content::BrowserThread::IO,
                                           FROM_HERE,
                                           d_resourceContext.release());
    }
}

void BrowserContextImpl::setRequestContextGetter(
    net::URLRequestContextGetter* getter)
{
    d_requestContextGetter = getter;
}

net::URLRequestContextGetter* BrowserContextImpl::requestContextGetter() const
{
    return d_requestContextGetter.get();
}


// ======== content::BrowserContext implementation =============

base::FilePath BrowserContextImpl::GetPath()
{
    return d_path;
}

bool BrowserContextImpl::IsOffTheRecord() const
{
    return true;
}

net::URLRequestContextGetter* BrowserContextImpl::GetRequestContext()
{
    return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter*
BrowserContextImpl::GetRequestContextForRenderProcess(int rendererChildId)
{
    return GetRequestContext();
}

net::URLRequestContextGetter* BrowserContextImpl::GetMediaRequestContext()
{
    return GetRequestContext();
}

net::URLRequestContextGetter*
BrowserContextImpl::GetMediaRequestContextForRenderProcess(int rendererChildId)
{
    return GetRequestContext();
}

net::URLRequestContextGetter*
BrowserContextImpl::GetMediaRequestContextForStoragePartition(
    const base::FilePath& partitionPath,
    bool inMemory)
{
    return GetRequestContext();
}

content::ResourceContext* BrowserContextImpl::GetResourceContext()
{
	DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

	if (!d_resourceContext.get())
		d_resourceContext.reset(new ResourceContextImpl());
    return d_resourceContext.get();
}

content::DownloadManagerDelegate*
BrowserContextImpl::GetDownloadManagerDelegate()
{
    return 0;
}

content::GeolocationPermissionContext*
BrowserContextImpl::GetGeolocationPermissionContext()
{
    return 0;
}

content::SpeechRecognitionPreferences*
BrowserContextImpl::GetSpeechRecognitionPreferences()
{
    return 0;
}

quota::SpecialStoragePolicy* BrowserContextImpl::GetSpecialStoragePolicy()
{
    return 0;
}

}  // close namespace blpwtk2

