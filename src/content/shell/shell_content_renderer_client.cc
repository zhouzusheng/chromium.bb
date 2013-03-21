// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_content_renderer_client.h"

// SHEZ: Remove upstream test-specific includes here.
#include "base/callback.h"
#include "base/command_line.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "content/public/renderer/render_view.h"
#include "content/shell/shell_render_process_observer.h"
#include "content/shell/shell_switches.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebPluginParams.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "v8/include/v8.h"

using WebKit::WebFrame;
// SHEZ: Remove upstream test-specific "using" statements here.

namespace content {

namespace {

bool IsLocalhost(const std::string& host) {
  return host == "127.0.0.1" || host == "localhost";
}

bool HostIsUsedBySomeTestsToGenerateError(const std::string& host) {
  return host == "255.255.255.255";
}

bool IsExternalPage(const GURL& url) {
  return !url.host().empty() &&
         (url.SchemeIs(chrome::kHttpScheme) ||
          url.SchemeIs(chrome::kHttpsScheme)) &&
         !IsLocalhost(url.host()) &&
         !HostIsUsedBySomeTestsToGenerateError(url.host());
}

}  // namespace

ShellContentRendererClient::ShellContentRendererClient() {
}

ShellContentRendererClient::~ShellContentRendererClient() {
}

void ShellContentRendererClient::RenderThreadStarted() {
  shell_observer_.reset(new ShellRenderProcessObserver());
}

void ShellContentRendererClient::RenderViewCreated(RenderView* render_view) {
  // SHEZ: remove upstream code here, used only for testing
}

bool ShellContentRendererClient::OverrideCreatePlugin(
    RenderView* render_view,
    WebKit::WebFrame* frame,
    const WebKit::WebPluginParams& params,
    WebKit::WebPlugin** plugin) {
  std::string mime_type = params.mimeType.utf8();
  if (mime_type == content::kBrowserPluginMimeType) {
    // Allow browser plugin in content_shell only if it is forced by flag.
    // Returning true here disables the plugin.
    return !CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kEnableBrowserPluginForAllViewTypes);
  }
  return false;
}

bool ShellContentRendererClient::WillSendRequest(
    WebFrame* frame,
    PageTransition transition_type,
    const GURL& url,
    const GURL& first_party_for_cookies,
    GURL* new_url) {
  // SHEZ: Remove upstream DumpRenderTree code here, used only for testing.
  return false;
}

// SHEZ: Remove upstream WebTestProxyCreated function here, used only for testing.

}  // namespace content
