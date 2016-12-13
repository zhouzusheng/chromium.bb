#include "HTTPUtils.h"
#include <Winsock2.h>
#include <MSWSock.h>

#define ANY_SIZE 1

namespace HTTPUtils {

	typedef struct
	{
		DWORD dwState;      //连接状态  
		DWORD dwLocalAddr;  //本地地址  
		DWORD dwLocalPort;  //本地端口  
		DWORD dwRemoteAddr; //远程地址  
		DWORD dwRemotePort; //远程端口  
		DWORD dwProcessId;  //进程标识  

	}MIB_TCPEXROW, *PMIB_TCPEXROW;


	typedef struct
	{
		DWORD dwLocalAddr;  //本地地址  
		DWORD dwLocalPort;  //本地端口  
		DWORD dwProcessId;  //进程标识  

	}MIB_UDPEXROW, *PMIB_UDPEXROW;


	typedef struct
	{
		DWORD dwState;      //连接状态  
		DWORD dwLocalAddr;  //本地地址  
		DWORD dwLocalPort;  //本地端口  
		DWORD dwRemoteAddr; //远程地址  
		DWORD dwRemotePort; //远程端口  
		DWORD dwProcessId;  //进程标识  
		DWORD Unknown;      //待定标识  

	}MIB_TCPEXROW_VISTA, *PMIB_TCPEXROW_VISTA;


	typedef struct
	{
		DWORD dwNumEntries;
		MIB_TCPEXROW table[ANY_SIZE];

	}MIB_TCPEXTABLE, *PMIB_TCPEXTABLE;


	typedef struct
	{
		DWORD dwNumEntries;
		MIB_TCPEXROW_VISTA table[ANY_SIZE];

	}MIB_TCPEXTABLE_VISTA, *PMIB_TCPEXTABLE_VISTA;


	typedef struct
	{
		DWORD dwNumEntries;
		MIB_UDPEXROW table[ANY_SIZE];

	}MIB_UDPEXTABLE, *PMIB_UDPEXTABLE;

	///QueryProcessInformation
#define ProcessBasicInformation 0
	typedef struct
	{
		USHORT Length;
		USHORT MaximumLength;
		PWSTR  Buffer;
	} UNICODE_STRING, *PUNICODE_STRING;

	typedef struct
	{
		ULONG          AllocationSize;
		ULONG          ActualSize;
		ULONG          Flags;
		ULONG          Unknown1;
		UNICODE_STRING Unknown2;
		HANDLE         InputHandle;
		HANDLE         OutputHandle;
		HANDLE         ErrorHandle;
		UNICODE_STRING CurrentDirectory;
		HANDLE         CurrentDirectoryHandle;
		UNICODE_STRING SearchPaths;
		UNICODE_STRING ApplicationName;
		UNICODE_STRING CommandLine;
		PVOID          EnvironmentBlock;
		ULONG          Unknown[9];
		UNICODE_STRING Unknown3;
		UNICODE_STRING Unknown4;
		UNICODE_STRING Unknown5;
		UNICODE_STRING Unknown6;
	} PROCESS_PARAMETERS, *PPROCESS_PARAMETERS;

	typedef struct
	{
		ULONG               AllocationSize;
		ULONG               Unknown1;
		HINSTANCE           ProcessHinstance;
		PVOID               ListDlls;
		PPROCESS_PARAMETERS ProcessParameters;
		ULONG               Unknown2;
		HANDLE              Heap;
	} PEB, *PPEB;

	typedef struct
	{
		DWORD ExitStatus;
		PPEB  PebBaseAddress;
		DWORD AffinityMask;
		DWORD BasePriority;
		ULONG UniqueProcessId;
		ULONG InheritedFromUniqueProcessId;
	}   PROCESS_BASIC_INFORMATION;

	typedef LONG (WINAPI *PROCNTQSIP)(HANDLE,UINT,PVOID,ULONG,PULONG);
	static PROCNTQSIP NtQueryInformationProcess;

