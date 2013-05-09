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

namespace content {
    class ContentBrowserClient;
}  // close namespace content

namespace blpwtk2 {

// FIXME: move this to a separate file
class ContentClient : public content::ContentClient {
  public:
    ContentClient();
    virtual ~ContentClient();

    // Returns the user agent.
    virtual std::string GetUserAgent() const OVERRIDE;

  private:
    DISALLOW_COPY_AND_ASSIGN(ContentClient);
};

// This is our implementation of the content::ContentMainDelegate interface.
// This allows us to hook into the "main" of the content module (the
// content::ContentMainRunner class).
class ContentMainDelegateImpl : public content::ContentMainDelegate {
  public:
    ContentMainDelegateImpl();
    virtual ~ContentMainDelegateImpl();

    // ContentMainDelegate implementation
    virtual bool BasicStartupComplete(int* exit_code) OVERRIDE;
    virtual content::ContentBrowserClient*
    CreateContentBrowserClient() OVERRIDE;

  private:
    ContentClient d_contentClient;
    scoped_ptr<content::ContentBrowserClient> d_contentBrowserClient;

    DISALLOW_COPY_AND_ASSIGN(ContentMainDelegateImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_CONTENTMAINDELEGATEIMPL_H

