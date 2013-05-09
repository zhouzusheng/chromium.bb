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

#ifndef INCLUDED_BLPWTK2_INPROCESSRENDERERHOST_H
#define INCLUDED_BLPWTK2_INPROCESSRENDERERHOST_H

#include <blpwtk2_config.h>

#include <base/memory/scoped_ptr.h>

namespace content {
    class BrowserContext;
    class RenderProcessHostImpl;  // TODO: remove dependency on the impl class
}  // close namespace content

namespace blpwtk2 {

// This class sets up the "browser" side of the InProcessRenderer.  It opens a
// dummy channel, which is used to send messages to the InProcessRenderer.
// This object must be created in the browser's main thread before creating the
// InProcessRenderer on the renderer thread.
class InProcessRendererHost {
  public:
    explicit InProcessRendererHost(content::BrowserContext* browserContext);
    ~InProcessRendererHost();

    int id() const;

  private:
    scoped_ptr<content::RenderProcessHostImpl> d_impl;

    DISALLOW_COPY_AND_ASSIGN(InProcessRendererHost);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_INPROCESSRENDERERHOST_H


