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
#include <blpwtk2_filechooserparams.h>
#include <blpwtk2_filechooserparamsimpl.h>
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
#include <content/public/browser/host_zoom_map.h>
#include <content/public/browser/media_capture_devices.h>
#include <content/public/browser/render_frame_host.h>
#include <content/public/browser/render_view_host.h>
#include <content/public/browser/render_process_host.h>
#include <content/public/browser/web_contents.h>
#include <content/public/browser/site_instance.h>
#include <content/public/common/file_chooser_file_info.h>
#include <content/public/common/web_preferences.h>
#include <third_party/WebKit/public/web/WebFindOptions.h>
#include <third_party/WebKit/public/web/WebView.h>
#include <ui/base/win/hidden_window.h>

namespace blpwtk2 {

class DummyMediaStreamUI : public content::MediaStreamUI {
public:
    DummyMediaStreamUI() {}
    virtual ~DummyMediaStreamUI() {}

    gfx::NativeViewId OnStarted(const base::Closure& stop) override
    {
        return 0;
    }
};

static const content::MediaStreamDevice* findDeviceById(
    const std::string& id,
    const content::MediaStreamDevices& devices)
{
    for (std::size_t i = 0; i < devices.size(); ++i) {
        if (id == devices[i].id) {
            return &devices[i];
        }
    }
    return 0;
}

WebViewImpl::WebViewImpl(WebViewDelegate* delegate,
                         blpwtk2::NativeView parent,
                         BrowserContextImpl* browserContext,
                         int hostAffinity,
                         bool initiallyVisible,
                         const WebViewProperties& properties)
: d_delegate(delegate)
, d_implClient(0)
, d_browserContext(browserContext)
, d_widget(0)
, d_properties(properties)
, d_focusBeforeEnabled(false)
, d_focusAfterEnabled(false)
, d_isReadyForDelete(false)
, d_wasDestroyed(false)
, d_isDeletingSoon(false)
, d_isPopup(false)
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
                         const WebViewProperties& properties)
: d_delegate(0)
, d_implClient(0)
, d_browserContext(browserContext)
, d_widget(0)
, d_properties(properties)
, d_focusBeforeEnabled(false)
, d_focusAfterEnabled(false)
, d_isReadyForDelete(false)
, d_wasDestroyed(false)
, d_isDeletingSoon(false)
, d_isPopup(false)
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
    show();
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
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(client);
    DCHECK(!d_implClient);
    d_implClient = client;
    if (d_widget) {
        d_implClient->updateNativeViews(d_widget->getNativeWidgetView(), ui::GetHiddenWindow());
    }
}

gfx::NativeView WebViewImpl::getNativeView() const
{
    DCHECK(Statics::isInBrowserMainThread());
    return d_webContents->GetNativeView();
}

void WebViewImpl::showContextMenu(const ContextMenuParams& params)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed) return;
    if (d_delegate)
        d_delegate->showContextMenu(this, params);
}

void WebViewImpl::saveCustomContextMenuContext(
    content::RenderFrameHost* rfh,
    const content::CustomContextMenuContext& context)
{
    d_customContext = context;
}

void WebViewImpl::handleFindRequest(const FindOnPageRequest& request)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);

    if (!request.reqId) {
        d_webContents->StopFinding(content::STOP_FIND_ACTION_CLEAR_SELECTION);
        return;
    }
    blink::WebFindOptions options;
    options.findNext = request.findNext;
    options.forward = request.forward;
    options.matchCase = request.matchCase;
    blink::WebString textStr =
        blink::WebString::fromUTF8(request.text.data(),
                                   request.text.length());
    d_webContents->Find(request.reqId, textStr, options);
}

void WebViewImpl::handleExternalProtocol(const GURL& url)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed || !d_delegate) return;

    d_delegate->handleExternalProtocol(this, url.spec());
}

void WebViewImpl::overrideWebkitPrefs(content::WebPreferences* prefs)
{
    prefs->dom_paste_enabled = d_properties.domPasteEnabled;
    prefs->javascript_can_access_clipboard = d_properties.javascriptCanAccessClipboard;
}

