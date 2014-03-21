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

#include <blpwtk2_profileproxy.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_browsercontextimplmanager.h>
#include <blpwtk2_processclient.h>
#include <blpwtk2_profile_messages.h>
#include <blpwtk2_proxyconfig.h>
#include <blpwtk2_proxyconfigimpl.h>
#include <blpwtk2_spellcheckconfig.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>

#include <base/bind.h>
#include <base/message_loop/message_loop.h>
#include <ipc/ipc_sender.h>

namespace blpwtk2 {

ProfileProxy::ProfileProxy(ProcessClient* processClient,
                           int routingId,
                           const std::string& dataDir,
                           bool diskCacheEnabled,
                           bool cookiePersistenceEnabled)
: d_processClient(processClient)
, d_routingId(routingId)
, d_numWebViews(0)
{
    // If disk-cache/cookie-persistence is enabled, then it must not be
    // incognito.
    DCHECK(!diskCacheEnabled || !dataDir.empty());
    DCHECK(!cookiePersistenceEnabled || !dataDir.empty());
    DCHECK(d_processClient);

    ++Statics::numProfiles;

    Send(new BlpProfileHostMsg_New(routingId,
                                   dataDir,
                                   diskCacheEnabled,
                                   cookiePersistenceEnabled));
}

ProfileProxy::~ProfileProxy()
{
}

void ProfileProxy::incrementWebViewCount()
{
    DCHECK(Statics::isInApplicationMainThread());
    ++d_numWebViews;
}

void ProfileProxy::decrementWebViewCount()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(0 < d_numWebViews);
    --d_numWebViews;
}

int ProfileProxy::routingId() const
{
    return d_routingId;
}

// Profile overrides

void ProfileProxy::destroy()
{
    DCHECK(Statics::isInApplicationMainThread());
    DCHECK(0 == d_numWebViews);

    DCHECK(0 < Statics::numProfiles);
    --Statics::numProfiles;

    Send(new BlpProfileHostMsg_Destroy(d_routingId));

    delete this;
}

void ProfileProxy::setProxyConfig(const ProxyConfig& config)
{
    DCHECK(Statics::isInApplicationMainThread());

    Send(new BlpProfileHostMsg_SetProxyConfig(d_routingId, config));
}

void ProfileProxy::useSystemProxyConfig()
{
    DCHECK(Statics::isInApplicationMainThread());

    Send(new BlpProfileHostMsg_UseSystemProxyConfig(d_routingId));
}

void ProfileProxy::setSpellCheckConfig(const SpellCheckConfig& config)
{
    DCHECK(Statics::isInApplicationMainThread());

    // Auto-correct cannot be enabled if spellcheck is disabled.
    DCHECK(SpellCheckConfig::AUTOCORRECT_NONE == config.autocorrectBehavior()
        || config.isSpellCheckEnabled());

    Send(new BlpProfileHostMsg_SetSpellCheckConfig(d_routingId, config));
}

void ProfileProxy::addCustomWords(const StringRef* words,
                                  size_t numWords)
{
    DCHECK(Statics::isInApplicationMainThread());
    std::vector<std::string> wordsVector(numWords);
    for (size_t i = 0; i < numWords; ++i) {
        wordsVector[i].assign(words[i].data(), words[i].length());
    }
    Send(new BlpProfileHostMsg_AddCustomWords(d_routingId, wordsVector));
}

void ProfileProxy::removeCustomWords(const StringRef* words,
                                     size_t numWords)
{
    DCHECK(Statics::isInApplicationMainThread());
    std::vector<std::string> wordsVector(numWords);
    for (size_t i = 0; i < numWords; ++i) {
        wordsVector[i].assign(words[i].data(), words[i].length());
    }
    Send(new BlpProfileHostMsg_RemoveCustomWords(d_routingId, wordsVector));
}

void ProfileProxy::addAutocorrectWords(const StringRef* badWords,
                                       const StringRef* goodWords,
                                       size_t numWords)
{
    DCHECK(Statics::isInApplicationMainThread());
    std::vector<std::string> badWordsVector(numWords);
    std::vector<std::string> goodWordsVector(numWords);
    for (size_t i = 0; i < numWords; ++i) {
        badWordsVector[i].assign(badWords[i].data(), badWords[i].length());
        goodWordsVector[i].assign(goodWords[i].data(), goodWords[i].length());
    }
    Send(new BlpProfileHostMsg_AddAutocorrectWords(d_routingId,
                                                   badWordsVector,
                                                   goodWordsVector));
}

void ProfileProxy::removeAutocorrectWords(const StringRef* badWords,
                                          size_t numWords)
{
    DCHECK(Statics::isInApplicationMainThread());
    std::vector<std::string> badWordsVector(numWords);
    for (size_t i = 0; i < numWords; ++i) {
        badWordsVector[i].assign(badWords[i].data(), badWords[i].length());
    }
    Send(new BlpProfileHostMsg_RemoveAutocorrectWords(d_routingId,
                                                      badWordsVector));
}

// IPC::Sender override

bool ProfileProxy::Send(IPC::Message* message)
{
    return d_processClient->Send(message);
}

}  // close namespace blpwtk2

