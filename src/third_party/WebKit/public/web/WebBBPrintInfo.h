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

#ifndef WebBBPrintInfo_h
#define WebBBPrintInfo_h

#include "../platform/WebCommon.h"
#include "../platform/WebColor.h"
#include "../platform/WebPrivatePtr.h"

namespace WebCore {
    class BBPrintInfo;
    class BBPrintHeader;
}

namespace WebKit {

class WebString;

class WebBBPrintHeader {
public:
    enum Align {
        LEFT = 0,
        CENTER = 1,
        RIGHT = 2
    };

    ~WebBBPrintHeader() { reset(); }

    WebBBPrintHeader() { }
    WebBBPrintHeader(const WebBBPrintHeader& o) { assign(o); }
    WebBBPrintHeader& operator=(const WebBBPrintHeader& o)
    {
        assign(o);
        return *this;
    }

    WEBKIT_EXPORT void setText(const WebString& text);
    WEBKIT_EXPORT void setFontSize(float fontSize);
    WEBKIT_EXPORT void setFontFamily(const WebString& fontFamily);
    WEBKIT_EXPORT void setColor(WebColor color);
    WEBKIT_EXPORT void setVerticalMargin(float verticalMargin);
    WEBKIT_EXPORT void setLeftMargin(float leftMargin);
    WEBKIT_EXPORT void setRightMargin(float rightMargin);
    WEBKIT_EXPORT void setAlign(unsigned short align);

    WEBKIT_EXPORT WebString text() const;
    WEBKIT_EXPORT float fontSize() const;
    WEBKIT_EXPORT WebString fontFamily() const;
    WEBKIT_EXPORT WebColor color() const;
    WEBKIT_EXPORT float verticalMargin() const;
    WEBKIT_EXPORT float leftMargin() const;
    WEBKIT_EXPORT float rightMargin() const;
    WEBKIT_EXPORT unsigned short align() const;

    WEBKIT_EXPORT void reset();
    WEBKIT_EXPORT void assign(const WebBBPrintHeader&);

    bool isNull() const { return m_private.isNull(); }

#if WEBKIT_IMPLEMENTATION
    WebBBPrintHeader(const WTF::PassRefPtr<WebCore::BBPrintHeader>&);
#endif

private:
    WebPrivatePtr<WebCore::BBPrintHeader> m_private;
};

// A container for passing around a reference to AccessibilityObject.
class WebBBPrintInfo {
public:
    ~WebBBPrintInfo() { reset(); }

    WebBBPrintInfo() { }
    WebBBPrintInfo(const WebBBPrintInfo& o) { assign(o); }
    WebBBPrintInfo& operator=(const WebBBPrintInfo& o)
    {
        assign(o);
        return *this;
    }

    WEBKIT_EXPORT WebBBPrintHeader headerLeft() const;
    WEBKIT_EXPORT WebBBPrintHeader headerCenter() const;
    WEBKIT_EXPORT WebBBPrintHeader headerRight() const;
    WEBKIT_EXPORT WebBBPrintHeader footerLeft() const;
    WEBKIT_EXPORT WebBBPrintHeader footerCenter() const;
    WEBKIT_EXPORT WebBBPrintHeader footerRight() const;

    WEBKIT_EXPORT void reset();
    WEBKIT_EXPORT void assign(const WebBBPrintInfo&);

    bool isNull() const { return m_private.isNull(); }

#if WEBKIT_IMPLEMENTATION
    WebBBPrintInfo(const WTF::PassRefPtr<WebCore::BBPrintInfo>&);
#endif

private:
    WebPrivatePtr<WebCore::BBPrintInfo> m_private;
};

} // namespace WebKit

#endif
