#include "min.h"
#include "BrowserControl.h"
#include "ScrollTabUI.h"
#include "atlwin.h"

extern minikit::Toolkit* g_toolkit;
extern minikit::Profile* g_profile;

std::set<MinikitWindowBase*> MinikitWindowBase::s_views;

int UniToUTF8(const wchar_t* pUniString, char *szUtf8)
{
	int nLen = WideCharToMultiByte(CP_UTF8, 0, pUniString, -1, NULL, 0, NULL, NULL);
	char *szUtf8Temp = new char[nLen + 1];
	memset(szUtf8Temp, 0, nLen + 1);
	WideCharToMultiByte(CP_UTF8, 0, pUniString, -1, szUtf8Temp, nLen, NULL, NULL);
	sprintf(szUtf8, "%s", szUtf8Temp);
	delete[] szUtf8Temp;
	return nLen;
}
int UniToUTF8(const wchar_t* pUniString, std::string& strUtf8)
{
	int nLen = WideCharToMultiByte(CP_UTF8, 0, pUniString, -1, NULL, 0, NULL, NULL);
	char *szUtf8Temp = new char[nLen + 1];
	memset(szUtf8Temp, 0, nLen + 1);
	WideCharToMultiByte(CP_UTF8, 0, pUniString, -1, szUtf8Temp, nLen, NULL, NULL);
	strUtf8 = szUtf8Temp;
	delete[] szUtf8Temp;
	return nLen;
}

std::wstring MultiByteToWide(const std::string& mb, UINT  code_page)
{
	if (mb.empty())
		return std::wstring();

	int mb_length = static_cast<int>(mb.length());
	// Compute the length of the buffer.
	int charcount = MultiByteToWideChar(code_page, 0,
		mb.data(), mb_length, NULL, 0);
	if (charcount == 0)
		return std::wstring();

	std::wstring wide;
	wide.resize(charcount);
	MultiByteToWideChar(code_page, 0, mb.data(), mb_length, &wide[0], charcount);

	return wide;
}

std::wstring MultiByteToWide(const minikit::StringRef& mb, UINT  code_page)
{
	if (mb.isEmpty())
		return std::wstring();

	int mb_length = static_cast<int>(mb.length());
	// Compute the length of the buffer.
	int charcount = MultiByteToWideChar(code_page, 0,
		mb.data(), mb_length, NULL, 0);
	if (charcount == 0)
		return std::wstring();

	std::wstring wide;
	wide.resize(charcount);
	MultiByteToWideChar(code_page, 0, mb.data(), mb_length, &wide[0], charcount);

	return wide;
}

/*
暂时用ATLWINDOW 稍后替换为 duilib
*/
class BrowserInspector : public CWindowImpl<BrowserInspector>, public MinikitWindowBase
{
public:
	DECLARE_WND_CLASS_EX(TEXT("Inspector_Window"), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, (HBRUSH)(COLOR_WINDOW + 1))

	BEGIN_MSG_MAP(BrowserInspector)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
	END_MSG_MAP()

	BrowserInspector(MinikitWindowBase* windowFor) 
		:MinikitWindowBase(NULL)
	{
		this->inspectorWindowFor = windowFor;
	}

	virtual void OnFinalMessage(_In_ HWND /*hWnd*/)
	{
		if (this->inspectorWindowFor) {
			this->inspectorWindowFor->ClearInspectorWindow();
			this->inspectorWindowFor = NULL;
		}
		delete this;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!webView) {
			minikit::WebViewCreateParams params;
			params.setProfile(g_profile);
			webView = g_toolkit->createWebView(m_hWnd, this, params);
		}
		else
			webView->setParent(m_hWnd);
		return 0;
	}
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		if (webView)
		{
			RECT rc;
			GetClientRect(&rc);
			webView->move(0, 0, rc.right - rc.left, rc.bottom - rc.top);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		TakeFocus();
		return 0;
	}

	LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (webView)
			webView->rootWindowPositionChanged();
		bHandled = FALSE;
		return 0;
	}

	HWND GetMainWnd(void)
	{
		return m_hWnd;
	}

	void loadInspector(MinikitWindowBase* inspectorWindowFor)
	{
		this->inspectorWindowFor = inspectorWindowFor;
		if (this->webView && inspectorWindowFor)
		{
			webView->loadInspector(inspectorWindowFor->getWebView());
		}
	}
};

