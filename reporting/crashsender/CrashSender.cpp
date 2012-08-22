/************************************************************************************* 
This file is a part of CrashRpt library.

Copyright (c) 2003, Michael Carruth
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

// File: CrashSender.cpp
// Description: Entry point to the application. 
// Authors: zexspectrum
// Date: 2010

#include "stdafx.h"
#include "resource.h"
#include "ErrorReportDlg.h"
#include "ResendDlg.h"
#include "CrashInfoReader.h"
#include "strconv.h"
#include "Utility.h"

CAppModule _Module;             // WTL's application module.

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int /*nCmdShow*/ = SW_SHOWDEFAULT)
{ 
	int nRet = 0; // Return code
	CErrorReportDlg dlgErrorReport; // Error Report dialog
	CResendDlg dlgResend; // Resend dialog

	// Get command line parameters.
	LPCWSTR szCommandLine = GetCommandLineW();

    // Split command line.
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(szCommandLine, &argc);

    // Check parameter count.
    if(argc!=2)
        return 1; // No arguments passed, exit.

	// Extract file mapping name from command line arg.    
    CString sFileMappingName = CString(argv[1]);
	
	// Create the sender model that will collect crash report data 
	// and send error report(s).
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

	// Init the sender object
	BOOL bInit = pSender->Init(sFileMappingName.GetBuffer(0));
	if(!bInit)
    {
		// Failed to init        
        return 0;
    }      
	    
	// Determine what to do next 
	// (either run in GUI more or run in silent mode).
	if(!pSender->GetCrashInfo()->m_bSilentMode)
	{
		// GUI mode.
		// Create message loop.
		CMessageLoop theLoop;
		_Module.AddMessageLoop(&theLoop);

		if(!pSender->GetCrashInfo()->m_bSendRecentReports)
		{
			// Create "Error Report" dialog			
			if(dlgErrorReport.Create(NULL) == NULL)
			{
				ATLTRACE(_T("Error report dialog creation failed!\n"));
				return 1;
			}			
		}
		else
		{        
			// Create "Send Error Reports" dialog.					
			if(dlgResend.Create(NULL) == NULL)
			{
				ATLTRACE(_T("Resend dialog creation failed!\n"));
				return 1;
			}			
		}

		// Process window messages.
		nRet = theLoop.Run();	    
		_Module.RemoveMessageLoop();
	}
	else
	{
		// Silent (non-GUI mode).
		// Run the sender and wait until it exits.
		pSender->Run();
		pSender->WaitForCompletion();
		// Get return status
		nRet = pSender->GetGlobalStatus();
	}
    
	// Delete sender object.
	delete pSender;

	// Exit.
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

