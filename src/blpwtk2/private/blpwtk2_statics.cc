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
#include <blpwtk2_profileimpl.h>

#include <base/file_util.h>
#include <base/logging.h>  // for DCHECK
#include <base/synchronization/lock.h>
#include <content/public/browser/browser_context.h>

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

// used for profiles that have dataDir
typedef std::map<std::string, ProfileImpl*> ProfileMap;

// used for profiles without dataDir
typedef std::list<ProfileImpl*> ProfileList;

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

static ProfileMap& profileMap()
{
    static ProfileMap s_map;
    return s_map;
}

static ProfileList& profileList()
{
    static ProfileList s_list;
    return s_list;
}

ProfileImpl* s_defaultProfile = 0;

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
int Statics::numWebViews = 0;

std::string& Statics::getDictionaryPath()
{
    static std::string path;
    return path;
}

std::vector<base::FilePath>& Statics::getPluginPaths()
{
    // The plugins registered via blpwtk2::Toolkit::registerPlugin need to be
    // stored here temporarily because we can only pass it onto chromium when
    // ToolkitImpl has been initialized.
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

Profile* Statics::getOrCreateProfile(const char* dataDir)
{
    DCHECK(isInApplicationMainThread());
    DCHECK(dataDir);
    DCHECK(*dataDir);

    ProfileMap::const_iterator it = profileMap().find(dataDir);
    if (it != profileMap().end()) {
        return it->second;
    }

    ProfileImpl* profile = new ProfileImpl(dataDir);
    profileMap()[dataDir] = profile;
    return profile;
}

Profile* Statics::createIncognitoProfile()
{
    ProfileImpl* profile = new ProfileImpl(0);
    profileList().push_back(profile);
    return profile;
}

Profile* Statics::defaultProfile()
{
    DCHECK(isInApplicationMainThread());
    if (!s_defaultProfile) {
        s_defaultProfile = new ProfileImpl(0);
    }
    return s_defaultProfile;
}

void Statics::deleteProfiles()
{
    DCHECK(isInApplicationMainThread());

    for (ProfileMap::iterator it = profileMap().begin();
                              it != profileMap().end(); ++it) {
        delete it->second;
    }
    profileMap().clear();

    for (ProfileList::iterator it = profileList().begin();
                               it != profileList().end(); ++it) {
        delete *it;
    }
    profileList().clear();

    if (s_defaultProfile) {
        delete s_defaultProfile;
        s_defaultProfile = 0;
    }
}

void Statics::deleteBrowserContexts()
{
    // This function is called from the browser-main thread, but occurs when
    // it is joining with the application-main thread, so it is thread-safe.

    // Verify that this function is only called when the browser thread is
    // shutting down.
    DCHECK(isInBrowserMainThread());
    DCHECK(!browserMainMessageLoop);

    for (ProfileMap::iterator it = profileMap().begin();
                              it != profileMap().end(); ++it) {
        DCHECK(it->second);
        if (it->second->browserContext()) {
            delete it->second->browserContext();
        }
    }
    for (ProfileList::iterator it = profileList().begin();
                               it != profileList().end(); ++it) {
        DCHECK(*it);
        if ((*it)->browserContext()) {
            delete (*it)->browserContext();
        }
    }

    if (s_defaultProfile && s_defaultProfile->browserContext()) {
        delete s_defaultProfile->browserContext();
    }
}


void Statics::initApplicationMainThread()
{
    if (applicationMainThreadId == base::kInvalidThreadId)
        applicationMainThreadId = base::PlatformThread::CurrentId();
    DCHECK(applicationMainThreadId == base::PlatformThread::CurrentId());
}

void Statics::initBrowserMainThread()
{
    if (browserMainThreadId == base::kInvalidThreadId)
        browserMainThreadId = base::PlatformThread::CurrentId();
    DCHECK(browserMainThreadId == base::PlatformThread::CurrentId());
}


}  // close namespace blpwtk2

