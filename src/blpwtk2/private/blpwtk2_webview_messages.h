/*
 * Copyright (C) 2014 Bloomberg Finance L.P.
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

// IPC messages for WebView.
// Multiply-included file, hence no include guard.

#include <blpwtk2_config.h>  // for NativeViewForTransit

#include <blpwtk2_contextmenuparams.h>
#include <blpwtk2_findonpage.h>
#include <blpwtk2_ipcparamtraits.h>
#include <blpwtk2_newviewparams.h>
#include <blpwtk2_textdirection.h>

#include <content/public/common/common_param_traits.h>
#include <ipc/ipc_message_macros.h>

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT

#define IPC_MESSAGE_START BlpWebViewMsgStart

IPC_STRUCT_BEGIN(BlpWebViewHostMsg_NewParams)
    IPC_STRUCT_MEMBER(int, routingId)
    IPC_STRUCT_MEMBER(int, profileId)
    IPC_STRUCT_MEMBER(bool, initiallyVisible)
    IPC_STRUCT_MEMBER(bool, takeFocusOnMouseDown)
    IPC_STRUCT_MEMBER(bool, domPasteEnabled)
    IPC_STRUCT_MEMBER(bool, javascriptCanAccessClipboard)
    IPC_STRUCT_MEMBER(int, rendererAffinity)
    IPC_STRUCT_MEMBER(blpwtk2::NativeViewForTransit, parent)
IPC_STRUCT_END()

// ============== Messages from client to host ======================

// This creates a new WebView.
IPC_MESSAGE_CONTROL1(BlpWebViewHostMsg_New,
                     BlpWebViewHostMsg_NewParams /* params */)

// See the WebView interface for an explanation of all these methods.
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_LoadUrl,
                    std::string /* url */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_LoadInspector,
                    int /* inspectedViewId */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_InspectElementAt,
                    gfx::Point /* point */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_Reload,
                    bool /* ignoreCache*/)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_GoBack)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_GoForward)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_Stop)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_Focus)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_Show)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_Hide)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_SetParent,
                    blpwtk2::NativeViewForTransit /* parent */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_Move,
                    gfx::Rect /* rect */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_SyncMove,
                    gfx::Rect /* rect */)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_CutSelection)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_CopySelection)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_Paste)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_DeleteSelection)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_EnableFocusBefore,
                    bool /* enabled */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_EnableFocusAfter,
                    bool /* enabled */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_EnableNCHitTest,
                    bool /* enabled */)
IPC_MESSAGE_ROUTED3(BlpWebViewHostMsg_OnNCHitTestResult,
                    int /* x */,
                    int /* y */,
                    int /* result */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_NCDragMoveAck,
                    gfx::Point /* movePoint */)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_NCDragEndAck)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_PerformContextMenuAction,
                    int /* actionId */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_EnableAltDragRubberbanding,
                    bool /* enabled */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_EnableCustomTooltip,
                    bool /* enabled */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_SetZoomPercent,
                    int /* value */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_Find,
                    blpwtk2::FindOnPageRequest /* request */)
IPC_MESSAGE_ROUTED1(BlpWebViewHostMsg_ReplaceMisspelledRange,
                    std::string /* text */)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_RootWindowPositionChanged)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_RootWindowSettingsChanged)
IPC_MESSAGE_ROUTED0(BlpWebViewHostMsg_Print)

// This destroys the WebView.
IPC_MESSAGE_CONTROL1(BlpWebViewHostMsg_Destroy,
                     int /* routingId */)


// ============== Messages from host to client ======================

IPC_MESSAGE_ROUTED1(BlpWebViewMsg_UpdateTargetURL,
                    std::string /* url */)
IPC_MESSAGE_ROUTED3(BlpWebViewMsg_UpdateNavigationState,
                    bool /* canGoBack */,
                    bool /* canGoForward */,
                    bool /* isLoading */)
IPC_MESSAGE_ROUTED1(BlpWebViewMsg_DidNavigateMainFramePostCommit,
                    std::string /* url */)
IPC_MESSAGE_ROUTED1(BlpWebViewMsg_DidFinishLoad,
                    std::string /* url */)
IPC_MESSAGE_ROUTED1(BlpWebViewMsg_DidFailLoad,
                    std::string /* url */)
IPC_MESSAGE_ROUTED2(BlpWebViewMsg_DidCreateNewView,
                    int /* routingId */,
                    blpwtk2::NewViewParams /* params */)
IPC_MESSAGE_ROUTED0(BlpWebViewMsg_DestroyView)
IPC_MESSAGE_ROUTED0(BlpWebViewMsg_FocusBefore)
IPC_MESSAGE_ROUTED0(BlpWebViewMsg_FocusAfter)
IPC_MESSAGE_ROUTED0(BlpWebViewMsg_Focused)
IPC_MESSAGE_ROUTED0(BlpWebViewMsg_Blurred)
IPC_MESSAGE_ROUTED1(BlpWebViewMsg_ShowContextMenu,
                    blpwtk2::ContextMenuParams /* params */)
IPC_MESSAGE_ROUTED1(BlpWebViewMsg_HandleExternalProtocol,
                    std::string /* url */)
IPC_MESSAGE_ROUTED1(BlpWebViewMsg_MoveView,
                    gfx::Rect /* rect */)
IPC_MESSAGE_ROUTED0(BlpWebViewMsg_RequestNCHitTest)
IPC_MESSAGE_ROUTED2(BlpWebViewMsg_NCDragBegin,
                    int /* hitTestCode */,
                    gfx::Point /* startPoint */)
IPC_MESSAGE_ROUTED0(BlpWebViewMsg_NCDragMove)
IPC_MESSAGE_ROUTED1(BlpWebViewMsg_NCDragEnd,
                    gfx::Point /* endPoint */)
IPC_MESSAGE_ROUTED2(BlpWebViewMsg_ShowTooltip,
                    std::string /* tooltipText */,
                    blpwtk2::TextDirection::Value /* direction */)
IPC_MESSAGE_ROUTED4(BlpWebViewMsg_FindState,
                    int /* reqId */,
                    int /* numberOfMatches */,
                    int /* activeMatchOrdinal */,
                    bool /* finalUpdate */)
IPC_MESSAGE_ROUTED0(BlpWebViewMsg_MoveAck)
IPC_MESSAGE_ROUTED1(BlpWebViewMsg_AboutToNavigateRenderView,
                    int /* rendererRoutingId */)

