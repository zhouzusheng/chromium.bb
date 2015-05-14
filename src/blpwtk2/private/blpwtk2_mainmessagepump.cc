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

// IMPLEMENTATION NOTES
//
// Our pump has two modes (see blpwtk2_pumpmode.h):
// * MANUAL
//   This mode closely resembles the mechanism used in the upstream pump,
//   however it requires that the application notify us whenever it processes
//   a message.
//   When Chromium has work to do, it will post kMsgHaveWork.  In the upstream
//   pump, this posted message is simply used to "wake up" the message loop.
//   In our pump, the handler for this message is where we actually do the
//   work.  We keep doing work until there is no more work, or until
//   shouldStopDoingWork() returns true (which would typically happen if
//   Windows has something else to do).
//   If we still have more work to do, then we ScheduleWork() again *after*
//   processing a Windows message other than our own kMsgHaveWork message.
//   This simulates the ProcessPumpReplacementMessage logic in the upstream
//   message pump.  A couple of exceptions to this is when we are in a modal
//   loop, or if PeekMessage says there is no Windows message.  In these two
//   cases, our postHandleMessage() will not be called.  To work around this,
//   we start an auto-pump timer to do the remaining work.  This timer will
//   keep firing until we no longer have any work to do, or until
//   postHandleMessage is eventually called.
//
// * AUTOMATIC
//   This mode is completely new and not available in the upstream pump.  In
//   this mode, the application will *not* notify us whenever it processes a
//   message.  This means our postHandleMessage() will never be called, so we
//   always need an auto-pump timer to do any remaining work that we couldn't
//   complete the first time round.  The auto-pump timer will keep firing until
//   we no longer have any work to do.

#include <blpwtk2_mainmessagepump.h>

#include <blpwtk2_statics.h>

#include <base/run_loop.h>
#include <base/message_loop/message_loop.h>
#include <base/win/wrapped_window_proc.h>
#include <base/time/time.h>

namespace blpwtk2 {

static HHOOK s_msgHook;
static HHOOK s_callWndHook;
static bool s_isInModal = false;
static bool s_isMovingOrResizing = false;

static const int kMsgHaveWork = WM_USER + 1;

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
        base::MessageLoop::current()->SetNestableTasksAllowed(true);
        base::MessageLoop::current()->set_os_modal_loop(true);
        base::MessageLoop::current()->get_pump()->ScheduleWork();
    }
    return CallNextHookEx(s_msgHook, code, wParam, lParam);
}

static LRESULT CALLBACK CallWndHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    CWPSTRUCT* cwp = (CWPSTRUCT*)lParam;
    switch (cwp->message) {
    case WM_ENTERSIZEMOVE:
        DebugWithTime("HOOK ENTER SIZEMOVE\n");
        s_isMovingOrResizing = true;
        base::MessageLoop::current()->SetNestableTasksAllowed(true);
        base::MessageLoop::current()->set_os_modal_loop(true);
        base::MessageLoop::current()->get_pump()->ScheduleWork();
        break;
    case WM_EXITSIZEMOVE:
        DebugWithTime("HOOK EXIT SIZEMOVE\n");
        s_isMovingOrResizing = false;
        base::MessageLoop::current()->set_os_modal_loop(false);
        break;
    case WM_ENTERMENULOOP:
        DebugWithTime("HOOK ENTER MENU\n");
        base::MessageLoop::current()->SetNestableTasksAllowed(true);
        base::MessageLoop::current()->set_os_modal_loop(true);
        base::MessageLoop::current()->get_pump()->ScheduleWork();
        break;
    case WM_EXITMENULOOP:
        DebugWithTime("HOOK EXIT MENU\n");
        base::MessageLoop::current()->set_os_modal_loop(false);
        break;
    }
    return CallNextHookEx(s_callWndHook, code, wParam, lParam);
}

