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

#include <blpwtk2_managedrenderprocesshost.h>

#include <content/public/browser/browser_context.h>
#include <content/public/browser/render_process_host.h>

namespace blpwtk2 {

ManagedRenderProcessHost::ManagedRenderProcessHost(
    base::ProcessHandle processHandle,
    content::BrowserContext* browserContext)
: d_impl(content::RenderProcessHost::CreateProcessHost(processHandle, browserContext))
{
    d_impl->Init();
}

ManagedRenderProcessHost::~ManagedRenderProcessHost()
{
    DCHECK(0 == d_impl->NumListeners());
    d_impl->Cleanup();
    // don't delete d_impl.  Calling Cleanup() does a DeleteSoon.
}

int ManagedRenderProcessHost::id() const
{
    return d_impl->GetID();
}

const std::string& ManagedRenderProcessHost::channelId() const
{
    return d_impl->GetChannel()->ChannelId();
}

}  // close namespace blpwtk2

