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

#include <blpwtk2_webcontentsviewdelegateimpl.h>

#include <blpwtk2_contextmenuparams.h>
#include <blpwtk2_contextmenuitem.h>
#include <blpwtk2_webviewimpl.h>
#include <blpwtk2_textdirection.h>

#include <content/public/browser/web_contents.h>
#include <content/public/common/context_menu_params.h>
#include <third_party/WebKit/public/web/WebContextMenuData.h>
#include <webkit/common/webmenuitem.h>

namespace {

void convertItem(const WebMenuItem& item1, blpwtk2::ContextMenuItem& item2);

void convertSubmenus(const WebMenuItem& item1, blpwtk2::ContextMenuItem& item2)
{
    item2.setNumSubMenuItems(item1.submenu.size());
    for (size_t i = 0; i < item1.submenu.size(); ++i) {
        convertItem(item1.submenu[i], item2.subMenuItem(i));
    }
}

void convertCustomItems(const content::ContextMenuParams& params, blpwtk2::ContextMenuParams& params2)
{
    params2.setNumCustomItems(params.custom_items.size());
    for (size_t i = 0; i <params.custom_items.size(); ++i) {
        convertItem(params.custom_items[i], params2.customItem(i));
    }
}

void convertItem(const WebMenuItem& item1, blpwtk2::ContextMenuItem& item2)
{
    item2.setLabel(blpwtk2::String(item1.label));
    item2.setTooltip(blpwtk2::String(item1.toolTip));
    switch (item1.type) {
    case WebKit::WebMenuItemInfo::Option: item2.setType(blpwtk2::ContextMenuItem::OPTION); break;
    case WebKit::WebMenuItemInfo::CheckableOption: item2.setType(blpwtk2::ContextMenuItem::CHECKABLE_OPTION); break;
    case WebKit::WebMenuItemInfo::Group: item2.setType(blpwtk2::ContextMenuItem::GROUP); break;
    case WebKit::WebMenuItemInfo::Separator: item2.setType(blpwtk2::ContextMenuItem::SEPARATOR); break;
    case WebKit::WebMenuItemInfo::SubMenu: item2.setType(blpwtk2::ContextMenuItem::SUBMENU); break;
    }
    item2.setAction(item1.action);
    item2.setTextDirection(item1.rtl ? blpwtk2::TextDirection::RIGHT_TO_LEFT : blpwtk2::TextDirection::LEFT_TO_RIGHT);
    item2.setHasDirectionalOverride(item1.has_directional_override);
    item2.setEnabled(item1.enabled);
    item2.setChecked(item1.checked);
    convertSubmenus(item1, item2);
}

void convertSpellcheck(const content::ContextMenuParams& params, blpwtk2::ContextMenuParams& params2)
{
    params2.setMisspelledWord(blpwtk2::String(params.misspelled_word));
    params2.setNumSpellSuggestions(params.dictionary_suggestions.size());
    for (std::size_t i = 0; i < params.dictionary_suggestions.size(); ++i) {
        params2.spellSuggestion(i) = blpwtk2::String(params.dictionary_suggestions[i]);
    }
}

} // close unnamed namespace

namespace blpwtk2 {

WebContentsViewDelegateImpl::WebContentsViewDelegateImpl(content::WebContents* webContents)
: d_webContents(webContents)
{
}

content::WebDragDestDelegate*
WebContentsViewDelegateImpl::GetDragDestDelegate()
{
    return 0;
}

void WebContentsViewDelegateImpl::ShowContextMenu(
            const content::ContextMenuParams& params)
{
    WebViewImpl* webViewImpl = static_cast<WebViewImpl*>(d_webContents->GetDelegate());
    webViewImpl->saveCustomContextMenuContext(params.custom_context);

    POINT point = { params.x, params.y };
    ClientToScreen(webViewImpl->getNativeView(), &point);

    bool hasSelection = !params.selection_text.empty();

    ContextMenuParams params2;
    params2.setPointOnScreen(point);
    params2.setCanCut(params.is_editable && (params.edit_flags & WebKit::WebContextMenuData::CanCut));
    params2.setCanCopy(hasSelection || (params.is_editable && (params.edit_flags & WebKit::WebContextMenuData::CanCopy)));
    params2.setCanPaste(params.is_editable && (params.edit_flags & WebKit::WebContextMenuData::CanPaste));
    params2.setCanDelete(params.is_editable && (params.edit_flags & WebKit::WebContextMenuData::CanDelete));

    convertCustomItems(params, params2);
    convertSpellcheck(params, params2);

    webViewImpl->showContextMenu(params2);
}

void WebContentsViewDelegateImpl::StoreFocus()
{
}

void WebContentsViewDelegateImpl::RestoreFocus()
{
}

bool WebContentsViewDelegateImpl::Focus()
{
    return false;
}

void WebContentsViewDelegateImpl::TakeFocus(bool reverse)
{
}

void WebContentsViewDelegateImpl::SizeChanged(const gfx::Size& size)
{
}

}  // close namespace blpwtk2

