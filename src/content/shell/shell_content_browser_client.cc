// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_content_browser_client.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_thread.h"
#include "content/shell/geolocation/shell_access_token_store.h"
#include "content/shell/shell.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_browser_main_parts.h"
#include "content/shell/shell_devtools_delegate.h"
#include "content/shell/shell_message_filter.h"
#include "content/shell/shell_messages.h"
#include "content/shell/shell_quota_permission_context.h"
#include "content/shell/shell_resource_dispatcher_host_delegate.h"
#include "content/shell/shell_switches.h"
#include "content/shell/shell_web_contents_view_delegate_creator.h"
#include "content/shell/webkit_test_controller.h"
#include "googleurl/src/gurl.h"
#include "webkit/glue/webpreferences.h"

#if defined(OS_ANDROID)
#include "base/android/path_utils.h"
#include "base/path_service.h"
#include "base/platform_file.h"
#include "content/shell/android/shell_descriptors.h"
#endif

namespace content {

namespace {

base::FilePath GetWebKitRootDirFilePath() {
  base::FilePath base_path;
  PathService::Get(base::DIR_SOURCE_ROOT, &base_path);
  if (file_util::PathExists(
          base_path.Append(FILE_PATH_LITERAL("third_party/WebKit")))) {
    // We're in a WebKit-in-chrome checkout.
    return base_path.Append(FILE_PATH_LITERAL("third_party/WebKit"));
  } else if (file_util::PathExists(
          base_path.Append(FILE_PATH_LITERAL("chromium")))) {
    // We're in a WebKit-only checkout on Windows.
    return base_path.Append(FILE_PATH_LITERAL("../.."));
  } else if (file_util::PathExists(
          base_path.Append(FILE_PATH_LITERAL("webkit/support")))) {
    // We're in a WebKit-only/xcodebuild checkout on Mac
    return base_path.Append(FILE_PATH_LITERAL("../../.."));
  }
  // We're in a WebKit-only, make-build, so the DIR_SOURCE_ROOT is already the
  // WebKit root. That, or we have no idea where we are.
  return base_path;
}

base::FilePath GetChromiumRootDirFilePath() {
  base::FilePath webkit_path = GetWebKitRootDirFilePath();
  if (file_util::PathExists(webkit_path.Append(
          FILE_PATH_LITERAL("Source/WebKit/chromium/webkit/support")))) {
    // We're in a WebKit-only checkout.
    return webkit_path.Append(FILE_PATH_LITERAL("Source/WebKit/chromium"));
  } else {
    // We're in a Chromium checkout, and WebKit is in third_party/WebKit.
    return webkit_path.Append(FILE_PATH_LITERAL("../.."));
  }
}

// This class creates the IO thread for the renderer when running in
// single-process mode.  It's not used in multi-process mode.
class RendererMainThread : public base::Thread {
 public:
  explicit RendererMainThread(const std::string& channel_id)
      : Thread("Chrome_InProcRendererThread"),
        channel_id_(channel_id) {
  }

  virtual ~RendererMainThread() {
    Stop();
  }

 protected:
  virtual void Init() OVERRIDE {
    RenderThread::InitInProcessRenderer(channel_id_);
  }

  virtual void CleanUp() OVERRIDE {
    RenderThread::CleanUpInProcessRenderer();

    // It's a little lame to manually set this flag.  But the single process
    // RendererThread will receive the WM_QUIT.  We don't need to assert on
    // this thread, so just force the flag manually.
    // If we want to avoid this, we could create the InProcRendererThread
    // directly with _beginthreadex() rather than using the Thread class.
    // We used to set this flag in the Init function above. However there
    // other threads like WebThread which are created by this thread
    // which resets this flag. Please see Thread::StartWithOptions. Setting
    // this flag to true in Cleanup works around these problems.
    SetThreadWasQuitProperly(true);
  }

 private:
  std::string channel_id_;

