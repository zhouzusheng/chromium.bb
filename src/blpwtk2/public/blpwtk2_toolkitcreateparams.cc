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

#include <blpwtk2_toolkitcreateparams.h>

#include <blpwtk2_constants.h>
#include <blpwtk2_products.h>
#include <blpwtk2_stringref.h>

#include <base/logging.h>  // for DCHECK

#include <string>
#include <vector>

namespace blpwtk2 {

struct ToolkitCreateParamsImpl {
    ThreadMode::Value d_threadMode;
    PumpMode::Value d_pumpMode;
    int d_maxSocketsPerProxy;
    std::vector<std::string> d_plugins;
    bool d_systemPluginsEnabled;
    std::vector<int> d_renderersUsingInProcessPlugins;
    HttpTransactionHandler* d_httpHandler;
    std::string d_dictionaryPath;
    std::string d_hostChannel;

    ToolkitCreateParamsImpl()
    : d_threadMode(ThreadMode::ORIGINAL)
    , d_pumpMode(PumpMode::MANUAL)
    , d_maxSocketsPerProxy(-1)
    , d_systemPluginsEnabled(true)
    , d_httpHandler(0)
    {
    }
};

ToolkitCreateParams::ToolkitCreateParams()
: d_impl(new ToolkitCreateParamsImpl())
{
}

ToolkitCreateParams::ToolkitCreateParams(const ToolkitCreateParams& other)
: d_impl(new ToolkitCreateParamsImpl(*other.d_impl))
{
}

ToolkitCreateParams::~ToolkitCreateParams()
{
    delete d_impl;
}

ToolkitCreateParams& ToolkitCreateParams::operator=(
    const ToolkitCreateParams& rhs)
{
    if (this != &rhs) {
        *d_impl = *rhs.d_impl;
    }
    return *this;
}

void ToolkitCreateParams::setThreadMode(ThreadMode::Value mode)
{
    d_impl->d_threadMode = mode;
}

void ToolkitCreateParams::setPumpMode(PumpMode::Value mode)
{
    d_impl->d_pumpMode = mode;
}

void ToolkitCreateParams::setMaxSocketsPerProxy(int count)
{
    DCHECK(1 <= count);
    DCHECK(99 >= count);
    d_impl->d_maxSocketsPerProxy = count;
}

void ToolkitCreateParams::registerPlugin(const StringRef& pluginPath)
{
    d_impl->d_plugins.push_back(std::string());
    d_impl->d_plugins.back().assign(pluginPath.data(), pluginPath.length());
}

void ToolkitCreateParams::enableSystemPlugins(bool enabled)
{
    d_impl->d_systemPluginsEnabled = enabled;
}

void ToolkitCreateParams::setRendererUsesInProcessPlugins(int renderer)
{
    DCHECK(renderer == Constants::ANY_OUT_OF_PROCESS_RENDERER
        || renderer == Constants::IN_PROCESS_RENDERER
        || renderer >= 0);

    d_impl->d_renderersUsingInProcessPlugins.push_back(renderer);
}

void ToolkitCreateParams::setHttpTransactionHandler(
    HttpTransactionHandler* handler)
{
    DCHECK(handler);
    d_impl->d_httpHandler = handler;
}

void ToolkitCreateParams::setDictionaryPath(const StringRef& path)
{
    d_impl->d_dictionaryPath.assign(path.data(), path.length());
}

void ToolkitCreateParams::setHostChannel(const StringRef& channelId)
{
    DCHECK(isValidHostChannelVersion(channelId));
    d_impl->d_hostChannel.assign(channelId.data(), channelId.length());
}

// static
bool ToolkitCreateParams::isValidHostChannelVersion(const StringRef& channelId)
{
    static StringRef expected(BLPWTK2_VERSION ".");
    return channelId.isEmpty() ||
        (channelId.length() > expected.length() &&
         expected.equals(StringRef(channelId.data(), expected.length())));
}

ThreadMode::Value ToolkitCreateParams::threadMode() const
{
    return d_impl->d_threadMode;
}

PumpMode::Value ToolkitCreateParams::pumpMode() const
{
    return d_impl->d_pumpMode;
}

bool ToolkitCreateParams::isMaxSocketsPerProxySet() const
{
    return -1 != d_impl->d_maxSocketsPerProxy;
}

int ToolkitCreateParams::maxSocketsPerProxy() const
{
    DCHECK(isMaxSocketsPerProxySet());
    return d_impl->d_maxSocketsPerProxy;
}

size_t ToolkitCreateParams::numRegisteredPlugins() const
{
    return d_impl->d_plugins.size();
}

StringRef ToolkitCreateParams::registeredPluginAt(size_t index) const
{
    DCHECK(index < d_impl->d_plugins.size());
    return d_impl->d_plugins[index];
}

bool ToolkitCreateParams::systemPluginsEnabled() const
{
    return d_impl->d_systemPluginsEnabled;
}

size_t ToolkitCreateParams::numRenderersUsingInProcessPlugins() const
{
    return d_impl->d_renderersUsingInProcessPlugins.size();
}

int ToolkitCreateParams::rendererUsingInProcessPluginsAt(size_t index) const
{
    DCHECK(index < d_impl->d_renderersUsingInProcessPlugins.size());
    return d_impl->d_renderersUsingInProcessPlugins[index];
}

HttpTransactionHandler* ToolkitCreateParams::httpTransactionHandler() const
{
    return d_impl->d_httpHandler;
}

StringRef ToolkitCreateParams::dictionaryPath() const
{
    return d_impl->d_dictionaryPath;
}

StringRef ToolkitCreateParams::hostChannel() const
{
    return d_impl->d_hostChannel;
}

}  // close namespace blpwtk2

