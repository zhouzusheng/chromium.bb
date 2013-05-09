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

#include <blpwtk2_webelement.h>

#include <blpwtk2_string.h>
#include <blpwtk2_stringref.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebElement.h>

namespace blpwtk2 {

COMPILE_ASSERT(sizeof(WebElement) == sizeof(WebKit::WebElement), webelement_size_mismatch);

// Returns the qualified name, which may contain a prefix and a colon.
String WebElement::tagName() const
{
    return fromWebString(internalToConst<WebKit::WebElement>().tagName());
}

bool WebElement::setAttribute(const StringRef& name, const StringRef& value)
{
    return internalTo<WebKit::WebElement>().setAttribute(toWebString(name),
                                                         toWebString(value));
}

String WebElement::innerText() const
{
    // const_cast required because WebKit::WebElement::innerText() is not const
    return fromWebString(internalToNonConst<WebKit::WebElement>().innerText());
}

String WebElement::attributeName(unsigned index) const
{
    return fromWebString(internalToConst<WebKit::WebElement>().attributeName(index));
}

String WebElement::attributeLocalName(unsigned index) const
{
    return fromWebString(internalToConst<WebKit::WebElement>().attributeLocalName(index));
}

String WebElement::attributeValue(unsigned index) const
{
    return fromWebString(internalToConst<WebKit::WebElement>().attributeValue(index));
}

int WebElement::attributeCount() const
{
    return internalToConst<WebKit::WebElement>().attributeCount();
}

WebElement::WebElement(const WebKit::WebElement& other)
: WebNode(other)
{
}

WebElement& WebElement::operator=(const WebKit::WebElement& rhs)
{
    WebNode::assign(rhs);
    return *this;
}

WebElement::operator WebKit::WebElement() const
{
    return internalToConst<WebKit::WebElement>();
}

}  // close namespace blpwtk2

