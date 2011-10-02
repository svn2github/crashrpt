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

#pragma once
#include "stdafx.h"
#include "AssyncNotification.h"

class CEmailMessage
{
public:
    CString m_sSubject;
    CString m_sFrom;
    CString m_sTo;
    int m_nRecipientPort;
    CString m_sText;
    std::set<CString> m_aAttachments;
};

class CSmtpClient
{
public:

    CSmtpClient();
    ~CSmtpClient();

    int SetSmtpProxy(CString sServer, int nPort);
    int SendEmail(CEmailMessage* msg);
    int SendEmailAssync(CEmailMessage* msg,  AssyncNotification* scn);

protected:

    static int _SendEmail(CEmailMessage* msg, AssyncNotification* scn);

    static int GetSmtpServerName(CEmailMessage* msg, AssyncNotification* scn, 
        std::map<WORD, CString>& host_list);

    static int SendEmailToRecipient(CString sSmtpServer, CEmailMessage* msg, 
        AssyncNotification* scn);

    static int GetMessageCode(LPSTR msg);

    static int CheckAddressSyntax(CString addr);

    static int SendMsg(AssyncNotification*, SOCKET sock, LPCTSTR pszMessage, LPSTR pszResponse=0, UINT uResponseSize=0);

    static int CheckAttachmentOK(CString sFileName);

    static int Base64EncodeAttachment(CString sFileName, 
        std::string& sEncodedFileData);

    static std::string UTF16toUTF8(LPCWSTR utf16);

    static DWORD WINAPI SmtpSendThread(VOID* pParam);

    struct SmtpSendThreadContext
    {
        CEmailMessage* m_msg;
        AssyncNotification* m_scn;
    };

    static CString m_sProxyServer;
    static int m_nProxyPort;

};



