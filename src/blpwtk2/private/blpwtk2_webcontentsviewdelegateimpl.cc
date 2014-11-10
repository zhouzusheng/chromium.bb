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
#include <blpwtk2_contextmenuparamsimpl.h>
#include <blpwtk2_contextmenuitem.h>
#include <blpwtk2_contextmenuitemimpl.h>
#include <blpwtk2_webviewimpl.h>
#include <blpwtk2_textdirection.h>

#include <base/strings/utf_string_conversions.h>
#include <content/public/browser/web_contents.h>
#include <content/public/common/context_menu_params.h>
#include <content/public/common/menu_item.h>
#include <third_party/WebKit/public/web/WebContextMenuData.h>
#include <ui/aura/window_tree_host.h>
#include <ui/aura/window.h>

namespace {

void convertItem(const content::MenuItem& item1, blpwtk2::ContextMenuItemImpl* item2Impl);

void convertSubmenus(const content::MenuItem& item1, blpwtk2::ContextMenuItemImpl* item2Impl)
{
    item2Impl->d_submenu.resize(item1.submenu.size());
    for (size_t i = 0; i < item1.submenu.size(); ++i) {
        convertItem(item1.submenu[i], getContextMenuItemImpl(item2Impl->d_submenu[i]));
    }
}

void convertCustomItems(const content::ContextMenuParams& params, blpwtk2::ContextMenuParamsImpl* params2Impl)
{
    params2Impl->d_customItems.resize(params.custom_items.size());
    for (size_t i = 0; i <params.custom_items.size(); ++i) {
        convertItem(params.custom_items[i], getContextMenuItemImpl(params2Impl->d_customItems[i]));
    }
}

void convertItem(const content::MenuItem& item1, blpwtk2::ContextMenuItemImpl* item2Impl)
{
    item2Impl->d_label = base::UTF16ToUTF8(item1.label);
    item2Impl->d_tooltip = base::UTF16ToUTF8(item1.tool_tip);
    switch (item1.type) {
    case blink::WebMenuItemInfo::Option: item2Impl->d_type = blpwtk2::ContextMenuItem::OPTION; break;
    case blink::WebMenuItemInfo::CheckableOption: item2Impl->d_type = blpwtk2::ContextMenuItem::CHECKABLE_OPTION; break;
    case blink::WebMenuItemInfo::Group: item2Impl->d_type = blpwtk2::ContextMenuItem::GROUP; break;
    case blink::WebMenuItemInfo::Separator: item2Impl->d_type = blpwtk2::ContextMenuItem::SEPARATOR; break;
    case blink::WebMenuItemInfo::SubMenu: item2Impl->d_type = blpwtk2::ContextMenuItem::SUBMENU; break;
    }
    item2Impl->d_action = item1.action;
    item2Impl->d_textDirection = item1.rtl ? blpwtk2::TextDirection::RIGHT_TO_LEFT : blpwtk2::TextDirection::LEFT_TO_RIGHT;
    item2Impl->d_hasDirectionalOverride = item1.has_directional_override;
    item2Impl->d_enabled = item1.enabled;
    item2Impl->d_checked = item1.checked;
    convertSubmenus(item1, item2Impl);
}

void convertSpellcheck(const content::ContextMenuParams& params, blpwtk2::ContextMenuParamsImpl* params2Impl)
{
    params2Impl->d_misspelledWord = base::UTF16ToUTF8(params.misspelled_word);
    params2Impl->d_suggestions.resize(params.dictionary_suggestions.size());
    for (std::size_t i = 0; i < params.dictionary_suggestions.size(); ++i) {
        params2Impl->d_suggestions[i] = base::UTF16ToUTF8(params.dictionary_suggestions[i]);
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
    content::RenderFrameHost* renderFrameHost,
    const content::ContextMenuParams& params)
{
    WebViewImpl* webViewImpl = static_cast<WebViewImpl*>(d_webContents->GetDelegate());
    webViewImpl->saveCustomContextMenuContext(renderFrameHost, params.custom_context);

    gfx::Point point(params.x, params.y);
    aura::WindowTreeHost* host = webViewImpl->getNativeView()->GetHost();
    host->ConvertPointToNativeScreen(&point);

    bool hasSelection = !params.selection_text.empty();

    ContextMenuParams params2;
    ContextMenuParamsImpl* params2Impl = getContextMenuParamsImpl(params2);
    params2Impl->d_pointOnScreen = point.ToPOINT();
    params2Impl->d_canCut = params.is_editable && (params.edit_flags & blink::WebContextMenuData::CanCut);
    params2Impl->d_canCopy = hasSelection || (params.is_editable && (params.edit_flags & blink::WebContextMenuData::CanCopy));
    params2Impl->d_canPaste = params.is_editable && (params.edit_flags & blink::WebContextMenuData::CanPaste);
    params2Impl->d_canDelete = params.is_editable && (params.edit_flags & blink::WebContextMenuData::CanDelete);

    convertCustomItems(params, params2Impl);
    convertSpellcheck(params, params2Impl);

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

