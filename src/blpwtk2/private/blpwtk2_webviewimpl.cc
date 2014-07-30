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

#include <blpwtk2_webviewimpl.h>

#include <blpwtk2_browsercontextimpl.h>
#include <blpwtk2_devtoolsfrontendhostdelegateimpl.h>
#include <blpwtk2_mediaobserverimpl.h>
#include <blpwtk2_nativeviewwidget.h>
#include <blpwtk2_newviewparams.h>
#include <blpwtk2_products.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>
#include <blpwtk2_webframeimpl.h>
#include <blpwtk2_webviewdelegate.h>
#include <blpwtk2_webviewimplclient.h>

#include <base/message_loop/message_loop.h>
#include <base/strings/utf_string_conversions.h>
#include <chrome/browser/printing/print_view_manager.h>
#include <content/browser/renderer_host/render_widget_host_view_base.h>
#include <content/public/browser/devtools_agent_host.h>
#include <content/public/browser/devtools_http_handler.h>
#include <content/public/browser/render_view_host.h>
#include <content/public/browser/render_process_host.h>
#include <content/public/browser/web_contents.h>
#include <content/public/browser/web_contents_view.h>
#include <content/public/browser/site_instance.h>
#include <third_party/WebKit/public/web/WebFindOptions.h>
#include <third_party/WebKit/public/web/WebView.h>
#include <ui/base/win/hidden_window.h>
#include <webkit/common/webpreferences.h>

namespace blpwtk2 {

WebViewImpl::WebViewImpl(WebViewDelegate* delegate,
                         blpwtk2::NativeView parent,
                         BrowserContextImpl* browserContext,
                         int hostAffinity,
                         bool initiallyVisible,
                         bool takeFocusOnMouseDown,
                         bool domPasteEnabled,
                         bool javascriptCanAccessClipboard)
: d_delegate(delegate)
, d_implClient(0)
, d_browserContext(browserContext)
, d_widget(0)
, d_focusBeforeEnabled(false)
, d_focusAfterEnabled(false)
, d_isReadyForDelete(false)
, d_wasDestroyed(false)
, d_isDeletingSoon(false)
, d_isPopup(false)
, d_takeFocusOnMouseDown(takeFocusOnMouseDown)
, d_domPasteEnabled(domPasteEnabled)
, d_javascriptCanAccessClipboard(javascriptCanAccessClipboard)
, d_altDragRubberbandingEnabled(false)
, d_customTooltipEnabled(false)
, d_ncHitTestEnabled(false)
, d_ncHitTestPendingAck(false)
, d_lastNCHitTestResult(HTCLIENT)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(browserContext);

    d_browserContext->incrementWebViewCount();

    content::WebContents::CreateParams createParams(browserContext);
    createParams.render_process_affinity = hostAffinity;
    d_webContents.reset(content::WebContents::Create(createParams));
    d_webContents->SetDelegate(this);
    Observe(d_webContents.get());

    printing::PrintViewManager::CreateForWebContents(d_webContents.get());

    if (parent)
        createWidget(parent);

    if (initiallyVisible)
        show();
}

WebViewImpl::WebViewImpl(content::WebContents* contents,
                         BrowserContextImpl* browserContext,
                         bool takeFocusOnMouseDown,
                         bool domPasteEnabled,
                         bool javascriptCanAccessClipboard)
: d_delegate(0)
, d_implClient(0)
, d_browserContext(browserContext)
, d_widget(0)
, d_focusBeforeEnabled(false)
, d_focusAfterEnabled(false)
, d_isReadyForDelete(false)
, d_wasDestroyed(false)
, d_isDeletingSoon(false)
, d_isPopup(false)
, d_takeFocusOnMouseDown(takeFocusOnMouseDown)
, d_domPasteEnabled(domPasteEnabled)
, d_javascriptCanAccessClipboard(javascriptCanAccessClipboard)
, d_altDragRubberbandingEnabled(false)
, d_customTooltipEnabled(false)
, d_ncHitTestEnabled(false)
, d_ncHitTestPendingAck(false)
, d_lastNCHitTestResult(HTCLIENT)
{
    DCHECK(Statics::isInBrowserMainThread());

    d_browserContext->incrementWebViewCount();

    d_webContents.reset(contents);
    d_webContents->SetDelegate(this);
    Observe(d_webContents.get());
}

