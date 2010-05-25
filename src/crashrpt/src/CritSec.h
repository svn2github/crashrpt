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

// File: CritSec.h
// Description: Critical section wrapper classes. Code of CCritSec and CAutoLock classes 
// is taken from DirectShow base classes and modified in some way.
// Authors: zexspectrum
// Date: 

#ifndef _CRITSEC_H
#define _CRITSEC_H

// wrapper for whatever critical section we have
class CCritSec 
{
  // make copy constructor and assignment operator inaccessible

  CCritSec(const CCritSec &refCritSec);
  CCritSec &operator=(const CCritSec &refCritSec);

  CRITICAL_SECTION m_CritSec;

public:

    CCritSec() 
    {
      InitializeCriticalSection(&m_CritSec);
    };

    ~CCritSec() 
    {
      DeleteCriticalSection(&m_CritSec);
    }

    void Lock() 
    {
      EnterCriticalSection(&m_CritSec);
    };

    void Unlock() 
    {
      LeaveCriticalSection(&m_CritSec);
    };
};

// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class CAutoLock 
{
  // make copy constructor and assignment operator inaccessible

  CAutoLock(const CAutoLock &refAutoLock);
  CAutoLock &operator=(const CAutoLock &refAutoLock);

protected:
  CCritSec * m_pLock;

public:
  CAutoLock(CCritSec * plock)
  {
    m_pLock = plock;
    m_pLock->Lock();
  };

  ~CAutoLock() 
  {
    m_pLock->Unlock();
  };
};


#endif  //_CRITSEC_H