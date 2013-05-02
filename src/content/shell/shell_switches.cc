// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_switches.h"

namespace switches {

// SHEZ: Remove upstream code here, used only for testing.

// Makes Content Shell use the given path for its data directory.
const char kContentShellDataPath[] = "data-path";

// Enable accelerated 2D canvas.
const char kEnableAccelerated2DCanvas[] = "enable-accelerated-2d-canvas";

// Alias for kEnableSoftwareCompositingGLAdapter.
const char kEnableSoftwareCompositing[] = "enable-software-compositing";

}  // namespace switches