WebViewImpl::~WebViewImpl()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(d_wasDestroyed);
    DCHECK(d_isReadyForDelete);
    DCHECK(d_isDeletingSoon);
    if (d_widget) {
        d_widget->setDelegate(0);
        d_widget->destroy();
    }
}

void WebViewImpl::setImplClient(WebViewImplClient* client)
{
    d_implClient = client;
}

bool WebViewImpl::rendererMatchesSize(const gfx::Size& newSize) const
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    DCHECK(d_webContents->GetRenderViewHost());
    return newSize == d_webContents->GetRenderViewHost()->LastKnownRendererSize();
}

gfx::NativeView WebViewImpl::getNativeView() const
{
    DCHECK(Statics::isInBrowserMainThread());
    return d_webContents->GetView()->GetNativeView();
}

void WebViewImpl::showContextMenu(const ContextMenuParams& params)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed) return;
    if (d_delegate)
        d_delegate->showContextMenu(this, params);
}

void WebViewImpl::saveCustomContextMenuContext(const content::CustomContextMenuContext& context)
{
    d_customContext = context;
}

void WebViewImpl::handleFindRequest(const FindOnPageRequest& request)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);

    content::RenderViewHost* host = d_webContents->GetRenderViewHost();
    if (!request.reqId) {
        host->StopFinding(content::STOP_FIND_ACTION_CLEAR_SELECTION);
        return;
    }
    WebKit::WebFindOptions options;
    options.findNext = request.findNext;
    options.forward = request.forward;
    options.matchCase = request.matchCase;
    WebKit::WebString textStr =
        WebKit::WebString::fromUTF8(request.text.data(),
                                    request.text.length());
    host->Find(request.reqId, textStr, options);
}

void WebViewImpl::handleExternalProtocol(const GURL& url)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed || !d_delegate) return;

    d_delegate->handleExternalProtocol(this, url.spec());
}

void WebViewImpl::overrideWebkitPrefs(WebPreferences* prefs)
{
    prefs->dom_paste_enabled = d_domPasteEnabled;
    prefs->javascript_can_access_clipboard = d_javascriptCanAccessClipboard;
}

void WebViewImpl::destroy()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    DCHECK(!d_isDeletingSoon);

    d_browserContext->decrementWebViewCount();

    Observe(0);  // stop observing the WebContents
    d_wasDestroyed = true;
    if (d_isReadyForDelete) {
        d_isDeletingSoon = true;
        base::MessageLoop::current()->DeleteSoon(FROM_HERE, this);
    }
}

WebFrame* WebViewImpl::mainFrame()
{
    NOTREACHED() << "mainFrame() not supported in WebViewImpl";
    return 0;
}

void WebViewImpl::loadUrl(const StringRef& url)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    std::string surl(url.data(), url.length());
    GURL gurl(surl);
    if (!gurl.has_scheme())
        gurl = GURL("http://" + surl);

    d_webContents->GetController().LoadURL(
        gurl,
        content::Referrer(),
        content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR),
        std::string());
}

void WebViewImpl::find(const StringRef& text, bool matchCase, bool forward)
{
    DCHECK(Statics::isOriginalThreadMode())
        <<  "renderer-main thread mode should use handleFindRequest";

    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);

    if (!d_find) d_find.reset(new FindOnPage());

    handleFindRequest(d_find->makeRequest(text, matchCase, forward));
}

void WebViewImpl::print()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);

    printing::PrintViewManager* printViewManager =
        printing::PrintViewManager::FromWebContents(d_webContents.get());
    printViewManager->PrintNow();
}

