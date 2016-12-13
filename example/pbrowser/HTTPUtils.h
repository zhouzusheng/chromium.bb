#pragma once

namespace HTTPUtils {
	unsigned long  getProcessIdByPort(int port);
	BOOL		   getProcessCmdLine(DWORD dwId,LPWSTR wBuf,DWORD dwBufLen);
}


