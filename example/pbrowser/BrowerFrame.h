#pragma once

#include "../duilib/UIlib.h"
#include "ScrollTabUI.h"
using namespace DuiLib;

enum LOOP_STATE
{
	EXIT_APP = 32		//程序退出
};

class BrowerFrame : public WindowImplBase, public CScrollTabCallback
{
public:
	explicit BrowerFrame(LPCTSTR pszXMLPath);
	~BrowerFrame();

	virtual void OnFinalMessage(HWND hWnd);

	void InitWindow();
	void Notify(TNotifyUI& msg);
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);

	void SetLoopState(LOOP_STATE state);
	LOOP_STATE GetLoopState();
	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual void    OnScrollTabCloseItem(CScrollTabUI* pTab, const int nDelIndex, const int nSelIndex, void* data);
	virtual void    OnScrollTabSelectChange(CScrollTabUI* pTab, const int nUnSelIndex, const int nSelIndex);
	virtual void    OnScrollTabAddItem(CScrollTabUI* pTab);
	virtual void    OnScrollTabDbClick(CScrollTabUI* pTab, const int nIndex);

	void OnClickSearch();
protected:
	LPCTSTR GetWindowClassName() const;
	CDuiString GetSkinFile();
	CDuiString GetSkinFolder();
private:
	CDuiString		m_strXMLPath;

	CDuiString		m_strBkImage;   //当隐藏音乐库时保存原来的背景信息用来恢复
	
	CButtonUI				*m_pBtnGoBack;
	CButtonUI				*m_pBtnRefresh;
	CTabLayoutUI			*m_pTabTop;
};