void WebViewImpl::loadInspector(WebView* inspectedView)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    DCHECK(inspectedView);
    DCHECK(Statics::hasDevTools) << "Could not find: " << BLPWTK2_DEVTOOLS_PAK_NAME;

    WebViewImpl* inspectedViewImpl
        = static_cast<WebViewImpl*>(inspectedView);
    content::WebContents* inspectedContents = inspectedViewImpl->d_webContents.get();
    DCHECK(inspectedContents);

    scoped_refptr<content::DevToolsAgentHost> agentHost
        = content::DevToolsAgentHost::GetOrCreateFor(inspectedContents->GetRenderViewHost());

    d_devToolsFrontEndHost.reset(
        new DevToolsFrontendHostDelegateImpl(d_webContents.get(), agentHost));

    GURL url = Statics::devToolsHttpHandler->GetFrontendURL();
    loadUrl(url.spec());
}

void WebViewImpl::inspectElementAt(const POINT& point)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    DCHECK(d_devToolsFrontEndHost.get())
        << "Need to call loadInspector first!";
    d_devToolsFrontEndHost->agentHost()->InspectElement(point.x, point.y);
}

void WebViewImpl::reload(bool ignoreCache)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    const bool checkForRepost = false;  // TODO: do we want to make this an argument

    if (ignoreCache)
        d_webContents->GetController().ReloadIgnoringCache(checkForRepost);
    else
        d_webContents->GetController().Reload(checkForRepost);
}

void WebViewImpl::goBack()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    if (d_webContents->GetController().CanGoBack())
        d_webContents->GetController().GoBack();
}

void WebViewImpl::goForward()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    if (d_webContents->GetController().CanGoForward())
        d_webContents->GetController().GoForward();
}

void WebViewImpl::stop()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->Stop();
}

void WebViewImpl::focus()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->GetView()->Focus();
}

void WebViewImpl::show()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    if (!d_widget)
        createWidget(ui::GetHiddenWindow());
    d_widget->show();
}

void WebViewImpl::hide()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    if (!d_widget)
        createWidget(ui::GetHiddenWindow());
    d_widget->hide();
}

void WebViewImpl::setParent(NativeView parent)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);

    if (!parent)
        parent = ui::GetHiddenWindow();

    if (!d_widget)
        createWidget(parent);
    else
        d_widget->setParent(parent);
}

void WebViewImpl::move(int left, int top, int width, int height)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    if (!d_widget)
        createWidget(ui::GetHiddenWindow());
    d_widget->move(left, top, width, height);
}

void WebViewImpl::cutSelection()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->GetRenderViewHost()->Cut();
}

void WebViewImpl::copySelection()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->GetRenderViewHost()->Copy();
}

void WebViewImpl::paste()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->GetRenderViewHost()->Paste();
}

void WebViewImpl::deleteSelection()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->GetRenderViewHost()->Delete();
}

void WebViewImpl::enableFocusBefore(bool enabled)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_focusBeforeEnabled = enabled;
}

void WebViewImpl::enableFocusAfter(bool enabled)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_focusAfterEnabled = enabled;
}

void WebViewImpl::enableNCHitTest(bool enabled)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_ncHitTestEnabled = enabled;
    d_lastNCHitTestResult = HTCLIENT;
}

void WebViewImpl::onNCHitTestResult(int x, int y, int result)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    DCHECK(d_ncHitTestPendingAck);
    d_lastNCHitTestResult = result;
    d_ncHitTestPendingAck = false;

    // Re-request it if the mouse position has changed, so that we
    // always have the latest info.
    if (d_delegate && d_ncHitTestEnabled) {
        POINT ptNow;
        ::GetCursorPos(&ptNow);
        if (ptNow.x != x || ptNow.y != y) {
            d_ncHitTestPendingAck = true;
            d_delegate->requestNCHitTest(this);
        }
    }
}

void WebViewImpl::performCustomContextMenuAction(int actionId)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->GetRenderViewHost()->ExecuteCustomContextMenuCommand(actionId, d_customContext);
}

void WebViewImpl::enableAltDragRubberbanding(bool enabled)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_altDragRubberbandingEnabled = enabled;

