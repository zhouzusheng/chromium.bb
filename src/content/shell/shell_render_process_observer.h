// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_SHELL_RENDER_PROCESS_OBSERVER_H_
#define CONTENT_SHELL_SHELL_RENDER_PROCESS_OBSERVER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/renderer/render_process_observer.h"

namespace WebKit {
class WebFrame;
}

// SHEZ: Remove upstream test-specific forward declarations here.

namespace content {

class RenderView;
// SHEZ: Remove upstream test-specific forward declaration here.

class ShellRenderProcessObserver : public RenderProcessObserver {
 public:
  static ShellRenderProcessObserver* GetInstance();

  ShellRenderProcessObserver();
  virtual ~ShellRenderProcessObserver();

  // SHEZ: Remove upstream test-specific code here.
  void SetMainWindow(RenderView* view);

  // RenderProcessObserver implementation.
  virtual void WebKitInitialized() OVERRIDE;
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

  // SHEZ: Remove upstream test-specific code here.
  const FilePath& webkit_source_dir() const { return webkit_source_dir_; }

 private:
  // Message handlers.
  // SHEZ: Removed upstream test-specific code here.
  void OnSetWebKitSourceDir(const FilePath& webkit_source_dir);

  // SHEZ: Remove upstream test-specific data-members here.
  RenderView* main_render_view_;

  base::FilePath webkit_source_dir_;

  DISALLOW_COPY_AND_ASSIGN(ShellRenderProcessObserver);
};

}  // namespace content

#endif  // CONTENT_SHELL_SHELL_RENDER_PROCESS_OBSERVER_H_
