// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sandbox/win/src/target_process.h"

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/win/pe_image.h"
#include "base/win/startup_information.h"
#include "base/win/windows_version.h"
#include "sandbox/win/src/crosscall_server.h"
#include "sandbox/win/src/crosscall_client.h"
#include "sandbox/win/src/policy_low_level.h"
#include "sandbox/win/src/sandbox_types.h"
#include "sandbox/win/src/sharedmem_ipc_server.h"

namespace {

void CopyPolicyToTarget(const void* source, size_t size, void* dest) {
  if (!source || !size)
    return;
  memcpy(dest, source, size);
  sandbox::PolicyGlobal* policy =
      reinterpret_cast<sandbox::PolicyGlobal*>(dest);

  size_t offset = reinterpret_cast<size_t>(source);

  for (size_t i = 0; i < sandbox::kMaxServiceCount; i++) {
    size_t buffer = reinterpret_cast<size_t>(policy->entry[i]);
    if (buffer) {
      buffer -= offset;
      policy->entry[i] = reinterpret_cast<sandbox::PolicyBuffer*>(buffer);
    }
  }
}

}

namespace sandbox {

SANDBOX_INTERCEPT HANDLE g_shared_section;
SANDBOX_INTERCEPT size_t g_shared_IPC_size;
SANDBOX_INTERCEPT size_t g_shared_policy_size;

// Returns the address of the main exe module in memory taking in account
// address space layout randomization.
void* GetBaseAddress(const wchar_t* exe_name, void* entry_point) {
  HMODULE exe = ::LoadLibrary(exe_name);
  if (NULL == exe)
    return exe;

  base::win::PEImage pe(exe);
  if (!pe.VerifyMagic()) {
    ::FreeLibrary(exe);
    return exe;
  }
  PIMAGE_NT_HEADERS nt_header = pe.GetNTHeaders();
  char* base = reinterpret_cast<char*>(entry_point) -
    nt_header->OptionalHeader.AddressOfEntryPoint;

  ::FreeLibrary(exe);
  return base;
}


TargetProcess::TargetProcess(HANDLE initial_token, HANDLE lockdown_token,
                             HANDLE job, ThreadProvider* thread_pool)
  // This object owns everything initialized here except thread_pool and
  // the job_ handle. The Job handle is closed by BrokerServices and results
  // eventually in a call to our dtor.
    : lockdown_token_(lockdown_token),
      initial_token_(initial_token),
      job_(job),
      thread_pool_(thread_pool),
#if SANDBOX_DLL
      module_base_address_(NULL),
#endif
      base_address_(NULL) {
}

TargetProcess::~TargetProcess() {
  DWORD exit_code = 0;
  // Give a chance to the process to die. In most cases the JOB_KILL_ON_CLOSE
  // will take effect only when the context changes. As far as the testing went,
  // this wait was enough to switch context and kill the processes in the job.
  // If this process is already dead, the function will return without waiting.
  // TODO(nsylvain):  If the process is still alive at the end, we should kill
  // it. http://b/893891
  // For now, this wait is there only to do a best effort to prevent some leaks
  // from showing up in purify.
  if (sandbox_process_info_.IsValid()) {
    ::WaitForSingleObject(sandbox_process_info_.process_handle(), 50);
    if (!::GetExitCodeProcess(sandbox_process_info_.process_handle(),
                              &exit_code) || (STILL_ACTIVE == exit_code)) {
      // It is an error to destroy this object while the target process is still
      // alive because we need to destroy the IPC subsystem and cannot risk to
      // have an IPC reach us after this point.
      if (shared_section_.IsValid())
        shared_section_.Take();
      SharedMemIPCServer* server = ipc_server_.release();
      sandbox_process_info_.TakeProcessHandle();
      return;
    }
  }

  // ipc_server_ references our process handle, so make sure the former is shut
  // down before the latter is closed (by ScopedProcessInformation).
  ipc_server_.reset();
}

// Creates the target (child) process suspended and assigns it to the job
// object.
DWORD TargetProcess::Create(const wchar_t* exe_path,
                            const wchar_t* command_line,
                            bool inherit_handles,
                            const base::win::StartupInformation& startup_info,
                            base::win::ScopedProcessInformation* target_info) {
  exe_name_.reset(_wcsdup(exe_path));

  // the command line needs to be writable by CreateProcess().
  scoped_ptr_malloc<wchar_t> cmd_line(_wcsdup(command_line));

  // Start the target process suspended.
  DWORD flags =
      CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT | DETACHED_PROCESS;

  if (startup_info.has_extended_startup_info())
    flags |= EXTENDED_STARTUPINFO_PRESENT;

  if (job_ && base::win::GetVersion() < base::win::VERSION_WIN8) {
    // Windows 8 implements nested jobs, but for older systems we need to
    // break out of any job we're in to enforce our restrictions.
    flags |= CREATE_BREAKAWAY_FROM_JOB;
  }

  base::win::ScopedProcessInformation process_info;

  if (!::CreateProcessAsUserW(lockdown_token_,
                              exe_path,
                              cmd_line.get(),
                              NULL,   // No security attribute.
                              NULL,   // No thread attribute.
                              inherit_handles,
                              flags,
                              NULL,   // Use the environment of the caller.
                              NULL,   // Use current directory of the caller.
                              startup_info.startup_info(),
                              process_info.Receive())) {
    return ::GetLastError();
  }
  lockdown_token_.Close();

  DWORD win_result = ERROR_SUCCESS;

  if (job_) {
    // Assign the suspended target to the windows job object.
    if (!::AssignProcessToJobObject(job_, process_info.process_handle())) {
      win_result = ::GetLastError();
      ::TerminateProcess(process_info.process_handle(), 0);
      return win_result;
    }
  }

  if (initial_token_.IsValid()) {
    // Change the token of the main thread of the new process for the
    // impersonation token with more rights. This allows the target to start;
    // otherwise it will crash too early for us to help.
    HANDLE temp_thread = process_info.thread_handle();
    if (!::SetThreadToken(&temp_thread, initial_token_)) {
      win_result = ::GetLastError();
      // It might be a security breach if we let the target run outside the job
      // so kill it before it causes damage.
      ::TerminateProcess(process_info.process_handle(), 0);
      return win_result;
    }

    // If sandbox is implemented as a DLL, impersonation token cannot be closed
    // here as the impersonation token should be used to inject dll in the
    // target process. DLL is injected by creating a remote thread and this
    // thread handle requires higher privileges to load DLL. The broker will
    // explicitly close the initial token after the dll is injected.
#if !SANDBOX_DLL
    initial_token_.Close();
#endif
  }

  CONTEXT context;
  context.ContextFlags = CONTEXT_ALL;
  if (!::GetThreadContext(process_info.thread_handle(), &context)) {
    win_result = ::GetLastError();
    ::TerminateProcess(process_info.process_handle(), 0);
    return win_result;
  }

#if defined(_WIN64)
  void* entry_point = reinterpret_cast<void*>(context.Rcx);
#else
#pragma warning(push)
#pragma warning(disable: 4312)
  // This cast generates a warning because it is 32 bit specific.
  void* entry_point = reinterpret_cast<void*>(context.Eax);
#pragma warning(pop)
#endif  // _WIN64

  if (!target_info->DuplicateFrom(process_info)) {
    win_result = ::GetLastError();  // This may or may not be correct.
    ::TerminateProcess(process_info.process_handle(), 0);
    return win_result;
  }

  base_address_ = GetBaseAddress(exe_path, entry_point);
  sandbox_process_info_.Set(process_info.Take());
  return win_result;
}

#if SANDBOX_DLL
// This function injects sandbox DLL to target process. The function runs
// LoadLibrary function in a remote thread with DLL path as parameter.
// The thread should be created in suspended mode and the impersonation token
// should be assigned as loading library required additional privileges.
DWORD TargetProcess::InjectSandboxDll(const wchar_t* module_path)
{
  module_path_.reset(_wcsdup(module_path));

  DWORD win_result = 0;
  LPVOID load_library_addr =
      (LPVOID)::GetProcAddress(GetModuleHandle(L"kernel32.dll"),
                               "LoadLibraryW");
  if (NULL == load_library_addr) {
    return ::GetLastError();
  }

  int length  = (wcslen(module_path_.get()) + 1)* sizeof(wchar_t);
  LPVOID param =
      (LPVOID)::VirtualAllocEx(sandbox_process_info_.process_handle(),
                               NULL,
                               length,
                               MEM_RESERVE | MEM_COMMIT,
                               PAGE_READWRITE);
  if (NULL == param){
    return ::GetLastError();
  }

  if (!::WriteProcessMemory(sandbox_process_info_.process_handle(),
                            param, module_path_.get(), length, NULL)) {
    win_result = ::GetLastError();
    ::VirtualFreeEx(sandbox_process_info_.process_handle(),
                    param, length, MEM_RELEASE);
    return win_result;
  }

  HANDLE remote_thread_handle = ::CreateRemoteThread(
                                    sandbox_process_info_.process_handle(),
                                    NULL,
                                    NULL,
                                    (LPTHREAD_START_ROUTINE)load_library_addr,
                                    param,
                                    CREATE_SUSPENDED,
                                    NULL);

  if (NULL == remote_thread_handle) {
    win_result = ::GetLastError();
    ::VirtualFreeEx(sandbox_process_info_.process_handle(),
                    param, length, MEM_RELEASE);
    return win_result;
  }

  if (!::SetThreadToken(&remote_thread_handle, initial_token_)) {
    win_result = ::GetLastError();
    ::CloseHandle(remote_thread_handle);
    ::VirtualFreeEx(sandbox_process_info_.process_handle(),
                    param, length, MEM_RELEASE);
    return win_result;
  }

  if (!::ResumeThread(remote_thread_handle)) {
    win_result = ::GetLastError();
    ::CloseHandle(remote_thread_handle);
    ::VirtualFreeEx(sandbox_process_info_.process_handle(),
                    param, length, MEM_RELEASE);
    return win_result;
  }

  if (WAIT_OBJECT_0 != ::WaitForSingleObject(remote_thread_handle, 1000)) {
    CloseHandle(remote_thread_handle);
    ::VirtualFreeEx(sandbox_process_info_.process_handle(),
                    param, length, MEM_RELEASE);
    return SBOX_ERROR_GENERIC;
  }

  ::VirtualFreeEx(sandbox_process_info_.process_handle(),
                  param, length, MEM_RELEASE);
  if (!::GetExitCodeThread(remote_thread_handle,
                           (LPDWORD)&module_base_address_)) {
    win_result = ::GetLastError();
    ::CloseHandle(remote_thread_handle);
    return win_result;
  }
  ::CloseHandle(remote_thread_handle);
  return ERROR_SUCCESS;
}

void TargetProcess::CloseInitialToken()
{
  initial_token_.Close();
}
#endif

ResultCode TargetProcess::TransferVariable(const char* name, void* address,
                                           size_t size) {
  if (!sandbox_process_info_.IsValid())
    return SBOX_ERROR_UNEXPECTED_CALL;

  void* child_var = address;

#if SANDBOX_EXPORTS
  HMODULE module = ::LoadLibrary(SandboxModuleName());
  if (NULL == module)
    return SBOX_ERROR_GENERIC;

  child_var = ::GetProcAddress(module, name);
  ::FreeLibrary(module);

  if (NULL == child_var)
    return SBOX_ERROR_GENERIC;

  size_t offset = reinterpret_cast<char*>(child_var) -
                  reinterpret_cast<char*>(module);
  child_var = reinterpret_cast<char*>(SandboxModule()) + offset;
#else
  UNREFERENCED_PARAMETER(name);
#endif

  SIZE_T written;
  if (!::WriteProcessMemory(sandbox_process_info_.process_handle(),
                            child_var, address, size, &written))
    return SBOX_ERROR_GENERIC;

  if (written != size)
    return SBOX_ERROR_GENERIC;

  return SBOX_ALL_OK;
}

// Construct the IPC server and the IPC dispatcher. When the target does
// an IPC it will eventually call the dispatcher.
DWORD TargetProcess::Init(Dispatcher* ipc_dispatcher, void* policy,
                          uint32 shared_IPC_size, uint32 shared_policy_size) {
  // We need to map the shared memory on the target. This is necessary for
  // any IPC that needs to take place, even if the target has not yet hit
  // the main( ) function or even has initialized the CRT. So here we set
  // the handle to the shared section. The target on the first IPC must do
  // the rest, which boils down to calling MapViewofFile()

  // We use this single memory pool for IPC and for policy.
  DWORD shared_mem_size = static_cast<DWORD>(shared_IPC_size +
                                             shared_policy_size);
  shared_section_.Set(::CreateFileMappingW(INVALID_HANDLE_VALUE, NULL,
                                           PAGE_READWRITE | SEC_COMMIT,
                                           0, shared_mem_size, NULL));
  if (!shared_section_.IsValid()) {
    return ::GetLastError();
  }

  DWORD access = FILE_MAP_READ | FILE_MAP_WRITE;
  HANDLE target_shared_section;
  if (!::DuplicateHandle(::GetCurrentProcess(), shared_section_,
                         sandbox_process_info_.process_handle(),
                         &target_shared_section, access, FALSE, 0)) {
    return ::GetLastError();
  }

  void* shared_memory = ::MapViewOfFile(shared_section_,
                                        FILE_MAP_WRITE|FILE_MAP_READ,
                                        0, 0, 0);
  if (NULL == shared_memory) {
    return ::GetLastError();
  }

  CopyPolicyToTarget(policy, shared_policy_size,
                     reinterpret_cast<char*>(shared_memory) + shared_IPC_size);

  ResultCode ret;
  // Set the global variables in the target. These are not used on the broker.
  g_shared_section = target_shared_section;
  ret = TransferVariable("g_shared_section", &g_shared_section,
                         sizeof(g_shared_section));
  g_shared_section = NULL;
  if (SBOX_ALL_OK != ret) {
    return (SBOX_ERROR_GENERIC == ret)?
           ::GetLastError() : ERROR_INVALID_FUNCTION;
  }
  g_shared_IPC_size = shared_IPC_size;
  ret = TransferVariable("g_shared_IPC_size", &g_shared_IPC_size,
                         sizeof(g_shared_IPC_size));
  g_shared_IPC_size = 0;
  if (SBOX_ALL_OK != ret) {
    return (SBOX_ERROR_GENERIC == ret) ?
           ::GetLastError() : ERROR_INVALID_FUNCTION;
  }
  g_shared_policy_size = shared_policy_size;
  ret = TransferVariable("g_shared_policy_size", &g_shared_policy_size,
                         sizeof(g_shared_policy_size));
  g_shared_policy_size = 0;
  if (SBOX_ALL_OK != ret) {
    return (SBOX_ERROR_GENERIC == ret) ?
           ::GetLastError() : ERROR_INVALID_FUNCTION;
  }

  ipc_server_.reset(
      new SharedMemIPCServer(sandbox_process_info_.process_handle(),
                             sandbox_process_info_.process_id(),
                             job_, thread_pool_, ipc_dispatcher));

  if (!ipc_server_->Init(shared_memory, shared_IPC_size, kIPCChannelSize))
    return ERROR_NOT_ENOUGH_MEMORY;

  // After this point we cannot use this handle anymore.
  ::CloseHandle(sandbox_process_info_.TakeThreadHandle());

  return ERROR_SUCCESS;
}

void TargetProcess::Terminate() {
  if (!sandbox_process_info_.IsValid())
    return;

  ::TerminateProcess(sandbox_process_info_.process_handle(), 0);
}

TargetProcess* MakeTestTargetProcess(HANDLE process, HMODULE base_address) {
  TargetProcess* target = new TargetProcess(NULL, NULL, NULL, NULL);
  PROCESS_INFORMATION process_info = {};
  process_info.hProcess = process;
  target->sandbox_process_info_.Set(process_info);
  target->base_address_ = base_address;
  return target;
}

}  // namespace sandbox
