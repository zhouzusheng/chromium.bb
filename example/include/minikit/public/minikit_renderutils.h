/*
 * Copyright (C) 2016 Bloomberg Finance L.P.
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

#ifndef INCLUDED_MINIKIT_RENDERUTILS_H
#define INCLUDED_MINIKIT_RENDERUTILS_H

#include <minikit_config.h>

namespace v8 {
	class Extension;
}

namespace minikit {

class BaseRef
{
public:
	virtual int AddRef() = 0;
	virtual int Release() = 0;
};

class  RendererWindow
{
public:
	virtual long getId() = 0;
	virtual void SetURL(const char* url) = 0;
	virtual const char* GetURL(int& size) = 0;
	virtual void ExecuteJs(const char* code, const char* url) = 0;
	virtual void SendJsonMsg(const char* msg) = 0;
	virtual void SendMsg(int code, const char* msg, const char* args) = 0;
	virtual void SetUserData(const char* name, void* data) = 0;
	virtual void* GetUserData(const char* name) = 0;
};

class RendererListener : public BaseRef
{
public:
	virtual void OnScriptContextStart(RendererWindow* win, void* scriptContext) = 0;
	virtual void OnScriptContextEnd(RendererWindow* win, void* scriptContext) = 0;
	virtual void OnRelease(RendererWindow* win) = 0;
	virtual void OnViewMsg(RendererWindow* win, int code, const char* cmd, const char* args) = 0;
	virtual bool ShouldCreatePlugin(RendererWindow* win, const char* url, const char* mimeType, const char* pluginName) = 0;
};

class _CommandLine {
public:
	virtual bool HasSwitch(const char* name) = 0;
	virtual char* GetSwitchValue(const char* name) = 0;
	virtual void AppendSwitch(const char* name) = 0;
	virtual void AppendSwitchValue(const char* name, const char* value) = 0;
	virtual void AppendSwitchValue(const char* name, const wchar_t* value) = 0;

};

class Renderer_Context {
public:
	virtual _CommandLine* GetCommandLine() = 0;
	virtual void Free(void* data) = 0;

	virtual void AddListener(RendererListener* listener) = 0;
	virtual void RemoveListener(RendererListener* listener) = 0;
	virtual void RegisterExtension(v8::Extension* extension) = 0;
	virtual RendererWindow* GetScriptWindow() = 0;
};

class RendererCallback
{
public:
	virtual void OnContextCreate(Renderer_Context* ctx) = 0;
	virtual void OnContextDestroy(Renderer_Context* ctx) = 0;
	virtual void OnPreStartup(Renderer_Context* ctx) = 0;
	virtual void OnPostStartup(Renderer_Context* ctx) = 0;
	virtual void OnRenderStartup(Renderer_Context* ctx) = 0;
	virtual void OnPreShutdown(Renderer_Context* ctx) = 0;
	virtual bool OnBeforeMessage(const MSG& msg) = 0;
};

}  // close namespace minikit
#endif  // INCLUDED_MINIKIT_PDFUTIL_H

