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
#include "public/web/WebBBPrintInfo.h"

#include "core/page/BBPrintInfo.h"

#include "public/platform/WebString.h"

using namespace WebCore;

namespace blink {

void WebBBPrintHeader::setText(const WebString& text)
{
    m_private->setText(text);
}

void WebBBPrintHeader::setFontSize(float fontSize)
{
    m_private->setFontSize(fontSize);
}

void WebBBPrintHeader::setFontFamily(const WebString& fontFamily)
{
    m_private->setFontFamily(fontFamily);
}

void WebBBPrintHeader::setColor(WebColor color)
{
    m_private->setColor(WebCore::RGBColor::create(color));
}

void WebBBPrintHeader::setVerticalMargin(float verticalMargin)
{
    m_private->setVerticalMargin(verticalMargin);
}

void WebBBPrintHeader::setLeftMargin(float leftMargin)
{
    m_private->setLeftMargin(leftMargin);
}

void WebBBPrintHeader::setRightMargin(float rightMargin)
{
    m_private->setRightMargin(rightMargin);
}

void WebBBPrintHeader::setAlign(unsigned short align)
{
    m_private->setAlign(align);
}


WebString WebBBPrintHeader::text() const
{
    return m_private->text();
}

float WebBBPrintHeader::fontSize() const
{
    return m_private->fontSize();
}

WebString WebBBPrintHeader::fontFamily() const
{
    return m_private->fontFamily();
}

WebColor WebBBPrintHeader::color() const
{
    return m_private->color()->color().rgb();
}

float WebBBPrintHeader::verticalMargin() const
{
    return m_private->verticalMargin();
}

float WebBBPrintHeader::leftMargin() const
{
    return m_private->leftMargin();
}

float WebBBPrintHeader::rightMargin() const
{
    return m_private->rightMargin();
}

unsigned short WebBBPrintHeader::align() const
{
    return m_private->align();
}


void WebBBPrintHeader::reset()
{
    m_private.reset();
}

void WebBBPrintHeader::assign(const WebBBPrintHeader& other)
{
    m_private = other.m_private;
}

WebBBPrintHeader::WebBBPrintHeader(const PassRefPtr<WebCore::BBPrintHeader>& header)
: m_private(header)
{
}

WebBBPrintHeader WebBBPrintInfo::headerLeft() const
{
    return WebBBPrintHeader(m_private->headerLeft());
}

WebBBPrintHeader WebBBPrintInfo::headerCenter() const
{
    return WebBBPrintHeader(m_private->headerCenter());
}

WebBBPrintHeader WebBBPrintInfo::headerRight() const
{
    return WebBBPrintHeader(m_private->headerRight());
}

WebBBPrintHeader WebBBPrintInfo::footerLeft() const
{
    return WebBBPrintHeader(m_private->footerLeft());
}

WebBBPrintHeader WebBBPrintInfo::footerCenter() const
{
    return WebBBPrintHeader(m_private->footerCenter());
}

WebBBPrintHeader WebBBPrintInfo::footerRight() const
{
    return WebBBPrintHeader(m_private->footerRight());
}

void WebBBPrintInfo::reset()
{
    m_private.reset();
}

void WebBBPrintInfo::assign(const WebBBPrintInfo& other)
{
    m_private = other.m_private;
}

WebBBPrintInfo::WebBBPrintInfo(const PassRefPtr<WebCore::BBPrintInfo>& info)
: m_private(info)
{
}

} // namespace blink
