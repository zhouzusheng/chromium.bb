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

#include <blpwtk2_filechooserparams.h>

#include <blpwtk2_filechooserparamsimpl.h>
#include <blpwtk2_stringref.h>

#include <base/logging.h>

#include <vector>

namespace blpwtk2 {

FileChooserParams::Mode FileChooserParams::mode() const
{
    return static_cast<Mode>(d_impl->d_mode);
}

StringRef FileChooserParams::title() const
{
    return d_impl->d_title;
}

StringRef FileChooserParams::defaultFileName() const
{
    return d_impl->d_defaultFileName;
}

size_t FileChooserParams::numAcceptTypes() const
{
    return d_impl->d_acceptTypes.size();
}

StringRef FileChooserParams::acceptTypeAt(size_t index) const
{
    DCHECK(index < d_impl->d_acceptTypes.size());
    return d_impl->d_acceptTypes[index];
}


// ----------- Non-exported methods --------------------

FileChooserParams::FileChooserParams()
: d_impl(new FileChooserParamsImpl())
{
}

FileChooserParams::~FileChooserParams()
{
    delete d_impl;
}

}  // close namespace blpwtk2
