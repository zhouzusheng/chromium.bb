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

#ifndef INCLUDED_BLPWTK2_MEDIAREQUEST_H
#define INCLUDED_BLPWTK2_MEDIAREQUEST_H

namespace blpwtk2 {

class StringRef;

// This class provides information on media devices currently available, and
// ability to grant access to any of those devices.
// A pointer of this class will be passed into
// WebViewDelegate::handleMediaRequest(), where the application can retrieve
// the list of available media devices, ask the user for permission and grant
// access if wished.
class MediaRequest {

public:
    enum DeviceType {
        DEVICE_TYPE_UNKNOWN,
        DEVICE_TYPE_AUDIO,
        DEVICE_TYPE_VIDEO
    };

    virtual ~MediaRequest() = 0;

    // Returns media device count.
    virtual int deviceCount() const = 0;

    // Returns the user-friendly name of the device with the specified
    // 'index', e.g. "Microsoft LifeCam Cinema". Note that names are not
    // necessarily unique.
    virtual StringRef deviceName(int index) const = 0;

    // Returns the type (i.e. audio or video) of the device with the specified
    // 'index'.
    virtual DeviceType deviceType(int index) const = 0;

    // Grants permission to use the devices, whose ids are pointed to by the
    // specified 'deviceIndices' pointer.
    // Note that the MediaRequest object gets destroyed after this call,
    // therefore it should not be used in any way.
    virtual void grantAccess(int *deviceIndices, int deviceCount) = 0;

};

} // close namespace blpwtk2

#endif //INCLUDED_BLPWTK2_MEDIAREQUEST_H
