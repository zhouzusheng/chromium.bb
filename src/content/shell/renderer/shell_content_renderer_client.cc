// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/renderer/shell_content_renderer_client.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "components/web_cache/renderer/web_cache_render_process_observer.h"
#include "content/common/sandbox_win.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"

// SHEZ: Disable the following (used for test only)
//#include "content/public/test/layouttest_support.h"

#include "content/shell/common/shell_switches.h"
#include "content/shell/common/webkit_test_helpers.h"
#include "content/shell/renderer/shell_render_frame_observer.h"
#include "content/shell/renderer/shell_render_process_observer.h"
#include "content/shell/renderer/shell_render_view_observer.h"

// SHEZ: Disable the following (used for test only)
// #include "content/shell/renderer/test_runner/mock_credential_manager_client.h"
// #include "content/shell/renderer/test_runner/web_test_interfaces.h"
// #include "content/shell/renderer/test_runner/web_test_proxy.h"
// #include "content/shell/renderer/webkit_test_runner.h"
// #include "content/test/mock_webclipboard_impl.h"

#include "ppapi/shared_impl/ppapi_switches.h"
#include "third_party/WebKit/public/platform/WebMediaStreamCenter.h"
#include "third_party/WebKit/public/web/WebPluginParams.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "v8/include/v8.h"

#if defined(OS_WIN)
#include "content/public/renderer/render_font_warmup_win.h"
#include "third_party/WebKit/public/web/win/WebFontRendering.h"
#include "third_party/skia/include/ports/SkFontMgr.h"
#endif

#include "chrome/renderer/printing/print_web_view_helper.h"
#include "chrome/renderer/spellchecker/spellcheck.h"
#include "chrome/renderer/spellchecker/spellcheck_provider.h"

using blink::WebAudioDevice;
using blink::WebClipboard;
using blink::WebLocalFrame;
using blink::WebMIDIAccessor;
using blink::WebMIDIAccessorClient;
using blink::WebMediaStreamCenter;
using blink::WebMediaStreamCenterClient;
using blink::WebPlugin;
using blink::WebPluginParams;
using blink::WebRTCPeerConnectionHandler;
using blink::WebRTCPeerConnectionHandlerClient;
using blink::WebThemeEngine;

namespace content {

#if defined(OS_WIN)
namespace {

// DirectWrite only has access to %WINDIR%\Fonts by default. For developer
// side-loading, support kRegisterFontFiles to allow access to additional fonts.
void RegisterSideloadedTypefaces(SkFontMgr* fontmgr) {
#if 0
  // TODO(SHEZ): Fix this.

  std::vector<std::string> files = GetSideloadFontFiles();
  for (std::vector<std::string>::const_iterator i(files.begin());
       i != files.end();
       ++i) {
    SkTypeface* typeface = fontmgr->createFromFile(i->c_str());
    DoPreSandboxWarmupForTypeface(typeface);
    blink::WebFontRendering::addSideloadedFontForTesting(typeface);
  }
#endif
}

}  // namespace
#endif  // OS_WIN

ShellContentRendererClient::ShellContentRendererClient() {
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree)) {
    // SHEZ: Disable the following (used for test only)
    //EnableWebTestProxyCreation(
    //    base::Bind(&ShellContentRendererClient::WebTestProxyCreated,
    //               base::Unretained(this)));
  }

#if defined(OS_WIN)
  if (ShouldUseDirectWrite())
    RegisterSideloadedTypefaces(GetPreSandboxWarmupFontMgr());
#endif
}

ShellContentRendererClient::~ShellContentRendererClient() {
}

void ShellContentRendererClient::RenderThreadStarted() {
  RenderThread* thread = RenderThread::Get();
  shell_observer_.reset(new ShellRenderProcessObserver());
  spellcheck_.reset(new SpellCheck());
  thread->AddObserver(spellcheck_.get());
  web_cache_observer_.reset(new web_cache::WebCacheRenderProcessObserver());
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif
  thread->AddObserver(web_cache_observer_.get());
}

void ShellContentRendererClient::RenderFrameCreated(RenderFrame* render_frame) {
  new ShellRenderFrameObserver(render_frame);
}

void ShellContentRendererClient::RenderViewCreated(RenderView* render_view) {
  new ShellRenderViewObserver(render_view);
  new SpellCheckProvider(render_view, spellcheck_.get());
  new printing::PrintWebViewHelper(render_view);

  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;
  // SHEZ: Remove test code.
#if 0
  WebKitTestRunner* test_runner = WebKitTestRunner::Get(render_view);
  test_runner->Reset();
  render_view->GetWebView()->setSpellCheckClient(
      test_runner->proxy()->GetSpellCheckClient());

  render_view->GetWebView()->setCredentialManagerClient(
      test_runner->proxy()->GetCredentialManagerClientMock());
  WebTestDelegate* delegate =
      ShellRenderProcessObserver::GetInstance()->test_delegate();
  if (delegate == static_cast<WebTestDelegate*>(test_runner))
    ShellRenderProcessObserver::GetInstance()->SetMainWindow(render_view);
#endif
}

bool ShellContentRendererClient::OverrideCreatePlugin(
    RenderFrame* render_frame,
    WebLocalFrame* frame,
    const WebPluginParams& params,
    WebPlugin** plugin) {
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
  return interfaces->CreateMediaStreamCenter(client);
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
  return interfaces->CreateWebRTCPeerConnectionHandler(client);
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
  return interfaces->CreateMIDIAccessor(client);
#else
  return 0;
#endif
}

WebAudioDevice*
ShellContentRendererClient::OverrideCreateAudioDevice(
    double sample_rate) {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return NULL;
  // SHEZ: Remove test code
#if 0
  WebTestInterfaces* interfaces =
      ShellRenderProcessObserver::GetInstance()->test_interfaces();
  return interfaces->CreateAudioDevice(sample_rate);
#else
  return NULL;
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
  return ShellRenderProcessObserver::GetInstance()
      ->test_interfaces()
      ->ThemeEngine();
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
  proxy->SetInterfaces(
      ShellRenderProcessObserver::GetInstance()->test_interfaces());
  test_runner->proxy()->SetDelegate(
      ShellRenderProcessObserver::GetInstance()->test_delegate());
#endif
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

bool ShellContentRendererClient::IsPluginAllowedToUseVideoDecodeAPI(
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
