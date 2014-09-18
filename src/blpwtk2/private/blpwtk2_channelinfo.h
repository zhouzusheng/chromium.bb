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

#ifndef INCLUDED_BLPWTK2_CHANNELINFO_H
#define INCLUDED_BLPWTK2_CHANNELINFO_H

#include <blpwtk2_config.h>

#include <string>
#include <vector>

class CommandLine;

namespace blpwtk2 {

class StringRef;

// This struct contains the necessary information for setting up a channel
// between a blpwtk2 host process and blpwtk2 client process.  It contains
// methods to serialize and deserialize itself into a string so that it can be
// easily passed between processes.
struct ChannelInfo {
    struct Switch {
        std::string d_key;
        std::string d_value;
    };
    std::string d_channelId;
    std::vector<Switch> d_switches;

    // Load switches from the specified 'commandLine' into the 'd_switches'
    // array.
    void loadSwitchesFromCommandLine(const CommandLine& commandLine);

    // Serialize this 'ChannelInfo' into a string that can be passed to another
    // process.
    std::string serialize() const;

    // Deserialize the specified 'data' into this 'ChannelInfo' object.  Return
    // true if successful, and false otherwise.
    bool deserialize(const StringRef& data);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_CHANNELINFO_H
