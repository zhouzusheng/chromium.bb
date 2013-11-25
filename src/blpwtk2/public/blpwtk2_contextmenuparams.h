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

#ifndef INCLUDED_BLPWTK2_CONTEXTMENUPARAMS_H
#define INCLUDED_BLPWTK2_CONTEXTMENUPARAMS_H

#include <blpwtk2_config.h>
#include <blpwtk2_contextmenuitem.h>

namespace blpwtk2 {

class CustomItems;
struct SpellSuggestionsData;

// This class contains parameters that are passed to the application whenever
// the user right clicks or presses the "Show Menu" key inside a WebView.  The
// application can use this information to show an appropriate context menu.
class BLPWTK2_EXPORT ContextMenuParams {
  public:
    ContextMenuParams();
    ~ContextMenuParams();

    ContextMenuParams(const ContextMenuParams& other);
    ContextMenuParams& operator=(const ContextMenuParams& other);

    void setPointOnScreen(const POINT& pt) { d_pointOnScreen = pt; }
    const POINT& pointOnScreen() const { return d_pointOnScreen; }

    void setCanCut(bool can) { d_canCut = can; }
    bool canCut() const { return d_canCut; }

    void setCanCopy(bool can) { d_canCopy = can; }
    bool canCopy() const { return d_canCopy; }

    void setCanPaste(bool can) { d_canPaste = can; }
    bool canPaste() const { return d_canPaste; }

    void setCanDelete(bool can) { d_canDelete = can; }
    bool canDelete() const { return d_canDelete; }

    int numCustomItems() const;
    void setNumCustomItems(int count);

    const ContextMenuItem& customItem(int index) const;
    ContextMenuItem& customItem(int index);

    int numSpellSuggestions() const;
    void setNumSpellSuggestions(int count);

    const String& spellSuggestion(int index) const;
    String& spellSuggestion(int index);

  private:
    POINT d_pointOnScreen;
    bool d_canCut;
    bool d_canCopy;
    bool d_canPaste;
    bool d_canDelete;
    CustomItems* d_customItems;
    SpellSuggestionsData *d_suggestions;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_CONTEXTMENUPARAMS_H

