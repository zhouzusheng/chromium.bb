// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/win/rubberband_windows.h"
#include "ui/gfx/win/window_impl.h"

namespace ui {

namespace {

class RubberbandWindowImpl : public gfx::WindowImpl {
 public:
  RubberbandWindowImpl();
  virtual ~RubberbandWindowImpl();

  CR_BEGIN_MSG_MAP_EX(RubberbandWindowImpl)
    CR_MSG_WM_PAINT(OnPaint)
    CR_MSG_WM_ERASEBKGND(OnEraseBkgnd)
  CR_END_MSG_MAP()

 private:
  LRESULT OnPaint(HDC);
  LRESULT OnEraseBkgnd(HDC);

  DISALLOW_COPY_AND_ASSIGN(RubberbandWindowImpl);
};

static HPEN s_pen = NULL;

RubberbandWindowImpl::RubberbandWindowImpl() {
  if (!s_pen) {
    LOGBRUSH lb = { BS_SOLID, RGB(255, 255, 255), 0 };
    DWORD style[2] = { 2, 4 };
    s_pen = ::ExtCreatePen(PS_GEOMETRIC | PS_USERSTYLE, 1, &lb, 2, style);
  }
}

RubberbandWindowImpl::~RubberbandWindowImpl() {
  DestroyWindow(hwnd());
}

LRESULT RubberbandWindowImpl::OnPaint(HDC /*_dc*/) {
  RECT rect;
  ::GetClientRect(hwnd(), &rect);

  PAINTSTRUCT ps;
  HDC dc = ::BeginPaint(hwnd(), &ps);

  ::FillRect(dc, &rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));

  HGDIOBJ oldPen = ::SelectObject(dc, s_pen);
  ::MoveToEx(dc, rect.left, rect.top, NULL);
  if (rect.right - rect.left == 1) {
    ::LineTo(dc, rect.right-1, rect.bottom);
  } else {
    ::LineTo(dc, rect.right, rect.bottom-1);
  }
  ::SelectObject(dc, oldPen);

  ::EndPaint(hwnd(), &ps);

  return S_OK;
}

LRESULT RubberbandWindowImpl::OnEraseBkgnd(HDC) {
  return 1;
}

static RubberbandWindowImpl* toWindowImpl(void* impl) {
  return reinterpret_cast<RubberbandWindowImpl*>(impl);
}

}  // close anonymous namespace

RubberbandWindow::RubberbandWindow()
: impl_(new RubberbandWindowImpl()) {
}

RubberbandWindow::~RubberbandWindow() {
  delete toWindowImpl(impl_);
}

void RubberbandWindow::Init(HWND parent, const gfx::Rect& bounds) {
  toWindowImpl(impl_)->Init(parent, bounds);
}

HWND RubberbandWindow::hwnd() const {
  return toWindowImpl(impl_)->hwnd();
}

void RubberbandWindow::set_window_style(DWORD style) {
  toWindowImpl(impl_)->set_window_style(style);
}

RubberbandOutline::RubberbandOutline() {
}

RubberbandOutline::~RubberbandOutline() {
}

static void SetRubberbandWindowRect(RubberbandWindow& window, HWND parent,
                                    int x, int y, int w, int h) {
  if (window.hwnd() == NULL) {
    window.set_window_style(WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE);
    window.Init(parent, gfx::Rect(x, y, w, h));
    ::BringWindowToTop(window.hwnd());
  }
  else {
    ::SetWindowPos(window.hwnd(),
                   NULL,
                   x, y, w, h,
                   SWP_NOZORDER | SWP_NOACTIVATE);
    ::InvalidateRect(window.hwnd(), NULL, FALSE);
  }
}

void RubberbandOutline::SetRect(HWND parent, RECT rect) {
  int w = rect.right - rect.left;
  int h = rect.bottom - rect.top;
  SetRubberbandWindowRect(windows_[0], parent, rect.left,  rect.top,    w, 1);
  SetRubberbandWindowRect(windows_[1], parent, rect.right, rect.top,    1, h);
  SetRubberbandWindowRect(windows_[2], parent, rect.left,  rect.bottom, w, 1);
  SetRubberbandWindowRect(windows_[3], parent, rect.left,  rect.top,    1, h);
}

}  // namespace ui
