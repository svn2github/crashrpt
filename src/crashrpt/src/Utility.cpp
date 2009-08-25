///////////////////////////////////////////////////////////////////////////////
//
//  Module: Utility.cpp
//
//    Desc: See Utility.h
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utility.h"
#include "resource.h"
#include <time.h>
#include "atldlgs.h"
#include <shellapi.h>


CString CUtility::getAppName()
{
   TCHAR szFileName[_MAX_PATH];
   GetModuleFileName(NULL, szFileName, _MAX_FNAME);

   CString sAppName; // Extract from last '\' to '.'
   sAppName = szFileName;
   sAppName = sAppName.Mid(sAppName.ReverseFind(_T('\\')) + 1)
                      .SpanExcluding(_T("."));

   return sAppName;
}

int CUtility::getTempDirectory(CString& strTemp)
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

CString CUtility::getTempFileName()
{
   TCHAR szTempDir[MAX_PATH - 14]   = _T("");
   TCHAR szTempFile[MAX_PATH]       = _T("");

   if (GetTempPath(MAX_PATH - 14, szTempDir))
      GetTempFileName(szTempDir, getAppName(), 0, szTempFile);

   return szTempFile;
}


CString CUtility::GetModulePath(HMODULE hModule)
{
	CString string;
	LPTSTR buf = string.GetBuffer(_MAX_PATH);
	GetModuleFileName(hModule, buf, _MAX_PATH);
	*(_tcsrchr(buf,'\\'))=0; // remove executable name
	string.ReleaseBuffer();
	return string;
}

int CUtility::GetSystemTimeUTC(CString& sTime)
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

int CUtility::GenerateGUID(CString& sGUID)
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

int CUtility::GetOSFriendlyName(CString& sOSName)
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

int CUtility::GetSpecialFolder(int csidl, CString& sFolderPath)
{
  sFolderPath.Empty();

  TCHAR szPath[_MAX_PATH];
  BOOL bResult = SHGetSpecialFolderPath(NULL, szPath, csidl, TRUE);
  if(!bResult)
    return 1;

  sFolderPath = CString(szPath);

  return 0;
}

CString CUtility::ReplaceInvalidCharsInFileName(CString sFileName)
{
	sFileName.Replace(_T("*"),_T("_"));
	sFileName.Replace(_T("|"),_T("_"));
	sFileName.Replace(_T("/"),_T("_"));
	sFileName.Replace(_T("?"),_T("_"));
	sFileName.Replace(_T("<"),_T("_"));
	sFileName.Replace(_T(">"),_T("_"));
	return sFileName;
}

int CUtility::RecycleFile(CString sFilePath, bool bPermanentDelete)
{
  SHFILEOPSTRUCT fop;
  memset(&fop, 0, sizeof(SHFILEOPSTRUCT));
  fop.fFlags |= FOF_SILENT;                // don't report progress
  fop.fFlags |= FOF_NOERRORUI;             // don't report errors
  fop.fFlags |= FOF_NOCONFIRMATION;        // don't confirm delete
  fop.wFunc = FO_DELETE;                   // REQUIRED: delete operation
  fop.pFrom = sFilePath.GetBuffer(0);      // REQUIRED: which file(s)
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

//CString CUtility::LoadString(UINT uID)
//{
//  CString str;
//	TCHAR buf[1024]=_T("");
//  ::LoadString(GetModuleHandle(NULL), uID, buf, 1024);
//	str = buf;
//	return str;
//}
