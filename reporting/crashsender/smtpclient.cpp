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
#include "smtpclient.h"
#include "Utility.h"
#include "strconv.h"
#include "Base64.h"

CString CSmtpClient::m_sProxyServer;
int CSmtpClient::m_nProxyPort = 25;

CSmtpClient::CSmtpClient()
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    ATLASSERT(iResult==0); 
    iResult;

    m_nProxyPort = 25;
}

CSmtpClient::~CSmtpClient()
{
    int iResult = WSACleanup();
    iResult;
    ATLASSERT(iResult==0);
}

int CSmtpClient::SetSmtpProxy(CString sServer, int nPort)
{
    m_sProxyServer = sServer;
    m_nProxyPort = nPort;
    return 0;
}

int CSmtpClient::SendEmail(CEmailMessage* msg)
{
    return _SendEmail(msg, NULL);
}

int CSmtpClient::SendEmailAssync(CEmailMessage* msg,  AssyncNotification* scn)
{
    DWORD dwThreadId = 0;
    SmtpSendThreadContext ctx;
    ctx.m_msg = msg;
    ctx.m_scn = scn;

    //ResetEvent(scn->m_hCompletionEvent);

    HANDLE hThread = CreateThread(NULL, 0, SmtpSendThread, (void*)&ctx, 0, &dwThreadId);
    if(hThread==NULL)
        return 1;

    scn->WaitForCompletion();

    return 0;
}

DWORD WINAPI CSmtpClient::SmtpSendThread(VOID* pParam)
{
    SmtpSendThreadContext* ctx = (SmtpSendThreadContext*)pParam;

    CEmailMessage* msg = ctx->m_msg;
    AssyncNotification* scn = ctx->m_scn;

    scn->SetCompleted(0);

    int nResult = _SendEmail(msg, scn);

    scn->SetCompleted(nResult);

    return nResult;
}


int CSmtpClient::_SendEmail(CEmailMessage* msg, AssyncNotification* scn)
{
    scn->SetProgress(_T("Start sending email"), 0, false);

    std::map<WORD, CString> host_list;

    if(!m_sProxyServer.IsEmpty())
    {
        host_list[0] = m_sProxyServer;
    }
    else
    {
        int res = GetSmtpServerName(msg, scn, host_list);
        if(res!=0)
        {
            scn->SetProgress(_T("Error querying DNS record."), 100, false);
            return 1;
        }
    }

    std::map<WORD, CString>::iterator it;
    for(it=host_list.begin(); it!=host_list.end(); it++)
    {
        if(scn->IsCancelled())    
            return 2;

        int res = SendEmailToRecipient(it->second, msg, scn);
        if(res==0)
        {
            scn->SetProgress(_T("Finished OK."), 100, false);
            return 0;
        }
        if(res==2)
        {
            scn->SetProgress(_T("Critical error detected."), 100, false);
            return 2;
        }
    }

    scn->SetProgress(_T("Error sending email."), 100, false);

    return 1;
}


int CSmtpClient::GetSmtpServerName(CEmailMessage* msg, AssyncNotification* scn, 
                                   std::map<WORD, CString>& host_list)
{
    DNS_RECORD *apResult = NULL;

    CString sServer;
    sServer = msg->m_sFrom.Mid(msg->m_sFrom.Find('@')+1);

    CString sStatusMsg;
    sStatusMsg.Format(_T("Quering MX record of domain %s"), sServer);
    scn->SetProgress(sStatusMsg, 2);

    int r = DnsQuery(sServer, DNS_TYPE_MX, DNS_QUERY_STANDARD, 
        NULL, (PDNS_RECORD*)&apResult, NULL);

    PDNS_RECORD pRecOrig = apResult;

    if(r==0)
    {
        while(apResult!=NULL)
        {
            if(apResult->wType==DNS_TYPE_MX)        
            {
                CString sServerName = CString(apResult->Data.MX.pNameExchange);        
                host_list[apResult->Data.MX.wPreference] = sServerName;
            }

            apResult = apResult->pNext;
        }

        DnsRecordListFree(pRecOrig, DnsFreeRecordList);

        return 0;
    } 

    sStatusMsg.Format(_T("DNS query failed with code %d"), r);
    scn->SetProgress(sStatusMsg, 2);
    return 1;
}


