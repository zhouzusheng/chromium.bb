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

#include <blpwtk2_browserthread.h>

#include <blpwtk2_browsermainrunner.h>
#include <blpwtk2_inprocessrendererhost.h>

#include <base/at_exit.h>
#include <base/bind.h>
#include <base/logging.h>  // for DCHECK
#include <base/message_loop.h>
#include <base/synchronization/waitable_event.h>

namespace blpwtk2 {

BrowserThread::BrowserThread(base::WaitableEvent* initializeEvent,
                             sandbox::SandboxInterfaceInfo* sandboxInfo)
: d_initializeEvent(initializeEvent)
, d_sandboxInfo(sandboxInfo)
{
    DCHECK(d_initializeEvent);
    base::PlatformThread::Create(0, this, &d_threadHandle);
}

BrowserThread::~BrowserThread()
{
    messageLoop()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
    base::PlatformThread::Join(d_threadHandle);
}

void BrowserThread::sync()
{
    base::WaitableEvent event(true, false);
    messageLoop()->PostTask(
        FROM_HERE,
        base::Bind(&base::WaitableEvent::Signal,
                   base::Unretained(&event)));
    event.Wait();
}

BrowserMainRunner* BrowserThread::mainRunner() const
{
    return d_mainRunner;
}

MessageLoop* BrowserThread::messageLoop() const
{
    DCHECK(d_messageLoop);
    return d_messageLoop;
}

void BrowserThread::ThreadMain()
{
    // Ensure that any singletons created by chromium will be destructed in the
    // browser-main thread, instead of the application's main thread.  This is
    // shadowing the AtExitManager created by ContentMainRunner.  The fact that
    // the main thread waits for the initialize event, and the fact that the
    // main thread joins this thread before shutting down ContentMainRunner,
    // makes this shadowing thread-safe.
    base::ShadowingAtExitManager atExitManager;

    d_mainRunner = new BrowserMainRunner(d_sandboxInfo);
    d_messageLoop = MessageLoop::current();

    // Force the creation of the in-process renderer host.  This has to be done
    // before signaling the initialize event, so that when the application
    // tries to create an in-process WebView, the application's main thread
    // will be able to access the InProcessRendererHost's ID immediately
    // (without having to synchronize with the browser-main thread again).
    d_mainRunner->createInProcessRendererHost();

    d_initializeEvent->Signal();
    d_initializeEvent = 0;

    int rc = d_mainRunner->Run();
    DCHECK(0 == rc);

    delete d_mainRunner;
    d_messageLoop = 0;
}

}  // close namespace blpwtk2

