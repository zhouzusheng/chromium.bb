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

#include <blpwtk2_contextmenuitem.h>
#include <blpwtk2_contextmenuitemimpl.h>
#include <blpwtk2_contextmenuparams.h>
#include <blpwtk2_contextmenuparamsimpl.h>
#include <blpwtk2_enumtraits.h>
#include <blpwtk2_findonpage.h>
#include <blpwtk2_newviewparams.h>
#include <blpwtk2_proxyconfig.h>
#include <blpwtk2_proxyconfigimpl.h>
#include <blpwtk2_spellcheckconfig.h>
#include <blpwtk2_stringref.h>

#include <ipc/ipc_message_utils.h>
#include <net/proxy/proxy_config.h>

using blpwtk2::getContextMenuItemImpl;
using blpwtk2::ContextMenuItem;
using blpwtk2::ContextMenuItemImpl;
using blpwtk2::getContextMenuParamsImpl;
using blpwtk2::ContextMenuParams;
using blpwtk2::ContextMenuParamsImpl;
using blpwtk2::FindOnPageRequest;
using blpwtk2::NewViewParams;
using blpwtk2::getProxyConfigImpl;
using blpwtk2::ProxyConfig;
using blpwtk2::ProxyConfigImpl;
using blpwtk2::SpellCheckConfig;
using blpwtk2::StringRef;

