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

#ifndef INCLUDED_BLPWTK2_MESSAGEPUMPFORUI_H
#define INCLUDED_BLPWTK2_MESSAGEPUMPFORUI_H

#include <blpwtk2_config.h>

#include <base/message_loop/message_pump_win.h>

namespace base {
    class RunLoop;
}  // close namespace base

namespace blpwtk2 {

// This message pump extends MessagePumpForUI and contains methods to
// facilitate integration with an application's main message loop.
class MainMessagePump : public base::MessagePumpForUI {
  public:
    static MainMessagePump* current();

    MainMessagePump();
    virtual ~MainMessagePump();

    void init();
    void cleanup();
    bool preHandleMessage(const MSG& msg);
    void postHandleMessage(const MSG& msg);

  private:
    void doWork();
    void scheduleMoreWorkIfNecessary();
    void handleWorkMessage();
    void handleTimerMessage();

    static LRESULT CALLBACK wndProcThunk(HWND hwnd,
                                         UINT message,
                                         WPARAM wparam,
                                         LPARAM lparam);

    // MessagePump overrides
    virtual void ScheduleDelayedWork(const base::TimeTicks& delayed_work_time) OVERRIDE;

    scoped_ptr<base::RunLoop> d_runLoop;
    RunState d_runState;
    bool d_hasAutoPumpTimer;
    bool d_didNotifyWillProcessMsg;
    bool d_moreWorkIsPlausible;  // whether or not we need work scheduled

    DISALLOW_COPY_AND_ASSIGN(MainMessagePump);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_MESSAGEPUMPFORUI_H

