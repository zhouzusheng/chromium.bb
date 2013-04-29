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

// This pragma makes us use the version 6.0 of ComCtl32.dll, which is necessary
// to make tooltips appear correctly.  See:
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb773175%28v=vs.85%29.aspx
#pragma comment(linker,"\"/manifestdependency:type='win32' \
 name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
 processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <algorithm>
#include <fstream>
#include <string>
#include <set>
#include <vector>

#include <assert.h>

#include <blpwtk2.h>
#include <v8.h>

HINSTANCE g_instance = 0;
WNDPROC g_defaultEditWndProc = 0;

#define OVERRIDE override
#define BUTTON_WIDTH 72
#define URLBAR_HEIGHT  24
#define AUTO_PUMP 1

// button ids
enum {
    IDC_START_OF_BUTTONS = 1000,
    IDC_RELOAD,
    IDC_END_OF_BUTTONS,
    NUM_BUTTONS = IDC_END_OF_BUTTONS - IDC_START_OF_BUTTONS - 1
};

// menu ids
enum {
    IDM_START_OF_MENU_ITEMS = 2000,
    IDM_FILE,
    IDM_NEW_WINDOW,
    IDM_CLOSE_WINDOW,
    IDM_EXIT,
    IDM_TEST,
    IDM_TEST_APPEND_ELEMENT,
    IDM_TEST_APPEND_TABLE,
    IDM_TEST_V8_APPEND_ELEMENT,
    IDM_CUT,
    IDM_COPY,
    IDM_PASTE,
    IDM_DELETE,
    IDM_INSPECT,
    IDM_END_OF_MENU_ITEMS,
    NUM_MENU_ITEMS = IDM_END_OF_MENU_ITEMS - IDM_START_OF_MENU_ITEMS - 1
};

class Shell;
int registerShellWindowClass();
Shell* createShell(blpwtk2::WebView* webView = 0);
HMENU createContextMenu(const blpwtk2::ContextMenuParams& params);
blpwtk2::HttpTransactionHandler* createHttpTransactionHandler();

void appendElement(blpwtk2::WebView* webView)
{
    blpwtk2::WebFrame* mainFrame = webView->mainFrame();
    blpwtk2::WebDocument document = mainFrame->document();
    blpwtk2::WebElement div = document.createElement("DIV");
    div.setTextContent("Hello From Shell!!");
    document.body().appendChild(div);
}

void appendTable(blpwtk2::WebView* webView)
{
    blpwtk2::WebFrame* mainFrame = webView->mainFrame();
    blpwtk2::WebDocument document = mainFrame->document();
    blpwtk2::WebElement table = document.createElement("TABLE");
    table.setAttribute("border", "1");
    blpwtk2::WebElement tr = document.createElement("TR");
    table.appendChild(tr);
    blpwtk2::WebElement td = document.createElement("TD");
    td.setTextContent("Hello From Shell!!");
    tr.appendChild(td);
    document.body().appendChild(table);
}

void v8AppendElement(blpwtk2::WebView* webView)
{
    blpwtk2::WebFrame* mainFrame = webView->mainFrame();
    v8::HandleScope handleScope;
    v8::Local<v8::Context> context = mainFrame->mainWorldScriptContext();
    static const char SCRIPT[] =
        "var div = document.createElement('div');\n"
        "div.textContent = 'Hello From Shell Using V8!!!';\n"
        "document.body.appendChild(div);\n";

    v8::Context::Scope contextScope(context);
    v8::Local<v8::Script> script = v8::Script::New(v8::String::New(SCRIPT));
    assert(!script.IsEmpty());  // this should never fail to compile

    v8::TryCatch tryCatch;
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
        v8::String::AsciiValue msg(tryCatch.Exception());
        char buf[1024];
        sprintf_s(buf, sizeof(buf), "EXCEPTION: %s\n", *msg);
        OutputDebugStringA(buf);
    }
}

class Shell : public blpwtk2::WebViewDelegate {
public:
    static std::set<Shell*> s_shells;

