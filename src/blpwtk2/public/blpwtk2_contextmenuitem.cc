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

#include <blpwtk2_contextmenuitemimpl.h>
#include <blpwtk2_stringref.h>

#include <base/logging.h>

namespace blpwtk2 {

ContextMenuItemImpl* getContextMenuItemImpl(ContextMenuItem& obj)
{
    return obj.d_impl;
}

const ContextMenuItemImpl* getContextMenuItemImpl(const ContextMenuItem& obj)
{
    return obj.d_impl;
}

ContextMenuItem::ContextMenuItem()
: d_impl(new ContextMenuItemImpl())
{
}

ContextMenuItem::~ContextMenuItem()
{
    delete d_impl;
}

ContextMenuItem::ContextMenuItem(const ContextMenuItem& other)
: d_impl(new ContextMenuItemImpl(*other.d_impl))
{
}

ContextMenuItem& ContextMenuItem::operator= (const ContextMenuItem& other)
{
    if (this != &other) {
        *d_impl = *other.d_impl;
    }
    return *this;
}

StringRef ContextMenuItem::label() const
{
    return d_impl->d_label;
}

StringRef ContextMenuItem::tooltip() const
{
    return d_impl->d_tooltip;
}

ContextMenuItem::Type ContextMenuItem::type() const
{
    return static_cast<Type>(d_impl->d_type);
}

unsigned ContextMenuItem::action() const
{
    return d_impl->d_action;
}

TextDirection::Value ContextMenuItem::textDirection() const
{
    return d_impl->d_textDirection;
}

bool ContextMenuItem::hasDirectionalOverride() const
{
    return d_impl->d_hasDirectionalOverride;
}

bool ContextMenuItem::enabled() const
{
    return d_impl->d_enabled;
}

bool ContextMenuItem::checked() const
{
    return d_impl->d_checked;
}

int ContextMenuItem::numSubMenuItems() const
{
    return d_impl->d_submenu.size();
}

const ContextMenuItem& ContextMenuItem::subMenuItem(int index) const
{
    DCHECK(index >= 0 && index < (int)d_impl->d_submenu.size());
    return d_impl->d_submenu[index];
}

};
