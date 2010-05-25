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

#include "stdafx.h"
#include "AssyncNotification.h"

AssyncNotification::AssyncNotification()
{
  m_hCompletionEvent = CreateEvent(0, FALSE, FALSE, 0);
  m_hCancelEvent = CreateEvent(0, FALSE, FALSE, 0);
  m_hFeedbackEvent = CreateEvent(0, FALSE, FALSE, 0);

  Reset();
}

void AssyncNotification::Reset()
{  
  m_cs.Lock();

  m_nCompletionStatus = -1;    
  m_nPercentCompleted = 0;
  m_statusLog.clear();
  
  ResetEvent(m_hCancelEvent);
  ResetEvent(m_hCompletionEvent);
  ResetEvent(m_hFeedbackEvent);

  m_cs.Unlock();
}

void AssyncNotification::SetProgress(CString sStatusMsg, int percentCompleted, bool bRelative)
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

void AssyncNotification::SetProgress(int percentCompleted, bool bRelative)
{
  m_cs.Lock();
  
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

void AssyncNotification::GetProgress(int& nProgressPct, std::vector<CString>& msg_log)
{
  msg_log.clear();
  
  m_cs.Lock();
  nProgressPct = m_nPercentCompleted;
  msg_log = m_statusLog;
  m_statusLog.clear();
  m_cs.Unlock();
}

void AssyncNotification::SetCompleted(int nCompletionStatus)
{
  m_cs.Lock();
  m_nCompletionStatus = nCompletionStatus;
  m_cs.Unlock();
  SetEvent(m_hCompletionEvent);
}

int AssyncNotification::WaitForCompletion()
{
  WaitForSingleObject(m_hCompletionEvent, INFINITE);
  
  int status = -1;
  m_cs.Lock();
  status = m_nCompletionStatus;
  m_cs.Unlock();

  return status;
}

void AssyncNotification::Cancel()
{
  SetProgress(_T("[cancelled_by_user]"), 0);
  SetEvent(m_hCancelEvent);
}

bool AssyncNotification::IsCancelled()
{
  DWORD dwWaitResult = WaitForSingleObject(m_hCancelEvent, 0);
  if(dwWaitResult==WAIT_OBJECT_0)
  {
    SetEvent(m_hCancelEvent);      
    return true;
  }
  
  return false;
}

void AssyncNotification::WaitForFeedback(int &code)
{
  ResetEvent(m_hFeedbackEvent);      
  WaitForSingleObject(m_hFeedbackEvent, INFINITE);
  m_cs.Lock();
  code = m_nCompletionStatus;
  m_cs.Unlock();
}

void AssyncNotification::FeedbackReady(int code)
{
  m_cs.Lock();
  m_nCompletionStatus = code;
  m_cs.Unlock();
  SetEvent(m_hFeedbackEvent);      
}
