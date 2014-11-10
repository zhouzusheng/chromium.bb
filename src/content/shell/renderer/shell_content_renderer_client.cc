// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/renderer/shell_content_renderer_client.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"

// SHEZ: Disable the following (used for test only)
//#include "content/public/test/layouttest_support.h"

#include "content/shell/common/shell_switches.h"
#include "content/shell/renderer/shell_render_frame_observer.h"
#include "content/shell/renderer/shell_render_process_observer.h"
#include "content/shell/renderer/shell_render_view_observer.h"

// SHEZ: Disable the following (used for test only)
// #include "content/shell/renderer/test_runner/WebTestInterfaces.h"
// #include "content/shell/renderer/test_runner/WebTestProxy.h"
// #include "content/shell/renderer/test_runner/WebTestRunner.h"
// #include "content/shell/renderer/webkit_test_runner.h"
// #include "content/test/mock_webclipboard_impl.h"

#include "third_party/WebKit/public/platform/WebMediaStreamCenter.h"
#include "third_party/WebKit/public/web/WebPluginParams.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "v8/include/v8.h"

#include "chrome/renderer/spellchecker/spellcheck.h"
#include "chrome/renderer/spellchecker/spellcheck_provider.h"

using blink::WebAudioDevice;
using blink::WebClipboard;
using blink::WebFrame;
using blink::WebMIDIAccessor;
using blink::WebMIDIAccessorClient;
using blink::WebMediaStreamCenter;
using blink::WebMediaStreamCenterClient;
using blink::WebPlugin;
using blink::WebPluginParams;
using blink::WebRTCPeerConnectionHandler;
using blink::WebRTCPeerConnectionHandlerClient;
using blink::WebThemeEngine;
using WebTestRunner::WebTestDelegate;
using WebTestRunner::WebTestInterfaces;
using WebTestRunner::WebTestProxyBase;

namespace content {

ShellContentRendererClient::ShellContentRendererClient() {
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree)) {
    // SHEZ: Disable the following (used for test only)
    //EnableWebTestProxyCreation(
    //    base::Bind(&ShellContentRendererClient::WebTestProxyCreated,
    //               base::Unretained(this)));
  }
}

ShellContentRendererClient::~ShellContentRendererClient() {
}

void ShellContentRendererClient::RenderThreadStarted() {
  shell_observer_.reset(new ShellRenderProcessObserver());
  spellcheck_.reset(new SpellCheck());
  RenderThread* thread = RenderThread::Get();
  thread->AddObserver(spellcheck_.get());
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif
}

void ShellContentRendererClient::RenderFrameCreated(RenderFrame* render_frame) {
  new ShellRenderFrameObserver(render_frame);
}

void ShellContentRendererClient::RenderViewCreated(RenderView* render_view) {
  new ShellRenderViewObserver(render_view);
  new SpellCheckProvider(render_view, spellcheck_.get());

  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;
  // SHEZ: Remove test code.
#if 0
  WebKitTestRunner* test_runner = WebKitTestRunner::Get(render_view);
  test_runner->Reset();
  render_view->GetWebView()->setSpellCheckClient(
      test_runner->proxy()->spellCheckClient());
  WebTestDelegate* delegate =
      ShellRenderProcessObserver::GetInstance()->test_delegate();
  if (delegate == static_cast<WebTestDelegate*>(test_runner))
    ShellRenderProcessObserver::GetInstance()->SetMainWindow(render_view);
#endif
}

bool ShellContentRendererClient::OverrideCreatePlugin(
    RenderFrame* render_frame,
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
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
  // SHEZ: Remove test-only code.
#if 0 && defined(ENABLE_WEBRTC)
  WebTestInterfaces* interfaces =
      ShellRenderProcessObserver::GetInstance()->test_interfaces();
  return interfaces->createMediaStreamCenter(client);
#else
  return NULL;
#endif
}

WebRTCPeerConnectionHandler*
ShellContentRendererClient::OverrideCreateWebRTCPeerConnectionHandler(
    WebRTCPeerConnectionHandlerClient* client) {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
  // SHEZ: Remove test-only code.
#if 0 && defined(ENABLE_WEBRTC)
  WebTestInterfaces* interfaces =
      ShellRenderProcessObserver::GetInstance()->test_interfaces();
  return interfaces->createWebRTCPeerConnectionHandler(client);
#else
  return NULL;
#endif
}

WebMIDIAccessor*
ShellContentRendererClient::OverrideCreateMIDIAccessor(
    WebMIDIAccessorClient* client) {
  // SHEZ: Remove test code
#if 0
  WebTestInterfaces* interfaces =
      ShellRenderProcessObserver::GetInstance()->test_interfaces();
  return interfaces->createMIDIAccessor(client);
#else
  return 0;
#endif
}

WebAudioDevice*
ShellContentRendererClient::OverrideCreateAudioDevice(
    double sample_rate) {
  // SHEZ: Remove test code
#if 0
  WebTestInterfaces* interfaces =
      ShellRenderProcessObserver::GetInstance()->test_interfaces();
  return interfaces->createAudioDevice(sample_rate);
#else
  return 0;
#endif
}

WebClipboard* ShellContentRendererClient::OverrideWebClipboard() {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
  // SHEZ: Remove test code.
#if 0
  if (!clipboard_)
    clipboard_.reset(new MockWebClipboardImpl);
  return clipboard_.get();
#else
  return NULL;
#endif
}

WebThemeEngine* ShellContentRendererClient::OverrideThemeEngine() {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
  // SHEZ: Remove test code.
#if 0
  return ShellRenderProcessObserver::GetInstance()->test_interfaces()
      ->themeEngine();
#else
  return NULL;
#endif
}

void ShellContentRendererClient::WebTestProxyCreated(RenderView* render_view,
                                                     WebTestProxyBase* proxy) {
  // SHEZ: Remove test code.
#if 0
  WebKitTestRunner* test_runner = new WebKitTestRunner(render_view);
  test_runner->set_proxy(proxy);
  if (!ShellRenderProcessObserver::GetInstance()->test_delegate())
    ShellRenderProcessObserver::GetInstance()->SetTestDelegate(test_runner);
  proxy->setInterfaces(
      ShellRenderProcessObserver::GetInstance()->test_interfaces());
  test_runner->proxy()->setDelegate(
      ShellRenderProcessObserver::GetInstance()->test_delegate());
#endif
}

bool ShellContentRendererClient::AllowBrowserPlugin(
    blink::WebPluginContainer* container) {
  if (CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableBrowserPluginForAllViewTypes)) {
    // Allow BrowserPlugin if forced by command line flag. This is generally
    // true for tests.
    return true;
  }
  return ContentRendererClient::AllowBrowserPlugin(container);
}

}  // namespace content
