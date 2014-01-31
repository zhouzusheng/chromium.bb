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

#include <blpwtk2_ncdragutil.h>

#include <blpwtk2_statics.h>

#include <base/bind.h>
#include <base/message_loop.h>
#include <ui/gfx/screen.h>

namespace blpwtk2 {

// Data shared between the browser thread and the application's main thread.
// A static array of these is maintained, and we cycle through them, using an
// instance for each drag-drop operation.  If we don't find any instances
// available, that means acks are not being delivered properly for some reason.
// In debug builds, we assert that this never happens.
struct DragData {
    int hitTestCode;
    RECT startRect;
    HWND root;
    POINT startPoint;
    POINT endPoint;
    MINMAXINFO minMaxInfo;

    // These flags are only touched in the browser thread.
    bool ended;
#ifndef NDEBUG
    bool available;
#endif
};

// Our static array of DragData objects.  10 slots should be more than enough!
static const int MAX_DRAG_DATA = 10;
static DragData s_dragData[MAX_DRAG_DATA];

// A resize drag-update or a drag-end would require an ack.  Nothing is posted
// to the application's thread until the ack is received.
static bool s_ackPending = false;

// The browser thread's opinion about what the current drag data index is.
static int s_currentDragIndex = -1;

static void onBrowserDragUpdateAck(int index, const POINT& lastDragPoint);
static void onBrowserDragEndAck(int index);

static void onMainDragBegin(int index)
{
    DCHECK(Statics::isInApplicationMainThread());

    DragData& dd = s_dragData[index];
    MINMAXINFO tmpMinMaxInfo = dd.minMaxInfo;
    if (0 == ::SendMessage(dd.root, WM_GETMINMAXINFO, 0,
                           (LPARAM)&tmpMinMaxInfo)) {
        const POINT& systemMinSize = dd.minMaxInfo.ptMinTrackSize;
        const POINT& systemMaxSize = dd.minMaxInfo.ptMaxTrackSize;
        POINT& tmpMinSize = tmpMinMaxInfo.ptMinTrackSize;
        POINT& tmpMaxSize = tmpMinMaxInfo.ptMaxTrackSize;

        DCHECK(tmpMinSize.x <= tmpMaxSize.x);
        DCHECK(tmpMinSize.y <= tmpMaxSize.y);

        tmpMinSize.x = std::max(tmpMinSize.x, systemMinSize.x);
        tmpMinSize.y = std::max(tmpMinSize.y, systemMinSize.y);
        tmpMaxSize.x = std::min(tmpMaxSize.x, systemMaxSize.x);
        tmpMaxSize.y = std::min(tmpMaxSize.y, systemMaxSize.y);

        dd.minMaxInfo = tmpMinMaxInfo;
    }
    ::SendMessage(dd.root, WM_ENTERSIZEMOVE, 0, 0);
}

static int clamp(int value, int minValue, int maxValue)
{
    DCHECK(minValue <= maxValue);
    return std::min(maxValue, std::max(minValue, value));
}

static void onMainDragUpdateToPoint(const DragData& dd, const POINT& pt)
{
    DCHECK(Statics::isInApplicationMainThread());

    int dx = pt.x - dd.startPoint.x;
    int dy = pt.y - dd.startPoint.y;
    RECT finalRect;
    int sizeType = 0;
    int minX, minY, maxX, maxY;
    switch (dd.hitTestCode) {
    case HTCAPTION:
        finalRect.left = dd.startRect.left + dx;
        finalRect.top = dd.startRect.top + dy;
        finalRect.right = dd.startRect.right + dx;
        finalRect.bottom = dd.startRect.bottom + dy;
        break;
    case HTTOP:
        minY = dd.startRect.bottom - dd.minMaxInfo.ptMaxTrackSize.y;
        maxY = dd.startRect.bottom - dd.minMaxInfo.ptMinTrackSize.y;
        finalRect.left = dd.startRect.left;
        finalRect.top = clamp(dd.startRect.top + dy, minY, maxY);
        finalRect.right = dd.startRect.right;
        finalRect.bottom = dd.startRect.bottom;
        sizeType = WMSZ_TOP;
        break;
    case HTBOTTOM:
        minY = dd.startRect.top + dd.minMaxInfo.ptMinTrackSize.y;
        maxY = dd.startRect.top + dd.minMaxInfo.ptMaxTrackSize.y;
        finalRect.left = dd.startRect.left;
        finalRect.top = dd.startRect.top;
        finalRect.right = dd.startRect.right;
        finalRect.bottom = clamp(dd.startRect.bottom + dy, minY, maxY);
        sizeType = WMSZ_BOTTOM;
        break;
    case HTLEFT:
        minX = dd.startRect.right - dd.minMaxInfo.ptMaxTrackSize.x;
        maxX = dd.startRect.right - dd.minMaxInfo.ptMinTrackSize.x;
        finalRect.left = clamp(dd.startRect.left + dx, minX, maxX);
        finalRect.top = dd.startRect.top;
        finalRect.right = dd.startRect.right;
        finalRect.bottom = dd.startRect.bottom;
        sizeType = WMSZ_LEFT;
        break;
    case HTRIGHT:
        minX = dd.startRect.left + dd.minMaxInfo.ptMinTrackSize.x;
        maxX = dd.startRect.left + dd.minMaxInfo.ptMaxTrackSize.x;
        finalRect.left = dd.startRect.left;
        finalRect.top = dd.startRect.top;
        finalRect.right = clamp(dd.startRect.right + dx, minX, maxX);
        finalRect.bottom = dd.startRect.bottom;
        sizeType = WMSZ_RIGHT;
        break;
    case HTTOPLEFT:
        minY = dd.startRect.bottom - dd.minMaxInfo.ptMaxTrackSize.y;
        maxY = dd.startRect.bottom - dd.minMaxInfo.ptMinTrackSize.y;
        minX = dd.startRect.right - dd.minMaxInfo.ptMaxTrackSize.x;
        maxX = dd.startRect.right - dd.minMaxInfo.ptMinTrackSize.x;
        finalRect.left = clamp(dd.startRect.left + dx, minX, maxX);
        finalRect.top = clamp(dd.startRect.top + dy, minY, maxY);
        finalRect.right = dd.startRect.right;
        finalRect.bottom = dd.startRect.bottom;
        sizeType = WMSZ_TOPLEFT;
        break;
    case HTTOPRIGHT:
        minY = dd.startRect.bottom - dd.minMaxInfo.ptMaxTrackSize.y;
        maxY = dd.startRect.bottom - dd.minMaxInfo.ptMinTrackSize.y;
        minX = dd.startRect.left + dd.minMaxInfo.ptMinTrackSize.x;
        maxX = dd.startRect.left + dd.minMaxInfo.ptMaxTrackSize.x;
        finalRect.left = dd.startRect.left;
        finalRect.top = clamp(dd.startRect.top + dy, minY, maxY);
        finalRect.right = clamp(dd.startRect.right + dx, minX, maxX);
        finalRect.bottom = dd.startRect.bottom;
        sizeType = WMSZ_TOPRIGHT;
        break;
    case HTBOTTOMLEFT:
        minY = dd.startRect.top + dd.minMaxInfo.ptMinTrackSize.y;
        maxY = dd.startRect.top + dd.minMaxInfo.ptMaxTrackSize.y;
        minX = dd.startRect.right - dd.minMaxInfo.ptMaxTrackSize.x;
        maxX = dd.startRect.right - dd.minMaxInfo.ptMinTrackSize.x;
        finalRect.left = clamp(dd.startRect.left + dx, minX, maxX);
        finalRect.top = dd.startRect.top;
        finalRect.right = dd.startRect.right;
        finalRect.bottom = clamp(dd.startRect.bottom + dy, minY, maxY);
        sizeType = WMSZ_BOTTOMLEFT;
        break;
    case HTBOTTOMRIGHT:
        minY = dd.startRect.top + dd.minMaxInfo.ptMinTrackSize.y;
        maxY = dd.startRect.top + dd.minMaxInfo.ptMaxTrackSize.y;
        minX = dd.startRect.left + dd.minMaxInfo.ptMinTrackSize.x;
        maxX = dd.startRect.left + dd.minMaxInfo.ptMaxTrackSize.x;
        finalRect.left = dd.startRect.left;
        finalRect.top = dd.startRect.top;
        finalRect.right = clamp(dd.startRect.right + dx, minX, maxX);
        finalRect.bottom = clamp(dd.startRect.bottom + dy, minY, maxY);
        sizeType = WMSZ_BOTTOMRIGHT;
        break;
    default:
        DCHECK(false);
        break;
    }

    int flags = SWP_NOZORDER | SWP_NOOWNERZORDER;
    if (0 != sizeType)
        ::SendMessage(dd.root, WM_SIZING, sizeType, (LPARAM)&finalRect);
    else
        ::SendMessage(dd.root, WM_MOVING, 0, (LPARAM)&finalRect);

    SetWindowPos(dd.root, NULL,
                 finalRect.left, finalRect.top,
                 finalRect.right - finalRect.left,
                 finalRect.bottom - finalRect.top,
                 flags);
}

static void onMainDragUpdate(int index)
{
    DCHECK(Statics::isInApplicationMainThread());

    DragData& dd = s_dragData[index];

    POINT ptNow;
    ::GetCursorPos(&ptNow);

    onMainDragUpdateToPoint(dd, ptNow);

    if (dd.hitTestCode != HTCAPTION) {
        Statics::browserMainMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&onBrowserDragUpdateAck, index, ptNow));
    }
}