// Return true if we should stop doing any work.  This is determined based on
// whether there is any work in the Windows message queue.  This function
// attempts to flush out any messages that are processed internally by
// PeekMessage, to increase the likelihood of the next PeekMessage actually
// returning TRUE.  However, it is not always guaranteed that the next
// PeekMessage will return TRUE, so this function is just a hint.  If the
// specified 'stopForTimers' is true, then we will return true if there is a
// WM_TIMER in the message queue, otherwise WM_TIMERs are ignored.
static bool shouldStopDoingWork(bool stopForTimers)
{
    const int POST_MASK = QS_POSTMESSAGE | QS_ALLPOSTMESSAGE | QS_HOTKEY;
    const int INPUT_MASK = QS_MOUSE | QS_KEY;
    const int SENT_MASK = QS_SENDMESSAGE;

    int mask = QS_ALLINPUT;
    if (!stopForTimers)
        mask &= ~QS_TIMER;

    DWORD queueStatus = HIWORD(GetQueueStatus(mask));
    MSG msg;

    while (queueStatus & SENT_MASK) {
        // flush sent messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_QS_SENDMESSAGE))
            return true;
        queueStatus = HIWORD(GetQueueStatus(mask));
    }

    // Posted messages should be processed before input messages:
    //     http://msdn.microsoft.com/en-us/library/windows/desktop/ms644943(v=vs.85).aspx
    if (queueStatus & POST_MASK)
        return true;

    while (queueStatus & (INPUT_MASK | SENT_MASK)) {
        // flush input messages and sent messages (again)
        if (s_isMovingOrResizing) {
            // This is a special case!
            // The Windows move/resize loop seems to handle QS_INPUT
            // differently.  If we are in this mode, just return true so that
            // we exit our work loop and relinquish control to Windows.  Note
            // that if we have more work to do, a WM_TIMER will be scheduled
            // to handle that work (with a lower priority).
            DCHECK(base::MessageLoop::current()->os_modal_loop());
            return true;
        }
        if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_QS_INPUT | PM_QS_SENDMESSAGE))
            return true;
        queueStatus = HIWORD(GetQueueStatus(mask));
    }

    return queueStatus & mask;
}

// static
MainMessagePump* MainMessagePump::current()
{
    base::MessageLoop* loop = base::MessageLoop::current();
    DCHECK_EQ(base::MessageLoop::TYPE_UI, loop->type());
    return static_cast<MainMessagePump*>(loop->get_pump());
}

MainMessagePump::MainMessagePump()
: base::MessagePumpForUI(base::win::WrappedWindowProc<wndProcThunk>)
, d_hasAutoPumpTimer(false)
, d_moreWorkIsPlausible(false)
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
    base::MessageLoop::current()->PrepareRunHandler();
    PushRunState(&d_runState, base::MessageLoop::current(), 0);
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
    if (CallMsgFilter(const_cast<MSG*>(&msg), kMessageFilterCode)) {
        return true;
    }
    else {
        uint32_t action = base::MessagePumpDispatcher::POST_DISPATCH_PERFORM_DEFAULT;
        if (state_->dispatcher) {
            action = state_->dispatcher->Dispatch(msg);
            if (action & base::MessagePumpDispatcher::POST_DISPATCH_QUIT_LOOP)
                state_->should_quit = true;
        }

        // Returning false will make the app perform the default action.
        return !(action & base::MessagePumpDispatcher::POST_DISPATCH_PERFORM_DEFAULT);
    }
}

void MainMessagePump::postHandleMessage(const MSG& msg)
{
    // There is no Windows hook that notifies us when exiting a modal dialog
    // loop.  However, when postHandleMessage is called, we can assume that we
    // are back in the application's main loop, so turn off the modal loop flag
    // if it was set.
    if (s_isInModal) {
        DebugWithTime("EXITING MODAL LOOP\n");
        base::MessageLoop::current()->set_os_modal_loop(false);
        s_isInModal = false;
    }

    // If we have immediate work waiting, then scheduleMoreWorkIfNecessary may
    // have created an auto-pump timer.  However, since we are in MANUAL pump
    // mode, let's kill that timer and post kMsgHaveWork instead.  Note that we
    // only do this if we just processed a message other than our kMsgHaveWork.
    // This is to prevent the Windows queue from starving, because constantly
    // posting kMsgHaveWork will prevent input messages and system internal
    // messages from being processed by PeekMessage:
    //     http://msdn.microsoft.com/en-us/library/windows/desktop/ms644943(v=vs.85).aspx
    // This is similar to the ProcessPumpReplacementMessage logic in the
    // upstream message pump.
    if (d_moreWorkIsPlausible &&
        (msg.hwnd != message_hwnd_ || msg.message != kMsgHaveWork)) {
        if (d_hasAutoPumpTimer) {
            KillTimer(message_hwnd_, reinterpret_cast<UINT_PTR>(this));
            d_hasAutoPumpTimer = false;
        }
        ScheduleWork();
    }
}

