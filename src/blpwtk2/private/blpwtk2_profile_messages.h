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

// IPC messages for Profile.
// Multiply-included file, hence no include guard.

#include <blpwtk2_ipcparamtraits.h>
#include <blpwtk2_proxyconfig.h>
#include <blpwtk2_spellcheckconfig.h>

#include <ipc/ipc_message_macros.h>

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT

#define IPC_MESSAGE_START BlpProfileMsgStart

// ============== Messages from client to host ======================

// This creates a new profile.
IPC_MESSAGE_CONTROL4(BlpProfileHostMsg_New,
                     int /* routingId */,
                     std::string /*dataDir*/,
                     bool /* diskCacheEnabled */,
                     bool /* cookiePersistenceEnabled */)

// Set the proxy configuration.
IPC_MESSAGE_ROUTED1(BlpProfileHostMsg_SetProxyConfig,
                    blpwtk2::ProxyConfig /* config */)

// Use the system proxy configuration.
IPC_MESSAGE_ROUTED0(BlpProfileHostMsg_UseSystemProxyConfig)

// Set the spellcheck configuration.
IPC_MESSAGE_ROUTED1(BlpProfileHostMsg_SetSpellCheckConfig,
                    blpwtk2::SpellCheckConfig /* config */)

// Add/remove custom words.
IPC_MESSAGE_ROUTED1(BlpProfileHostMsg_AddCustomWords,
                    std::vector<std::string> /* words */)
IPC_MESSAGE_ROUTED1(BlpProfileHostMsg_RemoveCustomWords,
                    std::vector<std::string> /* words */)

// Add/remove autocorrect words.
IPC_MESSAGE_ROUTED2(BlpProfileHostMsg_AddAutocorrectWords,
                    std::vector<std::string> /* badWords */,
                    std::vector<std::string> /* goodWords */)
IPC_MESSAGE_ROUTED1(BlpProfileHostMsg_RemoveAutocorrectWords,
                    std::vector<std::string> /* badWords */)

// This destroys the profile.
IPC_MESSAGE_CONTROL1(BlpProfileHostMsg_Destroy,
                     int /* routingId */)

