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

#ifndef INCLUDED_MINIKIT_CONTEXTMENUITEM_H
#define INCLUDED_MINIKIT_CONTEXTMENUITEM_H

#include <minikit_config.h>

#include <minikit_textdirection.h>

namespace minikit {

class ContextMenuItem;
class StringRef;

struct ContextMenuItemImpl;
ContextMenuItemImpl* getContextMenuItemImpl(ContextMenuItem& obj);
const ContextMenuItemImpl* getContextMenuItemImpl(const ContextMenuItem& obj);

//This class represents a single context menu item.
class MINIKIT_EXPORT ContextMenuItem {

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

    StringRef label() const;
    StringRef tooltip() const;
    Type type() const;
    unsigned action() const;
    TextDirection::Value textDirection() const;
    bool hasDirectionalOverride() const;
    bool enabled() const;
    bool checked() const;
    int numSubMenuItems() const;
    const ContextMenuItem& subMenuItem(int index) const;

  private:
    friend ContextMenuItemImpl* getContextMenuItemImpl(ContextMenuItem& obj);
    friend const ContextMenuItemImpl* getContextMenuItemImpl(const ContextMenuItem& obj);

    ContextMenuItemImpl* d_impl;
};

}  // close namespace minikit

#endif  // INCLUDED_MINIKIT_CONTEXTMENUITEM_H

