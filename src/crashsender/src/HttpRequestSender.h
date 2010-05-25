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

// File: httpsend.h
// Description: Sends error report over HTTP connection.
// Authors: zexspectrum
// Date: 2009

#pragma once
#include "stdafx.h"
#include <string>
#include <map>
#include "AssyncNotification.h"
#include <wininet.h>

struct CHttpRequestFile
{  
  CString m_sSrcFileName;  // Name of the file attachment.
  CString m_sContentType;  // Content type.
};

// HTTP request information
class CHttpRequest
{
public:
  CString m_sUrl;      // Script URL  
  std::map<CString, std::string> m_aTextFields;    // Array of text fields to include into POST data
  std::map<CString, CHttpRequestFile> m_aIncludedFiles; // Array of binary files to include into POST data
};

// Sends HTTP request
// See also: RFC 1867 - Form-based File Upload in HTML (http://www.ietf.org/rfc/rfc1867.txt)
class CHttpRequestSender
{
public:
  
  CHttpRequestSender();

  // Sends HTTP request assynchroniously
  BOOL SendAssync(CHttpRequest& Request, AssyncNotification* an);

private:
  
  // Worker thread procedure
  static DWORD WINAPI WorkerThread(VOID* pParam);  

  BOOL InternalSend();

  // Used to calculate summary size of the request
  BOOL CalcRequestSize(LONGLONG& lSize);
  BOOL FormatTextPartHeader(CString sName, CString& sText);
  BOOL FormatTextPartFooter(CString sName, CString& sText);  
  BOOL FormatAttachmentPartHeader(CString sName, CString& sText);
  BOOL FormatAttachmentPartFooter(CString sName, CString& sText);
  BOOL FormatTrailingBoundary(CString& sBoundary);
  BOOL CalcTextPartSize(CString sFileName, LONGLONG& lSize);
  BOOL CalcAttachmentPartSize(CString sFileName, LONGLONG& lSize);
  BOOL WriteTextPart(HINTERNET hRequest, CString sName);
  BOOL WriteAttachmentPart(HINTERNET hRequest, CString sName);
  BOOL WriteTrailingBoundary(HINTERNET hRequest);
  void UploadProgress(DWORD dwBytesWritten);

  // This helper function is used to split URL into several parts
  void ParseURL(LPCTSTR szURL, LPTSTR szProtocol, UINT cbProtocol,
    LPTSTR szAddress, UINT cbAddress, DWORD &dwPort, LPTSTR szURI, UINT cbURI);

  CHttpRequest m_Request;       // HTTP request being sent
  AssyncNotification* m_Assync; // Used to communicate with the main thread  
  
  CString m_sFilePartHeaderFmt;
  CString m_sFilePartFooterFmt;
  CString m_sTextPartHeaderFmt;
  CString m_sTextPartFooterFmt;
  CString m_sBoundary;
  DWORD m_dwPostSize;
  DWORD m_dwUploaded;
};


