/************************************************************************************* 
  This file is a part of CrashRpt library.

  CrashRpt is Copyright (c) 2003, Michael Carruth
  All rights reserved.
 
  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:
 
   * Redistributions of source code must retain the above copyright notice, this 
     list of conditions and the following disclaimer.
 
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.
 
   * Neither the name of the author nor the names of its contributors 
     may be used to endorse or promote products derived from this software without 
     specific prior written permission.
 

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************************/

// File: Utility.cpp
// Description: Miscellaneous helper functions
// Authors: mikecarruth, zexspectrum
// Date: 

#include "stdafx.h"
#include "Utility.h"
#include "resource.h"
#include <time.h>
#include "atldlgs.h"
#include <shellapi.h>
#include "strconv.h"

CString Utility::getAppName()
{
   TCHAR szFileName[_MAX_PATH];
   GetModuleFileName(NULL, szFileName, _MAX_FNAME);

   CString sAppName; // Extract from last '\' to '.'
   sAppName = szFileName;
   sAppName = sAppName.Mid(sAppName.ReverseFind(_T('\\')) + 1)
                      .SpanExcluding(_T("."));

   return sAppName;
}

CString Utility::GetModuleName(HMODULE hModule)
{
	CString string;
	LPTSTR buf = string.GetBuffer(_MAX_PATH);
	GetModuleFileName(hModule, buf, _MAX_PATH);
	string.ReleaseBuffer();
	return string;
}

CString Utility::GetModulePath(HMODULE hModule)
{
	CString string;
	LPTSTR buf = string.GetBuffer(_MAX_PATH);
	GetModuleFileName(hModule, buf, _MAX_PATH);
	*(_tcsrchr(buf,'\\'))=0; // remove executable name
	string.ReleaseBuffer();
	return string;
}

int Utility::getTempDirectory(CString& strTemp)
{
  TCHAR* pszTempVar = NULL;
  
#if _MSC_VER<1400
  pszTempVar = _tgetenv(_T("TEMP"));
  strTemp = CString(pszTempVar);
#else
  size_t len = 0;
  errno_t err = _tdupenv_s(&pszTempVar, &len, _T("TEMP"));
  if(err!=0)
  {
    // Couldn't get environment variable TEMP    
    return 1;
  }
  strTemp = CString(pszTempVar);
  free(pszTempVar);
#endif    

  return 0;
}

CString Utility::getTempFileName()
{
   TCHAR szTempDir[MAX_PATH - 14]   = _T("");
   TCHAR szTempFile[MAX_PATH]       = _T("");

   if (GetTempPath(MAX_PATH - 14, szTempDir))
      GetTempFileName(szTempDir, getAppName(), 0, szTempFile);

   return szTempFile;
}

int Utility::GetSystemTimeUTC(CString& sTime)
{
  sTime.Empty();

  // Get system time in UTC format

  time_t cur_time;
  time(&cur_time);
  char szDateTime[64];
  
#if _MSC_VER<1400
  struct tm* timeinfo = gmtime(&cur_time);
  strftime(szDateTime, 64,  "%Y-%m-%dT%H:%M:%SZ", timeinfo);
#else
  struct tm timeinfo;
  gmtime_s(&timeinfo, &cur_time);
  strftime(szDateTime, 64,  "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
#endif

  sTime = szDateTime;

  return 0;
}

int Utility::GenerateGUID(CString& sGUID)
{
  int status = 1;
  sGUID.Empty();

  strconv_t strconv;

  // Create GUID

  UCHAR *pszUuid = 0; 
  GUID *pguid = NULL;
  pguid = new GUID;
  if(pguid!=NULL)
  {
    HRESULT hr = CoCreateGuid(pguid);
    if(SUCCEEDED(hr))
    {
      // Convert the GUID to a string
      hr = UuidToStringA(pguid, &pszUuid);
      if(SUCCEEDED(hr) && pszUuid!=NULL)
      { 
        status = 0;
        sGUID = strconv.a2t((char*)pszUuid);
        RpcStringFreeA(&pszUuid);
      }
    }
    delete pguid; 
  }

  return status;
}

int Utility::GetOSFriendlyName(CString& sOSName)
{
  sOSName.Empty();
  CRegKey regKey;
  LONG lResult = regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), KEY_READ);
  if(lResult==ERROR_SUCCESS)
  {    
    TCHAR buf[1024];
    ULONG buf_size = 0;

	  TCHAR* PRODUCT_NAME = _T("ProductName");
	  TCHAR* CURRENT_BUILD_NUMBER = _T("CurrentBuildNumber");
	  TCHAR* CSD_VERSION = _T("CSDVersion");

#pragma warning(disable:4996)

    buf_size = 1023;
    if(ERROR_SUCCESS == regKey.QueryValue(buf, PRODUCT_NAME, &buf_size))
    {
      sOSName += buf;
    }
    
    buf_size = 1023;
    if(ERROR_SUCCESS == regKey.QueryValue(buf, CURRENT_BUILD_NUMBER, &buf_size))
    {
      sOSName += _T(" Build ");
      sOSName += buf;
    }

    buf_size = 1023;
    if(ERROR_SUCCESS == regKey.QueryValue(buf, CSD_VERSION, &buf_size))
    {
      sOSName += _T(" ");
      sOSName += buf;
    }

#pragma warning(default:4996)

    regKey.Close();    
    return 0;
  }

  return 1;
}

