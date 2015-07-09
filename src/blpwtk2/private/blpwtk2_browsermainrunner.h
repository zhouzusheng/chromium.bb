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

#ifndef INCLUDED_BLPWTK2_BROWSERMAINRUNNER_H
#define INCLUDED_BLPWTK2_BROWSERMAINRUNNER_H

#include <blpwtk2_config.h>

#include <base/memory/scoped_ptr.h>
#include <content/public/common/main_function_params.h>

namespace content {
    class BrowserMainRunner;
    class BrowserContext;
}  // close namespace content

namespace devtools_http_handler {
    class DevToolsHttpHandler;
}  // close namespace devtools_http_handler

namespace sandbox {
    struct SandboxInterfaceInfo;
}  // close namespace sandbox

namespace blpwtk2 {

class BrowserContextImplManager;
class ManagedRenderProcessHost;
class ProcessHostManager;
class RendererInfoMap;
class ViewsDelegateImpl;

// This class represents the browser-main-thread's "main".  See
// blpwtk2_toolkit.h for an explanation about the threads.
//
// When we are using 'ThreadMode::ORIGINAL', this object is instantiated in the
// application's main thread.  If we are using 'ThreadMode::RENDERER_MAIN',
// this object is instantiated, Run'd, and destroyed in a secondary thread (see
// 'blpwtk2_browserthread.h' where this is done).
class BrowserMainRunner {
  public:
    explicit BrowserMainRunner(sandbox::SandboxInterfaceInfo* sandboxInfo);
    ~BrowserMainRunner();

    void destroyProcessHostManager();
    int run();

  private:
    content::MainFunctionParams d_mainParams;
    scoped_ptr<content::BrowserMainRunner> d_impl;
    scoped_ptr<BrowserContextImplManager> d_browserContextImplManager;
    scoped_ptr<devtools_http_handler::DevToolsHttpHandler> d_devtoolsHttpHandler;
    scoped_ptr<ViewsDelegateImpl> d_viewsDelegate;

    DISALLOW_COPY_AND_ASSIGN(BrowserMainRunner);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_BROWSERMAINRUNNER_H

