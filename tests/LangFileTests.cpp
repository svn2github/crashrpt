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
#include "Tests.h"
#include "CrashRpt.h"
#include "Utility.h"
#include "strconv.h"

class LangFileTests : public CTestSuite
{
    BEGIN_TEST_MAP(LangFileTests, "CrashRpt language file tests")
        REGISTER_TEST(Test_lang_file_versions);    
    END_TEST_MAP()

public:

    void SetUp();
    void TearDown();

    void Test_lang_file_versions();
};


REGISTER_TEST_SUITE( LangFileTests );

void LangFileTests::SetUp()
{  
}

void LangFileTests::TearDown()
{
}

void LangFileTests::Test_lang_file_versions()
{ 
    // This test ensures that CrashRptVersion field of all CrashRpt language files
    // equals to CRASHRPT_VER constant value

    if(g_bRunningFromUNICODEFolder)
        return; // Skip this test if running from another process

    std::vector<CString> asLangAbbr;
    asLangAbbr.push_back(_T("DE"));
    asLangAbbr.push_back(_T("EN"));
    asLangAbbr.push_back(_T("ES"));
    asLangAbbr.push_back(_T("FR"));
    asLangAbbr.push_back(_T("HI"));
    asLangAbbr.push_back(_T("IT"));
    asLangAbbr.push_back(_T("PT"));
    asLangAbbr.push_back(_T("RU"));
    asLangAbbr.push_back(_T("ZH-CN"));

    const int BUFF_SIZE = 1024;
    TCHAR szBuffer[BUFF_SIZE]=_T("");

    UINT i;
    for(i=0; i<asLangAbbr.size(); i++)
    {
        CString sExePath = Utility::GetModulePath(NULL);
        CString sFileName;
#ifndef WIN64
        sFileName.Format(_T("%s\\..\\lang_files\\crashrpt_lang_%s.ini"), 
            sExePath.GetBuffer(0), asLangAbbr[i].GetBuffer(0));
#else
        sFileName.Format(_T("%s\\..\\..\\lang_files\\crashrpt_lang_%s.ini"), 
            sExePath.GetBuffer(0), asLangAbbr[i].GetBuffer(0));
#endif //!WIN64

        GetPrivateProfileString(_T("Settings"), _T("CrashRptVersion"), _T(""), szBuffer, BUFF_SIZE, sFileName);

        int nActualVer = _ttoi(szBuffer);

        TEST_ASSERT(nActualVer==CRASHRPT_VER);
    }

    __TEST_CLEANUP__;


}