#include "stdafx.h"
#include "SenderThread.h"
#include "MailMsg.h"
#include "smtpclient.h"
#include "httpsend.h"
#include "CrashRpt.h"
#include "md5.h"
#include "Utility.h"

int attempt = 0;
AssyncNotification an;
CEmailMessage msg;
CSmtpClient smtp;  
CHttpSender http;
CMailMsg mailmsg;

int CalcFileMD5Hash(CString sFileName, CString& sMD5Hash)
{
  FILE* f = NULL;
  BYTE buff[512];
  MD5 md5;
  MD5_CTX md5_ctx;
  unsigned char md5_hash[16];
  int i;

  sMD5Hash.Empty();

#if _MSC_VER<1400
  f = _tfopen(sFileName.GetBuffer(0), _T("rb"));
#else
  _tfopen_s(&f, sFileName.GetBuffer(0), _T("rb"));
#endif

  if(f==NULL) 
    return -1;

  md5.MD5Init(&md5_ctx);
  
  while(!feof(f))
  {
    size_t count = fread(buff, 1, 512, f);
    if(count>0)
    {
      md5.MD5Update(&md5_ctx, buff, (unsigned int)count);
    }
  }

  fclose(f);
  md5.MD5Final(md5_hash, &md5_ctx);

  for(i=0; i<16; i++)
  {
    CString number;
    number.Format(_T("%02x"), md5_hash[i]);
    sMD5Hash += number;
  }

  return 0;
}

void GetSenderThreadStatus(int& nProgressPct, std::vector<CString>& msg_log)
{
  an.GetProgress(nProgressPct, msg_log); 
}

void CancelSenderThread()
{
  an.Cancel();
}

void FeedbackReady(int code)
{
  an.FeedbackReady(code);
}

BOOL SendOverHTTP(SenderThreadContext* pc)
{  
  if(pc->m_sUrl.IsEmpty())
  {
    an.SetProgress(_T("No URL specified for sending error report over HTTP; skipping."), 0);
    return FALSE;
  }
  BOOL bSend = http.SendAssync(pc->m_sUrl, pc->m_sZipName, &an);  
  return bSend;
}

CString FormatEmailText(SenderThreadContext* pc)
{
  CString sFileTitle = pc->m_sZipName;
  sFileTitle.Replace('/', '\\');
  int pos = sFileTitle.ReverseFind('\\');
  if(pos>=0)
    sFileTitle = sFileTitle.Mid(pos+1);

  CString sText;

  sText += _T("This is an error report from ") + pc->m_sAppName + _T(" ") + pc->m_sAppVersion+_T(".\n\n");
 
  if(!pc->m_sEmailFrom.IsEmpty())
  {
    sText += _T("This error report was sent by ") + pc->m_sEmailFrom + _T(".\n");
    sText += _T("If you need additional info about the problem, you may want to contact this user again.\n\n");
  }     

  if(!pc->m_sEmailFrom.IsEmpty())
  {
    sText += _T("The user has provided the following problem description:\n<<< ") + pc->m_sEmailText + _T(" >>>\n\n");    
  }

  sText += _T("You may find detailed information about the error in files attached to this message:\n\n");
  sText += sFileTitle + _T(" is a ZIP archive which contains crash descriptor XML (crashrpt.xml), crash minidump (crashdump.dmp) ");
  sText += _T("and possibly other files that your application added to the crash report.\n\n");

  sText += sFileTitle + _T(".md5 file contains MD5 hash for the ZIP archive. You might want to use this file to check integrity of the error report.\n\n");
  
  sText += _T("For additional information about using error reports, see FAQ http://code.google.com/p/crashrpt/wiki/FAQ\n");

  return sText;
}

BOOL SendOverSMTP(SenderThreadContext* pc)
{  
  USES_CONVERSION;

  if(pc->m_sEmailTo.IsEmpty())
  {
    an.SetProgress(_T("No E-mail address is specified for sending error report over SMTP; skipping."), 0);
    return FALSE;
  }
  msg.m_sFrom = (!pc->m_sEmailFrom.IsEmpty())?pc->m_sEmailFrom:pc->m_sEmailTo;
  msg.m_sTo = pc->m_sEmailTo;
  msg.m_sSubject = pc->m_sEmailSubject;
  msg.m_sText = FormatEmailText(pc);
  
  msg.m_aAttachments.insert(pc->m_sZipName);  

  // Create and attach MD5 hash file
  CString sErrorRptHash;
  CalcFileMD5Hash(pc->m_sZipName, sErrorRptHash);
  CString sFileTitle = pc->m_sZipName;
  sFileTitle.Replace('/', '\\');
  int pos = sFileTitle.ReverseFind('\\');
  if(pos>=0)
    sFileTitle = sFileTitle.Mid(pos+1);
  sFileTitle += _T(".md5");
  CString sTempDir;
  CUtility::getTempDirectory(sTempDir);
  CString sTmpFileName = sTempDir +_T("\\")+ sFileTitle;
  FILE* f = NULL;
  _TFOPEN_S(f, sTmpFileName, _T("wt"));
  if(f!=NULL)
  {   
    LPSTR szErrorRptHash = T2A(sErrorRptHash.GetBuffer(0));
    fwrite(szErrorRptHash, strlen(szErrorRptHash), 1, f);
    fclose(f);
    msg.m_aAttachments.insert(sTmpFileName);  
  }

  int res = smtp.SendEmailAssync(&msg, &an); 
  return (res==0);
}

