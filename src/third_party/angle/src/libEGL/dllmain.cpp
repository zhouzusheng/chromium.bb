//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// dllmain.cpp: DLL entry point.

#include <windows.h>

extern "C" BOOL libEGLMain(DWORD reason);

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    return libEGLMain(reason);
}

