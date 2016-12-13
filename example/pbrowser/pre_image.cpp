#include "pre_image.h"
#include "windows.h"
#include "tchar.h"
#include "string"

namespace win {

	enum Version {
		VERSION_PRE_XP = 0,  // Not supported.
		VERSION_XP,
		VERSION_SERVER_2003, // Also includes XP Pro x64 and Server 2003 R2.
		VERSION_VISTA,       // Also includes Windows Server 2008.
		VERSION_WIN7,        // Also includes Windows Server 2008 R2.
		VERSION_WIN8,        // Also includes Windows Server 2012.
		VERSION_WIN8_1,      // Also includes Windows Server 2012 R2.
		VERSION_WIN10,       // Also includes Windows 10 Server.
		VERSION_WIN_LAST,    // Indicates error condition.
	};

	// A rough bucketing of the available types of versions of Windows. This is used
	// to distinguish enterprise enabled versions from home versions and potentially
	// server versions.
	enum VersionType {
		SUITE_HOME,
		SUITE_PROFESSIONAL,
		SUITE_SERVER,
		SUITE_LAST,
	};

	typedef BOOL(WINAPI *GetProductInfoPtr)(DWORD, DWORD, DWORD, DWORD, PDWORD);

	class  OSInfo {
	public:
		struct VersionNumber {
			int major;
			int minor;
			int build;
		};

		struct ServicePack {
			int major;
			int minor;
		};

		// The processor architecture this copy of Windows natively uses.  For
		// example, given an x64-capable processor, we have three possibilities:
		//   32-bit Chrome running on 32-bit Windows:           X86_ARCHITECTURE
		//   32-bit Chrome running on 64-bit Windows via WOW64: X64_ARCHITECTURE
		//   64-bit Chrome running on 64-bit Windows:           X64_ARCHITECTURE
		enum WindowsArchitecture {
			X86_ARCHITECTURE,
			X64_ARCHITECTURE,
			IA64_ARCHITECTURE,
			OTHER_ARCHITECTURE,
		};

		// Whether a process is running under WOW64 (the wrapper that allows 32-bit
		// processes to run on 64-bit versions of Windows).  This will return
		// WOW64_DISABLED for both "32-bit Chrome on 32-bit Windows" and "64-bit
		// Chrome on 64-bit Windows".  WOW64_UNKNOWN means "an error occurred", e.g.
		// the process does not have sufficient access rights to determine this.
		enum WOW64Status {
			WOW64_DISABLED,
			WOW64_ENABLED,
			WOW64_UNKNOWN,
		};

		static OSInfo* GetInstance();

		Version version() const { return version_; }
		// The next two functions return arrays of values, [major, minor(, build)].
		VersionNumber version_number() const { return version_number_; }
		VersionType version_type() const { return version_type_; }
		ServicePack service_pack() const { return service_pack_; }
		WindowsArchitecture architecture() const { return architecture_; }
		int processors() const { return processors_; }
		size_t allocation_granularity() const { return allocation_granularity_; }
		WOW64Status wow64_status() const { return wow64_status_; }
		std::string processor_model_name();

		// Like wow64_status(), but for the supplied handle instead of the current
		// process.  This doesn't touch member state, so you can bypass the singleton.
		static WOW64Status GetWOW64StatusForProcess(HANDLE process_handle);

	private:
		OSInfo();
		~OSInfo();

		Version version_;
		VersionNumber version_number_;
		VersionType version_type_;
		ServicePack service_pack_;
		WindowsArchitecture architecture_;
		int processors_;
		size_t allocation_granularity_;
		WOW64Status wow64_status_;

	};

	OSInfo* OSInfo::GetInstance() {
		// Note: we don't use the Singleton class because it depends on AtExitManager,
		// and it's convenient for other modules to use this classs without it. This
		// pattern is copied from gurl.cc.
		static OSInfo* info;
		if (!info) {
			OSInfo* new_info = new OSInfo();
			if (InterlockedCompareExchangePointer(
				reinterpret_cast<PVOID*>(&info), new_info, NULL)) {
				delete new_info;
			}
		}
		return info;
	}

