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

#include <blpwtk2_devtoolshttphandlerdelegateimpl.h>

#include <blpwtk2_statics.h>

#include <base/command_line.h>
#include <base/files/file_path.h>
#include <base/path_service.h>
#include <base/strings/string_number_conversions.h>
#include <content/public/browser/devtools_http_handler.h>
#include <content/public/common/content_switches.h>
#include <net/socket/tcp_listen_socket.h>

namespace blpwtk2 {

DevToolsHttpHandlerDelegateImpl::DevToolsHttpHandlerDelegateImpl()
{
    int port = 0;
    const CommandLine& command_line = *CommandLine::ForCurrentProcess();
    if (command_line.HasSwitch(switches::kRemoteDebuggingPort)) {
        int temp_port;
        std::string port_str =
            command_line.GetSwitchValueASCII(switches::kRemoteDebuggingPort);
        if (base::StringToInt(port_str, &temp_port) &&
            temp_port > 0 && temp_port < 65535) {
                port = temp_port;
        } else {
            DLOG(WARNING) << "Invalid http debugger port number " << temp_port;
        }
    }

    Statics::devToolsHttpHandler = content::DevToolsHttpHandler::Start(
        new net::TCPListenSocketFactory("127.0.0.1", port), "", this);
}

DevToolsHttpHandlerDelegateImpl::~DevToolsHttpHandlerDelegateImpl()
{
    // Stop() will delete the object
    Statics::devToolsHttpHandler->Stop();
    Statics::devToolsHttpHandler = 0;
}

}  // close namespace blpwtk2

