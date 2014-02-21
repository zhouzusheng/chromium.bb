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

#include <blpwtk2_browsercontextimplmanager.h>
#include <blpwtk2_devtoolshttphandlerdelegateimpl.h>
#include <blpwtk2_processhostmanager.h>
#include <blpwtk2_statics.h>

#include <base/logging.h>  // for DCHECK
#include <base/message_loop.h>
#include <content/public/browser/browser_main_runner.h>

namespace blpwtk2 {

BrowserMainRunner::BrowserMainRunner(
    sandbox::SandboxInterfaceInfo* sandboxInfo)
: d_mainParams(*CommandLine::ForCurrentProcess())
{
    Statics::initBrowserMainThread();

    d_mainParams.sandbox_info = sandboxInfo;
    d_impl.reset(content::BrowserMainRunner::Create());
    int rc = d_impl->Initialize(d_mainParams);
    DCHECK(-1 == rc);  // it returns -1 for success!!

    // The MessageLoop is created by content::BrowserMainRunner (inside
    // content::BrowserMainLoop).
    Statics::browserMainMessageLoop = base::MessageLoop::current();

    d_browserContextImplManager.reset(new BrowserContextImplManager());

    d_devToolsHttpHandlerDelegate.reset(
        new DevToolsHttpHandlerDelegateImpl());

    Statics::processHostManager = new ProcessHostManager();
}

BrowserMainRunner::~BrowserMainRunner()
{
    DCHECK(!Statics::processHostManager);

    d_devToolsHttpHandlerDelegate.reset();
    Statics::browserMainMessageLoop = 0;

    // This deletes the BrowserContextImpl objects, and needs to happen after
    // the main message loop has finished, but before shutting down threads,
    // because the BrowserContext holds on to state that needs to be deleted on
    // those threads.  For example, it holds on to the URLRequestContextGetter
    // inside UserData (base class for content::BrowserContext).
    d_browserContextImplManager.reset();

    d_impl->Shutdown();
}

void BrowserMainRunner::destroyProcessHostManager()
{
    delete Statics::processHostManager;
    Statics::processHostManager = 0;
}

int BrowserMainRunner::run()
{
    return d_impl->Run();
}

}  // close namespace blpwtk2