void MinikitWindowBase::showInspector()
{
	if (inspectorWindow == NULL) {
		BrowserInspector* ins = new BrowserInspector(this);
		inspectorWindow = ins;

		RECT rc;
		::GetWindowRect(::GetDesktopWindow(), &rc);

		ins->Create(::GetDesktopWindow(), rc, NULL, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		ins->SetWindowTextW(TEXT("审查元素"));

		ins->loadInspector(this);

		ins->ShowWindow(SW_SHOW);
		ins->UpdateWindow();

		inspectorWindow->getWebView()->inspectElementAt(getContextMenuPoint());
		inspectorWindow->getWebView()->takeKeyboardFocus();
		inspectorWindow->getWebView()->setLogicalFocus(true);;
	}
}

BrowserControl::BrowserControl(WindowImplBase* pParent)
{	
	m_bDelayCreate = false;
	m_pParent = pParent;
	m_hImc = ImmCreateContext();

	inspectorWindow = NULL;
	inspectorWindowFor = NULL;
}

void BrowserControl::DoCreateControl()
{
	if (m_hWnd)
		return;
	if (webView)
	{
		webView->setParent(m_pParent->GetHWND());
		if (m_bSetPos)
			webView->move(0, 0, 1024, 800);
		else
			webView->move(m_rcItem.left,
			m_rcItem.top, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top);
		m_hWnd = webView->getView();
		return;
	}

	minikit::WebViewCreateParams params;
	params.setProfile(g_profile);
	//TODO
	//params.setAllowRunningOfInsecureContent(true);
	webView = g_toolkit->createWebView(m_pParent->GetHWND(), this, params);

	webView->enableAltDragRubberbanding(false);
	webView->enableFocusBefore(true);
	webView->enableFocusAfter(true);
	webView->enableNCHitTest(false);
	webView->enableCustomTooltip(false);

	if (m_bSetPos)
		webView->move(0, 0, 1024, 800);
	else
		webView->move(m_rcItem.left,
						m_rcItem.top, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top);
	m_hWnd = webView->getView();
	webView->loadUrl(m_url.c_str());
}
BOOL BrowserControl::PreMessageHandler(LPMSG pMsg, LRESULT& lr)
{
	if (!m_hWnd)
		return FALSE;

	if (pMsg->message == WM_IME_COMPOSITION){
		// 解决微软输入法位置异常的问题
		HIMC hIMC = ImmGetContext(pMsg->hwnd);
		if (hIMC)
		{
			// Set composition window position near caret position
			POINT point;
			GetCursorPos(&point);
			ScreenToClient(pMsg->hwnd, &point);

			COMPOSITIONFORM Composition;
			Composition.dwStyle = CFS_POINT;
			Composition.ptCurrentPos.x = point.x;
			Composition.ptCurrentPos.y = point.y;
			ImmSetCompositionWindow(hIMC, &Composition);

			ImmReleaseContext(pMsg->hwnd, hIMC);
		}
		return FALSE;
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_F5) {
		if (webView){
			short f = GetAsyncKeyState(VK_CONTROL);
			webView->reload( (f>>15) == 1);
		}
		return TRUE;
	}
	return FALSE;
}

BrowserControl::~BrowserControl()
{
	if (webView)
	{
		if (inspectorWindow) {
			HWND main = inspectorWindow->GetMainWnd();
			::SendMessage(main, WM_CLOSE, 0, 0);
		}
		webView->destroy();
		webView = NULL;
	}
	if (m_hImc)
		ImmDestroyContext(m_hImc);
}


LPCTSTR BrowserControl::GetClass() const
{
	return _T("BrowserUI");
}

LPVOID BrowserControl::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, DUI_BROWSER_CONTROL) == 0) return this;
	return NULL;
}

void BrowserControl::CloseBrowser()
{
	if (webView)
	{
		webView->destroy();
		webView = NULL;
		CControlUI* p = this->GetParent();
		if (p)
		{
			CTabLayoutUI* pUI2 = (CTabLayoutUI*)p;
			if (pUI2)
				pUI2->Remove(this);
		}
	}
}

void BrowserControl::SetActive()
{
	CControlUI* p = this->GetParent();
	if (p)
	{
		CTabLayoutUI* pUI2 = (CTabLayoutUI*)p;
		if (pUI2)
			pUI2->SelectItem(this);
	}
}

