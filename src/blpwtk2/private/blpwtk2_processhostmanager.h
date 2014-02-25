/*
 * Copyright (C) 2014 Bloomberg Finance L.P.
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

#ifndef INCLUDED_BLPWTK2_PROCESSHOSTMANAGER_H
#define INCLUDED_BLPWTK2_PROCESSHOSTMANAGER_H

#include <blpwtk2_config.h>

#include <base/compiler_specific.h>  // for OVERRIDE
#include <base/memory/scoped_vector.h>
#include <base/process.h>
#include <base/threading/platform_thread.h>
#include <base/synchronization/lock.h>
#include <base/synchronization/waitable_event.h>
#include <base/timer.h>

#include <list>

namespace base {
class TimeDelta;
}  // close namespace base

namespace blpwtk2 {

class ProcessHost;

// This class is used from the browser-main thread, but internally starts its
// own thread to watch the processes for which ProcessHost objects are created.
// When one of those processes dies, this object will cleanup the associated
// ProcessHost.  Also, if a ProcessHost is created and hasn't been connected to
// within a specified timeout, this object will destroy that ProcessHost (and
// the listening channel).
class ProcessHostManager : private base::PlatformThread::Delegate {
  public:
    ProcessHostManager();
    ~ProcessHostManager();

    void addProcessHost(ProcessHost* processHost,
                        base::TimeDelta timeout);

  private:
    struct UnconnectedProcessHostEntry {
        ProcessHost* d_host;
        base::TimeTicks d_expireTime;
    };

    struct ConnectedProcessHostEntry {
        ProcessHost* d_host;
        base::WaitableEvent d_waitableEvent;

        ConnectedProcessHostEntry(ProcessHost* processHost,
                                  base::ProcessHandle processHandle)
        : d_host(processHost)
        , d_waitableEvent(processHandle)
        {
        }

        ~ConnectedProcessHostEntry()
        {
            d_waitableEvent.Release();
        }
    };

    void timerExpired();

    // PlatformThread::Delegate override
    virtual void ThreadMain() OVERRIDE;

    base::PlatformThreadHandle d_threadHandle;
    base::Timer d_timer;
    base::WaitableEvent d_wakeupEvent;
    base::Lock d_lock;

    // The list of ProcessHosts that haven't been connected.
    std::list<UnconnectedProcessHostEntry> d_unconnectedProcessHosts;

    // The vector of ProcessHosts that have been connected.
    ScopedVector<ConnectedProcessHostEntry> d_connectedProcessHosts;

    // Flag to notify the watcher thread to shutdown.
    bool d_isShuttingDown;

    DISALLOW_COPY_AND_ASSIGN(ProcessHostManager);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROCESSHOSTMANAGER_H

