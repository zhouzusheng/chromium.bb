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

#ifndef INCLUDED_BLPWTK2_DEVTOOLSHTTPHANDLERDELEGATEIMPL_H
#define INCLUDED_BLPWTK2_DEVTOOLSHTTPHANDLERDELEGATEIMPL_H

#include <blpwtk2_config.h>

#include <content/public/browser/devtools_http_handler_delegate.h>
#include <content/public/browser/devtools_manager_delegate.h>

namespace blpwtk2 {

// TODO: document this
class DevToolsHttpHandlerDelegateImpl
    : public content::DevToolsHttpHandlerDelegate {
  public:
    DevToolsHttpHandlerDelegateImpl();
    virtual ~DevToolsHttpHandlerDelegateImpl();

    // ====== DevToolsHttpHandlerDelegate overrides =======
    virtual std::string GetDiscoveryPageHTML() OVERRIDE { return std::string(); }
    virtual bool BundlesFrontendResources() OVERRIDE { return true; }
    virtual base::FilePath GetDebugFrontendDir() OVERRIDE { return base::FilePath(); }
    virtual scoped_ptr<net::StreamListenSocket> CreateSocketForTethering(
        net::StreamListenSocket::Delegate* delegate,
        std::string* name) OVERRIDE { return scoped_ptr<net::StreamListenSocket>(); }
};

// TODO: document this
// TODO: move this to a separate file
class DevToolsManagerDelegateImpl : public content::DevToolsManagerDelegate {
  public:
  DevToolsManagerDelegateImpl();
  virtual ~DevToolsManagerDelegateImpl();

  // DevToolsManagerDelegate implementation.
  virtual void Inspect(content::BrowserContext* browserContext,
                       content::DevToolsAgentHost* agentHost) OVERRIDE {}
  virtual void DevToolsAgentStateChanged(content::DevToolsAgentHost* agentHost,
                                         bool attached) OVERRIDE {}
  virtual base::DictionaryValue* HandleCommand(
      content::DevToolsAgentHost* agentHost,
      base::DictionaryValue* command) OVERRIDE;
  virtual scoped_ptr<content::DevToolsTarget> CreateNewTarget(const GURL& url) OVERRIDE;
  virtual void EnumerateTargets(TargetCallback callback) OVERRIDE;
  virtual std::string GetPageThumbnailData(const GURL& url) { return std::string(); }

 private:
  DISALLOW_COPY_AND_ASSIGN(DevToolsManagerDelegateImpl);
};


}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_DEVTOOLSHTTPHANDLERDELEGATEIMPL_H

