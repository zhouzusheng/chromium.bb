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
#include <content/public/browser/browser_context.h>
#include <content/public/browser/browser_thread.h>
#include <content/public/browser/devtools_agent_host.h>
#include <content/public/browser/render_frame_host.h>
#include <content/public/browser/web_contents.h>
#include <ipc/ipc_channel.h>
#include <net/base/io_buffer.h>
#include <net/base/net_errors.h>
#include <net/http/http_response_headers.h>
#include <net/url_request/url_fetcher.h>
#include <net/url_request/url_fetcher_response_writer.h>

namespace blpwtk2 {

namespace {

// ResponseWriter -------------------------------------------------------------
// This whole thing was copied from shell_devtools_frontend.cc

class ResponseWriter : public net::URLFetcherResponseWriter {
 public:
  ResponseWriter(base::WeakPtr<DevToolsFrontendHostDelegateImpl> shell_devtools_,
                 int stream_id);
  ~ResponseWriter() override;

  // URLFetcherResponseWriter overrides:
  int Initialize(const net::CompletionCallback& callback) override;
  int Write(net::IOBuffer* buffer,
            int num_bytes,
            const net::CompletionCallback& callback) override;
  int Finish(const net::CompletionCallback& callback) override;

 private:
  base::WeakPtr<DevToolsFrontendHostDelegateImpl> shell_devtools_;
  int stream_id_;

  DISALLOW_COPY_AND_ASSIGN(ResponseWriter);
};

ResponseWriter::ResponseWriter(
    base::WeakPtr<DevToolsFrontendHostDelegateImpl> shell_devtools,
    int stream_id)
    : shell_devtools_(shell_devtools),
      stream_id_(stream_id) {
}

ResponseWriter::~ResponseWriter() {
}

int ResponseWriter::Initialize(const net::CompletionCallback& callback) {
  return net::OK;
}

int ResponseWriter::Write(net::IOBuffer* buffer,
                          int num_bytes,
                          const net::CompletionCallback& callback) {
  base::FundamentalValue* id = new base::FundamentalValue(stream_id_);
  base::StringValue* chunk =
      new base::StringValue(std::string(buffer->data(), num_bytes));

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&DevToolsFrontendHostDelegateImpl::CallClientFunction,
                 shell_devtools_, "DevToolsAPI.streamWrite",
                 base::Owned(id), base::Owned(chunk), nullptr));
  return num_bytes;
}

int ResponseWriter::Finish(const net::CompletionCallback& callback) {
  return net::OK;
}

}  // namespace

// This constant should be in sync with
// the constant at devtools_ui_bindings.cc.
// This was copied from shell_devtools_frontend.cc
const size_t kMaxMessageChunkSize = IPC::Channel::kMaximumMessageSize / 4;

DevToolsFrontendHostDelegateImpl::DevToolsFrontendHostDelegateImpl(
    content::WebContents* inspectorContents,
    content::WebContents* inspectedContents)
: WebContentsObserver(inspectorContents)
, d_inspectedContents(inspectedContents)
, d_weakFactory(this)
, d_inspectElementPointPending(false)
{
}

DevToolsFrontendHostDelegateImpl::~DevToolsFrontendHostDelegateImpl()
{
    for (const auto& pair : d_pendingRequests)
        delete pair.first;
}

void DevToolsFrontendHostDelegateImpl::inspectElementAt(const POINT& point)
{
    if (d_agentHost) {
        d_agentHost->InspectElement(point.x, point.y);
        return;
    }
    d_inspectElementPointPending = true;
    d_inspectElementPoint = point;
}

// ======== WebContentsObserver overrides ============

void DevToolsFrontendHostDelegateImpl::RenderViewCreated(
    content::RenderViewHost* renderViewHost)
{
    if (!d_frontendHost) {
        d_frontendHost.reset(content::DevToolsFrontendHost::Create(web_contents()->GetMainFrame(), this));
    }
}

void DevToolsFrontendHostDelegateImpl::DocumentAvailableInMainFrame()
{
    d_agentHost = content::DevToolsAgentHost::GetOrCreateFor(d_inspectedContents);
    d_agentHost->AttachClient(this);
    if (d_inspectElementPointPending) {
        d_inspectElementPointPending = false;
        d_agentHost->InspectElement(d_inspectElementPoint.x, d_inspectElementPoint.y);
    }
}

void DevToolsFrontendHostDelegateImpl::WebContentsDestroyed()
{
    if (d_agentHost) {
        d_agentHost->DetachClient();
        d_agentHost = 0;
    }
}

