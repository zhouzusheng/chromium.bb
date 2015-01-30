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

#ifndef INCLUDED_BLPWTK2_DEVTOOLSFRONTENDHOSTDELEGATE_H
#define INCLUDED_BLPWTK2_DEVTOOLSFRONTENDHOSTDELEGATE_H

#include <blpwtk2_config.h>

#include <base/memory/scoped_ptr.h>
#include <content/public/browser/web_contents_observer.h>
#include <content/public/browser/devtools_client_host.h>
#include <content/public/browser/devtools_frontend_host.h>

namespace content {
    class DevToolsAgentHost;
}  // close namespace content

namespace blpwtk2 {


// TODO: document this
class DevToolsFrontendHostDelegateImpl
    : public content::WebContentsObserver,
      public content::DevToolsFrontendHost::Delegate,
      public content::DevToolsClientHost {
  public:
    DevToolsFrontendHostDelegateImpl(content::WebContents* inspectorContents,
                                     content::DevToolsAgentHost* agentHost);
    virtual ~DevToolsFrontendHostDelegateImpl();

    content::DevToolsAgentHost* agentHost() const { return d_agentHost.get(); }

    // ======== WebContentsObserver overrides ============

    virtual void RenderViewCreated(content::RenderViewHost* renderViewHost) OVERRIDE;
    virtual void WebContentsDestroyed() OVERRIDE;


    // ======== DevToolsFrontendHost::Delegate overrides ===========

    virtual void HandleMessageFromDevToolsFrontend(
        const std::string& message) OVERRIDE;
    virtual void HandleMessageFromDevToolsFrontendToBackend(
        const std::string& message) OVERRIDE;


    // ========= DevToolsClientHost overrides =========
    virtual void DispatchOnInspectorFrontend(const std::string& message) OVERRIDE;
    virtual void InspectedContentsClosing() OVERRIDE {}
    virtual void ReplacedWithAnotherClient() OVERRIDE {}

  private:
    scoped_refptr<content::DevToolsAgentHost> d_agentHost;
    scoped_ptr<content::DevToolsFrontendHost> d_frontendHost;
};


}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_DEVTOOLSFRONTENDHOSTDELEGATE_H

