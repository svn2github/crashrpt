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

#include "stdafx.h"
#include "resource.h"
#include "ErrorReportDlg.h"
#include "ResendDlg.h"
#include "CrashInfoReader.h"
#include "strconv.h"
#include "Utility.h"

CAppModule _Module;
CErrorReportDlg dlgErrorReport;
CResendDlg dlgResend;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int /*nCmdShow*/ = SW_SHOWDEFAULT)
{
  LPCWSTR szCommandLine = GetCommandLineW();
  
  int argc = 0;
  LPWSTR* argv = CommandLineToArgvW(szCommandLine, &argc);
 
  // Read the crash info passed by CrashRpt.dll to CrashSender.exe 
  if(argc!=2)
    return 1; // No arguments passed
  
  // Read crash info
  g_CrashInfo.Init(CString(argv[1]));

  if(!g_CrashInfo.m_bSendRecentReports)
  {
    // Do the rest of the work assynchroniosly
    g_ErrorReportSender.DoWork(COLLECT_CRASH_INFO);
  }
  
  // Check window mirroring settings 
  CString sRTL = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("Settings"), _T("RTLReading"));
  if(sRTL.CompareNoCase(_T("1"))==0)
  {
  	SetProcessDefaultLayout(LAYOUT_RTL);  
  }  

  CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);
    
  if(!g_CrashInfo.m_bSendRecentReports)
  {
	  if(dlgErrorReport.Create(NULL) == NULL)
	  {
		  ATLTRACE(_T("Main dialog creation failed!\n"));
		  return 0;
	  }
  }
  else
  {
    if(dlgResend.Create(NULL) == NULL)
	  {
		  ATLTRACE(_T("Resend dialog creation failed!\n"));
		  return 0;
	  }
  }
  
	int nRet = theLoop.Run();

  // Wait until the worker thread is exited
  g_ErrorReportSender.WaitForCompletion();

	_Module.RemoveMessageLoop();

	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

  int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
