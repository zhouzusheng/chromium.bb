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

#include <blpwtk2_browsermainrunner.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_devtoolshttphandlerdelegateimpl.h>
#include <blpwtk2_inprocessrendererhost.h>
#include <blpwtk2_statics.h>

#include <base/logging.h>  // for DCHECK
#include <content/public/browser/browser_main_runner.h>

namespace blpwtk2 {

BrowserMainRunner::BrowserMainRunner(sandbox::SandboxInterfaceInfo* sandboxInfo)
: d_mainParams(*CommandLine::ForCurrentProcess())
{
    Statics::initBrowserMainThread();

    d_mainParams.sandbox_info = sandboxInfo;
    d_impl.reset(content::BrowserMainRunner::Create());
    int rc = d_impl->Initialize(d_mainParams);
    DCHECK(-1 == rc);  // it returns -1 for success!!

    d_browserContext.reset(new BrowserContextImpl());

    d_devToolsHttpHandlerDelegate.reset(
        new DevToolsHttpHandlerDelegateImpl());
}

BrowserMainRunner::~BrowserMainRunner()
{
    d_devToolsHttpHandlerDelegate.reset();
    d_inProcessRendererHost.reset();
    d_browserContext.reset();
    d_impl->Shutdown();
}

int BrowserMainRunner::Run()
{
    return d_impl->Run();
}

InProcessRendererHost* BrowserMainRunner::createInProcessRendererHost()
{
    DCHECK(Statics::isInBrowserMainThread());
    if (!d_inProcessRendererHost.get()) {
        d_inProcessRendererHost.reset(
            new InProcessRendererHost(d_browserContext.get()));
    }
    return d_inProcessRendererHost.get();
}

InProcessRendererHost* BrowserMainRunner::inProcessRendererHost() const
{
    return d_inProcessRendererHost.get();  // can return 0
}

content::BrowserContext* BrowserMainRunner::browserContext() const
{
    return d_browserContext.get();
}

}  // close namespace blpwtk2

