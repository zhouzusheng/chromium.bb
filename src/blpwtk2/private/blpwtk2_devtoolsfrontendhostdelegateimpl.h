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
#include <content/public/browser/devtools_frontend_host_delegate.h>

namespace content {
    class DevToolsAgentHost;
    class DevToolsClientHost;
}  // close namespace content

namespace blpwtk2 {


// TODO: document this
class DevToolsFrontendHostDelegateImpl
    : public content::WebContentsObserver,
        public content::DevToolsFrontendHostDelegate {
  public:
    DevToolsFrontendHostDelegateImpl(content::WebContents* inspectorContents,
                                     content::DevToolsAgentHost* agentHost);
    virtual ~DevToolsFrontendHostDelegateImpl();

    content::DevToolsAgentHost* agentHost() const { return d_agentHost.get(); }

    // ======== WebContentsObserver overrides ============

    virtual void RenderViewCreated(content::RenderViewHost* renderViewHost) OVERRIDE;
    virtual void WebContentsDestroyed(content::WebContents* webContents) OVERRIDE;


    // ======== DevToolsFrontendHostDelegate overrides ===========

    virtual void ActivateWindow() OVERRIDE {}
    virtual void ChangeAttachedWindowHeight(unsigned height) OVERRIDE {}
    virtual void CloseWindow() OVERRIDE {}
    virtual void MoveWindow(int x, int y) OVERRIDE {}
    virtual void SetDockSide(const std::string& side) OVERRIDE {}
    virtual void OpenInNewTab(const std::string& url) OVERRIDE {}
    virtual void SaveToFile(const std::string& url,
                            const std::string& content,
                            bool save_as) OVERRIDE {}
    virtual void AppendToFile(const std::string& url,
                              const std::string& content) OVERRIDE {}
    virtual void RequestFileSystems() OVERRIDE {}
    virtual void AddFileSystem() OVERRIDE {}
    virtual void RemoveFileSystem(const std::string& file_system_path) OVERRIDE {}
    virtual void IndexPath(int request_id,
                           const std::string& file_system_path) OVERRIDE {}
    virtual void StopIndexing(int request_id) OVERRIDE {}
    virtual void SearchInPath(int request_id,
                              const std::string& file_system_path,
                              const std::string& query) OVERRIDE {}
    virtual void InspectedContentsClosing() OVERRIDE {}

  private:
    scoped_refptr<content::DevToolsAgentHost> d_agentHost;
    scoped_ptr<content::DevToolsClientHost> d_frontendHost;

};


}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_DEVTOOLSFRONTENDHOSTDELEGATE_H

