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


#ifndef BBPrintInfo_h
#define BBPrintInfo_h

#include "core/dom/ActiveDOMObject.h"
#include "core/page/BBPrintHeader.h"
#include <wtf/RefCounted.h>

namespace blink {

    class Document;

    class BBPrintInfo : public RefCounted<BBPrintInfo>, public ActiveDOMObject {
    public:

        static PassRefPtr<BBPrintInfo> create(Document* document) { return adoptRef(new BBPrintInfo(document)); }

        RefPtr<BBPrintHeader> headerLeft();
        RefPtr<BBPrintHeader> headerCenter();
        RefPtr<BBPrintHeader> headerRight();
        RefPtr<BBPrintHeader> footerLeft();
        RefPtr<BBPrintHeader> footerCenter();
        RefPtr<BBPrintHeader> footerRight();

    private:

        explicit BBPrintInfo(Document* document);

        RefPtr<BBPrintHeader> d_headerLeft;
        RefPtr<BBPrintHeader> d_headerCenter;
        RefPtr<BBPrintHeader> d_headerRight;
        RefPtr<BBPrintHeader> d_footerLeft;
        RefPtr<BBPrintHeader> d_footerCenter;
        RefPtr<BBPrintHeader> d_footerRight;
    };

} // namespace blink

#endif // BBPrintInfo_h
