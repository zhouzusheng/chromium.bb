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

#ifndef INCLUDED_BLPWTK2_REFCOUNTEDOBJECT_H
#define INCLUDED_BLPWTK2_REFCOUNTEDOBJECT_H

namespace blpwtk2 {

// This class represents a simple reference counted object.
// Users receiving an object of this type should call addRef() if they
// are planning to use it, and call release() when they are done.
class RefCountedObject {
  public:
    // Gain ownership on the object. Users should call release()
    // when they don't need it any more.
    virtual void addRef() = 0;

    // Release the object.
    virtual void release() = 0;

  protected:
    virtual ~RefCountedObject();
};

} // close namespace blpwtk2

#endif // INCLUDED_BLPWTK2_REFCOUNTEDOBJECT_H
