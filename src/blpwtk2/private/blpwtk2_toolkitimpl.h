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

#include <blpwtk2_contentmaindelegateimpl.h>
#include <blpwtk2_rendererinfomap.h>
#include <blpwtk2_toolkit.h>

#include <base/memory/scoped_ptr.h>
#include <sandbox/win/src/sandbox_types.h>

#include <string>
#include <vector>

namespace base {
    class FilePath;
}  // close namespace base

namespace content {
    class ContentMainRunner;
}  // close namespace content

namespace blpwtk2 {

class BrowserThread;
class BrowserMainRunner;
class ManagedRenderProcessHost;
class ProcessClientImpl;
class ProcessHostImpl;
class Profile;
class StringRef;

// This is the implementation of the Toolkit.  This class is responsible for
// setting up the threads (based on the application's selection thread mode),
// see blpwtk_toolkit.h for an explanation about the threads.
// There is only ever one instance of ToolkitImpl.  It is created when
// ToolkitFactory::create is called, and it is deleted when Toolkit::destroy()
// is called.  Note that chromium threads are only started when the first
// WebView is created.
class ToolkitImpl : public Toolkit {
  public:
    static ToolkitImpl* instance();

    ToolkitImpl(const StringRef& dictionaryPath,
                const StringRef& hostChannel);
    virtual ~ToolkitImpl();

    void startupThreads();
    void shutdownThreads();

    void appendCommandLineSwitch(const char* switchString);

    // blpwtk2::Toolkit overrides
    Profile* createProfile(const ProfileCreateParams& params) override;
    bool hasDevTools() override;
    void destroy() override;
    WebView* createWebView(NativeView parent,
                           WebViewDelegate* delegate,
                           const WebViewCreateParams& params) override;
    String createHostChannel(int timeoutInMilliseconds) override;
    bool preHandleMessage(const NativeMsg* msg) override;
    void postHandleMessage(const NativeMsg* msg) override;
    void clearWebCache() override;
    void setTimerHiddenPageAlignmentInterval(double) override;

  private:
    void createInProcessHost();
    void destroyInProcessHost();

    bool d_threadsStarted;
    bool d_threadsStopped;
    Profile* d_defaultProfile;
    RendererInfoMap d_rendererInfoMap;
    RendererInfo d_inProcessRendererInfo;
    sandbox::SandboxInterfaceInfo d_sandboxInfo;
    ContentMainDelegateImpl d_mainDelegate;
    scoped_ptr<content::ContentMainRunner> d_mainRunner;
    std::string d_dictionaryPath;
    std::string d_hostChannel;

    // only used for the RENDERER_MAIN thread mode, if host channel is empty
    scoped_ptr<BrowserThread> d_browserThread;
    scoped_ptr<ProcessHostImpl> d_inProcessHost;

    // only used for the RENDERER_MAIN thread mode
    scoped_ptr<ProcessClientImpl> d_processClient;

    // only used for the ORIGINAL thread mode
    scoped_ptr<BrowserMainRunner> d_browserMainRunner;
    scoped_ptr<ManagedRenderProcessHost> d_inProcessRendererHost;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_TOOLKITIMPL_H

