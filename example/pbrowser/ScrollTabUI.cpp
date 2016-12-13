#include "../duilib/UIlib.h"
#include "ScrollTabUI.h"


namespace UiLib{

#define BTN_CLOSE_WIDTH     14  
#define BTN_CLOSE_HEIGHT    14  

	CScrollOptionUI::CScrollOptionUI(void)
		: m_pBtnExit(NULL)
		, m_pParent(NULL)
		,m_pData(NULL)
	{
	}


	CScrollOptionUI::~CScrollOptionUI(void)
	{
	}

	void CScrollOptionUI::Init()
	{
		m_pBtnExit = new CButtonUI;
		CDuiString strAttr;
		strAttr.Format(TEXT("float=\"true\" pos=\"%d,%d,%d,%d\" normalimage=\"file='tab\\close_normal.png' source='0,0,23,18' dest='0,0,%d,%d'\" ")
			TEXT("hotimage=\"file='tab\\close_hover.png' source='0,0,23,18' dest='0,0,%d,%d'\""),
			m_cxyFixed.cx - BTN_CLOSE_WIDTH, 0, m_cxyFixed.cx, BTN_CLOSE_HEIGHT, BTN_CLOSE_WIDTH, BTN_CLOSE_HEIGHT, BTN_CLOSE_WIDTH, BTN_CLOSE_HEIGHT);
		m_pBtnExit->ApplyAttributeList(strAttr);
		Add(m_pBtnExit);
		m_pBtnExit->OnNotify += MakeDelegate(this, &CScrollOptionUI::OnBtnClose, _T(""));
	}

	void CScrollOptionUI::PaintText(HDC hDC)
	{
		RECT rc = m_rcItem;
		rc.top += m_pManager->GetDpiValue(2);
		rc.bottom += m_pManager->GetDpiValue(2);
		rc.left += m_pManager->GetDpiValue(4);
		rc.right -= m_pManager->GetDpiValue(4);
		CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, 0xFFFFFFFF, 0, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	}

	void CScrollOptionUI::SetPos(RECT rc)
	{
		CContainerUI::SetPos(rc);
		RECT rcExit = { rc.right - m_pManager->GetDpiValue(BTN_CLOSE_WIDTH - 1), rc.top + m_pManager->GetDpiValue(2), 
			rc.right - m_pManager->GetDpiValue(1), rc.top + m_pManager->GetDpiValue(BTN_CLOSE_HEIGHT -1) };
		m_pBtnExit->SetPos(rcExit);
	}

