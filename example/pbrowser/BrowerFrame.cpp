
#include "min.h"
#include "BrowerFrame.h"
#include "resource.h"
#include "BrowserControl.h"

BrowerFrame::BrowerFrame(LPCTSTR pszXMLPath):
	m_strXMLPath(pszXMLPath),
	m_pBtnGoBack(NULL),
	m_pBtnRefresh(NULL),
	m_pTabTop(NULL)
{
}


BrowerFrame::~BrowerFrame()
{
}
void BrowerFrame::OnFinalMessage(HWND hWnd)
{
	delete this;
	PostQuitMessage(0);
}
CDuiString BrowerFrame::GetSkinFile()
{
	return m_strXMLPath;
}

CDuiString BrowerFrame::GetSkinFolder()
{
	return _T("Skin");
}

LPCTSTR BrowerFrame::GetWindowClassName() const
{
	return _T("BrowserMainWnd");
}

void BrowerFrame::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("windowinit"))
	{
		//OnPrepare(msg);
	}
	if (msg.sType == _T("click"))
	{
		CDuiString name = msg.pSender->GetName();
		if (name== _T("closebtn")){
			SetLoopState(EXIT_APP);

			//ShowWindow(false);
			PostMessage(WM_CLOSE, 0, 0);
			return;
		}
		else if (name == _T("minbtn"))
		{
			::ShowWindow(m_hWnd, SW_MINIMIZE);
			return;

		}
		else if (name == _T("Btn_Home"))
		{
			CTabLayoutUI* pUI = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("mainTab"));
			if (pUI)
			{
				pUI->SelectItem(0);
			}
		}
		else if (name == _T("Btn_Goback"))
		{
			CTabLayoutUI* pUI = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("mainTab"));
			if (pUI)
			{
				int cursel = pUI->GetCurSel();
				if (pUI->GetCurSel() != 1)
					pUI->SelectItem(1);
				else
				{
					CTabLayoutUI* pBrowserTab = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("browserLayout"));
					if (pBrowserTab)
					{
						BrowserControl* pControl = (BrowserControl*)pBrowserTab->GetItemAt(pBrowserTab->GetCurSel());
						if (pControl)
							pControl->GoBack();

					}
				}
			}
		}
		else if (name == _T("Btn_Refresh"))
		{
			CTabLayoutUI* pUI = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("mainTab"));
			if (pUI)
			{
				int cursel = pUI->GetCurSel();
				if (cursel == 0)
				{
					BrowserControl* pBrowser = (BrowserControl*)m_PaintManager.FindControl(TEXT("homebrowser"));
					pBrowser->Reload();
				}
				else
				{
					CTabLayoutUI* pBrowserTab = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("browserLayout"));
					if (pBrowserTab)
					{
						BrowserControl* pControl = (BrowserControl*)pBrowserTab->GetItemAt(pBrowserTab->GetCurSel());
						if (pControl)
							pControl->Reload();

					}
				}
			}

		}
		else if (name == _T("Btn_Title_Search")){
			OnClickSearch();
		}
		
	}
	else if (msg.sType == DUI_MSGTYPE_KILLFOCUS){
		
	}
	else if (msg.sType == DUI_MSGTYPE_RETURN){
		CDuiString name = msg.pSender->GetName();
		if (name == TEXT("Edt_Title_Search")) {
			OnClickSearch();
		}
	}
}

void BrowerFrame::SetLoopState(LOOP_STATE state)
{

}

CControlUI* BrowerFrame::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, DUI_BROWSER_CONTROL) == 0)
	{
		return BrowserControl::CreateControl(this, pstrClass);
	}
	else if (_tcsicmp(pstrClass, DUI_SCROLLTABUI_CONTROL) == 0)
	{
		CScrollTabUI* pUI = new CScrollTabUI();
		pUI->SetCallback(this);
		return pUI;
	}
	return NULL;
}
void BrowerFrame::InitWindow()
{
	RECT rc;
	GetWindowRect(::GetDesktopWindow(), &rc);
	SetWindowPos(GetHWND(), HWND_TOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER);
	SetIcon(IDI_MAIN);
	CenterWindow();
}

