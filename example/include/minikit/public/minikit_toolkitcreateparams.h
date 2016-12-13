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

#ifndef INCLUDED_MINIKIT_TOOLKITCREATEPARAMS_H
#define INCLUDED_MINIKIT_TOOLKITCREATEPARAMS_H

#include <minikit_config.h>

#include <minikit_pumpmode.h>
#include <minikit_threadmode.h>

#include <stdlib.h>  // for _invalid_parameter_handler and _purecall_handler

namespace minikit {

class ResourceLoader;
class StringRef;
struct ToolkitCreateParamsImpl;
class RendererCallback;

// This class contains parameters that are passed to minikit when initializing
// the toolkit.
class ToolkitCreateParams {
  public:
    enum LogMessageSeverity {
        kSeverityVerbose = 0,
        kSeverityInfo = 1,
        kSeverityWarning = 2,
        kSeverityError = 3,
        kSeverityFatal = 4,
    };

    // The callback function that will be invoked whenever a log message
    // happens.
    typedef void(*LogMessageHandler)(LogMessageSeverity severity,
                                     const char* file,
                                     int line,
                                     const char* message);

    // The callback function that will be invoked whenever a log message
    // is printed to the Web Console
    typedef void(*ConsoleLogMessageHandler)(LogMessageSeverity severity,
                                            const StringRef& file,
                                            unsigned line,
                                            unsigned column,
                                            const StringRef& message,
                                            const StringRef& stack_trace);

    // The callback function that will be invoked whenever SEH exceptions
    // are caught in win procs.
    typedef int(__cdecl *WinProcExceptionFilter)(EXCEPTION_POINTERS* info);

    // The callback function that will be invoked whenever a channel error
    // happens.
    typedef void(*ChannelErrorHandler)(int reserved);

    MINIKIT_EXPORT ToolkitCreateParams();
    MINIKIT_EXPORT ToolkitCreateParams(const ToolkitCreateParams&);
    MINIKIT_EXPORT ~ToolkitCreateParams();
    MINIKIT_EXPORT ToolkitCreateParams& operator=(const ToolkitCreateParams&);

    // By default, minikit uses 'ThreadMode::ORIGINAL'.  Use this method to
    // change the thread mode.
    MINIKIT_EXPORT void setThreadMode(ThreadMode::Value mode);

    // By default, minikit uses 'PumpMode::MANUAL'.  Use this method to change
    // the pump mode.
    MINIKIT_EXPORT void setPumpMode(PumpMode::Value mode);

    // This only has any effect in the MANUAL pump mode.  By default, minikit
    // allows work messages to be posted to the main thread while it is doing
    // work.  This function can be used to disable that.  In this case, the
    // work message will be posted after doing work, if there is still more
    // work to be done.
    MINIKIT_EXPORT void disableWorkMessageWhileDoingWork();

    // By default, initiating a document print will cause the browser to open
    // a print dialog to ask for the target printing device.  Calling this
    // will disable the print dialog and use the default printing device on
    // the system.
    MINIKIT_EXPORT void enableDefaultPrintSettings();

    // By default, log messages go to a "minikit.log" file and to debug output.
    // Use this method to install a custom log message handler instead.  Note
    // that the handler callback can be invoked from any thread.
    MINIKIT_EXPORT void setLogMessageHandler(LogMessageHandler handler);

    // Use this method to install a custom log message handler for the
    // Web Console. This handler is only used for in-process renderers.
    MINIKIT_EXPORT void setConsoleLogMessageHandler(ConsoleLogMessageHandler handler);

    // Use this method to install a custom filter that will be invoked whenever
    // SEH exceptions happen inside win procs.
    MINIKIT_EXPORT void setWinProcExceptionFilter(WinProcExceptionFilter filter);

    // Use this method to install a channel error handler.
    MINIKIT_EXPORT void setChannelErrorHandler(ChannelErrorHandler handler);

    // By default, the in-process renderer is enabled.  This uses some
    // additional resources, even if in-process WebViews are not created.  Call
    // this method to disable the in-process renderer completely.  It is then
    // undefined behavior to create WebViews using 'IN_PROCESS_RENDERER'.
    MINIKIT_EXPORT void disableInProcessRenderer();

    // Set the maximum number of sockets per proxy, up to a maximum of 99.
    // Note that each Profile maintains its own pool of connections, so this is
    // actually the maximum number of sockets per proxy *per profile*.  The
    // behavior is undefined if 'count' is less than 1, or more than 99.
    MINIKIT_EXPORT void setMaxSocketsPerProxy(int count);

    // Set the threshold in which the mousemove and mousewheel events will be throttled
    // by the compositor.  If the time it takes for the events to be handled is longer
    // than this threshold, then the events will be throttled.  Note that this only
    // affects in-process renderers.
    MINIKIT_EXPORT void setInputHandlingTimeThrottlingThresholdMicroseconds(int us);

    // Add the specified 'switchString' to the list of command-line switches.
    // A list of switches can be found at:
    // http://peter.sh/experiments/chromium-command-line-switches/
    // Note, however, that minikit is based on a different version of chromium,
    // so it may not support *all* the switches mentioned on that page.
    MINIKIT_EXPORT void appendCommandLineSwitch(const StringRef& switchString);

    // Register the specified 'fontFile' to be side-loaded so that it is usable
    // by the in-process renderer.  The DirectWrite font implementation only
    // has access to %WINDIR%\Fonts by default.  This function allows
    // applications to load additional fonts.
    // Note that right now, this only works for in-process renderers.
    MINIKIT_EXPORT void appendSideLoadedFontInProcess(const StringRef& fontFile);

