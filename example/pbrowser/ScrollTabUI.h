#pragma once

#define  DUI_SCROLLTABUI_CONTROL                         (_T("ScrollTab"))

namespace UiLib {

	class CScrollTabUI;

	class UILIB_API CScrollOptionUI :
		public CContainerUI
	{
	public:
		CScrollOptionUI(void);
		~CScrollOptionUI(void);
		void    SetParent(CScrollTabUI* pTab)   { m_pParent = pTab; }
		void	SetUserParam(LPVOID data) { m_pData = data; }
		LPVOID	GetUserParam() { return m_pData; }
	protected:
		virtual void Init();
		virtual void PaintText(HDC hDC);
		virtual void SetPos(RECT rc);
		virtual void DoEvent(TEventUI& event);
		bool    OnBtnClose(TNotifyUI* pParam, LPARAM lParam, WPARAM wParam);
	private:
		CButtonUI*      m_pBtnExit;
		CScrollTabUI*   m_pParent;
		LPVOID			m_pData;
	};

	class CScrollTabCallback
	{
	public:
		virtual void    OnScrollTabCloseItem(CScrollTabUI* pTab, const int nDelIndex, const int nSelIndex, LPVOID userParam) = 0;
		virtual void    OnScrollTabSelectChange(CScrollTabUI* pTab, const int nUnSelIndex, const int nSelIndex) = 0;
		virtual void    OnScrollTabAddItem(CScrollTabUI* pTab) = 0;
		virtual void    OnScrollTabDbClick(CScrollTabUI* pTab, const int nIndex) = 0;
	};

	class UILIB_API CScrollTabUI
		: public CContainerUI
	{
	public:
		CScrollTabUI(void);
		~CScrollTabUI(void);
		CScrollOptionUI* AddTabItem(LPCTSTR lpText, bool selected = true, bool bReset = false);
		CScrollOptionUI* GetTabItem(int nIndex);
		void    SelectItem(CControlUI* pItem);
		void    DeleteItem(CControlUI* pItem);
		void    DeleteItem(const int nIndex);
		void    SetItemText(const int nIndex, LPCTSTR lpText);
		int     GetItemCount()const                             { return m_pTabItems.size(); }
		void    SetCallback(CScrollTabCallback* pCallback)      { m_pCallback = pCallback; }
		void    OnOptionDbClick(CControlUI* pOption);
		void    UpdateItemText(LPVOID param, LPCTSTR lpText);
	protected:
		virtual void    SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		virtual void    Init();
		virtual void    SetPos(RECT rc);
		void    ResetTabPos();
		int     GetTabItemIndex(CControlUI* pItem);
		bool    OnBtnClickAdd(TNotifyUI*, LPARAM lParam, WPARAM wParam);
	private:
		int     m_nNorWidth;
		int     m_nMinWidth;
		int     m_nCurWidth;
		int     m_nSelIndex;
		DWORD   m_dwTabSelColor;
		DWORD   m_dwTabNorColor;
		CButtonUI* m_pBtnAdd;
		vector<CScrollOptionUI*> m_pTabItems;
		CScrollTabCallback* m_pCallback;
	};

}