/*
 * Copyright (C) 2013 Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <blpwtk2_contentmaindelegateimpl.h>

#include <blpwtk2_contentbrowserclientimpl.h>
#include <blpwtk2_contentrendererclientimpl.h>
#include <blpwtk2_products.h>
#include <blpwtk2_statics.h>

#include <base/command_line.h>
#include <base/files/file_path.h>
#include <base/file_util.h>
#include <base/logging.h>
#include <base/path_service.h>
#include <content/public/browser/plugin_service.h>
#include <content/public/common/content_switches.h>
#include <webkit/common/user_agent/user_agent.h>
#include <webkit/common/user_agent/user_agent_util.h>
#include <ui/base/resource/resource_bundle.h>
#include <ui/base/resource/resource_bundle_win.h>
#include <ui/base/ui_base_switches.h>

extern HANDLE g_instDLL;

namespace blpwtk2 {

static void InitLogging()
{
    base::FilePath log_filename;
    PathService::Get(base::DIR_EXE, &log_filename);
    log_filename = log_filename.AppendASCII("blpwtk2.log");
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_ALL;
    settings.log_file = log_filename.value().c_str();
    settings.delete_old = logging::DELETE_OLD_LOG_FILE;
    logging::InitLogging(settings);
    logging::SetLogItems(true, true, true, true);
}

ContentClient::ContentClient()
{
}

ContentClient::~ContentClient()
{
}

void ContentClient::registerPlugin(const char* pluginPath)
{
    base::FilePath path = base::FilePath::FromUTF8Unsafe(pluginPath);
    path = base::MakeAbsoluteFilePath(path);
    d_pluginPaths.push_back(path);
}

// Returns the user agent.
std::string ContentClient::GetUserAgent() const
{
    // include Chrome in our user-agent because some sites actually look for
    // this.  For example, google's "Search as you type" feature.
    return webkit_glue::BuildUserAgentFromProduct("BlpWtk/" BB_PATCH_VERSION " Chrome/" CHROMIUM_VERSION);
}

base::StringPiece ContentClient::GetDataResource(
    int resource_id,
    ui::ScaleFactor scale_factor) const
{
    return ui::ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
        resource_id, scale_factor);
}

void ContentClient::AddNPAPIPlugins(
    webkit::npapi::PluginList* plugin_list)
{
    for (size_t i = 0; i < d_pluginPaths.size(); ++i) {
        // Using the PluginService instead of PluginList because we don't want
        // to link directly to PluginList.  PluginList is linked statically
        // in plugin_common, which is linked into the content module.  If we
        // link plugin_common as well, then this also exposes
        // PluginList::GetInstance(), which (in component builds) would give us
        // a different instance than the one used in the content module.
        content::PluginService::GetInstance()->AddExtraPluginPath(d_pluginPaths[i]);
    }
}

ContentMainDelegateImpl::ContentMainDelegateImpl(bool isSubProcess,
                                                 bool pluginDiscoveryDisabled,
                                                 bool sandboxDisabled)
: d_rendererInfoMap(0)
, d_pluginDiscoveryDisabled(pluginDiscoveryDisabled)
, d_isSubProcess(isSubProcess)
, d_sandboxDisabled(sandboxDisabled)
{
}

ContentMainDelegateImpl::~ContentMainDelegateImpl()
{
}

void ContentMainDelegateImpl::setRendererInfoMap(
    RendererInfoMap* rendererInfoMap)
{
    DCHECK(!d_isSubProcess);
    DCHECK(rendererInfoMap);
    d_rendererInfoMap = rendererInfoMap;
}

void ContentMainDelegateImpl::registerPlugin(const char* pluginPath)
{
    d_contentClient.registerPlugin(pluginPath);
}

// ContentMainDelegate implementation
bool ContentMainDelegateImpl::BasicStartupComplete(int* exit_code)
{
    CommandLine* commandLine = CommandLine::ForCurrentProcess();

    // point to our renderer
    if (!commandLine->HasSwitch(switches::kBrowserSubprocessPath)) {
        base::FilePath subprocess;
        bool success = PathService::Get(base::DIR_EXE, &subprocess);
        DCHECK(success);
        subprocess = subprocess.AppendASCII(BLPWTK2_SUBPROCESS_EXE_NAME);
        commandLine->AppendSwitchNative(switches::kBrowserSubprocessPath,
                                        subprocess.value().c_str());
    }

    if (d_pluginDiscoveryDisabled &&
        !commandLine->HasSwitch(switches::kDisablePluginsDiscovery)) {
        commandLine->AppendSwitch(switches::kDisablePluginsDiscovery);
    }

    if (d_sandboxDisabled &&
        !commandLine->HasSwitch(switches::kNoSandbox)) {
        commandLine->AppendSwitch(switches::kNoSandbox);
    }

    InitLogging();
    SetContentClient(&d_contentClient);

    return false;
}

void ContentMainDelegateImpl::PreSandboxStartup()
{
    ui::SetResourcesDataDLL((HINSTANCE)g_instDLL);
    ui::ResourceBundle::InitSharedInstance();
    ui::ResourceBundle::GetSharedInstance().AddDLLResources();
    if (!d_isSubProcess) {
        base::FilePath pak_file;
        base::FilePath pak_dir;
        PathService::Get(base::DIR_MODULE, &pak_dir);
        pak_file = pak_dir.AppendASCII(BLPWTK2_DEVTOOLS_PAK_NAME);
        if (file_util::PathExists(pak_file)) {
            Statics::hasDevTools = true;
            ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
                pak_file,
                ui::SCALE_FACTOR_NONE);
        }
    }
}

content::ContentBrowserClient*
ContentMainDelegateImpl::CreateContentBrowserClient()
{
    if (!d_contentBrowserClient.get()) {
        d_contentBrowserClient.reset(
            new ContentBrowserClientImpl(d_rendererInfoMap));

        // We always want to be able to have an in-process renderer.  Modify
        // the browser process' command line to include the browser locale, as
        // the renderer expects this flag to be set.  This is to replicate the
        // logic in RenderProcessHost::SetRunRendererInProcess, but we do it
        // regardless of whether --single-process is specified or not.
        CommandLine& commandLine = *CommandLine::ForCurrentProcess();
        if (!commandLine.HasSwitch(switches::kLang)) {
            commandLine.AppendSwitchASCII(
                switches::kLang,
                d_contentBrowserClient->GetApplicationLocale());
        }
    }
    return d_contentBrowserClient.get();
}

content::ContentRendererClient*
ContentMainDelegateImpl::CreateContentRendererClient()
{
    if (!d_contentRendererClient.get())
        d_contentRendererClient.reset(new ContentRendererClientImpl());
    return d_contentRendererClient.get();
}

}  // close namespace blpwtk2