	OSInfo::OSInfo()
		: version_(VERSION_PRE_XP),
		architecture_(OTHER_ARCHITECTURE),
		wow64_status_(GetWOW64StatusForProcess(GetCurrentProcess())) {
		OSVERSIONINFOEX version_info = { sizeof version_info };
		::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&version_info));
		version_number_.major = version_info.dwMajorVersion;
		version_number_.minor = version_info.dwMinorVersion;
		version_number_.build = version_info.dwBuildNumber;
		if ((version_number_.major == 5) && (version_number_.minor > 0)) {
			// Treat XP Pro x64, Home Server, and Server 2003 R2 as Server 2003.
			version_ = (version_number_.minor == 1) ? VERSION_XP : VERSION_SERVER_2003;
		}
		else if (version_number_.major == 6) {
			switch (version_number_.minor) {
			case 0:
				// Treat Windows Server 2008 the same as Windows Vista.
				version_ = VERSION_VISTA;
				break;
			case 1:
				// Treat Windows Server 2008 R2 the same as Windows 7.
				version_ = VERSION_WIN7;
				break;
			case 2:
				// Treat Windows Server 2012 the same as Windows 8.
				version_ = VERSION_WIN8;
				break;
			default:
				version_ = VERSION_WIN8_1;
				break;
			}
		}
		else if (version_number_.major == 10) {
			version_ = VERSION_WIN10;
		}
		else if (version_number_.major > 6) {
			version_ = VERSION_WIN_LAST;
		}
		service_pack_.major = version_info.wServicePackMajor;
		service_pack_.minor = version_info.wServicePackMinor;

		SYSTEM_INFO system_info = {};
		::GetNativeSystemInfo(&system_info);
		switch (system_info.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_INTEL: architecture_ = X86_ARCHITECTURE; break;
		case PROCESSOR_ARCHITECTURE_AMD64: architecture_ = X64_ARCHITECTURE; break;
		case PROCESSOR_ARCHITECTURE_IA64:  architecture_ = IA64_ARCHITECTURE; break;
		}
		processors_ = system_info.dwNumberOfProcessors;
		allocation_granularity_ = system_info.dwAllocationGranularity;

		GetProductInfoPtr get_product_info;
		DWORD os_type;

		if (version_info.dwMajorVersion == 6 || version_info.dwMajorVersion == 10) {
			// Only present on Vista+.
			get_product_info = reinterpret_cast<GetProductInfoPtr>(
				::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "GetProductInfo"));

			get_product_info(version_info.dwMajorVersion, version_info.dwMinorVersion,
				0, 0, &os_type);
			switch (os_type) {
			case PRODUCT_CLUSTER_SERVER:
			case PRODUCT_DATACENTER_SERVER:
			case PRODUCT_DATACENTER_SERVER_CORE:
			case PRODUCT_ENTERPRISE_SERVER:
			case PRODUCT_ENTERPRISE_SERVER_CORE:
			case PRODUCT_ENTERPRISE_SERVER_IA64:
			case PRODUCT_SMALLBUSINESS_SERVER:
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
			case PRODUCT_STANDARD_SERVER:
			case PRODUCT_STANDARD_SERVER_CORE:
			case PRODUCT_WEB_SERVER:
				version_type_ = SUITE_SERVER;
				break;
			case PRODUCT_PROFESSIONAL:
			case PRODUCT_ULTIMATE:
			case PRODUCT_ENTERPRISE:
			case PRODUCT_BUSINESS:
				version_type_ = SUITE_PROFESSIONAL;
				break;
			case PRODUCT_HOME_BASIC:
			case PRODUCT_HOME_PREMIUM:
			case PRODUCT_STARTER:
			default:
				version_type_ = SUITE_HOME;
				break;
			}
		}
		else if (version_info.dwMajorVersion == 5 &&
			version_info.dwMinorVersion == 2) {
			if (version_info.wProductType == VER_NT_WORKSTATION &&
				system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
				version_type_ = SUITE_PROFESSIONAL;
			}
			else if (version_info.wSuiteMask & VER_SUITE_WH_SERVER) {
				version_type_ = SUITE_HOME;
			}
			else {
				version_type_ = SUITE_SERVER;
			}
		}
		else if (version_info.dwMajorVersion == 5 &&
			version_info.dwMinorVersion == 1) {
			if (version_info.wSuiteMask & VER_SUITE_PERSONAL)
				version_type_ = SUITE_HOME;
			else
				version_type_ = SUITE_PROFESSIONAL;
		}
		else {
			// Windows is pre XP so we don't care but pick a safe default.
			version_type_ = SUITE_HOME;
		}
	}

	OSInfo::~OSInfo() {
	}

	
	// static
	OSInfo::WOW64Status OSInfo::GetWOW64StatusForProcess(HANDLE process_handle) {
		typedef BOOL(WINAPI* IsWow64ProcessFunc)(HANDLE, PBOOL);
		IsWow64ProcessFunc is_wow64_process = reinterpret_cast<IsWow64ProcessFunc>(
			GetProcAddress(GetModuleHandle(L"kernel32.dll"), "IsWow64Process"));
		if (!is_wow64_process)
			return WOW64_DISABLED;
		BOOL is_wow64 = FALSE;
		if (!(*is_wow64_process)(process_handle, &is_wow64))
			return WOW64_UNKNOWN;
		return is_wow64 ? WOW64_ENABLED : WOW64_DISABLED;
	}

	Version GetVersion() {
		return OSInfo::GetInstance()->version();
	}
}

bool ImagePreReader::PreReadImage(const wchar_t* file_path)
{
	if (win::GetVersion() > win::VERSION_XP) {

		HANDLE hFile = CreateFile(file_path,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			const DWORD actual_step_size = 1024 * 1024 * 4;
			DWORD size_to_read = GetFileSize(hFile, NULL);

			LPVOID buffer = ::VirtualAlloc(NULL,
				actual_step_size,
				MEM_COMMIT,
				PAGE_READWRITE);

			if (buffer != NULL) {

				DWORD len;
				size_t total_read = 0;
				while (::ReadFile(hFile, buffer, actual_step_size, &len, NULL) &&
					len > 0 &&
					(size_to_read ? total_read < size_to_read : true)) {
					total_read += static_cast<size_t>(len);
				}
				::VirtualFree(buffer, 0, MEM_RELEASE);
			}

			CloseHandle(hFile);
		}
		return true;
	}

	//TODO: XP 暂时不支持

	/*HMODULE dll_module = ::LoadLibraryExW(
		file_path,
		NULL,
		LOAD_WITH_ALTERED_SEARCH_PATH | DONT_RESOLVE_DLL_REFERENCES);

	if (!dll_module)
		return false;
	*/
	return false;
}