LRESULT BrowerFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (UIEVENT_TRAYICON == uMsg){
		if (lParam == WM_LBUTTONDBLCLK)
		{
			ShowWindow(true, true);
		}
	}
	bHandled = FALSE;
	return 0;
}

void BrowerFrame::OnClickSearch()
{
	CRichEditUI* pEdit = (CRichEditUI*)m_PaintManager.FindControl(TEXT("Edt_Title_Search"));
	if (!pEdit){
		return;
	}

	CScrollTabUI* pTab = (CScrollTabUI*)m_PaintManager.FindControl(TEXT("scrollToolBar"));
	if (!pTab) {
		return;
	}

	CDuiString text = pEdit->GetText();

	CTabLayoutUI* pUI = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("mainTab"));
	if (pUI)
	{
		pUI->SelectItem(1);
		//https://www.baidu.com/s?ie=UTF-8&wd=97cpz
		CTabLayoutUI* pUI2 = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("browserLayout"));
		
		CDuiString url;
		if (_tcsncmp(text.GetData(), TEXT("http://"), 7) == 0 || _tcsncmp(text.GetData(), TEXT("https://"), 8) == 0) {
			url = text;
		}
		else {
			url.Format(TEXT("%s%s"),TEXT("https://www.baidu.com/s?ie=UTF-8&wd="), text.GetData());
		}
		if (pUI2->GetCount() == 0) {
			BrowserControl* pNewControl = new BrowserControl(this);
			pNewControl->SetURL(url);
			pTab->AddTabItem(TEXT("百度"))->SetUserParam(pNewControl);
			pUI2->Add(pNewControl);
			pUI2->SelectItem(pNewControl);

		}
		else {
			BrowserControl* pControl = (BrowserControl*)pUI2->GetItemAt(pUI2->GetCurSel());
			pControl->SetURL(url);
		}
		
	}
}

void   BrowerFrame::OnScrollTabCloseItem(CScrollTabUI* pTab, const int nDelIndex, const int nSelIndex, void* data)
{
	BrowserControl* pControl = (BrowserControl*) data;
	if (pControl) {
		pControl->CloseBrowser();
	}
	if (nSelIndex != -1)
	{
		CScrollOptionUI* pUI = pTab->GetTabItem(nSelIndex);
		if (pUI)
		{
			BrowserControl* pControl = (BrowserControl*)pUI->GetUserParam();
			if (pControl)
				pControl->SetActive();
		}
	}
	else if (pTab->GetItemCount() == 0)
	{
		CTabLayoutUI* pUI = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("mainTab"));
		if (pUI)
		{
			pUI->SelectItem(0);
		}
	}
}
void   BrowerFrame::OnScrollTabSelectChange(CScrollTabUI* pTab, const int nUnSelIndex, const int nSelIndex)
{
	CScrollOptionUI* pUI = pTab->GetTabItem(nSelIndex);
	if (pUI)
	{
		BrowserControl* pControl = (BrowserControl*)pUI->GetUserParam();
		if (pControl)
			pControl->SetActive();
	}
}
void    BrowerFrame::OnScrollTabAddItem(CScrollTabUI* pTab)
{
	CTabLayoutUI* pUI = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("mainTab"));
	if (pUI)
	{
		if (pUI->GetCurSel() != 1)
			pUI->SelectItem(1);

		CTabLayoutUI* pUI2 = (CTabLayoutUI*)m_PaintManager.FindControl(TEXT("browserLayout"));
		BrowserControl* pNewControl = new BrowserControl(this);
		pNewControl->ApplyAttributeList(TEXT("url=\"http://news.baidu.com.cn\""));
		pTab->AddTabItem(TEXT("百度新闻"))->SetUserParam(pNewControl);
		pUI2->Add(pNewControl);
		pUI2->SelectItem(pNewControl);
	}
}

void    BrowerFrame::OnScrollTabDbClick(CScrollTabUI* pTab, const int nIndex)
{
	pTab->DeleteItem(nIndex);
}