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
#include <map>
#include "tinyxml.h"

class CCrashDescReader
{
public:

    CCrashDescReader();
    ~CCrashDescReader();

    int Load(CString sFileName);

    bool m_bLoaded;

    DWORD m_dwGeneratorVersion;

    CString m_sCrashGUID;
    CString m_sAppName;
    CString m_sAppVersion;
    CString m_sImageName;
    CString m_sOperatingSystem;
    BOOL    m_bOSIs64Bit;
    CString m_sSystemTimeUTC;
    CString m_sGeoLocation;

    DWORD m_dwExceptionType;
    DWORD m_dwExceptionCode;

    DWORD m_dwFPESubcode;

    CString m_sInvParamExpression;
    CString m_sInvParamFunction;
    CString m_sInvParamFile;
    DWORD m_dwInvParamLine;

    CString m_sUserEmail;
    CString m_sProblemDescription;

    CString m_sMemoryUsageKbytes;
    CString m_sGUIResourceCount;
    CString m_sOpenHandleCount;

    std::map<CString, CString> m_aFileItems;
    std::map<CString, CString> m_aCustomProps;

private:

    int LoadXmlv10(TiXmlHandle hDoc);
};

