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

#ifndef INCLUDED_BLPWTK2_MEDIAREQUESTIMPL_H
#define INCLUDED_BLPWTK2_MEDIAREQUESTIMPL_H

#include <blpwtk2_mediarequest.h>

#include <base/compiler_specific.h>
#include <content/public/browser/web_contents_delegate.h>

namespace blpwtk2 {

// This class is our implementation of the blpwtk2::MediaRequest interface.
class MediaRequestImpl : public MediaRequest {

public:
    MediaRequestImpl(const content::MediaStreamDevices& devices,
                     const content::MediaResponseCallback& callback);
    virtual ~MediaRequestImpl();

    virtual int deviceCount() const OVERRIDE;
    virtual StringRef deviceName(int index) const OVERRIDE;
    virtual MediaRequest::DeviceType deviceType(int index) const OVERRIDE;
    virtual void grantAccess(int *deviceIndices, int deviceCount) OVERRIDE;

private:
    content::MediaStreamDevices d_mediaStreamDevices;
    content::MediaResponseCallback d_mediaResponseCallback;

    DISALLOW_COPY_AND_ASSIGN(MediaRequestImpl);
};

} // close namespace blpwtk2

#endif //INCLUDED_BLPWTK2_MEDIAREQUESTIMPL_H
