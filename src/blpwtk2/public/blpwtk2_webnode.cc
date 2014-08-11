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

#include <blpwtk2_webnode.h>

#include <blpwtk2_webdocument.h>
#include <blpwtk2_string.h>
#include <blpwtk2_stringref.h>

#include <third_party/WebKit/public/web/WebDocument.h>
#include <third_party/WebKit/public/web/WebNode.h>
#include <third_party/WebKit/public/web/WebNodeList.h>

#include <v8/include/v8.h>

namespace blpwtk2 {

COMPILE_ASSERT(sizeof(WebNode) == sizeof(WebKit::WebNode), webnode_size_mismatch);

WebNode::WebNode()
{
    COMPILE_ASSERT(sizeof(d_buffer) == sizeof(WebKit::WebNode), webnode_buffer_size_mismatch);
    new (d_buffer) WebKit::WebNode();
}

WebNode::WebNode(const WebNode& other)
{
    new (d_buffer) WebKit::WebNode(other.internalToConst<WebKit::WebNode>());
}

WebNode::~WebNode()
{
    typedef WebKit::WebNode WebKitWebNode;
    internalTo<WebKit::WebNode>().~WebKitWebNode();
}

bool WebNode::equals(const WebNode& rhs) const
{
    return internalToConst<WebKit::WebNode>().equals(rhs.internalToConst<WebKit::WebNode>());
}

bool WebNode::lessThan(const WebNode& rhs) const
{
    return internalToConst<WebKit::WebNode>().lessThan(rhs.internalToConst<WebKit::WebNode>());
}

void WebNode::reset()
{
    internalTo<WebKit::WebNode>().reset();
}

void WebNode::assign(const WebNode& rhs)
{
    internalTo<WebKit::WebNode>().assign(rhs.internalToConst<WebKit::WebNode>());
}

bool WebNode::isNull() const
{
    return internalToConst<WebKit::WebNode>().isNull();
}

WebNode WebNode::parentNode() const
{
    return internalToConst<WebKit::WebNode>().parentNode();
}

String WebNode::nodeName() const
{
    return fromWebString(internalToConst<WebKit::WebNode>().nodeName());
}

String WebNode::nodeValue() const
{
    return fromWebString(internalToConst<WebKit::WebNode>().nodeValue());
}

WebDocument WebNode::document() const
{
    return internalToConst<WebKit::WebNode>().document();
}

WebNode WebNode::firstChild() const
{
    return internalToConst<WebKit::WebNode>().firstChild();
}

WebNode WebNode::lastChild() const
{
    return internalToConst<WebKit::WebNode>().lastChild();
}

WebNode WebNode::previousSibling() const
{
    return internalToConst<WebKit::WebNode>().previousSibling();
}

WebNode WebNode::nextSibling() const
{
    return internalToConst<WebKit::WebNode>().nextSibling();
}

bool WebNode::isTextNode() const
{
    return internalToConst<WebKit::WebNode>().isTextNode();
}

bool WebNode::isElementNode() const
{
    return internalToConst<WebKit::WebNode>().isElementNode();
}

bool WebNode::insertBefore(const WebNode& newChild, const WebNode& refChild)
{
    return internalTo<WebKit::WebNode>().insertBefore(newChild.internalToConst<WebKit::WebNode>(),
                                                      refChild.internalToConst<WebKit::WebNode>());
}

bool WebNode::replaceChild(const WebNode& newChild, const WebNode& oldChild)
{
    return internalTo<WebKit::WebNode>().replaceChild(newChild.internalToConst<WebKit::WebNode>(),
                                                      oldChild.internalToConst<WebKit::WebNode>());
}

bool WebNode::appendChild(const WebNode& child)
{
    return internalTo<WebKit::WebNode>().appendChild(child.internalToConst<WebKit::WebNode>());
}

bool WebNode::remove()
{
    return internalTo<WebKit::WebNode>().remove();
}

bool WebNode::setTextContent(const StringRef& text)
{
    WebKit::WebString textStr = toWebString(text);
    return internalTo<WebKit::WebNode>().setTextContent(textStr);
}

bool WebNode::removeChild(const WebNode& oldChild)
{
    return internalTo<WebKit::WebNode>().removeChild(oldChild.internalToConst<WebKit::WebNode>());
}

String WebNode::textContent() const
{
    return fromWebString(internalToConst<WebKit::WebNode>().textContent());
}

unsigned WebNode::numChildren() const
{
    return internalToNonConst<WebKit::WebNode>().childNodes().length();
}

WebNode WebNode::childAt(const size_t index) const
{
    return internalToNonConst<WebKit::WebNode>().childNodes().item(index);
}

v8::Handle<v8::Value> WebNode::toV8Handle() const
{
    return internalToConst<WebKit::WebNode>().toV8Handle();
}

bool WebNode::isWebNode(v8::Handle<v8::Value> handle)
{
    return WebKit::WebNode::isWebNode(handle);
}

WebNode WebNode::fromV8Handle(v8::Handle<v8::Value> handle)
{
    return WebKit::WebNode::fromV8Handle(handle);
}

WebNode::WebNode(const WebKit::WebNode& other)
{
    new (d_buffer) WebKit::WebNode(other);
}

WebNode& WebNode::operator=(const WebKit::WebNode& rhs)
{
    assign(rhs);
    return *this;
}

WebNode::operator WebKit::WebNode() const
{
    return internalToConst<WebKit::WebNode>();
}

}  // close namespace blpwtk2

