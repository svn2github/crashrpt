///////////////////////////////////////////////////////////////////////////////
//
//  Module: CrashHandler.h
//
//    Desc: CCrashHandler is the main class used by crashrpt to manage all
//          of the details associated with handling the exception, generating
//          the report, gathering client input, and sending the report.
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _CRASHHANDLER_H_
#define _CRASHHANDLER_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "crashrpt.h"      
#include <new.h>
#include <map>
#include <stdlib.h>
#include <signal.h>
#include <exception>
#include <string>
#include <dbghelp.h>

typedef std::map<CString, CString> TStrStrMap;

/* This structure contains pointer to the exception handlers for a thread.*/
struct _cpp_thread_exception_handlers
{
  _cpp_thread_exception_handlers()
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

int crSetErrorMsg(PTSTR pszErrorMsg);

////////////////////////////// Class Definitions /////////////////////////////

// ===========================================================================
// CCrashHandler
// 
// See the module comment at top of file.
//
class CCrashHandler  
{
public:
	
  // Default constructor.
  CCrashHandler();

  //-----------------------------------------------------------------------------
   // ~CCrashHandler
   //    Uninitializes the crashrpt library.
   //
   // Parameters
   //    none
   //
   // Return Values
   //    none
   //
   // Remarks
   //    none
   //
   virtual 
   ~CCrashHandler();

   //-----------------------------------------------------------------------------
   // Init
   //    Initializes the library and optionally set the client crash callback and
   //    sets up the email details.
   //
   // Parameters
   //    lpcszAppName Application name 
   //    lpfn        Client crash callback
   //    lpcszTo     Email address to send crash report
   //    lpcszSubject Subject line to be used with email
   //
   // Return Values
   //    Zero if initialization was successful
   //
   // Remarks
   //    Passing NULL for lpTo will disable the email feature and cause the crash 
   //    report to be saved to disk.
   //
   int Init(
      LPCTSTR lpcszAppName = NULL,
      LPCTSTR lpcszAppVersion = NULL,
      LPCTSTR lpcszCrashSenderPath = NULL,
      LPGETLOGFILE lpfn = NULL,           
      LPCTSTR lpcszTo = NULL,             
      LPCTSTR lpcszSubject = NULL,
      LPCTSTR lpcszUrl = NULL,
      UINT (*puPriorities)[5] = NULL,
      DWORD dwFlags = 0,
      LPCTSTR lpcszPrivacyPolicyURL = NULL);

   //-----------------------------------------------------------------------------
   // Destroy
   //

   int 
   Destroy();
   
   //-----------------------------------------------------------------------------
   // AddFile
   //    Adds a file to the crash report.
   //
   // Parameters
   //    lpFile      Fully qualified file name
   //    lpDesc      File description
   //
   // Return Values
   //    none
   //
   // Remarks
   //    Call this function to include application specific file(s) in the crash
   //    report.  For example, applicatoin logs, initialization files, etc.
   //
   int 
   AddFile(
      LPCTSTR lpFile,                     // File nae
      LPCTSTR lpDesc                      // File description
      );

   //-----------------------------------------------------------------------------
   // GenerateErrorReport
   //    Produces a crash report.
   //
   // Parameters
   //    pExInfo     Pointer to an EXCEPTION_POINTERS structure
   //
   // Return Values
   //    none
   //
   // Remarks
   //    Call this function to manually generate a crash report.
   //
   int GenerateErrorReport(PCR_EXCEPTION_INFO pExceptionInfo = NULL);
  
   int GenerateCrashDescriptorXML(LPTSTR pszFileName, 
     PCR_EXCEPTION_INFO pExceptionInfo);

   int SetProcessExceptionHandlers(DWORD dwFlags);
   int UnSetProcessExceptionHandlers();

   int SetThreadExceptionHandlers(DWORD dwFlags);   
   int UnSetThreadExceptionHandlers();
  
   static CCrashHandler* GetCurrentProcessCrashHandler();

protected:
  
  void GetExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS** pExceptionPointers);
  int CreateMinidump(LPCTSTR pszFileName, EXCEPTION_POINTERS* pExInfo);
  int ZipErrorReport(CString sFileName);  
  int LaunchCrashSender(CString sZipName);  

  CString _ReplaceRestrictedXMLCharacters(CString sText);
  
  // Sets internal pointers to exception handlers to NULL
  void InitPrevCPPExceptionHandlerPointers();

  LPTOP_LEVEL_EXCEPTION_FILTER  m_oldFilter;      // previous exception filter
      
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

  // List of exception handlers installed for threads of current process
  std::map<DWORD, _cpp_thread_exception_handlers> m_ThreadExceptionHandlers;

  LPGETLOGFILE m_lpfnCallback;   // Client crash callback.
  int m_pid;                     // Process id.
  TStrStrMap m_files;            // Custom files to add.
  CString m_sTo;                 // Email:To.
  CString m_sSubject;            // Email:Subject.
  CString m_sUrl;                // URL for sending reports via HTTP.
  UINT m_uPriorities[3];         // Which way to prefer when sending crash report?
  CString m_sAppName;            // Application name.
  CString m_sAppVersion;         // Application version.
  CString m_sImageName;          // Path to client executable file.
  CString m_sPathToCrashSender;  // Path to crash sender exectuable file.  
  CString m_sCrashGUID;          // Unique ID of the crash report.
  CString m_sOSName;             // Operating system name.
  CString m_sUnsentCrashReportsFolder; // Folder where unsent crash reports should be saved.
  CString m_sPrivacyPolicyURL;   // Privacy policy URL

  BOOL m_bInitialized;
};

#endif	// !_CRASHHANDLER_H_
