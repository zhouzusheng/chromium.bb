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

#include <blpwtk2_contextmenuitem.h>
#include <blpwtk2_contextmenuparamsimpl.h>
#include <blpwtk2_stringref.h>

#include <base/logging.h>

#include <vector>

namespace blpwtk2 {

ContextMenuParamsImpl* getContextMenuParamsImpl(ContextMenuParams& obj)
{
    return obj.d_impl;
}

const ContextMenuParamsImpl* getContextMenuParamsImpl(const ContextMenuParams& obj)
{
    return obj.d_impl;
}

ContextMenuParams::ContextMenuParams()
: d_impl(new ContextMenuParamsImpl())
{
}

ContextMenuParams::~ContextMenuParams()
{
    delete d_impl;
}

ContextMenuParams::ContextMenuParams(const ContextMenuParams& other)
: d_impl(new ContextMenuParamsImpl(*other.d_impl))
{
}

ContextMenuParams& ContextMenuParams::operator=(const ContextMenuParams& other)
{
    if (this != &other) {
        *d_impl = *other.d_impl;
    }
    return *this;
}

const POINT& ContextMenuParams::pointOnScreen() const
{
    return d_impl->d_pointOnScreen;
}

bool ContextMenuParams::canCut() const
{
    return d_impl->d_canCut;
}

bool ContextMenuParams::canCopy() const
{
    return d_impl->d_canCopy;
}

bool ContextMenuParams::canPaste() const
{
    return d_impl->d_canPaste;
}

bool ContextMenuParams::canDelete() const
{
    return d_impl->d_canDelete;
}

StringRef ContextMenuParams::misspelledWord() const
{
    return d_impl->d_misspelledWord;
}

int ContextMenuParams::numCustomItems() const
{
    return d_impl->d_customItems.size();
}

const ContextMenuItem& ContextMenuParams::customItem(int index) const
{
    DCHECK(index >= 0 && index < (int)d_impl->d_customItems.size());
    return d_impl->d_customItems[index];
}

int ContextMenuParams::numSpellSuggestions() const
{
    return d_impl->d_suggestions.size();
}

StringRef ContextMenuParams::spellSuggestion(int index) const
{
    DCHECK(index >= 0 && index < (int)d_impl->d_suggestions.size());
    return d_impl->d_suggestions[index];
}

}

