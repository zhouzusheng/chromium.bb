#pragma once
class TextResult {
public:
	virtual void free() = 0;
	virtual int  count() = 0;
	virtual const char*  item(int index) = 0;
	virtual const long   itemFlags(int index) = 0;
	virtual const long    itemOffset(int index) = 0;
	virtual const long    itemLength(int index) = 0;

};
extern "C" __declspec(dllimport) TextResult* __stdcall text_segment(const char* text, int len, const char* enc_name, int enclen, int flags);
