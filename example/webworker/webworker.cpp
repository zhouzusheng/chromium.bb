// webworker.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "webworker.h"

#include <minikit_products.h>
#include <minikit_renderutils.h>
#include "stdio.h"

namespace minikit {
	const char* Version::version_48_0_2564_103_bb0()
	{
		return "48.0.2564.103_bb0";
	}
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	/*int i = 0;
	if (_tcsstr(lpCmdLine, TEXT("npPdfViewer")) != NULL) {
		while (i == 0) {
			Sleep(10);
		}
	}*/
	
	{
		HMODULE blpwtk2Module = LoadLibraryA(MINIKIT_DLL_NAME);
		if (!blpwtk2Module) return -3456;
		typedef int(*MainFunc)(HINSTANCE hInstance, minikit::RendererCallback*);
		MainFunc mainFunc = (MainFunc)GetProcAddress(blpwtk2Module, "SubProcessMain");
		if (!mainFunc) return -4567;
		return mainFunc(hInstance, NULL);
	}

	return 0;
}


