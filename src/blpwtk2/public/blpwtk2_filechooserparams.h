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

#ifndef INCLUDED_BLPWTK2_FILECHOOSERPARAMS_H
#define INCLUDED_BLPWTK2_FILECHOOSERPARAMS_H

#include <blpwtk2_config.h>

namespace blpwtk2 {

class StringRef;

struct FileChooserParamsImpl;

// This class contains parameters that are passed to the application whenever
// the user needs to choose files from the file system.
class FileChooserParams {
  public:
    enum Mode {
        // Requires that the file exist before allowing the user to pick it.
        OPEN,

        // Like OPEN, but allows picking multiple files to open.
        OPEN_MULTIPLE,

        // Like OPEN, but selects a folder for upload.
        UPLOAD_FOLDER,

        // Allows picking a nonexistent file, and prompts to overwrite if the
        // file already exists.
        SAVE,
    };

    // Return the file chooser mode (whether we are opening a file, saving a
    // file etc).  See enumeration of possible values above.
    BLPWTK2_EXPORT Mode mode() const;

    // Return the title to be displayed in the file chooser.  Can be empty.
    BLPWTK2_EXPORT StringRef title() const;

    // Return the default file name.  Can be empty.
    BLPWTK2_EXPORT StringRef defaultFileName() const;

    // Return the number of "accept types" (i.e. mime types that would be
    // accepted).
    BLPWTK2_EXPORT size_t numAcceptTypes() const;

    // Return the "accept type" at the specified 'index'.  The behavior is
    // undefined if the specified 'index' is greater than or equal to the
    // 'numAcceptTypes()'.
    BLPWTK2_EXPORT StringRef acceptTypeAt(size_t index) const;


    // ----------- Non-exported methods --------------------

    FileChooserParams();
    ~FileChooserParams();

  private:
    // NOT IMPLEMENTED
    FileChooserParams(const FileChooserParams& other);
    FileChooserParams& operator=(const FileChooserParams& other);

    FileChooserParamsImpl* d_impl;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_FILECHOOSERPARAMS_H
