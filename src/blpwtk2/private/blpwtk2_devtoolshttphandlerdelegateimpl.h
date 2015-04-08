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
    std::string GetDiscoveryPageHTML() override { return std::string(); }
    bool BundlesFrontendResources() override { return true; }
    base::FilePath GetDebugFrontendDir() override { return base::FilePath(); }
    scoped_ptr<net::ServerSocket> CreateSocketForTethering(
        std::string* name) override { return scoped_ptr<net::ServerSocket>(); }
};

// TODO: document this
// TODO: move this to a separate file
class DevToolsManagerDelegateImpl : public content::DevToolsManagerDelegate {
  public:
  DevToolsManagerDelegateImpl();
  virtual ~DevToolsManagerDelegateImpl();

  // DevToolsManagerDelegate implementation.
  void Inspect(content::BrowserContext* browserContext,
               content::DevToolsAgentHost* agentHost) override {}
  void DevToolsAgentStateChanged(content::DevToolsAgentHost* agentHost,
                                         bool attached) override {}
  base::DictionaryValue* HandleCommand(
      content::DevToolsAgentHost* agentHost,
      base::DictionaryValue* command) override;
  scoped_ptr<content::DevToolsTarget> CreateNewTarget(const GURL& url) override;
  void EnumerateTargets(TargetCallback callback) override;
  std::string GetPageThumbnailData(const GURL& url) override { return std::string(); }

 private:
  DISALLOW_COPY_AND_ASSIGN(DevToolsManagerDelegateImpl);
};


}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_DEVTOOLSHTTPHANDLERDELEGATEIMPL_H

