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

#include <blpwtk2_mediaobserverimpl.h>

namespace blpwtk2 {

MediaObserverImpl::MediaObserverImpl()
{

}

MediaObserverImpl::~MediaObserverImpl()
{

}

void MediaObserverImpl::OnCaptureDevicesOpened(
    int render_process_id,
    int render_view_id,
    const content::MediaStreamDevices& devices,
    const base::Closure& close_callback)
{
    //LILIT: what should we do here?
}

void MediaObserverImpl::OnCaptureDevicesClosed(
    int render_process_id,
    int render_view_id,
    const content::MediaStreamDevices& devices)
{
    //LILIT: what should we do here?
}

void MediaObserverImpl::OnAudioCaptureDevicesChanged(
    const content::MediaStreamDevices& devices)
{
    d_audioDevices.assign(devices.begin(), devices.end());
}

void MediaObserverImpl::OnVideoCaptureDevicesChanged(
    const content::MediaStreamDevices& devices)
{
    d_videoDevices.assign(devices.begin(), devices.end());
}

void MediaObserverImpl::OnMediaRequestStateChanged(
    int render_process_id,
    int render_view_id,
    const content::MediaStreamDevice& device,
    content::MediaRequestState state)
{
    //LILIT: what should we do here?
}

void MediaObserverImpl::OnAudioStreamPlayingChanged(
    int render_process_id,
    int render_view_id,
    int stream_id,
    bool playing)
{
    //LILIT: what should we do here?
}

const content::MediaStreamDevices& MediaObserverImpl::getAudioDevices() const
{
    return d_audioDevices;
}

const content::MediaStreamDevices& MediaObserverImpl::getVideoDevices() const
{
    return d_videoDevices;
}

} // close namespace blpwtk2
