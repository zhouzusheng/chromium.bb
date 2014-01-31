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

#include <blpwtk2_mediarequestimpl.h>

#include <blpwtk2_stringref.h>

#include <base/logging.h>
#include <content/public/common/media_stream_request.h>

namespace blpwtk2 {

// TODO: Somehow expose this to clients of blpwtk2.
class MediaStreamUIImpl : public content::MediaStreamUI {
  public:
    MediaStreamUIImpl() {}

    // Called when MediaStream capturing is started. Chrome layer can call |stop|
    // to stop the stream.
    virtual void OnStarted(const base::Closure& stop) {}

  private:
    DISALLOW_COPY_AND_ASSIGN(MediaStreamUIImpl);
};

MediaRequestImpl::MediaRequestImpl(const content::MediaStreamDevices& devices,
                                   const content::MediaResponseCallback& callback)
: d_mediaStreamDevices(devices)
, d_mediaResponseCallback(callback)
{

}

MediaRequestImpl::~MediaRequestImpl()
{

}

int MediaRequestImpl::deviceCount() const
{
    return d_mediaStreamDevices.size();
}

StringRef MediaRequestImpl::deviceName(int index) const
{
    DCHECK(index >= 0);
    DCHECK(index < (int)d_mediaStreamDevices.size());
    return d_mediaStreamDevices[index].name;
}

MediaRequest::DeviceType MediaRequestImpl::deviceType(int index) const
{
    DCHECK(index >= 0);
    DCHECK(index < (int)d_mediaStreamDevices.size());
    if (IsAudioMediaType(d_mediaStreamDevices[index].type)) {
        return MediaRequest::DEVICE_TYPE_AUDIO;
    } else if (IsVideoMediaType(d_mediaStreamDevices[index].type)) {
        return MediaRequest::DEVICE_TYPE_VIDEO;
    } else {
        NOTREACHED();
        return MediaRequest::DEVICE_TYPE_UNKNOWN;
    }
}

void MediaRequestImpl::grantAccess(int *deviceIndices, int deviceCount)
{
    content::MediaStreamDevices devices;
    for (int i = 0; i < deviceCount; ++i)
    {
        const int index = deviceIndices[i];
        DCHECK(index >= 0);
        DCHECK(index < (int) d_mediaStreamDevices.size());
        devices.push_back(d_mediaStreamDevices[index]);
    }

    scoped_ptr<content::MediaStreamUI> mediaStreamUI(new MediaStreamUIImpl());
    d_mediaResponseCallback.Run(devices, mediaStreamUI.Pass());
}

void MediaRequestImpl::addRef()
{
    // calling base::RefCountedThreadSafe::AddRef()
    AddRef();
}

void MediaRequestImpl::release()
{
    // calling base::RefCountedThreadSafe::Release()
    Release();
}

} // close namespace blpwtk2
