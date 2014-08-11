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

#ifndef INCLUDED_BLPWTK2_MEDIAOBSERVERIMPL_H
#define INCLUDED_BLPWTK2_MEDIAOBSERVERIMPL_H

#include <base/callback_forward.h>
#include <base/compiler_specific.h>
#include <content/public/browser/media_observer.h>
#include <content/public/common/media_stream_request.h>

namespace blpwtk2 {

// This is our implementation of the content::MediaObserver interface, which
// allows us to handle media device related events such as media devices have
// opened, closed or changed.
// It also holds lists of currently available devices and has accessors for
// those.
class MediaObserverImpl : public content::MediaObserver {

public:

    MediaObserverImpl();
    virtual ~MediaObserverImpl();

    /////// content::MediaObserver overrides

    // Called when a audio capture device is plugged in or unplugged.
    virtual void OnAudioCaptureDevicesChanged(
        const content::MediaStreamDevices& devices) OVERRIDE;

    // Called when a video capture device is plugged in or unplugged.
    virtual void OnVideoCaptureDevicesChanged(
        const content::MediaStreamDevices& devices) OVERRIDE;

    // Called when a media request changes state.
    virtual void OnMediaRequestStateChanged(
        int render_process_id,
        int render_view_id,
        int page_request_id,
        const content::MediaStreamDevice& device,
        content::MediaRequestState state) OVERRIDE;

    // Called when an audio stream is played or paused.
    virtual void OnAudioStreamPlayingChanged(
        int render_process_id,
        int render_view_id,
        int stream_id,
        bool playing,
        float power_dbfs,
        bool clipped) OVERRIDE;

    // Called when the audio stream is being created.
    virtual void OnCreatingAudioStream(
        int render_process_id,
        int render_view_id) OVERRIDE;

    /////// Own accessors

    // Returns a vector of currently available audio devices.
    const content::MediaStreamDevices& getAudioDevices() const;

    // Returns a vector of currently available video devices.
    const content::MediaStreamDevices& getVideoDevices() const;

private:

    content::MediaStreamDevices d_audioDevices;
    content::MediaStreamDevices d_videoDevices;

    DISALLOW_COPY_AND_ASSIGN(MediaObserverImpl);
};

} // close namespace blpwtk2

#endif // INCLUDED_BLPWTK2_MEDIAOBSERVERIMPL_H
