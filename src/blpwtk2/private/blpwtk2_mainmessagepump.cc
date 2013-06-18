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

#include <blpwtk2_mainmessagepump.h>

#include <blpwtk2_statics.h>

#include <base/run_loop.h>
#include <base/message_loop.h>
#include <base/win/wrapped_window_proc.h>
#include <base/time.h>

namespace blpwtk2 {

static HHOOK s_msgHook;
static HHOOK s_callWndHook;
static bool s_isInModal = false;

static const int kMsgHaveWork = WM_USER + 1;

static void DebugWithTime(const char *format, ...)
{
    va_list arglist;
    va_start(arglist, format);

    static base::TimeTicks START_TIME = base::TimeTicks::Now();
    int milliseconds = (base::TimeTicks::Now() - START_TIME).InMilliseconds();

    char buf[1024];
    int timeLen = sprintf_s(buf, sizeof(buf), "%d: ", milliseconds);
    _vsprintf_s_l(buf+timeLen, sizeof(buf)-timeLen, format, NULL, arglist);
    OutputDebugStringA(buf);
}

static bool isModalCode(int code)
{
    return MSGF_DIALOGBOX == code
        || MSGF_MENU == code
        || MSGF_SCROLLBAR == code;
}

static LRESULT CALLBACK MsgFilterHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (!s_isInModal && isModalCode(code)) {
        s_isInModal = true;
        DebugWithTime("ENTERING MODAL LOOP\n");
        MessageLoop::current()->SetNestableTasksAllowed(true);
        MessageLoop::current()->set_os_modal_loop(true);
        MessageLoop::current()->pump_win()->ScheduleWork();
    }
    return CallNextHookEx(s_msgHook, code, wParam, lParam);
}

static LRESULT CALLBACK CallWndHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    CWPSTRUCT* cwp = (CWPSTRUCT*)lParam;
    switch (cwp->message) {
    case WM_ENTERSIZEMOVE:
        DebugWithTime("HOOK ENTER SIZEMOVE\n");
        MessageLoop::current()->SetNestableTasksAllowed(true);
        MessageLoop::current()->set_os_modal_loop(true);
        MessageLoop::current()->pump_win()->ScheduleWork();
        break;
    case WM_EXITSIZEMOVE:
        DebugWithTime("HOOK EXIT SIZEMOVE\n");
        MessageLoop::current()->set_os_modal_loop(false);
        break;
    case WM_ENTERMENULOOP:
        DebugWithTime("HOOK ENTER MENU\n");
        MessageLoop::current()->SetNestableTasksAllowed(true);
        MessageLoop::current()->set_os_modal_loop(true);
        MessageLoop::current()->pump_win()->ScheduleWork();
        break;
    case WM_EXITMENULOOP:
        DebugWithTime("HOOK EXIT MENU\n");
        MessageLoop::current()->set_os_modal_loop(false);
        break;
    }
    return CallNextHookEx(s_callWndHook, code, wParam, lParam);
}

// Return true if the next PeekMessage will return TRUE, or if the next
// GetMessage will return immediately.  This function is used to determine
// when Windows has work to do, in which case, we will break out of Chromium's
// doWork.
static bool isWindowsMessagePending()
{
    const int MASK =
        QS_POSTMESSAGE | QS_ALLPOSTMESSAGE | QS_HOTKEY | QS_PAINT | QS_TIMER;

    DWORD queueStatus = HIWORD(GetQueueStatus(MASK));
    MSG msg;

    while (queueStatus & QS_SENDMESSAGE) {
        // flush sent messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
            return true;
        queueStatus = HIWORD(GetQueueStatus(MASK));
    }

#ifndef NDEBUG
    if (queueStatus & MASK) {
        BOOL hasMessage = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
        DCHECK(hasMessage);
    }
#endif

    return queueStatus & MASK;
}

// static
MainMessagePump* MainMessagePump::current()
{
    MessageLoop* loop = MessageLoop::current();
    DCHECK_EQ(MessageLoop::TYPE_UI, loop->type());
    return static_cast<MainMessagePump*>(loop->pump_win());
}

