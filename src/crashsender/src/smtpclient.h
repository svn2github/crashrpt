#pragma once
#include "stdafx.h"

#include <set>
#include <map>
#include <vector>
#include <string>

class CEmailMessage
{
public:
  CString m_sSubject;
  CString m_sFrom;
  CString m_sTo;
  CString m_sText;
  std::set<CString> m_aAttachments;
};

struct AssyncNotification
{
  AssyncNotification()
  {
    m_nCompletionStatus = -1;    
    m_nPercentCompleted = 0;
    m_hCompletionEvent = CreateEvent(0, FALSE, FALSE, 0);
    m_hCancelEvent = CreateEvent(0, FALSE, FALSE, 0);
    m_hFeedbackEvent = CreateEvent(0, FALSE, FALSE, 0);
  }

  void SetProgress(CString sStatusMsg, int percentCompleted, bool bRelative=true)
  {
    m_cs.Lock();
  
    m_statusLog.push_back(sStatusMsg);
  
    if(bRelative)
    {
      m_nPercentCompleted += percentCompleted;
      if(m_nPercentCompleted>100)
        m_nPercentCompleted = 100;      
    }
    else
      m_nPercentCompleted = percentCompleted;

    m_cs.Unlock();
  }

  void GetProgress(int& nProgressPct, std::vector<CString>& msg_log)
  {
    msg_log.clear();
    
    m_cs.Lock();
    nProgressPct = m_nPercentCompleted;
    msg_log = m_statusLog;
    m_statusLog.clear();
    m_cs.Unlock();
  }

  void SetCompleted(int nCompletionStatus)
  {
    m_cs.Lock();
    m_nCompletionStatus = nCompletionStatus;
    m_cs.Unlock();
    SetEvent(m_hCompletionEvent);
  }

  int WaitForCompletion()
  {
    WaitForSingleObject(m_hCompletionEvent, INFINITE);
    
    int status = -1;
    m_cs.Lock();
    status = m_nCompletionStatus;
    m_cs.Unlock();

    return status;
  }

  void Cancel()
  {
    SetProgress(_T("[cancelled_by_user]"), 0);
    SetEvent(m_hCancelEvent);
  }

  bool IsCancelled()
  {
    DWORD dwWaitResult = WaitForSingleObject(m_hCancelEvent, 0);
    if(dwWaitResult==WAIT_OBJECT_0)
    {
      SetEvent(m_hCancelEvent);      
      return true;
    }
    
    return false;
  }

  void WaitForFeedback(int &code)
  {
    ResetEvent(m_hFeedbackEvent);      
    WaitForSingleObject(m_hFeedbackEvent, INFINITE);
    m_cs.Lock();
    code = m_nCompletionStatus;
    m_cs.Unlock();
  }

  void FeedbackReady(int code)
  {
    m_cs.Lock();
    m_nCompletionStatus = code;
    m_cs.Unlock();
    SetEvent(m_hFeedbackEvent);      
  }

private:

  CComAutoCriticalSection m_cs;
  int m_nCompletionStatus;
  HANDLE m_hCompletionEvent;
  HANDLE m_hCancelEvent;
  HANDLE m_hFeedbackEvent;
  int m_nPercentCompleted;
  std::vector<CString> m_statusLog;
};

class CSmtpClient
{
public:
  
  CSmtpClient();
  ~CSmtpClient();

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
  
  static int SendMsg(AssyncNotification*, SOCKET sock, LPCTSTR pszMessage, LPSTR pszResponce=0, UINT uResponceSize=0);

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
};



