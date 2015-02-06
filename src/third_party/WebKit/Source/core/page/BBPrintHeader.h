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


#ifndef BBPrintHeader_h
#define BBPrintHeader_h

#include "core/css/RGBColor.h"
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>

namespace blink {

    class BBPrintHeader : public RefCounted<BBPrintHeader> {
    public:
        enum Align {
            LEFT = 0,
            CENTER = 1,
            RIGHT = 2
        };

        static PassRefPtr<BBPrintHeader> create() { return adoptRef(new BBPrintHeader()); }

        void setText(const String& text);
        void setFontSize(float fontSize);
        void setFontFamily(const String& fontFamily);
        void setColor(PassRefPtr<RGBColor> color);
        void setVerticalMargin(float verticalMargin);
        void setLeftMargin(float leftMargin);
        void setRightMargin(float rightMargin);
        void setAlign(unsigned short align);
        void reset();

        const String& text() const;
        float fontSize() const;
        const String& fontFamily() const;
        PassRefPtr<RGBColor> color() const;
        float verticalMargin() const;
        float leftMargin() const;
        float rightMargin() const;
        unsigned short align() const;

    private:

        explicit BBPrintHeader();

        String               d_text;
        float                d_fontSize;
        String               d_fontFamily;
        RefPtr<RGBColor>     d_color;
        float                d_verticalMargin;
        float                d_leftMargin;
        float                d_rightMargin;
        unsigned short       d_align;
    };

} // namespace blink

#endif // BBPrintHeader_h
