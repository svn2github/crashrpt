/************************************************************************************* 
This file is a part of CrashRpt library.
Copyright (c) 2003-2012 The CrashRpt project authors. All Rights Reserved.

Use of this source code is governed by a BSD-style license
that can be found in the License.txt file in the root of the source
tree. All contributing project authors may
be found in the Authors.txt file in the root of the source tree.
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