	void CScrollOptionUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_BUTTONDOWN && m_pParent)
		{
			m_pParent->SelectItem(this);
		}
		else if (event.Type == UIEVENT_DBLCLICK && m_pParent)
		{
			m_pParent->OnOptionDbClick(this);
		}
		CContainerUI::DoEvent(event);
	}

	bool CScrollOptionUI::OnBtnClose(TNotifyUI* pParam, LPARAM lParam, WPARAM wParam)
	{
		TNotifyUI* pNotifyUI = (TNotifyUI*)pParam;
		if (pNotifyUI->sType == DUI_MSGTYPE_CLICK && m_pParent)
		{
			m_pParent->DeleteItem(this);
		}
		return true;
	}

	CScrollTabUI::CScrollTabUI(void)
		: m_nNorWidth(100)
		, m_nCurWidth(0)
		, m_nMinWidth(30)
		, m_dwTabSelColor(0xffff0000)
		, m_dwTabNorColor(0x0000000)
		, m_nSelIndex(-1)
		, m_pBtnAdd(NULL)
		, m_pCallback(NULL)
	{
	}


	CScrollTabUI::~CScrollTabUI(void)
	{
	}
	
	CScrollOptionUI* CScrollTabUI::GetTabItem(int nIndex)
	{
		if (nIndex < 0 || nIndex >= m_pTabItems.size())
			return NULL;
		return m_pTabItems.at(nIndex);
	}

	CScrollOptionUI* CScrollTabUI::AddTabItem(LPCTSTR lpText, bool selected, bool bReset/*=false*/)
	{
		CScrollOptionUI* pTabItem = new CScrollOptionUI;
		if (NULL == pTabItem)
			return NULL;
		CDuiString strAttr;
		strAttr.Format(L"float=\"true\" bordercolor=\"#FF999999\" bordersize=\"1\" borderround=\"4,4\" text=\"%s\" tooltip=\"%s\"",
			lpText, lpText);
		pTabItem->ApplyAttributeList(strAttr);
		pTabItem->SetParent(this);
		if (!Add(pTabItem))
		{
			delete pTabItem;
			return NULL;
		}
		if (m_pTabItems.empty())
			m_nSelIndex = 0;
		m_pTabItems.push_back(pTabItem);
		if (selected)
			m_nSelIndex = m_pTabItems.size() - 1;
		if (bReset)
			ResetTabPos();
		return pTabItem;
	}

	void CScrollTabUI::SelectItem(CControlUI* pItem)
	{
		if (m_nSelIndex >= 0 && m_nSelIndex<m_pTabItems.size())
		{
			CControlUI* pCurItem = m_pTabItems[m_nSelIndex];
			if (pCurItem)
				pCurItem->SetBkColor(m_dwTabNorColor);
		}
		pItem->SetBkColor(m_dwTabSelColor);
		int nUnSelIndex = m_nSelIndex;
		m_nSelIndex = GetTabItemIndex(pItem);
		if (m_pCallback)
			m_pCallback->OnScrollTabSelectChange(this, nUnSelIndex, m_nSelIndex);
	}

	void CScrollTabUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (wcscmp(pstrName, L"TabWidth") == 0)
		{
			m_nNorWidth = m_pManager->GetDpiValue(_ttoi(pstrValue));
			return;
		}
		if (wcscmp(pstrName, L"MinTabWidth") == 0)
		{
			m_nMinWidth = m_pManager->GetDpiValue(_ttoi(pstrValue));
			return;
		}
		if (wcscmp(pstrName, L"TabSelColor") == 0)
		{
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			m_dwTabSelColor = _tcstoul(pstrValue, &pstr, 16);
			return;
		}
		if (wcscmp(pstrName, L"TabNorColor") == 0)
		{
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			m_dwTabNorColor = _tcstoul(pstrValue, &pstr, 16);
			return;
		}
		CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CScrollTabUI::Init()
	{
		m_pBtnAdd = new CButtonUI;
		CDuiString strAttr = TEXT("float=\"true\" pos=\"0,0,0,0\" width=\"25\" height=\"25\" normalimage=\"file='tab\\newtab.png' source='0,0,28,18' dest='0,0,25,25'\" ")  
			TEXT("hotimage=\"file='tab\\newtab_hover.png' source='0,0,28,18' dest='0,0,25,25'\" pushedimage=\"file='tab\\newtab_pressed.png' source='0,0,28,18' dest='0,0,25,25'\" ")
			TEXT("tooltip=\"ÐÂ½¨\" ");
		m_pBtnAdd->ApplyAttributeList(strAttr);
		Add(m_pBtnAdd);
		m_pBtnAdd->OnNotify += MakeDelegate(this, &CScrollTabUI::OnBtnClickAdd,_T(""));
	}

	void CScrollTabUI::DeleteItem(CControlUI* pItem)
	{
		int nDelIndex = GetTabItemIndex(pItem);
		if (nDelIndex == -1)
			return;
		DeleteItem(nDelIndex);
	}

	void CScrollTabUI::DeleteItem(const int nIndex)
	{
		if (nIndex<0 || nIndex >= m_pTabItems.size())
			return;

		CScrollOptionUI* pUI = m_pTabItems[nIndex];
		LPVOID data = pUI->GetUserParam();
		Remove(pUI);
		m_pTabItems.erase(m_pTabItems.begin() + nIndex);
		int nCount = m_pTabItems.size();
		if (nCount>0)
		{
			if (nIndex<m_nSelIndex)
				m_nSelIndex--;
			else if (nIndex == m_nSelIndex)
			{
				m_nSelIndex = nCount - 1;
				CControlUI* pItem = m_pTabItems[m_nSelIndex];
				if (pItem)
					pItem->SetBkColor(m_dwTabSelColor);
			}
			ResetTabPos();
		}
		else
			m_nSelIndex = -1;
		if (m_pCallback)
			m_pCallback->OnScrollTabCloseItem(this, nIndex, m_nSelIndex, data);
	}

	void CScrollTabUI::ResetTabPos()
	{
		int newTabWidth = this->m_pManager->GetDpiValue(25);
		int nCount = m_pTabItems.size();
		if (nCount == 0)
		{
			RECT rcItem = { m_rcItem.left, m_rcItem.top, m_rcItem.left + newTabWidth, m_rcItem.bottom };
			m_pBtnAdd->SetPos(rcItem);
			return;
		}
		int nWidth = m_rcItem.right - m_rcItem.left - newTabWidth;
		int nHeight = m_rcItem.bottom - m_rcItem.top;
		if (nWidth / nCount < m_nMinWidth)
			return;
		if (nCount*m_nNorWidth <= nWidth)
			m_nCurWidth = m_nNorWidth;
		else
			m_nCurWidth = nWidth / nCount;
		RECT rcItem;
		for (size_t i = 0; i<m_pTabItems.size(); ++i)
		{
			rcItem.left = m_nCurWidth*i + m_rcItem.left;
			rcItem.top = 0 + m_rcItem.top;
			rcItem.right = rcItem.left + m_nCurWidth;
			rcItem.bottom = rcItem.top + nHeight;
			CControlUI* pItem = m_pTabItems[i];
			pItem->SetPos(rcItem);
			if (m_nSelIndex == i)
				pItem->SetBkColor(m_dwTabSelColor);
			else
				pItem->SetBkColor(m_dwTabNorColor);
		}
		rcItem.left = rcItem.right;
		rcItem.right = rcItem.left + newTabWidth;
		m_pBtnAdd->SetPos(rcItem);
	}

	void CScrollTabUI::SetItemText(const int nIndex, LPCTSTR lpText)
	{
		int nCount = m_pTabItems.size();
		if (0 == nCount || nCount <= nIndex)
			return;
		CControlUI* pItem = m_pTabItems[nIndex];
		if (pItem)
		{
			pItem->SetText(lpText);
			pItem->SetToolTip(lpText);
		}
	}

	int CScrollTabUI::GetTabItemIndex(CControlUI* pItem)
	{
		int nIndex = -1;
		for (size_t i = 0; i<m_pTabItems.size(); ++i)
		{
			if (m_pTabItems[i] == pItem)
			{
				nIndex = i;
				break;
			}
		}
		return nIndex;
	}

	bool CScrollTabUI::OnBtnClickAdd(TNotifyUI* pParam, LPARAM lParam, WPARAM wParam)
	{
		TNotifyUI* pNotifyUI = (TNotifyUI*)pParam;
		if (pNotifyUI->sType != DUI_MSGTYPE_CLICK)
			return true;
		if (m_pCallback)
			m_pCallback->OnScrollTabAddItem(this);
		return true;
	}

	void CScrollTabUI::SetPos(RECT rc)
	{
		CContainerUI::SetPos(rc);
		ResetTabPos();
	}

	void CScrollTabUI::OnOptionDbClick(CControlUI* pOption)
	{
		if (m_pCallback)
			m_pCallback->OnScrollTabDbClick(this, m_nSelIndex);
	}
	void    CScrollTabUI::UpdateItemText(LPVOID param, LPCTSTR lpText)
	{
		int nCount = m_pTabItems.size();
		for (int i = 0; i < nCount; i++)
		{
			CScrollOptionUI* pUI = m_pTabItems[i];
			if (pUI->GetUserParam() == param)
			{
				pUI->SetText(lpText);
				pUI->SetToolTip(lpText);
			}
		}
		
	}

}
