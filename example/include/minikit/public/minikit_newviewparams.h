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

#ifndef INCLUDED_MINIKIT_NEWVIEWPARAMS_H
#define INCLUDED_MINIKIT_NEWVIEWPARAMS_H

#include <minikit_config.h>

#include <minikit_string.h>

namespace minikit {

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

struct NewViewParamsImpl;

class MINIKIT_EXPORT NewViewParams {
  public:
    NewViewParams();
    NewViewParams(const NewViewParams& other);
    ~NewViewParams();
    NewViewParams& operator=(const NewViewParams& rhs);

    void setDisposition(NewViewDisposition::Value value);
    void setTargetUrl(const StringRef& value);
    void setX(float value);
    void setY(float value);
    void setWidth(float value);
    void setHeight(float value);
    void addAdditionalFeature(const StringRef& feature);

    NewViewDisposition::Value disposition() const;
    StringRef targetUrl() const;
    bool isXSet() const;
    float x() const;
    bool isYSet() const;
    float y() const;
    bool isWidthSet() const;
    float width() const;
    bool isHeightSet() const;
    float height() const;
    size_t additionalFeatureCount() const;
    StringRef additionalFeatureAt(size_t index) const;

  private:
    NewViewParamsImpl *d_impl;
};

}  // close namespace minikit

#endif  // INCLUDED_MINIKIT_NEWVIEWPARAMS_H

