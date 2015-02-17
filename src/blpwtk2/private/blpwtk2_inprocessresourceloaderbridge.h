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

#ifndef INCLUDED_BLPWTK2_INPROCESSRESOURCELOADERBRIDGE_H
#define INCLUDED_BLPWTK2_INPROCESSRESOURCELOADERBRIDGE_H

#include <blpwtk2_config.h>
#include <base/memory/ref_counted.h>
#include <content/child/resource_loader_bridge.h>

namespace content {
struct RequestInfo;
}  // close namespace content

namespace blpwtk2 {

class InProcessResourceLoaderBridge
    : public content::ResourceLoaderBridge {
  public:
    InProcessResourceLoaderBridge(
        const content::RequestInfo& requestInfo);
    virtual ~InProcessResourceLoaderBridge();

    // webkit_glue::ResourceLoaderBridge overrides
    virtual void SetRequestBody(
        content::ResourceRequestBody* request_body) OVERRIDE;
    virtual bool Start(content::RequestPeer* peer) OVERRIDE;
    virtual void Cancel() OVERRIDE;
    virtual void SetDefersLoading(bool value) OVERRIDE;
    virtual void DidChangePriority(net::RequestPriority new_priority,
                                   int intra_priority_value) OVERRIDE;
    virtual bool AttachThreadedDataReceiver(
        blink::WebThreadedDataReceiver* threaded_data_receiver) OVERRIDE;
    virtual void SyncLoad(content::SyncLoadResponse* response) OVERRIDE;

  private:
    class InProcessResourceContext;
    scoped_refptr<InProcessResourceContext> d_context;

    DISALLOW_COPY_AND_ASSIGN(InProcessResourceLoaderBridge);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_INPROCESSRESOURCELOADERBRIDGE_H

