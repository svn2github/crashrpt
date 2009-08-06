#pragma once
#include "stdafx.h"
#include <vector>

struct SenderThreadContext
{ 
  CString m_sAppName;
  CString m_sAppVersion;
  CString m_sEmailFrom;
  CString m_sEmailTo;
  CString m_sEmailSubject;
  CString m_sEmailText;
  CString m_sUrl;
  CString m_sZipName;
  UINT m_uPriorities[3];
};

void GetSenderThreadStatus(int& nProgressPct, std::vector<CString>& msg_log);
void CancelSenderThread();
void FeedbackReady(int code);
DWORD WINAPI SenderThread(LPVOID lpParam);

