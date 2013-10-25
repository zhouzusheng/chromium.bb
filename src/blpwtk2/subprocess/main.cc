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

#include <windows.h>  // NOLINT
#include <blpwtk2_products.h>
#include <content/public/app/startup_helper_win.h>  // for InitializeSandboxInfo
#include <sandbox/win/src/sandbox_types.h>  // for SandboxInterfaceInfo

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int)
{
    sandbox::SandboxInterfaceInfo sandboxInfo;
    content::InitializeSandboxInfo(&sandboxInfo);
    {
        HMODULE blpwtk2Module = LoadLibraryA(BLPWTK2_DLL_NAME);
        if (!blpwtk2Module) return -3456;
        typedef int (*MainFunc)(HINSTANCE hInstance,
                                sandbox::SandboxInterfaceInfo* sandboxInfo);
        MainFunc mainFunc = (MainFunc)GetProcAddress(blpwtk2Module, "SubProcessMain");
        if (!mainFunc) return -4567;
        return mainFunc(instance, &sandboxInfo);
    }
}


