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

#include <blpwtk2_webdocument.h>

#include <blpwtk2_webelement.h>
#include <blpwtk2_stringref.h>
#include <blpwtk2_string.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebDocument.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebElement.h>

#include <v8/include/v8.h>

namespace blpwtk2 {

COMPILE_ASSERT(sizeof(WebDocument) == sizeof(WebKit::WebDocument), webdocument_size_mismatch);

WebDocument::WebDocument(const WebNode& other)
: WebNode(other)
{
}

WebElement WebDocument::createElement(const StringRef& tagName)
{
    WebKit::WebString tagNameStr = toWebString(tagName);
    return internalTo<WebKit::WebDocument>().createElement(tagNameStr);
}

WebElement WebDocument::body() const
{
    return internalToConst<WebKit::WebDocument>().body();
}

WebElement WebDocument::head() const
{
    // const_cast required because WebKit::WebDocument::head() is not const
    return internalToNonConst<WebKit::WebDocument>().head();
}

WebElement WebDocument::documentElement() const
{
    return internalToConst<WebKit::WebDocument>().documentElement();
}

WebElement WebDocument::getElementById(const StringRef& id) const
{
    WebKit::WebString idStr = toWebString(id);
    return internalToConst<WebKit::WebDocument>().getElementById(idStr);
}

String WebDocument::innerHTML() const
{
    return fromWebString(internalToConst<WebKit::WebDocument>().innerHTML());
}

bool WebDocument::isWebDocument(v8::Handle<v8::Value> handle)
{
    return WebKit::WebDocument::isWebDocument(handle);
}

WebDocument WebDocument::fromV8Handle(v8::Handle<v8::Value> handle)
{
    return WebKit::WebDocument::fromV8Handle(handle);
}

WebDocument::WebDocument(const WebKit::WebDocument& other)
: WebNode(other)
{
}

WebDocument& WebDocument::operator=(const WebKit::WebDocument& rhs)
{
    assign(rhs);
    return *this;
}

WebDocument::operator WebKit::WebDocument() const
{
    return internalToConst<WebKit::WebDocument>();
}

}  // close namespace blpwtk2

