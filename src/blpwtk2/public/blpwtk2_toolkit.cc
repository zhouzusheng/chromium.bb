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

#include <blpwtk2_statics.h>
#include <blpwtk2_toolkitimpl.h>

#include <base/logging.h>  // for DCHECK


// dependencies needed for Toolkit::subProcessMain()
#include <blpwtk2_contentmaindelegateimpl.h>
#include <content/public/app/content_main.h>


namespace blpwtk2 {

static bool g_started = false;
static bool g_shutdown = false;

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

void Toolkit::registerPlugin(const char* pluginPath)
{
    Statics::initApplicationMainThread();
    DCHECK(!g_started);
    DCHECK(!ToolkitImpl::instance());
    Statics::registerPlugin(pluginPath);
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

    if (!g_started || g_shutdown)
        return;
    DCHECK(ToolkitImpl::instance());
    g_shutdown = true;
    delete ToolkitImpl::instance();
}

WebView* Toolkit::createWebView(NativeView parent,
                                WebViewDelegate* delegate,
                                const CreateParams& params)
{
    Statics::initApplicationMainThread();

    DCHECK(!g_shutdown);
    if (!g_started) {
        g_started = true;
        new ToolkitImpl();
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

}  // close namespace blpwtk2

// This is the entry point for blpwtk2_subprocess.exe
// Do not call this from anywhere else!!
extern "C" __declspec(dllexport)
int SubProcessMain(HINSTANCE hInstance,
                   sandbox::SandboxInterfaceInfo* sandboxInfo)
{
    blpwtk2::ContentMainDelegateImpl delegate;
    return content::ContentMain(hInstance, sandboxInfo, &delegate);
}
