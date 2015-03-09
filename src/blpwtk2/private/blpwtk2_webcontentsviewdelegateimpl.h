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

#ifndef INCLUDED_BLPWTK2_WEBCONTENTSVIEWDELEGATEIMPL_H
#define INCLUDED_BLPWTK2_WEBCONTENTSVIEWDELEGATEIMPL_H

#include <blpwtk2_config.h>

#include <base/compiler_specific.h>  // for OVERRIDE
#include <content/public/browser/web_contents_view_delegate.h>

namespace content {
    class WebContents;
}  // close namespace content

namespace blpwtk2 {

// This is our implementation of the content::WebContentsViewDelegate
// interface.  Right now, we just use it to hook up the context menu.
class WebContentsViewDelegateImpl : public content::WebContentsViewDelegate {
  public:
    WebContentsViewDelegateImpl(content::WebContents* webContents);

    // ====== WebContentsViewDelegate overrides

    // Returns a delegate to process drags not handled by content.
    virtual content::WebDragDestDelegate* GetDragDestDelegate() OVERRIDE;

    // Shows a context menu.
    virtual void ShowContextMenu(
        content::RenderFrameHost* renderFrameHost,
        const content::ContextMenuParams& params) OVERRIDE;

    // These methods allow the embedder to intercept WebContentsViewWin's
    // implementation of these WebContentsView methods. See the WebContentsView
    // interface documentation for more information about these methods.
    virtual void StoreFocus() OVERRIDE;
    virtual void RestoreFocus() OVERRIDE;
    virtual bool Focus() OVERRIDE;
    virtual void TakeFocus(bool reverse) OVERRIDE;
    virtual void SizeChanged(const gfx::Size& size) OVERRIDE;

#if defined(TOOLKIT_VIEWS) || defined(USE_AURA)
  virtual void ShowDisambiguationPopup(
      const gfx::Rect& target_rect,
      const SkBitmap& zoomed_bitmap,
      const gfx::NativeView content,
      const base::Callback<void(ui::GestureEvent*)>& gesture_cb,
      const base::Callback<void(ui::MouseEvent*)>& mouse_cb) OVERRIDE;
  virtual void HideDisambiguationPopup() OVERRIDE;
#endif

  private:
    content::WebContents* d_webContents;

    DISALLOW_COPY_AND_ASSIGN(WebContentsViewDelegateImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBCONTENTSVIEWDELEGATEIMPL_H

