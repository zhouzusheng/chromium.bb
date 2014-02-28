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

namespace blpwtk2 {

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
    virtual void Init() OVERRIDE
    {
        Statics::rendererMessageLoop = message_loop();
        content::RenderThread::InitInProcessRenderer("");
    }

    // Called just after the message loop ends
    virtual void CleanUp()
    {
        content::RenderThread::CleanUpInProcessRenderer();
        Statics::rendererMessageLoop = 0;
    }

    DISALLOW_COPY_AND_ASSIGN(InProcessRendererThread);
};
static InProcessRendererThread* g_inProcessRendererThread = 0;

// static
void InProcessRenderer::init(bool usesInProcessPlugins)
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(!g_inProcessRendererThread);
    DCHECK(!Statics::rendererMessageLoop);

    if (usesInProcessPlugins) {
        content::RenderProcessImpl::ForceInProcessPlugins();
    }

    if (Statics::isRendererMainThreadMode()) {
        Statics::rendererMessageLoop = base::MessageLoop::current();
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
