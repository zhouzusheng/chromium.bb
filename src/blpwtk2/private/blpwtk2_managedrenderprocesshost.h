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

#ifndef INCLUDED_BLPWTK2_MANAGEDRENDERPROCESSHOST_H
#define INCLUDED_BLPWTK2_MANAGEDRENDERPROCESSHOST_H

#include <blpwtk2_config.h>

#include <base/process/process_handle.h>

#include <string>

namespace content {
class BrowserContext;
class RenderProcessHost;
}  // close namespace content

namespace blpwtk2 {

// This class sets up a RenderProcessHost for an "externally-managed" renderer
// process.  Normally RenderProcessHosts are created and destroyed
// automatically by chromium.  The ManagedRenderProcessHost allows us to create
// a RenderProcessHost and keep it alive until the ManagedRenderProcessHost is
// destroyed.
// The process handle provided at compile time must be the handle for the
// process where the renderer is running.  It can be the current process handle
// (for in-process renderers).
class ManagedRenderProcessHost {
  public:
    ManagedRenderProcessHost(base::ProcessHandle processHandle,
                             content::BrowserContext* browserContext);
    ~ManagedRenderProcessHost();

    int id() const;
    const std::string& channelId() const;

  private:
    content::RenderProcessHost* d_impl;

    DISALLOW_COPY_AND_ASSIGN(ManagedRenderProcessHost);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_MANAGEDRENDERPROCESSHOST_H


