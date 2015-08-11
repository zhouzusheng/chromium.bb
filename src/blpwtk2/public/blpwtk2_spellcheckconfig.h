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

#ifndef INCLUDED_BLPWTK2_SPELLCHECKCONFIG_H
#define INCLUDED_BLPWTK2_SPELLCHECKCONFIG_H

#include <blpwtk2_config.h>

namespace blpwtk2 {

class StringRef;
struct SpellCheckConfigImpl;

class BLPWTK2_EXPORT SpellCheckConfig {
  public:
    SpellCheckConfig();
    SpellCheckConfig(const SpellCheckConfig& other);
    ~SpellCheckConfig();
    SpellCheckConfig& operator=(const SpellCheckConfig& rhs);

    // Enable or disable spellchecking.  Note that the other properties in the
    // spellcheck config have no effect if spellchecking is not enabled.
    void enableSpellCheck(bool enabled);

    // Set the list of languages to be used when spellchecking.  Note that
    // blpwtk2 does not automatically download dictionary files, so it is the
    // application's responsibility to ensure the all dictionary files exist in
    // the dictionary directory.  The dictionary directory can be set from
    // blpwtk2::Toolkit.
    void setLanguages(const StringRef *languages, size_t numLanguages);

    bool isSpellCheckEnabled() const;
    size_t numLanguages() const;
    StringRef languageAt(size_t index) const;

  private:
    SpellCheckConfigImpl* d_impl;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_SPELLCHECKCONFIG_H
