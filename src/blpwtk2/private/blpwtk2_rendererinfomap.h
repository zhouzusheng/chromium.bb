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

#ifndef INCLUDED_BLPWTK2_RENDERERINFOMAP_H
#define INCLUDED_BLPWTK2_RENDERERINFOMAP_H

#include <blpwtk2_config.h>

#include <base/basictypes.h>
#include <base/synchronization/lock.h>

#include <map>

namespace blpwtk2 {

class Profile;

// A renderer is the process in which Blink lives.  This class maintains
// information about each renderer that the browser process manages.  In
// blpwtk2, a WebView can be created with affinity to a particular renderer.
// This renderer is identified by a unique positive number, or by the constant
// 'IN_PROCESS_RENDERER' (see 'WebViewCreateParams::setRendererAffinity' for
// more details about this).  In chromium, each renderer is managed by a
// RenderProcessHost, which is identified by a hostId.
//
// This class contains the mapping from blpwtk2's renderer number to the
// hostId.  It also stores renderer-specific configuration, such as whether the
// renderer uses in-process plugins or not.
class RendererInfoMap {
  public:
    RendererInfoMap();
    ~RendererInfoMap();

    // Sets up the mapping between the specified 'renderer' and 'hostId'.
    void setRendererHostId(int renderer, int hostId);

    // Marks the specified 'renderer' as using in-process plugins.
    void setRendererUsesInProcessPlugins(int renderer);

    // Return true if the specified 'renderer' should use in-process plugins.
    bool rendererUsesInProcessPlugins(int renderer);

    // Return true if the specified 'hostId' should use in-process plugins.
    bool hostIdUsesInProcessPlugins(int hostId);

    // Return the hostId for the specified 'renderer', or -1 if the hostId has
    // not been set for this renderer.
    int rendererToHostId(int renderer);

    // This is only used if DCHECK is enabled.  Since a renderer can only be
    // associated with a single Profile, we want to check that WebViews for
    // different Profiles will not be created with affinity to the same
    // renderer.  This method will return false if the specified 'renderer' had
    // already been used with a different 'profile'.
    bool dcheckProfileForRenderer(int renderer, Profile* profile);

  private:
    struct RendererInfo {
        int d_hostId;
        bool d_usesInProcessPlugins;

        Profile* d_profileForDCheck;  // only set when DCHECKs are enabled

        RendererInfo()
        : d_hostId(-1)
        , d_usesInProcessPlugins(false)
        , d_profileForDCheck(0)
        {
        }
    };
    typedef std::map<int, RendererInfo> InfoMap;

    base::Lock d_lock;
    InfoMap d_map;
    bool d_anyOutOfProcessRenderersUseInProcessPlugins;

    DISALLOW_COPY_AND_ASSIGN(RendererInfoMap);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_RENDERERINFOMAP_H

