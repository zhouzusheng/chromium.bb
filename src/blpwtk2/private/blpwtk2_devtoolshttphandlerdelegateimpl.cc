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
#include <net/socket/tcp_listen_socket.h>

namespace blpwtk2 {

namespace {

// This class is copied from the content_shell implementation of
// DevToolsHttpHandlerDelegate.  We might customize stuff in here in
// the future.
class Target : public content::DevToolsTarget {
  public:
    explicit Target(content::WebContents* webContents);

    virtual std::string GetId() const OVERRIDE{ return d_id; }
    virtual std::string GetType() const OVERRIDE{ return "page"; }
    virtual std::string GetTitle() const OVERRIDE{ return d_title; }
    virtual std::string GetDescription() const OVERRIDE{ return std::string(); }
    virtual GURL GetUrl() const OVERRIDE{ return d_url; }
    virtual GURL GetFaviconUrl() const OVERRIDE{ return d_faviconUrl; }
    virtual base::TimeTicks GetLastActivityTime() const OVERRIDE{
        return d_lastActivityTime;
    }
    virtual bool IsAttached() const OVERRIDE{
        return d_agentHost->IsAttached();
    }
    virtual scoped_refptr<content::DevToolsAgentHost> GetAgentHost() const OVERRIDE{
        return d_agentHost;
    }
    virtual bool Activate() const OVERRIDE;
    virtual bool Close() const OVERRIDE;

private:
    scoped_refptr<content::DevToolsAgentHost> d_agentHost;
    std::string d_id;
    std::string d_title;
    GURL d_url;
    GURL d_faviconUrl;
    base::TimeTicks d_lastActivityTime;
};

Target::Target(content::WebContents* webContents) {
    d_agentHost =
        content::DevToolsAgentHost::GetOrCreateFor(webContents->GetRenderViewHost());
    d_id = d_agentHost->GetId();
    d_title = base::UTF16ToUTF8(webContents->GetTitle());
    d_url = webContents->GetURL();
    content::NavigationController& controller = webContents->GetController();
    content::NavigationEntry* entry = controller.GetActiveEntry();
    if (entry != NULL && entry->GetURL().is_valid())
        d_faviconUrl = entry->GetFavicon().url;
    d_lastActivityTime = webContents->GetLastActiveTime();
}

bool Target::Activate() const {
    content::RenderViewHost* rvh = d_agentHost->GetRenderViewHost();
    if (!rvh)
        return false;
    content::WebContents* webContents = content::WebContents::FromRenderViewHost(rvh);
    if (!webContents)
        return false;
    webContents->GetDelegate()->ActivateContents(webContents);
    return true;
}

bool Target::Close() const {
    content::RenderViewHost* rvh = d_agentHost->GetRenderViewHost();
    if (!rvh)
        return false;
    rvh->ClosePage();
    return true;
}

}  // close anonymous namespace

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
        new net::TCPListenSocketFactory("127.0.0.1", port), "", this);
}

DevToolsHttpHandlerDelegateImpl::~DevToolsHttpHandlerDelegateImpl()
{
    // Stop() will delete the object
    Statics::devToolsHttpHandler->Stop();
    Statics::devToolsHttpHandler = 0;
}

scoped_ptr<content::DevToolsTarget>
DevToolsHttpHandlerDelegateImpl::CreateNewTarget(const GURL& url)
{
    // TODO(SHEZ): implement this
    NOTIMPLEMENTED();
    return scoped_ptr<content::DevToolsTarget>();
}

void DevToolsHttpHandlerDelegateImpl::EnumerateTargets(TargetCallback callback)
{
    // This is copied from the implementation in content_shell.

    TargetList targets;
    std::vector<content::RenderViewHost*> rvhList =
        content::DevToolsAgentHost::GetValidRenderViewHosts();
    for (std::vector<content::RenderViewHost*>::iterator it = rvhList.begin();
                                                         it != rvhList.end(); ++it) {
        content::WebContents* webContents = content::WebContents::FromRenderViewHost(*it);
        if (webContents)
            targets.push_back(new Target(webContents));
    }
    callback.Run(targets);
}

}  // close namespace blpwtk2
