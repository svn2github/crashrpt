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
#include "ErrorReportSender.h"
#include "MailMsg.h"
#include "smtpclient.h"
#include "HttpRequestSender.h"
#include "CrashRpt.h"
#include "md5.h"
#include "Utility.h"
#include "zip.h"
#include "CrashInfoReader.h"
#include "strconv.h"
#include "ScreenCap.h"
#include "base64.h"
#include <sys/stat.h>
#include "dbghelp.h"

CErrorReportSender* CErrorReportSender::m_pInstance = NULL;

// Constructor
CErrorReportSender::CErrorReportSender()
{
    // Init variables
    m_nGlobalStatus = 0;
    m_nCurReport = 0;
    m_hThread = NULL;
    m_SendAttempt = 0;
    m_Action=COLLECT_CRASH_INFO;
    m_bExport = FALSE;
}

// Destructor
CErrorReportSender::~CErrorReportSender()
{
	Destroy();
}

CErrorReportSender* CErrorReportSender::GetInstance()
{
	// Return singleton object
	if(m_pInstance==NULL)	
		m_pInstance = new CErrorReportSender();
	return m_pInstance;
}

BOOL CErrorReportSender::Init(LPCTSTR szFileMappingName)
{
	m_sErrorMsg = _T("Unspecified error.");
	
	// Read crash information from the file mapping object.
    int nInit = m_CrashInfo.Init(szFileMappingName);
    if(nInit!=0)
    {
        m_sErrorMsg.Format(_T("Error reading crash info: %s"), m_CrashInfo.GetErrorMsg());
        return FALSE;
    }

	// Check window mirroring settings 
    CString sRTL = Utility::GetINIString(m_CrashInfo.m_sLangFileName, _T("Settings"), _T("RTLReading"));
    if(sRTL.CompareNoCase(_T("1"))==0)
    {
		// Set Right-to-Left reading order
        SetProcessDefaultLayout(LAYOUT_RTL);  
    }

    if(!m_CrashInfo.m_bSendRecentReports)
    {
        // Start crash info collection work assynchronously
        DoWork(COLLECT_CRASH_INFO);
    }
	else
	{
		// Check if another instance of CrashSender.exe is running.
        ::CreateMutex( NULL, FALSE,_T("Local\\43773530-129a-4298-88f2-20eea3e4a59b"));
        if (::GetLastError() == ERROR_ALREADY_EXISTS)
        {		
            m_sErrorMsg = _T("Another CrashSender.exe already tries to resend recent reports.");
            return FALSE;
        }

        if(m_CrashInfo.GetReportCount()==0)
		{
			m_sErrorMsg = _T("There are no reports for us to send.");
            return FALSE; 
		}

        // Check if it is ok to remind user now.
        if(!m_CrashInfo.IsRemindNowOK())
		{
			m_sErrorMsg = _T("Not enough time elapsed to remind user about recent crash reports.");
            return FALSE;
		}
	}

	// Done.
	m_sErrorMsg = _T("Success.");
	return TRUE;
}

void CErrorReportSender::Destroy()
{
	// Wait until the worker thread is exited. 
    WaitForCompletion();
    
	//nRet = g_ErrorReportSender.GetGlobalStatus();

    // Remove temporary files we might create and perform other finalizing work.
    Finalize();
}

CCrashInfoReader* CErrorReportSender::GetCrashInfo()
{
	return &m_CrashInfo;
}

CString CErrorReportSender::GetErrorMsg()
{
	return m_sErrorMsg;
}

void CErrorReportSender::SetNotificationWindow(HWND hWnd)
{
	// Set notification window
	m_hWndNotify = hWnd;
}

BOOL CErrorReportSender::Run()
{
	// Wait for completion of crash info collector.
	WaitForCompletion();

	// Determine whether to send crash report now 
	// or to exit without sending report.
	if(m_CrashInfo.m_bSendErrorReport) // If we should send error report now
    {
        // Compress report files and send the report
		SetExportFlag(FALSE, _T(""));
        DoWork(COMPRESS_REPORT|SEND_REPORT);    
    }
    else // If we shouldn't send error report now
    {      
        // Exit        

    }    

	return TRUE;
}

int CErrorReportSender::GetGlobalStatus()
{
    // Return global error report delivery status
    return m_nGlobalStatus;
}

int CErrorReportSender::GetCurReport()
{
    // Returns the index of error report currently being sent
    return m_nCurReport;
}

BOOL CErrorReportSender::SetCurReport(int nCurReport)
{
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

    // Validate input params
    if(nCurReport<0 || nCurReport>=pSender->GetCrashInfo()->GetReportCount())
    {
        ATLASSERT(0);
        return FALSE;
    }

    // Update current report index
    m_nCurReport = nCurReport;
    return TRUE;
}

// This method performs crash files collection and/or
// error report sending work in a worker thread.
BOOL CErrorReportSender::DoWork(int action)
{
    // Save the action code
    m_Action = action;

    // Create worker thread which will do all work assynchronously
    m_hThread = CreateThread(NULL, 0, WorkerThread, (LPVOID)this, 0, NULL);

    // Check if the thread was created ok
    if(m_hThread==NULL)
        return FALSE; // Failed to create worker thread

    // Done, return
    return TRUE;
}

// This method is the worker thread procedure that delegates further work 
// back to the CErrorReportSender class
DWORD WINAPI CErrorReportSender::WorkerThread(LPVOID lpParam)
{
    // Delegate the action to the CErrorReportSender::DoWorkAssync() method
    CErrorReportSender* pSender = (CErrorReportSender*)lpParam;
    pSender->DoWorkAssync();
	pSender->m_hThread = NULL; // clean up
    // Exit code can be ignored
    return 0;
}

// This method unblocks the parent process
void CErrorReportSender::UnblockParentProcess()
{
    // Notify the parent process that we have finished with minidump,
    // so the parent process is able to unblock and terminate itself.

	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

    // Open the event the parent process had created for us
    CString sEventName;
    sEventName.Format(_T("Local\\CrashRptEvent_%s"), pSender->GetCrashInfo()->GetReport(0).m_sCrashGUID);
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, sEventName);
    if(hEvent!=NULL)
        SetEvent(hEvent); // Signal event
}

// This method collects required crash report files (minidump, screenshot etc.)
// and then sends the error report over the Internet.
void CErrorReportSender::DoWorkAssync()
{
    // Reset the completion event
    m_Assync.Reset();

    if(m_CrashInfo.m_bSendRecentReports) // If we are currently sending pending error reports
    {
        // Add a message to log
        CString sMsg;
        sMsg.Format(_T(">>> Performing actions with error report: '%s'"), 
            m_CrashInfo.GetReport(m_nCurReport).m_sErrorReportDirName);
        m_Assync.SetProgress(sMsg, 0, false);
    }

    if(m_Action&COLLECT_CRASH_INFO) // Collect crash report files
    {
        // Add a message to log
        m_Assync.SetProgress(_T("Start collecting information about the crash..."), 0, false);

        // First take a screenshot of user's desktop (if needed).
        TakeDesktopScreenshot();

        if(m_Assync.IsCancelled()) // Check if user-cancelled
        {      
            // Parent process can now terminate
            UnblockParentProcess();

            // Add a message to log
            m_Assync.SetProgress(_T("[exit_silently]"), 0, false);
            return;
        }

        // Create crash dump.
        CreateMiniDump();

        if(m_Assync.IsCancelled()) // Check if user-cancelled
        {      
            // Parent process can now terminate
            UnblockParentProcess();

            // Add a message to log
            m_Assync.SetProgress(_T("[exit_silently]"), 0, false);
            return;
        }

        // Notify the parent process that we have finished with minidump,
        // so the parent process is able to unblock and terminate itself.
        UnblockParentProcess();

        // Copy user-provided files.
        CollectCrashFiles();

        if(m_Assync.IsCancelled()) // Check if user-cancelled
        {      
            // Add a message to log
            m_Assync.SetProgress(_T("[exit_silently]"), 0, false);
            return;
        }

        // Add a message to log
        m_Assync.SetProgress(_T("[confirm_send_report]"), 100, false);
    }

    if(m_Action&COMPRESS_REPORT) // We have to compress error report file into ZIP archive
    { 
        // Compress error report files
        BOOL bCompress = CompressReportFiles(m_CrashInfo.GetReport(m_nCurReport));
        if(!bCompress)
        {
            // Add a message to log
            m_Assync.SetProgress(_T("[status_failed]"), 100, false);
            return; // Error compressing files
        }
    }

    if(m_Action&RESTART_APP) // We need to restart the parent process
    { 
        // Restart the application
        RestartApp();
    }

    if(m_Action&SEND_REPORT) // We need to send the report over the Internet
    {
        // Send the error report.
        SendReport();
    }

    // Done
    return;
}

// Returns the export flag (the flag is set if we are exporting error report as a ZIP archive)
void CErrorReportSender::SetExportFlag(BOOL bExport, CString sExportFile)
{
    // This is used when we need to export error report files as a ZIP archive
    m_bExport = bExport;
    m_sExportFileName = sExportFile;
}

// This method blocks until worker thread is exited
void CErrorReportSender::WaitForCompletion()
{	
	if(m_hThread!=NULL)
		WaitForSingleObject(m_hThread, INFINITE);	
}

// Gets status of the local operation
void CErrorReportSender::GetStatus(int& nProgressPct, std::vector<CString>& msg_log)
{
    m_Assync.GetProgress(nProgressPct, msg_log); 
}

// This method cancels the current operation
void CErrorReportSender::Cancel()
{
    // User-cancelled
    m_Assync.Cancel();
}

// This method notifies the main thread that we have finished assync operation
void CErrorReportSender::FeedbackReady(int code)
{
    m_Assync.FeedbackReady(code);
}