void MainMessagePump::doWork()
{
    // Return immediately if we are called outside the scope of init/cleanup.
    if (!state_ || state_->should_quit)
        return;

    // If we are using an auto-pump timer, then we want to ignore WM_TIMER
    // messages so that our work will not be interrupted by our constant
    // auto-pump timer.  However, we don't want to let other WM_TIMERs starve.
    // So we will only do work for 100ms, at which point we will stop and let
    // the timer fire.
    // Note that for AUTOMATIC pump mode, we will do this even for the first
    // time round, when d_hasAutoPumpTimer would be false.
    const int TIME_LIMIT_MS = 100;
    bool ignoreTimers = d_hasAutoPumpTimer || PumpMode::AUTOMATIC == Statics::pumpMode;

    // startTime is only used if we want to ignore timers, otherwise it will be -1.
    DWORD startTime = ignoreTimers ? GetTickCount() : -1;

    while (true) {
        d_moreWorkIsPlausible = state_->delegate->DoWork();
        if (state_->should_quit)
            break;

        d_moreWorkIsPlausible |=
            state_->delegate->DoDelayedWork(&delayed_work_time_);

        bool stopForTimers = !ignoreTimers || (GetTickCount() - startTime) > TIME_LIMIT_MS;
        if (state_->should_quit || shouldStopDoingWork(stopForTimers))
            break;

        if (d_moreWorkIsPlausible)
            continue;

        d_moreWorkIsPlausible = state_->delegate->DoIdleWork();
        if (state_->should_quit || !d_moreWorkIsPlausible || shouldStopDoingWork(stopForTimers))
            break;
    }
}

void MainMessagePump::scheduleMoreWorkIfNecessary()
{
    bool shouldKillTimer = d_hasAutoPumpTimer;

    if (state_ && !state_->should_quit) {
        if (PumpMode::AUTOMATIC == Statics::pumpMode) {
            // If have_work_ is 1, that means ScheduleWork was called while we
            // were doing work.  In this case, don't kill the timer.  We'll set
            // have_work_ back to 2 so that we can know if ScheduleWork was
            // called again the next time round.
            if (1 == InterlockedCompareExchange(&have_work_, 2, 1)) {
                d_moreWorkIsPlausible = true;
            }
            // However, if we finished all our work, then try setting
            // have_work_ back to 0, only if it is still 2.  If it has
            // become 1 between now and the previous switch, that means
            // ScheduleWork was called (note: this can happen on a separate
            // thread!), so don't kill the timer.  Also, set have_work_ back
            // to 2 so that we can know if ScheduleWork was called again the
            // next time round.
            else if (!d_moreWorkIsPlausible && 1 == InterlockedCompareExchange(&have_work_, 0, 2)) {
                d_moreWorkIsPlausible = true;
                InterlockedExchange(&have_work_, 2);
            }
        }

        if (d_moreWorkIsPlausible) {
            shouldKillTimer = false;
        }
    }

    if (shouldKillTimer) {
        KillTimer(message_hwnd_, reinterpret_cast<UINT_PTR>(this));
        d_hasAutoPumpTimer = false;
    }

    if (!state_ || state_->should_quit) {
        // Reset flags so that postHandleMessage will not try to schedule
        // anything.
        d_moreWorkIsPlausible = false;
        delayed_work_time_ = base::TimeTicks();
        return;
    }

    if (d_moreWorkIsPlausible) {
        // If we are in AUTOMATIC mode, then any remaining work should be done
        // on a lower priority WM_TIMER.  This is necessary so that we give a
        // chance for Windows input and internal events to be processed.
        // In MANUAL mode, we will ScheduleWork() in postHandleMessage()
        // *after* handling a non-kMsgHaveWork message.  However, if we are in
        // a modal loop or if there are no messages in the Windows message
        // loop, then postHandleMessage() will not happen.  So in these two
        // cases, we need to use a timer.
        MSG msg;
        bool shouldUseAutoPumpTimer =
            PumpMode::AUTOMATIC == Statics::pumpMode
            || base::MessageLoop::current()->os_modal_loop()
            || FALSE == ::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

        if (shouldUseAutoPumpTimer) {
            ScheduleDelayedWork(base::TimeTicks::Now());
            d_hasAutoPumpTimer = true;
        }
    }
    else if (!delayed_work_time_.is_null())
        ScheduleDelayedWork(delayed_work_time_);
}

