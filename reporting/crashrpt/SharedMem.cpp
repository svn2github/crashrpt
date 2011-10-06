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
#include "SharedMem.h"

CSharedMem::CSharedMem()  
{
    m_uSize = 0;
    m_hFileMapping = NULL;  

    SYSTEM_INFO si;  
    GetSystemInfo(&si);
    m_dwAllocGranularity = si.dwAllocationGranularity;  
}

CSharedMem::~CSharedMem()
{
    Destroy();
}


BOOL CSharedMem::Init(LPCTSTR szName, BOOL bOpenExisting, ULONG64 uSize)
{
    if(m_hFileMapping!=NULL)
        return FALSE;

    if(!bOpenExisting)
    {	
        ULARGE_INTEGER i;
        i.QuadPart = uSize;
        m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, i.HighPart, i.LowPart, szName);
    }
    else
    {    
        m_hFileMapping = OpenFileMapping(FILE_MAP_READ|FILE_MAP_WRITE, FALSE, szName);
    }

    m_sName = szName;
    m_uSize = uSize; 

    if(m_hFileMapping==NULL)
    {
        m_uSize = 0;  
        return FALSE;
    }

    return TRUE;
}

BOOL CSharedMem::Destroy()
{
    std::map<LPBYTE, LPBYTE>::iterator it;
    for(it=m_aViewStartPtrs.begin(); it!=m_aViewStartPtrs.end(); it++)
    {
        UnmapViewOfFile(it ->second);    
    }
    m_aViewStartPtrs.clear();

    if(m_hFileMapping!=NULL)
    {
        CloseHandle(m_hFileMapping);    
    }

    m_hFileMapping = NULL;	
    m_uSize = 0;  

    return TRUE;
}

CString CSharedMem::GetName()
{
    return m_sName;
}

ULONG64 CSharedMem::GetSize()
{
    return m_uSize;
}

LPBYTE CSharedMem::CreateView(DWORD dwOffset, DWORD dwLength)
{
    DWORD dwBaseOffs = dwOffset-dwOffset%m_dwAllocGranularity;
    DWORD dwDiff = dwOffset-dwBaseOffs;
    LPBYTE pPtr = NULL;

    pPtr = (LPBYTE)MapViewOfFile(m_hFileMapping, FILE_MAP_READ|FILE_MAP_WRITE, 0, dwBaseOffs, dwLength + dwDiff);
    m_aViewStartPtrs[pPtr + dwDiff] = pPtr;

    return (pPtr+dwDiff);
}

void CSharedMem::DestroyView(LPBYTE pViewPtr)
{
    std::map<LPBYTE, LPBYTE>::iterator it = m_aViewStartPtrs.find(pViewPtr);
    if(it!=m_aViewStartPtrs.end())
    {
        UnmapViewOfFile(it ->second);
        m_aViewStartPtrs.erase(it);
    }
}