int Utility::GetSpecialFolder(int csidl, CString& sFolderPath)
{
  sFolderPath.Empty();

  TCHAR szPath[_MAX_PATH];
  BOOL bResult = SHGetSpecialFolderPath(NULL, szPath, csidl, TRUE);
  if(!bResult)
    return 1;

  sFolderPath = CString(szPath);

  return 0;
}

CString Utility::ReplaceInvalidCharsInFileName(CString sFileName)
{
	sFileName.Replace(_T("*"),_T("_"));
	sFileName.Replace(_T("|"),_T("_"));
	sFileName.Replace(_T("/"),_T("_"));
	sFileName.Replace(_T("?"),_T("_"));
	sFileName.Replace(_T("<"),_T("_"));
	sFileName.Replace(_T(">"),_T("_"));
	return sFileName;
}

int Utility::RecycleFile(CString sFilePath, bool bPermanentDelete)
{
  SHFILEOPSTRUCT fop;
  memset(&fop, 0, sizeof(SHFILEOPSTRUCT));
  
  TCHAR szFrom[_MAX_PATH];  
  memset(szFrom, 0, sizeof(TCHAR)*(_MAX_PATH));
  _TCSCPY_S(szFrom, _MAX_PATH, sFilePath.GetBuffer(0));
  szFrom[sFilePath.GetLength()+1] = 0;

  fop.fFlags |= FOF_SILENT;                // don't report progress
  fop.fFlags |= FOF_NOERRORUI;           // don't report errors
  fop.fFlags |= FOF_NOCONFIRMATION;        // don't confirm delete
  fop.wFunc = FO_DELETE;                   // REQUIRED: delete operation
  fop.pFrom = szFrom;                      // REQUIRED: which file(s)
  fop.pTo = NULL;                          // MUST be NULL
  if (bPermanentDelete) 
  { 
    // if delete requested..
    fop.fFlags &= ~FOF_ALLOWUNDO;   // ..don't use Recycle Bin
  } 
  else 
  {                                 // otherwise..
    fop.fFlags |= FOF_ALLOWUNDO;    // ..send to Recycle Bin
  }
  
  return SHFileOperation(&fop); // do it!  
}

//CString Utility::GetINIString(LPCTSTR pszSection, LPCTSTR pszName)
//{
//  static CString sINIFileName = _T("");
//
//  if(sINIFileName.IsEmpty())
//  {
//    sINIFileName = GetModulePath(GetModuleHandle(NULL)) + _T("\\crashrpt_lang.ini");
//  }
//  
//  return GetINIString(sINIFileName, pszSection, pszName);
//}

CString Utility::GetINIString(LPCTSTR pszFile, LPCTSTR pszSection, LPCTSTR pszName)
{  
  TCHAR szBuffer[1024] = _T("");  
  GetPrivateProfileString(pszSection, pszName, _T(""), szBuffer, 1024, pszFile);

  CString sResult = szBuffer;
  sResult.Replace(_T("\\n"), _T("\n"));

  return sResult;
}

void Utility::SetLayoutRTL(HWND hWnd)
{
  DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
  dwExStyle |= WS_EX_LAYOUTRTL;
  SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle);

  SetLayout(GetDC(hWnd), LAYOUT_RTL);

  CRect rcWnd;
  ::GetClientRect(hWnd, &rcWnd);

  HWND hWndChild = GetWindow(hWnd, GW_CHILD);
  while(hWndChild!=NULL)
  {    
    SetLayoutRTL(hWndChild);

    CRect rc;
    ::GetWindowRect(hWndChild, &rc);    
    ::MapWindowPoints(0, hWnd, (LPPOINT)&rc, 2);
    ::MoveWindow(hWndChild, rcWnd.Width()-rc.right, rc.top, rc.Width(), rc.Height(), TRUE);

    SetLayout(GetDC(hWndChild), LAYOUT_RTL);

    hWndChild = GetWindow(hWndChild, GW_HWNDNEXT);
  }  
}

