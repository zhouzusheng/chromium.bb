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

#include <blpwtk2_inprocessresourceloaderbridge.h>

#include <blpwtk2_resourceloader.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_string.h>

#include <base/bind.h>
#include <base/message_loop/message_loop.h>
#include <content/child/request_info.h>
#include <content/child/sync_load_response.h>
#include <content/public/child/request_peer.h>
#include <net/base/mime_sniffer.h>
#include <net/base/net_errors.h>
#include <net/http/http_response_headers.h>

namespace blpwtk2 {

InProcessResourceLoaderBridge::InProcessResourceLoaderBridge(
    const content::RequestInfo& requestInfo)
: d_url(requestInfo.url)
, d_peer(0)
, d_userData(0)
, d_totalTransferSize(0)
, d_started(false)
, d_waitingForCancelLoad(false)
, d_canceled(false)
, d_failed(false)
, d_finished(false)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(Statics::inProcessResourceLoader);
    DCHECK(Statics::inProcessResourceLoader->canHandleURL(d_url.spec()));
    d_responseHeaders = new net::HttpResponseHeaders("HTTP/1.1 200 OK\0\0");
}

InProcessResourceLoaderBridge::~InProcessResourceLoaderBridge()
{
}

// webkit_glue::ResourceLoaderBridge overrides

void InProcessResourceLoaderBridge::SetRequestBody(
    content::ResourceRequestBody* request_body)
{
}

bool InProcessResourceLoaderBridge::Start(content::RequestPeer* peer)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_started);
    DCHECK(!d_waitingForCancelLoad);
    DCHECK(!d_canceled);
    DCHECK(!d_failed);
    DCHECK(!d_finished);

    d_peer = peer;

    // Safe to use base::Unretained because this object will only get deleted
    // when d_peer->OnCompleteRequest is called.
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&InProcessResourceLoaderBridge::startLoad,
                   base::Unretained(this)));
    return true;
}

void InProcessResourceLoaderBridge::Cancel()
{
    DCHECK(Statics::isInApplicationMainThread());

    if (d_waitingForCancelLoad || d_canceled) {
        // Sometimes Cancel() gets called twice.  If we already got canceled,
        // then ignore any further calls to Cancel().
        return;
    }

    d_waitingForCancelLoad = true;
    base::MessageLoop::current()->PostTask(
        FROM_HERE,
        base::Bind(&InProcessResourceLoaderBridge::cancelLoad,
                   base::Unretained(this)));
}

void InProcessResourceLoaderBridge::SetDefersLoading(bool value)
{
}

void InProcessResourceLoaderBridge::DidChangePriority(
    net::RequestPriority new_priority,
    int intra_priority_value)
{
}

bool InProcessResourceLoaderBridge::AttachThreadedDataReceiver(
    blink::WebThreadedDataReceiver* threaded_data_receiver)
{
    return false;
}

void InProcessResourceLoaderBridge::SyncLoad(content::SyncLoadResponse* response)
{
    DLOG(ERROR) << "Synchronous requests not supported: url("
                << d_url << ")";
    response->error_code = net::ERR_FAILED;
}

// ResourceContext overrides

void InProcessResourceLoaderBridge::replaceStatusLine(
    const StringRef& newStatus)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_failed);
    DCHECK(d_responseHeaders.get());

    std::string str(newStatus.data(), newStatus.length());
    d_responseHeaders->ReplaceStatusLine(str);
}

void InProcessResourceLoaderBridge::addResponseHeader(const StringRef& header)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_failed);
    DCHECK(d_responseHeaders.get());

    std::string str(header.data(), header.length());
    d_responseHeaders->AddHeader(str);
}

void InProcessResourceLoaderBridge::addResponseData(const char* buffer,
                                                    int length)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_failed);

    if (0 != length) {
        d_totalTransferSize += length;
        ensureResponseHeadersSent(buffer, length);
        d_peer->OnReceivedData(buffer, length, length);
    }
}

void InProcessResourceLoaderBridge::failed()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_finished);
    d_failed = true;
}

void InProcessResourceLoaderBridge::finish()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_finished);
    d_finished = true;

    if (d_waitingForCancelLoad) {
        // Application finished before we could notify it that the resource
        // was canceled.  We should wait for 'cancelLoad()' to get called,
        // where we will destroy ourself.
        return;
    }

    ensureResponseHeadersSent(0, 0);

    int errorCode = d_failed ? net::ERR_FAILED
                             : d_canceled ? net::ERR_ABORTED
                                          : net::OK;

    // We will get deleted inside this callback.
    d_peer->OnCompletedRequest(errorCode, false, false, "",
                               base::TimeTicks::Now(), d_totalTransferSize);
}

void InProcessResourceLoaderBridge::startLoad()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!d_started);
    DCHECK(!d_canceled);

    if (d_waitingForCancelLoad) {
        // We got canceled before we even started the resource on the loader.
        // We should wait for 'cancelLoad()' to get called, where we will
        // destroy ourself.
        return;
    }

    d_started = true;
    Statics::inProcessResourceLoader->start(d_url.spec(), this, &d_userData);
}

void InProcessResourceLoaderBridge::cancelLoad()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(d_waitingForCancelLoad);

    if (!d_started || d_finished) {
        // Resource canceled before we could start it on the loader, or the
        // loader finished before we could notify it of cancellation.  We can
        // now safely destroy ourself.

        // We will get deleted inside this callback.
        d_peer->OnCompletedRequest(net::ERR_ABORTED,
                                   false,
                                   false,
                                   "",
                                   base::TimeTicks::Now(),
                                   d_totalTransferSize);
        return;
    }

    d_waitingForCancelLoad = false;
    d_canceled = true;
    Statics::inProcessResourceLoader->cancel(this, d_userData);
}

void InProcessResourceLoaderBridge::ensureResponseHeadersSent(
    const char* buffer,
    int length)
{
    DCHECK(Statics::isInApplicationMainThread());

    if (!d_responseHeaders.get()) {
        return;
    }

    content::ResourceResponseInfo responseInfo;
    responseInfo.headers = d_responseHeaders;
    responseInfo.content_length = d_responseHeaders->GetContentLength();
    d_responseHeaders->GetMimeTypeAndCharset(&responseInfo.mime_type,
        &responseInfo.charset);
    d_responseHeaders = 0;

    if (responseInfo.mime_type.empty() && length > 0) {
        net::SniffMimeType(buffer,
                           std::min(length, net::kMaxBytesToSniff),
                           d_url,
                           "",
                           &responseInfo.mime_type);
    }

    d_peer->OnReceivedResponse(responseInfo);
}

}  // close namespace blpwtk2

