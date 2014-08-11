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

#include <blpwtk2_channelinfo.h>

#include <blpwtk2_stringref.h>

#include <base/base64.h>
#include <base/command_line.h>
#include <base/pickle.h>
#include <base/strings/string_util.h>

namespace blpwtk2 {

const uint16 PICKLE_VERSION = 1;  // Increment this version whenever the
                                  // serialization format changes.

void ChannelInfo::loadSwitchesFromCommandLine(const CommandLine& commandLine)
{
    d_switches.clear();
    typedef CommandLine::SwitchMap::const_iterator SwitchIterator;
    const CommandLine::SwitchMap& switches = commandLine.GetSwitches();
    for (SwitchIterator it = switches.begin(); it != switches.end(); ++it) {
        d_switches.push_back(Switch());
        d_switches.back().d_key = it->first;
        d_switches.back().d_value = WideToASCII(it->second);
    }
}

std::string ChannelInfo::serialize() const
{
    Pickle pickle(sizeof(Pickle::Header));
    pickle.WriteUInt16(PICKLE_VERSION);
    pickle.WriteString(d_channelId);
    pickle.WriteInt(d_switches.size());
    for (size_t i = 0; i < d_switches.size(); ++i) {
        pickle.WriteString(d_switches[i].d_key);
        pickle.WriteString(d_switches[i].d_value);
    }
    base::StringPiece sp((const char*)pickle.data(), pickle.size());
    std::string result;
    bool check = Base64Encode(sp, &result);
    DCHECK(check);
    return result;
}

bool ChannelInfo::deserialize(const StringRef& data)
{
    base::StringPiece sp((const char*)data.data(), data.length());
    std::string decoded;
    if (!Base64Decode(sp, &decoded))
        return false;

    Pickle pickle(decoded.data(), decoded.size());
    PickleIterator it(pickle);
    uint16 versionCheck;
    if (!it.ReadUInt16(&versionCheck) || versionCheck != PICKLE_VERSION)
        return false;
    if (!it.ReadString(&d_channelId))
        return false;
    int length;
    if (!it.ReadLength(&length))
        return false;
    d_switches.resize(length);
    for (int i = 0; i < length; ++i) {
        if (!it.ReadString(&d_switches[i].d_key))
            return false;
        if (!it.ReadString(&d_switches[i].d_value))
            return false;
    }
    return true;
}

}  // close namespace blpwtk2
