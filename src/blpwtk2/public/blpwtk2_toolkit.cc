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

#include <blpwtk2_toolkit.h>

#include <blpwtk2_products.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>
#include <blpwtk2_toolkitimpl.h>

#include <base/logging.h>  // for DCHECK
#include <net/http/http_network_session.h>
#include <net/socket/client_socket_pool_manager.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebKit.h>
#include <ui/gl/gl_implementation.h>


// dependencies needed for Toolkit::subProcessMain()
#include <blpwtk2_contentmaindelegateimpl.h>
#include <content/public/app/content_main.h>


namespace blpwtk2 {

static bool g_started = false;
static bool g_shutdown = false;

static void startToolkitImpl()
{
    gfx::SetBLPAngleDLLName(BLPANGLE_DLL_NAME);
    g_started = true;
    new ToolkitImpl();
}

void Toolkit::setThreadMode(ThreadMode::Value mode)
{
    Statics::initApplicationMainThread();

    if (g_started) {
        DCHECK(mode == Statics::threadMode);
        return;
    }
    DCHECK(!ToolkitImpl::instance());
    Statics::threadMode = mode;
}

void Toolkit::setPumpMode(PumpMode::Value mode)
{
    Statics::initApplicationMainThread();

    if (g_started) {
        DCHECK(mode == Statics::pumpMode);
        return;
    }
    DCHECK(!ToolkitImpl::instance());
    Statics::pumpMode = mode;
}

void Toolkit::setMaxSocketsPerProxy(int count)
{
    Statics::initApplicationMainThread();

    const net::HttpNetworkSession::SocketPoolType POOL =
        net::HttpNetworkSession::NORMAL_SOCKET_POOL;

    if (g_started) {
        DCHECK(count ==
            net::ClientSocketPoolManager::max_sockets_per_proxy_server(POOL));
        return;
    }

    DCHECK(!ToolkitImpl::instance());
    DCHECK(1 <= count);
    DCHECK(99 >= count);

    // The max per group can never exceed the max per proxy.  Use the default
    // max per group, unless count is less than the default.

    int prevMaxPerProxy =
        net::ClientSocketPoolManager::max_sockets_per_proxy_server(POOL);
    int newMaxPerGroup = std::min(count,
                                  (int)net::kDefaultMaxSocketsPerGroupNormal);

    if (newMaxPerGroup > prevMaxPerProxy) {
        net::ClientSocketPoolManager::set_max_sockets_per_proxy_server(
            POOL,
            count);
        net::ClientSocketPoolManager::set_max_sockets_per_group(
            POOL,
            newMaxPerGroup);
    }
    else {
        net::ClientSocketPoolManager::set_max_sockets_per_group(
            POOL,
            newMaxPerGroup);
        net::ClientSocketPoolManager::set_max_sockets_per_proxy_server(
            POOL,
            count);
    }
}

void Toolkit::registerPlugin(const char* pluginPath)
{
    Statics::initApplicationMainThread();
    DCHECK(!g_started);
    DCHECK(!ToolkitImpl::instance());
    Statics::registerPlugin(pluginPath);
}

void Toolkit::enableDefaultPlugins(bool enabled)
{
    Statics::initApplicationMainThread();
    if (g_started) {
        DCHECK(Statics::enableDefaultPlugins == enabled);
        return;
    }
    DCHECK(!ToolkitImpl::instance());
    Statics::enableDefaultPlugins = enabled;
}

void Toolkit::setRendererUsesInProcessPlugins(int renderer)
{
    Statics::initApplicationMainThread();
    Statics::setRendererUsesInProcessPlugins(renderer);
}

Profile* Toolkit::getProfile(const char* dataDir)
{
    Statics::initApplicationMainThread();
    DCHECK(!g_shutdown);
    DCHECK(dataDir);
    DCHECK(*dataDir);
    return Statics::getOrCreateProfile(dataDir);
}

Profile* Toolkit::createIncognitoProfile()
{
    Statics::initApplicationMainThread();
    DCHECK(!g_shutdown);
    return Statics::createIncognitoProfile();
}

bool Toolkit::hasDevTools()
{
    Statics::isInApplicationMainThread();
    return Statics::hasDevTools;
}

void Toolkit::setHttpTransactionHandler(HttpTransactionHandler* handler)
{
    Statics::initApplicationMainThread();

    if (g_started) {
        DCHECK(Statics::httpTransactionHandler == handler);
        return;
    }
    DCHECK(!ToolkitImpl::instance());
    Statics::httpTransactionHandler = handler;
}

void Toolkit::shutdown()
{
    Statics::initApplicationMainThread();
    DCHECK(0 == Statics::numWebViews) << "Have not destroyed "
                                      << Statics::numWebViews
                                      << " WebViews!";
    if (g_started && !g_shutdown) {
        DCHECK(ToolkitImpl::instance());
        g_shutdown = true;
        delete ToolkitImpl::instance();
    }
    Statics::deleteProfiles();
}

WebView* Toolkit::createWebView(NativeView parent,
                                WebViewDelegate* delegate,
                                const CreateParams& params)
{
    Statics::initApplicationMainThread();

    DCHECK(!g_shutdown);
    if (!g_started) {
        startToolkitImpl();
    }
    DCHECK(ToolkitImpl::instance());
    return ToolkitImpl::instance()->createWebView(parent,
                                                  delegate,
                                                  params);
}

void Toolkit::onRootWindowPositionChanged(gfx::NativeView root)
{
    Statics::initApplicationMainThread();

    if (!g_started || g_shutdown)
        return;
    DCHECK(ToolkitImpl::instance());
    return ToolkitImpl::instance()->onRootWindowPositionChanged(root);
}

void Toolkit::onRootWindowSettingChange(gfx::NativeView root)
{
    Statics::initApplicationMainThread();

    if (!g_started || g_shutdown)
        return;
    DCHECK(ToolkitImpl::instance());
    return ToolkitImpl::instance()->onRootWindowSettingChange(root);
}

bool Toolkit::preHandleMessage(const NativeMsg* msg)
{
    Statics::initApplicationMainThread();

    if (!g_started || g_shutdown)
        return false;
    DCHECK(ToolkitImpl::instance());
    DCHECK(PumpMode::MANUAL == Statics::pumpMode);
    return ToolkitImpl::instance()->preHandleMessage(msg);
}

void Toolkit::postHandleMessage(const NativeMsg* msg)
{
    Statics::initApplicationMainThread();

    if (!g_started || g_shutdown)
        return;
    DCHECK(ToolkitImpl::instance());
    DCHECK(PumpMode::MANUAL == Statics::pumpMode);
    ToolkitImpl::instance()->postHandleMessage(msg);
}

void Toolkit::setDictionaryPath(const StringRef& path)
{
    Statics::initApplicationMainThread();

    DCHECK(!g_started && !g_shutdown);
    Statics::getDictionaryPath().assign(path.data(), path.length());
}

void Toolkit::allowNonWindowContexts_Unsafe()
{
    WebKit::allowNonWindowContexts_Unsafe();
}

}  // close namespace blpwtk2

// This is the entry point for blpwtk2_subprocess.exe
// Do not call this from anywhere else!!
extern "C" __declspec(dllexport)
int SubProcessMain(HINSTANCE hInstance,
                   sandbox::SandboxInterfaceInfo* sandboxInfo)
{
    gfx::SetBLPAngleDLLName(BLPANGLE_DLL_NAME);
    blpwtk2::ContentMainDelegateImpl delegate(true);
    return content::ContentMain(hInstance, sandboxInfo, &delegate);
}
