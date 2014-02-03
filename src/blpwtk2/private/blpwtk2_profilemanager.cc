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

#include <blpwtk2_profilemanager.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_profileimpl.h>
#include <blpwtk2_statics.h>

#include <base/logging.h> // for DCHECK
#include <content/public/browser/browser_context.h>

namespace blpwtk2 {

ProfileManager::ProfileManager()
: d_defaultProfile(0)
{
}

ProfileManager::~ProfileManager()
{
    DCHECK(Statics::isInApplicationMainThread());

    for (ProfileMap::iterator it = d_profileMap.begin();
                              it != d_profileMap.end(); ++it) {
        delete it->second;
    }

    for (ProfileList::iterator it = d_profileList.begin();
                               it != d_profileList.end(); ++it) {
        delete *it;
    }

    if (d_defaultProfile) {
        d_defaultProfile->destroy();
        delete d_defaultProfile;
    }
}

Profile* ProfileManager::createProfile(const std::string& dataDir,
                                       bool diskCacheEnabled,
                                       base::MessageLoop* uiLoop)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!dataDir.empty());
    DCHECK(d_profileMap.find(dataDir) == d_profileMap.end());

    ProfileImpl* profile = new ProfileImpl(dataDir, diskCacheEnabled, uiLoop);
    d_profileMap[dataDir] = profile;
    return profile;
}

Profile* ProfileManager::createIncognitoProfile(base::MessageLoop* uiLoop)
{
    DCHECK(Statics::isInApplicationMainThread());
    ProfileImpl* profile = new ProfileImpl("", false, uiLoop);
    d_profileList.push_back(profile);
    return profile;
}

Profile* ProfileManager::defaultProfile(base::MessageLoop* uiLoop)
{
    DCHECK(Statics::isInApplicationMainThread());
    if (!d_defaultProfile) {
        d_defaultProfile = new ProfileImpl("", false, uiLoop);
    }
    return d_defaultProfile;
}

void ProfileManager::deleteBrowserContexts()
{
    // This function is called from the browser-main thread, but occurs when
    // it is joining with the application-main thread, so it is thread-safe.

    // Verify that this function is only called when the browser thread is
    // shutting down.
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!Statics::browserMainMessageLoop);

    for (ProfileMap::iterator it = d_profileMap.begin();
                              it != d_profileMap.end(); ++it) {
        DCHECK(it->second);
        if (it->second->browserContext()) {
            delete it->second->browserContext();
        }
    }
    for (ProfileList::iterator it = d_profileList.begin();
                               it != d_profileList.end(); ++it) {
        DCHECK(*it);
        if ((*it)->browserContext()) {
            delete (*it)->browserContext();
        }
    }

    if (d_defaultProfile && d_defaultProfile->browserContext()) {
        delete d_defaultProfile->browserContext();
    }
}

}  // close namespace blpwtk2

