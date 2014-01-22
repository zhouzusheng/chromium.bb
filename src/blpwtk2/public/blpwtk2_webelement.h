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

#ifndef INCLUDED_BLPWTK2_ELEMENT_H
#define INCLUDED_BLPWTK2_ELEMENT_H

#include <blpwtk2_config.h>
#include <blpwtk2_webnode.h>

#ifdef BLPWTK2_IMPLEMENTATION
namespace WebKit {
    class WebElement;
}  // close namespace WebKit
#endif  // BLPWTK2_IMPLEMENTATION

namespace blpwtk2 {

class String;
class StringRef;

class WebElement : public WebNode {
  public:
    WebElement() : WebNode() {}
    WebElement(const WebElement& other) : WebNode(other) {}
    explicit BLPWTK2_EXPORT WebElement(const WebNode& other);
    WebElement& operator=(const WebElement& rhs) { assign(rhs); return *this; }

    // Returns the qualified name, which may contain a prefix and a colon.
    BLPWTK2_EXPORT String tagName() const;
    BLPWTK2_EXPORT bool setAttribute(const StringRef& name, const StringRef& value);
    BLPWTK2_EXPORT String innerText() const;
    BLPWTK2_EXPORT String attributeName(unsigned index) const;
    BLPWTK2_EXPORT String attributeLocalName(unsigned index) const;
    BLPWTK2_EXPORT String attributeValue(unsigned index) const;
    BLPWTK2_EXPORT int attributeCount() const;
    BLPWTK2_EXPORT String getAttribute(const StringRef& name) const;
    BLPWTK2_EXPORT void removeAttribute(const StringRef& name);
    BLPWTK2_EXPORT bool setCssProperty(const StringRef& name, const StringRef& value, const StringRef& priority);
    BLPWTK2_EXPORT bool removeCssProperty(const StringRef& name);
    BLPWTK2_EXPORT bool addClass(const StringRef& name);
    BLPWTK2_EXPORT bool removeClass(const StringRef& name);
    BLPWTK2_EXPORT bool containsClass(const StringRef& name) const;
    BLPWTK2_EXPORT bool toggleClass(const StringRef& name);
    BLPWTK2_EXPORT String innerHTML() const;
    BLPWTK2_EXPORT void requestSpellCheck();

    BLPWTK2_EXPORT static bool isWebElement(v8::Handle<v8::Value> handle);
    BLPWTK2_EXPORT static WebElement fromV8Handle(v8::Handle<v8::Value> handle);

#ifdef BLPWTK2_IMPLEMENTATION
    WebElement(const WebKit::WebElement&);
    WebElement& operator=(const WebKit::WebElement&);
    operator WebKit::WebElement() const;
#endif
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_ELEMENT_H