// This method cleans up temporary files
BOOL CErrorReportSender::Finalize()
{  
    if(m_CrashInfo.m_bSendErrorReport && !m_CrashInfo.m_bQueueEnabled)
    {
        // Remove report files if queue disabled
        Utility::RecycleFile(m_CrashInfo.GetReport(0).m_sErrorReportDirName, true);    
    }

    if(!m_CrashInfo.m_bSendErrorReport && 
        m_CrashInfo.m_bStoreZIPArchives) // If we should generate a ZIP archive
    {
        // Compress error report files
        DoWork(COMPRESS_REPORT);    
        WaitForCompletion();
    }

    // If needed, restart the application
    if(!m_CrashInfo.m_bSendRecentReports)
    {
        DoWork(RESTART_APP); 
        WaitForCompletion();
    }

    // Done OK
    return TRUE;
}

// This method takes the desktop screenshot (screenshot of entire virtual screen
// or screenshot of the main window). 
BOOL CErrorReportSender::TakeDesktopScreenshot()
{
    CScreenCapture sc; // Screen capture object
    ScreenshotInfo ssi; // Screenshot params
    std::vector<CString> screenshot_names; // The list of screenshot files

    // Add a message to log
    m_Assync.SetProgress(_T("[taking_screenshot]"), 0);    

    // Check if screenshot capture is allowed
    if(!m_CrashInfo.m_bAddScreenshot)
    {
        // Add a message to log
        m_Assync.SetProgress(_T("Desktop screenshot generation disabled; skipping."), 0);    
        // Exit, nothing to do here
        return TRUE;
    }

    // Add a message to log
    m_Assync.SetProgress(_T("Taking desktop screenshot"), 0);    

    // Get screenshot flags passed by the parent process
    DWORD dwFlags = m_CrashInfo.m_dwScreenshotFlags;

    // Determine what image format to use (JPG or PNG)
    SCREENSHOT_IMAGE_FORMAT fmt = SCREENSHOT_FORMAT_PNG;

    if((dwFlags&CR_AS_USE_JPEG_FORMAT)!=0)
        fmt = SCREENSHOT_FORMAT_JPG;

    // Determine what to use - color or grayscale
    BOOL bGrayscale = (dwFlags&CR_AS_GRAYSCALE_IMAGE)!=0;

    std::vector<CRect> wnd_list; // List of window handles

    if((dwFlags&CR_AS_MAIN_WINDOW)!=0) // We need to capture the main window
    {     
        // Take screenshot of the main window
        std::vector<WindowInfo> aWindows; 
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_CrashInfo.m_dwProcessId);
        if(hProcess!=NULL)
        {
            sc.FindWindows(hProcess, FALSE, &aWindows);
            CloseHandle(hProcess);
        }
        if(aWindows.size()>0)
        {
            wnd_list.push_back(aWindows[0].m_rcWnd);
            ssi.m_aWindows.push_back(aWindows[0]);
        }
    }
    else if((dwFlags&CR_AS_PROCESS_WINDOWS)!=0) // Capture all process windows
    {          
        std::vector<WindowInfo> aWindows; 
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_CrashInfo.m_dwProcessId);
        if(hProcess!=NULL)
        {
            sc.FindWindows(hProcess, TRUE, &aWindows);
            CloseHandle(hProcess);
        }

        int i;
        for(i=0; i<(int)aWindows.size(); i++)
            wnd_list.push_back(aWindows[i].m_rcWnd);
        ssi.m_aWindows = aWindows;
    }
    else // (dwFlags&CR_AS_VIRTUAL_SCREEN)!=0 // Capture the virtual screen
    {
        // Take screenshot of the entire desktop
        CRect rcScreen;
        sc.GetScreenRect(&rcScreen);    
        wnd_list.push_back(rcScreen);
    }

    ssi.m_bValid = TRUE;
    sc.GetScreenRect(&ssi.m_rcVirtualScreen);  

	// Take the screen shot
    BOOL bTakeScreenshot = sc.CaptureScreenRect(wnd_list, 
        m_CrashInfo.GetReport(m_nCurReport).m_sErrorReportDirName, 
        0, fmt, m_CrashInfo.m_nJpegQuality, bGrayscale, 
        ssi.m_aMonitors, screenshot_names);
    if(bTakeScreenshot==FALSE)
    {
        return FALSE;
    }

    m_CrashInfo.GetReport(0).m_ScreenshotInfo = ssi;

    // Prepare the list of screenshot files we will add to the error report
    std::vector<ERIFileItem> FilesToAdd;
    size_t i;
    for(i=0; i<screenshot_names.size(); i++)
    {
        CString sDestFile;
        int nSlashPos = screenshot_names[i].ReverseFind('\\');
        sDestFile = screenshot_names[i].Mid(nSlashPos+1);
        ERIFileItem fi;
        fi.m_sSrcFile = screenshot_names[i];
        fi.m_sDestFile = sDestFile;
        fi.m_sDesc = Utility::GetINIString(m_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("DescScreenshot"));    
        m_CrashInfo.GetReport(0).m_FileItems[fi.m_sDestFile] = fi;
    }

    // Done
    return TRUE;
}

// This callbask function is called by MinidumpWriteDump
BOOL CALLBACK CErrorReportSender::MiniDumpCallback(
    PVOID CallbackParam,
    PMINIDUMP_CALLBACK_INPUT CallbackInput,
    PMINIDUMP_CALLBACK_OUTPUT CallbackOutput )
{
    // Delegate back to the CErrorReportSender
    CErrorReportSender* pErrorReportSender = (CErrorReportSender*)CallbackParam;  
    return pErrorReportSender->OnMinidumpProgress(CallbackInput, CallbackOutput);  
}

// This method is called when MinidumpWriteDump notifies us about
// currently performed action
BOOL CErrorReportSender::OnMinidumpProgress(const PMINIDUMP_CALLBACK_INPUT CallbackInput,
                                            PMINIDUMP_CALLBACK_OUTPUT CallbackOutput)
{
    switch(CallbackInput->CallbackType)
    {
    case CancelCallback: 
        {
            // This callback allows to cancel minidump generation
            if(m_Assync.IsCancelled())
            {
                CallbackOutput->Cancel = TRUE;      
                m_Assync.SetProgress(_T("Dump generation cancelled by user"), 0, true);
            }
        }
        break;

    case ModuleCallback:
        {
            // We are currently dumping some module
            strconv_t strconv;
            CString sMsg;
            sMsg.Format(_T("Dumping info for module %s"), 
                strconv.w2t(CallbackInput->Module.FullPath));

			// Here we want to collect module information
			ErrorReportInfo& eri = m_CrashInfo.GetReport(0);
			if(eri.m_dwExceptionAddress!=0)
			{
				// Check if this is the module where exception has happened
				ULONG64 dwExcAddr = eri.m_dwExceptionAddress;
				if(dwExcAddr>=CallbackInput->Module.BaseOfImage && 
					dwExcAddr<=CallbackInput->Module.BaseOfImage+CallbackInput->Module.SizeOfImage)
				{
					// Save module information to the report
					eri.m_sExceptionModule = CallbackInput->Module.FullPath;
					eri.m_dwExceptionModuleBase = CallbackInput->Module.BaseOfImage;

					// Save module version info
					VS_FIXEDFILEINFO* fi = &CallbackInput->Module.VersionInfo;
					if(fi)
					{
						WORD dwVerMajor = HIWORD(fi->dwProductVersionMS);
						WORD dwVerMinor = LOWORD(fi->dwProductVersionMS);
						WORD dwPatchLevel = HIWORD(fi->dwProductVersionLS);
						WORD dwVerBuild = LOWORD(fi->dwProductVersionLS);

						eri.m_sExceptionModuleVersion.Format(_T("%u.%u.%u.%u"), 
									dwVerMajor, dwVerMinor, dwPatchLevel, dwVerBuild);  					
					}
				}
			}

			// Update progress
            m_Assync.SetProgress(sMsg, 0, true);
        }
        break;
    case ThreadCallback:
        {      
            // We are currently dumping some thread 
            CString sMsg;
            sMsg.Format(_T("Dumping info for thread 0x%X"), 
                CallbackInput->Thread.ThreadId);
            m_Assync.SetProgress(sMsg, 0, true);
        }
        break;

    }

    return TRUE;
}