    // By default, minikit will automatically load plugins it finds on the
    // system (e.g. from paths in the Windows registry).  Use this method to
    // disable this behavior.  If it is disabled, then only plugins registered
    // via 'registerPlugin' will be loaded.
    MINIKIT_EXPORT void disablePluginDiscovery();

    // Install a custom ResourceLoader.  Note that this is only valid when
    // using the 'RENDERER_MAIN' thread-mode, and will only be used for
    // in-process renderers.
    MINIKIT_EXPORT void setInProcessResourceLoader(ResourceLoader*);

    // By default, minikit will look for .bdic files in the application's
    // working directory.  Use this method to change the path where minikit
    // would look for the .bdic files.  Note that this is only used if
    // spellchecking is enabled in one of the Profile objects.
    MINIKIT_EXPORT void setDictionaryPath(const StringRef& path);

    // By default, the user agent string will be generated based on the current
    // version number.  Use this method to override the default user agent
    // string.
    MINIKIT_EXPORT void setUserAgent(const StringRef& userAgent);

    // Set the CRT's invalid parameter handler.  If this is not set, then a
    // default handler will be installed, which sets a breakpoint and exits
    // the application.
    MINIKIT_EXPORT void setInvalidParameterHandler(_invalid_parameter_handler handler);

    // Set the CRT's purecall handler.  If this is not set, then a default
    // handler will be installed, which sets a breakpoint and exits the
    // application.
    MINIKIT_EXPORT void setPurecallHandler(_purecall_handler handler);

    // By default, minikit will allocate new browser process resources for each
    // minikit process.  However, the 'Toolkit::createHostChannel' method can
    // be used to setup an IPC channel that this process can use to share the
    // same browser process resources.  Use this method to set the channel-info
    // that will be used to connect this process to the browser process.  This
    // channel-info must have been obtained from 'Toolkit::createHostChannel'
    // in another process using the same version of minikit (i.e.
    // 'isValidHostChannelVersion' must return true).
    MINIKIT_EXPORT void setHostChannel(const StringRef& channelInfoString);

    // Return true if the specified 'channelInfoString' was obtained from a
    // process using the same version of minikit, and false otherwise.  It is
    // undefined behavior to use a channel-info obtained from a different
    // version of minikit.
    MINIKIT_EXPORT static bool isValidHostChannelVersion(
        const StringRef& channelInfoString);

    MINIKIT_EXPORT void setTooltipStyle(NativeFont font);

    // Set the highlight color of the active item in text searches.  The default
    // color is orange.  Note that this only works for in-process renderers
    // currently.
    MINIKIT_EXPORT void setActiveTextSearchHighlightColor(NativeColor color);

    // Set the highlight color for inactive items in text searches.  The default
    // color is yellow.  Note that this only works for in-process renderers
    // currently.
    MINIKIT_EXPORT void setInactiveTextSearchHighlightColor(NativeColor color);

    // This method is used to set the HTML file used to format header and
    // footer of printed pages.
    MINIKIT_EXPORT void setHeaderFooterHTML(const StringRef& htmlContent);

    // This method enables printing background graphics.
    MINIKIT_EXPORT void enablePrintBackgroundGraphics();

    // Set the name of the module that subprocesses will load.  By default,
    // subprocesses load the minikit dll.  The module specified here must
    // export the SubProcessMain symbol.
    MINIKIT_EXPORT void setSubProcessModule(const StringRef& moduleName);

	MINIKIT_EXPORT void  setRendererCallback(RendererCallback* callback);

    // ACCESSORS
    ThreadMode::Value threadMode() const;
    PumpMode::Value pumpMode() const;
    bool workMessageWhileDoingWorkDisabled() const;
    bool useDefaultPrintSettings() const;
    LogMessageHandler logMessageHandler() const;
    ConsoleLogMessageHandler consoleLogMessageHandler() const;
    WinProcExceptionFilter winProcExceptionFilter() const;
    ChannelErrorHandler channelErrorHandler() const;
    bool isInProcessRendererDisabled() const;
    bool isMaxSocketsPerProxySet() const;
    int maxSocketsPerProxy() const;
    bool isInputHandlingTimeThrottlingThresholdMicrosecondsSet() const;
    int inputHandlingTimeThrottlingThresholdMicroseconds() const;
    size_t numCommandLineSwitches() const;
    StringRef commandLineSwitchAt(size_t index) const;
    size_t numSideLoadedFonts() const;
    StringRef sideLoadedFontAt(size_t index) const;
    ResourceLoader* inProcessResourceLoader() const;
    StringRef dictionaryPath() const;
    StringRef userAgent() const;
    _invalid_parameter_handler invalidParameterHandler() const;
    _purecall_handler purecallHandler() const;
    StringRef hostChannel() const;
    NativeFont tooltipFont() const;
    NativeColor activeTextSearchHighlightColor() const;
    NativeColor inactiveTextSearchHighlightColor() const;
    StringRef headerFooterHTMLContent() const;
    bool isPrintBackgroundGraphicsEnabled() const;
    StringRef subProcessModule() const;
	RendererCallback* rendererCallback() const;
  private:
    ToolkitCreateParamsImpl* d_impl;
};

}  // close namespace minikit

#endif  // INCLUDED_MINIKIT_TOOLKITCREATEPARAMS_H

