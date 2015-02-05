// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/browser/shell_browser_context.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/environment.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_switches.h"
#include "content/shell/browser/shell_download_manager_delegate.h"
#include "content/shell/browser/shell_url_request_context_getter.h"
#include "content/shell/common/shell_switches.h"

#include "base/prefs/json_pref_store.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service_factory.h"
#include "base/prefs/pref_service.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/user_prefs/user_prefs.h"

#if defined(OS_WIN)
#include "base/base_paths_win.h"
#elif defined(OS_LINUX)
#include "base/nix/xdg_util.h"
#elif defined(OS_MACOSX)
#include "base/base_paths_mac.h"
#endif

namespace content {

class ShellBrowserContext::ShellResourceContext : public ResourceContext {
 public:
  ShellResourceContext() : getter_(NULL) {}
  virtual ~ShellResourceContext() {}

  // ResourceContext implementation:
  virtual net::HostResolver* GetHostResolver() OVERRIDE {
    CHECK(getter_);
    return getter_->host_resolver();
  }
  virtual net::URLRequestContext* GetRequestContext() OVERRIDE {
    CHECK(getter_);
    return getter_->GetURLRequestContext();
  }

  void set_url_request_context_getter(ShellURLRequestContextGetter* getter) {
    getter_ = getter;
  }

 private:
  ShellURLRequestContextGetter* getter_;

  DISALLOW_COPY_AND_ASSIGN(ShellResourceContext);
};

ShellBrowserContext::ShellBrowserContext(bool off_the_record,
                                         net::NetLog* net_log)
    : off_the_record_(off_the_record),
      net_log_(net_log),
      ignore_certificate_errors_(false),
      guest_manager_(NULL),
      resource_context_(new ShellResourceContext) {
  InitWhileIOAllowed();
}

ShellBrowserContext::~ShellBrowserContext() {
  if (resource_context_) {
    BrowserThread::DeleteSoon(
      BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
  BrowserContextDependencyManager::GetInstance()->DestroyBrowserContextServices(this);
}

void ShellBrowserContext::InitWhileIOAllowed() {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kIgnoreCertificateErrors) ||
      cmd_line->HasSwitch(switches::kDumpRenderTree)) {
    ignore_certificate_errors_ = true;
  }
  if (cmd_line->HasSwitch(switches::kContentShellDataPath)) {
    path_ = cmd_line->GetSwitchValuePath(switches::kContentShellDataPath);
    return;
  }
#if defined(OS_WIN)
  CHECK(PathService::Get(base::DIR_LOCAL_APP_DATA, &path_));
  path_ = path_.Append(std::wstring(L"content_shell"));
#elif defined(OS_LINUX)
  scoped_ptr<base::Environment> env(base::Environment::Create());
  base::FilePath config_dir(
      base::nix::GetXDGDirectory(env.get(),
                                 base::nix::kXdgConfigHomeEnvVar,
                                 base::nix::kDotConfigDir));
  path_ = config_dir.Append("content_shell");
#elif defined(OS_MACOSX)
  CHECK(PathService::Get(base::DIR_APP_DATA, &path_));
  path_ = path_.Append("Chromium Content Shell");
#elif defined(OS_ANDROID)
  CHECK(PathService::Get(base::DIR_ANDROID_APP_DATA, &path_));
  path_ = path_.Append(FILE_PATH_LITERAL("content_shell"));
#else
  NOTIMPLEMENTED();
#endif

  if (!base::PathExists(path_))
    base::CreateDirectory(path_);
  base::FilePath pref_file = path_.AppendASCII("prefs.json");
  pref_registry_ = new PrefRegistrySimple();
  pref_registry_->RegisterStringPref("spellcheck.dictionary", "en-US");
  pref_registry_->RegisterBooleanPref("browser.enable_spellchecking", true);
  pref_registry_->RegisterBooleanPref("spellcheck.use_spelling_service", true);
  pref_registry_->RegisterBooleanPref("browser.enable_autospellcorrect", true);
  pref_registry_->RegisterBooleanPref("printing.enabled", true);
  base::PrefServiceFactory factory;
  factory.SetUserPrefsFile(pref_file, JsonPrefStore::GetTaskRunnerForFile(pref_file, BrowserThread::GetBlockingPool()));
  pref_service_ = factory.Create(pref_registry_.get());
  user_prefs::UserPrefs::Set(this, pref_service_.get());

  BrowserContextDependencyManager::GetInstance()->CreateBrowserContextServices(this);
}

base::FilePath ShellBrowserContext::GetPath() const {
  return path_;
}

bool ShellBrowserContext::IsOffTheRecord() const {
  return off_the_record_;
}

DownloadManagerDelegate* ShellBrowserContext::GetDownloadManagerDelegate()  {
  DownloadManager* manager = BrowserContext::GetDownloadManager(this);

  if (!download_manager_delegate_.get()) {
    download_manager_delegate_.reset(new ShellDownloadManagerDelegate());
    download_manager_delegate_->SetDownloadManager(manager);
    CommandLine* cmd_line = CommandLine::ForCurrentProcess();
    if (cmd_line->HasSwitch(switches::kDumpRenderTree)) {
      download_manager_delegate_->SetDownloadBehaviorForTesting(
          path_.Append(FILE_PATH_LITERAL("downloads")));
    }
  }

  return download_manager_delegate_.get();
}

net::URLRequestContextGetter* ShellBrowserContext::GetRequestContext()  {
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter* ShellBrowserContext::CreateRequestContext(
    ProtocolHandlerMap* protocol_handlers,
    URLRequestInterceptorScopedVector request_interceptors) {
  DCHECK(!url_request_getter_.get());
  url_request_getter_ = new ShellURLRequestContextGetter(
      ignore_certificate_errors_,
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers,
      request_interceptors.Pass(),
      net_log_);
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetRequestContextForRenderProcess(
        int renderer_child_id)  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetMediaRequestContext()  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetMediaRequestContextForRenderProcess(
        int renderer_child_id)  {
  return GetRequestContext();
}

net::URLRequestContextGetter*
    ShellBrowserContext::GetMediaRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory) {
  return GetRequestContext();
}

net::URLRequestContextGetter*
ShellBrowserContext::CreateRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers,
    URLRequestInterceptorScopedVector request_interceptors) {
  return NULL;
}

ResourceContext* ShellBrowserContext::GetResourceContext()  {
  return resource_context_.get();
}

BrowserPluginGuestManager* ShellBrowserContext::GetGuestManager() {
  return guest_manager_;
}

storage::SpecialStoragePolicy* ShellBrowserContext::GetSpecialStoragePolicy() {
  return NULL;
}

PushMessagingService* ShellBrowserContext::GetPushMessagingService() {
  return NULL;
}

SSLHostStateDelegate* ShellBrowserContext::GetSSLHostStateDelegate() {
  return NULL;
}

}  // namespace content
