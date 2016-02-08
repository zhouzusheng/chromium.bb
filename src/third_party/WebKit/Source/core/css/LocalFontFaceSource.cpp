// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "core/css/LocalFontFaceSource.h"

#include "platform/fonts/FontCache.h"
#include "platform/fonts/FontDescription.h"
#include "platform/fonts/SimpleFontData.h"
#include "public/platform/Platform.h"

namespace blink {

static void adjustedFontDescriptionForBoldItalic(FontDescription& fontDescription,
                                                 WTF::String& fontName)
{
    if (fontName.endsWith(" Italic")) {
        fontDescription.setStyle(FontStyleItalic);
        fontName = fontName.substring(0, fontName.length() - 7);
    }
    if (fontName.endsWith(" Bold")) {
        fontDescription.setWeight(FontWeightBold);
        fontName = fontName.substring(0, fontName.length() - 5);
    }
}

LocalFontFaceSource::LocalFontFaceSource(const String& fontName)
    : m_fontName(fontName)
    , m_needToAdjustForBoldItalic(fontName.endsWith(" Bold") || fontName.endsWith(" Italic"))
{
}

bool LocalFontFaceSource::isLocalFontAvailable(const FontDescription& fontDescription)
{
    if (m_needToAdjustForBoldItalic) {
        FontDescription adjustedFontDescription = fontDescription;
        WTF::String adjustedFontName = m_fontName.string();
        adjustedFontDescriptionForBoldItalic(adjustedFontDescription, adjustedFontName);
        return FontCache::fontCache()->isPlatformFontAvailable(adjustedFontDescription,
                                                               WTF::AtomicString(adjustedFontName));
    }
    return FontCache::fontCache()->isPlatformFontAvailable(fontDescription, m_fontName);
}

PassRefPtr<SimpleFontData> LocalFontFaceSource::createFontData(const FontDescription& fontDescription)
{
    // We don't want to check alternate font family names here, so pass true as the checkingAlternateName parameter.
    RefPtr<SimpleFontData> fontData;
    if (m_needToAdjustForBoldItalic) {
        FontDescription adjustedFontDescription = fontDescription;
        WTF::String adjustedFontName = m_fontName.string();
        adjustedFontDescriptionForBoldItalic(adjustedFontDescription, adjustedFontName);
        fontData = FontCache::fontCache()->getFontData(adjustedFontDescription,
                                                       WTF::AtomicString(adjustedFontName),
                                                       true);
    }
    else {
        fontData = FontCache::fontCache()->getFontData(fontDescription, m_fontName, true);
    }
    m_histograms.record(fontData);
    return fontData.release();
}

void LocalFontFaceSource::LocalFontHistograms::record(bool loadSuccess)
{
    if (m_reported)
        return;
    m_reported = true;
    Platform::current()->histogramEnumeration("WebFont.LocalFontUsed", loadSuccess ? 1 : 0, 2);
}

} // namespace blink
