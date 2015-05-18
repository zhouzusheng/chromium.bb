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

// IPC messages for global process control.
// Multiply-included file, hence no include guard.

#include <ipc/ipc_message_macros.h>

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT

#define IPC_MESSAGE_START BlpControlMsgStart

// ============== Messages from client to host ======================

// This syncs the ProcessClient with the ProcessHost.
IPC_SYNC_MESSAGE_CONTROL1_0(BlpControlHostMsg_Sync,
                            bool /*isFinalSync*/)

// Create a new ProcessHost and return the channelId.
IPC_SYNC_MESSAGE_CONTROL1_1(BlpControlHostMsg_CreateNewHostChannel,
                            int /* timeoutInMilliseconds */,
                            std::string /* channelInfo */)

// Clear the web cache on all renderer processes managed by the ProcessHost.
IPC_MESSAGE_CONTROL0(BlpControlHostMsg_ClearWebCache)


// ============= Messages from host to client =======================

IPC_MESSAGE_CONTROL1(BlpControlMsg_SetInProcessRendererChannelName,
                     std::string /* channelName */)