void WebViewImpl::onRenderViewHostMadeCurrent(content::RenderViewHost* renderViewHost)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(renderViewHost);
    if (d_wasDestroyed) return;
    if (d_implClient) {
        int routingId = renderViewHost->GetRoutingID();
        d_implClient->gotNewRenderViewRoutingId(routingId);
    }
#ifdef BB_RENDER_VIEW_HOST_SUPPORTS_RUBBERBANDING
    renderViewHost->EnableAltDragRubberbanding(d_altDragRubberbandingEnabled);
#endif
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
        ui::PageTransitionFromInt(ui::PAGE_TRANSITION_TYPED | ui::PAGE_TRANSITION_FROM_ADDRESS_BAR),
        std::string());

    d_webContents->GetRenderViewHost()->GetView()->SetBackgroundColor(SK_ColorBLACK);
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

void WebViewImpl::drawContents(const NativeRect &srcRegion,
                               const NativeRect &destRegion,
                               int dpiMultiplier,
                               const StringRef &styleClass,
                               NativeDeviceContext deviceContext)
{
    NOTREACHED() << "drawContents() not supported in WebViewImpl";
}

void WebViewImpl::handleInputEvents(const InputEvent *events, size_t eventsCount)
{
    NOTREACHED() << "handleInputEvents() not supported in WebViewImpl";
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
        = content::DevToolsAgentHost::GetOrCreateFor(inspectedContents);

    d_devToolsFrontEndHost.reset(
        new DevToolsFrontendHostDelegateImpl(d_webContents.get(), agentHost));

    GURL url = Statics::devToolsHttpHandler->GetFrontendURL("/devtools/devtools.html");
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

void WebViewImpl::takeKeyboardFocus()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_widget->focus();
}

void WebViewImpl::setLogicalFocus(bool focused)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    if (focused) {
        d_webContents->Focus();
    }
    else {
        content::RenderWidgetHostViewBase* viewBase
            = static_cast<content::RenderWidgetHostViewBase*>(
                d_webContents->GetRenderWidgetHostView());
        viewBase->Blur();
    }
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

void WebViewImpl::embedChild(NativeView child)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);

    ::SetParent(child, d_widget->getNativeWidgetView());
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
    d_webContents->Cut();
}

void WebViewImpl::copySelection()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->Copy();
}

void WebViewImpl::paste()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->Paste();
}

void WebViewImpl::deleteSelection()
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->Delete();
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

void WebViewImpl::fileChooserCompleted(const StringRef* paths,
                                       size_t numPaths)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);

    std::vector<content::FileChooserFileInfo> files(numPaths);
    for (size_t i = 0; i < numPaths; ++i) {
        files[i].file_path = base::FilePath::FromUTF8Unsafe(paths[i].toStdString());
        files[i].display_name = files[i].file_path.BaseName().value();
    }

    d_webContents->GetRenderViewHost()->FilesSelectedInChooser(files, d_lastFileChooserMode);
}

void WebViewImpl::performCustomContextMenuAction(int actionId)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    d_webContents->ExecuteCustomContextMenuCommand(actionId, d_customContext);
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
    content::HostZoomMap::SetZoomLevel(
        d_webContents.get(),
        blink::WebView::zoomFactorToZoomLevel((double)value / 100));
}

void WebViewImpl::replaceMisspelledRange(const StringRef& text)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(!d_wasDestroyed);
    base::string16 text16;
    base::UTF8ToUTF16(text.data(), text.length(), &text16);
    d_webContents->ReplaceMisspelling(text16);
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

void WebViewImpl::setDelegate(blpwtk2::WebViewDelegate *delegate)
{
    DCHECK(Statics::isInBrowserMainThread());
    d_delegate = delegate;
}

