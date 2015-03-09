/*
 * Copyright (C) 2015 Bloomberg Finance L.P.
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

#include <blpwtk2_contentutilityclientimpl.h>

#include <chrome/common/chrome_utility_messages.h>
#include <chrome/utility/printing_handler.h>
#include <content/public/utility/utility_thread.h>
#include <ipc/ipc_message_macros.h>

namespace blpwtk2 {

static bool Send(IPC::Message* message)
{
    return content::UtilityThread::Get()->Send(message);
}

ContentUtilityClientImpl::ContentUtilityClientImpl()
{
    d_handlers.push_back(new PrintingHandler());
}

ContentUtilityClientImpl::~ContentUtilityClientImpl()
{
}

//static
void ContentUtilityClientImpl::PreSandboxStartup()
{
    PrintingHandler::PreSandboxStartup();
}

bool ContentUtilityClientImpl::OnMessageReceived(const IPC::Message& message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(ContentUtilityClientImpl, message)
        IPC_MESSAGE_HANDLER(ChromeUtilityMsg_StartupPing, onStartupPing)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()

    for (Handlers::iterator it = d_handlers.begin(); !handled && it != d_handlers.end(); ++it)
        handled = (*it)->OnMessageReceived(message);

    return handled;
}

void ContentUtilityClientImpl::onStartupPing()
{
    Send(new ChromeUtilityHostMsg_ProcessStarted);
}

}  // close namespace blpwtk2