	unsigned long  getProcessIdByPort(int port)
	{
		typedef DWORD(WINAPI *PFNAllocateAndGetTcpExTableFromStack)(
			PMIB_TCPEXTABLE *pTcpTabel,
			bool bOrder,
			HANDLE heap,
			DWORD zero,
			DWORD flags
			);
		typedef DWORD(WINAPI *PFNInternalGetTcpTable2)(
			PMIB_TCPEXTABLE_VISTA *pTcpTable_Vista,
			HANDLE heap,
			DWORD flags
			);

		HMODULE hModule = LoadLibraryW(L"iphlpapi.dll");
		if (hModule == NULL)
			return 0;

		DWORD dwPort = (DWORD)port;

		PFNAllocateAndGetTcpExTableFromStack pAllocateAndGetTcpExTableFromStack;
		pAllocateAndGetTcpExTableFromStack =
			(PFNAllocateAndGetTcpExTableFromStack)GetProcAddress(hModule, "AllocateAndGetTcpExTableFromStack");
		if (pAllocateAndGetTcpExTableFromStack)
		{
			PMIB_TCPEXTABLE pTcpExTable = NULL;
			if (pAllocateAndGetTcpExTableFromStack(&pTcpExTable, TRUE, GetProcessHeap(), 0, AF_INET) != 0)
			{
				if (pTcpExTable)
				{
					HeapFree(GetProcessHeap(), 0, pTcpExTable);
				}

				FreeLibrary(hModule);
				hModule = NULL;

				return 0;
			}

			for (UINT i = 0; i < pTcpExTable->dwNumEntries; i++)
			{
				// 过滤掉数据，只查询我们需要的进程数据  
				if (dwPort == ntohs(0x0000FFFF & pTcpExTable->table[i].dwLocalPort))
				{
					DWORD dwProcessId = pTcpExTable->table[i].dwProcessId;
					if (pTcpExTable)
					{
						HeapFree(GetProcessHeap(), 0, pTcpExTable);
					}

					FreeLibrary(hModule);
					hModule = NULL;

					return dwProcessId;
				}
			}

			if (pTcpExTable)
			{
				HeapFree(GetProcessHeap(), 0, pTcpExTable);
			}

			FreeLibrary(hModule);
			hModule = NULL;

			return 0;
		}
		else
		{
			// 表明为 Vista 或者 7 操作系统  
			PMIB_TCPEXTABLE_VISTA pTcpExTable = NULL;
			PFNInternalGetTcpTable2 pInternalGetTcpTable2 =
				(PFNInternalGetTcpTable2)GetProcAddress(hModule, "InternalGetTcpTable2");
			if (pInternalGetTcpTable2 == NULL)
			{
				if (pTcpExTable)
				{
					HeapFree(GetProcessHeap(), 0, pTcpExTable);
				}

				FreeLibrary(hModule);
				hModule = NULL;

				return 0;
			}

			if (pInternalGetTcpTable2(&pTcpExTable, GetProcessHeap(), 1))
			{
				if (pTcpExTable)
				{
					HeapFree(GetProcessHeap(), 0, pTcpExTable);
				}

				FreeLibrary(hModule);
				hModule = NULL;

				return 0;
			}

			for (UINT i = 0; i < pTcpExTable->dwNumEntries; i++)
			{
				// 过滤掉数据，只查询我们需要的进程数据  
				if (dwPort == ntohs(0x0000FFFF & pTcpExTable->table[i].dwLocalPort))
				{
					DWORD dwProcessId = pTcpExTable->table[i].dwProcessId;
					if (pTcpExTable)
					{
						HeapFree(GetProcessHeap(), 0, pTcpExTable);
					}

					FreeLibrary(hModule);
					hModule = NULL;

					return dwProcessId;
				}
			}

			if (pTcpExTable)
			{
				HeapFree(GetProcessHeap(), 0, pTcpExTable);
			}

			FreeLibrary(hModule);
			hModule = NULL;

			return 0;
		}
		FreeLibrary(hModule);
		return 0;
	}

	BOOL	getProcessCmdLine(DWORD dwId,LPWSTR wBuf,DWORD dwBufLen)
	{
		if(NtQueryInformationProcess == NULL)
		{
			NtQueryInformationProcess = (PROCNTQSIP)GetProcAddress(
				GetModuleHandle(TEXT("ntdll")),
				"NtQueryInformationProcess"
				);

			if (!NtQueryInformationProcess)
				return FALSE;
		}
		LONG                      status;
		HANDLE                    hProcess;
		PROCESS_BASIC_INFORMATION pbi;
		PEB                       Peb;
		PROCESS_PARAMETERS        ProcParam;
		DWORD                     dwDummy;
		DWORD                     dwSize;
		LPVOID                    lpAddress;
		BOOL                      bRet = FALSE;

		// Get process handle
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,dwId);
		if (!hProcess)
			return FALSE;

		// Retrieve information
		status = NtQueryInformationProcess( hProcess,
			ProcessBasicInformation,
			(PVOID)&pbi,
			sizeof(PROCESS_BASIC_INFORMATION),
			NULL
			);

		if (status)
			goto cleanup;

		if (!ReadProcessMemory( hProcess,
			pbi.PebBaseAddress,
			&Peb,
			sizeof(PEB),
			&dwDummy
			)
			)
			goto cleanup;

		if (!ReadProcessMemory( hProcess,
			Peb.ProcessParameters,
			&ProcParam,
			sizeof(PROCESS_PARAMETERS),
			&dwDummy
			)
			)
			goto cleanup;

		lpAddress = ProcParam.CommandLine.Buffer;
		dwSize = ProcParam.CommandLine.Length;

		if (dwBufLen<dwSize)
			goto cleanup;

		if (!ReadProcessMemory( hProcess,
			lpAddress,
			wBuf,
			dwSize,
			&dwDummy
			)
			)
			goto cleanup;

		bRet = TRUE;
cleanup:
		CloseHandle (hProcess);
		return bRet;
	}
}