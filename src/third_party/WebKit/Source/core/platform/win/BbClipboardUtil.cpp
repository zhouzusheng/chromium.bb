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

// BbClipboardUtil.cpp                                                -*-C++-*-
#include <BbClipboardUtil.h>

#include <BbUtfUtil.h>

#include <string>
#include <vector>

#include <windows.h>

namespace WebCore {

namespace {
class ClipboardGuard {
    // A simple scope guard mechanism to ensure we are good citizens and close the
    // clipboard when we are done or if an issue occurs

    bool d_isOpen;
  public:
    ClipboardGuard(HWND owner)
        : d_isOpen(false)
    {
        if (OpenClipboard(owner)) {
            d_isOpen = true;
        }
    }
    ~ClipboardGuard()
    {
        release();
    }
    void release()
    {
        if (d_isOpen) {
            CloseClipboard();
            d_isOpen = false;
        }
    }
    bool isOpen()
    {
        return d_isOpen;
    }
};
} // close anonymous namespace

bool saveHBitmapIntoClipboard(HBITMAP   sourceBitmap,
                              const int width,
                              const int height)
{
    // see ui/base/clipboard/clipboard_win.cc for details
    HDC dc = ::GetDC(NULL);
    HDC compatible_dc = ::CreateCompatibleDC(NULL);
    HDC source_dc = ::CreateCompatibleDC(NULL);

    // This is the HBITMAP we will eventually write to the clipboard
    HBITMAP hbitmap = ::CreateCompatibleBitmap(dc, width, height);
    if (!hbitmap) {
        // Failed to create the bitmap
        ::DeleteDC(compatible_dc);
        ::DeleteDC(source_dc);
        ::ReleaseDC(NULL, dc);
        return false;
    }

    HBITMAP old_hbitmap = (HBITMAP)SelectObject(compatible_dc, hbitmap);
    HBITMAP old_source = (HBITMAP)SelectObject(source_dc, sourceBitmap);

    // Now we need to blend it into an HBITMAP we can place on the clipboard
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    ::GdiAlphaBlend(compatible_dc, 0, 0, width, height,
                    source_dc, 0, 0, width, height, bf);

    // Clean up all the handles we just opened
    ::SelectObject(compatible_dc, old_hbitmap);
    ::SelectObject(source_dc, old_source);
    ::DeleteDC(compatible_dc);
    ::DeleteDC(source_dc);
    ::ReleaseDC(NULL, dc);

    ClipboardGuard clipboardGuard(GetClipboardOwner());
    EmptyClipboard();
    HANDLE ret = SetClipboardData(CF_BITMAP, hbitmap);
    if (!ret) {
        ::DeleteObject(hbitmap);
    }
    return ret != 0;
}

bool getStandardClipboardFormatName(std::string *formatNameString, UINT uformat)
{
    //KITDIAG_ASSERT(formatNameString);
    switch (uformat) {
        case CF_TEXT:            { (*formatNameString) = "CF_TEXT";            break; }
        case CF_BITMAP:          { (*formatNameString) = "CF_BITMAP";          break; }
        case CF_METAFILEPICT:    { (*formatNameString) = "CF_METAFILEPICT";    break; }
        case CF_SYLK:            { (*formatNameString) = "CF_SYLK";            break; }
        case CF_DIF:             { (*formatNameString) = "CF_DIF";             break; }
        case CF_TIFF:            { (*formatNameString) = "CF_TIFF";            break; }
        case CF_OEMTEXT:         { (*formatNameString) = "CF_OEMTEXT";         break; }
        case CF_DIB:             { (*formatNameString) = "CF_DIB";             break; }
        case CF_PALETTE:         { (*formatNameString) = "CF_PALETTE";         break; }
        case CF_PENDATA:         { (*formatNameString) = "CF_PENDATA";         break; }
        case CF_RIFF:            { (*formatNameString) = "CF_RIFF";            break; }
        case CF_WAVE:            { (*formatNameString) = "CF_WAVE";            break; }
        case CF_UNICODETEXT:     { (*formatNameString) = "CF_UNICODETEXT";     break; }
        case CF_ENHMETAFILE:     { (*formatNameString) = "CF_ENHMETAFILE";     break; }
        case CF_HDROP:           { (*formatNameString) = "CF_HDROP";           break; }
        case CF_LOCALE:          { (*formatNameString) = "CF_LOCALE";          break; }
        case CF_DIBV5:           { (*formatNameString) = "CF_DIBV5";           break; }
        case CF_MAX:             { (*formatNameString) = "CF_MAX";             break; }
        case CF_OWNERDISPLAY:    { (*formatNameString) = "CF_OWNERDISPLAY";    break; }
        case CF_DSPTEXT:         { (*formatNameString) = "CF_DSPTEXT";         break; }
        case CF_DSPBITMAP:       { (*formatNameString) = "CF_DSPBITMAP";       break; }
        case CF_DSPMETAFILEPICT: { (*formatNameString) = "CF_DSPMETAFILEPICT"; break; }
        case CF_DSPENHMETAFILE:  { (*formatNameString) = "CF_DSPENHMETAFILE";  break; }
        default:
            return false;
    }
    return true;
}

UINT getClipboardFormat(const char *formatName)
{
         if (strcmp(formatName, "CF_TEXT")            == 0) { return CF_TEXT; }
    else if (strcmp(formatName, "CF_BITMAP")          == 0) { return CF_BITMAP; }
    else if (strcmp(formatName, "CF_METAFILEPICT")    == 0) { return CF_METAFILEPICT; }
    else if (strcmp(formatName, "CF_SYLK")            == 0) { return CF_SYLK; }
    else if (strcmp(formatName, "CF_DIF")             == 0) { return CF_DIF; }
    else if (strcmp(formatName, "CF_TIFF")            == 0) { return CF_TIFF; }
    else if (strcmp(formatName, "CF_OEMTEXT")         == 0) { return CF_OEMTEXT; }
    else if (strcmp(formatName, "CF_DIB")             == 0) { return CF_DIB; }
    else if (strcmp(formatName, "CF_PALETTE")         == 0) { return CF_PALETTE; }
    else if (strcmp(formatName, "CF_PENDATA")         == 0) { return CF_PENDATA; }
    else if (strcmp(formatName, "CF_RIFF")            == 0) { return CF_RIFF; }
    else if (strcmp(formatName, "CF_WAVE")            == 0) { return CF_WAVE; }
    else if (strcmp(formatName, "CF_UNICODETEXT")     == 0) { return CF_UNICODETEXT; }
    else if (strcmp(formatName, "CF_ENHMETAFILE")     == 0) { return CF_ENHMETAFILE; }
    else if (strcmp(formatName, "CF_HDROP")           == 0) { return CF_HDROP; }
    else if (strcmp(formatName, "CF_LOCALE")          == 0) { return CF_LOCALE; }
    else if (strcmp(formatName, "CF_DIBV5")           == 0) { return CF_DIBV5; }
    else if (strcmp(formatName, "CF_MAX")             == 0) { return CF_MAX; }
    else if (strcmp(formatName, "CF_OWNERDISPLAY")    == 0) { return CF_OWNERDISPLAY; }
    else if (strcmp(formatName, "CF_DSPTEXT")         == 0) { return CF_DSPTEXT; }
    else if (strcmp(formatName, "CF_DSPBITMAP")       == 0) { return CF_DSPBITMAP; }
    else if (strcmp(formatName, "CF_DSPMETAFILEPICT") == 0) { return CF_DSPMETAFILEPICT; }
    else if (strcmp(formatName, "CF_DSPENHMETAFILE")  == 0) { return CF_DSPENHMETAFILE; }
    else {
        return RegisterClipboardFormatA(formatName);
    }
}

bool hasUnsignedFormat(unsigned format)
{
    ClipboardGuard clipboardGuard(GetClipboardOwner());
    if (!clipboardGuard.isOpen()) {
        return false;
    }

    bool didFind = false;
    UINT enumFormat = EnumClipboardFormats(0);
    while (enumFormat && !didFind) {
        if (enumFormat == format) {
            didFind = true;
        }
        enumFormat = EnumClipboardFormats(enumFormat);
    }

    return didFind;
}

bool ClipboardUtil::getAvailableClipboardFormats(std::vector<std::string> *formatsArray)
{
    //KITDIAG_ASSERT(formatsArray);
    ClipboardGuard clipboardGuard(GetClipboardOwner());
    if (!clipboardGuard.isOpen()) {
        return false;
    }

    formatsArray->clear();
    char formatName[100];
    std::string formatNameString;
    UINT uFormat = EnumClipboardFormats(0);
    while (uFormat) {
        if (getStandardClipboardFormatName(&formatNameString, uFormat)) {
            formatsArray->push_back(formatNameString);
        }
        else if (GetClipboardFormatNameA(uFormat, formatName, sizeof(formatName))) {
            formatNameString = formatName;
            formatsArray->push_back(formatNameString);
        }
        uFormat = EnumClipboardFormats(uFormat);
    }
    return true;
}

bool ClipboardUtil::hasFormat(const std::string& format)
{
    UINT clipFormat = getClipboardFormat(format.c_str());
    if (clipFormat == 0) {
        return false;
    }

    return hasUnsignedFormat(clipFormat);
}

bool ClipboardUtil::getTextFromClipboard(std::string *result, unsigned format)
{
    //KITDIAG_ASSERT(result);

    if (!hasUnsignedFormat(format)) {
        return false;
    }

    ClipboardGuard clipboardGuard(GetClipboardOwner());
    if (!clipboardGuard.isOpen()) {
        return false;
    }

    bool foundIt = false;
    HANDLE hData = GetClipboardData(format);
    LPVOID pData = GlobalLock(hData);
    if (pData) {
        if (format == CF_TEXT) {
            *result = (char*)pData;
            foundIt = true;
        }
        else if (format == CF_UNICODETEXT) {
            std::wstring wideStr = (wchar_t*)pData;
            UtfUtil::utf16ToUtf8(wideStr, result);
            foundIt = true;
        }
    }
    GlobalUnlock(hData);
    return foundIt;
}

bool ClipboardUtil::getTextFromClipboard(std::string *result, const char *format)
{
    //KITDIAG_ASSERT(result);

    UINT clipFormat = getClipboardFormat(format);
    if (clipFormat == 0) {
        return false;
    }

    return getTextFromClipboard(result, clipFormat);
}

bool ClipboardUtil::getStringData(std::string *result, const std::string& format)
{
    //KITDIAG_ASSERT(result);

    UINT clipFormat = getClipboardFormat(format.c_str());
    if (clipFormat == 0) {
        return false;
    }

    if (!hasUnsignedFormat(clipFormat)) {
        return false;
    }

    ClipboardGuard clipboardGuard(GetClipboardOwner());
    if (!clipboardGuard.isOpen()) {
        return false;
    }
    HANDLE hData = GetClipboardData(clipFormat);
    LPVOID pData = GlobalLock(hData);
    *result = (char*)pData;
    GlobalUnlock(hData);
    return true;
}

bool ClipboardUtil::getWStringData(std::wstring *result, const std::string& format)
{
    //KITDIAG_ASSERT(result);

    UINT clipFormat = getClipboardFormat(format.c_str());
    if (clipFormat == 0) {
        return false;
    }

    if (!hasUnsignedFormat(clipFormat)) {
        return false;
    }

    ClipboardGuard clipboardGuard(GetClipboardOwner());
    if (!clipboardGuard.isOpen()) {
        return false;
    }
    HANDLE hData = GetClipboardData(clipFormat);
    LPVOID pData = GlobalLock(hData);
    *result = (wchar_t *)pData;
    GlobalUnlock(hData);
    return true;
}

bool ClipboardUtil::setStringData(const std::string& data, const std::string& format)
{
    UINT clipFormat = getClipboardFormat(format.c_str());
    if (clipFormat == 0) {
        return false;
    }
    {
        ClipboardGuard clipboardGuard(NULL);
        if (!clipboardGuard.isOpen()) {
            return false;
        }
        HGLOBAL clipBuffer;
        char *buffer;
        clipBuffer = GlobalAlloc(GHND, data.length()+1);
        buffer = static_cast<char *>(GlobalLock(clipBuffer));
        strncpy_s(buffer, data.length()+1, data.c_str(), data.length());
        GlobalUnlock(clipBuffer);
        HANDLE ret = SetClipboardData(clipFormat, clipBuffer);
        if (ret == NULL) {
            // Set failed, free the buffer
            GlobalFree(clipBuffer);
            return false;
        }
    }
    return true;
}

bool ClipboardUtil::setWStringData(const std::wstring& data, const std::string& format)
{
    UINT clipFormat = getClipboardFormat(format.c_str());
    if (clipFormat == 0) {
        return false;
    }
    {
        ClipboardGuard clipboardGuard(NULL);
        if (!clipboardGuard.isOpen()) {
            return false;
        }
        HGLOBAL clipBuffer;
        wchar_t *buffer;
        clipBuffer = GlobalAlloc(GHND, (data.length()+1)*sizeof(wchar_t));
        buffer = static_cast<wchar_t *>(GlobalLock(clipBuffer));
        wcsncpy_s(buffer, data.length()+1, data.c_str(), data.length());
        GlobalUnlock(clipBuffer);
        HANDLE ret = SetClipboardData(clipFormat, clipBuffer);
        if (ret == NULL) {
            // Set failed, free the buffer
            GlobalFree(clipBuffer);
            return false;
        }
    }
    return true;

}

bool ClipboardUtil::setCustomData(const void         *pData,
                                  size_t              len,
                                  const std::string&  format)
{
    //KITDIAG_ASSERT(pData);
    UINT clipFormat = getClipboardFormat(format.c_str());
    if (clipFormat == 0) {
        return false;
    }
    {
        ClipboardGuard clipboardGuard(NULL);
        if (!clipboardGuard.isOpen()) {
            return false;
        }
        HGLOBAL clipBuffer;
        void * buffer;
        clipBuffer = GlobalAlloc(GHND, len);
        buffer = static_cast<void *>(GlobalLock(clipBuffer));
        memcpy_s(buffer, len, pData, len);
        GlobalUnlock(clipBuffer);
        HANDLE ret = SetClipboardData(clipFormat, clipBuffer);
        if (ret == NULL) {
            // Set failed, free the buffer
            GlobalFree(clipBuffer);
            return false;
        }
    }
    return true;
}

bool ClipboardUtil::setBitmapData(const unsigned char* rgbData,
                                  const int            width,
                                  const int            height,
                                  const int            bytesPerPixel)
{
    const int bitsPerPixel = bytesPerPixel * 8;

    HDC dc = ::GetDC(NULL);

    // Create an HBITMAP
    BITMAPINFO bm_info = {0};
    bm_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bm_info.bmiHeader.biWidth = width;
    bm_info.bmiHeader.biHeight = -height;  // sets vertical orientation
    bm_info.bmiHeader.biPlanes = 1;
    bm_info.bmiHeader.biBitCount = (WORD)bitsPerPixel;
    bm_info.bmiHeader.biCompression = BI_RGB;

    // We can't write the created bitmap into the clipboard.
    // We have to create a new one and save that into the clipboard.
    void *bits;
    HBITMAP sourceBitmap =
        ::CreateDIBSection(dc, &bm_info, DIB_RGB_COLORS, &bits, NULL, 0);

    bool ret = false;
    if (bits && sourceBitmap) {
        // Copy the bitmap out of shared memory and into GDI
        memcpy(bits, rgbData, 4 *width * height);
        ret = saveHBitmapIntoClipboard(sourceBitmap, width, height);
    }

    ::DeleteObject(sourceBitmap);
    ::ReleaseDC(NULL, dc);

    return ret;
}

bool ClipboardUtil::emptyClipboard()
{
    ClipboardGuard clipboardGuard(GetClipboardOwner());
    if (!clipboardGuard.isOpen()) {
        return false;
    }
    BOOL ret = EmptyClipboard();
    //KITDIAG_ASSERT(ret != 0);
    return ret != 0;
}

}  // close namespace WebCore

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2012
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// --------------------------------- END-OF-FILE -----------------------------