#ifdef BB_RENDER_VIEW_HOST_SUPPORTS_RUBBERBANDING
    if (d_webContents->GetRenderViewHost()) {
        d_webContents->GetRenderViewHost()->EnableAltDragRubberbanding(enabled);
    }
#endif
}

void WebViewImpl::enableCustomTooltip(bool enabled)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_customTooltipEnabled = enabled;
}

void WebViewImpl::setZoomPercent(int value)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->GetRenderViewHost()->SetZoomLevel(
        WebKit::WebView::zoomFactorToZoomLevel((double)value/100));
}

void WebViewImpl::replaceMisspelledRange(const StringRef& text)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    base::string16 text16;
    UTF8ToUTF16(text.data(), text.length(), &text16);
    d_webContents->GetRenderViewHost()->ReplaceMisspelling(text16);
}

void WebViewImpl::rootWindowPositionChanged()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    content::RenderWidgetHostViewBase* rwhv =
        static_cast<content::RenderWidgetHostViewBase*>(
            d_webContents->GetRenderWidgetHostView());
    if (rwhv)
        rwhv->UpdateScreenInfo(rwhv->GetNativeView());
}

void WebViewImpl::rootWindowSettingsChanged()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    content::RenderWidgetHostViewBase* rwhv =
        static_cast<content::RenderWidgetHostViewBase*>(
            d_webContents->GetRenderWidgetHostView());
    if (rwhv)
        rwhv->UpdateScreenInfo(rwhv->GetNativeView());
}

void WebViewImpl::createWidget(blpwtk2::NativeView parent)
{
    DCHECK(!d_widget);
    DCHECK(!d_wasDestroyed);

    // This creates the HWND that will host the WebContents.  The widget
    // will be deleted when the HWND is destroyed.
    d_widget = new blpwtk2::NativeViewWidget(
        d_webContents->GetView()->GetNativeView(),
        parent,
        this);
}

void WebViewImpl::onDestroyed(NativeViewWidget* source)
{
    DCHECK(source == d_widget);
    d_widget = 0;
}

void WebViewImpl::UpdateTargetURL(content::WebContents* source,
                                  int32 page_id,
                                  const GURL& url)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source == d_webContents);
    if (d_wasDestroyed) return;
    if (d_delegate)
        d_delegate->updateTargetURL(this, url.spec());
}

void WebViewImpl::LoadingStateChanged(content::WebContents* source)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source == d_webContents);
    if (d_wasDestroyed) return;
    if (d_delegate) {
        WebViewDelegate::NavigationState state;
        state.canGoBack = source->GetController().CanGoBack();
        state.canGoForward = source->GetController().CanGoForward();
        state.isLoading = source->IsLoading();
        d_delegate->updateNavigationState(this, state);
    }
}

void WebViewImpl::DidNavigateMainFramePostCommit(content::WebContents* source)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source == d_webContents);
    d_isReadyForDelete = true;
    if (d_wasDestroyed) {
        if (!d_isDeletingSoon) {
            d_isDeletingSoon = true;
            base::MessageLoop::current()->DeleteSoon(FROM_HERE, this);
        }
        return;
    }
    if (d_delegate)
        d_delegate->didNavigateMainFramePostCommit(this, source->GetURL().spec());
}

bool WebViewImpl::TakeFocus(content::WebContents* source, bool reverse)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source == d_webContents);
    if (d_wasDestroyed || !d_delegate) return false;
    if (reverse) {
        if (d_focusBeforeEnabled) {
            d_delegate->focusBefore(this);
            return true;
        }
        return false;
    }
    if (d_focusAfterEnabled) {
        d_delegate->focusAfter(this);
        return true;
    }
    return false;
}

void WebViewImpl::WebContentsFocused(content::WebContents* contents)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(contents == d_webContents);
    if (d_wasDestroyed) return;
    if (d_delegate)
        d_delegate->focused(this);
}

void WebViewImpl::WebContentsBlurred(content::WebContents* contents)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(contents == d_webContents);
    if (d_wasDestroyed) return;
    if (d_delegate)
        d_delegate->blurred(this);
}

