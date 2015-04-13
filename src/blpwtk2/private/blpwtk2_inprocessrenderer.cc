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

#include <blpwtk2_inprocessrenderer.h>

#include <blpwtk2_statics.h>

#include <base/message_loop/message_loop.h>
#include <content/public/renderer/render_thread.h>
#include <content/renderer/render_process_impl.h>
#include <third_party/WebKit/public/web/WebRuntimeFeatures.h>
#include <third_party/WebKit/public/web/win/WebFontRendering.h>
#include <ui/gfx/win/direct_write.h>

namespace blpwtk2 {

static void InitDirectWrite()
{
    // This code is adapted from RendererMainPlatformDelegate::PlatformInitialize,
    // which is used for out-of-process renderers, but is not used for in-process
    // renderers.  So, we need to do it here.

    bool useDirectWrite = gfx::win::ShouldUseDirectWrite();
    blink::WebFontRendering::setUseDirectWrite(useDirectWrite);
}

class InProcessRendererThread : public base::Thread {
public:
    InProcessRendererThread()
    : base::Thread("BlpInProcRenderer")
    {
        base::Thread::Options options;
        options.message_loop_type = base::MessageLoop::TYPE_UI;
        StartWithOptions(options);
    }
    ~InProcessRendererThread()
    {
        Stop();
    }

private:
    // Called just prior to starting the message loop
    void Init() override
    {
        Statics::rendererMessageLoop = message_loop();
        InitDirectWrite();
        content::RenderThread::InitInProcessRenderer("");
    }

    // Called just after the message loop ends
    void CleanUp() override
    {
        content::RenderThread::CleanUpInProcessRenderer();
        Statics::rendererMessageLoop = 0;
    }

    DISALLOW_COPY_AND_ASSIGN(InProcessRendererThread);
};
static InProcessRendererThread* g_inProcessRendererThread = 0;

// static
void InProcessRenderer::init()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!g_inProcessRendererThread);
    DCHECK(!Statics::rendererMessageLoop);

    if (Statics::isRendererMainThreadMode()) {
        Statics::rendererMessageLoop = base::MessageLoop::current();
        InitDirectWrite();
        content::RenderThread::InitInProcessRenderer("");
    }
    else {
        DCHECK(Statics::isOriginalThreadMode());
        g_inProcessRendererThread = new InProcessRendererThread();
    }

    DCHECK(Statics::rendererMessageLoop);
}

// static
void InProcessRenderer::cleanup()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(Statics::rendererMessageLoop);

    if (Statics::isRendererMainThreadMode()) {
        DCHECK(!g_inProcessRendererThread);
        content::RenderThread::CleanUpInProcessRenderer();
        Statics::rendererMessageLoop = 0;
    }
    else {
        DCHECK(g_inProcessRendererThread);
        delete g_inProcessRendererThread;
        g_inProcessRendererThread = 0;
    }

    DCHECK(!Statics::rendererMessageLoop);
}

// static
base::SingleThreadTaskRunner* InProcessRenderer::ipcTaskRunner()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(Statics::isRendererMainThreadMode());
    return content::RenderThread::IPCTaskRunner();
}

// static
void InProcessRenderer::setChannelName(const std::string& channelName)
{
    DCHECK(Statics::isInApplicationMainThread());
    if (Statics::isInBrowserMainThread()) {
        DCHECK(Statics::rendererMessageLoop);
        Statics::rendererMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&content::RenderThread::SetInProcessRendererChannelName,
                       channelName));
    }
    else {
        content::RenderThread::SetInProcessRendererChannelName(channelName);
    }
}

}  // close namespace blpwtk2
