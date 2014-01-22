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

#include <blpwtk2_devtoolsfrontendhostdelegateimpl.h>

#include <content/public/browser/devtools_agent_host.h>
#include <content/public/browser/devtools_client_host.h>
#include <content/public/browser/devtools_manager.h>
#include <content/public/browser/web_contents.h>

namespace blpwtk2 {

DevToolsFrontendHostDelegateImpl::DevToolsFrontendHostDelegateImpl(
    content::WebContents* inspectorContents,
    content::DevToolsAgentHost* agentHost)
: WebContentsObserver(inspectorContents)
, d_agentHost(agentHost)
{
    d_frontendHost.reset(
        content::DevToolsClientHost::CreateDevToolsFrontendHost(web_contents(),
                                                                this));
}

DevToolsFrontendHostDelegateImpl::~DevToolsFrontendHostDelegateImpl()
{
}

// ======== WebContentsObserver overrides ============

void DevToolsFrontendHostDelegateImpl::RenderViewCreated(
    content::RenderViewHost* renderViewHost)
{
    content::DevToolsClientHost::SetupDevToolsFrontendClient(
        web_contents()->GetRenderViewHost());
    content::DevToolsManager* manager = content::DevToolsManager::GetInstance();
    manager->RegisterDevToolsClientHostFor(d_agentHost.get(), d_frontendHost.get());
}

void DevToolsFrontendHostDelegateImpl::WebContentsDestroyed(
    content::WebContents* webContents)
{
    DCHECK(!web_contents());

    content::DevToolsManager* manager = content::DevToolsManager::GetInstance();
    manager->ClientHostClosing(d_frontendHost.get());
    d_frontendHost.reset();
    d_agentHost = 0;
}


}  // close namespace blpwtk2

