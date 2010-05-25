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

// File: CrashInfoReader.h
// Description: Retrieves crash information passed from CrashRpt.dll in form of XML files.
// Authors: zexspectrum
// Date: 2010

#pragma once
#include <map>
#include <vector>
#include "tinyxml.h"
#include "dbghelp.h"

struct FileItem
{
  FileItem()
  {
    m_bMakeCopy = FALSE;
  }

  CString m_sDestFile;
  CString m_sSrcFile;
  CString m_sDesc;
  BOOL m_bMakeCopy;
};

struct ErrorReportInfo
{
  CString     m_sErrorReportDirName;
  CString     m_sCrashGUID;
  CString     m_sAppName;
  CString     m_sAppVersion;
  CString     m_sImageName;  
  CString     m_sEmailFrom;     
  CString     m_sDescription;    

  std::map<CString, FileItem>  m_FileItems; 
};

class CCrashInfoReader
{
public:

  CString     m_sLangFileName;
  CString     m_sDbgHelpPath;  
  CString     m_sAppName;    
  CString     m_sEmailTo;
  CString     m_sEmailSubject;
  CString     m_sEmailText;  
  int         m_nSmtpPort;  
  CString     m_sSmtpProxyServer;
  int         m_nSmtpProxyPort;  
  CString     m_sUrl;
  BOOL        m_bHttpBinaryEncoding;
  BOOL        m_bSilentMode;
  BOOL        m_bSendErrorReport;
  BOOL        m_bAppRestart;
  CString     m_sRestartCmdLine;
  UINT        m_uPriorities[3];
  CString     m_sPrivacyPolicyURL;
  BOOL        m_bGenerateMinidump;  
  MINIDUMP_TYPE m_MinidumpType;
  DWORD       m_dwProcessId;
  DWORD       m_dwThreadId;
  PEXCEPTION_POINTERS m_pExInfo;
  BOOL        m_bAddScreenshot;
  DWORD       m_dwScreenshotFlags;
  CPoint      m_ptCursorPos;
  CRect       m_rcAppWnd;
  BOOL        m_bSendRecentReports;
  CString     m_sUnsentCrashReportsFolder;

  std::vector<ErrorReportInfo> m_Reports;
  int m_nCurrentReport;

  // Gets crash info from internal crash info XML file
  int Init(CString sCrashInfoFile);

  BOOL AddUserInfoToCrashDescriptionXML(CString sEmail, CString sDesc);
  BOOL AddFilesToCrashDescriptionXML(std::vector<FileItem>);

  BOOL GetErrorReportInfo(CString sXmlName, ErrorReportInfo& eri);

  ErrorReportInfo& GetCurReport(){return m_Reports[m_nCurrentReport];}

private:

  // Gets the list of file items 
  int ParseFileList(int nReport, TiXmlHandle& hRoot);

  // Retrieves some crash info from crash description XML
  int ParseCrashDescription(int nReport, CString sFileName);

};

// Declare globally available object
extern CCrashInfoReader g_CrashInfo;