    HWND d_mainWnd;
    HWND d_urlEntryWnd;
    blpwtk2::WebView* d_webView;
    Shell* d_inspectorShell;
    Shell* d_inspectorFor;

    Shell(HWND mainWnd, HWND urlEntryWnd, blpwtk2::WebView* webView = 0)
    : d_mainWnd(mainWnd)
    , d_urlEntryWnd(urlEntryWnd)
    , d_webView(webView)
    , d_inspectorShell(0)
    , d_inspectorFor(0)
    {
        s_shells.insert(this);

        if (!d_webView) {
            blpwtk2::CreateParams params;
            //params.setRendererAffinity(blpwtk2::Constants::IN_PROCESS_RENDERER);
            d_webView = blpwtk2::Toolkit::createWebView(d_mainWnd, this, params);
        }
        else
            d_webView->setParent(d_mainWnd);

        d_webView->enableFocusBefore(true);
        d_webView->enableFocusAfter(true);

        SetWindowLongPtr(d_mainWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        SetWindowLongPtr(d_urlEntryWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

    ~Shell()
    {
        SetWindowLongPtr(d_mainWnd, GWLP_USERDATA, NULL);
        SetWindowLongPtr(d_urlEntryWnd, GWLP_USERDATA, NULL);

        if (d_inspectorFor) {
            d_inspectorFor->d_inspectorShell = 0;
            d_inspectorFor = 0;
        }

        if (d_inspectorShell) {
            DestroyWindow(d_inspectorShell->d_mainWnd);
            d_inspectorShell = 0;
        }

        d_webView->destroy();

        s_shells.erase(this);
        if (0 == s_shells.size()) {
            PostQuitMessage(0);
        }
    }

    void resizeSubViews()
    {
        if (!d_webView) return;

        RECT rect;
        GetClientRect(d_mainWnd, &rect);
        assert(0 == rect.left);
        assert(0 == rect.top);
        d_webView->move(0, URLBAR_HEIGHT, rect.right, rect.bottom-URLBAR_HEIGHT, true);

        int x = NUM_BUTTONS * BUTTON_WIDTH;
        MoveWindow(d_urlEntryWnd, x, 0, rect.right-x, URLBAR_HEIGHT, TRUE);
    }


    ///////// WebViewDelegate overrides

    virtual void updateTargetURL(blpwtk2::WebView* source, const blpwtk2::StringRef& url) OVERRIDE
    {
        assert(source == d_webView);
        std::string str(url.data(), url.length());

        char buf[1024];
        sprintf_s(buf, sizeof(buf), "DELEGATE: updateTargetUrl('%s')\n", str.c_str());
        OutputDebugStringA(buf);
    }

    // Invoked when a main frame navigation occurs.
    virtual void didNavigateMainFramePostCommit(blpwtk2::WebView* source, const blpwtk2::StringRef& url) OVERRIDE
    {
        assert(source == d_webView);
        OutputDebugStringA("DELEGATE: didNavigateMainFramePostCommit\n");

        blpwtk2::String surl(url);
        SendMessageA(d_urlEntryWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(surl.c_str()));
    }

    // Invoked when the WebView creates a new WebView, for example by using
    // 'window.open'.  The default implementation of this method is to simply
    // destroy the specified 'newView'.  The delegate for the 'newView' will
    // initially be null, it can be set by modifying the specified
    // 'newViewDelegate'.
    virtual void didCreateNewView(blpwtk2::WebView* source,
                                  blpwtk2::WebView* newView,
                                  const blpwtk2::NewViewParams& params,
                                  blpwtk2::WebViewDelegate** newViewDelegate) OVERRIDE
    {
        assert(source == d_webView);

        char buf[1024];
        sprintf_s(buf, sizeof(buf), "DELEGATE: didCreateNewView('%s')\n",
                  params.targetUrl().c_str());
        OutputDebugStringA(buf);

        Shell* newShell = createShell(newView);
        *newViewDelegate = newShell;
        ShowWindow(newShell->d_mainWnd, SW_SHOW);
        UpdateWindow(newShell->d_mainWnd);
        SetFocus(newShell->d_urlEntryWnd);
    }

    // Invoked when the WebView needs to be destroyed, for example by using
    // 'window.close'.  The default implementation of this method is to simply
    // destroy the specified 'source'.
    virtual void destroyView(blpwtk2::WebView* source) OVERRIDE
    {
        assert(source == d_webView);
        OutputDebugStringA("DELEGATE: destroyView()\n");
        DestroyWindow(d_mainWnd);
    }

    // Invoked when WebKit is done tabbing backwards through controls in the page.  This
    // is only invoked if 'enableFocusBefore(true)' was called on the WebView.
    virtual void focusBefore(blpwtk2::WebView* source) OVERRIDE
    {
        assert(source == d_webView);
        OutputDebugStringA("DELEGATE: focusBefore()\n");
    }

    // Invoked when WebKit is done tabbing forwards through controls in the page.  This
    // is only invoked if 'enableFocusAfter(true)' was called on the WebView.
    virtual void focusAfter(blpwtk2::WebView* source) OVERRIDE
    {
        assert(source == d_webView);
        OutputDebugStringA("DELEGATE: focusAfter()\n");
    }

    // Notification that |source| has gained focus.
    virtual void focused(blpwtk2::WebView* source) OVERRIDE
    {
        assert(source == d_webView);
        OutputDebugStringA("DELEGATE: focused\n");
    }

    virtual void showContextMenu(blpwtk2::WebView* source, const blpwtk2::ContextMenuParams& params) OVERRIDE
    {
        assert(source == d_webView);
        OutputDebugStringA("DELEGATE: showContextMenu\n");

        HMENU menu = createContextMenu(params);
        TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                       params.pointOnScreen().x, params.pointOnScreen().y,
                       0, d_mainWnd, NULL);
        DestroyMenu(menu);
    }

    virtual void handleMediaRequest(blpwtk2::WebView* source, blpwtk2::MediaRequest* request) OVERRIDE
    {
        assert(source == d_webView);
        assert (request);
        OutputDebugStringA("DELEGATE: handleMedaiRequest\n");

        request->addRef();

        // Grant access to the first audio and first video devices if they exist.
        int* deviceIndices = new int[2];
        bool audioFound = false, videoFound = false;
        int index = 0;
        for (int i = 0; i < request->deviceCount(); ++i) {
            if (!audioFound && request->deviceType(i) == blpwtk2::MediaRequest::DEVICE_TYPE_AUDIO){
                deviceIndices[index++] = i;
                audioFound = true;
            }
            if (!videoFound && request->deviceType(i) == blpwtk2::MediaRequest::DEVICE_TYPE_VIDEO){
                deviceIndices[index++] = i;
                videoFound = true;
            }
        }
        request->grantAccess(deviceIndices, index);

        delete[] deviceIndices;
        request->release();
    }
};
std::set<Shell*> Shell::s_shells;

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int)
{
    g_instance = instance;

    int rc = registerShellWindowClass();
    if (rc) return rc;

    blpwtk2::Toolkit::setThreadMode(blpwtk2::ThreadMode::RENDERER_MAIN);
    blpwtk2::Toolkit::setHttpTransactionHandler(createHttpTransactionHandler());
#if AUTO_PUMP
    blpwtk2::Toolkit::setPumpMode(blpwtk2::PumpMode::AUTOMATIC);
#endif

    Shell* firstShell = createShell();
    firstShell->d_webView->loadUrl("http://www.google.com");
    //firstShell->d_webView->loadUrl("file://c:/stuff/test.html");
    //firstShell->d_webView->loadUrl("http://get.webgl.org");
    //firstShell->d_webView->loadUrl("http://andrew-hoyer.com/experiments/walking/");
    ShowWindow(firstShell->d_mainWnd, SW_SHOW);
    UpdateWindow(firstShell->d_mainWnd);
    firstShell->d_webView->focus();

    MSG msg;
#if AUTO_PUMP
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#else
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (!blpwtk2::Toolkit::preHandleMessage(&msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        blpwtk2::Toolkit::postHandleMessage(&msg);
    }
#endif

    blpwtk2::Toolkit::shutdown();
    return 0;
}

LRESULT CALLBACK shellWndProc(HWND hwnd,        // handle to window
                              UINT uMsg,        // message identifier
                              WPARAM wParam,    // first message parameter
                              LPARAM lParam)    // second message parameter
{
    Shell* shell = reinterpret_cast<Shell*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!shell) return DefWindowProc(hwnd, uMsg, wParam, lParam);

    int wmId;
    Shell* newShell;
    switch(uMsg) {
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        switch (wmId) {
        case IDC_RELOAD:
            shell->d_webView->reload();
            shell->d_webView->focus();
            break;
        case IDM_NEW_WINDOW:
            newShell = createShell();
            ShowWindow(newShell->d_mainWnd, SW_SHOW);
            UpdateWindow(newShell->d_mainWnd);
            SetFocus(newShell->d_urlEntryWnd);
            break;
        case IDM_CLOSE_WINDOW:
            DestroyWindow(shell->d_mainWnd);
            return 0;
        case IDM_TEST_APPEND_ELEMENT:
            appendElement(shell->d_webView);
            return 0;
        case IDM_TEST_APPEND_TABLE:
            appendTable(shell->d_webView);
            return 0;
        case IDM_TEST_V8_APPEND_ELEMENT:
            v8AppendElement(shell->d_webView);
            return 0;
        case IDM_CUT:
            shell->d_webView->cutSelection();
            return 0;
        case IDM_COPY:
            shell->d_webView->copySelection();
            return 0;
        case IDM_PASTE:
            shell->d_webView->paste();
            return 0;
        case IDM_DELETE:
            shell->d_webView->deleteSelection();
            return 0;
        case IDM_INSPECT:
            if (shell->d_inspectorShell) {
                BringWindowToTop(shell->d_inspectorShell->d_mainWnd);
                shell->d_inspectorShell->d_webView->focus();
                return 0;
            }
            shell->d_inspectorShell = createShell();
            shell->d_inspectorShell->d_inspectorFor = shell;
            ShowWindow(shell->d_inspectorShell->d_mainWnd, SW_SHOW);
            UpdateWindow(shell->d_inspectorShell->d_mainWnd);
            shell->d_inspectorShell->d_webView->loadInspector(shell->d_webView);
            shell->d_inspectorShell->d_webView->focus();
            return 0;
        case IDM_EXIT:
            std::vector<Shell*> shells(Shell::s_shells.begin(), Shell::s_shells.end());
            for (int i = 0, size = shells.size(); i < size; ++i)
                DestroyWindow(shells[i]->d_mainWnd);
            return 0;
        }
        break;
    case WM_WINDOWPOSCHANGED:
        blpwtk2::Toolkit::onRootWindowPositionChanged(hwnd);
        break;
    case WM_SETTINGCHANGE:
        blpwtk2::Toolkit::onRootWindowSettingChange(hwnd);
        break;
    case WM_DESTROY:
        delete shell;
        return 0;
    case WM_SIZE:
        shell->resizeSubViews();
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK urlEntryWndProc(HWND hwnd,        // handle to window
                                 UINT uMsg,        // message identifier
                                 WPARAM wParam,    // first message parameter
                                 LPARAM lParam)    // second message parameter
{
    Shell* shell = reinterpret_cast<Shell*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!shell) return CallWindowProc(g_defaultEditWndProc, hwnd, uMsg, wParam, lParam);

    switch (uMsg) {
    case WM_CHAR:
        if (wParam == VK_RETURN) {
            const int MAX_URL_LENGTH = 255;
            char str[MAX_URL_LENGTH + 1];  // Leave room for adding a NULL;
            *((WORD*)str) = MAX_URL_LENGTH;
            LRESULT str_len = SendMessageA(hwnd, EM_GETLINE, 0, (LPARAM)str);
            if (str_len > 0) {
                str[str_len] = 0;  // EM_GETLINE doesn't NULL terminate.
                shell->d_webView->loadUrl(str);
                shell->d_webView->focus();
            }
            return 0;
        }
    }

    return CallWindowProc(g_defaultEditWndProc, hwnd, uMsg, wParam, lParam);
}



int registerShellWindowClass()
{
    WNDCLASSEX wcx;

    // Fill in the window class structure with parameters
    // that describe the main window.

    wcx.cbSize = sizeof(wcx);               // size of structure
    wcx.style = CS_HREDRAW | CS_VREDRAW;    // redraw if size changes
    wcx.lpfnWndProc = shellWndProc;         // points to window procedure
    wcx.cbClsExtra = 0;                     // no extra class memory
    wcx.cbWndExtra = 0;                     // no extra window memory
    wcx.hInstance = g_instance;             // handle to instance
    wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);    // predefined app. icon
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);      // predefined arrow
    wcx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);    // white background brush
    wcx.lpszMenuName =  NULL;               // name of menu resource
    wcx.lpszClassName = L"ShellWClass";     // name of window class
    wcx.hIconSm = (HICON)LoadImage(g_instance,  // small class icon
                                   MAKEINTRESOURCE(5),
                                   IMAGE_ICON,
                                   GetSystemMetrics(SM_CXSMICON),
                                   GetSystemMetrics(SM_CYSMICON),
                                   LR_DEFAULTCOLOR);

    // Register the window class.  RegisterClassEx returns 0 for failure!!
    return RegisterClassEx(&wcx) == 0 ? -1 : 0;
}