static void onMainDragEnd(int index)
{
    DCHECK(Statics::isInApplicationMainThread());

    DragData& dd = s_dragData[index];

    onMainDragUpdateToPoint(dd, dd.endPoint);
    ::SendMessage(dd.root, WM_EXITSIZEMOVE, 0, 0);

    Statics::browserMainMessageLoop->PostTask(
        FROM_HERE,
        base::Bind(&onBrowserDragEndAck, index));
}

static void onBrowserDragUpdateAck(int index, const POINT& lastDragPoint)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(s_ackPending);

    DragData& dd = s_dragData[index];

    if (dd.ended) {
        Statics::rendererMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&onMainDragEnd, index));
        return;
    }

    s_ackPending = false;

    POINT ptNow;
    ::GetCursorPos(&ptNow);

    if (lastDragPoint.x != ptNow.x || lastDragPoint.y != ptNow.y) {
        if (dd.hitTestCode != HTCAPTION)
            s_ackPending = true;
        Statics::rendererMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&onMainDragUpdate, index));
    }
}

static void onBrowserDragEndAck(int index)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(s_ackPending);

    DragData& dd = s_dragData[index];
    DCHECK(dd.ended);
#ifndef NDEBUG
    DCHECK(!dd.available);
    dd.available = true;
#endif

    s_ackPending = false;

    if (index == s_currentDragIndex) {
        // This would be the most common case, unless the user drags twice
        // really quickly before the ack comes back.
        return;
    }

    int newIndex = (index + 1) % MAX_DRAG_DATA;
    Statics::rendererMessageLoop->PostTask(
        FROM_HERE,
        base::Bind(&onMainDragBegin, newIndex));
    if (s_dragData[newIndex].ended) {
        s_ackPending = true;
        Statics::rendererMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&onMainDragEnd, newIndex));
    }
    else {
        if (s_dragData[newIndex].hitTestCode != HTCAPTION)
            s_ackPending = true;
        Statics::rendererMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&onMainDragUpdate, newIndex));
    }
}