MainMessagePump::MainMessagePump()
: base::MessagePumpForUI(base::win::WrappedWindowProc<wndProcThunk>,
                         L"blpwtk2_MainMessagePump")
, d_didNotifyWillProcessMsg(false)
, d_immediateWorkIsPlausible(false)
{
}

MainMessagePump::~MainMessagePump()
{
}

void MainMessagePump::init()
{
    // Setup some Windows hooks.  These hooks are used to detect when we enter
    // a modal loop, in which case we put Chromium's work in a lower priority
    // by using WM_TIMER (instead of kMsgHaveWork) to schedule work.
    s_msgHook = SetWindowsHookEx(WH_MSGFILTER, MsgFilterHookProc, NULL,
                                 GetCurrentThreadId());
    s_callWndHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndHookProc, NULL,
                                     GetCurrentThreadId());

    d_runLoop.reset(new base::RunLoop());
    d_runLoop->BeforeRun();
    MessageLoop::current()->PrepareRunInternal();
    PushRunState(&d_runState, MessageLoop::current(), 0);
}

void MainMessagePump::cleanup()
{
    // Before exiting the app, make sure any pending work is done.  This is
    // necessary because there are ref counted objects (e.g.
    // content::RenderViewImpl) whose destructors assert that queued work (in
    // this case, RenderWidget::Close) has been completed.
    bool moreWorkIsPlausible = true;
    while (moreWorkIsPlausible) {
        moreWorkIsPlausible = state_->delegate->DoWork();
        moreWorkIsPlausible |=
            state_->delegate->DoDelayedWork(&delayed_work_time_);
    }

    PopRunState();
    d_runLoop->AfterRun();
    d_runLoop.reset();

    UnhookWindowsHookEx(s_callWndHook);
    UnhookWindowsHookEx(s_msgHook);
}

bool MainMessagePump::preHandleMessage(const MSG& msg)
{
    bool didHandleMessage;
    if (CallMsgFilter(const_cast<MSG*>(&msg), kMessageFilterCode)) {
        d_didNotifyWillProcessMsg = false;
        didHandleMessage = true;
    }
    else {
        WillProcessMessage(msg);
        d_didNotifyWillProcessMsg = true;
        didHandleMessage = message_filter_->ProcessMessage(msg);
        if (!didHandleMessage && state_->dispatcher) {
            if (!state_->dispatcher->Dispatch(msg))
                state_->should_quit = true;
            didHandleMessage = true;
        }
    }
    return didHandleMessage;
}

void MainMessagePump::postHandleMessage(const MSG& msg)
{
    if (d_didNotifyWillProcessMsg) {
        d_didNotifyWillProcessMsg = false;
        DidProcessMessage(msg);
    }

    // There is no Windows hook that notifies us when exiting a modal dialog
    // loop.  However, when postHandleMessage is called, we can assume that we
    // are back in the application's main loop, so turn off the modal loop flag
    // if it was set.
    if (s_isInModal) {
        DebugWithTime("EXITING MODAL LOOP\n");
        MessageLoop::current()->set_os_modal_loop(false);
        s_isInModal = false;
    }

    // If we have immediate work waiting, the kMsgHaveWork handler doesn't
    // schedule work if there is a Windows message pending.  This is to prevent
    // the Windows queue from starving.  So we wait for the Windows message to
    // finish, then ScheduleWork.  This is similar to the
    // ProcessPumpReplacementMessage logic in the upstream message pump.
    if (d_immediateWorkIsPlausible &&
        (msg.hwnd != message_hwnd_ || msg.message != kMsgHaveWork)) {
        ScheduleWork();
    }
}

