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

#include <blpwtk2_contextmenuparams.h>

#include <base/logging.h>

#include <vector>

namespace blpwtk2 {

struct SpellSuggestionsData {
    std::vector<String> d_suggestions;
};

class CustomItems {
public:
    CustomItems(int count) : d_items(count) {}
    int count() const { return d_items.size(); }
    const ContextMenuItem& item(int i) const { return d_items[i]; }
    ContextMenuItem& item(int i) { return d_items[i]; }
private:
    std::vector<ContextMenuItem> d_items;
};

ContextMenuParams::ContextMenuParams()
: d_customItems(0)
, d_suggestions(0)
{
}

ContextMenuParams::ContextMenuParams(const ContextMenuParams& other)
: d_misspelledWord(other.d_misspelledWord)
, d_pointOnScreen(other.d_pointOnScreen)
, d_canCut(other.d_canCut)
, d_canCopy(other.d_canCopy)
, d_canPaste(other.d_canPaste)
, d_canDelete(other.d_canDelete)
{
    if (!other.d_customItems) {
        d_customItems = 0;
    } else {
        d_customItems = new CustomItems(*other.d_customItems);
    }

    if (!other.d_suggestions) {
        d_suggestions = 0;
    } else {
        d_suggestions = new SpellSuggestionsData(*other.d_suggestions);
    }
}

ContextMenuParams& ContextMenuParams::operator=(const ContextMenuParams& other)
{
    if (this == &other) {
        return *this;
    }
    d_misspelledWord = other.d_misspelledWord;
    d_pointOnScreen = other.d_pointOnScreen;
    d_canCut = other.d_canCut;
    d_canCopy = other.d_canCopy;
    d_canPaste = other.d_canPaste;
    d_canDelete = other.d_canDelete;
    if (d_customItems) {
        delete d_customItems;
    }
    if (!other.d_customItems) {
        d_customItems = 0;
    } else {
        d_customItems = new CustomItems(*other.d_customItems);
    }
    if (d_suggestions) {
        delete d_suggestions;
    }
    if (!other.d_suggestions) {
        d_suggestions = 0;
    } else {
        d_suggestions = new SpellSuggestionsData(*other.d_suggestions);
    }
    return *this;
}

ContextMenuParams::~ContextMenuParams()
{
    if (d_customItems) {
        delete d_customItems;
        d_customItems = 0;
    }
    if (d_suggestions) {
        delete d_suggestions;
        d_suggestions = 0;
    }
}

int ContextMenuParams::numCustomItems() const
{
    if (d_customItems) {
        return d_customItems->count();
    }
    return 0;
}

void ContextMenuParams::setNumCustomItems(const int count)
{
    DCHECK(d_customItems == 0);
    if (count > 0) {
        d_customItems = new CustomItems(count);
    }
}

const ContextMenuItem& ContextMenuParams::customItem(const int index) const
{
    DCHECK(d_customItems != 0);
    DCHECK(index >= 0 && index < d_customItems->count());
    return d_customItems->item(index);
}

ContextMenuItem& ContextMenuParams::customItem(const int index)
{
    DCHECK(d_customItems != 0);
    DCHECK(index >= 0 && index < d_customItems->count());
    return d_customItems->item(index);
}

int ContextMenuParams::numSpellSuggestions() const
{
    return d_suggestions ? d_suggestions->d_suggestions.size() : 0;
}

void ContextMenuParams::setNumSpellSuggestions(int count)
{
    DCHECK(d_suggestions == 0);
    if (count > 0) {
        d_suggestions = new SpellSuggestionsData();
        d_suggestions->d_suggestions.resize(count);
    }
}

const String& ContextMenuParams::spellSuggestion(int index) const
{
    DCHECK(d_suggestions != 0);
    DCHECK(index >= 0 && index < (int)d_suggestions->d_suggestions.size());
    return d_suggestions->d_suggestions[index];
}

String& ContextMenuParams::spellSuggestion(int index)
{
    DCHECK(d_suggestions != 0);
    DCHECK(index >= 0 && index < (int)d_suggestions->d_suggestions.size());
    return d_suggestions->d_suggestions[index];
}

}

