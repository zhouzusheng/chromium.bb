// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_render_process_observer.h"

// SHEZ: Removed upstream test-specific includes.
#include "base/command_line.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_thread.h"
#include "content/shell/shell_messages.h"
#include "content/shell/shell_switches.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebRuntimeFeatures.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/support/gc_extension.h"

using WebKit::WebFrame;
using WebKit::WebRuntimeFeatures;

namespace content {

namespace {
ShellRenderProcessObserver* g_instance = NULL;
}

// static
ShellRenderProcessObserver* ShellRenderProcessObserver::GetInstance() {
  return g_instance;
}

ShellRenderProcessObserver::ShellRenderProcessObserver()
    : main_render_view_(NULL)
      // SHEZ: Remove upstream test-specific initializers here.
{
  CHECK(!g_instance);
  g_instance = this;
  RenderThread::Get()->AddObserver(this);
  
  // SHEZ: Remove upstream DumpRenderTree code here, used only for testing.
}

ShellRenderProcessObserver::~ShellRenderProcessObserver() {
  CHECK(g_instance == this);
  g_instance = NULL;
}

// SHEZ: Remove upstream test-specific code here.
void ShellRenderProcessObserver::SetMainWindow(
    RenderView* view) {
  main_render_view_ = view;
}

// SHEZ: Remove upstream test-specific code here.

void ShellRenderProcessObserver::WebKitInitialized() {
  // SHEZ: Remove upstream DumpRenderTree code here, used only for testing.
}

bool ShellRenderProcessObserver::OnControlMessageReceived(
    const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ShellRenderProcessObserver, message)
    // SHEZ: Remove upstream test-specific code here.
    IPC_MESSAGE_HANDLER(ShellViewMsg_SetWebKitSourceDir, OnSetWebKitSourceDir)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

// SHEZ: Remove upstream test-specific code here.

void ShellRenderProcessObserver::OnSetWebKitSourceDir(
    const base::FilePath& webkit_source_dir) {
  webkit_source_dir_ = webkit_source_dir;
}

}  // namespace content
