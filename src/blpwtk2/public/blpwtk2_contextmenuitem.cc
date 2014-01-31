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

#include <blpwtk2_contextmenuitem.h>

#include <base/logging.h>

#include <vector>

namespace blpwtk2 {

class ContextMenuSubItems {
public:
    ContextMenuSubItems(int count) : d_items(count) {}
    int count() const { return d_items.size(); }
    const ContextMenuItem& item(int i) const { return d_items[i]; }
    ContextMenuItem& item(int i) { return d_items[i]; }
private:
    std::vector<ContextMenuItem> d_items;
};

ContextMenuItem::ContextMenuItem()
: d_type(OPTION)
, d_action(0)
, d_submenu(0)
{
}

ContextMenuItem::~ContextMenuItem()
{
    if (d_submenu) {
        delete d_submenu;
        d_submenu = 0;
    }
}

ContextMenuItem::ContextMenuItem(const ContextMenuItem& other)
: d_label(other.d_label)
, d_tooltip(other.d_tooltip)
, d_type(other.d_type)
, d_action(other.d_action)
, d_textDirection(other.d_textDirection)
, d_hasDirectionalOverride(other.d_hasDirectionalOverride)
, d_enabled(other.d_enabled)
, d_checked(other.d_checked)
{
    if (!other.d_submenu) {
        d_submenu = 0;
    } else {
        d_submenu = new ContextMenuSubItems(*other.d_submenu);
    }
}

ContextMenuItem& ContextMenuItem::operator= (const ContextMenuItem& other)
{
    if (this == &other) {
        return *this;
    }
    d_label = other.d_label;
    d_tooltip = other.d_tooltip;
    d_type = other.d_type;
    d_action = other.d_action;
    d_textDirection = other.d_textDirection;
    d_hasDirectionalOverride = other.d_hasDirectionalOverride;
    d_enabled = other.d_enabled;
    d_checked = other.d_checked;
    if (d_submenu) {
        delete d_submenu;
    }
    if (!other.d_submenu) {
        d_submenu = 0;
    } else {
        d_submenu = new ContextMenuSubItems(*other.d_submenu);
    }
    return *this;
}

int ContextMenuItem::numSubMenuItems() const
{
    if (d_submenu) {
        return d_submenu->count();
    }
    return 0;
}
void ContextMenuItem::setNumSubMenuItems(const int count)
{
    DCHECK(d_submenu == 0);
    if (count != 0) {
         d_submenu = new ContextMenuSubItems(count);
    }
}

const ContextMenuItem& ContextMenuItem::subMenuItem(const int index) const
{
    DCHECK(d_submenu != 0);
    DCHECK(index >= 0 && index < d_submenu->count());
    return d_submenu->item(index);
}

ContextMenuItem& ContextMenuItem::subMenuItem(const int index)
{
    DCHECK(d_submenu != 0);
    DCHECK(index >= 0 && index < d_submenu->count());
    return d_submenu->item(index);
}

};
