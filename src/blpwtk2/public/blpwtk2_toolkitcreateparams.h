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

#ifndef INCLUDED_BLPWTK2_TOOLKITCREATEPARAMS_H
#define INCLUDED_BLPWTK2_TOOLKITCREATEPARAMS_H

#include <blpwtk2_config.h>

#include <blpwtk2_pumpmode.h>
#include <blpwtk2_threadmode.h>

namespace blpwtk2 {

class ResourceLoader;
class StringRef;
struct ToolkitCreateParamsImpl;

// This class contains parameters that are passed to blpwtk2 when initializing
// the toolkit.
class ToolkitCreateParams {
  public:
    BLPWTK2_EXPORT ToolkitCreateParams();
    BLPWTK2_EXPORT ToolkitCreateParams(const ToolkitCreateParams&);
    BLPWTK2_EXPORT ~ToolkitCreateParams();
    BLPWTK2_EXPORT ToolkitCreateParams& operator=(const ToolkitCreateParams&);

    // By default, blpwtk2 uses 'ThreadMode::ORIGINAL'.  Use this method to
    // change the thread mode.
    BLPWTK2_EXPORT void setThreadMode(ThreadMode::Value mode);

    // By default, blpwtk2 uses 'PumpMode::MANUAL'.  Use this method to change
    // the pump mode.
    BLPWTK2_EXPORT void setPumpMode(PumpMode::Value mode);

    // Set the maximum number of sockets per proxy, up to a maximum of 99.
    // Note that each Profile maintains its own pool of connections, so this is
    // actually the maximum number of sockets per proxy *per profile*.  The
    // behavior is undefined if 'count' is less than 1, or more than 99.
    BLPWTK2_EXPORT void setMaxSocketsPerProxy(int count);

    // Add the specified 'switchString' to the list of command-line switches.
    // A list of switches can be found at:
    // http://peter.sh/experiments/chromium-command-line-switches/
    // Note, however, that blpwtk2 is based on a different version of chromium,
    // so it may not support *all* the switches mentioned on that page.
    BLPWTK2_EXPORT void appendCommandLineSwitch(const StringRef& switchString);

    // Register a plugin at the specified 'pluginPath'.  The 'pluginPath'
    // should point to a DLL that exports the standard NPAPI entry points.
    BLPWTK2_EXPORT void registerPlugin(const StringRef& pluginPath);

    // By default, blpwtk2 will automatically load plugins it finds on the
    // system (e.g. from paths in the Windows registry).  Use this method to
    // disable this behavior.  If it is disabled, then only plugins registered
    // via 'registerPlugin' will be loaded.
    BLPWTK2_EXPORT void disablePluginDiscovery();

    // By default, renderers will run plugins in a separate process.  Use this
    // method to make any WebView created with affinity to the specified
    // 'renderer' use in-process plugins.  Note that using in-process plugins
    // will disable the sandbox for that renderer.
    BLPWTK2_EXPORT void setRendererUsesInProcessPlugins(int renderer);

    // Install a custom ResourceLoader.  Note that this is only valid when
    // using the 'RENDERER_MAIN' thread-mode, and will only be used for
    // in-process renderers.
    BLPWTK2_EXPORT void setInProcessResourceLoader(ResourceLoader*);

    // By default, blpwtk2 will look for .bdic files in the application's
    // working directory.  Use this method to change the path where blpwtk2
    // would look for the .bdic files.  Note that this is only used if
    // spellchecking is enabled in one of the Profile objects.
    BLPWTK2_EXPORT void setDictionaryPath(const StringRef& path);

    // By default, blpwtk2 will allocate new browser process resources for each
    // blpwtk2 process.  However, the 'Toolkit::createHostChannel' method can
    // be used to setup an IPC channel that this process can use to share the
    // same browser process resources.  Use this method to set the channel-info
    // that will be used to connect this process to the browser process.  This
    // channel-info must have been obtained from 'Toolkit::createHostChannel'
    // in another process using the same version of blpwtk2 (i.e.
    // 'isValidHostChannelVersion' must return true).
    BLPWTK2_EXPORT void setHostChannel(const StringRef& channelInfoString);

    // Return true if the specified 'channelInfoString' was obtained from a
    // process using the same version of blpwtk2, and false otherwise.  It is
    // undefined behavior to use a channel-info obtained from a different
    // version of blpwtk2.
    BLPWTK2_EXPORT static bool isValidHostChannelVersion(
        const StringRef& channelInfoString);

    // ACCESSORS
    ThreadMode::Value threadMode() const;
    PumpMode::Value pumpMode() const;
    bool isMaxSocketsPerProxySet() const;
    int maxSocketsPerProxy() const;
    size_t numCommandLineSwitches() const;
    StringRef commandLineSwitchAt(size_t index) const;
    size_t numRegisteredPlugins() const;
    StringRef registeredPluginAt(size_t index) const;
    size_t numRenderersUsingInProcessPlugins() const;
    int rendererUsingInProcessPluginsAt(size_t index) const;
    ResourceLoader* inProcessResourceLoader() const;
    StringRef dictionaryPath() const;
    StringRef hostChannel() const;

  private:
    ToolkitCreateParamsImpl* d_impl;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_TOOLKITCREATEPARAMS_H

