// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200

#include <atldef.h>
#if ( _ATL_VER < 0x0800 )
#define _WTL_SUPPORT_SDK_ATL3 // Support of VC++ Express 2005 and ATL 3.0
#endif

// Support for VS2005 Express & SDK ATL
#ifdef _WTL_SUPPORT_SDK_ATL3
  #define _CRT_SECURE_NO_DEPRECATE
  #define _CRT_NON_CONFORMING_SWPRINTFS
  #pragma conform(forScope, off)
  #pragma comment(linker, "/NODEFAULTLIB:atlthunk.lib")  
#endif // _WTL_SUPPORT_SDK_ATL3

#include <atlbase.h>

// Support for VS2005 Express & SDK ATL
#ifdef _WTL_SUPPORT_SDK_ATL3
  namespace ATL
  {
	inline void * __stdcall __AllocStdCallThunk()
	{
		return ::HeapAlloc(::GetProcessHeap(), 0, sizeof(_stdcallthunk));
	}

	inline void __stdcall __FreeStdCallThunk(void *p)
	{
		::HeapFree(::GetProcessHeap(), 0, p);
	}
  };
#endif // _WTL_SUPPORT_SDK_ATL3

#include <atlapp.h>
extern CAppModule _Module;
#include <atlwin.h>

// CString-related includes
#define _WTL_USE_CSTRING
#include <atlmisc.h>


#if _MSC_VER<1400
#define WCSNCPY_S(strDest, sizeInBytes, strSource, count) wcsncpy(strDest, strSource, count)
#define STRCPY_S(strDestination, numberOfElements, strSource) strcpy(strDestination, strSource)
#else
#define WCSNCPY_S(strDest, sizeInBytes, strSource, count) wcsncpy_s(strDest, sizeInBytes, strSource, count)
#define STRCPY_S(strDestination, numberOfElements, strSource) strcpy_s(strDestination, numberOfElements, strSource)
#endif