void MainMessagePump::doWorkUntilNextMessage()
{
    if (PumpMode::AUTOMATIC != Statics::pumpMode && isWindowsMessagePending())
        return;

    while (true) {
        bool moreWorkIsPlausible = state_->delegate->DoWork();
        if (state_->should_quit) {
            // Reset these flags so that scheduleMoreWorkIfNecessary will not
            // schedule any more work.
            d_immediateWorkIsPlausible = false;
            delayed_work_time_ = base::TimeTicks();
            break;
        }

        d_immediateWorkIsPlausible = moreWorkIsPlausible;
        moreWorkIsPlausible |=
            state_->delegate->DoDelayedWork(&delayed_work_time_);

        if (state_->should_quit) {
            // Reset these flags so that scheduleMoreWorkIfNecessary will not
            // schedule any more work.
            d_immediateWorkIsPlausible = false;
            delayed_work_time_ = base::TimeTicks();
            break;
        }

        if (isWindowsMessagePending())
            break;

        if (moreWorkIsPlausible)
            continue;

        moreWorkIsPlausible = state_->delegate->DoIdleWork();
        if (state_->should_quit) {
            // Reset these flags so that scheduleMoreWorkIfNecessary will not
            // schedule any more work.
            DCHECK(!d_immediateWorkIsPlausible);
            delayed_work_time_ = base::TimeTicks();
            break;
        }

        if (!moreWorkIsPlausible || isWindowsMessagePending())
            break;
    }
}

void MainMessagePump::scheduleMoreWorkIfNecessary()
{
    if (d_immediateWorkIsPlausible) {
        // Use a WM_TIMER if we are in a modal loop.  This puts the chromium
        // work at a lower priority for the duration of the modal loop.
        // Also, don't ScheduleWork immediately if there is a Windows message
        // pending.  We'll wait for that message to finish processing, then
        // schedule the work in postHandleMessage.
        if (PumpMode::AUTOMATIC == Statics::pumpMode || MessageLoop::current()->os_modal_loop())
            ScheduleDelayedWork(base::TimeTicks::Now());
        else if (!isWindowsMessagePending())
            ScheduleWork();
    }
    else if (!delayed_work_time_.is_null())
        ScheduleDelayedWork(delayed_work_time_);
}

void MainMessagePump::handleWorkMessage()
{
    // Since we discarded a kMsgHaveWork message, we must update the flag.
    int old_have_work = InterlockedExchange(&have_work_, 0);
    DCHECK(old_have_work);

    // Return immediately if we are called outside the scope of init/cleanup.
    if (!state_)
        return;

    // Set this to true here, in case doWorkUntilNextMessage returns
    // immediately due to a Windows message in the queue.  This makes us
    // schedule work again.
    d_immediateWorkIsPlausible = true;

    if (PumpMode::AUTOMATIC == Statics::pumpMode || MessageLoop::current()->os_modal_loop()) {
        // If we are in a modal loop, don't do work when we get kMsgHaveWork.
        // Instead, do work when we get WM_TIMER.  This forces chromium's work
        // to run at a lower priority, and prevents the Windows message queue
        // from starving.
        ScheduleDelayedWork(base::TimeTicks::Now());
        return;
    }

    doWorkUntilNextMessage();
    scheduleMoreWorkIfNecessary();
}

void MainMessagePump::handleTimerMessage()
{
    KillTimer(message_hwnd_, reinterpret_cast<UINT_PTR>(this));

    // Return immediately if we are called outside the scope of init/cleanup.
    if (!state_)
        return;

    doWorkUntilNextMessage();
    scheduleMoreWorkIfNecessary();
}

// static
LRESULT CALLBACK MainMessagePump::wndProcThunk(HWND hwnd,
                                               UINT message,
                                               WPARAM wparam,
                                               LPARAM lparam)
{
    switch (message) {
    case kMsgHaveWork:
        reinterpret_cast<MainMessagePump*>(wparam)->handleWorkMessage();
        break;
    case WM_TIMER:
        reinterpret_cast<MainMessagePump*>(wparam)->handleTimerMessage();
        break;
    }
    return DefWindowProc(hwnd, message, wparam, lparam);
}

}  // close namespace blpwtk2

