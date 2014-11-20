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

#ifndef INCLUDED_BLPWTK2_JSWIDGET_H
#define INCLUDED_BLPWTK2_JSWIDGET_H

#include <blpwtk2_config.h>

#include <third_party/WebKit/public/web/WebPlugin.h>

namespace blink {
class WebFrame;
}  // close namespace blink

namespace blpwtk2 {

// This is a WebPlugin implementation that is created whenever there is an
// object element with "application/x-bloomberg-jswidget" mime type.  All it
// does is raise custom events on the DOM element whenever certain plugin
// callbacks are invoked.
class JsWidget : public blink::WebPlugin {
  public:
    explicit JsWidget(blink::WebFrame* frame);
    virtual ~JsWidget();

    // blink::WebPlugin overrides
    virtual bool initialize(blink::WebPluginContainer*) OVERRIDE;
    virtual void destroy() OVERRIDE;
    virtual NPObject* scriptableObject() OVERRIDE { return nullptr; }
    virtual void paint(blink::WebCanvas*, const blink::WebRect&) OVERRIDE {}
    virtual void updateGeometry(
        const blink::WebRect& frameRect, const blink::WebRect& clipRect,
        const blink::WebVector<blink::WebRect>& cutOutsRects, bool isVisible) OVERRIDE;
    virtual void updateFocus(bool) OVERRIDE {}
    virtual void updateVisibility(bool isVisible) OVERRIDE;
    virtual bool acceptsInputEvents() OVERRIDE { return false; }
    virtual bool handleInputEvent(const blink::WebInputEvent&, blink::WebCursorInfo&) OVERRIDE { return false; }
    virtual void didReceiveResponse(const blink::WebURLResponse&) OVERRIDE {}
    virtual void didReceiveData(const char* data, int dataLength) OVERRIDE {}
    virtual void didFinishLoading() OVERRIDE {}
    virtual void didFailLoading(const blink::WebURLError&) OVERRIDE {}
    virtual void didFinishLoadingFrameRequest(
        const blink::WebURL&, void* notifyData) OVERRIDE {}
    virtual void didFailLoadingFrameRequest(
        const blink::WebURL&, void* notifyData, const blink::WebURLError&) OVERRIDE {}

  private:
    blink::WebPluginContainer* d_container;
    blink::WebFrame* d_frame;

    DISALLOW_COPY_AND_ASSIGN(JsWidget);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_JSWIDGET_H
