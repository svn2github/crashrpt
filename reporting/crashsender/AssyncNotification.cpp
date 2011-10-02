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
#include "AssyncNotification.h"

AssyncNotification::AssyncNotification()
{
    // Init variables
    m_hCompletionEvent = CreateEvent(0, FALSE, FALSE, 0);
    m_hCancelEvent = CreateEvent(0, FALSE, FALSE, 0);
    m_hFeedbackEvent = CreateEvent(0, FALSE, FALSE, 0);

    Reset();
}

void AssyncNotification::Reset()
{ 
    // Reset the event

    m_cs.Lock(); // Acquire lock

    m_nCompletionStatus = -1;    
    m_nPercentCompleted = 0;
    m_statusLog.clear();

    ResetEvent(m_hCancelEvent);
    ResetEvent(m_hCompletionEvent);
    ResetEvent(m_hFeedbackEvent);

    m_cs.Unlock(); // Free lock
}

void AssyncNotification::SetProgress(CString sStatusMsg, int percentCompleted, bool bRelative)
{
    m_cs.Lock(); // Acquire lock

    m_statusLog.push_back(sStatusMsg);

    if(bRelative) // Update progress relatively to its previous value
    {
        m_nPercentCompleted += percentCompleted;
        if(m_nPercentCompleted>100)
            m_nPercentCompleted = 100;      
    }
    else // Update progress relatively to zero
    {
        m_nPercentCompleted = percentCompleted;
    }

    m_cs.Unlock(); // Free lock
}

void AssyncNotification::SetProgress(int percentCompleted, bool bRelative)
{
    m_cs.Lock(); // Acquire lock

    if(bRelative) // Update progress relatively to its previous value
    {
        m_nPercentCompleted += percentCompleted;
        if(m_nPercentCompleted>100)
            m_nPercentCompleted = 100;      
    }
    else // Update progress relatively to zero
    {
        m_nPercentCompleted = percentCompleted;
    }

    m_cs.Unlock(); // Free lock
}

void AssyncNotification::GetProgress(int& nProgressPct, std::vector<CString>& msg_log)
{
    msg_log.clear(); // Init message log (clear it)

    m_cs.Lock(); // Acquire lock

    nProgressPct = m_nPercentCompleted;
    msg_log = m_statusLog;
    m_statusLog.clear();

    m_cs.Unlock(); // Free lock
}

void AssyncNotification::SetCompleted(int nCompletionStatus)
{
    // Notifies about assynchronious operation completion
    m_cs.Lock(); // Acquire lock
    m_nCompletionStatus = nCompletionStatus;
    m_cs.Unlock(); // Free lock
    SetEvent(m_hCompletionEvent); // Set event
}

int AssyncNotification::WaitForCompletion()
{
    // Blocks until assynchronous operation is completed
    WaitForSingleObject(m_hCompletionEvent, INFINITE);

    // Get completion status
    int status = -1;
    m_cs.Lock(); // Acquire lock
    status = m_nCompletionStatus;
    m_cs.Unlock(); // Free lock

    return status;
}

void AssyncNotification::Cancel()
{
    // Cansels the assync operation
    SetProgress(_T("[cancelled_by_user]"), 0);
    SetEvent(m_hCancelEvent);
}

bool AssyncNotification::IsCancelled()
{
    // Determines if the assync operation is cancelled or not
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
    // Waits until the main thread's signal
    ResetEvent(m_hFeedbackEvent);      
    WaitForSingleObject(m_hFeedbackEvent, INFINITE);
    m_cs.Lock();
    code = m_nCompletionStatus;
    m_cs.Unlock();
}

void AssyncNotification::FeedbackReady(int code)
{
    // Sends signal to the waiting thread
    m_cs.Lock();
    m_nCompletionStatus = code;
    m_cs.Unlock();
    SetEvent(m_hFeedbackEvent);      
}
