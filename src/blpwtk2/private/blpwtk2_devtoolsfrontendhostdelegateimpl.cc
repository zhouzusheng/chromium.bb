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

#include <base/json/json_reader.h>
#include <base/json/json_writer.h>
#include <base/strings/string_number_conversions.h>
#include <base/strings/utf_string_conversions.h>
#include <base/values.h>
#include <content/public/browser/devtools_agent_host.h>
#include <content/public/browser/render_frame_host.h>
#include <content/public/browser/web_contents.h>
#include <ipc/ipc_channel.h>

namespace blpwtk2 {

// This constant should be in sync with
// the constant at devtools_ui_bindings.cc.
// This was copied from shell_devtools_frontend.cc
const size_t kMaxMessageChunkSize = IPC::Channel::kMaximumMessageSize / 4;

DevToolsFrontendHostDelegateImpl::DevToolsFrontendHostDelegateImpl(
    content::WebContents* inspectorContents,
    const scoped_refptr<content::DevToolsAgentHost>& agentHost)
: WebContentsObserver(inspectorContents)
, d_agentHost(agentHost)
{
}

DevToolsFrontendHostDelegateImpl::~DevToolsFrontendHostDelegateImpl()
{
}

// ======== WebContentsObserver overrides ============

void DevToolsFrontendHostDelegateImpl::RenderViewCreated(
    content::RenderViewHost* renderViewHost)
{
    d_frontendHost.reset(content::DevToolsFrontendHost::Create(web_contents()->GetMainFrame(), this));
    d_agentHost->AttachClient(this);
}

void DevToolsFrontendHostDelegateImpl::WebContentsDestroyed()
{
    d_agentHost->DetachClient();
    d_agentHost = 0;
}

void DevToolsFrontendHostDelegateImpl::HandleMessageFromDevToolsFrontend(
    const std::string& message)
{
    // This implementation was copied from shell_devtools_frontend.cc

    std::string method;
    int id = 0;
    base::ListValue* params = NULL;
    base::DictionaryValue* dict = NULL;
    scoped_ptr<base::Value> parsed_message(base::JSONReader::Read(message));
    if (!parsed_message ||
        !parsed_message->GetAsDictionary(&dict) ||
        !dict->GetString("method", &method)) {
        return;
    }
    dict->GetList("params", &params);

    std::string browser_message;
    if (method == "sendMessageToBrowser" && params &&
        params->GetSize() == 1 && params->GetString(0, &browser_message)) {
        d_agentHost->DispatchProtocolMessage(browser_message);
    }
    else if (method == "loadCompleted") {
        web_contents()->GetMainFrame()->ExecuteJavaScript(
            base::ASCIIToUTF16("DevToolsAPI.setUseSoftMenu(true);"));
    }
    else {
        return;
    }

    dict->GetInteger("id", &id);
    if (id) {
        std::string code = "DevToolsAPI.embedderMessageAck(" +
            base::IntToString(id) + ",\"\");";
        base::string16 javascript = base::UTF8ToUTF16(code);
        web_contents()->GetMainFrame()->ExecuteJavaScript(javascript);
    }
}

void DevToolsFrontendHostDelegateImpl::HandleMessageFromDevToolsFrontendToBackend(
    const std::string& message)
{
    // This implementation was copied from shell_devtools_frontend.cc

    d_agentHost->DispatchProtocolMessage(message);
}

void DevToolsFrontendHostDelegateImpl::DispatchProtocolMessage(
        content::DevToolsAgentHost* agentHost,
        const std::string& message)
{
    // This implementation was copied from shell_devtools_frontend.cc

    if (message.length() < kMaxMessageChunkSize) {
        base::string16 javascript = base::UTF8ToUTF16(
            "DevToolsAPI.dispatchMessage(" + message + ");");
        web_contents()->GetMainFrame()->ExecuteJavaScript(javascript);
        return;
    }

    base::FundamentalValue total_size(static_cast<int>(message.length()));
    for (size_t pos = 0; pos < message.length(); pos += kMaxMessageChunkSize) {
        base::StringValue message_value(message.substr(pos, kMaxMessageChunkSize));
        std::string param;
        base::JSONWriter::Write(&message_value, &param);
        std::string code = "DevToolsAPI.dispatchMessageChunk(" + param + ");";
        base::string16 javascript = base::UTF8ToUTF16(code);
        web_contents()->GetMainFrame()->ExecuteJavaScript(javascript);
    }
}

void DevToolsFrontendHostDelegateImpl::AgentHostClosed(
        content::DevToolsAgentHost* agentHost,
        bool replacedWithAnotherClient)
{
    // TODO: notify blpwtk2 clients?
}

}  // close namespace blpwtk2

