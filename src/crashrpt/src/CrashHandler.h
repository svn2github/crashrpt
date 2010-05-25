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

// File: CrashHandler.h
// Description: Exception handling and report generation functionality.
// Authors: mikecarruth, zexspectrum
// Date: 

#ifndef _CRASHHANDLER_H_
#define _CRASHHANDLER_H_

#include "stdafx.h"
#include <signal.h>
#include <exception>
#include "CrashRpt.h"      
#include "Utility.h"
#include "CritSec.h"

/* This structure contains pointer to the exception handlers for a thread.*/
struct ThreadExceptionHandlers
{
  ThreadExceptionHandlers()
  {
    m_prevTerm = NULL;
    m_prevUnexp = NULL;
    m_prevSigFPE = NULL;
    m_prevSigILL = NULL;
    m_prevSigSEGV = NULL;
  }

  terminate_handler m_prevTerm;        // Previous terminate handler   
  unexpected_handler m_prevUnexp;      // Previous unexpected handler
  void (__cdecl *m_prevSigFPE)(int);   // Previous FPE handler
  void (__cdecl *m_prevSigILL)(int);   // Previous 
  void (__cdecl *m_prevSigSEGV)(int);  // Previous illegal storage access handler
};

// Sets the last error message (for the caller thread).
int crSetErrorMsg(PTSTR pszErrorMsg);

struct FileItem
{
  CString m_sFileName;    // Path to the original file 
  CString m_sDescription; // Description
  BOOL m_bMakeCopy;       // Should we make a copy of this file on crash?
};

class CCrashHandler  
{
public:
	
  // Default constructor.
  CCrashHandler();

  virtual ~CCrashHandler();

  int Init(
      LPCTSTR lpcszAppName = NULL,
      LPCTSTR lpcszAppVersion = NULL,
      LPCTSTR lpcszCrashSenderPath = NULL,
      LPGETLOGFILE lpfnCallback = NULL,           
      LPCTSTR lpcszTo = NULL,             
      LPCTSTR lpcszSubject = NULL,
      LPCTSTR lpcszUrl = NULL,
      UINT (*puPriorities)[5] = NULL,
      DWORD dwFlags = 0,
      LPCTSTR lpcszPrivacyPolicyURL = NULL,
      LPCTSTR lpcszDebugHelpDLLPath = NULL,
      MINIDUMP_TYPE MiniDumpType = MiniDumpNormal,
      LPCTSTR lpcszErrorReportSaveDir = NULL,
      LPCTSTR lpcszRestartCmdLine = NULL,
      LPCTSTR lpcszLangFilePath = NULL,
      LPCTSTR lpcszEmailText = NULL,
      LPCTSTR lpcszSmtpProxy = NULL);

  int Destroy();
   
  // Adds a file to the crash report
  int AddFile(LPCTSTR lpFile, LPCTSTR lpDestFile, LPCTSTR lpDesc, DWORD dwFlags);

  // Adds a named text property to the report
  int AddProperty(CString sPropName, CString sPropValue);

  // Adds desktop screenshot on crash
  int AddScreenshot(DWORD dwFlags);

  // Generates error report
  int GenerateErrorReport(PCR_EXCEPTION_INFO pExceptionInfo = NULL);
     
  // Sets/unsets exception handlers for the current process
  int SetProcessExceptionHandlers(DWORD dwFlags);
  int UnSetProcessExceptionHandlers();

  // Sets/unsets exception handlers for the caller thread
  int SetThreadExceptionHandlers(DWORD dwFlags);   
  int UnSetThreadExceptionHandlers();
  
  // Returns the crash handler object if such object was 
  // created for the current process
  static CCrashHandler* GetCurrentProcessCrashHandler();

private:

  /* Exception handler functions. */

