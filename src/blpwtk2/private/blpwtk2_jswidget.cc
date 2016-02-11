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

#include <blpwtk2_jswidget.h>

#include <base/bind.h>
#include <base/message_loop/message_loop.h>
#include <third_party/WebKit/public/web/WebDOMEvent.h>
#include <third_party/WebKit/public/web/WebElement.h>
#include <third_party/WebKit/public/web/WebLocalFrame.h>
#include <third_party/WebKit/public/web/WebPluginContainer.h>
#include <third_party/WebKit/public/web/WebSerializedScriptValue.h>
#include <v8/include/v8.h>

namespace blpwtk2 {

static v8::Handle<v8::Object> toV8(v8::Isolate* isolate, const blink::WebRect& rc)
{
    // TODO: make a template for this
    v8::Handle<v8::Object> result = v8::Object::New(isolate);
    result->Set(v8::String::NewFromUtf8(isolate, "x"), v8::Integer::New(isolate, rc.x));
    result->Set(v8::String::NewFromUtf8(isolate, "y"), v8::Integer::New(isolate, rc.y));
    result->Set(v8::String::NewFromUtf8(isolate, "width"), v8::Integer::New(isolate, rc.width));
    result->Set(v8::String::NewFromUtf8(isolate, "height"), v8::Integer::New(isolate, rc.height));
    return result;
}

static void scheduleDispatchEvent(const tracked_objects::Location& location,
                                  JsWidget* widget,
                                  const blink::WebDOMEvent& event)
{
    base::MessageLoop::current()->PostTask(
        location,
        base::Bind(&JsWidget::dispatchEvent, base::Unretained(widget), event));
}

JsWidget::JsWidget(blink::WebLocalFrame* frame)
: d_container(nullptr)
, d_frame(frame)
{
}

JsWidget::~JsWidget()
{
}

void JsWidget::dispatchEvent(const blink::WebDOMEvent& event)
{
    d_webElement.dispatchEvent(event);
}

// blink::WebPlugin overrides

bool JsWidget::initialize(blink::WebPluginContainer* container)
{
    d_container = container;
    d_webElement = container->element();
    blink::WebDOMEvent event = blink::WebDOMEvent::createCustomEvent("bbOnInitialize", false, false, blink::WebSerializedScriptValue());
    scheduleDispatchEvent(FROM_HERE, this, event);
    return true;
}

void JsWidget::destroy()
{
    blink::WebDOMEvent event = blink::WebDOMEvent::createCustomEvent("bbOnDestroy", false, false, blink::WebSerializedScriptValue());
    scheduleDispatchEvent(FROM_HERE, this, event);
    base::MessageLoop::current()->DeleteSoon(FROM_HERE, this);
}

void JsWidget::updateGeometry(
    const blink::WebRect& windowRect, const blink::WebRect& clipRect,
    const blink::WebRect& unobscuredRect, const blink::WebVector<blink::WebRect>& cutOutsRects,
    bool isVisible)
{
    v8::Isolate* isolate = d_frame->scriptIsolate();

    v8::HandleScope handleScope(isolate);

    v8::Handle<v8::Context> context = d_frame->mainWorldScriptContext();
    v8::Context::Scope contextScope(context);

    v8::Handle<v8::Object> detailObj = v8::Object::New(isolate);
    detailObj->Set(v8::String::NewFromUtf8(isolate, "windowRect"), toV8(isolate, windowRect));
    detailObj->Set(v8::String::NewFromUtf8(isolate, "clipRect"), toV8(isolate, clipRect));
    detailObj->Set(v8::String::NewFromUtf8(isolate, "unobscuredRect"), toV8(isolate, unobscuredRect));
    detailObj->Set(v8::String::NewFromUtf8(isolate, "isVisible"), v8::Boolean::New(isolate, isVisible));

    v8::Handle<v8::Array> cutOutsRectsV8 = v8::Array::New(isolate, cutOutsRects.size());
    for (size_t i = 0; i < cutOutsRects.size(); ++i) {
        cutOutsRectsV8->Set(i, toV8(isolate, cutOutsRects[i]));
    }
    detailObj->Set(v8::String::NewFromUtf8(isolate, "cutOutsRects"), cutOutsRectsV8);

    // TODO: Remove once clients are updated.
    detailObj->Set(v8::String::NewFromUtf8(isolate, "frameRect"), toV8(isolate, windowRect));

    blink::WebDOMEvent event = blink::WebDOMEvent::createCustomEvent("bbOnUpdateGeometry", false, false,
                                                                     blink::WebSerializedScriptValue::serialize(detailObj));
    scheduleDispatchEvent(FROM_HERE, this, event);
}

void JsWidget::updateVisibility(bool isVisible)
{
    v8::Isolate* isolate = d_frame->scriptIsolate();

    v8::HandleScope handleScope(isolate);

    v8::Handle<v8::Context> context = d_frame->mainWorldScriptContext();
    v8::Context::Scope contextScope(context);

    v8::Handle<v8::Object> detailObj = v8::Object::New(isolate);
    detailObj->Set(v8::String::NewFromUtf8(isolate, "isVisible"), v8::Boolean::New(isolate, isVisible));

    blink::WebDOMEvent event = blink::WebDOMEvent::createCustomEvent("bbOnUpdateVisibility", false, false,
                                                                     blink::WebSerializedScriptValue::serialize(detailObj));
    scheduleDispatchEvent(FROM_HERE, this, event);
}

}  // close namespace blpwtk2
