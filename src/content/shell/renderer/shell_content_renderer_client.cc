// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/renderer/shell_content_renderer_client.h"

#include "base/command_line.h"
#include "components/web_cache/renderer/web_cache_render_process_observer.h"
#include "content/public/renderer/render_thread.h"
#include "content/shell/renderer/shell_render_view_observer.h"
#include "ppapi/shared_impl/ppapi_switches.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "v8/include/v8.h"

#include "chrome/renderer/printing/print_web_view_helper.h"
#include "chrome/renderer/spellchecker/spellcheck.h"
#include "chrome/renderer/spellchecker/spellcheck_provider.h"

namespace content {

ShellContentRendererClient::ShellContentRendererClient() {
}

ShellContentRendererClient::~ShellContentRendererClient() {
}

void ShellContentRendererClient::RenderThreadStarted() {
  RenderThread* thread = RenderThread::Get();
  spellcheck_.reset(new SpellCheck());
  thread->AddObserver(spellcheck_.get());
  web_cache_observer_.reset(new web_cache::WebCacheRenderProcessObserver());
  thread->AddObserver(web_cache_observer_.get());
}

void ShellContentRendererClient::RenderViewCreated(RenderView* render_view) {
  new ShellRenderViewObserver(render_view);
  new SpellCheckProvider(render_view, spellcheck_.get());
  new printing::PrintWebViewHelper(render_view);
}

bool ShellContentRendererClient::IsPluginAllowedToUseCompositorAPI(
    const GURL& url) {
  // SHEZ: Remove test code.
#if 0
  return CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kEnablePepperTesting);
#endif
  return false;
}

bool ShellContentRendererClient::IsPluginAllowedToUseDevChannelAPIs() {
  // SHEZ: Remove test code.
#if 0
  return CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kEnablePepperTesting);
#endif
  return false;
}

}  // namespace content
