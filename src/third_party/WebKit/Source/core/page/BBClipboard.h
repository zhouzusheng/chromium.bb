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


#ifndef BBCLipboard_h
#define BBCLipboard_h

#include "core/frame/DOMWindowProperty.h"
#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>
#include <wtf/ArrayBuffer.h>

namespace WebCore {

    class Frame;

    class BBClipboard : public RefCounted<BBClipboard>, public DOMWindowProperty {
    public:
        static PassRefPtr<BBClipboard> create(Frame *frame) { return adoptRef(new BBClipboard(frame)); }

        void empty();
        void setCustomString(const String& format, const String& data, bool useUtf8 = true);
        String getCustomString(const String& format, bool useUtf8 = true);
        bool hasCustomFormat(const String& format);
        void setCustomBinaryData(const String& format, ArrayBuffer* arrayBuffer);

    private:

        explicit BBClipboard(Frame *frame);
    };

} // namespace WebCore

#endif // BBCLipboard_h
