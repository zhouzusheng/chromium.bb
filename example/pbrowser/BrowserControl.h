#pragma once
#include "../duilib/UIlib.h"
#include "minikit.h"
#include "set"
#include "FileDlg.h"

using namespace DuiLib;

std::wstring MultiByteToWide(const minikit::StringRef& mb, UINT  code_page);


#define  DUI_BROWSER_CONTROL                         (_T("Browser"))

enum {
	IDM_START_OF_MENU_ITEMS = 2000,
	IDM_FILE,
	IDM_NEW_WINDOW,
	IDM_CLOSE_WINDOW,
	IDM_CLEAR_WEB_CACHE,
	IDM_PRINT,
	IDM_EXIT__,
	IDM_ZOOM,
	IDM_ZOOM_025,
	IDM_ZOOM_050,
	IDM_ZOOM_075,
	IDM_ZOOM_100,
	IDM_ZOOM_125,
	IDM_ZOOM_150,
	IDM_ZOOM_200,
	IDM_TEST,
	IDM_TEST_V8_APPEND_ELEMENT,
	IDM_TEST_KEYBOARD_FOCUS,
	IDM_TEST_LOGICAL_FOCUS,
	IDM_TEST_LOGICAL_BLUR,
	IDM_TEST_PLAY_KEYBOARD_EVENTS,
	IDM_TEST_GET_PICTURE,
	IDM_TEST_DUMP_LAYOUT_TREE,
	IDM_SPELLCHECK,
	IDM_SPELLCHECK_ENABLED,
	IDM_AUTOCORRECT,
	IDM_AUTOCORRECT_WORDMAP,
	IDM_AUTOCORRECT_SWAP_ADJACENT_CHARS,
	IDM_LANGUAGES,
	IDM_LANGUAGE_DE,
	IDM_LANGUAGE_EN_GB,
	IDM_LANGUAGE_EN_US,
	IDM_LANGUAGE_ES,
	IDM_LANGUAGE_FR,
	IDM_LANGUAGE_IT,
	IDM_LANGUAGE_PT_BR,
	IDM_LANGUAGE_PT_PT,
	IDM_LANGUAGE_RU,
	IDM_CUT,
	IDM_COPY,
	IDM_PASTE,
	IDM_DELETE,
	IDM_REFRESH,
	IDM_INSPECT,
	IDM_ADD_TO_DICTIONARY,
	IDM_CONTEXT_MENU_BASE_CUSTOM_TAG = 5000,
	IDM_CONTEXT_MENU_END_CUSTOM_TAG = 5999,
	IDM_CONTEXT_MENU_BASE_SPELL_TAG = 6000,
	IDM_CONTEXT_MENU_END_SPELL_TAG = 6999
};

class MinikitWindowBase : public minikit::WebViewDelegate
{
public:
	static std::set<MinikitWindowBase*> s_views;
	MinikitWindowBase(minikit::WebView* webview = NULL)
	{
		this->webView = webview;
		inspectorWindow = NULL;
		inspectorWindowFor = NULL;

		s_views.insert(this);
	}

	virtual ~MinikitWindowBase()
	{
		s_views.erase(this);
	}

	virtual HWND GetMainWnd() = 0;

	void ClearInspectorWindow()
	{
		inspectorWindow = NULL;
	}

	void loadURL(const char* url)
	{
		m_url = url;
		if (webView)
			webView->loadUrl(m_url.c_str());
	}

	void TakeFocus()
	{
		if (webView)
		{
			webView->takeKeyboardFocus();
			webView->setLogicalFocus(true);
		}
	}

	void SetView(minikit::WebView* view)
	{
		if (webView == NULL)
		{
			webView = view;
		}
	}

	void GoBack()
	{
		if (webView)
		{
			webView->goBack();
		}
	}

	void Reload()
	{
		if (webView)
		{
			webView->reload();
		}
	}

	void showContextMenu(minikit::WebView* source, const minikit::ContextMenuParams& params) override
	{
		OutputDebugStringA("DELEGATE: showContextMenu\n");

		contextMenuPoint = params.pointOnScreen();
		::ScreenToClient(GetMainWnd(), &contextMenuPoint);

		HMENU menu = createContextMenu(params);
		int ret = TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			params.pointOnScreen().x, params.pointOnScreen().y,
			0, GetMainWnd(), NULL);
		DestroyMenu(menu);

