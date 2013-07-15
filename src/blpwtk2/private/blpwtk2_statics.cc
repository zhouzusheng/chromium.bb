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

#include <base/file_util.h>
#include <base/logging.h>  // for DCHECK

namespace blpwtk2 {

ThreadMode::Value Statics::threadMode = ThreadMode::ORIGINAL;
PumpMode::Value Statics::pumpMode = PumpMode::MANUAL;
base::PlatformThreadId Statics::applicationMainThreadId = base::kInvalidThreadId;
base::PlatformThreadId Statics::browserMainThreadId = base::kInvalidThreadId;
content::DevToolsHttpHandler* Statics::devToolsHttpHandler = 0;
HttpTransactionHandler* Statics::httpTransactionHandler = 0;
MessageLoop* Statics::rendererMessageLoop = 0;

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
    file_util::AbsolutePath(&path);
    getPluginPaths().push_back(path);
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

