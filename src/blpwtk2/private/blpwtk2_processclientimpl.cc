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

#include <blpwtk2_processclientimpl.h>

#include <blpwtk2_control_messages.h>
#include <blpwtk2_inprocessrenderer.h>
#include <blpwtk2_statics.h>

#include <ipc/ipc_sync_channel.h>

namespace blpwtk2 {

ProcessClientImpl::ProcessClientImpl(
    const std::string& channelId,
    base::SingleThreadTaskRunner* ipcTaskRunner)
: d_shutdownEvent(true, false)
{
    d_channel = IPC::SyncChannel::Create(channelId,
                                         IPC::Channel::MODE_CLIENT,
                                         this,
                                         ipcTaskRunner,
                                         true,
                                         &d_shutdownEvent);
}

ProcessClientImpl::~ProcessClientImpl()
{
    d_shutdownEvent.Signal();
    d_channel.reset();
}

// ProcessClient overrides

void ProcessClientImpl::addRoute(int routingId, IPC::Listener* listener)
{
    LOG(INFO) << "Adding route: routingId(" << routingId << ")";
    d_routes.AddWithID(listener, routingId);
}

void ProcessClientImpl::removeRoute(int routingId)
{
    d_routes.Remove(routingId);
    LOG(INFO) << "Removed route: routingId(" << routingId << ")";
}

IPC::Listener* ProcessClientImpl::findListener(int routingId)
{
    return d_routes.Lookup(routingId);
}

// IPC::Sender overrides

bool ProcessClientImpl::Send(IPC::Message* message)
{
    return d_channel->Send(message);
}

// IPC::Listener overrides

bool ProcessClientImpl::OnMessageReceived(const IPC::Message& message)
{
    if (message.routing_id() == MSG_ROUTING_CONTROL) {
        // Dispatch control messages
        IPC_BEGIN_MESSAGE_MAP(ProcessClientImpl, message)
            IPC_MESSAGE_HANDLER(BlpControlMsg_SetInProcessRendererChannelName, onSetInProcessRendererChannelName)
            IPC_MESSAGE_UNHANDLED_ERROR()
        IPC_END_MESSAGE_MAP()

        return true;
    }

    // Dispatch incoming messages to the appropriate IPC::Listener.
    IPC::Listener* listener = d_routes.Lookup(message.routing_id());
    if (!listener) {
        if (message.is_sync()) {
            // The listener has gone away, so we must respond or else the
            // caller will hang waiting for a reply.
            IPC::Message* reply = IPC::SyncMessage::GenerateReply(&message);
            reply->set_reply_error();
            Send(reply);
        }

        LOG(WARNING) << "message received, but no listener: routingId("
                     << message.routing_id() << ") type(" << message.type()
                     << ")";
        return true;
    }

    return listener->OnMessageReceived(message);
}

void ProcessClientImpl::OnChannelConnected(int32 peer_pid)
{
    LOG(INFO) << "channel connected: peer_pid(" << peer_pid << ")";
}

void ProcessClientImpl::OnChannelError()
{
    LOG(ERROR) << "channel error!";
    if (Statics::channelErrorHandler) {
        Statics::channelErrorHandler(1);
    }
}

void ProcessClientImpl::OnBadMessageReceived(const IPC::Message& message)
{
    LOG(ERROR) << "bad message " << message.type();
}

// Control message handlers

void ProcessClientImpl::onSetInProcessRendererChannelName(
    const std::string& channelName)
{
    LOG(INFO) << "onSetInProcessRendererChannelName(" << channelName << ")";
    InProcessRenderer::setChannelName(channelName);
}

}  // close namespace blpwtk2
