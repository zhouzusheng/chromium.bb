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

#include <blpwtk2_newviewparams.h>
#include <vector>

namespace blpwtk2 {

struct NewViewParamsImpl {
    NewViewDisposition::Value d_disposition;
    float d_x, d_y, d_width, d_height;
    bool d_isXSet, d_isYSet, d_isWidthSet, d_isHeightSet;
    std::string d_targetUrl;
    std::vector<std::string> d_additionalFeatures;

    NewViewParamsImpl()
    : d_disposition(NewViewDisposition::NEW_WINDOW)
    , d_isXSet(false)
    , d_isYSet(false)
    , d_isWidthSet(false)
    , d_isHeightSet(false)
    {
    }
};

NewViewParams::NewViewParams()
: d_impl(new NewViewParamsImpl())
{

}

NewViewParams::NewViewParams(const NewViewParams& other)
: d_impl(new NewViewParamsImpl(*other.d_impl))
{
}

NewViewParams::~NewViewParams()
{
    delete d_impl;
}

NewViewParams& NewViewParams::operator=(const NewViewParams& rhs)
{
    if (this != &rhs) {
        *d_impl = *rhs.d_impl;
    }
    return *this;
}

void NewViewParams::setDisposition(NewViewDisposition::Value value)
{
    d_impl->d_disposition = value;
}

void NewViewParams::setTargetUrl(const StringRef& value)
{
    d_impl->d_targetUrl.assign(value.data(), value.length());
}

void NewViewParams::setX(float value)
{
    d_impl->d_x = value;
    d_impl->d_isXSet = true;
}

void NewViewParams::setY(float value)
{
    d_impl->d_y = value;
    d_impl->d_isYSet = true;
}

void NewViewParams::setWidth(float value)
{
    d_impl->d_width = value;
    d_impl->d_isWidthSet = true;
}

void NewViewParams::setHeight(float value)
{
    d_impl->d_height = value;
    d_impl->d_isHeightSet = true;
}

void NewViewParams::addAdditionalFeature(const StringRef& feature)
{
    d_impl->d_additionalFeatures.push_back(std::string(feature.data(), feature.length()));
}

NewViewDisposition::Value NewViewParams::disposition() const
{
    return d_impl->d_disposition;
}

StringRef NewViewParams::targetUrl() const
{
    return d_impl->d_targetUrl;
}

bool NewViewParams::isXSet() const
{
    return d_impl->d_isXSet;
}

float NewViewParams::x() const
{
    return d_impl->d_x;
}

bool NewViewParams::isYSet() const
{
    return d_impl->d_isYSet;
}

float NewViewParams::y() const
{
    return d_impl->d_y;
}

bool NewViewParams::isWidthSet() const
{
    return d_impl->d_isWidthSet;
}

float NewViewParams::width() const
{
    return d_impl->d_width;
}

bool NewViewParams::isHeightSet() const
{
    return d_impl->d_isHeightSet;
}

float NewViewParams::height() const
{
    return d_impl->d_height;
}

size_t NewViewParams::additionalFeatureCount() const
{
    return d_impl->d_additionalFeatures.size();
}

StringRef NewViewParams::additionalFeatureAt(size_t index) const
{
    return d_impl->d_additionalFeatures[index];
}

}  // close namespace blpwtk2
