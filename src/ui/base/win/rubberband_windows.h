// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_RUBBERBAND_WINDOWS_H_
#define UI_BASE_WIN_RUBBERBAND_WINDOWS_H_

#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <atlcrack.h>

#include "base/logging.h"
#include "ui/base/ui_export.h"
#include "ui/gfx/win/window_impl.h"

namespace ui {

class RubberbandWindow : public gfx::WindowImpl {
 public:
  RubberbandWindow();
  virtual ~RubberbandWindow();

  BEGIN_MSG_MAP_EX(RubberbandWindow)
    MSG_WM_PAINT(OnPaint)
  END_MSG_MAP()

 private:
  LRESULT OnPaint(HDC);

  DISALLOW_COPY_AND_ASSIGN(RubberbandWindow);
};

class UI_EXPORT RubberbandOutline {
 public:
  RubberbandOutline();
  ~RubberbandOutline();

  void SetRect(HWND parent, RECT rect);

private:
  RubberbandWindow windows_[4];

  DISALLOW_COPY_AND_ASSIGN(RubberbandOutline);
};

}  // namespace ui

#endif  // UI_BASE_WIN_WINDOW_IMPL_H_