  DISALLOW_COPY_AND_ASSIGN(RendererMainThread);
};
RendererMainThread* g_in_process_renderer = 0;

}  // namespace

ShellContentBrowserClient::ShellContentBrowserClient()
    : shell_browser_main_parts_(NULL) {
  // SHEZ: Removed upstream DumpRenderTree code here, used only for testing.
}

ShellContentBrowserClient::~ShellContentBrowserClient() {
}

BrowserMainParts* ShellContentBrowserClient::CreateBrowserMainParts(
    const MainFunctionParams& parameters) {
  shell_browser_main_parts_ = new ShellBrowserMainParts(parameters);
  return shell_browser_main_parts_;
}

void ShellContentBrowserClient::RenderProcessHostCreated(
    RenderProcessHost* host) {
  // SHEZ: Removed upstream DumpRenderTree code here, used only for testing.
}

net::URLRequestContextGetter* ShellContentBrowserClient::CreateRequestContext(
    BrowserContext* content_browser_context,
    ProtocolHandlerMap* protocol_handlers) {
  ShellBrowserContext* shell_browser_context =
      ShellBrowserContextForBrowserContext(content_browser_context);
  return shell_browser_context->CreateRequestContext(protocol_handlers);
}

net::URLRequestContextGetter*
ShellContentBrowserClient::CreateRequestContextForStoragePartition(
    BrowserContext* content_browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers) {
  ShellBrowserContext* shell_browser_context =
      ShellBrowserContextForBrowserContext(content_browser_context);
  return shell_browser_context->CreateRequestContextForStoragePartition(
      partition_path, in_memory, protocol_handlers);
}

void ShellContentBrowserClient::AppendExtraCommandLineSwitches(
    CommandLine* command_line, int child_process_id) {
  // SHEZ: Removed upstream DumpRenderTree code here, used only for testing.
}

void ShellContentBrowserClient::OverrideWebkitPrefs(
    RenderViewHost* render_view_host,
    const GURL& url,
    webkit_glue::WebPreferences* prefs) {
  // SHEZ: Removed upstream DumpRenderTree code here, used only for testing.
}

bool ShellContentBrowserClient::SupportsInProcessRenderer()
{
#if !defined(OS_IOS) && (!defined(GOOGLE_CHROME_BUILD) || defined(OS_ANDROID))
  return CommandLine::ForCurrentProcess()->HasSwitch(switches::kSingleProcess);
#else
  // Single-process is an unsupported and not fully tested mode, so don't
  // enable it for official Chrome builds (except on Android).
  return false;
#endif
}

void ShellContentBrowserClient::StartInProcessRendererThread(
    const std::string& channel_id) {
  DCHECK(!g_in_process_renderer);
  g_in_process_renderer = new RendererMainThread(channel_id);

  base::Thread::Options options;
#if !defined(TOOLKIT_GTK) && !defined(OS_ANDROID)
  // In-process plugins require this to be a UI message loop.
  options.message_loop_type = MessageLoop::TYPE_UI;
#else
  // We can't have multiple UI loops on GTK and Android, so we don't support
  // in-process plugins.
  options.message_loop_type = MessageLoop::TYPE_DEFAULT;
#endif

  g_in_process_renderer->StartWithOptions(options);
}

void ShellContentBrowserClient::StopInProcessRendererThread() {
  DCHECK(g_in_process_renderer);
  delete g_in_process_renderer;
  g_in_process_renderer = 0;
}

void ShellContentBrowserClient::ResourceDispatcherHostCreated() {
  resource_dispatcher_host_delegate_.reset(
      new ShellResourceDispatcherHostDelegate());
  ResourceDispatcherHost::Get()->SetDelegate(
      resource_dispatcher_host_delegate_.get());
}

std::string ShellContentBrowserClient::GetDefaultDownloadName() {
  return "download";
}

bool ShellContentBrowserClient::SupportsBrowserPlugin(
    content::BrowserContext* browser_context, const GURL& url) {
  return CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kEnableBrowserPluginForAllViewTypes);
}

WebContentsViewDelegate* ShellContentBrowserClient::GetWebContentsViewDelegate(
    WebContents* web_contents) {
#if !defined(USE_AURA)
  return CreateShellWebContentsViewDelegate(web_contents);
#else
  return NULL;
#endif
}

QuotaPermissionContext*
ShellContentBrowserClient::CreateQuotaPermissionContext() {
  return new ShellQuotaPermissionContext();
}

#if defined(OS_ANDROID)
void ShellContentBrowserClient::GetAdditionalMappedFilesForChildProcess(
    const CommandLine& command_line,
    int child_process_id,
    std::vector<content::FileDescriptorInfo>* mappings) {
  int flags = base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ;
  base::FilePath pak_file;
  bool r = PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_file);
  CHECK(r);
  pak_file = pak_file.Append(FILE_PATH_LITERAL("paks"));
  pak_file = pak_file.Append(FILE_PATH_LITERAL("content_shell.pak"));

  base::PlatformFile f =
      base::CreatePlatformFile(pak_file, flags, NULL, NULL);
  if (f == base::kInvalidPlatformFileValue) {
    NOTREACHED() << "Failed to open file when creating renderer process: "
                 << "content_shell.pak";
  }
  mappings->push_back(
      content::FileDescriptorInfo(kShellPakDescriptor,
                                  base::FileDescriptor(f, true)));
}
#endif

ShellBrowserContext* ShellContentBrowserClient::browser_context() {
  return shell_browser_main_parts_->browser_context();
}

ShellBrowserContext*
    ShellContentBrowserClient::off_the_record_browser_context() {
  return shell_browser_main_parts_->off_the_record_browser_context();
}

AccessTokenStore* ShellContentBrowserClient::CreateAccessTokenStore() {
  return new ShellAccessTokenStore(browser_context()->GetRequestContext());
}

ShellBrowserContext*
ShellContentBrowserClient::ShellBrowserContextForBrowserContext(
    BrowserContext* content_browser_context) {
  if (content_browser_context == browser_context())
    return browser_context();
  DCHECK_EQ(content_browser_context, off_the_record_browser_context());
  return off_the_record_browser_context();
}

}  // namespace content