int CSmtpClient::SendEmailToRecipient(CString sSmtpServer, CEmailMessage* msg, AssyncNotification* scn)
{
    int status = 1;

    strconv_t strconv;  

    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;

    int iResult = -1;  
    CString sPostServer;
    CString sServiceName;
    sServiceName.Format(_T("%d"), 
        m_sProxyServer.IsEmpty()?msg->m_nRecipientPort:m_nProxyPort);  
    SOCKET sock = INVALID_SOCKET;
    CString sMsg, str;
    std::set<CString>::iterator it;
    CString sStatusMsg;

    // Prepare message text
    CString sMessageText = msg->m_sText;
    sMessageText.Replace(_T("\n"),_T("\r\n"));
    sMessageText.Replace(_T("\r\n.\r\n"), _T("\r\n*\r\n"));
    LPCWSTR lpwszMessageText = strconv.t2w(sMessageText.GetBuffer(0));
    std::string sUTF8Text = UTF16toUTF8(lpwszMessageText);

    // Check that all attachments exist
    for(it=msg->m_aAttachments.begin(); it!=msg->m_aAttachments.end(); it++)
    {
        if(CheckAttachmentOK(*it)!=0)
        {
            sStatusMsg.Format(_T("Attachment not found: %s"), *it);
            scn->SetProgress(sStatusMsg, 1);
            return 2; // critical error
        }
    }

    sStatusMsg.Format(_T("Getting address info of %s port %s"), sSmtpServer, CString(sServiceName));
    scn->SetProgress(sStatusMsg, 1);

    int res = SOCKET_ERROR;
    char buf[1024]="";
    std::string sEncodedFileData;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;  

    LPCSTR lpszSmtpServer = strconv.t2a(sSmtpServer);
    LPCSTR lpszServiceName = strconv.t2a(sServiceName);
    iResult = getaddrinfo(lpszSmtpServer, lpszServiceName, &hints, &result);
    if(iResult!=0)
        goto exit;

    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) 
    {
        if(scn->IsCancelled()) {status = 2; goto exit;}

        sStatusMsg.Format(_T("Creating socket"));
        scn->SetProgress(sStatusMsg, 1);

        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(sock==INVALID_SOCKET)
        {
            scn->SetProgress(_T("Socket creation failed."), 1);
            goto exit;
        }

        sStatusMsg.Format(_T("Connecting to SMTP server %s port %s"), sSmtpServer, CString(sServiceName));
        scn->SetProgress(sStatusMsg, 1);

        res = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if(res!=SOCKET_ERROR)
            break;     

        closesocket(sock);
    }

    if(res==SOCKET_ERROR)
        goto exit;

    sStatusMsg.Format(_T("Connected OK."));
    scn->SetProgress(sStatusMsg, 5);

    if(scn->IsCancelled()) {status = 2; goto exit;}

    scn->SetProgress(_T("Waiting for greeting message from SMTP server..."), 1);

    res = recv(sock, buf, 1024, 0);
    if(res==SOCKET_ERROR)
    {
        sStatusMsg.Format(_T("Failed to receive greeting message from SMTP server (recv code %d)."), res);
        scn->SetProgress(sStatusMsg, 1);
        goto exit;
    }

    if(220!=GetMessageCode(buf)) 
    {
        goto exit;
    }


    char response[1024]="";

    sStatusMsg.Format(_T("Sending HELO"));
    scn->SetProgress(sStatusMsg, 1);

    // send HELO
    res=SendMsg(scn, sock, _T("HELO CrashSender\r\n"), response, 1024);
    if(res!=250)
    {
        sStatusMsg = CString(response, 1024);
        scn->SetProgress(sStatusMsg, 0);    
        goto exit;
    }
    
    sStatusMsg.Format(_T("Sending sender and recipient information"));
    scn->SetProgress(sStatusMsg, 1);

    sMsg.Format(_T("MAIL FROM:<%s>\r\n"), msg->m_sFrom);
    res=SendMsg(scn, sock, sMsg, response, 1024);
    if(res!=250)
    {
        sStatusMsg = CString(response, 1024);
        scn->SetProgress(sStatusMsg, 0);    
        goto exit;
    }

    sMsg.Format(_T("RCPT TO:<%s>\r\n"), msg->m_sTo);
    res=SendMsg(scn, sock, sMsg, response, 1024);
    if(res!=250)
    {
        sStatusMsg = CString(response, 1024);
        scn->SetProgress(sStatusMsg, 0);
        goto exit;
    }

    sStatusMsg.Format(_T("Start sending email data"));
    scn->SetProgress(sStatusMsg, 1);

    // Send DATA
    res=SendMsg(scn, sock, _T("DATA\r\n"), response, 1024);
    if(res!=354)
    {
        sStatusMsg = CString(response, 1024);
        scn->SetProgress(sStatusMsg, 0);    
        goto exit;
    }

    // Get current time
    time_t cur_time;
    time(&cur_time);
    char szDateTime[64] = "";

    struct tm ltimeinfo;
    localtime_s(&ltimeinfo, &cur_time );
    strftime(szDateTime, 64,  "%a, %d %b %Y %H:%M:%S", &ltimeinfo);
    
    TIME_ZONE_INFORMATION tzi;
    GetTimeZoneInformation(&tzi);

    int diff_hours = -tzi.Bias/60;
    int diff_mins = abs(tzi.Bias%60);

    str.Format(_T("Date: %s %c%02d%02d\r\n"), strconv.a2t(szDateTime), diff_hours>=0?'+':'-', diff_hours, diff_mins);
    sMsg = str;
    str.Format(_T("From: <%s>\r\n"), msg->m_sFrom);
    sMsg  += str;
    str.Format(_T("To: <%s>\r\n"), msg->m_sTo);
    sMsg += str;
    str.Format(_T("Subject: %s\r\n"), msg->m_sSubject);
    sMsg += str;
    sMsg += "MIME-Version: 1.0\r\n";
    sMsg += "Content-Type: multipart/mixed; boundary=KkK170891tpbkKk__FV_KKKkkkjjwq\r\n";
    sMsg += "\r\n\r\n";
    res = SendMsg(scn, sock, sMsg);
    if(res!=sMsg.GetLength())
        goto exit;

    /* Message text */

    sStatusMsg.Format(_T("Sending message text"));
    scn->SetProgress(sStatusMsg, 15);

    sMsg =  "--KkK170891tpbkKk__FV_KKKkkkjjwq\r\n";
    sMsg += "Content-Type: text/plain; charset=UTF-8\r\n";
    sMsg += "\r\n";  
    sMsg += sUTF8Text.c_str();
    sMsg += "\r\n";
    res = SendMsg(scn, sock, sMsg);
    if(res!=sMsg.GetLength())
        goto exit;

    sStatusMsg.Format(_T("Sending attachments"));
    scn->SetProgress(sStatusMsg, 1);

    /* Attachments. */
    for(it=msg->m_aAttachments.begin(); it!=msg->m_aAttachments.end(); it++)
    {    
        CString sFileName = *it;
        sFileName.Replace('/', '\\');
        CString sDisplayName = sFileName.Mid(sFileName.ReverseFind('\\')+1);

        // Header
        sMsg =  "\r\n--KkK170891tpbkKk__FV_KKKkkkjjwq\r\n";
        sMsg += "Content-Type: application/octet-stream\r\n";
        sMsg += "Content-Transfer-Encoding: base64\r\n";
        sMsg += "Content-Disposition: attachment; filename=\"";
        sMsg += sDisplayName;
        sMsg += "\"\r\n";
        sMsg += "\r\n";
        res = SendMsg(scn, sock, sMsg);
        if(res!=sMsg.GetLength())
            goto exit;

        // Encode data
        LPBYTE buf = NULL;
        //int buf_len = 0;
        int nEncode=Base64EncodeAttachment(sFileName, sEncodedFileData);
        if(nEncode!=0)
        {
            sStatusMsg.Format(_T("Error BASE64-encoding attachment %s"), sFileName);
            scn->SetProgress(sStatusMsg, 1);
            goto exit;
        }

        // Send encoded data
        sMsg = sEncodedFileData.c_str();        
        res = SendMsg(scn, sock, sMsg);
        if(res!=sMsg.GetLength())
            goto exit;

        delete [] buf;
    }

    sMsg =  "\r\n--KkK170891tpbkKk__FV_KKKkkkjjwq--";
    res = SendMsg(scn, sock, sMsg);
    if(res!=sMsg.GetLength())
        goto exit;

    // End of message marker
    if(250!=SendMsg(scn, sock, _T("\r\n.\r\n"), response, 1024))
    {
        sStatusMsg = CString(response, 1024);
        scn->SetProgress(sStatusMsg, 0);    
        goto exit;
    }

    // quit
    if(221!=SendMsg(scn, sock, _T("QUIT\r\n"), response, 1024))
    {
        sStatusMsg = CString(response, 1024);
        scn->SetProgress(sStatusMsg, 0);    
        goto exit;
    }

    // OK.
    status = 0;

