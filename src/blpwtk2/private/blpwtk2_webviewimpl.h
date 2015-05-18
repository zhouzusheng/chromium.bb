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

#ifndef INCLUDED_BLPWTK2_WEBVIEWIMPL_H
#define INCLUDED_BLPWTK2_WEBVIEWIMPL_H

#include <blpwtk2_config.h>

#include <blpwtk2_findonpage.h>
#include <blpwtk2_nativeviewwidgetdelegate.h>
#include <blpwtk2_webview.h>
#include <blpwtk2_webviewproperties.h>

#include <content/public/browser/web_contents_delegate.h>
#include <content/public/browser/web_contents_observer.h>
#include <content/public/common/context_menu_params.h>
#include <content/public/common/file_chooser_params.h>
#include <ui/gfx/native_widget_types.h>
#include <third_party/WebKit/public/web/WebTextDirection.h>

namespace content {
    class WebContents;
    struct WebPreferences;
}  // close namespace content

namespace views {
    class Widget;
}  // close namespace views

namespace blpwtk2 {

class BrowserContextImpl;
class ContextMenuParams;
class DevToolsFrontendHostDelegateImpl;
class NativeViewWidget;
class WebViewDelegate;
class WebViewImplClient;

// This is the implementation of the blpwtk2::WebView interface.  It creates a
// content::WebContents object, and implements the content::WebContentsDelegate
// interface.  It forwards the content::WebContentsDelegate override events to
// the blpwtk2::WebViewDelegate, which is provided by the application.
//
// This class can only be instantiated from the browser-main thread.
//
// When we are using 'ThreadMode::ORIGINAL', this object is returned to the
// application by 'Toolkit::createWebView'.  When we are using
// 'ThreadMode::RENDERER_MAIN', this object is created on the secondary
// browser-main thread, and the application instead gets a 'WebViewProxy',
// which forwards events back and forth between the secondary browser-main
// thread and the application thread.  See blpwtk2_toolkit.h for an explanation
// about threads.
class WebViewImpl : public WebView,
                    private NativeViewWidgetDelegate,
                    private content::WebContentsDelegate,
                    private content::WebContentsObserver {
  public:
    WebViewImpl(WebViewDelegate* delegate,
                blpwtk2::NativeView parent,
                BrowserContextImpl* browserContext,
                int hostAffinity,
                bool initiallyVisible,
                const WebViewProperties& properties);
    WebViewImpl(content::WebContents* contents,
                BrowserContextImpl* browserContext,
                const WebViewProperties& properties);
    virtual ~WebViewImpl();

    void setImplClient(WebViewImplClient* client);
    gfx::NativeView getNativeView() const;
    void showContextMenu(const ContextMenuParams& params);
    void saveCustomContextMenuContext(
        content::RenderFrameHost* rfh,
        const content::CustomContextMenuContext& context);
    void handleFindRequest(const FindOnPageRequest& request);
    void handleExternalProtocol(const GURL& url);
    void overrideWebkitPrefs(content::WebPreferences* prefs);
    void onRenderViewHostMadeCurrent(content::RenderViewHost* renderViewHost);

    /////////////// WebView overrides

    void destroy() override;
    WebFrame* mainFrame() override;
    void loadUrl(const StringRef& url) override;
    void loadInspector(WebView* inspectedView) override;
    void inspectElementAt(const POINT& point) override;
    void reload(bool ignoreCache) override;
    void goBack() override;
    void goForward() override;
    void stop() override;
    void takeKeyboardFocus() override;
    void setLogicalFocus(bool focused) override;
    void show() override;
    void hide() override;
    void setParent(NativeView parent) override;
    void embedChild(NativeView child) override;
    void move(int left, int top, int width, int height) override;
    void cutSelection() override;
    void copySelection() override;
    void paste() override;
    void deleteSelection() override;
    void enableFocusBefore(bool enabled) override;
    void enableFocusAfter(bool enabled) override;
    void enableNCHitTest(bool enabled) override;
    void onNCHitTestResult(int x, int y, int result) override;
    void fileChooserCompleted(const StringRef* paths,
                              size_t numPaths) override;
    void performCustomContextMenuAction(int actionId) override;
    void enableAltDragRubberbanding(bool enabled) override;
    void enableCustomTooltip(bool enabled) override;
    void setZoomPercent(int value) override;
    void find(const StringRef& text, bool matchCase, bool forward) override;
    void replaceMisspelledRange(const StringRef& text) override;
    void rootWindowPositionChanged() override;
    void rootWindowSettingsChanged() override;
    void print() override;
    void handleInputEvents(const InputEvent *events, size_t eventsCount) override;
    void setDelegate(WebViewDelegate* delegate) override;
    void drawContents(const NativeRect &srcRegion,
                      const NativeRect &destRegion,
                      int dpiMultiplier,
                      const StringRef &styleClass,
                      NativeDeviceContext deviceContext) override;

  private:
    void createWidget(blpwtk2::NativeView parent);

    /////// NativeViewWidgetDelegate overrides

    void onDestroyed(NativeViewWidget* source) override;
    bool OnNCHitTest(int* result) override;
    bool OnNCDragBegin(int hitTestCode) override;
    void OnNCDragMove() override;
    void OnNCDragEnd() override;

    /////// WebContentsDelegate overrides

    // Notification that the target URL has changed.
    void UpdateTargetURL(content::WebContents* source,
                         const GURL& url) override;

    // Notifies the delegate that this contents is starting or is done loading
    // some resource. The delegate should use this notification to represent
    // loading feedback. See WebContents::IsLoading()
    void LoadingStateChanged(content::WebContents* source,
                             bool to_different_document) override;

    // Invoked when a main frame navigation occurs.
    void DidNavigateMainFramePostCommit(content::WebContents* source) override;

    // Called when a file selection is to be done.
    void RunFileChooser(content::WebContents* source,
                        const content::FileChooserParams& params) override;

    // This is called when WebKit tells us that it is done tabbing through
    // controls on the page. Provides a way for WebContentsDelegates to handle
    // this. Returns true if the delegate successfully handled it.
    bool TakeFocus(content::WebContents* source, bool reverse) override;

    // Notification that |contents| has gained focus.
    void WebContentsFocused(content::WebContents* contents) override;

    // Notification that |contents| has lost focus.
    void WebContentsBlurred(content::WebContents* contents) override;

    // Notifies the delegate about the creation of a new WebContents. This
    // typically happens when popups are created.
    void WebContentsCreated(content::WebContents* source_contents,
                            int opener_render_frame_id,
                            const base::string16& frame_name,
                            const GURL& target_url,
                            const content::ContentCreatedParams& params,
                            content::WebContents* new_contents) override;

    // Request the delegate to close this web contents, and do whatever cleanup
    // it needs to do.
    void CloseContents(content::WebContents* source) override;

    // Request the delegate to move this WebContents to the specified position
    // in screen coordinates.
    void MoveContents(content::WebContents* source, const gfx::Rect& pos) override;

    // Called to determine if the WebContents is contained in a popup window
    // or a panel window.
    bool IsPopupOrPanel(const content::WebContents* source) const override;

    // Asks permission to use the camera and/or microphone. If permission is
    // granted, a call should be made to |callback| with the devices. If the
    // request is denied, a call should be made to |callback| with an empty list
    // of devices. |request| has the details of the request (e.g. which of audio
    // and/or video devices are requested, and lists of available devices).
    void RequestMediaAccessPermission(
        content::WebContents* web_contents,
        const content::MediaStreamRequest& request,
        const content::MediaResponseCallback& callback) override;

    // Return true if the RWHV should take focus on mouse-down.
    bool ShouldSetKeyboardFocusOnMouseDown() override;
    bool ShouldSetLogicalFocusOnMouseDown() override;

    // Allows delegate to show a custom tooltip. If the delegate doesn't want a
    // custom tooltip, it should just return 'false'. Otherwise, it should show
    // the tooltip and return 'true'. By default, the delegate doesn't provide a
    // custom tooltip.
    bool ShowTooltip(content::WebContents* source,
                     const base::string16& tooltip_text,
                     blink::WebTextDirection text_direction_hint) override;

    // Information about current find request
    void FindReply(content::WebContents* source_contents,
                   int request_id,
                   int number_of_matches,
                   const gfx::Rect& selection_rect,
                   int active_match_ordinal,
                   bool final_update) override;

    /////// WebContentsObserver overrides

    // This is called when a RVH is created for a WebContents, but not if it's an
    // interstitial.
    void RenderViewCreated(content::RenderViewHost* render_view_host) override;

    // This method is invoked when a WebContents swaps its visible RenderViewHost
    // with another one, possibly changing processes. The RenderViewHost that has
    // been replaced is in |old_host|, which is NULL if the old RVH was shut down.
    void RenderViewHostChanged(content::RenderViewHost* old_host,
                               content::RenderViewHost* new_host) override;

    // This method is invoked when the navigation is done, i.e. the spinner of
    // the tab will stop spinning, and the onload event was dispatched.
    //
    // If the WebContents is displaying replacement content, e.g. network error
    // pages, DidFinishLoad is invoked for frames that were not sending
    // navigational events before. It is safe to ignore these events.
    void DidFinishLoad(
        content::RenderFrameHost* render_frame_host,
        const GURL& validated_url) override;

    // This method is like DidFinishLoad, but when the load failed or was
    // cancelled, e.g. window.stop() is invoked.
    void DidFailLoad(
        content::RenderFrameHost* render_frame_host,
        const GURL& validated_url,
        int error_code,
        const base::string16& error_description) override;

  private:
    scoped_ptr<DevToolsFrontendHostDelegateImpl> d_devToolsFrontEndHost;
    scoped_ptr<content::WebContents> d_webContents;
    scoped_ptr<FindOnPage> d_find;
    WebViewDelegate* d_delegate;
    WebViewImplClient* d_implClient;
    BrowserContextImpl* d_browserContext;
    NativeViewWidget* d_widget;  // owned by the views system
    WebViewProperties d_properties;  // TODO(SHEZ): move more properties into this struct
    bool d_focusBeforeEnabled;
    bool d_focusAfterEnabled;
    bool d_isReadyForDelete;  // when the underlying WebContents can be deleted
    bool d_wasDestroyed;      // if destroy() has been called
    bool d_isDeletingSoon;    // when DeleteSoon has been called
    bool d_isPopup;           // if this view is a popup view
    bool d_altDragRubberbandingEnabled;
    bool d_customTooltipEnabled;
    bool d_ncHitTestEnabled;
    bool d_ncHitTestPendingAck;
    int d_lastNCHitTestResult;
    content::FileChooserParams::Mode d_lastFileChooserMode;

    // For calling performCustomContextMenuAction().  TODO: clean this
    content::CustomContextMenuContext d_customContext;


    DISALLOW_COPY_AND_ASSIGN(WebViewImpl);
};

}  // close namespace blpwtk2

#endif  // INCLUDED_BLPWTK2_WEBVIEWIMPL_H

