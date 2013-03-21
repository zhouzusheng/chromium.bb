// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sandbox/win/src/process_mitigations.h"

#include "base/win/windows_version.h"
#include "sandbox/win/src/nt_internals.h"
#include "sandbox/win/src/sandbox_utils.h"
#include "sandbox/win/src/win_utils.h"

namespace {

// Functions for enabling policies.
typedef BOOL (WINAPI *SetProcessDEPPolicyFunction)(DWORD dwFlags);

// SHEZ: Remove upstream code here to compile with Win7.1 SDK

typedef BOOL (WINAPI *SetDefaultDllDirectoriesFunction)(
    DWORD DirectoryFlags);

}  // namespace

namespace sandbox {

bool ApplyProcessMitigationsToCurrentProcess(MitigationFlags flags) {
  if (!CanSetProcessMitigationsPostStartup(flags))
    return false;

  // We can't apply anything before Win XP, so just return cleanly.
  if (!IsXPSP2OrLater())
    return true;

  base::win::Version version = base::win::GetVersion();
  HMODULE module = ::GetModuleHandleA("kernel32.dll");

  // SHEZ: Remove upstream code here to compile with Win7.1 SDK
  //       (LOAD_LIBRARY_SEARCH_DEFAULT_DIRS is not defined)

  // Set the heap to terminate on corruption
  if (version >= base::win::VERSION_VISTA &&
      (flags & MITIGATION_HEAP_TERMINATE)) {
    if (!::HeapSetInformation(NULL, HeapEnableTerminationOnCorruption,
                              NULL, 0) &&
        ERROR_ACCESS_DENIED != ::GetLastError()) {
      return false;
    }
  }

#if !defined(_WIN64)  // DEP is always enabled on 64-bit.
  if (flags & MITIGATION_DEP) {
    DWORD dep_flags = PROCESS_DEP_ENABLE;
    // DEP support is quirky on XP, so don't force a failure in that case.
    const bool return_on_fail = version >= base::win::VERSION_VISTA;

    if (flags & MITIGATION_DEP_NO_ATL_THUNK)
      dep_flags |= PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION;

    SetProcessDEPPolicyFunction set_process_dep_policy =
        reinterpret_cast<SetProcessDEPPolicyFunction>(
            ::GetProcAddress(module, "SetProcessDEPPolicy"));
    if (set_process_dep_policy) {
      if (!set_process_dep_policy(dep_flags) &&
          ERROR_ACCESS_DENIED != ::GetLastError() && return_on_fail) {
        return false;
      }
    } else {
      // We're on XP sp2, so use the less standard approach.
      // For reference: http://www.uninformed.org/?v=2&a=4
      const int MEM_EXECUTE_OPTION_ENABLE = 1;
      const int MEM_EXECUTE_OPTION_DISABLE = 2;
      const int MEM_EXECUTE_OPTION_ATL7_THUNK_EMULATION = 4;
      const int MEM_EXECUTE_OPTION_PERMANENT = 8;

      NtSetInformationProcessFunction set_information_process = NULL;
      ResolveNTFunctionPtr("NtSetInformationProcess",
                           &set_information_process);
      if (!set_information_process)
        return false;
      ULONG dep = MEM_EXECUTE_OPTION_DISABLE | MEM_EXECUTE_OPTION_PERMANENT;
      if (!(dep_flags & PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION))
        dep |= MEM_EXECUTE_OPTION_ATL7_THUNK_EMULATION;
      if (!SUCCEEDED(set_information_process(GetCurrentProcess(),
                                             ProcessExecuteFlags,
                                             &dep, sizeof(dep))) &&
          ERROR_ACCESS_DENIED != ::GetLastError() && return_on_fail) {
        return false;
      }
    }
  }
#endif

  // This is all we can do in Win7 and below.
  // SHEZ: Remove upstream code here in order to compile using Win7.1 SDK
  return true;
}

void ConvertProcessMitigationsToPolicy(MitigationFlags flags,
                                       DWORD64* policy_flags, size_t* size) {
  base::win::Version version = base::win::GetVersion();

  *policy_flags = 0;
#if defined(_WIN64)
  *size = sizeof(*policy_flags);
#elif defined(_M_IX86)
  // A 64-bit flags attribute is illegal on 32-bit Win 7 and below.
  if (version < base::win::VERSION_WIN8)
    *size = sizeof(DWORD);
  else
    *size = sizeof(*policy_flags);
#else
#error This platform is not supported.
#endif

  // Nothing for Win XP or Vista.
  if (version <= base::win::VERSION_VISTA)
    return;

  // DEP and SEHOP are not valid for 64-bit Windows
#if !defined(_WIN64)
  if (flags & MITIGATION_DEP) {
    *policy_flags |= PROCESS_CREATION_MITIGATION_POLICY_DEP_ENABLE;
    if (!(flags & MITIGATION_DEP_NO_ATL_THUNK))
      *policy_flags |= PROCESS_CREATION_MITIGATION_POLICY_DEP_ATL_THUNK_ENABLE;
  }

  if (flags & MITIGATION_SEHOP)
    *policy_flags |= PROCESS_CREATION_MITIGATION_POLICY_SEHOP_ENABLE;
#endif

  // Win 7
  if (version < base::win::VERSION_WIN8)
    return;

  // SHEZ: Remove upstream code here to compile with Win7.1 SDK
}

MitigationFlags FilterPostStartupProcessMitigations(MitigationFlags flags) {
  // Anything prior to XP SP2.
  if (!IsXPSP2OrLater())
    return 0;

  base::win::Version version = base::win::GetVersion();

  // Windows XP SP2+.
  if (version < base::win::VERSION_VISTA) {
    return flags & (MITIGATION_DEP |
                    MITIGATION_DEP_NO_ATL_THUNK);

  // Windows Vista
  if (version < base::win::VERSION_WIN7) {
    return flags & (MITIGATION_DEP |
                    MITIGATION_DEP_NO_ATL_THUNK |
                    MITIGATION_BOTTOM_UP_ASLR |
                    MITIGATION_DLL_SEARCH_ORDER |
                    MITIGATION_HEAP_TERMINATE);
  }

  // Windows 7 and Vista.
  } else if (version < base::win::VERSION_WIN8) {
    return flags & (MITIGATION_BOTTOM_UP_ASLR |
                    MITIGATION_DLL_SEARCH_ORDER |
                    MITIGATION_HEAP_TERMINATE);
  }

  // Windows 8 and above.
  return flags & (MITIGATION_BOTTOM_UP_ASLR |
                  MITIGATION_DLL_SEARCH_ORDER);
}

bool ApplyProcessMitigationsToSuspendedProcess(HANDLE process,
                                               MitigationFlags flags) {
// This is a hack to fake a weak bottom-up ASLR on 32-bit Windows.
#if !defined(_WIN64)
  if (flags & MITIGATION_BOTTOM_UP_ASLR) {
    unsigned int limit;
    rand_s(&limit);
    char* ptr = 0;
    const size_t kMask64k = 0xFFFF;
    // Random range (512k-16.5mb) in 64k steps.
    const char* end = ptr + ((((limit % 16384) + 512) * 1024) & ~kMask64k);
    while (ptr < end) {
      MEMORY_BASIC_INFORMATION memory_info;
      if (!::VirtualQueryEx(process, ptr, &memory_info, sizeof(memory_info)))
        break;
      size_t size = std::min((memory_info.RegionSize + kMask64k) & ~kMask64k,
                             static_cast<SIZE_T>(end - ptr));
      if (ptr && memory_info.State == MEM_FREE)
        ::VirtualAllocEx(process, ptr, size, MEM_RESERVE, PAGE_NOACCESS);
      ptr += size;
    }
  }
#endif

  return true;
}

bool CanSetProcessMitigationsPostStartup(MitigationFlags flags) {
  // All of these mitigations can be enabled after startup.
  return !(flags & ~(MITIGATION_HEAP_TERMINATE |
                     MITIGATION_DEP |
                     MITIGATION_DEP_NO_ATL_THUNK |
                     MITIGATION_RELOCATE_IMAGE |
                     MITIGATION_RELOCATE_IMAGE_REQUIRED |
                     MITIGATION_BOTTOM_UP_ASLR |
                     MITIGATION_STRICT_HANDLE_CHECKS |
                     MITIGATION_WIN32K_DISABLE |
                     MITIGATION_EXTENSION_DLL_DISABLE |
                     MITIGATION_DLL_SEARCH_ORDER));
}

bool CanSetProcessMitigationsPreStartup(MitigationFlags flags) {
  // These mitigations cannot be enabled prior to startup.
  return !(flags & (MITIGATION_STRICT_HANDLE_CHECKS |
                    MITIGATION_WIN32K_DISABLE |
                    MITIGATION_DLL_SEARCH_ORDER));
}

}  // namespace sandbox

