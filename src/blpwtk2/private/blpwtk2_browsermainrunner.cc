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
#include <blpwtk2_viewsdelegateimpl.h>

#include <base/logging.h>  // for DCHECK
#include <base/message_loop/message_loop.h>
#include <base/strings/string_number_conversions.h>
#include <chrome/browser/printing/print_job_manager.h>
#include <content/public/browser/browser_main_runner.h>
#include <content/public/browser/devtools_http_handler.h>
#include <content/public/common/content_switches.h>
#include <net/socket/tcp_server_socket.h>
#include <ui/gfx/screen.h>
#include <ui/views/widget/desktop_aura/desktop_screen.h>

namespace {

class TCPServerSocketFactory
    : public content::DevToolsHttpHandler::ServerSocketFactory {
 public:
  TCPServerSocketFactory(const std::string& address, int port, int backlog)
      : content::DevToolsHttpHandler::ServerSocketFactory(
            address, port, backlog) {}

 private:
  // content::DevToolsHttpHandler::ServerSocketFactory.
  scoped_ptr<net::ServerSocket> Create() const override {
    return scoped_ptr<net::ServerSocket>(
        new net::TCPServerSocket(NULL, net::NetLog::Source()));
  }

  DISALLOW_COPY_AND_ASSIGN(TCPServerSocketFactory);
};

int getRemoteDebuggingPort()
{
    int port = 0;
    const base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
    if (command_line.HasSwitch(switches::kRemoteDebuggingPort)) {
        int temp_port;
        std::string port_str =
            command_line.GetSwitchValueASCII(switches::kRemoteDebuggingPort);
        if (base::StringToInt(port_str, &temp_port) &&
            temp_port > 0 && temp_port < 65535) {
                port = temp_port;
        } else {
            DLOG(WARNING) << "Invalid http debugger port number " << temp_port;
        }
    }
    return port;
}

}  // close anonymous namespace

namespace printing {
    extern PrintJobManager* g_print_job_manager;
}

namespace blpwtk2 {

BrowserMainRunner::BrowserMainRunner(
    sandbox::SandboxInterfaceInfo* sandboxInfo)
: d_mainParams(*base::CommandLine::ForCurrentProcess())
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

    d_devtoolsHttpHandler.reset(
        content::DevToolsHttpHandler::Start(
            scoped_ptr<content::DevToolsHttpHandler::ServerSocketFactory>(
                new TCPServerSocketFactory("127.0.0.1", getRemoteDebuggingPort(), 1)),
            "",
            new DevToolsHttpHandlerDelegateImpl(),  // the DevToolsHttpHandler takes ownership of this
            base::FilePath()));
    Statics::devToolsHttpHandler = d_devtoolsHttpHandler.get();

    gfx::Screen::SetScreenInstance(
        gfx::SCREEN_TYPE_NATIVE, views::CreateDesktopScreen());

    d_viewsDelegate.reset(
        new ViewsDelegateImpl());

    printing::g_print_job_manager = new printing::PrintJobManager();

    Statics::processHostManager = new ProcessHostManager();
}

BrowserMainRunner::~BrowserMainRunner()
{
    DCHECK(!Statics::processHostManager);

    delete printing::g_print_job_manager;
    Statics::devToolsHttpHandler = 0;
    d_devtoolsHttpHandler.reset();
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

