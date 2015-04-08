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

#include <blpwtk2_devtoolshttphandlerdelegateimpl.h>

#include <blpwtk2_statics.h>

#include <base/command_line.h>
#include <base/files/file_path.h>
#include <base/path_service.h>
#include <base/strings/string_number_conversions.h>
#include <base/strings/utf_string_conversions.h>
#include <content/public/browser/devtools_agent_host.h>
#include <content/public/browser/devtools_http_handler.h>
#include <content/public/browser/devtools_target.h>
#include <content/public/browser/favicon_status.h>
#include <content/public/browser/navigation_entry.h>
#include <content/public/browser/render_view_host.h>
#include <content/public/browser/web_contents.h>
#include <content/public/browser/web_contents_delegate.h>
#include <content/public/common/content_switches.h>
#include <net/socket/tcp_server_socket.h>

namespace blpwtk2 {

namespace {

// This class is copied from the content_shell implementation of
// DevToolsHttpHandlerDelegate.  We might customize stuff in here in
// the future.
class Target : public content::DevToolsTarget {
  public:
    explicit Target(scoped_refptr<content::DevToolsAgentHost> agentHost);

    std::string GetId() const override { return d_agentHost->GetId(); }
    std::string GetParentId() const override { return std::string(); }
    std::string GetType() const override { return "page"; }
    std::string GetTitle() const override { return d_agentHost->GetTitle(); }
    std::string GetDescription() const override { return std::string(); }
    GURL GetURL() const override { return d_agentHost->GetURL(); }
    GURL GetFaviconURL() const override { return d_faviconUrl; }
    base::TimeTicks GetLastActivityTime() const override {
        return d_lastActivityTime;
    }
    bool IsAttached() const override {
        return d_agentHost->IsAttached();
    }
    scoped_refptr<content::DevToolsAgentHost> GetAgentHost() const override {
        return d_agentHost;
    }
    bool Activate() const override;
    bool Close() const override;

private:
    scoped_refptr<content::DevToolsAgentHost> d_agentHost;
    GURL d_faviconUrl;
    base::TimeTicks d_lastActivityTime;
};

Target::Target(scoped_refptr<content::DevToolsAgentHost> agentHost)
: d_agentHost(agentHost)
{
    if (content::WebContents* webContents = d_agentHost->GetWebContents()) {
        content::NavigationController& controller = webContents->GetController();
        content::NavigationEntry* entry = controller.GetActiveEntry();
        if (entry != NULL && entry->GetURL().is_valid())
            d_faviconUrl = entry->GetFavicon().url;
        d_lastActivityTime = webContents->GetLastActiveTime();
    }
}

bool Target::Activate() const {
    return d_agentHost->Activate();
}

bool Target::Close() const {
    return d_agentHost->Close();
}

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

}  // close anonymous namespace


// DevToolsHttpHandlerDelegateImpl -----------------------------------------

DevToolsHttpHandlerDelegateImpl::DevToolsHttpHandlerDelegateImpl()
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

    Statics::devToolsHttpHandler = content::DevToolsHttpHandler::Start(
        scoped_ptr<content::DevToolsHttpHandler::ServerSocketFactory>(
            new TCPServerSocketFactory("127.0.0.1", port, 1)),
        "", this,
        base::FilePath());
}

DevToolsHttpHandlerDelegateImpl::~DevToolsHttpHandlerDelegateImpl()
{
    // Stop() will delete the object
    Statics::devToolsHttpHandler->Stop();
    Statics::devToolsHttpHandler = 0;
}


// DevToolsManagerDelegateImpl -----------------------------------------------
// TODO: move this to a separate file

DevToolsManagerDelegateImpl::DevToolsManagerDelegateImpl()
{
}

DevToolsManagerDelegateImpl::~DevToolsManagerDelegateImpl()
{
}

base::DictionaryValue* DevToolsManagerDelegateImpl::HandleCommand(
      content::DevToolsAgentHost* agentHost,
      base::DictionaryValue* command)
{
    return NULL;
}

scoped_ptr<content::DevToolsTarget>
DevToolsManagerDelegateImpl::CreateNewTarget(const GURL& url)
{
    // TODO(SHEZ): implement this
    NOTIMPLEMENTED();
    return scoped_ptr<content::DevToolsTarget>();
}

void DevToolsManagerDelegateImpl::EnumerateTargets(TargetCallback callback)
{
    // This is copied from the implementation in content_shell.

    TargetList targets;
    content::DevToolsAgentHost::List agents =
        content::DevToolsAgentHost::GetOrCreateAll();
    for (content::DevToolsAgentHost::List::iterator it = agents.begin();
         it != agents.end(); ++it) {
        targets.push_back(new Target(*it));
    }
    callback.Run(targets);
}

}  // close namespace blpwtk2