  static LONG WINAPI SehHandler(PEXCEPTION_POINTERS pExceptionPtrs);
  static void __cdecl TerminateHandler();
  static void __cdecl UnexpectedHandler();

#if _MSC_VER>=1300
  static void __cdecl PureCallHandler();
#endif 

#if _MSC_VER>=1300 && _MSC_VER<1400
  static void __cdecl SecurityHandler(int code, void *x);
#endif

#if _MSC_VER>=1400
  static void __cdecl InvalidParameterHandler(const wchar_t* expression, 
    const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
#endif

#if _MSC_VER>=1300
  static int __cdecl NewHandler(size_t);
#endif

  static void SigabrtHandler(int);
  static void SigfpeHandler(int /*code*/, int subcode);
  static void SigintHandler(int);
  static void SigillHandler(int);
  static void SigsegvHandler(int);
  static void SigtermHandler(int);

  /* Crash report generation methods */

  // Collects current process state
  void GetExceptionPointers(DWORD dwExceptionCode, 
    EXCEPTION_POINTERS** pExceptionPointers);
  
  // Collects various information useful for crash analyzis
  void CollectMiscCrashInfo();
    
  // Creates crash description XML file
  int CreateCrashDescriptionXML(LPTSTR pszFileName, 
     PCR_EXCEPTION_INFO pExceptionInfo);

  // Creates internally used crash description file
  int CreateInternalCrashInfoFile(CString sFileName, 
    EXCEPTION_POINTERS* pExInfo);
  
  // Launches the CrashSender.exe process
  int LaunchCrashSender(CString sCmdLineParams, BOOL bWait);  

  // Replaces characters that are restricted in XML.
  std::string XmlEncodeStr(CString sText);
  
  // Sets internal pointers to exception handlers to NULL
  void InitPrevExceptionHandlerPointers();

  /* Private member variables. */

  static CCrashHandler* m_pProcessCrashHandler; // Singleton of the CCrashHandler class
  
  LPTOP_LEVEL_EXCEPTION_FILTER  m_oldSehHandler;  // previous SEH exception filter
      
#if _MSC_VER>=1300
  _purecall_handler m_prevPurec;   // Previous pure virtual call exception filter
  _PNH m_prevNewHandler; // Previous new operator exception filter
#endif

#if _MSC_VER>=1400
  _invalid_parameter_handler m_prevInvpar; // Previous invalid parameter exception filter  
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400
  _secerr_handler_func m_prevSec; // Previous security exception filter
#endif

  void (__cdecl *m_prevSigABRT)(int); // Previous SIGABRT handler  
  void (__cdecl *m_prevSigINT)(int);  // Previous SIGINT handler
  void (__cdecl *m_prevSigTERM)(int); // Previous SIGTERM handler

  // List of exception handlers installed for worker threads of current process
  std::map<DWORD, ThreadExceptionHandlers> m_ThreadExceptionHandlers;
  CCritSec m_csThreadExceptionHandlers; // Synchronization lock for m_ThreadExceptionHandlers

  std::map<CString, FileItem> m_files;  // Files to add.
  std::map<CString, CString> m_props;   // User-defined properties
  LPGETLOGFILE m_lpfnCallback;   // Client crash callback.
  CString m_sEmailTo;            // Email recipient address.
  int m_nSmtpPort;               // Port for SMTP connection.
  CString m_sEmailSubject;       // Email subject.
  CString m_sEmailText;          // Email message text.
  CString m_sSmtpProxyServer;    // SMTP proxy server address.
  int m_nSmtpProxyPort;          // SMTP proxy server port.
  CString m_sUrl;                // URL for sending reports via HTTP.
  UINT m_uPriorities[3];         // Which way to prefer when sending crash report?
  CString m_sAppName;            // Application name.
  CString m_sAppVersion;         // Application version.
  CString m_sImageName;          // Path to client executable file.
  CString m_sPathToCrashSender;  // Path to crash sender exectuable file.  
  CString m_sCrashGUID;          // Unique ID of the crash report.
  CString m_sUnsentCrashReportsFolder; // Folder where unsent crash reports should be saved.
  CString m_sReportFolderName;   // Folder where current crash report will be saved.
  CString m_sPrivacyPolicyURL;   // Privacy policy URL  
  HMODULE m_hDbgHelpDll;         // HANDLE to debug help DLL
  CString m_sPathToDebugHelpDll; // Path to dbghelp DLL
  BOOL m_bGenerateMinidump;      // Should we generate minidump file?
  BOOL m_bSendRecentReports;     // Should we send recently generated and unsent reports again?
  MINIDUMP_TYPE m_MiniDumpType;  // Mini dump type 
  BOOL m_bSilentMode;            // Do not show GUI on crash, send report silently.
  BOOL m_bHttpBinaryEncoding;    // Use HTTP uploads with binary encoding instead of the legacy (Base-64) encoding.
  BOOL m_bSendErrorReport;       // Should we send error report or just save it  
  CString m_sLangFileName;       // Language file to use.
  SYSTEMTIME m_AppStartTime;     // The time this application was started.
  CString m_sCrashTime;          // Crash time in UTC format
  CString m_sOSName;             // Operating system name.
  DWORD m_dwGuiResources;        // Count of GUI resources in use
  DWORD m_dwProcessHandleCount;  // Count of opened handles
  CString m_sMemUsage;           // Memory usage

  BOOL m_bAddScreenshot;         // Should we make a desktop screenshot on crash?
  DWORD m_dwScreenshotFlags;     // Screenshot flags
  CRect m_rcAppWnd;              // Rectangle of the main app window (used for screenshot generation).
  CPoint m_ptCursorPos;          // Mouse cursor position at the moment of crash

  BOOL m_bAppRestart;            // Should we restart the crashed app or not?
  CString m_sRestartCmdLine;     // Command line for app restart.

  HANDLE m_hEvent;               // Event used to synchronize with CrashSender.exe

  BOOL m_bInitialized;           // Flag telling if this object was are initialized.
};

#endif	// !_CRASHHANDLER_H_
