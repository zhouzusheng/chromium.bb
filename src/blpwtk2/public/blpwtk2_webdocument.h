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

#ifndef INCLUDED_BLPWTK2_WEBDOCUMENT_H
#define INCLUDED_BLPWTK2_WEBDOCUMENT_H

#include <blpwtk2_config.h>
#include <blpwtk2_webnode.h>

#ifdef BLPWTK2_IMPLEMENTATION
namespace WebKit {
    class WebDocument;
}  // close namespace WebKit
#endif  // BLPWTK2_IMPLEMENTATION

namespace blpwtk2 {

class WebElement;
class StringRef;
class String;

class WebDocument : public WebNode {
public:
    WebDocument() : WebNode() {}
    WebDocument(const WebDocument& other) : WebNode(other) {}
    WebDocument& operator=(const WebDocument& rhs) { assign(rhs); return *this; }

    BLPWTK2_EXPORT WebElement createElement(const StringRef& tagName);
    BLPWTK2_EXPORT WebElement body() const;
    BLPWTK2_EXPORT WebElement head() const;
    BLPWTK2_EXPORT WebElement documentElement() const;
    BLPWTK2_EXPORT WebElement getElementById(const StringRef& id) const;
    BLPWTK2_EXPORT String innerHTML() const;

    BLPWTK2_EXPORT static bool isWebDocument(v8::Handle<v8::Value> handle);
    BLPWTK2_EXPORT static WebDocument fromV8Handle(v8::Handle<v8::Value> handle);

#ifdef BLPWTK2_IMPLEMENTATION
    WebDocument(const WebKit::WebDocument&);
    WebDocument& operator=(const WebKit::WebDocument&);
    operator WebKit::WebDocument() const;
#endif
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBDOCUMENT_H

