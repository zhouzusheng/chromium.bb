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

#include "config.h"
#include "BBPrintHeader.h"

namespace blink {

BBPrintHeader::BBPrintHeader()
{
    reset();
}

void BBPrintHeader::setText(const String& text)
{
    d_text = text;
}
void BBPrintHeader::setFontSize(float fontSize)
{
    d_fontSize = fontSize;
}
void BBPrintHeader::setFontFamily(const String& fontFamily)
{
    d_fontFamily = fontFamily;
}
void BBPrintHeader::setColor(PassRefPtr<RGBColor> color) {
    d_color = color;
}
void BBPrintHeader::setVerticalMargin(float verticalMargin)
{
    d_verticalMargin = verticalMargin;
}
void BBPrintHeader::setLeftMargin(float leftMargin)
{
    d_leftMargin = leftMargin;
}
void BBPrintHeader::setRightMargin(float rightMargin)
{
    d_rightMargin = rightMargin;
}
void BBPrintHeader::setAlign(unsigned short align)
{
    d_align = align;
}

void BBPrintHeader::reset()
{
    d_text = L"";
    d_fontSize = 12;
    d_fontFamily = "Arial";
    d_color = RGBColor::create(makeRGB(0, 0, 0));
    d_verticalMargin = 0;
    d_leftMargin = 0;
    d_rightMargin = 0;
    d_align = BBPrintHeader::LEFT;
}

const String& BBPrintHeader::text() const
{
    return d_text;
}
float BBPrintHeader::fontSize() const
{
    return d_fontSize;
}
const String& BBPrintHeader::fontFamily() const
{
    return d_fontFamily;
}
PassRefPtr<RGBColor> BBPrintHeader::color() const
{
    return d_color;
}
float BBPrintHeader::verticalMargin() const
{
    return d_verticalMargin;
}
float BBPrintHeader::leftMargin() const
{
    return d_leftMargin;
}
float BBPrintHeader::rightMargin() const
{
    return d_rightMargin;
}
unsigned short BBPrintHeader::align() const
{
    return d_align;
}

} // namespace blink
