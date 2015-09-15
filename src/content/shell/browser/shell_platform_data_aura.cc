// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/browser/shell_platform_data_aura.h"

#include "content/shell/browser/shell.h"
#include "ui/aura/client/default_capture_client.h"
#include "ui/aura/env.h"
#include "ui/aura/layout_manager.h"

// SHEZ: Remove test-only code
// #include "ui/aura/test/test_focus_client.h"
// #include "ui/aura/test/test_window_tree_client.h"

#include "ui/aura/window.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/base/ime/input_method.h"
#include "ui/base/ime/input_method_delegate.h"
#include "ui/base/ime/input_method_factory.h"
#include "ui/gfx/screen.h"
#include "ui/wm/core/default_activation_client.h"

namespace content {

namespace {

class FillLayout : public aura::LayoutManager {
 public:
  explicit FillLayout(aura::Window* root)
      : root_(root) {
  }

  ~FillLayout() override {}

 private:
  // aura::LayoutManager:
  void OnWindowResized() override {}

  void OnWindowAddedToLayout(aura::Window* child) override {
    child->SetBounds(root_->bounds());
  }

  void OnWillRemoveWindowFromLayout(aura::Window* child) override {}

  void OnWindowRemovedFromLayout(aura::Window* child) override {}

  void OnChildWindowVisibilityChanged(aura::Window* child,
                                      bool visible) override {}

  void SetChildBounds(aura::Window* child,
                      const gfx::Rect& requested_bounds) override {
    SetChildBoundsDirect(child, requested_bounds);
  }

  aura::Window* root_;

  DISALLOW_COPY_AND_ASSIGN(FillLayout);
};

}

ShellPlatformDataAura* Shell::platform_ = NULL;

ShellPlatformDataAura::ShellPlatformDataAura(const gfx::Size& initial_size) {
  CHECK(aura::Env::GetInstance());
  host_.reset(aura::WindowTreeHost::Create(gfx::Rect(initial_size)));
  host_->InitHost();
  host_->window()->SetLayoutManager(new FillLayout(host_->window()));

  // SHEZ: Remove test-only code
  // focus_client_.reset(new aura::test::TestFocusClient());
  // aura::client::SetFocusClient(host_->window(), focus_client_.get());

  new wm::DefaultActivationClient(host_->window());
  capture_client_.reset(
      new aura::client::DefaultCaptureClient(host_->window()));

  // SHEZ: Remove test-only code
  // window_tree_client_.reset(
  //     new aura::test::TestWindowTreeClient(host_->window()));
}

ShellPlatformDataAura::~ShellPlatformDataAura() {
}

void ShellPlatformDataAura::ShowWindow() {
  host_->Show();
}

void ShellPlatformDataAura::ResizeWindow(const gfx::Size& size) {
  host_->SetBounds(gfx::Rect(size));
}

}  // namespace content
