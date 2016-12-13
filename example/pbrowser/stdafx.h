// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include<algorithm>
#include<typeinfo>
using namespace std;

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>

template< int t_nBufferLength = 128 >
class CW2UTF8EX
{
public:
	CW2UTF8EX(LPCWSTR psz) throw(...) :
		m_psz(m_szBuffer)
	{
		Init(psz, CP_UTF8);
	}
	~CW2UTF8EX() throw()
	{
		if (m_psz != m_szBuffer)
		{
			free(m_psz);
		}
	}

	operator LPSTR() const throw()
	{
		return(m_psz);
	}

private:
	void Init(LPCWSTR psz, UINT nCodePage) throw(...)
	{
		if (psz == NULL)
		{
			m_psz = NULL;
			return;
		}
		int nLengthW = lstrlenW(psz) + 1;
		int nLengthA = nLengthW * 3;

		if (nLengthA > t_nBufferLength)
		{
			m_psz = static_cast< LPSTR >(malloc(nLengthA*sizeof(char)));
			if (m_psz == NULL)
			{
				//AtlThrow( E_OUTOFMEMORY );
			}
		}

		if (::WideCharToMultiByte(nCodePage, 0, psz, nLengthW, m_psz, nLengthA, NULL, NULL) == 0)
		{
			//AtlThrowLastWin32();
		}
	}

public:
	LPSTR m_psz;
	char m_szBuffer[t_nBufferLength];

private:
	CW2UTF8EX(const CW2UTF8EX&) throw();
	CW2UTF8EX& operator=(const CW2UTF8EX&) throw();
};
typedef CW2UTF8EX<> CW2UTF8;