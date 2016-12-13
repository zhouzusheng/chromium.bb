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

#ifndef INCLUDED_MINIKIT_TOOLKITFACTORY_H
#define INCLUDED_MINIKIT_TOOLKITFACTORY_H

#include <minikit_config.h>

namespace minikit {

class Toolkit;
class ToolkitCreateParams;

// Factory for creating the Toolkit object.  The Toolkit object can be used to
// create WebViews.  Each process can only create one Toolkit object.
struct MINIKIT_EXPORT ToolkitFactory {

    // Create and return a Toolkit object that is configured using the
    // specified 'params'.  The returned Toolkit object should be destroyed
    // using the 'Toolkit::destroy()' method before the minikit DLL is
    // unloaded.  Note that only one Toolkit object can ever be created by each
    // process.
    static Toolkit* create(const ToolkitCreateParams& params);
};

}  // close namespace minikit

#endif  // INCLUDED_MINIKIT_TOOLKITCREATEPARAMS_H

