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
#include "BBClipboard.h"

#include "BBClipboardUtil.h"
#include "BbUtfUtil.h"
#include "Frame.h"

namespace WebCore {

BBClipboard::BBClipboard(Frame *frame)
    : DOMWindowProperty(frame)
{
}


void BBClipboard::empty()
{
    ClipboardUtil::emptyClipboard();
}

void  BBClipboard::setCustomString(const String& format, const String& data, bool useUtf8)
{
    String mutableFormat(format);
    String mutableData(data);

    std::string formatUtf8;
    UtfUtil::utf16ToUtf8(std::wstring(mutableFormat.charactersWithNullTermination()), &formatUtf8);

    if (useUtf8) {
        std::string dataUtf8;
        UtfUtil::utf16ToUtf8(std::wstring(mutableData.charactersWithNullTermination()), &dataUtf8);
        ClipboardUtil::setStringData(dataUtf8, formatUtf8);
    } else {
        ClipboardUtil::setWStringData(
            std::wstring(mutableData.charactersWithNullTermination()), formatUtf8);
    }
}

String BBClipboard::getCustomString(const String& format, bool useUtf8)
{
    String mutableFormat(format);

    std::string formatUtf8;
    UtfUtil::utf16ToUtf8(std::wstring(mutableFormat.charactersWithNullTermination()), &formatUtf8);

    std::wstring wDataOut;
    if (useUtf8) {
        std::string dataOut;
        ClipboardUtil::getStringData(&dataOut, formatUtf8);
        UtfUtil::utf8ToUtf16(dataOut, &wDataOut);
    } else {
        ClipboardUtil::getWStringData(&wDataOut, formatUtf8);
    }
    return String(wDataOut.c_str());
}

bool  BBClipboard::hasCustomFormat(const String& format)
{
    String mutableFormat(format);

    std::string formatUtf8;
    UtfUtil::utf16ToUtf8(std::wstring(mutableFormat.charactersWithNullTermination()), &formatUtf8);

    return ClipboardUtil::hasFormat(formatUtf8);
}

void BBClipboard::setCustomBinaryData(const String& format, ArrayBuffer* arrayBuffer)
{
    String mutableFormat(format);

    std::string formatUtf8;
    UtfUtil::utf16ToUtf8(std::wstring(mutableFormat.charactersWithNullTermination()), &formatUtf8);

    ClipboardUtil::setCustomData(arrayBuffer->data(), arrayBuffer->byteLength(), formatUtf8);
}

} // namespace WebCore
