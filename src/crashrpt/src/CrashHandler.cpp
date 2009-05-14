///////////////////////////////////////////////////////////////////////////////
//
//  Module: CrashHandler.cpp
//
//    Desc: See CrashHandler.h
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CrashHandler.h"
#include "zlibcpp.h"
#include "excprpt.h"
#include "maindlg.h"
#include "process.h"
#include "mailmsg.h"

// global app module
CAppModule _Module;

// maps crash objects to processes
CSimpleMap<int, CCrashHandler*> _crashStateMap;

// unhandled exception callback set with SetUnhandledExceptionFilter()
LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
   _crashStateMap.Lookup(_getpid())->GenerateErrorReport(pExInfo);

   return EXCEPTION_EXECUTE_HANDLER;
}

CCrashHandler::CCrashHandler(LPGETLOGFILE lpfn /*=NULL*/, 
                             LPCTSTR lpcszTo /*=NULL*/, 
                             LPCTSTR lpcszSubject /*=NULL*/)
{
   // wtl initialization stuff...
	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

   hRes = _Module.Init(NULL, GetModuleHandle("CrashRpt.dll"));
   ATLASSERT(SUCCEEDED(hRes));

	::DefWindowProc(NULL, 0, 0, 0L);

   // initialize member data
   m_lpfnCallback = NULL;
   m_oldFilter    = NULL;

   // save user supplied callback
   if (lpfn)
      m_lpfnCallback = lpfn;

   // add this filter in the exception callback chain
   m_oldFilter = SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

   // attach this handler with this process
   m_pid = _getpid();
   _crashStateMap.Add(m_pid, this);

   // save optional email info
   m_sTo = lpcszTo;
   m_sSubject = lpcszSubject;
}

CCrashHandler::~CCrashHandler()
{
   // reset exception callback
   if (m_oldFilter)
      SetUnhandledExceptionFilter(m_oldFilter);

   _crashStateMap.Remove(m_pid);

   // uninitialize
   _Module.Term();
	::CoUninitialize();

}

void CCrashHandler::AddFile(LPCTSTR lpFile, LPCTSTR lpDesc)
{
   // make sure the file exist
   HANDLE hFile = ::CreateFile(
                     lpFile,
                     GENERIC_READ,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     0);
   if (hFile)
   {
      // add file to report
      m_files[lpFile] = lpDesc;
      ::CloseHandle(hFile);
   }
}

void CCrashHandler::GenerateErrorReport(PEXCEPTION_POINTERS pExInfo)
{
   CExceptionReport  rpt(pExInfo);
   CMainDlg          mainDlg;
   CZLib             zlib;
   CString           sTempFileName = CUtility::getTempFileName();
   unsigned int      i;

   // let client add application specific files to report
   if (m_lpfnCallback && !m_lpfnCallback(this))
      return;

   // add crash files to report
   m_files[rpt.getCrashFile()] = CString((LPCTSTR)IDS_CRASH_DUMP);
   m_files[rpt.getCrashLog()] = CString((LPCTSTR)IDS_CRASH_LOG);

   // add symbol files to report
   for (i = 0; i < (UINT)rpt.getNumSymbolFiles(); i++)
      m_files[(LPCTSTR)rpt.getSymbolFile(i)] = 
      CString((LPCTSTR)IDS_SYMBOL_FILE);
 
   // zip the report
   if (!zlib.Open(sTempFileName))
      return;
   
   // add report files to zip
   TStrStrMap::iterator cur = m_files.begin();
   for (i = 0; i < m_files.size(); i++, cur++)
      zlib.AddFile((*cur).first);

   zlib.Close();

   // display main dialog
   mainDlg.m_pUDFiles = &m_files;
   if (IDOK == mainDlg.DoModal())
   {
      if (m_sTo.IsEmpty() || 
          !MailReport(rpt, sTempFileName, mainDlg.m_sEmail, mainDlg.m_sDescription))
      {
         SaveReport(rpt, sTempFileName);
      }
   }

   DeleteFile(sTempFileName);
}

BOOL CCrashHandler::SaveReport(CExceptionReport&, LPCTSTR lpcszFile)
{
   // let user more zipped report
   return (CopyFile(lpcszFile, CUtility::getSaveFileName(), TRUE));
}

BOOL CCrashHandler::MailReport(CExceptionReport&, LPCTSTR lpcszFile,
                               LPCTSTR lpcszEmail, LPCTSTR lpcszDesc)
{
   CMailMsg msg;
   msg
      .SetTo(m_sTo)
      .SetFrom(lpcszEmail)
      .SetSubject(m_sSubject.IsEmpty()?_T("Incident Report"):m_sSubject)
      .SetMessage(lpcszDesc)
      .AddAttachment(lpcszFile, CUtility::getAppName() + _T(".zip"));

   return (msg.Send());
}
