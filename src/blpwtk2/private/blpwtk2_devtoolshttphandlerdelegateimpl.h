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

#ifndef INCLUDED_BLPWTK2_DEVTOOLSHTTPHANDLERDELEGATEIMPL_H
#define INCLUDED_BLPWTK2_DEVTOOLSHTTPHANDLERDELEGATEIMPL_H

#include <blpwtk2_config.h>

#include <content/public/browser/devtools_http_handler_delegate.h>

namespace blpwtk2 {

// TODO: document this
class DevToolsHttpHandlerDelegateImpl
    : public content::DevToolsHttpHandlerDelegate {
  public:
    DevToolsHttpHandlerDelegateImpl();
    virtual ~DevToolsHttpHandlerDelegateImpl();

    // ====== DevToolsHttpHandlerDelegate overrides =======
    virtual std::string GetDiscoveryPageHTML() OVERRIDE { return ""; }
    virtual bool BundlesFrontendResources() OVERRIDE { return false; }
    virtual base::FilePath GetDebugFrontendDir() OVERRIDE;
    virtual std::string GetPageThumbnailData(const GURL& url) OVERRIDE { return ""; }
    virtual content::RenderViewHost* CreateNewTarget() OVERRIDE { return 0; }
    virtual TargetType GetTargetType(content::RenderViewHost*) OVERRIDE { return kTargetTypeTab; }
    virtual std::string GetViewDescription(content::RenderViewHost*) OVERRIDE { return ""; }
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_DEVTOOLSHTTPHANDLERDELEGATEIMPL_H

