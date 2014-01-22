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

#ifndef INCLUDED_BLPWTK2_NEWVIEWPARAMS_H
#define INCLUDED_BLPWTK2_NEWVIEWPARAMS_H

#include <blpwtk2_config.h>

#include <blpwtk2_string.h>

namespace blpwtk2 {

struct NewViewDisposition {
    enum Value {
        DOWNLOAD,
        CURRENT_TAB,
        NEW_BACKGROUND_TAB,
        NEW_FOREGROUND_TAB,
        NEW_POPUP,
        NEW_WINDOW
    };
};

class NewViewParams {
  public:
    NewViewParams()
    : d_disposition(NewViewDisposition::NEW_WINDOW)
    , d_isXSet(false)
    , d_isYSet(false)
    , d_isWidthSet(false)
    , d_isHeightSet(false)
    , d_isHidden(false)
    , d_isTopMost(false)
    , d_isNoFocus(false)
    {
    }

    void setDisposition(NewViewDisposition::Value value)
    {
        d_disposition = value;
    }
    void setTargetUrl(const StringRef& value) { d_targetUrl.assign(value); }
    void setX(float value) { d_x = value; d_isXSet = true; }
    void setY(float value) { d_y = value; d_isYSet = true; }
    void setWidth(float value) { d_width = value; d_isWidthSet = true; }
    void setHeight(float value) { d_height = value; d_isHeightSet = true; }

    void setIsHidden(bool value) { d_isHidden = value; }
    void setIsTopMost(bool value) { d_isTopMost = value; }
    void setIsNoFocus(bool value) { d_isNoFocus = value; }

    NewViewDisposition::Value disposition() const { return d_disposition; }
    const String& targetUrl() const { return d_targetUrl; }
    bool isXSet() const { return d_isXSet; }
    float x() const { return d_x; }
    bool isYSet() const { return d_isYSet; }
    float y() const { return d_y; }
    bool isWidthSet() const { return d_isWidthSet; }
    float width() const { return d_width; }
    bool isHeightSet() const { return d_isHeightSet; }
    float height() const { return d_height; }

    bool isHidden() const { return d_isHidden; }
    bool isTopMost() const { return d_isTopMost; }
    bool isNoFocus() const { return d_isNoFocus; }

  private:
    NewViewDisposition::Value d_disposition;
    float d_x, d_y, d_width, d_height;
    bool d_isXSet, d_isYSet, d_isWidthSet, d_isHeightSet;
    bool d_isHidden, d_isTopMost, d_isNoFocus;
    String d_targetUrl;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_NEWVIEWPARAMS_H

