// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_content_renderer_client.h"

// SHEZ: Remove upstream test-specific includes here.
#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_view.h"
#include "content/shell/shell_render_process_observer.h"
#include "content/shell/shell_switches.h"
#include "third_party/WebKit/Source/Platform/chromium/public/WebMediaStreamCenter.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebPluginParams.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebPluginParams.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "v8/include/v8.h"
#include "webkit/mocks/mock_webhyphenator.h"
#include "webkit/tools/test_shell/mock_webclipboard_impl.h"
#include "webkit/tools/test_shell/test_shell_webmimeregistry_impl.h"

using WebKit::WebClipboard;
using WebKit::WebFrame;
using WebKit::WebHyphenator;
using WebKit::WebMediaStreamCenter;
using WebKit::WebMediaStreamCenterClient;
using WebKit::WebMimeRegistry;
using WebKit::WebPlugin;
using WebKit::WebPluginParams;
using WebKit::WebRTCPeerConnectionHandler;
using WebKit::WebRTCPeerConnectionHandlerClient;
using WebKit::WebThemeEngine;
// SHEZ: Remove upstream test-specific "using" statements here.

namespace content {

ShellContentRendererClient::ShellContentRendererClient() {
  // SHEZ: remove upstream code here, used only for testing
}

ShellContentRendererClient::~ShellContentRendererClient() {
}

void ShellContentRendererClient::LoadHyphenDictionary(
    base::PlatformFile dict_file) {
  if (!hyphenator_)
    hyphenator_.reset(new webkit_glue::MockWebHyphenator);
  base::SeekPlatformFile(dict_file, base::PLATFORM_FILE_FROM_BEGIN, 0);
  hyphenator_->LoadDictionary(dict_file);
}

void ShellContentRendererClient::RenderThreadStarted() {
  shell_observer_.reset(new ShellRenderProcessObserver());
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif
}

void ShellContentRendererClient::RenderViewCreated(RenderView* render_view) {
  // SHEZ: remove upstream code here, used only for testing
}

bool ShellContentRendererClient::OverrideCreatePlugin(
    RenderView* render_view,
    WebFrame* frame,
    const WebPluginParams& params,
    WebPlugin** plugin) {
  std::string mime_type = params.mimeType.utf8();
  if (mime_type == content::kBrowserPluginMimeType) {
    // Allow browser plugin in content_shell only if it is forced by flag.
    // Returning true here disables the plugin.
    return !CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kEnableBrowserPluginForAllViewTypes);
  }
  return false;
}

WebMediaStreamCenter*
ShellContentRendererClient::OverrideCreateWebMediaStreamCenter(
    WebMediaStreamCenterClient* client) {
  // SHEZ: remove upstream code here, used only for testing
  return NULL;
}

WebRTCPeerConnectionHandler*
ShellContentRendererClient::OverrideCreateWebRTCPeerConnectionHandler(
    WebRTCPeerConnectionHandlerClient* client) {
  // SHEZ: remove upstream code here, used only for testing
  return NULL;
}

WebClipboard* ShellContentRendererClient::OverrideWebClipboard() {
  // SHEZ: remove upstream code here, used only for testing
  return NULL;
}

WebMimeRegistry* ShellContentRendererClient::OverrideWebMimeRegistry() {
  // SHEZ: remove upstream code here, used only for testing
  return NULL;
}

WebHyphenator* ShellContentRendererClient::OverrideWebHyphenator() {
  // SHEZ: remove upstream code here, used only for testing
  return NULL;
}

WebThemeEngine* ShellContentRendererClient::OverrideThemeEngine() {
  // SHEZ: remove upstream code here, used only for testing
  return NULL;
}

// SHEZ: remove upstream code here, used only for testing

bool ShellContentRendererClient::AllowBrowserPlugin(
    WebKit::WebPluginContainer* container) const {
  if (CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableBrowserPluginForAllViewTypes)) {
    // Allow BrowserPlugin if forced by command line flag. This is generally
    // true for tests.
    return true;
  }
  return ContentRendererClient::AllowBrowserPlugin(container);
}

}  // namespace content
