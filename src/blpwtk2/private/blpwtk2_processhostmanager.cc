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

#include <blpwtk2_processhostmanager.h>

#include <blpwtk2_processhost.h>
#include <blpwtk2_statics.h>

#include <base/message_loop/message_loop.h>

#include <vector>

namespace blpwtk2 {

void deleteProcessHost(ProcessHost* processHost)
{
    DCHECK(Statics::isInBrowserMainThread());
    delete processHost;
}

ProcessHostManager::ProcessHostManager()
: d_timer(false, false)
, d_wakeupEvent(true, false)
, d_isShuttingDown(false)
{
    DCHECK(Statics::isInBrowserMainThread());
    base::PlatformThread::Create(0, this, &d_threadHandle);
}

ProcessHostManager::~ProcessHostManager()
{
    DCHECK(Statics::isInBrowserMainThread());

    {
        base::AutoLock guard(d_lock);
        d_isShuttingDown = true;
    }

    d_timer.Stop();
    d_wakeupEvent.Signal();
    base::PlatformThread::Join(d_threadHandle);

    if (!d_connectedProcessHosts.empty()) {
        LOG(WARNING) << "deleting ProcessHost for "
                     << d_connectedProcessHosts.size()
                     << " connected processes";
        for (size_t i = 0; i < d_connectedProcessHosts.size(); ++i) {
            deleteProcessHost(d_connectedProcessHosts[i]->d_host);
        }
    }

    if (!d_unconnectedProcessHosts.empty()) {
        LOG(WARNING) << "deleting ProcessHost for "
                     << d_unconnectedProcessHosts.size()
                     << " unconnected processes";

        typedef std::list<UnconnectedProcessHostEntry>::iterator Iterator;
        base::AutoLock guard(d_lock);
        for (Iterator it = d_unconnectedProcessHosts.begin();
                      it != d_unconnectedProcessHosts.end(); ++it) {
            deleteProcessHost(it->d_host);
        }
    }
}

void ProcessHostManager::addProcessHost(ProcessHost* processHost,
                                        base::TimeDelta timeout)
{
    DCHECK(Statics::isInBrowserMainThread());

    base::TimeTicks expireTime = base::TimeTicks::Now() + timeout;
    {
        base::AutoLock guard(d_lock);
        UnconnectedProcessHostEntry entry;
        entry.d_host = processHost;
        entry.d_expireTime = expireTime;
        d_unconnectedProcessHosts.push_back(entry);
    }

    if (d_timer.IsRunning()) {
        if (expireTime < d_timer.desired_run_time()) {
            d_timer.Stop();
            d_timer.Start(
                FROM_HERE,
                timeout,
                base::Bind(&ProcessHostManager::timerExpired,
                           base::Unretained(this)));
        }
    }
    else {
        d_timer.Start(
            FROM_HERE,
            timeout,
            base::Bind(&ProcessHostManager::timerExpired,
                       base::Unretained(this)));
    }
}

void ProcessHostManager::timerExpired()
{
    DCHECK(Statics::isInBrowserMainThread());

    base::TimeTicks now = base::TimeTicks::Now();

    typedef std::list<UnconnectedProcessHostEntry>::iterator Iterator;
    base::AutoLock guard(d_lock);
    Iterator it = d_unconnectedProcessHosts.begin();
    while (it != d_unconnectedProcessHosts.end()) {
        base::ProcessHandle handle = it->d_host->processHandle();
        if (handle == base::kNullProcessHandle) {
            if (it->d_expireTime > now) {
                ++it;
                continue;
            }

            LOG(WARNING) << "deleting ProcessHost because nothing connected";
            deleteProcessHost(it->d_host);
        }
        else {
            LOG(INFO) << "moving ProcessHost to connected list";

            d_connectedProcessHosts.push_back(
                new ConnectedProcessHostEntry(it->d_host, handle));

            // Signal the watch thread to repopulate its vector of waitable
            // events.
            d_wakeupEvent.Signal();
        }

        it = d_unconnectedProcessHosts.erase(it);
    }
}

// PlatformThread::Delegate override

void ProcessHostManager::ThreadMain()
{
    base::PlatformThread::SetName("blpwtk2_ProcessHostManager");

    while (true) {
        std::vector<base::WaitableEvent*> events;
        events.push_back(&d_wakeupEvent);
        {
            base::AutoLock guard(d_lock);
            for (size_t i = 0; i < d_connectedProcessHosts.size(); ++i) {
                events.push_back(&d_connectedProcessHosts[i]->d_waitableEvent);
            }
        }
        int eventIndex = base::WaitableEvent::WaitMany(&events[0],
                                                       events.size());
        if (0 == eventIndex) {
            d_wakeupEvent.Reset();
            base::AutoLock guard(d_lock);
            if (d_isShuttingDown) {
                return;
            }
        }
        else {
            int processIndex = eventIndex - 1;
            DCHECK(0 <= processIndex);

            ProcessHost* processHost;
            {
                base::AutoLock guard(d_lock);
                DCHECK(processIndex < (int)d_connectedProcessHosts.size());
                processHost = d_connectedProcessHosts[processIndex]->d_host;
                d_connectedProcessHosts.erase(
                    d_connectedProcessHosts.begin() + processIndex);
            }

            LOG(WARNING) << "deleting ProcessHost because peer has terminated";

            DCHECK(Statics::browserMainMessageLoop);
            Statics::browserMainMessageLoop->PostTask(
                FROM_HERE,
                base::Bind(&deleteProcessHost, base::Unretained(processHost)));
        }
    }
}

}  // close namespace blpwtk2
