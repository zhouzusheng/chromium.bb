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

#ifndef INCLUDED_BLPWTK2_PROFILECREATEPARAMS_H
#define INCLUDED_BLPWTK2_PROFILECREATEPARAMS_H

#include <blpwtk2_config.h>

#include <blpwtk2_string.h>

namespace blpwtk2 {

// This class contains parameters that are passed to blpwtk2 whenever the
// application wants to create a new Profile.  A browser will typically create
// a profile for each user on the system.
class BLPWTK2_EXPORT ProfileCreateParams {
  public:
    // The 'dataDir' parameter specifies the directory on disk where the user's
    // data will be stored (e.g. cookies, local storage, cache etc).  If the
    // 'dataDir' is an empty string, then an incognito profile will be created.
    // If the 'dataDir' is not an empty string, it must contain a valid path
    // name, and must not point to a file.
    // Note that profile settings (such as proxy/spelling configuration) are
    // not persisted in the 'dataDir', so applications are responsible for
    // restoring those settings (if desired) when the Profile is created.
    ProfileCreateParams(const StringRef& dataDir);

    // By default, profiles with a data directory will cache resources to disk
    // inside the data directory.  However, setting this flag to false will
    // disable that disk cache.  The behavior is undefined if there is no data
    // directory.
    void setDiskCacheEnabled(bool enabled);

    StringRef dataDir() const;
    bool diskCacheEnabled() const;

  private:
    String d_dataDir;
    bool d_diskCacheEnabled;
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_PROFILECREATEPARAMS_H

