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

#ifndef INCLUDED_BLPWTK2_WEBNODE_H
#define INCLUDED_BLPWTK2_WEBNODE_H

#include <blpwtk2_config.h>

#ifdef BLPWTK2_IMPLEMENTATION
namespace WebKit {
    class WebNode;
}  // close namespace WebKit
#endif  // BLPWTK2_IMPLEMENTATION

namespace v8 {
    template <class T> class Handle;
    class Value;
}

namespace blpwtk2 {

class String;
class StringRef;
class WebElement;
class WebDocument;

class WebNode {
  public:
    BLPWTK2_EXPORT WebNode();
    BLPWTK2_EXPORT WebNode(const WebNode&);
    BLPWTK2_EXPORT ~WebNode();
    WebNode& operator=(const WebNode& rhs) { assign(rhs); return *this; }

    BLPWTK2_EXPORT bool equals(const WebNode&) const;
    // Required for using WebNodes in std maps.  Note the order used is
    // arbitrary and should not be expected to have any specific meaning.
    BLPWTK2_EXPORT bool lessThan(const WebNode&) const;

    BLPWTK2_EXPORT void reset();
    BLPWTK2_EXPORT void assign(const WebNode&);
    BLPWTK2_EXPORT bool isNull() const;

    BLPWTK2_EXPORT WebNode parentNode() const;
    BLPWTK2_EXPORT String nodeName() const;
    BLPWTK2_EXPORT String nodeValue() const;
    BLPWTK2_EXPORT WebDocument document() const;
    BLPWTK2_EXPORT WebNode firstChild() const;
    BLPWTK2_EXPORT WebNode lastChild() const;
    BLPWTK2_EXPORT WebNode previousSibling() const;
    BLPWTK2_EXPORT WebNode nextSibling() const;
    BLPWTK2_EXPORT bool isTextNode() const;
    BLPWTK2_EXPORT bool isElementNode() const;
    BLPWTK2_EXPORT bool insertBefore(const WebNode& newChild, const WebNode& refChild);
    BLPWTK2_EXPORT bool replaceChild(const WebNode& newChild, const WebNode& oldChild);
    BLPWTK2_EXPORT bool appendChild(const WebNode& child);
    BLPWTK2_EXPORT bool remove();
    BLPWTK2_EXPORT bool setTextContent(const StringRef&);
    BLPWTK2_EXPORT bool removeChild(const WebNode& oldChild);
    BLPWTK2_EXPORT String textContent() const;
    BLPWTK2_EXPORT unsigned numChildren() const;
    BLPWTK2_EXPORT WebNode childAt(size_t index) const;

    BLPWTK2_EXPORT v8::Handle<v8::Value> toV8Handle() const;

    BLPWTK2_EXPORT static bool isWebNode(v8::Handle<v8::Value> handle);
    BLPWTK2_EXPORT static WebNode fromV8Handle(v8::Handle<v8::Value> handle);

#ifdef BLPWTK2_IMPLEMENTATION
    WebNode(const WebKit::WebNode&);
    WebNode& operator=(const WebKit::WebNode&);
    operator WebKit::WebNode() const;

    template<typename T> T& internalTo()
    {
        return *reinterpret_cast<T*>(d_buffer);
    }
    template<typename T> T& internalToNonConst() const
    {
        // Some WebKit::* methods are non-const even though they
        // should be const, hence this const_cast is required.
        return *const_cast<T*>(reinterpret_cast<const T*>(d_buffer));
    }
    template<typename T> const T& internalToConst() const
    {
        return *reinterpret_cast<const T*>(d_buffer);
    }
#endif

  private:
    union {
        char  d_buffer[8];  // must be the size of a WebKit::WebNode
        void* d_align;
    };
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBNODE_H

