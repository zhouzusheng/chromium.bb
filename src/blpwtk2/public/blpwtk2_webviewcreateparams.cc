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

#include <blpwtk2_webviewcreateparams.h>

#include <blpwtk2_constants.h>

#include <base/logging.h>  // for DCHECK

namespace blpwtk2 {

WebViewCreateParams::WebViewCreateParams()
: d_initiallyVisible(true)
, d_takeKeyboardFocusOnMouseDown(true)
, d_takeLogicalFocusOnMouseDown(true)
, d_activateWindowOnMouseDown(true)
, d_domPasteEnabled(false)
, d_javascriptCanAccessClipboard(false)
, d_rendererAffinity(Constants::ANY_OUT_OF_PROCESS_RENDERER)
, d_profile(0)
, d_rerouteMouseWheelToAnyRelatedWindow(false)
{
}

void WebViewCreateParams::setInitiallyVisible(bool visible)
{
    d_initiallyVisible = visible;
}

void WebViewCreateParams::setTakeKeyboardFocusOnMouseDown(bool enable)
{
    d_takeKeyboardFocusOnMouseDown = enable;
}

void WebViewCreateParams::setTakeLogicalFocusOnMouseDown(bool enable)
{
    d_takeLogicalFocusOnMouseDown = enable;
}

void WebViewCreateParams::setActivateWindowOnMouseDown(bool enable)
{
    d_activateWindowOnMouseDown = enable;
}

void WebViewCreateParams::setDOMPasteEnabled(bool enable)
{
    d_domPasteEnabled = enable;
}

void WebViewCreateParams::setJavascriptCanAccessClipboard(bool enable)
{
    d_javascriptCanAccessClipboard = enable;
}

void WebViewCreateParams::setRendererAffinity(int affinity)
{
    DCHECK(affinity == Constants::ANY_OUT_OF_PROCESS_RENDERER
        || affinity == Constants::IN_PROCESS_RENDERER
        || affinity >= 0);

    d_rendererAffinity = affinity;
}

void WebViewCreateParams::setProfile(Profile* profile)
{
    d_profile = profile;
}

void WebViewCreateParams::setRerouteMouseWheelToAnyRelatedWindow(bool rerouteMouseWheelToAnyRelatedWindow)
{
    d_rerouteMouseWheelToAnyRelatedWindow = rerouteMouseWheelToAnyRelatedWindow;
}

}  // close namespace blpwtk2

