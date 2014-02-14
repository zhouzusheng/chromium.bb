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

#ifndef INCLUDED_BLPWTK2_CONTENTMAINDELEGATEIMPL_H
#define INCLUDED_BLPWTK2_CONTENTMAINDELEGATEIMPL_H

#include <blpwtk2_config.h>

#include <base/memory/scoped_ptr.h>
#include <content/public/app/content_main_delegate.h>
#include <content/public/common/content_client.h>

namespace base {
class FilePath;
}  // close namespace base

namespace blpwtk2 {

class RendererInfoMap;

// FIXME: move this to a separate file
class ContentClient : public content::ContentClient {
  public:
    ContentClient();
    virtual ~ContentClient();

    void registerPlugin(const char* pluginPath);

    // Returns the user agent.
    virtual std::string GetUserAgent() const OVERRIDE;

    // Return the contents of a resource in a StringPiece given the resource id.
    virtual base::StringPiece GetDataResource(
        int resource_id,
        ui::ScaleFactor scale_factor) const OVERRIDE;

    // Gives the embedder a chance to register its own internal NPAPI plugins.
    virtual void AddNPAPIPlugins(
        webkit::npapi::PluginList* plugin_list) OVERRIDE;

  private:
    std::vector<base::FilePath> d_pluginPaths;

    DISALLOW_COPY_AND_ASSIGN(ContentClient);
};

// This is our implementation of the content::ContentMainDelegate interface.
// This allows us to hook into the "main" of the content module (the
// content::ContentMainRunner class).
class ContentMainDelegateImpl : public content::ContentMainDelegate {
  public:
    ContentMainDelegateImpl(bool isSubProcess,
                            bool pluginDiscoveryEnabled,
                            bool sandboxDisabled);
    virtual ~ContentMainDelegateImpl();

    void setRendererInfoMap(RendererInfoMap* rendererInfoMap);
    void registerPlugin(const char* pluginPath);

    // ContentMainDelegate implementation
    virtual bool BasicStartupComplete(int* exit_code) OVERRIDE;
    virtual void PreSandboxStartup() OVERRIDE;
    virtual content::ContentBrowserClient*
        CreateContentBrowserClient() OVERRIDE;
    virtual content::ContentRendererClient*
        CreateContentRendererClient() OVERRIDE;

  private:
    ContentClient d_contentClient;
    scoped_ptr<content::ContentBrowserClient> d_contentBrowserClient;
    scoped_ptr<content::ContentRendererClient> d_contentRendererClient;
    RendererInfoMap* d_rendererInfoMap;
    bool d_pluginDiscoveryEnabled;
    bool d_isSubProcess;
    bool d_sandboxDisabled;

    DISALLOW_COPY_AND_ASSIGN(ContentMainDelegateImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_CONTENTMAINDELEGATEIMPL_H