// This method creates the minidump of the process
BOOL CErrorReportSender::CreateMiniDump()
{   
    BOOL bStatus = FALSE;
    HMODULE hDbgHelp = NULL;
    HANDLE hFile = NULL;
    MINIDUMP_EXCEPTION_INFORMATION mei;
    MINIDUMP_CALLBACK_INFORMATION mci;
    CString sMinidumpFile = m_CrashInfo.GetReport(m_nCurReport).
        m_sErrorReportDirName + _T("\\crashdump.dmp");
    std::vector<ERIFileItem> files_to_add;
    ERIFileItem fi;
    CString sErrorMsg;

	// Check our config - should we generate the minidump or not?
    if(m_CrashInfo.m_bGenerateMinidump==FALSE)
    {
        m_Assync.SetProgress(_T("Crash dump generation disabled; skipping."), 0, false);
        return TRUE;
    }

	// Update progress
    m_Assync.SetProgress(_T("Creating crash dump file..."), 0, false);
    m_Assync.SetProgress(_T("[creating_dump]"), 0, false);

    // Load dbghelp.dll
    hDbgHelp = LoadLibrary(m_CrashInfo.m_sDbgHelpPath);
    if(hDbgHelp==NULL)
    {
        // Try again ... fallback to dbghelp.dll in path
        const CString sDebugHelpDLL_name = "dbghelp.dll";
        hDbgHelp = LoadLibrary(sDebugHelpDLL_name);    
    }

    if(hDbgHelp==NULL)
    {
        sErrorMsg = _T("dbghelp.dll couldn't be loaded");
        m_Assync.SetProgress(_T("dbghelp.dll couldn't be loaded."), 0, false);
        goto cleanup;
    }

    // Create the minidump file
    hFile = CreateFile(
        sMinidumpFile,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

	// Check if file has been created
    if(hFile==INVALID_HANDLE_VALUE)
    {
        DWORD dwError = GetLastError();
        CString sMsg;    
        sMsg.Format(_T("Couldn't create minidump file: %s"), 
            Utility::FormatErrorMsg(dwError));
        m_Assync.SetProgress(sMsg, 0, false);
        sErrorMsg = sMsg;
        return FALSE;
    }

    // Set valid dbghelp API version  
    typedef LPAPI_VERSION (WINAPI* LPIMAGEHLPAPIVERSIONEX)(LPAPI_VERSION AppVersion);  
    LPIMAGEHLPAPIVERSIONEX lpImagehlpApiVersionEx = 
        (LPIMAGEHLPAPIVERSIONEX)GetProcAddress(hDbgHelp, "ImagehlpApiVersionEx");
    ATLASSERT(lpImagehlpApiVersionEx!=NULL);
    if(lpImagehlpApiVersionEx!=NULL)
    {    
        API_VERSION CompiledApiVer;
        CompiledApiVer.MajorVersion = 6;
        CompiledApiVer.MinorVersion = 1;
        CompiledApiVer.Revision = 11;    
        CompiledApiVer.Reserved = 0;
        LPAPI_VERSION pActualApiVer = lpImagehlpApiVersionEx(&CompiledApiVer);    
        pActualApiVer;
        ATLASSERT(CompiledApiVer.MajorVersion==pActualApiVer->MajorVersion);
        ATLASSERT(CompiledApiVer.MinorVersion==pActualApiVer->MinorVersion);
        ATLASSERT(CompiledApiVer.Revision==pActualApiVer->Revision);    
    }

    // Write minidump to the file
    mei.ThreadId = m_CrashInfo.m_dwThreadId;
    mei.ExceptionPointers = m_CrashInfo.m_pExInfo;
    mei.ClientPointers = TRUE;

    mci.CallbackRoutine = MiniDumpCallback;
    mci.CallbackParam = this;

    typedef BOOL (WINAPI *LPMINIDUMPWRITEDUMP)(
        HANDLE hProcess, 
        DWORD ProcessId, 
        HANDLE hFile, 
        MINIDUMP_TYPE DumpType, 
        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
        CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam, 
        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

	// Get address of MiniDumpWirteDump function
    LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump = 
        (LPMINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    if(!pfnMiniDumpWriteDump)
    {    
        m_Assync.SetProgress(_T("Bad MiniDumpWriteDump function."), 0, false);
        sErrorMsg = _T("Bad MiniDumpWriteDump function");
        return FALSE;
    }

	// Open client process
    HANDLE hProcess = OpenProcess(
        PROCESS_ALL_ACCESS, 
        FALSE, 
        m_CrashInfo.m_dwProcessId);

	// Now actually write the minidump
    BOOL bWriteDump = pfnMiniDumpWriteDump(
        hProcess,
        m_CrashInfo.m_dwProcessId,
        hFile,
        m_CrashInfo.m_MinidumpType,
        &mei,
        NULL,
        &mci);

	// Check result
    if(!bWriteDump)
    {    
        CString sMsg = Utility::FormatErrorMsg(GetLastError());
        m_Assync.SetProgress(_T("Error writing dump."), 0, false);
        m_Assync.SetProgress(sMsg, 0, false);
        sErrorMsg = sMsg;
        goto cleanup;
    }

	// Update progress
    bStatus = TRUE;
    m_Assync.SetProgress(_T("Finished creating dump."), 100, false);

cleanup:

    // Close file
    if(hFile)
        CloseHandle(hFile);

    // Unload dbghelp.dll
    if(hDbgHelp)
        FreeLibrary(hDbgHelp);

	// Add the minidump file to error report
    fi.m_bMakeCopy = false;
    fi.m_sDesc = Utility::GetINIString(m_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("DescCrashDump"));
    fi.m_sDestFile = _T("crashdump.dmp");
    fi.m_sSrcFile = sMinidumpFile;
    fi.m_sErrorStatus = sErrorMsg;
    files_to_add.push_back(fi);

    // Add file to the list
    m_CrashInfo.GetReport(0).m_FileItems[fi.m_sDestFile] = fi;
	
    return bStatus;
}

// This method adds an element to XML file
void CErrorReportSender::AddElemToXML(CString sName, CString sValue, TiXmlNode* root)
{
    strconv_t strconv;
    TiXmlHandle hElem = new TiXmlElement(strconv.t2utf8(sName));
    root->LinkEndChild(hElem.ToNode());
    TiXmlText* text = new TiXmlText(strconv.t2utf8(sValue));
    hElem.ToElement()->LinkEndChild(text);
}

// This method generates an XML file describing the crash
BOOL CErrorReportSender::CreateCrashDescriptionXML(ErrorReportInfo& eri)
{
    BOOL bStatus = FALSE;
    ERIFileItem fi;
    CString sFileName = eri.m_sErrorReportDirName + _T("\\crashrpt.xml");
    CString sErrorMsg;
    strconv_t strconv;
    TiXmlDocument doc;
    FILE* f = NULL; 
    CString sNum;
    CString sCrashRptVer;
    CString sOSIs64Bit;
    CString sExceptionType;

    fi.m_bMakeCopy = false;
    fi.m_sDesc = Utility::GetINIString(m_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("DescXML"));
    fi.m_sDestFile = _T("crashrpt.xml");
    fi.m_sSrcFile = sFileName;
    fi.m_sErrorStatus = sErrorMsg;  
    // Add this file to the list
    eri.m_FileItems[fi.m_sDestFile] = fi;

    TiXmlNode* root = root = new TiXmlElement("CrashRpt");
    doc.LinkEndChild(root);  
    sCrashRptVer.Format(_T("%d"), CRASHRPT_VER);
    TiXmlHandle(root).ToElement()->SetAttribute("version", strconv.t2utf8(sCrashRptVer));

    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
    doc.InsertBeforeChild(root, *decl);
	
    AddElemToXML(_T("CrashGUID"), eri.m_sCrashGUID, root);
    AddElemToXML(_T("AppName"), eri.m_sAppName, root);
    AddElemToXML(_T("AppVersion"), eri.m_sAppVersion, root);  
    AddElemToXML(_T("ImageName"), eri.m_sImageName, root);
    AddElemToXML(_T("OperatingSystem"), eri.m_sOSName, root);


    sOSIs64Bit.Format(_T("%d"), eri.m_bOSIs64Bit);
    AddElemToXML(_T("OSIs64Bit"), sOSIs64Bit, root);

    AddElemToXML(_T("GeoLocation"), eri.m_sGeoLocation, root);
    AddElemToXML(_T("SystemTimeUTC"), eri.m_sSystemTimeUTC, root);
		
	if(eri.m_dwExceptionAddress!=0)
	{
		sNum.Format(_T("0x%I64x"), eri.m_dwExceptionAddress);
		AddElemToXML(_T("ExceptionAddress"), sNum, root);

		AddElemToXML(_T("ExceptionModule"), eri.m_sExceptionModule, root);

		sNum.Format(_T("0x%I64x"), eri.m_dwExceptionModuleBase);
		AddElemToXML(_T("ExceptionModuleBase"), sNum, root);

		AddElemToXML(_T("ExceptionModuleVersion"), eri.m_sExceptionModuleVersion, root);
	}

    sExceptionType.Format(_T("%d"), m_CrashInfo.m_nExceptionType);
    AddElemToXML(_T("ExceptionType"), sExceptionType, root);
    if(m_CrashInfo.m_nExceptionType==CR_SEH_EXCEPTION)
    {
        CString sExceptionCode;
        sExceptionCode.Format(_T("%d"), m_CrashInfo.m_dwExceptionCode);
        AddElemToXML(_T("ExceptionCode"), sExceptionCode, root);
    }
    else if(m_CrashInfo.m_nExceptionType==CR_CPP_SIGFPE)
    {
        CString sFPESubcode;
        sFPESubcode.Format(_T("%d"), m_CrashInfo.m_uFPESubcode);
        AddElemToXML(_T("FPESubcode"), sFPESubcode, root);
    }
    else if(m_CrashInfo.m_nExceptionType==CR_CPP_INVALID_PARAMETER)
    {
        AddElemToXML(_T("InvParamExpression"), m_CrashInfo.m_sInvParamExpr, root);
        AddElemToXML(_T("InvParamFunction"), m_CrashInfo.m_sInvParamFunction, root);
        AddElemToXML(_T("InvParamFile"), m_CrashInfo.m_sInvParamFile, root);

        CString sInvParamLine;
        sInvParamLine.Format(_T("%d"), m_CrashInfo.m_uInvParamLine);
        AddElemToXML(_T("InvParamLine"), sInvParamLine, root);
    }

    CString sGuiResources;
    sGuiResources.Format(_T("%d"), eri.m_dwGuiResources);
    AddElemToXML(_T("GUIResourceCount"), sGuiResources, root);

    CString sProcessHandleCount;
    sProcessHandleCount.Format(_T("%d"), eri.m_dwProcessHandleCount);
    AddElemToXML(_T("OpenHandleCount"), sProcessHandleCount, root);

    AddElemToXML(_T("MemoryUsageKbytes"), eri.m_sMemUsage, root);

    if(eri.m_ScreenshotInfo.m_bValid)
    {
        TiXmlHandle hScreenshotInfo = new TiXmlElement("ScreenshotInfo");
        root->LinkEndChild(hScreenshotInfo.ToNode());

        TiXmlHandle hVirtualScreen = new TiXmlElement("VirtualScreen");    

        sNum.Format(_T("%d"), eri.m_ScreenshotInfo.m_rcVirtualScreen.left);
        hVirtualScreen.ToElement()->SetAttribute("left", strconv.t2utf8(sNum));

        sNum.Format(_T("%d"), eri.m_ScreenshotInfo.m_rcVirtualScreen.top);
        hVirtualScreen.ToElement()->SetAttribute("top", strconv.t2utf8(sNum));

        sNum.Format(_T("%d"), eri.m_ScreenshotInfo.m_rcVirtualScreen.Width());
        hVirtualScreen.ToElement()->SetAttribute("width", strconv.t2utf8(sNum));

        sNum.Format(_T("%d"), eri.m_ScreenshotInfo.m_rcVirtualScreen.Height());
        hVirtualScreen.ToElement()->SetAttribute("height", strconv.t2utf8(sNum));

        hScreenshotInfo.ToNode()->LinkEndChild(hVirtualScreen.ToNode());

        TiXmlHandle hMonitors = new TiXmlElement("Monitors");
        hScreenshotInfo.ToElement()->LinkEndChild(hMonitors.ToNode());                  

        size_t i;
        for(i=0; i<eri.m_ScreenshotInfo.m_aMonitors.size(); i++)
        { 
            MonitorInfo& mi = eri.m_ScreenshotInfo.m_aMonitors[i];      
            TiXmlHandle hMonitor = new TiXmlElement("Monitor");

            sNum.Format(_T("%d"), mi.m_rcMonitor.left);
            hMonitor.ToElement()->SetAttribute("left", strconv.t2utf8(sNum));

            sNum.Format(_T("%d"), mi.m_rcMonitor.top);
            hMonitor.ToElement()->SetAttribute("top", strconv.t2utf8(sNum));

            sNum.Format(_T("%d"), mi.m_rcMonitor.Width());
            hMonitor.ToElement()->SetAttribute("width", strconv.t2utf8(sNum));

            sNum.Format(_T("%d"), mi.m_rcMonitor.Height());
            hMonitor.ToElement()->SetAttribute("height", strconv.t2utf8(sNum));

            hMonitor.ToElement()->SetAttribute("file", strconv.t2utf8(Utility::GetFileName(mi.m_sFileName)));

            hMonitors.ToElement()->LinkEndChild(hMonitor.ToNode());                  
        }

        TiXmlHandle hWindows = new TiXmlElement("Windows");
        hScreenshotInfo.ToElement()->LinkEndChild(hWindows.ToNode());                  

        for(i=0; i<eri.m_ScreenshotInfo.m_aWindows.size(); i++)
        { 
            WindowInfo& wi = eri.m_ScreenshotInfo.m_aWindows[i];      
            TiXmlHandle hWindow = new TiXmlElement("Window");

            sNum.Format(_T("%d"), wi.m_rcWnd.left);
            hWindow.ToElement()->SetAttribute("left", strconv.t2utf8(sNum));

            sNum.Format(_T("%d"), wi.m_rcWnd.top);
            hWindow.ToElement()->SetAttribute("top", strconv.t2utf8(sNum));

            sNum.Format(_T("%d"), wi.m_rcWnd.Width());
            hWindow.ToElement()->SetAttribute("width", strconv.t2utf8(sNum));

            sNum.Format(_T("%d"), wi.m_rcWnd.Height());
            hWindow.ToElement()->SetAttribute("height", strconv.t2utf8(sNum));

            hWindow.ToElement()->SetAttribute("title", strconv.t2utf8(wi.m_sTitle));

            hWindows.ToElement()->LinkEndChild(hWindow.ToNode());                  
        }
    }

    TiXmlHandle hCustomProps = new TiXmlElement("CustomProps");
    root->LinkEndChild(hCustomProps.ToNode());

    std::map<CString, CString>::iterator it2;
    for(it2=eri.m_Props.begin(); it2!=eri.m_Props.end(); it2++)
    { 
        TiXmlHandle hProp = new TiXmlElement("Prop");

        hProp.ToElement()->SetAttribute("name", strconv.t2utf8(it2->first));
        hProp.ToElement()->SetAttribute("value", strconv.t2utf8(it2->second));

        hCustomProps.ToElement()->LinkEndChild(hProp.ToNode());                  
    }

    TiXmlHandle hFileItems = new TiXmlElement("FileList");
    root->LinkEndChild(hFileItems.ToNode());

    std::map<CString, ERIFileItem>::iterator it;
    for(it=eri.m_FileItems.begin(); it!=eri.m_FileItems.end(); it++)
    {    
        ERIFileItem& rfi = it->second;
        TiXmlHandle hFileItem = new TiXmlElement("FileItem");

        hFileItem.ToElement()->SetAttribute("name", strconv.t2utf8(rfi.m_sDestFile));
        hFileItem.ToElement()->SetAttribute("description", strconv.t2utf8(rfi.m_sDesc));
        if(!rfi.m_sErrorStatus.IsEmpty())
            hFileItem.ToElement()->SetAttribute("error", strconv.t2utf8(rfi.m_sErrorStatus));

        hFileItems.ToElement()->LinkEndChild(hFileItem.ToNode());                  
    }

#if _MSC_VER<1400
    f = _tfopen(sFileName, _T("w"));
#else
    _tfopen_s(&f, sFileName, _T("w"));
#endif

    if(f==NULL)
    {
        sErrorMsg = _T("Error opening file for writing");
        goto cleanup;
    }

    doc.useMicrosoftBOM = true;
    bool bSave = doc.SaveFile(f); 
    if(!bSave)
    {
        sErrorMsg = doc.ErrorDesc();
        goto cleanup;
    }

    fclose(f);
    f = NULL;

    bStatus = TRUE;

cleanup:

    if(f)
        fclose(f);

    if(!bStatus)
    {
        eri.m_FileItems[fi.m_sDestFile].m_sErrorStatus = sErrorMsg;
    }

    return bStatus;
}

// This method collects user-specified files
BOOL CErrorReportSender::CollectCrashFiles()
{ 
    BOOL bStatus = FALSE;
    CString str;
    CString sErrorReportDir = m_CrashInfo.GetReport(m_nCurReport).m_sErrorReportDirName;
    CString sSrcFile;
    CString sDestFile;
    HANDLE hSrcFile = INVALID_HANDLE_VALUE;
    HANDLE hDestFile = INVALID_HANDLE_VALUE;
    LARGE_INTEGER lFileSize;
    BOOL bGetSize = FALSE;
    LPBYTE buffer[1024];
    LARGE_INTEGER lTotalWritten;
    DWORD dwBytesRead=0;
    DWORD dwBytesWritten=0;
    BOOL bRead = FALSE;
    BOOL bWrite = FALSE;
    std::map<CString, CString>::iterator rit;

    // Copy application-defined files that should be copied on crash
    m_Assync.SetProgress(_T("[copying_files]"), 0, false);

	// Walk through error report files
    std::map<CString, ERIFileItem>::iterator it;
    for(it=m_CrashInfo.GetReport(m_nCurReport).m_FileItems.begin(); it!=m_CrashInfo.GetReport(m_nCurReport).m_FileItems.end(); it++)
    {
		// Check if operation was cancelled by user
        if(m_Assync.IsCancelled())
            goto cleanup;

		// If we should make a copy of the file
        if(it->second.m_bMakeCopy)
        {
            str.Format(_T("Copying file %s."), it->second.m_sSrcFile);
            m_Assync.SetProgress(str, 0, false);

            hSrcFile = CreateFile(it->second.m_sSrcFile, GENERIC_READ, 
                FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if(hSrcFile==INVALID_HANDLE_VALUE)
            {
                it->second.m_sErrorStatus = Utility::FormatErrorMsg(GetLastError());
                str.Format(_T("Error opening file %s."), it->second.m_sSrcFile);
                m_Assync.SetProgress(str, 0, false);
            }

            bGetSize = GetFileSizeEx(hSrcFile, &lFileSize);
            if(!bGetSize)
            {
                it->second.m_sErrorStatus = Utility::FormatErrorMsg(GetLastError());
                str.Format(_T("Couldn't get file size of %s"), it->second.m_sSrcFile);
                m_Assync.SetProgress(str, 0, false);
                CloseHandle(hSrcFile);
                hSrcFile = INVALID_HANDLE_VALUE;
                continue;
            }

            sDestFile = sErrorReportDir + _T("\\") + it->second.m_sDestFile;

            hDestFile = CreateFile(sDestFile, GENERIC_WRITE, 
                FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
            if(hDestFile==INVALID_HANDLE_VALUE)
            {
                it->second.m_sErrorStatus = Utility::FormatErrorMsg(GetLastError());
                str.Format(_T("Error creating file %s."), sDestFile);
                m_Assync.SetProgress(str, 0, false);
                CloseHandle(hSrcFile);
                hSrcFile = INVALID_HANDLE_VALUE;
                continue;
            }

            lTotalWritten.QuadPart = 0;

            for(;;)
            {        
                if(m_Assync.IsCancelled())
                    goto cleanup;

                bRead = ReadFile(hSrcFile, buffer, 1024, &dwBytesRead, NULL);
                if(!bRead || dwBytesRead==0)
                    break;

                bWrite = WriteFile(hDestFile, buffer, dwBytesRead, &dwBytesWritten, NULL);
                if(!bWrite || dwBytesRead!=dwBytesWritten)
                    break;

                lTotalWritten.QuadPart += dwBytesWritten;

                int nProgress = (int)(100.0f*lTotalWritten.QuadPart/lFileSize.QuadPart);

                m_Assync.SetProgress(nProgress, false);
            }

            CloseHandle(hSrcFile);
            hSrcFile = INVALID_HANDLE_VALUE;
            CloseHandle(hDestFile);
            hDestFile = INVALID_HANDLE_VALUE;
        }
    }

    // Create dump of registry keys

    ErrorReportInfo& eri = m_CrashInfo.GetReport(m_nCurReport);  
    if(eri.m_RegKeys.size()!=0)
    {
        m_Assync.SetProgress(_T("Dumping registry keys..."), 0, false);    
    }

	// Walk through our registry key list
    for(rit=eri.m_RegKeys.begin(); rit!=eri.m_RegKeys.end(); rit++)
    {
        if(m_Assync.IsCancelled())
            goto cleanup;

        CString sFileName = eri.m_sErrorReportDirName + _T("\\") + rit->second;

        str.Format(_T("Dumping registry key '%s' to file '%s' "), rit->first, sFileName);
        m_Assync.SetProgress(str, 0, false);    

        // Create registry key dump
        CString sErrorMsg;
        DumpRegKey(rit->first, sFileName, sErrorMsg);
        ERIFileItem fi;
        fi.m_sSrcFile = sFileName;
        fi.m_sDestFile = rit->second;
        fi.m_sDesc = Utility::GetINIString(m_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("DescRegKey"));
        fi.m_bMakeCopy = FALSE;
        fi.m_sErrorStatus = sErrorMsg;
        std::vector<ERIFileItem> file_list;
        file_list.push_back(fi);
        // Add file to the list of file items
        m_CrashInfo.GetReport(0).m_FileItems[fi.m_sDestFile] = fi;
    }

    // Create crash description XML
    CreateCrashDescriptionXML(m_CrashInfo.GetReport(0));

    // Success
    bStatus = TRUE;

cleanup:

	// Clean up

    if(hSrcFile!=INVALID_HANDLE_VALUE)
        CloseHandle(hSrcFile);

    if(hDestFile!=INVALID_HANDLE_VALUE)
        CloseHandle(hDestFile);

    m_Assync.SetProgress(_T("Finished copying files."), 100, false);

    return 0;
}

// This method dumps a registry key contents to an XML file
int CErrorReportSender::DumpRegKey(CString sRegKey, CString sDestFile, CString& sErrorMsg)
{
    strconv_t strconv;
    TiXmlDocument document; 

    // Load document if file already exists
    // otherwise create new document.

    FILE* f = NULL;
#if _MSC_VER<1400
    f = _tfopen(sDestFile, _T("rb"));
#else
    _tfopen_s(&f, sDestFile, _T("rb"));
#endif
    if(f!=NULL)
    { 
        document.LoadFile(f);
        fclose(f);
        f = NULL;
    }  

    TiXmlHandle hdoc(&document);

    TiXmlElement* registry = document.RootElement();

    if(registry==NULL)
    {
        registry = new TiXmlElement("registry");
        document.LinkEndChild(registry);
    }

    TiXmlNode* declaration = hdoc.Child(0).ToNode();
    if(declaration==NULL || declaration->Type()!=TiXmlNode::TINYXML_DECLARATION)
    {
        TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
        document.InsertBeforeChild(registry, *decl);
    }   

    DumpRegKey(NULL, sRegKey, registry);

#if _MSC_VER<1400
    f = _tfopen(sDestFile, _T("wb"));
#else
    _tfopen_s(&f, sDestFile, _T("wb"));
#endif
    if(f==NULL)
    {
        sErrorMsg = _T("Error opening file for writing.");
        return 1;
    }

    bool bSave = document.SaveFile(f);

    fclose(f);

    if(!bSave)
    {
        sErrorMsg = _T("Error saving XML document to file: ");
        sErrorMsg += document.ErrorDesc();
    }

    return (bSave==true)?0:1;
}

int CErrorReportSender::DumpRegKey(HKEY hParentKey, CString sSubKey, TiXmlElement* elem)
{
    strconv_t strconv;
    HKEY hKey = NULL;  

    if(hParentKey==NULL)
    {    
        int nSkip = 0;
        if(sSubKey.Left(19).Compare(_T("HKEY_LOCAL_MACHINE\\"))==0)
        {
            hKey = HKEY_LOCAL_MACHINE;
            nSkip = 18;
        }
        else if(sSubKey.Left(18).Compare(_T("HKEY_CURRENT_USER\\"))==0)
        {
            hKey = HKEY_CURRENT_USER;
            nSkip = 17;
        }    
        else 
        {
            return 1; // Invalid key.
        }

        CString sKey = sSubKey.Mid(0, nSkip);
        LPCSTR szKey = strconv.t2utf8(sKey);
        sSubKey = sSubKey.Mid(nSkip+1);

        TiXmlHandle key_node = elem->FirstChild(szKey);
        if(key_node.ToElement()==NULL)
        {
            key_node = new TiXmlElement("k");
            elem->LinkEndChild(key_node.ToNode());
            key_node.ToElement()->SetAttribute("name", szKey);
        }

        DumpRegKey(hKey, sSubKey, key_node.ToElement());
    }
    else
    {
        int pos = sSubKey.Find('\\');
        CString sKey = sSubKey;
        if(pos>0)
            sKey = sSubKey.Mid(0, pos);
        LPCSTR szKey = strconv.t2utf8(sKey);

        TiXmlHandle key_node = elem->FirstChild(szKey);
        if(key_node.ToElement()==NULL)
        {
            key_node = new TiXmlElement("k");
            elem->LinkEndChild(key_node.ToNode());
            key_node.ToElement()->SetAttribute("name", szKey);
        }

        if(ERROR_SUCCESS==RegOpenKeyEx(hParentKey, sKey, 0, GENERIC_READ, &hKey))
        {
            if(pos>0)
            {
                sSubKey = sSubKey.Mid(pos+1);
                DumpRegKey(hKey, sSubKey, key_node.ToElement());
            }
            else
            {
                DWORD dwSubKeys = 0;
                DWORD dwMaxSubKey = 0;
                DWORD dwValues = 0;
                DWORD dwMaxValueNameLen = 0;
                DWORD dwMaxValueLen = 0;
                LONG lResult = RegQueryInfoKey(hKey, NULL, 0, 0, &dwSubKeys, &dwMaxSubKey, 
                    0, &dwValues, &dwMaxValueNameLen, &dwMaxValueLen, NULL, NULL); 
                if(lResult==ERROR_SUCCESS)
                {
                    // Enumerate and dump subkeys          
                    int i;
                    for(i=0; i<(int)dwSubKeys; i++)
                    { 
                        LPWSTR szName = new WCHAR[dwMaxSubKey];
                        DWORD dwLen = dwMaxSubKey;
                        lResult = RegEnumKeyEx(hKey, i, szName, &dwLen, 0, NULL, 0, NULL);
                        if(lResult==ERROR_SUCCESS)
                        {
                            DumpRegKey(hKey, CString(szName), key_node.ToElement());
                        }

                        delete [] szName;
                    }         

                    // Dump key values 
                    for(i=0; i<(int)dwValues; i++)
                    { 
                        LPWSTR szName = new WCHAR[dwMaxValueNameLen];
                        LPBYTE pData = new BYTE[dwMaxValueLen];
                        DWORD dwNameLen = dwMaxValueNameLen;
                        DWORD dwValueLen = dwMaxValueLen;
                        DWORD dwType = 0;

                        lResult = RegEnumValue(hKey, i, szName, &dwNameLen, 0, &dwType, pData, &dwValueLen);
                        if(lResult==ERROR_SUCCESS)
                        {
                            TiXmlHandle val_node = key_node.ToElement()->FirstChild(strconv.w2utf8(szName));
                            if(val_node.ToElement()==NULL)
                            {
                                val_node = new TiXmlElement("v");
                                key_node.ToElement()->LinkEndChild(val_node.ToNode());
                            }

                            val_node.ToElement()->SetAttribute("name", strconv.w2utf8(szName));

                            char str[128] = "";
                            LPSTR szType = NULL;
                            if(dwType==REG_BINARY)
                                szType = "REG_BINARY";
                            else if(dwType==REG_DWORD)
                                szType = "REG_DWORD";
                            else if(dwType==REG_EXPAND_SZ)
                                szType = "REG_EXPAND_SZ";
                            else if(dwType==REG_MULTI_SZ)
                                szType = "REG_MULTI_SZ";
                            else if(dwType==REG_QWORD)
                                szType = "REG_QWORD";
                            else if(dwType==REG_SZ)
                                szType = "REG_SZ";
                            else 
                            {
#if _MSC_VER<1400
                                sprintf(str, "Unknown type (0x%08x)", dwType);
#else
                                sprintf_s(str, 128, "Unknown type (0x%08x)", dwType);
#endif
                                szType = str;                
                            }

                            val_node.ToElement()->SetAttribute("type", szType);              

                            if(dwType==REG_BINARY)
                            {
                                std::string str2;
                                int j;
                                for(j=0; j<(int)dwValueLen; j++)
                                {
                                    char num[10];
#if _MSC_VER<1400
                                    sprintf(num, "%02X", pData[i]);
#else
                                    sprintf_s(num, 10, "%02X", pData[i]);
#endif
                                    str2 += num;
                                    if(j<(int)dwValueLen)
                                        str2 += " ";
                                }

                                val_node.ToElement()->SetAttribute("value", str2.c_str());
                            }
                            else if(dwType==REG_DWORD)
                            {
                                LPDWORD pdwValue = (LPDWORD)pData;
                                char str3[64]="";
#if _MSC_VER<1400
                                sprintf(str3, "0x%08x (%lu)", *pdwValue, *pdwValue);                
#else
                                sprintf_s(str3, 64, "0x%08x (%lu)", *pdwValue, *pdwValue);                
#endif
                                val_node.ToElement()->SetAttribute("value", str3);
                            }
                            else if(dwType==REG_SZ || dwType==REG_EXPAND_SZ)
                            {
                                LPCSTR szValue = strconv.t2utf8((LPCTSTR)pData);
                                val_node.ToElement()->SetAttribute("value", szValue);                
                            }
                            else if(dwType==REG_MULTI_SZ)
                            {                
                                LPCTSTR szValues = (LPCTSTR)pData;
                                int prev = 0;
                                int pos2 = 0;
                                for(;;)
                                {
                                    if(szValues[pos2]==0)
                                    {
                                        CString sValue = CString(szValues+prev, pos2-prev);
                                        LPCSTR szValue = strconv.t2utf8(sValue);

                                        TiXmlHandle str_node = new TiXmlElement("str");
                                        val_node.ToElement()->LinkEndChild(str_node.ToNode());                    
                                        str_node.ToElement()->SetAttribute("value", szValue);              

                                        prev = pos2+1;
                                    }

                                    if(szValues[pos2]==0 && szValues[pos2+1]==0)
                                        break; // Double-null

                                    pos2++;
                                }                     
                            }
                        }

                        delete [] szName;
                        delete [] pData;
                    }         
                }
            }

            RegCloseKey(hKey);
        }
        else
        {      
            CString sErrMsg = Utility::FormatErrorMsg(GetLastError());
            LPCSTR szErrMsg = strconv.t2utf8(sErrMsg);
            key_node.ToElement()->SetAttribute("error", szErrMsg);
        }
    }

    return 0;
}

// This method calculates an MD5 hash for the file
int CErrorReportSender::CalcFileMD5Hash(CString sFileName, CString& sMD5Hash)
{
    FILE* f = NULL;  // Handle to file
    BYTE buff[512];  // Read buffer
    MD5 md5;         // MD5 hash
    MD5_CTX md5_ctx; // MD5 context
    unsigned char md5_hash[16]; // MD5 hash as sequence of bytes
    int i;

	// Clear output
    sMD5Hash.Empty();

	// Open file
#if _MSC_VER<1400
    f = _tfopen(sFileName.GetBuffer(0), _T("rb"));
#else
    _tfopen_s(&f, sFileName.GetBuffer(0), _T("rb"));
#endif

	// Check if file has been opened
    if(f==NULL) 
        return -1;

	// Init MD5 context
    md5.MD5Init(&md5_ctx);

	// Read file contents and update MD5 hash as each portion is being read
    while(!feof(f))
    {
        size_t count = fread(buff, 1, 512, f);
        if(count>0)
        {
            md5.MD5Update(&md5_ctx, buff, (unsigned int)count);
        }
    }

	// Close file
    fclose(f);

	// Finalize MD5 hash calculation
    md5.MD5Final(md5_hash, &md5_ctx);

	// Format hash as a string
    for(i=0; i<16; i++)
    {
        CString number;
        number.Format(_T("%02x"), md5_hash[i]);
        sMD5Hash += number;
    }

	// Done
    return 0;
}

// This method restarts the client application
BOOL CErrorReportSender::RestartApp()
{
	// Check our config - if we should restart the client app or not?
    if(m_CrashInfo.m_bAppRestart==FALSE)
        return FALSE; // No need to restart

	// Add a message to log and reset progress 
    m_Assync.SetProgress(_T("Restarting the application..."), 0, false);

	// Set up process start up info
    STARTUPINFO si;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

	// Set up process information
    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(PROCESS_INFORMATION));  

	// Format command line
    CString sCmdLine;
    if(m_CrashInfo.m_sRestartCmdLine.IsEmpty())
    {
        // Format with double quotes to avoid first empty parameter
        sCmdLine.Format(_T("\"%s\""), m_CrashInfo.GetReport(m_nCurReport).m_sImageName);
    }
    else
    {
		// Format with double quotes to avoid first empty parameters
        sCmdLine.Format(_T("\"%s\" %s"), m_CrashInfo.GetReport(m_nCurReport).m_sImageName, 
            m_CrashInfo.m_sRestartCmdLine.GetBuffer(0));
    }

	// Create process using the command line prepared earlier
    BOOL bCreateProcess = CreateProcess(
        m_CrashInfo.GetReport(m_nCurReport).m_sImageName, sCmdLine.GetBuffer(0), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    
	// The following is to avoid a handle leak
	if(pi.hProcess)
    {
        CloseHandle(pi.hProcess);
        pi.hProcess = NULL;
    }

	// The following is to avoid a handle leak
    if(pi.hThread)
    {
        CloseHandle(pi.hThread);
        pi.hThread = NULL;
    }

	// Check if process was created
    if(!bCreateProcess)
    {    
		// Add error message
        m_Assync.SetProgress(_T("Error restarting the application!"), 0, false);
        return FALSE;
    }

	// Success
    m_Assync.SetProgress(_T("Application restarted OK."), 0, false);
    return TRUE;
}

// This method calculates the total size of files included into error report
LONG64 CErrorReportSender::GetUncompressedReportSize(ErrorReportInfo& eri)
{
	// Add a message to log and reset progress
    m_Assync.SetProgress(_T("Calculating total size of files to compress..."), 0, false);

    LONG64 lTotalSize = 0;
    std::map<CString, ERIFileItem>::iterator it;
    HANDLE hFile = INVALID_HANDLE_VALUE;  
    CString sMsg;
    BOOL bGetSize = FALSE;
    LARGE_INTEGER lFileSize;

	// Enumerate files contained in the error report
    for(it=eri.m_FileItems.begin(); it!=eri.m_FileItems.end(); it++)
    {    
		// Check if operation cancelled by user
        if(m_Assync.IsCancelled())    
            return 0;

		// Get name of the file
        CString sFileName = it->second.m_sSrcFile.GetBuffer(0);
		// Open file for reading
        hFile = CreateFile(sFileName, 
            GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL); 
        if(hFile==INVALID_HANDLE_VALUE)
        {
            sMsg.Format(_T("Couldn't open file %s"), sFileName);
            m_Assync.SetProgress(sMsg, 0, false);
            continue;
        }

		// Get file size
        bGetSize = GetFileSizeEx(hFile, &lFileSize);
        if(!bGetSize)
        {
            sMsg.Format(_T("Couldn't get file size of %s"), sFileName);
            m_Assync.SetProgress(sMsg, 0, false);
            CloseHandle(hFile);
            continue;
        }

		// Update totals
        lTotalSize += lFileSize.QuadPart;

		// Close file
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

	// Return total file size
    return lTotalSize;
}

// This method compresses the files contained in the report and produces a ZIP archive.
BOOL CErrorReportSender::CompressReportFiles(ErrorReportInfo& eri)
{ 
    BOOL bStatus = FALSE;
    strconv_t strconv;
    zipFile hZip = NULL;
    CString sMsg;
    LONG64 lTotalSize = 0;
    LONG64 lTotalCompressed = 0;
    BYTE buff[1024];
    DWORD dwBytesRead=0;
    HANDLE hFile = INVALID_HANDLE_VALUE;  
    std::map<CString, ERIFileItem>::iterator it;
    FILE* f = NULL;
    CString sMD5Hash;

	// Add a different log message depending on the current mode.
    if(m_bExport)
        m_Assync.SetProgress(_T("[exporting_report]"), 0, false);
    else
        m_Assync.SetProgress(_T("[compressing_files]"), 0, false);

	// Calculate the total size of error report files
    lTotalSize = GetUncompressedReportSize(eri);

	// Add a message to log
    sMsg.Format(_T("Total file size for compression is %I64d bytes"), lTotalSize);
    m_Assync.SetProgress(sMsg, 0, false);

	// Determine what name to use for the output ZIP archive file.
    if(m_bExport)
        m_sZipName = m_sExportFileName;  
    else
        m_sZipName = eri.m_sErrorReportDirName + _T(".zip");  

	// Update progress
    sMsg.Format(_T("Creating ZIP archive file %s"), m_sZipName);
    m_Assync.SetProgress(sMsg, 1, false);

	// Create ZIP archive
    hZip = zipOpen((const char*)m_sZipName.GetBuffer(0), APPEND_STATUS_CREATE);
    if(hZip==NULL)
    {
        m_Assync.SetProgress(_T("Failed to create ZIP file."), 100, true);
        goto cleanup;
    }

	// Enumerate files contained in the report
    for(it=eri.m_FileItems.begin(); it!=eri.m_FileItems.end(); it++)
    { 
		// Check if the operation was cancelled by user
        if(m_Assync.IsCancelled())    
            goto cleanup;

		// Define destination file name in ZIP archive
        CString sDstFileName = it->second.m_sDestFile.GetBuffer(0);
		// Define source file name
        CString sFileName = it->second.m_sSrcFile.GetBuffer(0);
		// Define file description
        CString sDesc = it->second.m_sDesc;

		// Update progress
        sMsg.Format(_T("Compressing file %s"), sDstFileName);
        m_Assync.SetProgress(sMsg, 0, false);

		// Open file for reading
        hFile = CreateFile(sFileName, 
            GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL); 
        if(hFile==INVALID_HANDLE_VALUE)
        {
            sMsg.Format(_T("Couldn't open file %s"), sFileName);
            m_Assync.SetProgress(sMsg, 0, false);
            continue;
        }

		// Get file information.
        BY_HANDLE_FILE_INFORMATION fi;
        GetFileInformationByHandle(hFile, &fi);

		// Convert file creation time to system file time.
        SYSTEMTIME st;
        FileTimeToSystemTime(&fi.ftCreationTime, &st);

		// Fill in the ZIP file info
        zip_fileinfo info;
        info.dosDate = 0;
        info.tmz_date.tm_year = st.wYear;
        info.tmz_date.tm_mon = st.wMonth-1;
        info.tmz_date.tm_mday = st.wDay;
        info.tmz_date.tm_hour = st.wHour;
        info.tmz_date.tm_min = st.wMinute;
        info.tmz_date.tm_sec = st.wSecond;
        info.external_fa = FILE_ATTRIBUTE_NORMAL;
        info.internal_fa = FILE_ATTRIBUTE_NORMAL;

		// Create new file inside of our ZIP archive
        int n = zipOpenNewFileInZip( hZip, (const char*)strconv.t2a(sDstFileName.GetBuffer(0)), &info,
            NULL, 0, NULL, 0, strconv.t2a(sDesc), Z_DEFLATED, Z_DEFAULT_COMPRESSION);
        if(n!=0)
        {
            sMsg.Format(_T("Couldn't compress file %s"), sDstFileName);
            m_Assync.SetProgress(sMsg, 0, false);
            continue;
        }

		// Read source file contents and write it to ZIP archive
        for(;;)
        {
			// Check if operation was cancelled by user
            if(m_Assync.IsCancelled())    
                goto cleanup;

			// Read a portion of source file
            BOOL bRead = ReadFile(hFile, buff, 1024, &dwBytesRead, NULL);
            if(!bRead || dwBytesRead==0)
                break;

			// Write a portion into destination file
            int res = zipWriteInFileInZip(hZip, buff, dwBytesRead);
            if(res!=0)
            {
                zipCloseFileInZip(hZip);
                sMsg.Format(_T("Couldn't write to compressed file %s"), sDstFileName);
                m_Assync.SetProgress(sMsg, 0, false);        
                break;
            }

			// Update totals
            lTotalCompressed += dwBytesRead;

			// Update progress
            float fProgress = 100.0f*lTotalCompressed/lTotalSize;
            m_Assync.SetProgress((int)fProgress, false);
        }

		// Close file
        zipCloseFileInZip(hZip);
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

	// Close ZIP archive
    if(hZip!=NULL)
    {
        zipClose(hZip, NULL);
        hZip = NULL;
    }

    // Save MD5 hash file
    if(!m_bExport)
    {
        sMsg.Format(_T("Calculating MD5 hash for file %s"), m_sZipName);
        m_Assync.SetProgress(sMsg, 0, false);

        int nCalcMD5 = CalcFileMD5Hash(m_sZipName, sMD5Hash);
        if(nCalcMD5!=0)
        {
            sMsg.Format(_T("Couldn't calculate MD5 hash for file %s"), m_sZipName);
            m_Assync.SetProgress(sMsg, 0, false);
            goto cleanup;
        }

#if _MSC_VER <1400
        f = _tfopen(m_sZipName + _T(".md5"), _T("wt"));
#else
        _tfopen_s(&f, m_sZipName + _T(".md5"), _T("wt"));
#endif
        if(f==NULL)
        {
            sMsg.Format(_T("Couldn't save MD5 hash for file %s"), m_sZipName);
            m_Assync.SetProgress(sMsg, 0, false);
            goto cleanup;
        }

        _ftprintf(f, sMD5Hash);
        fclose(f);
        f = NULL;
    }

	// Check if totals match
    if(lTotalSize==lTotalCompressed)
        bStatus = TRUE;

cleanup:

	// Clean up

    if(hZip!=NULL)
        zipClose(hZip, NULL);

    if(hFile!=INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    if(f!=NULL)
        fclose(f);

    if(bStatus)
        m_Assync.SetProgress(_T("Finished compressing files...OK"), 100, true);
    else
        m_Assync.SetProgress(_T("File compression failed."), 100, true);

    if(m_bExport)
    {
        if(bStatus)
            m_Assync.SetProgress(_T("[end_exporting_report_ok]"), 100, false);    
        else
            m_Assync.SetProgress(_T("[end_exporting_report_failed]"), 100, false);    
    }
    else
    {    
        m_Assync.SetProgress(_T("[end_compressing_files]"), 100, false);   
    }

    return bStatus;
}

// This method sends the error report over the Internet
BOOL CErrorReportSender::SendReport()
{
    int status = 1;

    m_Assync.SetProgress(_T("[sending_report]"), 0);

	// Arrange priorities in reverse order
    std::multimap<int, int> order;

    std::pair<int, int> pair3(m_CrashInfo.m_uPriorities[CR_SMAPI], CR_SMAPI);
    order.insert(pair3);

    std::pair<int, int> pair2(m_CrashInfo.m_uPriorities[CR_SMTP], CR_SMTP);
    order.insert(pair2);

    std::pair<int, int> pair1(m_CrashInfo.m_uPriorities[CR_HTTP], CR_HTTP);
    order.insert(pair1);

    std::multimap<int, int>::reverse_iterator rit;

	// Walk through priorities
    for(rit=order.rbegin(); rit!=order.rend(); rit++)
    {
        m_Assync.SetProgress(_T("[sending_attempt]"), 0);
        m_SendAttempt++;    

		// Check if operation was cancelled.
        if(m_Assync.IsCancelled()){ break; }

        int id = rit->second;

        BOOL bResult = FALSE;

		// Send the report
        if(id==CR_HTTP)
            bResult = SendOverHTTP();    
        else if(id==CR_SMTP)
            bResult = SendOverSMTP();  
        else if(id==CR_SMAPI)
            bResult = SendOverSMAPI();

		// Check if this attempt has failed
        if(bResult==FALSE)
            continue;

		// If currently sending through Simple MAPI, do not wait for completion
        if(id==CR_SMAPI && bResult==TRUE)
        {
            status = 0;
            break;
        }

		// else wait for completion
        if(0==m_Assync.WaitForCompletion())
        {
            status = 0;
            break;
        }
    }

    // Remove compressed ZIP file and MD5 file
    Utility::RecycleFile(m_sZipName, true);
    Utility::RecycleFile(m_sZipName+_T(".md5"), true);

	// Check status
    if(status==0)
    {
		// Success
        m_Assync.SetProgress(_T("[status_success]"), 0);
        m_CrashInfo.GetReport(m_nCurReport).m_DeliveryStatus = DELIVERED;
        // Delete report files
        Utility::RecycleFile(m_CrashInfo.GetReport(m_nCurReport).m_sErrorReportDirName, true);    
    }
    else
    {
		// Some error occurred
        m_CrashInfo.GetReport(m_nCurReport).m_DeliveryStatus = FAILED;    
        m_Assync.SetProgress(_T("[status_failed]"), 0);    

        // Check if we should store files for later delivery or we should remove them
        if(!m_CrashInfo.m_bQueueEnabled)
        {
            // Delete report files
            Utility::RecycleFile(m_CrashInfo.GetReport(m_nCurReport).m_sErrorReportDirName, true);    
        }
    }

	// Done
    m_nGlobalStatus = status;
    m_Assync.SetCompleted(status);  
    return 0;
}

// This method sends the report over HTTP request
BOOL CErrorReportSender::SendOverHTTP()
{  
    strconv_t strconv;

	// Check our config - should we send the report over HTTP or not?
    if(m_CrashInfo.m_uPriorities[CR_HTTP]==CR_NEGATIVE_PRIORITY)
    {
        m_Assync.SetProgress(_T("Sending error report over HTTP is disabled (negative priority); skipping."), 0);
        return FALSE;
    }

	// Check URL
    if(m_CrashInfo.m_sUrl.IsEmpty())
    {
        m_Assync.SetProgress(_T("No URL specified for sending error report over HTTP; skipping."), 0);
        return FALSE;
    }

	// Update progress
    m_Assync.SetProgress(_T("Sending error report over HTTP..."), 0);
    m_Assync.SetProgress(_T("Preparing HTTP request data..."), 0);

	// Create HTTP request
    CHttpRequest request;
    request.m_sUrl = m_CrashInfo.m_sUrl;  

	ErrorReportInfo& eri = m_CrashInfo.GetReport(m_nCurReport);

	// Fill in the request fields
	CString sNum;
	sNum.Format(_T("%d"), CRASHRPT_VER);
	request.m_aTextFields[_T("crashrptver")] = strconv.t2utf8(sNum);
    request.m_aTextFields[_T("appname")] = strconv.t2utf8(eri.m_sAppName);
    request.m_aTextFields[_T("appversion")] = strconv.t2utf8(eri.m_sAppVersion);
    request.m_aTextFields[_T("crashguid")] = strconv.t2utf8(eri.m_sCrashGUID);
    request.m_aTextFields[_T("emailfrom")] = strconv.t2utf8(eri.m_sEmailFrom);
    request.m_aTextFields[_T("emailsubject")] = strconv.t2utf8(m_CrashInfo.m_sEmailSubject);
    request.m_aTextFields[_T("description")] = strconv.t2utf8(eri.m_sDescription);
	request.m_aTextFields[_T("exceptionmodule")] = strconv.t2utf8(eri.m_sExceptionModule);
	request.m_aTextFields[_T("exceptionmoduleversion")] = strconv.t2utf8(eri.m_sExceptionModuleVersion);	
	sNum.Format(_T("%I64u"), eri.m_dwExceptionModuleBase);
	request.m_aTextFields[_T("exceptionmodulebase")] = strconv.t2utf8(sNum);
	sNum.Format(_T("%I64u"), eri.m_dwExceptionAddress);
	request.m_aTextFields[_T("exceptionaddress")] = strconv.t2utf8(sNum);

	// Add an MD5 hash of file attachment
    CString sMD5Hash;
    CalcFileMD5Hash(m_sZipName, sMD5Hash);
    request.m_aTextFields[_T("md5")] = strconv.t2utf8(sMD5Hash);

	// Set content type depending on content transfer encoding
    if(m_CrashInfo.m_bHttpBinaryEncoding)
    {
        CHttpRequestFile f;
        f.m_sSrcFileName = m_sZipName;
        f.m_sContentType = _T("application/zip");  
        request.m_aIncludedFiles[_T("crashrpt")] = f;  
    }
    else
    {
        m_Assync.SetProgress(_T("Base-64 encoding file attachment, please wait..."), 1);

        std::string sEncodedData;
        int nRet = Base64EncodeAttachment(m_sZipName, sEncodedData);
        if(nRet!=0)
        {
            return FALSE;
        }
        request.m_aTextFields[_T("crashrpt")] = sEncodedData;
    }

	// Send HTTP request assynchronously
    BOOL bSend = m_HttpSender.SendAssync(request, &m_Assync);  
    return bSend;
}

int CErrorReportSender::Base64EncodeAttachment(CString sFileName, 
                                               std::string& sEncodedFileData)
{
    strconv_t strconv;

    int uFileSize = 0;
    BYTE* uchFileData = NULL;  
    struct _stat st;  

	// Get file information
    int nResult = _tstat(sFileName, &st);
    if(nResult != 0)
        return 1;  // File not found.

    // Allocate buffer of file size
    uFileSize = st.st_size;
    uchFileData = new BYTE[uFileSize];

    // Read file data to buffer.
    FILE* f = NULL;
#if _MSC_VER<1400
    f = _tfopen(sFileName, _T("rb"));
#else
    /*errno_t err = */_tfopen_s(&f, sFileName, _T("rb"));  
#endif 

    if(!f || fread(uchFileData, uFileSize, 1, f)!=1)
    {
        delete [] uchFileData;
        uchFileData = NULL;
        return 2; // Coudln't read file data.
    }

	// Close file
    fclose(f);

	// BASE-64 ecode data
    sEncodedFileData = base64_encode(uchFileData, uFileSize);

	// Clean up
    delete [] uchFileData;

    // OK.
    return 0;
}

// This method formats the E-mail message text
CString CErrorReportSender::FormatEmailText()
{
    CString sFileTitle = m_sZipName;
    sFileTitle.Replace('/', '\\');
    int pos = sFileTitle.ReverseFind('\\');
    if(pos>=0)
        sFileTitle = sFileTitle.Mid(pos+1);

    CString sText;

    sText += _T("This is the error report from ") + m_CrashInfo.m_sAppName + 
        _T(" ") + m_CrashInfo.GetReport(m_nCurReport).m_sAppVersion+_T(".\n\n");

    if(!m_CrashInfo.GetReport(m_nCurReport).m_sEmailFrom.IsEmpty())
    {
        sText += _T("This error report was sent by ") + m_CrashInfo.GetReport(m_nCurReport).m_sEmailFrom + _T(".\n");
        sText += _T("If you need additional info about the problem, you may want to contact this user again.\n\n");
    }     

    if(!m_CrashInfo.GetReport(m_nCurReport).m_sEmailFrom.IsEmpty())
    {
        sText += _T("The user has provided the following problem description:\n<<< ") + 
            m_CrashInfo.GetReport(m_nCurReport).m_sDescription + _T(" >>>\n\n");    
    }

    sText += _T("You may find detailed information about the error in files attached to this message:\n\n");
    sText += sFileTitle + _T(" is a ZIP archive which contains crash description XML (crashrpt.xml), crash minidump (crashdump.dmp) ");
    sText += _T("and possibly other files that your application added to the crash report.\n\n");

    sText += sFileTitle + _T(".md5 file contains MD5 hash for the ZIP archive. You might want to use this file to check integrity of the error report.\n\n");

    sText += _T("For additional information, see FAQ http://code.google.com/p/crashrpt/wiki/FAQ\n");

    return sText;
}

// This method sends the report over SMTP 
BOOL CErrorReportSender::SendOverSMTP()
{  
    strconv_t strconv;

	// Check our config - should we send the report over SMTP or not?
    if(m_CrashInfo.m_uPriorities[CR_SMTP]==CR_NEGATIVE_PRIORITY)
    {
        m_Assync.SetProgress(_T("Sending error report over SMTP is disabled (negative priority); skipping."), 0);
        return FALSE;
    }

	// Check recipient's email
    if(m_CrashInfo.m_sEmailTo.IsEmpty())
    {
        m_Assync.SetProgress(_T("No E-mail address is specified for sending error report over SMTP; skipping."), 0);
        return FALSE;
    }

	// Fill in email fields
    m_EmailMsg.m_sFrom = (!m_CrashInfo.GetReport(m_nCurReport).m_sEmailFrom.IsEmpty())?
        m_CrashInfo.GetReport(m_nCurReport).m_sEmailFrom:m_CrashInfo.m_sEmailTo;
    m_EmailMsg.m_sTo = m_CrashInfo.m_sEmailTo;
    m_EmailMsg.m_nRecipientPort = m_CrashInfo.m_nSmtpPort;
    m_EmailMsg.m_sSubject = m_CrashInfo.m_sEmailSubject;

    if(m_CrashInfo.m_sEmailText.IsEmpty())
        m_EmailMsg.m_sText = FormatEmailText();
    else
        m_EmailMsg.m_sText = m_CrashInfo.m_sEmailText;

    m_EmailMsg.m_aAttachments.clear();

    m_EmailMsg.m_aAttachments.insert(m_sZipName);  

    // Create and attach MD5 hash file
    CString sErrorRptHash;
    CalcFileMD5Hash(m_sZipName, sErrorRptHash);
    CString sFileTitle = m_sZipName;
    sFileTitle.Replace('/', '\\');
    int pos = sFileTitle.ReverseFind('\\');
    if(pos>=0)
        sFileTitle = sFileTitle.Mid(pos+1);
    sFileTitle += _T(".md5");
    CString sTempDir;
    Utility::getTempDirectory(sTempDir);
    CString sTmpFileName = sTempDir +_T("\\")+ sFileTitle;
    FILE* f = NULL;
    _TFOPEN_S(f, sTmpFileName, _T("wt"));
    if(f!=NULL)
    {   
        LPCSTR szErrorRptHash = strconv.t2a(sErrorRptHash.GetBuffer(0));
        fwrite(szErrorRptHash, strlen(szErrorRptHash), 1, f);
        fclose(f);
        m_EmailMsg.m_aAttachments.insert(sTmpFileName);  
    }

    // Set SMTP proxy server (if specified)
    if ( !m_CrashInfo.m_sSmtpProxyServer.IsEmpty())
        m_SmtpClient.SetSmtpProxy( m_CrashInfo.m_sSmtpProxyServer, m_CrashInfo.m_nSmtpProxyPort);

    // Send mail assynchronously
    int res = m_SmtpClient.SendEmailAssync(&m_EmailMsg, &m_Assync); 
    return (res==0);
}

// This method sends the report over Simple MAPI
BOOL CErrorReportSender::SendOverSMAPI()
{  
    strconv_t strconv;

	// Check our config - should we send the report over Simple MAPI or not?
    if(m_CrashInfo.m_uPriorities[CR_SMAPI]==CR_NEGATIVE_PRIORITY)
    {
        m_Assync.SetProgress(_T("Sending error report over SMAPI is disabled (negative priority); skipping."), 0);
        return FALSE;
    }

	// Check recipient's email address
    if(m_CrashInfo.m_sEmailTo.IsEmpty())
    {
        m_Assync.SetProgress(_T("No E-mail address is specified for sending error report over Simple MAPI; skipping."), 0);
        return FALSE;
    }

	// Do not send if we are in silent mode
    if(m_CrashInfo.m_bSilentMode)
    {
        m_Assync.SetProgress(_T("Simple MAPI may require user interaction (not acceptable for non-GUI mode); skipping."), 0);
        return FALSE;
    }

	// Update progress
    m_Assync.SetProgress(_T("Sending error report using Simple MAPI"), 0, false);
    m_Assync.SetProgress(_T("Initializing MAPI"), 1);

	// Initialize MAPI
    BOOL bMAPIInit = m_MapiSender.MAPIInitialize();
    if(!bMAPIInit)
    {
        m_Assync.SetProgress(m_MapiSender.GetLastErrorMsg(), 100, false);
        return FALSE;
    }

	// Request user confirmation
    if(m_SendAttempt!=0)
    {
        m_Assync.SetProgress(_T("[confirm_launch_email_client]"), 0);
        int confirm = 1;
        m_Assync.WaitForFeedback(confirm);
        if(confirm!=0)
        {
            m_Assync.SetProgress(_T("Cancelled by user"), 100, false);
            return FALSE;
        }
    }

	// Detect mail client (Microsoft Outlook, Mozilla Thunderbird and so on)
    CString msg;
    CString sMailClientName;
    m_MapiSender.DetectMailClient(sMailClientName);

	msg.Format(_T("Launching the default email client (%s)"), sMailClientName);
    m_Assync.SetProgress(msg, 10);

	// Fill in email fields
    m_MapiSender.SetFrom(m_CrashInfo.GetReport(m_nCurReport).m_sEmailFrom);
    m_MapiSender.SetTo(m_CrashInfo.m_sEmailTo);
    m_MapiSender.SetSubject(m_CrashInfo.m_sEmailSubject);
    CString sFileTitle = m_sZipName;
    sFileTitle.Replace('/', '\\');
    int pos = sFileTitle.ReverseFind('\\');
    if(pos>=0)
        sFileTitle = sFileTitle.Mid(pos+1);

    if(m_CrashInfo.m_sEmailText.IsEmpty())
        m_MapiSender.SetMessage(FormatEmailText());
    else
        m_MapiSender.SetMessage(m_CrashInfo.m_sEmailText);
    m_MapiSender.AddAttachment(m_sZipName, sFileTitle);

    // Create and attach MD5 hash file
    CString sErrorRptHash;
    CalcFileMD5Hash(m_sZipName, sErrorRptHash);
    sFileTitle += _T(".md5");
    CString sTempDir;
    Utility::getTempDirectory(sTempDir);
    CString sTmpFileName = sTempDir +_T("\\")+ sFileTitle;
    FILE* f = NULL;
    _TFOPEN_S(f, sTmpFileName, _T("wt"));
    if(f!=NULL)
    { 
        LPCSTR szErrorRptHash = strconv.t2a(sErrorRptHash.GetBuffer(0));
        fwrite(szErrorRptHash, strlen(szErrorRptHash), 1, f);
        fclose(f);
        m_MapiSender.AddAttachment(sTmpFileName, sFileTitle);  
    }

	// Send email
    BOOL bSend = m_MapiSender.Send();
    if(!bSend)
        m_Assync.SetProgress(m_MapiSender.GetLastErrorMsg(), 100, false);
    else
        m_Assync.SetProgress(_T("Sent OK"), 100, false);

    return bSend;
}

CString CErrorReportSender::GetLangStr(LPCTSTR szSection, LPCTSTR szName)
{
	return Utility::GetINIString(m_CrashInfo.m_sLangFileName, szSection, szName);
}