void BrowserControl::didCreateNewView(minikit::WebView* source,
	minikit::WebView* newView,
	const minikit::NewViewParams& params,
	minikit::WebViewDelegate** newViewDelegate)
{
	CTabLayoutUI* pUI = (CTabLayoutUI*)m_pManager->FindControl(TEXT("mainTab"));
	if (pUI)
	{
		pUI->SelectItem(1);

		CTabLayoutUI* pUI2 = (CTabLayoutUI*)m_pManager->FindControl(TEXT("browserLayout"));
		BrowserControl* pNewControl = new BrowserControl(m_pParent);
		CScrollTabUI* pScroll = (CScrollTabUI*)m_pManager->FindControl(TEXT("scrollToolBar"));
		pScroll->AddTabItem(TEXT("空白页"))->SetUserParam(pNewControl);
		pNewControl->SetView(newView);
		pUI2->Add(pNewControl);
		*newViewDelegate = pNewControl;
		pUI2->SelectItem(pNewControl);
	}
}

void BrowserControl::DoEvent(TEventUI& event)
{
	if (event.Type == UIEVENT_SETFOCUS) {
		if (webView) {
			HWND hFocus = ::GetFocus();
			if (hFocus != webView->getView()) {
				HIMC old = ImmGetContext(webView->getView());
				if (old == NULL) {
					old = ImmAssociateContext(webView->getView(), m_hImc);
				}
				::SetFocus(webView->getView());
			}
			webView->setLogicalFocus(true);
		}
		m_bFocused = TRUE;
		return;
	}
	else if (event.Type == UIEVENT_KILLFOCUS) {
		if (webView) {
			webView->setLogicalFocus(false);
		}
		m_bFocused = FALSE;
		return;
	}
	
	__super::DoEvent(event);
}

void BrowserControl::SetURL(LPCTSTR url)
{
	m_url.clear();
	UniToUTF8(url, m_url);
	size_t pos = m_url.find("local://");
	if (pos == 0) {
		m_url = "http://localhost:10995/" + m_url.substr(pos + 8);
	}

	if (webView)
	{
		webView->loadUrl(m_url.c_str());
	}
	Invalidate();
}
void BrowserControl::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("url")) == 0)
	{
		SetURL(pstrValue);
		return;
	}
	else if (_tcscmp(pstrName, _T("delaycreate")) == 0)
	{
		SetDelayCreate(_tcscmp(pstrValue, _T("true")) == 0);
	}
	__super::SetAttribute(pstrName, pstrValue);
}
void BrowserControl::SetVisible(bool bVisible)
{
	__super::SetVisible(bVisible);
	HWND win = GetHWND();
	if (win) {
		::ShowWindow(win, bVisible ? SW_SHOW : SW_HIDE);
		::EnableWindow(win, bVisible);
	}

}

void BrowserControl::SetDelayCreate(bool v)
{
	m_bDelayCreate = v;
}
void BrowserControl::SetInternVisible(bool bVisible)
{
	__super::SetInternVisible(bVisible);
	HWND win = GetHWND();
	if (win) {
		::ShowWindow(win, bVisible ? SW_SHOW : SW_HIDE);
		::EnableWindow(win, bVisible);
	}
}

void BrowserControl::SetPos(RECT rc)
{
	__super::SetPos(rc);
	if (!m_hWnd) {
		if (m_bDelayCreate)
			return;
		DoCreateControl();
	}
	HWND win = GetHWND();
	if (win)
		::SetWindowPos(win, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
}

BrowserControl* BrowserControl::CreateControl(WindowImplBase* window, LPCTSTR pstrClassName)
{
	if (_tcsicmp(pstrClassName, DUI_BROWSER_CONTROL) == 0)
	{
		return new BrowserControl(window);
	}
	return NULL;
}

void BrowserControl::updateTitle(minikit::WebView* source, const minikit::StringRef& title)
{
	if (source == webView) {
		std::wstring str = MultiByteToWide(title, CP_UTF8);
		if (str.length() > 20)
			str = str.substr(0, 20);
		CScrollTabUI* pScroll = (CScrollTabUI*)m_pManager->FindControl(TEXT("scrollToolBar"));
		if (pScroll != NULL) {
			pScroll->UpdateItemText(this, str.c_str());
		}
	}
}