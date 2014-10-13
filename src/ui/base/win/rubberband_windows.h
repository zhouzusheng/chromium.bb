// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_RUBBERBAND_WINDOWS_H_
#define UI_BASE_WIN_RUBBERBAND_WINDOWS_H_

#include "base/macros.h"
#include "ui/base/ui_base_export.h"

#include <windows.h>

namespace gfx {
class Rect;
}  // namespace gfx

namespace ui {

class RubberbandWindow {
 public:
  RubberbandWindow();
  ~RubberbandWindow();

  void Init(HWND parent, const gfx::Rect& bounds);
  HWND hwnd() const;
  void set_window_style(DWORD style);

 private:
  void* impl_;

  DISALLOW_COPY_AND_ASSIGN(RubberbandWindow);
};

class UI_BASE_EXPORT RubberbandOutline {
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