void DevToolsFrontendHostDelegateImpl::HandleMessageFromDevToolsFrontend(
    const std::string& message)
{
    // This implementation was copied from shell_devtools_frontend.cc

    if (!d_agentHost)
        return;
    std::string method;
    base::ListValue* params = NULL;
    base::DictionaryValue* dict = NULL;
    scoped_ptr<base::Value> parsed_message(base::JSONReader::Read(message));
    if (!parsed_message ||
        !parsed_message->GetAsDictionary(&dict) ||
        !dict->GetString("method", &method)) {
        return;
    }
    int request_id = 0;
    dict->GetInteger("id", &request_id);
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
    else if (method == "loadNetworkResource" && params->GetSize() == 3) {
        // TODO(pfeldman): handle some of the embedder messages in content.
        std::string url;
        std::string headers;
        int stream_id;
        if (!params->GetString(0, &url) ||
            !params->GetString(1, &headers) ||
            !params->GetInteger(2, &stream_id)) {
            return;
        }

        GURL gurl(url);
        if (!gurl.is_valid()) {
            base::DictionaryValue response;
            response.SetInteger("statusCode", 404);
            SendMessageAck(request_id, &response);
            return;
        }

        net::URLFetcher* fetcher =
            net::URLFetcher::Create(gurl, net::URLFetcher::GET, this).release();
        d_pendingRequests[fetcher] = request_id;
        fetcher->SetRequestContext(web_contents()->GetBrowserContext()->
            GetRequestContext());
        fetcher->SetExtraRequestHeaders(headers);
        fetcher->SaveResponseWithWriter(scoped_ptr<net::URLFetcherResponseWriter>(
            new ResponseWriter(d_weakFactory.GetWeakPtr(), stream_id)));
        fetcher->Start();
        return;
    }
    else if (method == "getPreferences") {
        SendMessageAck(request_id, &d_preferences);
        return;
    }
    else if (method == "setPreference") {
        std::string name;
        std::string value;
        if (!params->GetString(0, &name) ||
            !params->GetString(1, &value)) {
            return;
        }
        d_preferences.SetStringWithoutPathExpansion(name, value);
    }
    else if (method == "removePreference") {
        std::string name;
        if (!params->GetString(0, &name))
            return;
        d_preferences.RemoveWithoutPathExpansion(name, nullptr);
    }
    else {
        return;
    }

    if (request_id)
        SendMessageAck(request_id, nullptr);
}

void DevToolsFrontendHostDelegateImpl::HandleMessageFromDevToolsFrontendToBackend(
    const std::string& message)
{
    // This implementation was copied from shell_devtools_frontend.cc

    if (d_agentHost)
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

void DevToolsFrontendHostDelegateImpl::OnURLFetchComplete(const net::URLFetcher* source)
{
    // This implementation was copied from shell_devtools_frontend.cc

    DCHECK(source);
    PendingRequestsMap::iterator it = d_pendingRequests.find(source);
    DCHECK(it != d_pendingRequests.end());

    base::DictionaryValue response;
    base::DictionaryValue* headers = new base::DictionaryValue();
    net::HttpResponseHeaders* rh = source->GetResponseHeaders();
    response.SetInteger("statusCode", rh ? rh->response_code() : 200);
    response.Set("headers", headers);

    void* iterator = NULL;
    std::string name;
    std::string value;
    while (rh && rh->EnumerateHeaderLines(&iterator, &name, &value))
        headers->SetString(name, value);

    SendMessageAck(it->second, &response);
    d_pendingRequests.erase(it);
    delete source;
}

void DevToolsFrontendHostDelegateImpl::CallClientFunction(
    const std::string& function_name,
    const base::Value* arg1,
    const base::Value* arg2,
    const base::Value* arg3)
{
    std::string javascript = function_name + "(";
    if (arg1) {
        std::string json;
        base::JSONWriter::Write(arg1, &json);
        javascript.append(json);
        if (arg2) {
            base::JSONWriter::Write(arg2, &json);
            javascript.append(", ").append(json);
            if (arg3) {
                base::JSONWriter::Write(arg3, &json);
                javascript.append(", ").append(json);
            }
        }
    }
    javascript.append(");");
    web_contents()->GetMainFrame()->ExecuteJavaScript(
        base::UTF8ToUTF16(javascript));
}

void DevToolsFrontendHostDelegateImpl::SendMessageAck(
    int request_id,
    const base::Value* arg)
{
    base::FundamentalValue id_value(request_id);
    CallClientFunction("DevToolsAPI.embedderMessageAck",
        &id_value, arg, nullptr);
}

}  // close namespace blpwtk2