BOOL SendOverSMAPI(SenderThreadContext* pc)
{  
  USES_CONVERSION;

  if(pc->m_sEmailTo.IsEmpty())
  {
    an.SetProgress(_T("No E-mail address is specified for sending error report over Simple MAPI; skipping."), 0);
    return FALSE;
  }

  an.SetProgress(_T("Sending error report using Simple MAPI"), 0, false);
  an.SetProgress(_T("Initializing MAPI"), 1);

  BOOL bMAPIInit = mailmsg.MAPIInitialize();
  if(!bMAPIInit)
  {
    an.SetProgress(mailmsg.GetLastErrorMsg(), 100, false);
    return FALSE;
  }
  
  if(attempt!=0)
  {
    an.SetProgress(_T("[confirm_launch_email_client]"), 0);
    int confirm = 1;
    an.WaitForFeedback(confirm);
    if(confirm!=0)
    {
      an.SetProgress(_T("Cancelled by user"), 100, false);
      return FALSE;
    }
  }

  an.SetProgress(_T("Launching the default email client"), 10);
  
  mailmsg.SetFrom(pc->m_sEmailFrom);
  mailmsg.SetTo(pc->m_sEmailTo);
  mailmsg.SetSubject(pc->m_sEmailSubject);
  CString sFileTitle = pc->m_sZipName;
  sFileTitle.Replace('/', '\\');
  int pos = sFileTitle.ReverseFind('\\');
  if(pos>=0)
    sFileTitle = sFileTitle.Mid(pos+1);
  mailmsg.SetMessage(FormatEmailText(pc));
  mailmsg.AddAttachment(pc->m_sZipName, sFileTitle);

  // Create and attach MD5 hash file
  CString sErrorRptHash;
  CalcFileMD5Hash(pc->m_sZipName, sErrorRptHash);
  sFileTitle += _T(".md5");
  CString sTempDir;
  CUtility::getTempDirectory(sTempDir);
  CString sTmpFileName = sTempDir +_T("\\")+ sFileTitle;
  FILE* f = NULL;
  _TFOPEN_S(f, sTmpFileName, _T("wt"));
  if(f!=NULL)
  { 
    LPSTR szErrorRptHash = T2A(sErrorRptHash.GetBuffer(0));
    fwrite(szErrorRptHash, strlen(szErrorRptHash), 1, f);
    fclose(f);
    mailmsg.AddAttachment(sTmpFileName, sFileTitle);  
  }

  BOOL bSend = mailmsg.Send();
  if(!bSend)
    an.SetProgress(mailmsg.GetLastErrorMsg(), 100, false);
  else
    an.SetProgress(_T("Sent OK"), 100, false);
  
  return bSend;
}

DWORD WINAPI SenderThread(LPVOID lpParam)
{
  SenderThreadContext* pc = (SenderThreadContext*)lpParam;

  int status = 1;

  std::multimap<int, int> order;

  std::pair<int, int> pair3(pc->m_uPriorities[CR_SMAPI], CR_SMAPI);
  order.insert(pair3);

  std::pair<int, int> pair2(pc->m_uPriorities[CR_SMTP], CR_SMTP);
  order.insert(pair2);

  std::pair<int, int> pair1(pc->m_uPriorities[CR_HTTP], CR_HTTP);
  order.insert(pair1);

  std::multimap<int, int>::reverse_iterator rit;
  
  for(rit=order.rbegin(); rit!=order.rend(); rit++)
  {
    an.SetProgress(_T("[sending_attempt]"), 0);
    attempt++;    

    if(an.IsCancelled()){ break; }

    int id = rit->second;

    BOOL bResult = FALSE;

    if(id==CR_HTTP)
      bResult = SendOverHTTP(pc);    
    else if(id==CR_SMTP)
      bResult = SendOverSMTP(pc);  
    else if(id==CR_SMAPI)
      bResult = SendOverSMAPI(pc);

    if(bResult==FALSE)
      continue;

    if(id==CR_SMAPI && bResult==TRUE)
    {
      status = 0;
      break;
    }

    if(0==an.WaitForCompletion())
    {
      status = 0;
      break;
    }
  }

  if(status==0)
  {
    an.SetProgress(_T("[status_success]"), 0);
    // Move the ZIP to Recycle Bin
    CUtility::RecycleFile(pc->m_sZipName, false);
  }
  else
  {
    CString str;
    str.Format(_T("The error report is saved to '%s'"), pc->m_sZipName);
    an.SetProgress(str, 0);    
    an.SetProgress(_T("[status_failed]"), 0);    
  }

  an.SetCompleted(status);
  
  return 0;
}

