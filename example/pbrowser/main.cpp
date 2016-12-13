#include "windows.h"
#include "tchar.h"
#include "minikit.h"
#include "pre_image.h"

#include "min.h"
#include "BrowerFrame.h"
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "minikit.dll.lib")

#include "BrowserHost.h"

static ULONG_PTR gdiplusToken = 0;

minikit::Toolkit* g_toolkit = NULL;
minikit::Profile* g_profile = NULL;
minikit::HttpServer* g_server = NULL;

class BrowserHostImpl : public pb::BroswserHost {
public:
	virtual minikit::Toolkit* GetMinikitToolkit() {
		return g_toolkit;
	}
	virtual minikit::Profile* GetMinikitProfile() {
		return g_profile;
	}
	virtual minikit::HttpServer* GetMinikiHTTPServer() {
		return g_server;
	}
} g_browserHost;

pb::BroswserApp* g_browserapp = NULL;

void logMessageHandler(minikit::ToolkitCreateParams::LogMessageSeverity severity,
	const char* file,
	int line,
	const char* message)
{
	
}

void consoleLogMessageHandler(minikit::ToolkitCreateParams::LogMessageSeverity severity,
	const minikit::StringRef& file,
	unsigned line,
	unsigned column,
	const minikit::StringRef& message,
	const minikit::StringRef& stack_trace)
{
	
}

void startApp(){
	ImagePreReader::PreReadImage(L"pbrowserapp.dll");
	HINSTANCE h = LoadLibrary(TEXT("pbrowserapp.dll"));
	if (h) {
		pb::startapp proc = (pb::startapp)GetProcAddress(h, "startapp");
		if (proc) {
			g_browserapp = proc(&g_browserHost);
		}
	}
}

void stopApp() {
	HINSTANCE h = GetModuleHandle(TEXT("pbrowserapp.dll"));
	if (h) {
		pb::stoptapp proc = (pb::stoptapp)GetProcAddress(h, "stopapp");
		if (proc && g_browserapp) {
			proc(g_browserapp);
			g_browserapp = NULL;
		}
		FreeLibrary(h);
	}
}

void Show(LPTSTR lpCmdLine, int nCmdShow)
{
	BrowerFrame* browserFrame = new BrowerFrame(_T("BrowserWnd.xml"));

	browserFrame->Create(NULL, _T("海量情报服务平台 3.2.4"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	browserFrame->ShowWindow();

	startApp();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (!g_toolkit->preHandleMessage(&msg)) {
			if (!CPaintManagerUI::TranslateMessage(&msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		g_toolkit->postHandleMessage(&msg);
	}
	stopApp();
}

class HTTPServerDelegate : public minikit::HttpServerDelegate {
public:
	virtual void OnHttpRequest(int connection_id, minikit::HttRequestInfo* info)
	{
		minikit::HttResponseInfo* res = g_server->makeResponse(404);
		//res->AddHeader("Content-Length", "9");
		res->SetData("Not Found", "text/plain");
		g_server->sendResponse(connection_id, res);
	}

	virtual void OnWebSocketRequest(int connection_id, minikit::HttRequestInfo* info)
	{

	}
	virtual void OnWebSocketMessage(int connection_id, const char* data, int len)
	{

	}
	virtual void OnClose(int connection_id)
	{

	}
};
void Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);

	minikit::ToolkitCreateParams toolkitParams;
	toolkitParams.disableInProcessRenderer();
	toolkitParams.appendCommandLineSwitch("--browser-subprocess-path=webworker.exe");
	toolkitParams.appendCommandLineSwitch("--enable-npapi");
	toolkitParams.setLogMessageHandler(logMessageHandler);
	toolkitParams.setConsoleLogMessageHandler(consoleLogMessageHandler);

	g_toolkit = minikit::ToolkitFactory::create(toolkitParams);
	minikit::ProfileCreateParams profileParams("");
	g_profile = g_toolkit->createProfile(profileParams);

	minikit::ProxyConfig config;
	g_profile->setProxyConfig(config);

	HTTPServerDelegate deletegate;
	g_server = g_toolkit->createHTTPServer(&deletegate);
	g_server->start(10995, 4);

	Show(lpCmdLine, nCmdShow);

	if (g_server)
	{
		g_server->shutdown();
		g_server->destroy();
		g_server = NULL;
	}
	if (g_toolkit)
	{
		g_profile->destroy();
		g_toolkit = 0;
		Sleep(200);
	}

}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	ImagePreReader::PreReadImage(L"minikit.dll");
	ImagePreReader::PreReadImage(L"miniv8.dll");

	::OleInitialize(NULL);
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	
	Gdiplus::GdiplusShutdown(gdiplusToken);
	::OleUninitialize();

	return 0;
}