Shell* createShell(blpwtk2::WebView* webView)
{
    // Create the main window.
    HWND mainWnd = CreateWindow(L"ShellWClass",      // name of window class
                                L"Sample",           // title-bar string
                                WS_OVERLAPPEDWINDOW, // top-level window
                                CW_USEDEFAULT,       // default horizontal position
                                CW_USEDEFAULT,       // default vertical position
                                CW_USEDEFAULT,       // default width
                                CW_USEDEFAULT,       // default height
                                (HWND) NULL,         // no owner window
                                (HMENU) NULL,        // use class menu
                                g_instance,          // handle to application instance
                                (LPVOID) NULL);      // no window-creation data
    assert(mainWnd);

    HMENU menu = CreateMenu();
    HMENU fileMenu = CreateMenu();
    AppendMenu(fileMenu, MF_STRING, IDM_NEW_WINDOW, L"&New Window");
    AppendMenu(fileMenu, MF_STRING, IDM_CLOSE_WINDOW, L"&Close Window");
    AppendMenu(fileMenu, MF_SEPARATOR, 0, 0);
    AppendMenu(fileMenu, MF_STRING, IDM_EXIT, L"E&xit");
    AppendMenu(menu, MF_POPUP, (UINT_PTR)fileMenu, L"&File");
    HMENU testMenu = CreateMenu();
    AppendMenu(testMenu, MF_STRING, IDM_TEST_APPEND_ELEMENT, L"&Append Element");
    AppendMenu(testMenu, MF_STRING, IDM_TEST_APPEND_TABLE, L"Append &Table");
    AppendMenu(testMenu, MF_STRING, IDM_TEST_V8_APPEND_ELEMENT, L"Append Element Using &V8");
    AppendMenu(menu, MF_POPUP, (UINT_PTR)testMenu, L"&Test");
    SetMenu(mainWnd, menu);

    HWND hwnd;
    int x = 0;

    hwnd = CreateWindow(L"BUTTON", L"Reload",
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON ,
                        x, 0, BUTTON_WIDTH, URLBAR_HEIGHT,
                        mainWnd, (HMENU) IDC_RELOAD, g_instance, 0);
    assert(hwnd);
    x += BUTTON_WIDTH;

    // This control is positioned by resizeSubViews.
    HWND urlEntryWnd = CreateWindow(L"EDIT", 0,
                                    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT |
                                    ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                                    x, 0, 0, 0, mainWnd, 0, g_instance, 0);
    assert(urlEntryWnd);

    if (!g_defaultEditWndProc)
        g_defaultEditWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(urlEntryWnd, GWLP_WNDPROC));
    SetWindowLongPtr(urlEntryWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(urlEntryWndProc));

    return new Shell(mainWnd, urlEntryWnd, webView);
}

