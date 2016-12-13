#pragma once
#include "minikit.h"

namespace pb {

	class BroswserHost {
	public:
		virtual minikit::Toolkit* GetMinikitToolkit() = 0;
		virtual minikit::Profile* GetMinikitProfile() = 0;
		virtual minikit::HttpServer* GetMinikiHTTPServer() = 0;
	};

	class BroswserApp {
	public:
		//TODO
		virtual minikit::String getAppName() = 0;
	};

	typedef BroswserApp* (*startapp)(BroswserHost*);
	typedef void (*stoptapp)(BroswserApp*);
}