void WebViewImpl::WebContentsCreated(content::WebContents* source_contents,
                                     int64 source_frame_id,
                                     const string16& frame_name,
                                     const GURL& target_url,
                                     const content::ContentCreatedParams& params,
                                     content::WebContents* new_contents)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source_contents == d_webContents);
    WebViewImpl* newView = new WebViewImpl(new_contents,
                                           d_browserContext,
                                           d_takeFocusOnMouseDown,
                                           d_domPasteEnabled,
                                           d_javascriptCanAccessClipboard);
    if (d_wasDestroyed || !d_delegate) {
        newView->destroy();
        return;
    }

    NewViewParams delegateParams;
    switch (params.disposition) {
    case SAVE_TO_DISK:
        delegateParams.setDisposition(NewViewDisposition::DOWNLOAD);
        break;
    case CURRENT_TAB:
        delegateParams.setDisposition(NewViewDisposition::CURRENT_TAB);
        break;
    case NEW_BACKGROUND_TAB:
        delegateParams.setDisposition(NewViewDisposition::NEW_BACKGROUND_TAB);
        break;
    case NEW_FOREGROUND_TAB:
        delegateParams.setDisposition(NewViewDisposition::NEW_FOREGROUND_TAB);
        break;
    case NEW_POPUP:
        delegateParams.setDisposition(NewViewDisposition::NEW_POPUP);
        newView->d_isPopup = true;
        break;
    case NEW_WINDOW:
    default:
        delegateParams.setDisposition(NewViewDisposition::NEW_WINDOW);
        break;
    }
    if (params.x_set) delegateParams.setX(params.x);
    if (params.y_set) delegateParams.setY(params.y);
    if (params.width_set) delegateParams.setWidth(params.width);
    if (params.height_set) delegateParams.setHeight(params.height);
    delegateParams.setTargetUrl(target_url.spec());

    for (size_t i = 0; i < params.additional_features.size(); ++i) {
        delegateParams.addAdditionalFeature(params.additional_features[i]);
    }

    d_delegate->didCreateNewView(this,
                                 newView,
                                 delegateParams,
                                 &newView->d_delegate);

    // The new WebViewImpl doesn't receive these WebContentsObserver callbacks
    // in the WebContentsCreated() path, so let's invoke them manually.
    newView->RenderViewCreated(new_contents->GetRenderViewHost());
    newView->AboutToNavigateRenderView(new_contents->GetRenderViewHost());
}

void WebViewImpl::CloseContents(content::WebContents* source)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source == d_webContents);
    if (d_wasDestroyed) return;
    if (!d_delegate) {
        destroy();
        return;
    }

    d_delegate->destroyView(this);
}

void WebViewImpl::MoveContents(content::WebContents* source_contents, const gfx::Rect& pos)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source_contents == d_webContents);
    if (d_wasDestroyed) return;
    if (d_delegate) {
        d_delegate->moveView(this, pos.x(), pos.y(), pos.width(), pos.height());
    }
}

bool WebViewImpl::IsPopupOrPanel(const content::WebContents* source) const
{
    return d_isPopup;
}

bool WebViewImpl::OnNCHitTest(int* result)
{
    if (d_ncHitTestEnabled && d_delegate) {
        if (!d_ncHitTestPendingAck) {
            d_ncHitTestPendingAck = true;
            d_delegate->requestNCHitTest(this);
        }
        *result = d_lastNCHitTestResult;
        return true;
    }
    return false;
}

bool WebViewImpl::OnNCDragBegin(int hitTestCode)
{
    if (!d_ncHitTestEnabled || !d_delegate) {
        return false;
    }

    POINT screenPoint;
    switch (hitTestCode) {
    case HTCAPTION:
    case HTLEFT:
    case HTTOP:
    case HTRIGHT:
    case HTBOTTOM:
    case HTTOPLEFT:
    case HTTOPRIGHT:
    case HTBOTTOMRIGHT:
    case HTBOTTOMLEFT:
        ::GetCursorPos(&screenPoint);
        d_delegate->ncDragBegin(this, hitTestCode, screenPoint);
        return true;
    default:
        return false;
    }
}

