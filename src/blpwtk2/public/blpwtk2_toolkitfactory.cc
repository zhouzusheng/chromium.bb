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

#include <blpwtk2_toolkitfactory.h>

#include <blpwtk2_products.h>
#include <blpwtk2_statics.h>
#include <blpwtk2_stringref.h>
#include <blpwtk2_toolkitcreateparams.h>
#include <blpwtk2_toolkitimpl.h>

#include <base/files/file_path.h>
#include <base/logging.h>  // for DCHECK
#include <base/strings/string16.h>
#include <base/strings/utf_string_conversions.h>
#include <content/public/common/dwrite_font_platform_win.h>
#include <content/public/renderer/render_font_warmup_win.h>
#include <content/renderer/render_frame_impl.h>
#include <net/http/http_network_session.h>
#include <net/socket/client_socket_pool_manager.h>
#include <printing/print_settings.h>
#include <ui/views/corewm/tooltip_win.h>

namespace blpwtk2 {

static bool g_created = false;
static ToolkitCreateParams::LogMessageHandler g_logMessageHandler = nullptr;
static ToolkitCreateParams::ConsoleLogMessageHandler g_consoleLogMessageHandler = nullptr;

static void setMaxSocketsPerProxy(int count)
{
    DCHECK(1 <= count);
    DCHECK(99 >= count);

    const net::HttpNetworkSession::SocketPoolType POOL =
        net::HttpNetworkSession::NORMAL_SOCKET_POOL;

    // The max per group can never exceed the max per proxy.  Use the default
    // max per group, unless count is less than the default.

    int prevMaxPerProxy =
        net::ClientSocketPoolManager::max_sockets_per_proxy_server(POOL);
    int newMaxPerGroup = std::min(count,
                                  (int)net::kDefaultMaxSocketsPerGroupNormal);

    if (newMaxPerGroup > prevMaxPerProxy) {
        net::ClientSocketPoolManager::set_max_sockets_per_proxy_server(
            POOL,
            count);
        net::ClientSocketPoolManager::set_max_sockets_per_group(
            POOL,
            newMaxPerGroup);
    }
    else {
        net::ClientSocketPoolManager::set_max_sockets_per_group(
            POOL,
            newMaxPerGroup);
        net::ClientSocketPoolManager::set_max_sockets_per_proxy_server(
            POOL,
            count);
    }
}

static ToolkitCreateParams::LogMessageSeverity decodeLogSeverity(int severity)
{
    switch (severity) {
    case logging::LOG_INFO:
        return ToolkitCreateParams::kSeverityInfo;
    case logging::LOG_WARNING:
        return ToolkitCreateParams::kSeverityWarning;
    case logging::LOG_ERROR:
        return ToolkitCreateParams::kSeverityError;
    case logging::LOG_FATAL:
        return ToolkitCreateParams::kSeverityFatal;
    default:
        return ToolkitCreateParams::kSeverityVerbose;
    }
}

static bool wtk2LogMessageHandlerFunction(int severity,
                                          const char* file,
                                          int line,
                                          size_t message_start,
                                          const std::string& str)
{
    g_logMessageHandler(decodeLogSeverity(severity), file, line, str.c_str() + message_start);
    return true;
}

static void wtk2ConsoleLogMessageHandlerFunction(int severity,
                                                 const std::string& file,
                                                 int line,
                                                 int column,
                                                 const std::string& message,
                                                 const std::string& stack_trace)
{
    g_consoleLogMessageHandler(decodeLogSeverity(severity),
                               StringRef(file.data(), file.length()),
                               line,
                               column,
                               StringRef(message.data(), message.length()),
                               StringRef(stack_trace.data(), stack_trace.length()));
}

// static
Toolkit* ToolkitFactory::create(const ToolkitCreateParams& params)
{
    DCHECK(!g_created);
    DCHECK(!ToolkitImpl::instance());

    Statics::initApplicationMainThread();
    Statics::threadMode = params.threadMode();
    Statics::pumpMode = params.pumpMode();
    Statics::inProcessResourceLoader = params.inProcessResourceLoader();
    Statics::isInProcessRendererDisabled = params.isInProcessRendererDisabled();
    Statics::channelErrorHandler = params.channelErrorHandler();
    content::DisableDWriteFactoryPatching();

    g_logMessageHandler = params.logMessageHandler();
    if (g_logMessageHandler) {
        logging::SetWtk2LogMessageHandler(wtk2LogMessageHandlerFunction);
    }

    g_consoleLogMessageHandler = params.consoleLogMessageHandler();
    if (g_consoleLogMessageHandler) {
        content::RenderFrameImpl::SetConsoleLogMessageHandler(wtk2ConsoleLogMessageHandlerFunction);
    }

    views::corewm::TooltipWin::SetTooltipStyle(params.tooltipFont());

    DCHECK(!Statics::inProcessResourceLoader ||
            Statics::isRendererMainThreadMode());

    if (params.isMaxSocketsPerProxySet()) {
        setMaxSocketsPerProxy(params.maxSocketsPerProxy());
    }

    ToolkitImpl* toolkit = new ToolkitImpl(params.dictionaryPath(),
                                           params.hostChannel());

    for (size_t i = 0; i < params.numCommandLineSwitches(); ++i) {
        StringRef switchRef = params.commandLineSwitchAt(i);
        std::string switchString(switchRef.data(), switchRef.length());
        toolkit->appendCommandLineSwitch(switchString.c_str());
    }

    for (size_t i = 0; i < params.numSideLoadedFonts(); ++i) {
        StringRef fontFileRef = params.sideLoadedFontAt(i);
        base::FilePath filePath = base::FilePath::FromUTF8Unsafe(
            std::string(fontFileRef.data(), fontFileRef.length()));
        content::AddCustomFontFile(filePath);
    }

    std::string html(params.headerFooterHTMLContent().data(),
                     params.headerFooterHTMLContent().length());
    printing::PrintSettings::SetDefaultPrinterSettings(
        base::UTF8ToUTF16(html), params.isPrintBackgroundGraphicsEnabled());

    g_created = true;

    if (!params.hostChannel().isEmpty()) {
        // Eagerly startup threads so that we connect the host channel when
        // the toolkit is initialized, instead of waiting for the first
        // profile/webview to be created.
        toolkit->startupThreads();
    }

    return toolkit;
}

}  // close namespace blpwtk2