namespace IPC {

// ============== blpwtk2::ContextMenuItem =============== //

template <>
struct ParamTraits<blpwtk2::ContextMenuItem> {
    typedef blpwtk2::ContextMenuItem param_type;
    static void Write(Message* m, const param_type& p);
    static bool Read(const Message* m, PickleIterator* iter, param_type* r);
    static void Log(const param_type& p, std::string* l);
};

void ParamTraits<ContextMenuItem>::Write(Message* m, const param_type& p)
{
    const ContextMenuItemImpl& impl = *getContextMenuItemImpl(p);
    WriteParam(m, impl.d_label);
    WriteParam(m, impl.d_tooltip);
    WriteParam(m, impl.d_type);
    WriteParam(m, impl.d_action);
    WriteParam(m, impl.d_textDirection);
    WriteParam(m, impl.d_hasDirectionalOverride);
    WriteParam(m, impl.d_enabled);
    WriteParam(m, impl.d_checked);
    WriteParam(m, impl.d_submenu);
}

bool ParamTraits<ContextMenuItem>::Read(const Message* m, PickleIterator* iter, param_type* r)
{
    ContextMenuItemImpl& impl = *getContextMenuItemImpl(*r);
    if (!ReadParam(m, iter, &impl.d_label))
        return false;
    if (!ReadParam(m, iter, &impl.d_tooltip))
        return false;
    if (!ReadParam(m, iter, &impl.d_type))
        return false;
    if (!ReadParam(m, iter, &impl.d_action))
        return false;
    if (!ReadParam(m, iter, &impl.d_textDirection))
        return false;
    if (!ReadParam(m, iter, &impl.d_hasDirectionalOverride))
        return false;
    if (!ReadParam(m, iter, &impl.d_enabled))
        return false;
    if (!ReadParam(m, iter, &impl.d_checked))
        return false;
    if (!ReadParam(m, iter, &impl.d_submenu))
        return false;
    return true;
}

void ParamTraits<ContextMenuItem>::Log(const param_type& p, std::string* l)
{
    const ContextMenuItemImpl& impl = *getContextMenuItemImpl(p);
    l->append("ContextMenuItem(");
    LogParam(impl.d_label, l);
    l->append(", ");
    LogParam(impl.d_tooltip, l);
    l->append(", ");
    LogParam(impl.d_type, l);
    l->append(", ");
    LogParam(impl.d_action, l);
    l->append(", ");
    LogParam(impl.d_textDirection, l);
    l->append(", ");
    LogParam(impl.d_hasDirectionalOverride, l);
    l->append(", ");
    LogParam(impl.d_enabled, l);
    l->append(", ");
    LogParam(impl.d_checked, l);
    l->append(", ");
    LogParam(impl.d_submenu, l);
    l->append(")");
}

// ============== blpwtk2::ContextMenuParams =============== //

void ParamTraits<ContextMenuParams>::Write(Message* m, const param_type& p)
{
    const ContextMenuParamsImpl& impl = *getContextMenuParamsImpl(p);
    WriteParam(m, impl.d_misspelledWord);
    WriteParam(m, impl.d_pointOnScreen.x);
    WriteParam(m, impl.d_pointOnScreen.y);
    WriteParam(m, impl.d_canCut);
    WriteParam(m, impl.d_canCopy);
    WriteParam(m, impl.d_canPaste);
    WriteParam(m, impl.d_canDelete);
    WriteParam(m, impl.d_customItems);
    WriteParam(m, impl.d_suggestions);
}

bool ParamTraits<ContextMenuParams>::Read(const Message* m, PickleIterator* iter, param_type* r)
{
    ContextMenuParamsImpl& impl = *getContextMenuParamsImpl(*r);
    if (!ReadParam(m, iter, &impl.d_misspelledWord))
        return false;
    if (!ReadParam(m, iter, &impl.d_pointOnScreen.x))
        return false;
    if (!ReadParam(m, iter, &impl.d_pointOnScreen.y))
        return false;
    if (!ReadParam(m, iter, &impl.d_canCut))
        return false;
    if (!ReadParam(m, iter, &impl.d_canCopy))
        return false;
    if (!ReadParam(m, iter, &impl.d_canPaste))
        return false;
    if (!ReadParam(m, iter, &impl.d_canDelete))
        return false;
    if (!ReadParam(m, iter, &impl.d_customItems))
        return false;
    if (!ReadParam(m, iter, &impl.d_suggestions))
        return false;
    return true;
}

void ParamTraits<ContextMenuParams>::Log(const param_type& p, std::string* l)
{
    const ContextMenuParamsImpl& impl = *getContextMenuParamsImpl(p);
    l->append("ContextMenuParams(");
    LogParam(impl.d_misspelledWord, l);
    l->append(", ");
    LogParam(impl.d_pointOnScreen.x, l);
    l->append(", ");
    LogParam(impl.d_pointOnScreen.y, l);
    l->append(", ");
    LogParam(impl.d_canCut, l);
    l->append(", ");
    LogParam(impl.d_canCopy, l);
    l->append(", ");
    LogParam(impl.d_canPaste, l);
    l->append(", ");
    LogParam(impl.d_canDelete, l);
    l->append(", ");
    LogParam(impl.d_customItems, l);
    l->append(", ");
    LogParam(impl.d_suggestions, l);
    l->append(")");
}

// ============== blpwtk2::FindOnPageRequest =============== //

void ParamTraits<FindOnPageRequest>::Write(Message* m, const param_type& p)
{
    WriteParam(m, p.reqId);
    WriteParam(m, p.text);
    WriteParam(m, p.matchCase);
    WriteParam(m, p.findNext);
    WriteParam(m, p.forward);
}

bool ParamTraits<FindOnPageRequest>::Read(const Message* m, PickleIterator* iter, param_type* r)
{
    if (!ReadParam(m, iter, &r->reqId))
        return false;
    if (!ReadParam(m, iter, &r->text))
        return false;
    if (!ReadParam(m, iter, &r->matchCase))
        return false;
    if (!ReadParam(m, iter, &r->findNext))
        return false;
    if (!ReadParam(m, iter, &r->forward))
        return false;
    return true;
}

void ParamTraits<FindOnPageRequest>::Log(const param_type& p, std::string* l)
{
    l->append("FindOnPageRequest(");
    LogParam(p.reqId, l);
    l->append(", ");
    LogParam(p.text, l);
    l->append(", ");
    LogParam(p.matchCase, l);
    l->append(", ");
    LogParam(p.findNext, l);
    l->append(", ");
    LogParam(p.forward, l);
    l->append(")");
}

// ================= blpwtk2::NewViewParams ================== //

void ParamTraits<NewViewParams>::Write(Message* m, const param_type& p)
{
    m->WriteInt(p.disposition());
    {
        StringRef str = p.targetUrl();
        m->WriteInt(str.length());
        m->WriteBytes(str.data(), str.length());
    }
    m->WriteBool(p.isXSet());
    if (p.isXSet()) {
        m->WriteFloat(p.x());
    }
    m->WriteBool(p.isYSet());
    if (p.isYSet()) {
        m->WriteFloat(p.y());
    }
    m->WriteBool(p.isWidthSet());
    if (p.isWidthSet()) {
        m->WriteFloat(p.width());
    }
    m->WriteBool(p.isHeightSet());
    if (p.isHeightSet()) {
        m->WriteFloat(p.height());
    }

    m->WriteInt(p.additionalFeatureCount());
    for (size_t i = 0; i < p.additionalFeatureCount(); ++i) {
        StringRef str = p.additionalFeatureAt(i);
        m->WriteInt(str.length());
        m->WriteBytes(str.data(), str.length());
    }
}

bool ParamTraits<NewViewParams>::Read(const Message* m, PickleIterator* iter, param_type* r)
{
    int intValue;
    int length, strLength;
    const char* bytes;
    bool boolValue;
    float floatValue;

    if (!m->ReadInt(iter, &intValue))
        return false;
    r->setDisposition(static_cast<blpwtk2::NewViewDisposition::Value>(intValue));
    {
        int length;
        const char* bytes;
        if (!m->ReadLength(iter, &length))
            return false;
        if (!m->ReadBytes(iter, &bytes, length))
            return false;
        r->setTargetUrl(blpwtk2::StringRef(bytes, length));
    }
    if (!m->ReadBool(iter, &boolValue))
        return false;
    if (boolValue) {
        if (!m->ReadFloat(iter, &floatValue))
            return false;
        r->setX(floatValue);
    }
    if (!m->ReadBool(iter, &boolValue))
        return false;
    if (boolValue) {
        if (!m->ReadFloat(iter, &floatValue))
            return false;
        r->setY(floatValue);
    }
    if (!m->ReadBool(iter, &boolValue))
        return false;
    if (boolValue) {
        if (!m->ReadFloat(iter, &floatValue))
            return false;
        r->setWidth(floatValue);
    }
    if (!m->ReadBool(iter, &boolValue))
        return false;
    if (boolValue) {
        if (!m->ReadFloat(iter, &floatValue))
            return false;
        r->setHeight(floatValue);
    }

    if (!m->ReadLength(iter, &length))
        return false;
    for (int i = 0; i < length; ++i) {
        if (!m->ReadLength(iter, &strLength))
            return false;
        if (!m->ReadBytes(iter, &bytes, strLength))
            return false;
        r->addAdditionalFeature(StringRef(bytes, strLength));
    }

    return true;
}

void ParamTraits<NewViewParams>::Log(const param_type& p, std::string* l)
{
    l->append("NewViewParams(");
    LogParam((int)p.disposition(), l);
    l->append(", ");
    LogParam(std::string(p.targetUrl().data(),
                         p.targetUrl().length()), l);
    if (p.isXSet()) {
        l->append(", x:");
        LogParam(p.x(), l);
    }
    if (p.isYSet()) {
        l->append(", y:");
        LogParam(p.y(), l);
    }
    if (p.isWidthSet()) {
        l->append(", width:");
        LogParam(p.width(), l);
    }
    if (p.isHeightSet()) {
        l->append(", height:");
        LogParam(p.height(), l);
    }

    for (size_t i = 0; i < p.additionalFeatureCount(); ++i) {
        StringRef feature = p.additionalFeatureAt(i);
        l->append(", ");
        LogParam(std::string(feature.data(), feature.length()), l);
    }

    l->append(")");
}

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
    WriteParam(m, p.autocorrectBehavior());
    m->WriteInt(p.numLanguages());
    for (size_t i = 0; i < p.numLanguages(); ++i) {
        StringRef str = p.languageAt(i);
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
    int intValue;
    int length;
    const char* bytes;
    std::vector<StringRef> strs;

    if (!ReadParam(m, iter, &boolValue))
        return false;
    r->enableSpellCheck(boolValue);
    if (!ReadParam(m, iter, &intValue))
        return false;
    r->setAutocorrectBehavior(intValue);

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

    return true;
}

void ParamTraits<SpellCheckConfig>::Log(const param_type& p, std::string* l)
{
    l->append("SpellCheckConfig(");
    // TODO: fill this
    l->append(")");
}

}  // close namespace IPC

