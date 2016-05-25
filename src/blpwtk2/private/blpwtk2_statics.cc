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

#include <base/logging.h>  // for DCHECK

namespace blpwtk2 {

ThreadMode::Value Statics::threadMode = ThreadMode::ORIGINAL;
PumpMode::Value Statics::pumpMode = PumpMode::MANUAL;
bool Statics::workMessageWhileDoingWorkDisabled = false;
base::PlatformThreadId Statics::applicationMainThreadId = base::kInvalidThreadId;
base::PlatformThreadId Statics::browserMainThreadId = base::kInvalidThreadId;
devtools_http_handler::DevToolsHttpHandler* Statics::devToolsHttpHandler = 0;
ResourceLoader* Statics::inProcessResourceLoader = 0;
base::MessageLoop* Statics::rendererMessageLoop = 0;
base::MessageLoop* Statics::browserMainMessageLoop = 0;
BrowserContextImplManager* Statics::browserContextImplManager = 0;
ProcessHostManager* Statics::processHostManager = 0;
ToolkitCreateParams::ChannelErrorHandler Statics::channelErrorHandler = 0;
bool Statics::hasDevTools = false;
bool Statics::isInProcessRendererDisabled = false;
int Statics::numProfiles = 0;
bool Statics::inProcessResizeOptimizationDisabled = false;

static int lastRoutingId = 0;

void Statics::initApplicationMainThread()
{
    DCHECK(applicationMainThreadId == base::kInvalidThreadId);
    applicationMainThreadId = base::PlatformThread::CurrentId();
}

void Statics::initBrowserMainThread()
{
    DCHECK(browserMainThreadId == base::kInvalidThreadId);
    browserMainThreadId = base::PlatformThread::CurrentId();
}

int Statics::getUniqueRoutingId()
{
    DCHECK(isInApplicationMainThread());
    return ++lastRoutingId;
}

std::string& Statics::userAgentFromEmbedder()
{
    static std::string str;
    return str;
}

}  // close namespace blpwtk2

