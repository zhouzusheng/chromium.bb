#ifndef INCLUDED_MINIKIT_HTTPSERVER_H
#define INCLUDED_MINIKIT_HTTPSERVER_H

#include <minikit_config.h>
#include <minikit_stringref.h>

namespace minikit {

	class HttRequestInfo {
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
		virtual StringRef GetHeaderValue(const StringRef& header_name) = 0;
		virtual int GetHeaderCount() = 0;
		virtual  StringRef GetHeaderValue(int index) = 0;
		virtual StringRef GetPath() = 0;
		virtual StringRef GetMethod() = 0;
		virtual StringRef GetData() = 0;
	};

	class HttResponseInfo {
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
		virtual void AddHeader(const StringRef& name, const StringRef& value) = 0;
		virtual void SetData(const StringRef& data, const StringRef& type) = 0;
		virtual void AddData(const StringRef& data) = 0;
	};

	class HttpServerDelegate {
	public:
		virtual void OnHttpRequest(int connection_id, HttRequestInfo* info) = 0;
		virtual void OnWebSocketRequest(int connection_id, HttRequestInfo* info) = 0;
		virtual void OnWebSocketMessage(int connection_id, const char* data, int len) = 0;
		virtual void OnClose(int connection_id) = 0;
	};

	class Task {
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
		virtual void Run() = 0;
	};

	class TaskContainer {
		virtual void runTask(Task* task) = 0;
		virtual void runTaskWithThread(Task* task, int threadId) = 0;
	};
	
	class HttpServer {
	public:
		HttpServer(){}
		virtual ~HttpServer(){}
		virtual void destroy() = 0;
		virtual bool start(int port, int theadCount) = 0;
		virtual void shutdown() = 0;
		virtual void sendResponse(int id, HttResponseInfo* response) = 0;
		virtual void sendWebSocketResponse(int connection_id, const StringRef& data) = 0;
		virtual void closeChannel(int id) = 0;
		virtual HttResponseInfo* makeResponse(int code) = 0;
		virtual TaskContainer* taskContainer() = 0;
	};
}

#endif