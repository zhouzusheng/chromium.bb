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

// BbUtfUtil.cpp                                                      -*-C++-*-

#include <BbUtfUtil.h>

#include <string>
#include <windows.h>

namespace WebCore {

void UtfUtil::utf8ToUtf16(const std::string&  utf8In,
                          std::wstring       *utf16Out)
{
    //KITDIAG_ASSERT(utf16Out);
    utf16Out->clear();

    if (utf8In.empty()) {
        return;
    }

    // this first call just works out how much buffer space we need
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
    const int numWideChars = MultiByteToWideChar(
                                   CP_UTF8,         // CodePage
                                   0,               // dwFlags
                                   utf8In.c_str(),  // MultiByteString in
                                   utf8In.size(),   // size of MultiByteString
                                   NULL,            // WideCharString out
                                   0);              // size of WideCharStr

    utf16Out->resize(numWideChars);

    // see the comments here for why this strange usage is ok:
    // http://herbsutter.com/2008/04/07/cringe-not-vectors-are-guaranteed-to-be-contiguous/#comment-483
    wchar_t *pData = &(*utf16Out)[0];

    // this second call actually does the conversion
    // Note that because we are passing source_len in, the output will not
    // have a NUL-terminator appended to it automatically, and the return value will
    // not include that terminator
    MultiByteToWideChar(CP_UTF8,         // CodePage
                        0,               // dwFlags
                        utf8In.c_str(),  // MultiByteString in
                        utf8In.size(),   // size of MultiByteString
                        pData,           // WideCharString out
                        numWideChars);   // size of WideCharStr
}

void UtfUtil::utf16ToUtf8(const std::wstring& utf16In, std::string *utf8Out)
{
    //KITDIAG_ASSERT(utf8Out);
    (*utf8Out).clear();

    if (utf16In.empty()) {
        return;
    }

    // this first call just works out how much buffer space we need
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd374130(v=vs.85).aspx
    const int numBytes = WideCharToMultiByte(
                                           CP_UTF8, // CodePage
                                           0,       // dwFlags
                                           utf16In.c_str(), // WideCharString in
                                           utf16In.size(),  // size of WideCharStr
                                           NULL,    // MultiByteString out
                                           0,       // size of MultiByteString
                                           NULL,    // defaultCharacter
                                           NULL);   // UsedDefaultCharacter
    utf8Out->resize(numBytes);

    // see the comments here for why this strange usage is ok:
    // http://herbsutter.com/2008/04/07/cringe-not-vectors-are-guaranteed-to-be-contiguous/#comment-483
    char *pData = &(*utf8Out)[0];

    // this second call actually does the conversion
    WideCharToMultiByte(CP_UTF8,          // CodePage
                        0,                // dwFlags
                        utf16In.c_str(),  // WideCharString in
                        utf16In.size(),   // size of WideCharStr
                        pData,            // MultiByteString out
                        numBytes,         // size of MultiByteString
                        NULL,             // defaultCharacter
                        NULL);            // UsedDefaultCharacter
}

} // close namespace WebCore

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2012
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// --------------------------------- END-OF-FILE -----------------------------