void NCDragUtil::onDragBegin(HWND view, int hitTestCode, const POINT& pt)
{
    DCHECK(Statics::isInBrowserMainThread());

#ifndef NDEBUG
    // one-time initialization
    if (-1 == s_currentDragIndex) {
        for (int i = 0; i < MAX_DRAG_DATA; ++i) {
            s_dragData[i].available = true;
        }
    }
#endif

    s_currentDragIndex = (s_currentDragIndex + 1) % MAX_DRAG_DATA;
    DragData& dd = s_dragData[s_currentDragIndex];
#ifndef NDEBUG
    DCHECK(dd.available);
    dd.available = false;
#endif

    gfx::Screen* screen = gfx::Screen::GetScreenFor(view);
    gfx::Display primaryDisplay = screen->GetPrimaryDisplay();
    dd.minMaxInfo.ptMaxSize.x = primaryDisplay.bounds().width();
    dd.minMaxInfo.ptMaxSize.y = primaryDisplay.bounds().height();
    dd.minMaxInfo.ptMaxPosition.x = primaryDisplay.bounds().x();
    dd.minMaxInfo.ptMaxPosition.y = primaryDisplay.bounds().y();
    dd.minMaxInfo.ptMinTrackSize.x = GetSystemMetrics(SM_CXMINTRACK);
    dd.minMaxInfo.ptMinTrackSize.y = GetSystemMetrics(SM_CYMINTRACK);
    dd.minMaxInfo.ptMaxTrackSize.x = GetSystemMetrics(SM_CXMAXTRACK);
    dd.minMaxInfo.ptMaxTrackSize.y = GetSystemMetrics(SM_CYMAXTRACK);
    DCHECK(dd.minMaxInfo.ptMinTrackSize.x <= dd.minMaxInfo.ptMaxTrackSize.x);
    DCHECK(dd.minMaxInfo.ptMinTrackSize.y <= dd.minMaxInfo.ptMaxTrackSize.y);
    dd.ended = false;
    dd.hitTestCode = hitTestCode;
    dd.startPoint = pt;
    dd.root = ::GetAncestor(view, GA_ROOT);
    ::GetWindowRect(dd.root, &dd.startRect);

    if (!s_ackPending) {
        Statics::rendererMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&onMainDragBegin, s_currentDragIndex));
    }
}

void NCDragUtil::onDragMove()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(-1 != s_currentDragIndex);

    DragData& dd = s_dragData[s_currentDragIndex];
    DCHECK(!dd.ended);

    if (!s_ackPending) {
        if (dd.hitTestCode != HTCAPTION)
            s_ackPending = true;
        Statics::rendererMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&onMainDragUpdate, s_currentDragIndex));
    }
}

void NCDragUtil::onDragEnd()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(-1 != s_currentDragIndex);


    DragData& dd = s_dragData[s_currentDragIndex];
    DCHECK(!dd.ended);
    dd.ended = true;
    ::GetCursorPos(&dd.endPoint);

    if (!s_ackPending) {
        s_ackPending = true;
        Statics::rendererMessageLoop->PostTask(
            FROM_HERE,
            base::Bind(&onMainDragEnd, s_currentDragIndex));
    }
}

}  // close namespace blpwtk2

