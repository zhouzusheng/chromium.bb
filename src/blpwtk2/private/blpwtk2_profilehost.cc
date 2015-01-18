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

#include <blpwtk2_profilehost.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_browsercontextimplmanager.h>
#include <blpwtk2_processhost.h>
#include <blpwtk2_profile_messages.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>

#include <ipc/ipc_message_macros.h>

namespace blpwtk2 {

ProfileHost::ProfileHost(ProcessHost* processHost,
                         int routingId,
                         const std::string& dataDir,
                         bool diskCacheEnabled,
                         bool cookiePersistenceEnabled)
: d_processHost(processHost)
, d_routingId(routingId)
{
    d_processHost->addRoute(d_routingId, this);
    d_browserContext =
        Statics::browserContextImplManager->obtainBrowserContextImpl(
            dataDir,
            diskCacheEnabled,
            cookiePersistenceEnabled);
}

ProfileHost::~ProfileHost()
{
    d_browserContext->destroy();
    d_processHost->removeRoute(d_routingId);
}

BrowserContextImpl* ProfileHost::browserContext() const
{
    return d_browserContext;
}

// IPC::Listener overrides

bool ProfileHost::OnMessageReceived(const IPC::Message& message)
{
    bool handled = true;
    IPC_BEGIN_MESSAGE_MAP(ProfileHost, message)
        IPC_MESSAGE_HANDLER(BlpProfileHostMsg_SetProxyConfig, onSetProxyConfig)
        IPC_MESSAGE_HANDLER(BlpProfileHostMsg_UseSystemProxyConfig, onUseSystemProxyConfig)
        IPC_MESSAGE_HANDLER(BlpProfileHostMsg_SetSpellCheckConfig, onSetSpellCheckConfig)
        IPC_MESSAGE_HANDLER(BlpProfileHostMsg_AddCustomWords, onAddCustomWords)
        IPC_MESSAGE_HANDLER(BlpProfileHostMsg_RemoveCustomWords, onRemoveCustomWords)
        IPC_MESSAGE_HANDLER(BlpProfileHostMsg_AddAutocorrectWords, onAddAutocorrectWords)
        IPC_MESSAGE_HANDLER(BlpProfileHostMsg_RemoveAutocorrectWords, onRemoveAutocorrectWords)
        IPC_MESSAGE_UNHANDLED(handled = false)
    IPC_END_MESSAGE_MAP()

    return handled;
}

void ProfileHost::OnBadMessageReceived(const IPC::Message& message)
{
    LOG(ERROR) << "bad message " << message.type();
}

// Message handlers

void ProfileHost::onSetProxyConfig(const ProxyConfig& config)
{
    d_browserContext->setProxyConfig(config);
}

void ProfileHost::onUseSystemProxyConfig()
{
    d_browserContext->useSystemProxyConfig();
}

void ProfileHost::onSetSpellCheckConfig(const SpellCheckConfig& config)
{
    d_browserContext->setSpellCheckConfig(config);
}

void ProfileHost::onAddCustomWords(const std::vector<std::string>& words)
{
    std::vector<StringRef> wordRefs(words.size());
    for (size_t i = 0; i < words.size(); ++i) {
        wordRefs[i].assign(words[i].data(), words[i].length());
    }
    d_browserContext->addCustomWords(wordRefs.data(), wordRefs.size());
}

void ProfileHost::onRemoveCustomWords(const std::vector<std::string>& words)
{
    std::vector<StringRef> wordRefs(words.size());
    for (size_t i = 0; i < words.size(); ++i) {
        wordRefs[i].assign(words[i].data(), words[i].length());
    }
    d_browserContext->removeCustomWords(wordRefs.data(), wordRefs.size());
}

void ProfileHost::onAddAutocorrectWords(
    const std::vector<std::string>& badWords,
    const std::vector<std::string>& goodWords)
{
    DCHECK(badWords.size() == goodWords.size());
    std::vector<StringRef> badWordRefs(badWords.size());
    std::vector<StringRef> goodWordRefs(goodWords.size());
    for (size_t i = 0; i < badWords.size(); ++i) {
        badWordRefs[i].assign(badWords[i].data(), badWords[i].length());
        goodWordRefs[i].assign(goodWords[i].data(), goodWords[i].length());
    }
    d_browserContext->addAutocorrectWords(badWordRefs.data(),
                                          goodWordRefs.data(),
                                          badWordRefs.size());
}

void ProfileHost::onRemoveAutocorrectWords(
    const std::vector<std::string>& badWords)
{
    std::vector<StringRef> badWordRefs(badWords.size());
    for (size_t i = 0; i < badWords.size(); ++i) {
        badWordRefs[i].assign(badWords[i].data(), badWords[i].length());
    }
    d_browserContext->removeAutocorrectWords(badWordRefs.data(),
                                             badWordRefs.size());
}

}  // close namespace blpwtk2

