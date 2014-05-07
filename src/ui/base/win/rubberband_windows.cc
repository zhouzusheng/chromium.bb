// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/win/rubberband_windows.h"

namespace ui {

RubberbandWindow::RubberbandWindow() {
}

RubberbandWindow::~RubberbandWindow() {
  DestroyWindow(hwnd());
}

LRESULT RubberbandWindow::OnPaint(HDC /*_dc*/) {
  CRect rect;
  ::GetClientRect(hwnd(), &rect);

  PAINTSTRUCT ps;
  HDC dc = ::BeginPaint(hwnd(), &ps);

  COLORREF oldbg = ::SetBkColor(dc, RGB(0,0,0));
  static const char* space = " ";
  ::ExtTextOutA(dc, rect.left, rect.top, ETO_CLIPPED|ETO_OPAQUE, rect, space, 1, NULL);
  ::SetBkColor(dc, oldbg);

  LOGBRUSH lb = {BS_SOLID, RGB(255, 255, 255), 0};
  DWORD style[2] = {2, 4};
  HPEN pen = ::ExtCreatePen(PS_GEOMETRIC | PS_USERSTYLE, 1, &lb, 2, style);

  HGDIOBJ oldPen = ::SelectObject(dc, pen);
  ::MoveToEx(dc, rect.left, rect.top, NULL);
  if (rect.Width() == 1) {
    ::LineTo(dc, rect.right-1, rect.bottom);
  } else {
    ::LineTo(dc, rect.right, rect.bottom-1);
  }
  ::SelectObject(dc, oldPen);
  ::DeleteObject(pen);

  ::EndPaint(hwnd(), &ps);

  return S_OK;
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
