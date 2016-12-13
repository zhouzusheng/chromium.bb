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

#ifndef INCLUDED_MINIKIT_CONSTANTS_H
#define INCLUDED_MINIKIT_CONSTANTS_H

#include <minikit_config.h>

namespace minikit {

struct Constants {

    // Constants that can be used for the 'affinity' parameter in
    // WebViewCreateParams::setRendererAffinity().
    enum {
        // Use any out-of-process renderer.  The actual renderer process that
        // will be used will be selected based on the internal heuristic used
        // by chromium (based on the site's domain etc).  See
        // https://sites.google.com/a/chromium.org/dev/developers/design-documents/process-models
        ANY_OUT_OF_PROCESS_RENDERER = -1,

        // Use the in-process renderer.
        IN_PROCESS_RENDERER = -2,
    };
};

}  // close namespace minikit

#endif  // INCLUDED_MINIKIT_CONSTANTS_H

