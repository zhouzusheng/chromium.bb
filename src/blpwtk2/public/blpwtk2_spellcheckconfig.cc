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

#include <blpwtk2_spellcheckconfig.h>

#include <blpwtk2_stringref.h>

#include <base/logging.h>  // for DCHECK

#include <string>
#include <vector>

namespace blpwtk2 {

struct SpellCheckConfigImpl {
    bool d_isSpellCheckEnabled;
    bool d_isAutoCorrectEnabled;
    std::vector<std::string> d_languages;
    std::vector<std::string> d_customWords;

    SpellCheckConfigImpl()
    : d_isSpellCheckEnabled(false)
    , d_isAutoCorrectEnabled(false)
    {
    }
};

SpellCheckConfig::SpellCheckConfig()
: d_impl(new SpellCheckConfigImpl())
{
}

SpellCheckConfig::SpellCheckConfig(const SpellCheckConfig& other)
: d_impl(new SpellCheckConfigImpl(*other.d_impl))
{
}

SpellCheckConfig::~SpellCheckConfig()
{
    delete d_impl;
}

SpellCheckConfig& SpellCheckConfig::operator=(const SpellCheckConfig& rhs)
{
    if (this != &rhs) {
        *d_impl = *rhs.d_impl;
    }
    return *this;
}


void SpellCheckConfig::enableSpellCheck(bool enable)
{
    d_impl->d_isSpellCheckEnabled = enable;
}

void SpellCheckConfig::enableAutoCorrect(bool enable)
{
    d_impl->d_isAutoCorrectEnabled = enable;
}

void SpellCheckConfig::setLanguages(const StringRef *languages, size_t numLanguages)
{
    d_impl->d_languages.resize(numLanguages);
    for (size_t i = 0; i < numLanguages; ++i) {
        d_impl->d_languages[i].assign(languages[i].data(), languages[i].length());
    }
}

void SpellCheckConfig::setCustomWords(const StringRef *words, size_t numWords)
{
    d_impl->d_customWords.resize(numWords);
    for (size_t i = 0; i < numWords; ++i) {
        d_impl->d_customWords[i].assign(words[i].data(), words[i].length());
    }
}

bool SpellCheckConfig::isSpellCheckEnabled() const
{
    return d_impl->d_isSpellCheckEnabled;
}

bool SpellCheckConfig::isAutoCorrectEnabled() const
{
    return d_impl->d_isAutoCorrectEnabled;
}

size_t SpellCheckConfig::numLanguages() const
{
    return d_impl->d_languages.size();
}

StringRef SpellCheckConfig::languageAt(size_t index) const
{
    DCHECK(index < d_impl->d_languages.size());
    return d_impl->d_languages[index];
}

size_t SpellCheckConfig::numCustomWords() const
{
    return d_impl->d_customWords.size();
}

StringRef SpellCheckConfig::customWordAt(size_t index) const
{
    DCHECK(index < d_impl->d_customWords.size());
    return d_impl->d_customWords[index];
}

}  // close namespace blpwtk2
