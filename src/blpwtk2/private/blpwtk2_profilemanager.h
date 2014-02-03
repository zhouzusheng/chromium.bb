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

#ifndef INCLUDED_BLPWTK2_PROFILEMANAGER_H
#define INCLUDED_BLPWTK2_PROFILEMANAGER_H

#include <blpwtk2_config.h>

#include <base/basictypes.h>

#include <list>
#include <map>
#include <string>

namespace base {
class MessageLoop;
}  // close namespace base

namespace blpwtk2 {

class Profile;
class ProfileImpl;

class ProfileManager {
  public:
    ProfileManager();
    ~ProfileManager();

    Profile* createProfile(const std::string& dataDir,
                           bool diskCacheEnabled,
                           base::MessageLoop* uiLoop);
    Profile* createIncognitoProfile(base::MessageLoop* uiLoop);
    Profile* defaultProfile(base::MessageLoop* uiLoop);

    // Must be called just before returning from the browser main thread.
    void deleteBrowserContexts();

  private:
    typedef std::map<std::string, ProfileImpl*> ProfileMap;
    typedef std::list<ProfileImpl*> ProfileList;

    ProfileMap d_profileMap;    // used for profiles that have dataDir
    ProfileList d_profileList;  // used for profiles without dataDir
    ProfileImpl* d_defaultProfile;

    DISALLOW_COPY_AND_ASSIGN(ProfileManager);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROFILEMANAGER_H