CString Utility::FormatErrorMsg(DWORD dwErrorCode)
{
	LPTSTR msg = 0;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&msg, 0, NULL);
	CString str = msg;
  str.Replace(_T("\r\n"), _T(""));
	GlobalFree(msg);
	return str;
}

// GetBaseFileName
// This helper function returns file name without extension
CString Utility::GetFileName(CString sPath)
{
  CString sBase = sPath;
  int pos1 = sPath.ReverseFind('\\');
  if(pos1>=0)
    sBase = sBase.Mid(pos1+1);
  
  return sBase;
}

// GetBaseFileName
// This helper function returns file name without extension
CString Utility::GetBaseFileName(CString sFileName)
{
  CString sBase = sFileName;
  int pos1 = sFileName.ReverseFind('\\');
  if(pos1>=0)
    sBase = sBase.Mid(pos1+1);

  int pos2 = sBase.ReverseFind('.');
  if(pos2>=0)
  {
    sBase = sFileName.Mid(0, pos2);
  }
  return sBase;
}

// GetFileExtension
// This helper function returns file extension by file name
CString Utility::GetFileExtension(CString sFileName)
{
  CString sExt;
  int pos = sFileName.ReverseFind('.');
  if(pos>=0)
  {
    sExt = sFileName.Mid(pos+1);
  }
  return sExt;
}

BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam)
{
  HWND* phWnd = (HWND*)lParam;

  DWORD dwMyProcessId = GetCurrentProcessId();

  if(IsWindowVisible(hWnd))
  {
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    if(dwProcessId == dwMyProcessId)
    {      
      DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
      DWORD dwExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
      HWND hWndParent = GetParent(hWnd);
      if(hWndParent==NULL && 
        (dwExStyle&WS_EX_APPWINDOW || (dwStyle&WS_CAPTION && dwStyle&WS_SYSMENU)) )
      {      
        *phWnd = hWnd; // Found
        return FALSE;
      }
    }
  }
   
  return TRUE;
}

HWND Utility::FindAppWindow()
{
  HWND hWnd = NULL;
  EnumWindows(EnumWndProc, (LPARAM)&hWnd);
  return hWnd;
}

CString Utility::GetProductVersion(CString sModuleName)
{
  CString sProductVer; 

  DWORD dwBuffSize = GetFileVersionInfoSize(sModuleName, 0);
  LPBYTE pBuff = new BYTE[dwBuffSize];
    
  if(0!=GetFileVersionInfo(sModuleName, 0, dwBuffSize, pBuff))
  {
    VS_FIXEDFILEINFO* fi = NULL;
    UINT uLen = 0;
    VerQueryValue(pBuff, _T("\\"), (LPVOID*)&fi, &uLen);

    WORD dwVerMajor = (WORD)(fi->dwProductVersionMS>>16);
    WORD dwVerMinor = (WORD)(fi->dwProductVersionMS&0xFF);
    WORD dwPatchLevel = (WORD)(fi->dwProductVersionLS>>16);
    WORD dwVerBuild = (WORD)(fi->dwProductVersionLS&0xFF);

    sProductVer.Format(_T("%u.%u.%u.%u"), 
      dwVerMajor, dwVerMinor, dwPatchLevel, dwVerBuild);
  }

  delete [] pBuff;

  return sProductVer;
}

// Creates a folder. If some intermediate folders in the path do not exist,
// it creates them.
BOOL Utility::CreateFolder(CString sFolderName)
{  
  CString sIntermediateFolder;

  // Skip disc drive name "X:\" if presents
  int start = sFolderName.Find(':', 0);
  if(start>=0)
    start+=2; 

  int pos = start;  
  for(;;)
  {
    pos = sFolderName.Find('\\', pos);
    if(pos<0)
    {
      sIntermediateFolder = sFolderName;
    }
    else
    {
      sIntermediateFolder = sFolderName.Left(pos);
    }

    BOOL bCreate = CreateDirectory(sIntermediateFolder, NULL);
    if(!bCreate && GetLastError()!=ERROR_ALREADY_EXISTS)
      return FALSE;

    if(pos==-1)
      break;

    pos++;
  }

  return TRUE;
}

ULONG64 Utility::SystemTimeToULONG64( const SYSTEMTIME& st )
{
  FILETIME ft ;
  SystemTimeToFileTime( &st, &ft ) ;
  ULARGE_INTEGER integer ;
  integer.LowPart = ft.dwLowDateTime ;
  integer.HighPart = ft.dwHighDateTime ;
  return integer.QuadPart ;
}