#pragma once

#include "../duilib/UIlib.h"
#include "minikit.h"
class FileDialogRunner
{
public:
	FileDialogRunner() {}
	~FileDialogRunner() {}
	void Run(minikit::WebView* source, const minikit::FileChooserParams& params);
};

