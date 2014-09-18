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

namespace blink {

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

    BLINK_EXPORT void setText(const WebString& text);
    BLINK_EXPORT void setFontSize(float fontSize);
    BLINK_EXPORT void setFontFamily(const WebString& fontFamily);
    BLINK_EXPORT void setColor(WebColor color);
    BLINK_EXPORT void setVerticalMargin(float verticalMargin);
    BLINK_EXPORT void setLeftMargin(float leftMargin);
    BLINK_EXPORT void setRightMargin(float rightMargin);
    BLINK_EXPORT void setAlign(unsigned short align);

    BLINK_EXPORT WebString text() const;
    BLINK_EXPORT float fontSize() const;
    BLINK_EXPORT WebString fontFamily() const;
    BLINK_EXPORT WebColor color() const;
    BLINK_EXPORT float verticalMargin() const;
    BLINK_EXPORT float leftMargin() const;
    BLINK_EXPORT float rightMargin() const;
    BLINK_EXPORT unsigned short align() const;

    BLINK_EXPORT void reset();
    BLINK_EXPORT void assign(const WebBBPrintHeader&);

    bool isNull() const { return m_private.isNull(); }

#if BLINK_IMPLEMENTATION
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

    BLINK_EXPORT WebBBPrintHeader headerLeft() const;
    BLINK_EXPORT WebBBPrintHeader headerCenter() const;
    BLINK_EXPORT WebBBPrintHeader headerRight() const;
    BLINK_EXPORT WebBBPrintHeader footerLeft() const;
    BLINK_EXPORT WebBBPrintHeader footerCenter() const;
    BLINK_EXPORT WebBBPrintHeader footerRight() const;

    BLINK_EXPORT void reset();
    BLINK_EXPORT void assign(const WebBBPrintInfo&);

    bool isNull() const { return m_private.isNull(); }

#if BLINK_IMPLEMENTATION
    WebBBPrintInfo(const WTF::PassRefPtr<WebCore::BBPrintInfo>&);
#endif

private:
    WebPrivatePtr<WebCore::BBPrintInfo> m_private;
};

} // namespace blink

#endif
