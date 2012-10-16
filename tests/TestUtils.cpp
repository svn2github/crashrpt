/************************************************************************************* 
This file is a part of CrashRpt library.
Copyright (c) 2003-2012 The CrashRpt project authors. All Rights Reserved.

Use of this source code is governed by a BSD-style license
that can be found in the License.txt file in the root of the source
tree. All contributing project authors may
be found in the Authors.txt file in the root of the source tree.
***************************************************************************************/

#include "stdafx.h"
#include "TestUtils.h"
#include "CrashRpt.h"
#include "Utility.h"
#include "strconv.h"

// A helper function that creates a error report for testing
BOOL TestUtils::CreateErrorReport(CString sTmpFolder, CString& sErrorReportName, CString& sMD5Hash)
{
    BOOL bStatus = FALSE;
    CString sReportFolder;
    DWORD dwExitCode = 1;
    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    CString sSearchPattern = sTmpFolder + "\\*.zip";
    CString sMD5FileName;
    FILE* f = NULL;
    TCHAR szHashBuff[256] = _T("");
    HKEY hKey = NULL;
    LONG lResult = -1;
    CString sKeyName = _T("Software\\CrashRpt&&#4216wer\\应用程序名称");
    CString sKeyName2 = _T("HKEY_CURRENT_USER\\") + sKeyName;

    lResult = RegCreateKey(HKEY_CURRENT_USER, sKeyName, &hKey); 
    if(lResult!=ERROR_SUCCESS)
        goto cleanup;

    DWORD dwVal = 12345;
    lResult = RegSetValueEx(hKey, _T("Value$%^!@#&123fer"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));
    if(lResult!=ERROR_SUCCESS)
        goto cleanup;

    CR_INSTALL_INFOW infoW;
    memset(&infoW, 0, sizeof(CR_INSTALL_INFOW));
    infoW.cb = sizeof(CR_INSTALL_INFOW);  
    infoW.pszAppName = L"My& app Name & ' 应用程序名称"; // Use Chineese characters for app name
    // Use appname with restricted XML characters
    infoW.pszAppVersion = L"1.0.0 &<'a应> \"<"; 
    infoW.pszErrorReportSaveDir = sTmpFolder;
    infoW.dwFlags = CR_INST_NO_GUI|CR_INST_DONT_SEND_REPORT|CR_INST_STORE_ZIP_ARCHIVES;  

    int nInstallResult = crInstallW(&infoW);
    if(nInstallResult!=0)
        goto cleanup;

    crAddScreenshot(CR_AS_MAIN_WINDOW);
    crAddPropertyW(L"CustomProp", L"Property Value");
    crAddRegKey(sKeyName2, L"regkey.xml", 0);

    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(ei);
    ei.exctype = CR_SEH_EXCEPTION;
    ei.code = 0x123;

    // Generate error report
    int nGenResult = crGenerateErrorReport(&ei);
    if(nGenResult!=0)
        goto cleanup;

    // Wait until CrashSender process exits
    WaitForSingleObject(ei.hSenderProcess, INFINITE);

    // Check exit code  
    GetExitCodeProcess(ei.hSenderProcess, &dwExitCode);
    if(dwExitCode!=0)
        goto cleanup;

    // Get ZIP name  
    hFind = FindFirstFile(sSearchPattern, &ffd);
    if(hFind==INVALID_HANDLE_VALUE)
        goto cleanup;

    sErrorReportName = sTmpFolder + _T("\\") + CString(ffd.cFileName);

    FindClose(hFind);
    hFind = NULL;

    // Get MD5 name
    sSearchPattern = sTmpFolder + "\\*.md5";
    hFind = FindFirstFile(sSearchPattern, &ffd);
    if(hFind==INVALID_HANDLE_VALUE)
        goto cleanup;

    sMD5FileName = sTmpFolder + _T("\\") + CString(ffd.cFileName);

#if _MSC_VER < 1400
    f = _tfopen(sMD5FileName, _T("rt"));
#else
    _tfopen_s(&f, sMD5FileName, _T("rt"));
#endif
    if(f==NULL)
        goto cleanup;

    TCHAR* szHash = _fgetts(szHashBuff, 256, f);
    if(szHash==NULL)
        goto cleanup;

    sMD5Hash = szHash;

    if(sMD5Hash.GetLength()!=32)
        goto cleanup; // Hash must be 32 characters in length

    bStatus = TRUE;

cleanup:

    crUninstall();

    if(f!=NULL)
        fclose(f);

    if(hFind!=INVALID_HANDLE_VALUE)
        FindClose(hFind);

    if(hKey)    
        RegCloseKey(hKey);

    RegDeleteKey(HKEY_CURRENT_USER, sKeyName);

    return bStatus;
}