void WebViewImpl::createWidget(blpwtk2::NativeView parent)
{
    DCHECK(!d_widget);
    DCHECK(!d_wasDestroyed);

    // This creates the HWND that will host the WebContents.  The widget
    // will be deleted when the HWND is destroyed.
    d_widget = new blpwtk2::NativeViewWidget(
        d_webContents->GetNativeView(),
        parent,
        this,
        d_properties.activateWindowOnMouseDown);

    if (d_implClient) {
        d_implClient->updateNativeViews(d_widget->getNativeWidgetView(), ui::GetHiddenWindow());
    }
}

void WebViewImpl::onDestroyed(NativeViewWidget* source)
{
    DCHECK(source == d_widget);
    d_widget = 0;
}

void WebViewImpl::UpdateTargetURL(content::WebContents* source,
                                  const GURL& url)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source == d_webContents);
    if (d_wasDestroyed) return;
    if (d_delegate)
        d_delegate->updateTargetURL(this, url.spec());
}

void WebViewImpl::LoadingStateChanged(content::WebContents* source,
                                      bool to_different_document)
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

void WebViewImpl::RunFileChooser(content::WebContents* source,
                                 const content::FileChooserParams& params)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source == d_webContents);

    if (!d_delegate) {
        return;
    }

    FileChooserParams wtk2Params;
    FileChooserParamsImpl* impl = *reinterpret_cast<FileChooserParamsImpl**>(&wtk2Params);

    switch (params.mode) {
    case content::FileChooserParams::Open:
        impl->d_mode = FileChooserParams::OPEN;
        break;
    case content::FileChooserParams::OpenMultiple:
        impl->d_mode = FileChooserParams::OPEN_MULTIPLE;
        break;
    case content::FileChooserParams::UploadFolder:
        impl->d_mode = FileChooserParams::UPLOAD_FOLDER;
        break;
    case content::FileChooserParams::Save:
        impl->d_mode = FileChooserParams::SAVE;
        break;
    default:
        NOTREACHED() << "Unsupported file chooser mode: " << params.mode;
        break;
    }
    impl->d_title = base::UTF16ToUTF8(params.title);
    impl->d_defaultFileName = params.default_file_name.AsUTF8Unsafe();
    impl->d_acceptTypes.resize(params.accept_types.size());
    for (size_t i = 0; i < params.accept_types.size(); ++i) {
        impl->d_acceptTypes[i] = base::UTF16ToUTF8(params.accept_types[i]);
    }

    d_lastFileChooserMode = params.mode;
    d_delegate->runFileChooser(this, wtk2Params);
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
                                     int opener_render_frame_id,
                                     const base::string16& frame_name,
                                     const GURL& target_url,
                                     const content::ContentCreatedParams& params,
                                     content::WebContents* new_contents)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source_contents == d_webContents);
    WebViewImpl* newView = new WebViewImpl(new_contents,
                                           d_browserContext,
                                           d_properties);
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
        delegateParams.addAdditionalFeature(
            base::UTF16ToUTF8(params.additional_features[i]));
    }

    d_delegate->didCreateNewView(this,
                                 newView,
                                 delegateParams,
                                 &newView->d_delegate);

    // The new WebViewImpl doesn't receive this WebContentsObserver callback
    // in the WebContentsCreated() path, so let's invoke it manually.
    newView->RenderViewCreated(new_contents->GetRenderViewHost());
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

void WebViewImpl::RequestMediaAccessPermission(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback)
{
    const content::MediaStreamDevices& audioDevices =
        content::MediaCaptureDevices::GetInstance()->GetAudioCaptureDevices();
    const content::MediaStreamDevices& videoDevices =
        content::MediaCaptureDevices::GetInstance()->GetVideoCaptureDevices();

    scoped_ptr<content::MediaStreamUI> ui(new DummyMediaStreamUI());
    content::MediaStreamDevices devices;
    if (request.requested_video_device_id.empty()) {
        if (request.video_type != content::MEDIA_NO_SERVICE && !videoDevices.empty()) {
            devices.push_back(videoDevices[0]);
        }
    }
    else {
        const content::MediaStreamDevice* device = findDeviceById(request.requested_video_device_id, videoDevices);
        if (device) {
            devices.push_back(*device);
        }
    }
    if (request.requested_audio_device_id.empty()) {
        if (request.audio_type != content::MEDIA_NO_SERVICE && !audioDevices.empty()) {
            devices.push_back(audioDevices[0]);
        }
    }
    else {
        const content::MediaStreamDevice* device = findDeviceById(request.requested_audio_device_id, audioDevices);
        if (device) {
            devices.push_back(*device);
        }
    }
    callback.Run(devices, content::MEDIA_DEVICE_OK, ui.Pass());
}

