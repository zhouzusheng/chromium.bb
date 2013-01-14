# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../build/win_precompile.gypi',
    '../third_party/WebKit/Source/WebKit/chromium/features.gypi',
    'tools/test_shell/test_shell.gypi',
  ],
  'variables': {
    'chromium_code': 1,
  },
}
