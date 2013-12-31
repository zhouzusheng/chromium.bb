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

#include <blpwtk2_statics.h>

#include <blpwtk2_constants.h>

#include <base/file_util.h>
#include <base/logging.h>  // for DCHECK
#include <base/synchronization/lock.h>

#include <list>
#include <map>

namespace blpwtk2 {

struct RendererInfo {
    int  d_hostId;
    bool d_usesInProcessPlugins;

    Profile* d_profileForDCheck;  // only set when DCHECKs are enabled

    RendererInfo()
    : d_hostId(-1)
    , d_usesInProcessPlugins(false)
    , d_profileForDCheck(0)
    {
    }
};
typedef std::map<int, RendererInfo> RendererInfoMap;

static base::Lock& getLock()
{
    static base::Lock s_lock;
    return s_lock;
}

static RendererInfoMap& rendererInfoMap()
{
    static RendererInfoMap s_map;
    return s_map;
}

ThreadMode::Value Statics::threadMode = ThreadMode::ORIGINAL;
PumpMode::Value Statics::pumpMode = PumpMode::MANUAL;
base::PlatformThreadId Statics::applicationMainThreadId = base::kInvalidThreadId;
base::PlatformThreadId Statics::browserMainThreadId = base::kInvalidThreadId;
content::DevToolsHttpHandler* Statics::devToolsHttpHandler = 0;
HttpTransactionHandler* Statics::httpTransactionHandler = 0;
base::MessageLoop* Statics::rendererMessageLoop = 0;
base::MessageLoop* Statics::browserMainMessageLoop = 0;
MediaObserverImpl* Statics::mediaObserver = 0;
bool Statics::hasDevTools = false;
bool Statics::enableDefaultPlugins = true;
int Statics::numWebViews = 0;

std::string& Statics::getDictionaryPath()
{
    static std::string path;
    return path;
}

std::vector<base::FilePath>& Statics::getPluginPaths()
{
    // The plugins registered via blpwtk2::ToolkitCreateParams::registerPlugin
    // need to be stored here temporarily because we can only pass it onto
    // chromium when ToolkitImpl has been initialized.
    static std::vector<base::FilePath> paths;
    return paths;
}

void Statics::registerPlugin(const char* pluginPath)
{
    base::FilePath path = base::FilePath::FromUTF8Unsafe(pluginPath);
    path = base::MakeAbsoluteFilePath(path);
    getPluginPaths().push_back(path);
}


bool Statics::rendererUsesInProcessPlugins(int renderer)
{
    base::AutoLock guard(getLock());
    RendererInfo& info = rendererInfoMap()[renderer];
    return info.d_usesInProcessPlugins;
}

void Statics::setRendererUsesInProcessPlugins(int renderer)
{
    base::AutoLock guard(getLock());
    RendererInfo& info = rendererInfoMap()[renderer];
    DCHECK(-1 == info.d_hostId);
    info.d_usesInProcessPlugins = true;
}

void Statics::setRendererHostId(int renderer, int hostId)
{
    base::AutoLock guard(getLock());
    RendererInfo& info = rendererInfoMap()[renderer];
    DCHECK(-1 == info.d_hostId);
    info.d_hostId = hostId;
}

int Statics::rendererToHostId(int renderer)
{
    base::AutoLock guard(getLock());
    RendererInfo& info = rendererInfoMap()[renderer];
    return info.d_hostId;
}

int Statics::hostIdToRenderer(int hostId)
{
    base::AutoLock guard(getLock());
    for (RendererInfoMap::const_iterator it = rendererInfoMap().begin();
        it != rendererInfoMap().end(); ++it) {
        if (it->second.d_hostId == hostId) {
            return it->first;
        }
    }
    return Constants::ANY_OUT_OF_PROCESS_RENDERER;
}

bool Statics::dcheckProfileForRenderer(int renderer, Profile* profile)
{
    base::AutoLock guard(getLock());
    RendererInfo& info = rendererInfoMap()[renderer];
    if (!info.d_profileForDCheck) {
        info.d_profileForDCheck = profile;
        return true;
    }
    return info.d_profileForDCheck == profile;
}

void Statics::initApplicationMainThread()
{
    DCHECK(applicationMainThreadId == base::kInvalidThreadId);
    applicationMainThreadId = base::PlatformThread::CurrentId();
}

void Statics::initBrowserMainThread()
{
    DCHECK(browserMainThreadId == base::kInvalidThreadId);
    browserMainThreadId = base::PlatformThread::CurrentId();
}


}  // close namespace blpwtk2

