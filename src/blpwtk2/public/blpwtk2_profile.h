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

#ifndef INCLUDED_BLPWTK2_PROFILE_H
#define INCLUDED_BLPWTK2_PROFILE_H

#include <blpwtk2_config.h>

namespace blpwtk2 {

class ProxyConfig;
class SpellCheckConfig;

// A profile represents a collection of settings that are used to control how
// WebViews operate.  In a browser, multiple profiles may exist simultaneously,
// where each profile has a different data directory for cookies/cache etc.
// A profile may be provided upon creation of a WebView.  A profile may have
// multiple WebViews associated with it, but a WebView can only be associated
// with a single profile.  If a profile is not provided when creating a
// WebView, blpwtk2 will use a default profile, which is incognito and uses the
// system proxy configuration.
// Note: Right now, WebViews that have affinity to a particular renderer
//       process must use the same Profile.
class Profile {
  public:
    // Destroy this profile.  Note that all WebViews created from this profile
    // must be destroyed before the profile is destroyed.  The behavior is
    // undefined if this Profile object is used after 'destroy' has been
    // called.
    virtual void destroy() = 0;

    // Configure the proxy using the specified 'config'.  This method may be
    // called even after WebViews have been created, and the new proxy settings
    // will be used for subsequent network requests.  Note that if the
    // specified 'config' does not contain a pac url or any manual proxy
    // settings, this effectively causes the WebViews to always attempt direct
    // connections.
    virtual void setProxyConfig(const ProxyConfig& config) = 0;

    // Discard any proxy configuration set by previous 'setProxyConfig' calls,
    // and instead use the system proxy configuration.  This method may be
    // called even after WebViews have been created, and the new proxy settings
    // will be used for subsequent network requests.
    virtual void useSystemProxyConfig() = 0;

    // Set the spellcheck configuration for this profile.  This method may be
    // called even after WebViews have been created.
    virtual void setSpellCheckConfig(const SpellCheckConfig& config) = 0;

  protected:
    // Destroy this Profile object.  Note that clients of blpwtk2 should use
    // the 'destroy()' method, instead of deleting the object directly.
    virtual ~Profile();
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROFILE_H