void WebViewImpl::OnNCDragMove()
{
    if (d_delegate) {
        POINT screenPoint;
        ::GetCursorPos(&screenPoint);
        d_delegate->ncDragMove(this, screenPoint);
    }
}

void WebViewImpl::OnNCDragEnd()
{
    if (d_delegate) {
        POINT screenPoint;
        ::GetCursorPos(&screenPoint);
        d_delegate->ncDragEnd(this, screenPoint);
    }
}

void WebViewImpl::DidUpdateBackingStore()
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed || !d_implClient) return;
    d_implClient->didUpdatedBackingStore(
        d_webContents->GetRenderViewHost()->LastKnownRendererSize());
}

bool WebViewImpl::ShouldSetFocusOnMouseDown()
{
    DCHECK(Statics::isInBrowserMainThread());
    return d_takeFocusOnMouseDown;
}

bool WebViewImpl::ShowTooltip(content::WebContents* source_contents,
                              const string16& tooltip_text,
                              WebKit::WebTextDirection text_direction_hint)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source_contents == d_webContents);
    if (d_wasDestroyed) return false;
    if (!d_customTooltipEnabled) return false;
    if (d_delegate) {
        TextDirection::Value direction;
        switch (text_direction_hint) {
            case WebKit::WebTextDirectionLeftToRight:
                direction = TextDirection::LEFT_TO_RIGHT; break;
            case WebKit::WebTextDirectionRightToLeft:
                direction = TextDirection::RIGHT_TO_LEFT; break;
            default:
                direction = TextDirection::LEFT_TO_RIGHT;
        }
        String tooltipText(tooltip_text.c_str(), tooltip_text.length());
        d_delegate->showTooltip(this, tooltipText, direction);
        return true;
    }
    return false;
}

void WebViewImpl::FindReply(content::WebContents* source_contents,
                            int request_id,
                            int number_of_matches,
                            const gfx::Rect& selection_rect,
                            int active_match_ordinal,
                            bool final_update)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source_contents == d_webContents);
    DCHECK(d_implClient || d_find.get());

    if (d_wasDestroyed) return;

    if (d_implClient) {
        d_implClient->findStateWithReqId(request_id, number_of_matches,
                                         active_match_ordinal, final_update);
    }
    else if (d_delegate &&
        d_find->applyUpdate(request_id, number_of_matches, active_match_ordinal)) {
        d_delegate->findState(this,
                              d_find->numberOfMatches(),
                              d_find->activeMatchIndex(),
                              final_update);
    }
}

/////// WebContentsObserver overrides

void WebViewImpl::RenderViewCreated(content::RenderViewHost* render_view_host)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed || !d_implClient) return;
    if (d_implClient->shouldDisableBrowserSideResize()) {
        render_view_host->DisableBrowserSideResize();
    }
}

void WebViewImpl::AboutToNavigateRenderView(content::RenderViewHost* render_view_host)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed) return;
    if (d_implClient) {
        int routingId = render_view_host->GetRoutingID();
        d_implClient->aboutToNativateRenderView(routingId);
    }
#ifdef BB_RENDER_VIEW_HOST_SUPPORTS_RUBBERBANDING
    render_view_host->EnableAltDragRubberbanding(d_altDragRubberbandingEnabled);
#endif
}

void WebViewImpl::DidFinishLoad(int64 frame_id,
                                const GURL& validated_url,
                                bool is_main_frame,
                                content::RenderViewHost* render_view_host)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed || !d_delegate) return;

    // TODO: figure out what to do for iframes
    if (is_main_frame) {
        d_delegate->didFinishLoad(this, validated_url.spec());
    }
}

void WebViewImpl::DidFailLoad(int64 frame_id,
                              const GURL& validated_url,
                              bool is_main_frame,
                              int error_code,
                              const string16& error_description,
                              content::RenderViewHost* render_view_host)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed || !d_delegate) return;

    // TODO: figure out what to do for iframes
    if (is_main_frame) {
        d_delegate->didFailLoad(this, validated_url.spec());
    }
}

}  // close namespace blpwtk2