bool WebViewImpl::OnNCHitTest(int* result)
{
    if (d_ncHitTestEnabled && d_delegate) {
        if (!d_ncHitTestPendingAck) {
            d_ncHitTestPendingAck = true;
            d_delegate->requestNCHitTest(this);
        }

        // Windows treats HTBOTTOMRIGHT in a 'special' way when a child window
        // (i.e. this WebView's hwnd) overlaps with the bottom-right 3x3 corner
        // of the parent window.  In this case, subsequent messages like
        // WM_SETCURSOR and other WM_NC* messages get routed to the parent
        // window instead of the child window.
        // To work around this, we will lie to Windows when the app returns
        // HTBOTTOMRIGHT.  We'll return HTOBJECT instead.  AFAICT, HTOBJECT is
        // a completely unused hit-test code.  We'll forward HTOBJECT events to
        // the app as HTBOTTOMRIGHT (see further below).
        if (HTBOTTOMRIGHT == d_lastNCHitTestResult)
            *result = HTOBJECT;
        else
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

    // See explanation in 'OnNCHitTest' above.
    if (HTOBJECT == hitTestCode)
        hitTestCode = HTBOTTOMRIGHT;

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

bool WebViewImpl::ShouldSetKeyboardFocusOnMouseDown()
{
    DCHECK(Statics::isInBrowserMainThread());
    return d_properties.takeKeyboardFocusOnMouseDown;
}

bool WebViewImpl::ShouldSetLogicalFocusOnMouseDown()
{
    DCHECK(Statics::isInBrowserMainThread());
    return d_properties.takeLogicalFocusOnMouseDown;
}

bool WebViewImpl::ShowTooltip(content::WebContents* source_contents,
                              const base::string16& tooltip_text,
                              blink::WebTextDirection text_direction_hint)
{
    DCHECK(Statics::isInBrowserMainThread());
    DCHECK(source_contents == d_webContents);
    if (d_wasDestroyed) return false;
    if (!d_customTooltipEnabled) return false;
    if (d_delegate) {
        TextDirection::Value direction;
        switch (text_direction_hint) {
            case blink::WebTextDirectionLeftToRight:
                direction = TextDirection::LEFT_TO_RIGHT; break;
            case blink::WebTextDirectionRightToLeft:
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
    onRenderViewHostMadeCurrent(render_view_host);
}

void WebViewImpl::RenderViewHostChanged(content::RenderViewHost* old_host,
                                        content::RenderViewHost* new_host)
{
    onRenderViewHostMadeCurrent(new_host);
}

void WebViewImpl::DidFinishLoad(content::RenderFrameHost* render_frame_host,
                                const GURL& validated_url)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed || !d_delegate) return;

    // TODO: figure out what to do for iframes
    if (!render_frame_host->GetParent()) {
        d_delegate->didFinishLoad(this, validated_url.spec());
    }
}

void WebViewImpl::DidFailLoad(content::RenderFrameHost* render_frame_host,
                              const GURL& validated_url,
                              int error_code,
                              const base::string16& error_description)
{
    DCHECK(Statics::isInBrowserMainThread());
    if (d_wasDestroyed || !d_delegate) return;

    // TODO: figure out what to do for iframes
    if (!render_frame_host->GetParent()) {
        d_delegate->didFailLoad(this, validated_url.spec());
    }
}

}  // close namespace blpwtk2