HMENU createContextMenu(const blpwtk2::ContextMenuParams& params)
{
    bool addSeparator = false;
    if (params.canCut() || params.canCopy() || params.canPaste() || params.canDelete())
        addSeparator = true;

    HMENU menu = CreatePopupMenu();
    if (params.canCut())
        AppendMenu(menu, MF_STRING, IDM_CUT, L"C&ut");
    if (params.canCopy())
        AppendMenu(menu, MF_STRING, IDM_COPY, L"&Copy");
    if (params.canPaste())
        AppendMenu(menu, MF_STRING, IDM_PASTE, L"&Paste");
    if (params.canDelete())
        AppendMenu(menu, MF_STRING, IDM_DELETE, L"&Delete");

    if (addSeparator)
        AppendMenu(menu, MF_SEPARATOR, 0, NULL);

    AppendMenu(menu, MF_STRING, IDM_INSPECT, L"I&nspect");
    return menu;
}


class DummyHttpTransactionHandler : public blpwtk2::HttpTransactionHandler {
    // This dummy HttpTransactionHandler handles all "http://cdrive/" requests
    // and responds with the file at the specified path in the C drive.  For
    // example:
    //
    //     http://cdrive/stuff/test.html
    //
    // will return the contents of:
    //
    //     C:\stuff\test.html

public:
    virtual bool startTransaction(blpwtk2::HttpTransaction* transaction,
                                  void** userData) OVERRIDE
    {
        const char PREFIX[] = "http://cdrive/";
        blpwtk2::String url = transaction->url();
        if (url.length() <= sizeof(PREFIX)-1)
            return false;
        blpwtk2::StringRef prefix(url.data(), sizeof(PREFIX)-1);
        if (!prefix.equals(PREFIX))
            return false;

        std::string filePath = "C:\\";
        filePath.append(url.c_str() + sizeof(PREFIX)-1);
        std::replace(filePath.begin(), filePath.end(), '/', '\\');

        std::ifstream* fstream = new std::ifstream(filePath.c_str());
        *userData = fstream;
        if (!fstream->is_open())
            transaction->replaceStatusLine("HTTP/1.1 404 Not Found");
        transaction->notifyDataAvailable();
        return true;
    }

    virtual int readResponseBody(blpwtk2::HttpTransaction* transaction,
                                 void* userData,
                                 char* buffer,
                                 int bufferLen,
                                 bool* isCompleted) OVERRIDE
    {
        std::ifstream* fstream = (std::ifstream*)userData;
        if (!fstream->is_open()) {
            strncpy(buffer, "The specified file was not found.", bufferLen-1);
            *isCompleted = true;
            return strlen(buffer);
        }

        fstream->read(buffer, bufferLen);
        if (fstream->bad())
            return -1;  // some other failure

        *isCompleted = fstream->eof();

        // If we are not at eof, that means the supplied buffer was not big
        // enough for our file.  Notify that we already have data so that we
        // will be called back immediately.
        if (!fstream->eof())
            transaction->notifyDataAvailable();

        return fstream->gcount();
    }

    virtual void endTransaction(blpwtk2::HttpTransaction* transaction, void* userData) OVERRIDE
    {
        std::ifstream* fstream = (std::ifstream*)userData;
        delete fstream;
    }
};

blpwtk2::HttpTransactionHandler* createHttpTransactionHandler()
{
    return new DummyHttpTransactionHandler();
}

