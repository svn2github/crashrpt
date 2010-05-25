// stdafx.cpp : source file that includes just the standard includes
// CrashRptProbe.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

int _STPRINTF_S(TCHAR* buffer, size_t sizeOfBuffer, const TCHAR* format, ... )
{
  va_list args; 
	va_start(args, format);
  
#if _MSC_VER<1400
  UNREFERENCED_PARAMETER(sizeOfBuffer);
  return _vstprintf(buffer, format, args);
#else
  return _vstprintf_s(buffer, sizeOfBuffer, format, args);
#endif
}
