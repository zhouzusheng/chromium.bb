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

#include <blpwtk2_profileimpl.h>
#include <blpwtk2_resourcecontextimpl.h>

#include <base/files/file_path.h>
#include <base/file_util.h>
#include <base/logging.h>  // for CHECK
#include <base/threading/thread_restrictions.h>
#include <content/public/browser/browser_thread.h>
#include <content/public/browser/storage_partition.h>
#include <net/url_request/url_request_context_getter.h>

namespace blpwtk2 {

// static
BrowserContextImpl* BrowserContextImpl::fromProfile(ProfileImpl* profile)
{
    DCHECK(profile);
    if (profile->browserContext()) {
        return static_cast<BrowserContextImpl*>(profile->browserContext());
    }
    return new BrowserContextImpl(profile);
}

BrowserContextImpl::BrowserContextImpl(ProfileImpl* profile)
: d_profile(profile)
{
    DCHECK(d_profile);
    DCHECK(!d_profile->browserContext());

    if (d_profile->dataDir()) {
        // allow IO during creation of data directory
        base::ThreadRestrictions::ScopedAllowIO allowIO;

        d_path = base::FilePath::FromUTF8Unsafe(d_profile->dataDir());
        if (!file_util::PathExists(d_path))
            file_util::CreateDirectory(d_path);
    }
    else {
        // It seems that even incognito browser contexts need to return a valid
        // data path.  Not providing one causes a DCHECK failure in
        // fileapi::DeviceMediaAsyncFileUtil::Create.
        // For now, just create a temporary directory for this.
        // TODO: see if we can remove this requirement.

        // allow IO during creation of temporary directory
        base::ThreadRestrictions::ScopedAllowIO allowIO;
        file_util::CreateNewTempDirectory(L"blpwtk2_", &d_path);
    }

    d_profile->initFromBrowserUIThread(
        this,
        content::BrowserThread::UnsafeGetMessageLoopForThread(content::BrowserThread::IO),
        content::BrowserThread::UnsafeGetMessageLoopForThread(content::BrowserThread::FILE));
}

BrowserContextImpl::~BrowserContextImpl()
{
    if (!d_profile->dataDir()) {
        // Delete the temporary directory that we created in the constructor.

        // allow IO during deletion of temporary directory
        base::ThreadRestrictions::ScopedAllowIO allowIO;
        DCHECK(file_util::PathExists(d_path));
        file_util::Delete(d_path, true);
    }

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

    if (d_resourceContext) {
        d_resourceContext->setRequestContextGetter(d_requestContextGetter);
    }
}

net::URLRequestContextGetter* BrowserContextImpl::requestContextGetter() const
{
    return d_requestContextGetter.get();
}

ProfileImpl* BrowserContextImpl::profile() const
{
    return d_profile;
}


// ======== content::BrowserContext implementation =============

base::FilePath BrowserContextImpl::GetPath()
{
    return d_path;
}

bool BrowserContextImpl::IsOffTheRecord() const
{
    return 0 == d_profile->dataDir();
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

	if (!d_resourceContext.get()) {
		d_resourceContext.reset(new ResourceContextImpl());
        d_resourceContext->setRequestContextGetter(d_requestContextGetter.get());
    }
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

