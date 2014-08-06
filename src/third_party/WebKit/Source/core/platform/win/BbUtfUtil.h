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

// BbUtfUtil.h                                                        -*-C++-*-

#ifndef INCLUDED_BBUTFUTIL
#define INCLUDED_BBUTFUTIL

#ifndef _STRING_
#include <string>
#endif

namespace WebCore {

struct UtfUtil {
  private:
    // NOT IMPLEMENTED
    UtfUtil();

  public:
    // This utility object contains methods for the manipulation and conversion
    // of UCS Transformation Format (UTF) data.

    static void utf8ToUtf16(const std::string& utf8In, std::wstring *utf16Out);
        // Convert the specified 'utf8In' multi-byte string into a
        // wide-character string and load the result into the specified
        // 'utf16Out'.

    static void utf16ToUtf8(const std::wstring& utf16In, std::string *utf8Out);
        // Convert the specified 'utf16In' wide-character string into a
        // multi-byte string and load the result into the specified 'utf8Out'.
};

} // close namespace WebCore

#endif
// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2012
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// --------------------------------- END-OF-FILE -----------------------------