exit:

    if(scn->IsCancelled()) 
        status = 2;

    sStatusMsg.Format(_T("Finished with error code %d"), status);
    scn->SetProgress(sStatusMsg, 100, false);

    // Clean up
    closesocket(sock);
    freeaddrinfo(result);  
    return status;
}

int CSmtpClient::GetMessageCode(LPSTR msg)
{
    if(msg==NULL)
        return -1;

    return atoi(msg);
}

int CSmtpClient::CheckAddressSyntax(CString addr)
{
    if(addr=="") return FALSE;
    return TRUE;
}

std::string CSmtpClient::UTF16toUTF8(LPCWSTR utf16)
{
    std::string utf8;
    int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, 0, 0);
    if (len>1)
    { 
        char* buf = new char[len+1];	  
        WideCharToMultiByte(CP_UTF8, 0, utf16, -1, buf, len, 0, 0);
        utf8 = buf;
        delete [] buf;
    }

    return utf8;
}

int CSmtpClient::SendMsg(AssyncNotification* scn, SOCKET sock, LPCTSTR pszMessage, LPSTR pszResponse, UINT uResponseSize)
{	
    strconv_t strconv;

    if(scn->IsCancelled()) {return -1;}

    int msg_len = (int)_tcslen(pszMessage);

    LPCSTR lpszMessageA = strconv.t2a((TCHAR*)pszMessage);

    int res = send(sock, lpszMessageA, msg_len, 0);	
    if(pszResponse==NULL) 
        return res;

    int br = recv(sock, pszResponse, uResponseSize, 0);
    if(br==SOCKET_ERROR)
        return br;

    if(br>2)
        pszResponse[br-2]=0; //zero terminate string

    return GetMessageCode(pszResponse);
}

int CSmtpClient::CheckAttachmentOK(CString sFileName)
{
    struct _stat st;  

    int nResult = _tstat(sFileName, &st);
    if(nResult != 0)
        return 1;  // File not found.

    return 0;
}

int CSmtpClient::Base64EncodeAttachment(CString sFileName, 
                                        std::string& sEncodedFileData)
{
    strconv_t strconv;  
    int uFileSize = 0;
    BYTE* uchFileData = NULL;  
    struct _stat st;  

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

    fclose(f);

    sEncodedFileData = base64_encode(uchFileData, uFileSize);

    delete [] uchFileData;

    // OK.
    return 0;
}


