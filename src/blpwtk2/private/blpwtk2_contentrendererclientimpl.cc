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

#include <blpwtk2_contentrendererclientimpl.h>

#include <blpwtk2_renderviewobserverimpl.h>

#include <base/strings/utf_string_conversions.h>
#include <net/base/net_errors.h>
#include <third_party/WebKit/public/platform/WebURLError.h>
#include <third_party/WebKit/public/platform/WebURLRequest.h>
#include <content/public/renderer/render_thread.h>
#include <chrome/renderer/spellchecker/spellcheck_provider.h>
#include <chrome/renderer/spellchecker/spellcheck.h>

namespace blpwtk2 {

ContentRendererClientImpl::ContentRendererClientImpl()
{
}

ContentRendererClientImpl::~ContentRendererClientImpl()
{
}

void ContentRendererClientImpl::RenderThreadStarted()
{
    content::RenderThread* thread = content::RenderThread::Get();

    if (!d_spellcheck) {
        d_spellcheck.reset(new SpellCheck());
        thread->AddObserver(d_spellcheck.get());
    }
}

void ContentRendererClientImpl::RenderViewCreated(
    content::RenderView* render_view)
{
    // Note that RenderViewObserverImpl automatically gets deleted when the
    // RenderView is destroyed.
    new RenderViewObserverImpl(render_view);

    new SpellCheckProvider(render_view, d_spellcheck.get());
}

void ContentRendererClientImpl::GetNavigationErrorStrings(
    WebKit::WebFrame* frame,
    const WebKit::WebURLRequest& failed_request,
    const WebKit::WebURLError& error,
    std::string* error_html,
    string16* error_description)
{
    GURL gurl = failed_request.url();

    WebKit::WebCString webcs_domain = error.domain.utf8();
    std::string domain(webcs_domain.data(), webcs_domain.length());

    std::string description;
    if (0 == strcmp(domain.c_str(), net::kErrorDomain)) {
        description = net::ErrorToString(error.reason);
    }

    std::string errorCode;
    {
        char tmp[128];
        sprintf_s(tmp, sizeof(tmp), "%d", error.reason);
        errorCode = tmp;
    }

    WebKit::WebCString webcs_localdesc = error.localizedDescription.utf8();
    std::string localdesc(webcs_localdesc.data(), webcs_localdesc.length());

    if (error_html) {
        *error_html = "<h2>Navigation Error</h2>";
        *error_html += "<p>Failed to load '<b>" + gurl.spec() + "</b>'</p>";
        if (description.length()) {
            *error_html += "<p>" + description + "</p>";
        }
        if (domain.length()) {
            *error_html += "<p>Error Domain: " + domain + "</p>";
        }
        *error_html += "<p>Error Reason: " + errorCode + "</p>";
        if (localdesc.length()) {
            *error_html += "<p>" + localdesc + "</p>";
        }
    }

    if (error_description) {
        std::string tmp = "Failed to load '" + gurl.spec() + "'.";
        if (description.length()) {
            tmp += " " + description;
        }
        if (domain.length()) {
            tmp += " -- Error Domain: " + domain;
        }
        tmp += " -- Error Reason: " + errorCode;
        if (localdesc.length()) {
            tmp += " -- " + localdesc;
        }
        *error_description = UTF8ToUTF16(tmp);
    }
}

}  // close namespace blpwtk2
