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

#include "stdafx.h"
#include "resource.h"
#include "ErrorReportDlg.h"
#include "ResendDlg.h"
#include "CrashInfoReader.h"
#include "strconv.h"
#include "Utility.h"

CAppModule _Module;             // WTL's application module.
CErrorReportDlg dlgErrorReport; // "Error Report" dialog.
CResendDlg dlgResend;           // "Send Error Reports" dialog.

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int /*nCmdShow*/ = SW_SHOWDEFAULT)
{ 
	// Get command line parameters.
	LPCWSTR szCommandLine = GetCommandLineW();

    // Split command line.
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(szCommandLine, &argc);

    // Check parameter count.
    if(argc!=2)
        return 1; // No arguments passed

    // Read crash information from the file mapping object.
    CString sFileMappingName = CString(argv[1]);
    int nInit = g_CrashInfo.Init(sFileMappingName.GetBuffer(0));
    if(nInit!=0)
    {
        MessageBox(NULL, _T("Couldn't initialize!"), _T("CrashSender.exe"), MB_ICONERROR);
        return 1;
    }

    if(!g_CrashInfo.m_bSendRecentReports)
    {
        // Do the crash info collection work assynchronously
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
        // Create "Error Report" dialog
        if(dlgErrorReport.Create(NULL) == NULL)
        {
            ATLTRACE(_T("Error report dialog creation failed!\n"));
            return 0;
        }
    }
    else
    {
        // Check if another instance of CrashSender.exe is running.
        ::CreateMutex( NULL, FALSE,_T("Local\\43773530-129a-4298-88f2-20eea3e4a59b"));
        if (::GetLastError() == ERROR_ALREADY_EXISTS)
        {		
            // Another CrashSender.exe already tries to resend recent reports; exit.
            return 0;
        }

        if(g_CrashInfo.GetReportCount()==0)
            return 0; // There are no reports for us to send.

        // Check if it is ok to remind user now.
        if(!g_CrashInfo.IsRemindNowOK())
            return 0;

		// Create "Send Error Reports" dialog.
        if(dlgResend.Create(NULL) == NULL)
        {
            ATLTRACE(_T("Resend dialog creation failed!\n"));
            return 0;
        }
    }

    // Process window messages.
    int nRet = theLoop.Run();

    // Wait until the worker thread is exited. 
    g_ErrorReportSender.WaitForCompletion();
    nRet = g_ErrorReportSender.GetGlobalStatus();

    // Remove temporary files we might create and perform other finalizing work.
    g_ErrorReportSender.Finalize();

    _Module.RemoveMessageLoop();

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