		if (ret)
		{
			switch (ret){
			case IDM_CUT:
				webView->cutSelection();
				break;
			case IDM_COPY:
				webView->copySelection();
				break;
			case IDM_PASTE:
				webView->paste();
				break;
			case IDM_DELETE:
				webView->deleteSelection();
				break;
			case IDM_REFRESH:
				webView->reload();
				break;
			case IDM_INSPECT:
				showInspector();
			}
		}
	}

	virtual void runFileChooser(minikit::WebView* source,
		const minikit::FileChooserParams& params) 
	{
		FileDialogRunner runner;
		runner.Run(source, params);
	}

	virtual HMENU createContextMenu(const minikit::ContextMenuParams& params)
	{
		bool addSeparator = false;
		if (params.canCut() || params.canCopy() || params.canPaste() || params.canDelete())
			addSeparator = true;

		HMENU menu = CreatePopupMenu();

		if (params.numCustomItems() > 0) {
			populateContextMenu(menu, IDM_CONTEXT_MENU_BASE_CUSTOM_TAG, params);
			AppendMenuA(menu, MF_SEPARATOR, 0, NULL);
		}

		if (params.canCut())
			AppendMenu(menu, MF_STRING, IDM_CUT, L"¼ôÇÐ(&U)");
		if (params.canCopy())
			AppendMenu(menu, MF_STRING, IDM_COPY, L"¸´ÖÆ(&C)");
		if (params.canPaste())
			AppendMenu(menu, MF_STRING, IDM_PASTE, L"Õ³Ìù(&P)");
		if (params.canDelete())
			AppendMenu(menu, MF_STRING, IDM_DELETE, L"É¾³ý(&D)");

		if (addSeparator)
			AppendMenu(menu, MF_SEPARATOR, 0, NULL);

		int num = params.numCustomItems();
		for (int i = 0; i < num; i++) {
			const minikit::ContextMenuItem& item = params.customItem(i);
			std::wstring str = MultiByteToWide(item.label(), CP_UTF8);
			AppendMenu(menu, MF_STRING, item.action(), str.c_str());
		}

		if (this->inspectorWindowFor == NULL)
			AppendMenu(menu, MF_STRING, IDM_INSPECT, L"Éó²éÔªËØ(&I)");

		AppendMenu(menu, MF_STRING, IDM_REFRESH, L"Ë¢ÐÂ(&R)");

		return menu;
	}

	void populateMenuItem(HMENU menu, int menuIdStart, const minikit::ContextMenuItem& item)
	{
		UINT flags = MF_STRING | (item.enabled() ? MF_ENABLED : MF_GRAYED);
		std::string label(item.label().data(),
			item.label().length());
		if (item.type() == minikit::ContextMenuItem::OPTION) {
			AppendMenuA(menu, flags, menuIdStart + item.action(), label.c_str());
		}
		else if (item.type() == minikit::ContextMenuItem::CHECKABLE_OPTION) {
			flags = flags | (item.checked() ? MF_CHECKED : MF_UNCHECKED);
			AppendMenuA(menu, flags, menuIdStart + item.action(), label.c_str());
		}
		else if (item.type() == minikit::ContextMenuItem::SEPARATOR) {
			AppendMenuA(menu, MF_SEPARATOR, 0, NULL);
		}
		else if (item.type() == minikit::ContextMenuItem::SUBMENU) {
			HMENU popupMenu = CreatePopupMenu();
			flags = flags | MF_POPUP;
			AppendMenuA(menu, flags, (UINT_PTR)popupMenu, label.c_str());
			populateSubmenu(popupMenu, menuIdStart, item);
		}
	}

	void populateContextMenu(HMENU menu, int menuIdStart, const minikit::ContextMenuParams& params)
	{
		for (int i = 0; i < params.numCustomItems(); ++i) {
			populateMenuItem(menu, menuIdStart, params.customItem(i));
		}
	}

	void populateSubmenu(HMENU menu, int menuIdStart, const minikit::ContextMenuItem& item)
	{
		for (int i = 0; i < item.numSubMenuItems(); ++i) {
			populateMenuItem(menu, menuIdStart, item.subMenuItem(i));
		}
	}

	minikit::WebView* getWebView()
	{
		return webView;
	}
	POINT		getContextMenuPoint()
	{
		return contextMenuPoint;
	}

	void showInspector();

protected:
	minikit::WebView* webView;
	MinikitWindowBase * inspectorWindow;
	MinikitWindowBase * inspectorWindowFor;
	std::string       m_url;
	POINT			  contextMenuPoint;
};

class BrowserControl : public CWindowControlUI, public MinikitWindowBase
{
public:
	BrowserControl(WindowImplBase* pParent);
	~BrowserControl();
	virtual LPCTSTR GetClass() const __override;
	virtual LPVOID GetInterface(LPCTSTR pstrName) __override;
	void DoEvent(TEventUI& event) __override;
	virtual void SetVisible(bool bVisible = true) __override;
	virtual void SetInternVisible(bool bVisible = true) __override;
	virtual void SetPos(RECT rc) __override;
	void SetURL(LPCTSTR url);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) __override;
	HWND GetHWND(){
		return webView ? webView->getView() : NULL;
	}
	
	virtual BOOL PreMessageHandler(LPMSG, LRESULT& lr);
	virtual void DoCreateControl();

	virtual void didCreateNewView(minikit::WebView* source,
		minikit::WebView* newView,
		const minikit::NewViewParams& params,
		minikit::WebViewDelegate** newViewDelegate);

	void CloseBrowser();
	void SetActive();
	virtual void updateTitle(minikit::WebView* source, const minikit::StringRef& title);

	static BrowserControl* CreateControl(WindowImplBase* window, LPCTSTR pstrClassName);

protected:
	virtual HWND GetMainWnd() {
		return m_pParent->GetHWND();
	}
	void SetDelayCreate(bool v);
private:
	WindowImplBase* m_pParent;
	bool m_bDelayCreate;
	HIMC m_hImc;

};

