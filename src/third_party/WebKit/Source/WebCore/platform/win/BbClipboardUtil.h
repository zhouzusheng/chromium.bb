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

// BbClipboardUtil.h                                                  -*-C++-*-

#ifndef INCLUDED_BBCLIPBOARDUTIL
#define INCLUDED_BBCLIPBOARDUTIL

#ifndef _STRING_
#include <string>
#endif

#ifndef _VECTOR_
#include <vector>
#endif

namespace WebCore {

class ClipboardUtil {
  private:
    // NOT IMPLEMENTED
    ClipboardUtil();

  public:

    static bool getAvailableClipboardFormats(std::vector<std::string>
                                                               *formatsArray);
        // Load into the specified 'formatsArray' the list of available
        // clipboard formats.

    static bool hasFormat(const std::string& format);
        // Return true if the specified 'format' exists and false otherwise.

    static bool getTextFromClipboard(std::string *result, unsigned format);
        // Load into the specified 'result' in the specified 'format' (if
        // available) on the clipboard and returns 'true' if text data is
        // available. Note that behavior is undefined unless the data on the
        // Clipboard is a null terminated string.

    static bool getTextFromClipboard(std::string *result, const char *format);
        // Load into the specified 'result' in the specified 'format' (if
        // available) on the clipboard and returns 'true' if text data is
        // available. Note that behavior is undefined unless the data on the
        // Clipboard is a null terminated string.

    static bool getStringData(std::string *result, const std::string& format);
        // Load into the specified 'result' the string data for the specified
        // 'format' from the Windows clipboard, returning true upon success,
        // and false otherwise.

    static bool getWStringData(std::wstring *result, const std::string& format);
        // Load into the specified 'result' the string data for the specified
        // 'format' from the Windows clipboard, returning true upon success,
        // and false otherwise.

    static bool setStringData(const std::string& data, const std::string& format);
        // Write the specified 'data' onto the clipboard as the specified
        // 'format', returning true upon success and false otherwise.

    static bool setWStringData(const std::wstring& data, const std::string& format);
        // Write the specified 'data' onto the clipboard as the specified
        // 'format', returning true upon success and false otherwise.

    static bool setCustomData(const void         *pData,
                              size_t              len,
                              const std::string&  format);
        // Write the specified 'len' bytes located at the specified 'pData'
        // onto the clipboard with the specified 'format', returning true upon
        // success and false otherwise.

    static bool setBitmapData(const unsigned char* rgbData,
                              int                  width,
                              int                  height,
                              int                  bytesPerPixel);
        // Write the specified 'rgbData' onto the clipboard. 'rgbData' represents
        // bitmap pixel data, which has the specified 'width', 'height' and
        // 'bytesPerPixel'.

    static bool emptyClipboard();
        // Empty the clipboard
};

}  // close namespace WebCore

#endif
// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2012
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// --------------------------------- END-OF-FILE -----------------------------
