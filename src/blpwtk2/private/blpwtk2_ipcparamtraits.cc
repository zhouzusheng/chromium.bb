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

#include <blpwtk2_ipcparamtraits.h>

#include <blpwtk2_proxyconfig.h>
#include <blpwtk2_proxyconfigimpl.h>
#include <blpwtk2_spellcheckconfig.h>
#include <blpwtk2_stringref.h>

#include <ipc/ipc_message_utils.h>
#include <net/proxy/proxy_config.h>

using blpwtk2::getProxyConfigImpl;
using blpwtk2::ProxyConfig;
using blpwtk2::ProxyConfigImpl;
using blpwtk2::SpellCheckConfig;
using blpwtk2::StringRef;

namespace IPC {

// =================== net::ProxyServer ==================== //

template <>
struct ParamTraits<net::ProxyServer> {
    typedef net::ProxyServer param_type;
    static void Write(Message* m, const param_type& p);
    static bool Read(const Message* m, PickleIterator* iter, param_type* r);
    static void Log(const param_type& p, std::string* l);
};

void ParamTraits<net::ProxyServer>::Write(Message* m, const param_type& p)
{
    m->WriteInt(p.scheme());
    m->WriteString(p.host_port_pair().host());
    m->WriteUInt16(p.host_port_pair().port());
}

bool ParamTraits<net::ProxyServer>::Read(const Message* m, PickleIterator* iter, param_type* r)
{
    int scheme;
    if (!m->ReadInt(iter, &scheme))
        return false;
    r->internalScheme() = static_cast<net::ProxyServer::Scheme>(scheme);
    if (!m->ReadString(iter, &r->internalHostPortPair().internalHost()))
        return false;
    if (!m->ReadUInt16(iter, &r->internalHostPortPair().internalPort()))
        return false;
    return true;
}

void ParamTraits<net::ProxyServer>::Log(const param_type& p, std::string* l)
{
    l->append("ProxyServer(");
    LogParam((int)p.scheme(), l);
    l->append(", ");
    LogParam(p.host_port_pair().host(), l);
    l->append(", ");
    LogParam(p.host_port_pair().port(), l);
    l->append(")");
}

// =================== blpwtk2::ProxyConfig ==================== //

void ParamTraits<ProxyConfig>::Write(Message* m, const param_type& p)
{
    const net::ProxyConfig& impl = getProxyConfigImpl(p)->d_config;
    WriteParam(m, impl.has_pac_url());
    if (impl.has_pac_url()) {
        WriteParam(m, impl.pac_url().spec());
    }
    WriteParam(m, impl.proxy_rules().proxies_for_http.internalProxies());
    WriteParam(m, impl.proxy_rules().proxies_for_https.internalProxies());
    WriteParam(m, impl.proxy_rules().proxies_for_ftp.internalProxies());
    WriteParam(m, impl.proxy_rules().fallback_proxies.internalProxies());
    WriteParam(m, impl.proxy_rules().bypass_rules.ToString());
}

bool ParamTraits<ProxyConfig>::Read(const Message* m, PickleIterator* iter, param_type* r)
{
    bool hasPacUrl;
    net::ProxyConfig& impl = getProxyConfigImpl(*r)->d_config;
    if (!ReadParam(m, iter, &hasPacUrl))
        return false;
    if (hasPacUrl) {
        std::string pacUrl;
        if (!ReadParam(m, iter, &pacUrl))
            return false;
        impl.set_pac_url(GURL(pacUrl));
    }
    if (!ReadParam(m, iter, &impl.proxy_rules().proxies_for_http.internalProxies()))
        return false;
    if (!ReadParam(m, iter, &impl.proxy_rules().proxies_for_https.internalProxies()))
        return false;
    if (!ReadParam(m, iter, &impl.proxy_rules().proxies_for_ftp.internalProxies()))
        return false;
    if (!ReadParam(m, iter, &impl.proxy_rules().fallback_proxies.internalProxies()))
        return false;
    std::string bypassRules;
    if (!ReadParam(m, iter, &bypassRules))
        return false;
    impl.proxy_rules().bypass_rules.ParseFromString(bypassRules);
    return true;
}

void ParamTraits<ProxyConfig>::Log(const param_type& p, std::string* l)
{
    const net::ProxyConfig& impl = getProxyConfigImpl(p)->d_config;
    l->append("ProxyConfig(");
    if (impl.has_pac_url()) {
        l->append("pacurl: ");
        LogParam(impl.pac_url().spec(), l);
        l->append(", ");
    }
    LogParam(impl.proxy_rules().proxies_for_http.internalProxies(), l);
    l->append(", ");
    LogParam(impl.proxy_rules().proxies_for_https.internalProxies(), l);
    l->append(", ");
    LogParam(impl.proxy_rules().proxies_for_ftp.internalProxies(), l);
    l->append(", ");
    LogParam(impl.proxy_rules().fallback_proxies.internalProxies(), l);
    l->append(", ");
    LogParam(impl.proxy_rules().bypass_rules.ToString(), l);
    l->append(")");
}

// =================== blpwtk2::SpellCheckConfig ==================== //

void ParamTraits<SpellCheckConfig>::Write(Message* m, const param_type& p)
{
    // TODO: expose the internal vectors of SpellCheckConfig to make this
    // TODO: simpler.

    WriteParam(m, p.isSpellCheckEnabled());
    WriteParam(m, p.isAutoCorrectEnabled());
    m->WriteInt(p.numLanguages());
    for (size_t i = 0; i < p.numLanguages(); ++i) {
        StringRef str = p.languageAt(i);
        m->WriteInt(str.length());
        m->WriteBytes(str.data(), str.length());
    }
    m->WriteInt(p.numCustomWords());
    for (size_t i = 0; i < p.numCustomWords(); ++i) {
        StringRef str = p.customWordAt(i);
        m->WriteInt(str.length());
        m->WriteBytes(str.data(), str.length());
    }
}

bool ParamTraits<SpellCheckConfig>::Read(const Message* m,
                                         PickleIterator* iter,
                                         param_type* r)
{
    // TODO: expose the internal vectors of SpellCheckConfig to make this
    // TODO: simpler.

    bool boolValue;
    int length;
    const char* bytes;
    std::vector<StringRef> strs;

    if (!ReadParam(m, iter, &boolValue))
        return false;
    r->enableSpellCheck(boolValue);
    if (!ReadParam(m, iter, &boolValue))
        return false;
    r->enableAutoCorrect(boolValue);

    if (!m->ReadLength(iter, &length))
        return false;
    strs.resize(length);
    for (size_t i = 0; i < strs.size(); ++i) {
        if (!m->ReadLength(iter, &length))
            return false;
        if (!m->ReadBytes(iter, &bytes, length))
            return false;
        strs[i].assign(bytes, length);
    }
    r->setLanguages(strs.data(), strs.size());

    if (!m->ReadLength(iter, &length))
        return false;
    strs.resize(length);
    for (size_t i = 0; i < strs.size(); ++i) {
        if (!m->ReadLength(iter, &length))
            return false;
        if (!m->ReadBytes(iter, &bytes, length))
            return false;
        strs[i].assign(bytes, length);
    }
    r->setCustomWords(strs.data(), strs.size());

    return true;
}

void ParamTraits<SpellCheckConfig>::Log(const param_type& p, std::string* l)
{
    l->append("SpellCheckConfig(");
    // TODO: fill this
    l->append(")");
}

}  // close namespace IPC

