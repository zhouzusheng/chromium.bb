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
#include "BBPrintInfo.h"
#include "Document.h"

namespace WebCore {

BBPrintInfo::BBPrintInfo(Document* document)
    : ActiveDOMObject(document)
    , d_headerLeft(BBPrintHeader::create())
    , d_headerCenter(BBPrintHeader::create())
    , d_headerRight(BBPrintHeader::create())
    , d_footerLeft(BBPrintHeader::create())
    , d_footerCenter(BBPrintHeader::create())
    , d_footerRight(BBPrintHeader::create())
{
    d_headerLeft->setAlign(BBPrintHeader::LEFT);
    d_headerCenter->setAlign(BBPrintHeader::CENTER);
    d_headerRight->setAlign(BBPrintHeader::RIGHT);
    d_footerLeft->setAlign(BBPrintHeader::LEFT);
    d_footerCenter->setAlign(BBPrintHeader::CENTER);
    d_footerRight->setAlign(BBPrintHeader::RIGHT);

    suspendIfNeeded();
}

RefPtr<BBPrintHeader> BBPrintInfo::headerLeft()
{
    return d_headerLeft;
}

RefPtr<BBPrintHeader> BBPrintInfo::headerCenter()
{
    return d_headerCenter;
}

RefPtr<BBPrintHeader> BBPrintInfo::headerRight()
{
    return d_headerRight;
}

RefPtr<BBPrintHeader> BBPrintInfo::footerLeft()
{
    return d_footerLeft;
}

RefPtr<BBPrintHeader> BBPrintInfo::footerCenter()
{
    return d_footerCenter;
}

RefPtr<BBPrintHeader> BBPrintInfo::footerRight()
{
    return d_footerRight;
}

} // namespace WebCore