void MainMessagePump::handleWorkMessage()
{
    if (PumpMode::MANUAL == Statics::pumpMode) {
        // In MANUAL pump mode, have_work_ alternates between 0 and 1, where 1
        // indicates that there is a kMsgHaveWork in the queue.  Now that we
        // are processing the kMsgHaveWork message, set it back to 0.  This is
        // identical to how have_work_ is used in the upstream pump.
        int old_have_work = InterlockedExchange(&have_work_, 0);
        DCHECK(1 == old_have_work);
    }
    else {
        DCHECK(PumpMode::AUTOMATIC == Statics::pumpMode);

        // In AUTOMATIC pump mode, we have 3 values for have_work_:
        //   0: nothing is scheduled/happening.
        //   1: ScheduleWork has been called (kMsgHaveWork may or may not be in
        //      the queue, see below).
        //   2: work is happening, or we are waiting for the auto-pump timer.
        // The kMsgHaveWork is only posted if have_work_ is 0.  When
        // ScheduleWork is called, have_work_ will be set to 1.  This is the
        // same as what the upstream pump does.
        // However, when we start handling the kMsgHaveWork message, we set
        // have_work_ to 2 instead of 0 (which is what upstream does).  This
        // essentially prevents kMsgHaveWork from being posted again, but lets
        // us know whether ScheduleWork() has been called (in which case
        // have_work_ would go back to 1).  Preventing kMsgHaveWork from being
        // posted is important, because the posted message would preempt the
        // auto-pump timer, which we use to continue work not completed in the
        // first doWork().
        // When we finish doing some work (in doWork), we look at have_work_
        // again (this happens in scheduleMoreWorkIfNecessary).  If have_work_
        // has become 1, then we set it back to 2 and use the auto-pump timer
        // to do the rest of the work.  This timer will continue firing until
        // we no longer have any immediate work to do.  At this point,
        // have_work_ will be set back to zero, so everything goes back to the
        // original state.
        int old_have_work = InterlockedExchange(&have_work_, 2);
        DCHECK(1 == old_have_work || 2 == old_have_work);
    }

    doWork();
    scheduleMoreWorkIfNecessary();

    if (state_ && state_->should_quit) {
        LOG(INFO) << "MessagePump state should_quit == true";
    }
}

void MainMessagePump::handleTimerMessage()
{
    if (!d_hasAutoPumpTimer) {
        KillTimer(message_hwnd_, reinterpret_cast<UINT_PTR>(this));
        d_hasAutoPumpTimer = false;
    }

    doWork();
    scheduleMoreWorkIfNecessary();

    if (state_ && state_->should_quit) {
        LOG(INFO) << "MessagePump state should_quit == true";
    }
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

// MessagePump overrides

void MainMessagePump::ScheduleDelayedWork(const base::TimeTicks& delayed_work_time)
{
    // Verify that we never get called outside the main thread.
    DCHECK(this == base::MessageLoop::current()->get_pump());

    if (!d_hasAutoPumpTimer) {
        if (PumpMode::AUTOMATIC == Statics::pumpMode) {
            // If we are in AUTOMATIC mode, and an event is scheduled to be
            // executed immediately, then just make this our auto-pump timer.
            int64 us = (delayed_work_time - base::TimeTicks::Now()).InMicroseconds();
            if (0 >= us)
                d_hasAutoPumpTimer = true;
        }
        MessagePumpForUI::ScheduleDelayedWork(delayed_work_time);
    }
}

}  // close namespace blpwtk2

