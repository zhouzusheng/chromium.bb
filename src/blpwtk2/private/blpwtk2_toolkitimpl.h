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

#ifndef INCLUDED_BLPWTK2_TOOLKITIMPL_H
#define INCLUDED_BLPWTK2_TOOLKITIMPL_H

#include <blpwtk2_config.h>

#include <blpwtk2_threadmode.h>
#include <blpwtk2_contentmaindelegateimpl.h>

#include <base/memory/scoped_ptr.h>
#include <sandbox/win/src/sandbox_types.h>

#include <map>

namespace content {
    class ContentMainRunner;
}  // close namespace content

namespace blpwtk2 {

class CreateParams;
class WebView;
class WebViewDelegate;
class BrowserThread;
class BrowserContextImpl;
class BrowserMainRunner;

// This is the implementation of the Toolkit.  This class is responsible for
// setting up the threads (based on the application's selection thread mode),
// see blpwtk_toolkit.h for an explanation about the threads.
// There is only ever one instance of ToolkitImpl.  It is created when the
// first WebView is created, and it is deleted when Toolkit::shutdown() is
// called.
class ToolkitImpl {
  public:
    static ToolkitImpl* instance();

    ToolkitImpl();
    ~ToolkitImpl();

    WebView* createWebView(NativeView parent,
                           WebViewDelegate* delegate,
                           const CreateParams& params);

    void onRootWindowPositionChanged(gfx::NativeView root);
    void onRootWindowSettingChange(gfx::NativeView root);

    bool preHandleMessage(const NativeMsg* msg);
    void postHandleMessage(const NativeMsg* msg);

  private:
    typedef std::map<int,int> RendererToHostIdMap;

    sandbox::SandboxInterfaceInfo d_sandboxInfo;
    ContentMainDelegateImpl d_mainDelegate;
    scoped_ptr<content::ContentMainRunner> d_mainRunner;
    RendererToHostIdMap d_rendererToHostIdMap;

    // only used for the RENDERER_MAIN thread mode
    scoped_ptr<BrowserThread> d_browserThread;

    // only used for the ORIGINAL thread mode
    scoped_ptr<BrowserMainRunner> d_browserMainRunner;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_TOOLKITIMPL_H

