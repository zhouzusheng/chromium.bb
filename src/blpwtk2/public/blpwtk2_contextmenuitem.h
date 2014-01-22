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

#ifndef INCLUDED_BLPWTK2_CONTEXTMENUITEM_H
#define INCLUDED_BLPWTK2_CONTEXTMENUITEM_H

#include <blpwtk2_config.h>
#include <blpwtk2_string.h>
#include <blpwtk2_textdirection.h>

namespace blpwtk2 {

class ContextMenuSubItems;

//This class represents a single context menu item.
class BLPWTK2_EXPORT ContextMenuItem {

  public:
    enum Type {
        OPTION,
        CHECKABLE_OPTION,
        GROUP,
        SEPARATOR,
        SUBMENU
    };

    ContextMenuItem();
    ~ContextMenuItem();
    ContextMenuItem(const ContextMenuItem& other);
    ContextMenuItem& operator= (const ContextMenuItem& other);

    String label() const { return d_label;}
    void setLabel(const String& label) { d_label = label;}

    String tooltip() const { return d_tooltip; }
    void setTooltip(const String& tooltip) { d_tooltip = tooltip; }

    Type type() const { return d_type; }
    void setType(Type type) { d_type = type; }

    unsigned action() const { return d_action; }
    void setAction(unsigned action) { d_action = action; }

    TextDirection::Value textDirection() const { return d_textDirection; }
    void setTextDirection(TextDirection::Value textDirection) { d_textDirection = textDirection; }

    bool hasDirectionalOverride() const { return d_hasDirectionalOverride; }
    void setHasDirectionalOverride(bool hasDirectionalOverride) { d_hasDirectionalOverride = hasDirectionalOverride; }

    bool enabled() const { return d_enabled; }
    void setEnabled(bool enabled) { d_enabled = enabled; }

    bool checked() const { return d_checked; }
    void setChecked(bool checked) { d_checked = checked; }

    int numSubMenuItems() const;
    void setNumSubMenuItems(int count);

    const ContextMenuItem& subMenuItem(int index) const;
    ContextMenuItem& subMenuItem(int index);

  private:
    String d_label;
    String d_tooltip;
    Type d_type;
    unsigned d_action;
    TextDirection::Value d_textDirection;
    bool d_hasDirectionalOverride;
    bool d_enabled;
    bool d_checked;
    ContextMenuSubItems* d_submenu;

};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_CONTEXTMENUITEM_H

