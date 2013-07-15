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
#include <blpwtk2_statics.h>

#include <base/command_line.h>
#include <base/files/file_path.h>
#include <base/logging.h>
#include <base/path_service.h>
#include <content/public/common/content_switches.h>
#include <webkit/plugins/npapi/plugin_list.h>
#include <webkit/user_agent/user_agent.h>
#include <webkit/user_agent/user_agent_util.h>
#include <ui/base/ui_base_switches.h>

namespace blpwtk2 {

static void InitLogging()
{
    base::FilePath log_filename;
    PathService::Get(base::DIR_EXE, &log_filename);
    log_filename = log_filename.AppendASCII("blpwtk2.log");
    logging::InitLogging(
        log_filename.value().c_str(),
        logging::LOG_TO_BOTH_FILE_AND_SYSTEM_DEBUG_LOG,
        logging::LOCK_LOG_FILE,
        logging::DELETE_OLD_LOG_FILE,
        logging::DISABLE_DCHECK_FOR_NON_OFFICIAL_RELEASE_BUILDS);
    logging::SetLogItems(true, true, true, true);
}

ContentClient::ContentClient()
{
}

ContentClient::~ContentClient()
{
}

// Returns the user agent.
std::string ContentClient::GetUserAgent() const
{
    // include Chrome in our user-agent because some sites actually look for
    // this.  For example, google's "Search as you type" feature.
    return webkit_glue::BuildUserAgentFromProduct("BlpWtk/" BB_PATCH_VERSION " Chrome/" CHROMIUM_VERSION);
}


ContentMainDelegateImpl::ContentMainDelegateImpl()
{
}

ContentMainDelegateImpl::~ContentMainDelegateImpl()
{
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

    InitLogging();
    SetContentClient(&d_contentClient);

    for (size_t i = 0; i < Statics::getPluginPaths().size(); ++i) {
        const base::FilePath& path = Statics::getPluginPaths()[i];
        webkit::npapi::PluginList::Singleton()->AddExtraPluginPath(path);
    }

    return false;
}

content::ContentBrowserClient*
ContentMainDelegateImpl::CreateContentBrowserClient()
{
    if (!d_contentBrowserClient.get()) {
        d_contentBrowserClient.reset(new ContentBrowserClientImpl());

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

