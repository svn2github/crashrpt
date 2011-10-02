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

// File: CrashInfoReader.cpp
// Description: Retrieves crash information passed from CrashRpt.dll in form of XML files.
// Authors: zexspectrum
// Date: 2010

#include "stdafx.h"
#include "CrashRpt.h"
#include "CrashInfoReader.h"
#include "strconv.h"
#include "tinyxml.h"
#include "Utility.h"
#include "SharedMem.h"

// Define global CCrashInfoReader object
CCrashInfoReader g_CrashInfo;

int CCrashInfoReader::Init(LPCTSTR szFileMappingName)
{ 
    strconv_t strconv;
    ErrorReportInfo eri;

    // Init shared memory
    BOOL bInitMem = m_SharedMem.Init(szFileMappingName, TRUE, 0);
    if(!bInitMem)
        return 1;

    m_pCrashDesc = (CRASH_DESCRIPTION*)m_SharedMem.CreateView(0, sizeof(CRASH_DESCRIPTION));

    int nUnpack = UnpackCrashDescription(eri);
    if(0!=nUnpack)
    {
        return 2;
    }

    BOOL bCreateFolder = Utility::CreateFolder(m_sUnsentCrashReportsFolder);
    if(!bCreateFolder)
        return 3;

    m_sINIFile = m_sUnsentCrashReportsFolder + _T("\\~CrashRpt.ini");          

    if(!m_bSendRecentReports) // We should send report immediately
    { 
        CollectMiscCrashInfo(eri);

        eri.m_sErrorReportDirName = m_sUnsentCrashReportsFolder + _T("\\") + eri.m_sCrashGUID;
        Utility::CreateFolder(eri.m_sErrorReportDirName);

        m_Reports.push_back(eri);
    }  
    else // We should look for pending error reports
    {
        // Unblock the parent process
        CString sEventName;
        sEventName.Format(_T("Local\\CrashRptEvent_%s"), eri.m_sCrashGUID);
        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, sEventName);
        if(hEvent!=NULL)
            SetEvent(hEvent);

        // Look for pending error reports and add them to the list
        CString sSearchPattern = m_sUnsentCrashReportsFolder + _T("\\*");
        CFindFile find;
        BOOL bFound = find.FindFile(sSearchPattern);
        while(bFound)
        {
            if(find.IsDirectory() && !find.IsDots())
            {
                CString sErrorReportDirName = m_sUnsentCrashReportsFolder + _T("\\") + 
                    find.GetFileName();
                CString sFileName = sErrorReportDirName + _T("\\crashrpt.xml");
                ErrorReportInfo eri2;
                eri2.m_sErrorReportDirName = sErrorReportDirName;
                if(0==ParseCrashDescription(sFileName, TRUE, eri2))
                {          
                    eri2.m_uTotalSize = GetUncompressedReportSize(eri2);
                    m_Reports.push_back(eri2);
                }
            }

            bFound = find.FindNextFile();
        }
    }

    return 0;
}

int CCrashInfoReader::UnpackCrashDescription(ErrorReportInfo& eri)
{
    if(memcmp(m_pCrashDesc->m_uchMagic, "CRD", 3)!=0)
        return 1; // Invalid magic word

    if(m_pCrashDesc->m_dwCrashRptVer!=CRASHRPT_VER)
        return 2; // Invalid CrashRpt version

    // Unpack process ID, thread ID and exception pointers address.
    m_dwProcessId = m_pCrashDesc->m_dwProcessId;
    m_dwThreadId = m_pCrashDesc->m_dwThreadId;
    m_pExInfo = m_pCrashDesc->m_pExceptionPtrs;  
    m_bSendRecentReports = m_pCrashDesc->m_bSendRecentReports;
    m_nExceptionType = m_pCrashDesc->m_nExceptionType;
    if(m_nExceptionType==CR_SEH_EXCEPTION)
    {
        m_dwExceptionCode = m_pCrashDesc->m_dwExceptionCode;    
    }
    else if(m_nExceptionType==CR_CPP_SIGFPE)
    {
        m_uFPESubcode = m_pCrashDesc->m_uFPESubcode;
    }
    else if(m_nExceptionType==CR_CPP_INVALID_PARAMETER)
    {
        UnpackString(m_pCrashDesc->m_dwInvParamExprOffs, m_sInvParamExpr);  
        UnpackString(m_pCrashDesc->m_dwInvParamFunctionOffs, m_sInvParamFunction);  
        UnpackString(m_pCrashDesc->m_dwInvParamFileOffs, m_sInvParamFile);      
        m_uInvParamLine = m_pCrashDesc->m_uInvParamLine;
    }

    // Unpack other info
    UnpackString(m_pCrashDesc->m_dwAppNameOffs, eri.m_sAppName);
    m_sAppName = eri.m_sAppName;
    UnpackString(m_pCrashDesc->m_dwAppVersionOffs, eri.m_sAppVersion);  
    UnpackString(m_pCrashDesc->m_dwCrashGUIDOffs, eri.m_sCrashGUID);  
    UnpackString(m_pCrashDesc->m_dwImageNameOffs, eri.m_sImageName);  
    // Unpack install flags
    DWORD dwInstallFlags = m_pCrashDesc->m_dwInstallFlags;   
    m_bHttpBinaryEncoding = (dwInstallFlags&CR_INST_HTTP_BINARY_ENCODING)!=0;
    m_bSilentMode = (dwInstallFlags&CR_INST_NO_GUI)!=0;    
    m_bSendErrorReport = (dwInstallFlags&CR_INST_DONT_SEND_REPORT)==0;
    m_bStoreZIPArchives = (dwInstallFlags&CR_INST_STORE_ZIP_ARCHIVES)!=0;
    m_bAppRestart = (dwInstallFlags&CR_INST_APP_RESTART)!=0;
    m_bGenerateMinidump = (dwInstallFlags&CR_INST_NO_MINIDUMP)==0;
    m_bQueueEnabled = (dwInstallFlags&CR_INST_SEND_QUEUED_REPORTS)!=0;
    m_MinidumpType = m_pCrashDesc->m_MinidumpType;
    //m_bAppRestart = m_pCrashDesc->m_bAppRestart (unpacked from dwFlags);
    UnpackString(m_pCrashDesc->m_dwRestartCmdLineOffs, m_sRestartCmdLine);
    UnpackString(m_pCrashDesc->m_dwUrlOffs, m_sUrl);
    UnpackString(m_pCrashDesc->m_dwEmailToOffs, m_sEmailTo);  
    m_nSmtpPort = m_pCrashDesc->m_nSmtpPort;
    UnpackString(m_pCrashDesc->m_dwSmtpProxyServerOffs, m_sSmtpProxyServer);
    m_nSmtpProxyPort = m_pCrashDesc->m_nSmtpProxyPort;
    UnpackString(m_pCrashDesc->m_dwEmailSubjectOffs, m_sEmailSubject);
    UnpackString(m_pCrashDesc->m_dwEmailTextOffs, m_sEmailText);  
    memcpy(m_uPriorities, m_pCrashDesc->m_uPriorities, sizeof(UINT)*3);
    UnpackString(m_pCrashDesc->m_dwPrivacyPolicyURLOffs, m_sPrivacyPolicyURL);
    UnpackString(m_pCrashDesc->m_dwLangFileNameOffs, m_sLangFileName);  
    UnpackString(m_pCrashDesc->m_dwPathToDebugHelpDllOffs, m_sDbgHelpPath);
    UnpackString(m_pCrashDesc->m_dwUnsentCrashReportsFolderOffs, m_sUnsentCrashReportsFolder);
    m_bAddScreenshot = m_pCrashDesc->m_bAddScreenshot;
    m_dwScreenshotFlags = m_pCrashDesc->m_dwScreenshotFlags; 
    m_nJpegQuality = m_pCrashDesc->m_nJpegQuality;
    UnpackString(m_pCrashDesc->m_dwCustomSenderIconOffs, m_sCustomSenderIcon);  

    DWORD dwOffs = m_pCrashDesc->m_wSize;
    while(dwOffs<m_pCrashDesc->m_dwTotalSize)
    {
        LPBYTE pView = m_SharedMem.CreateView(dwOffs, sizeof(GENERIC_HEADER));
        GENERIC_HEADER* pHeader = (GENERIC_HEADER*)pView;

        if(memcmp(pHeader->m_uchMagic, "FIL", 3)==0)
        {
            // File item entry
            FILE_ITEM* pFileItem = (FILE_ITEM*)m_SharedMem.CreateView(dwOffs, pHeader->m_wSize);

            ERIFileItem fi;
            UnpackString(pFileItem->m_dwSrcFilePathOffs, fi.m_sSrcFile);
            UnpackString(pFileItem->m_dwDstFileNameOffs, fi.m_sDestFile);
            UnpackString(pFileItem->m_dwDescriptionOffs, fi.m_sDesc);
            fi.m_bMakeCopy = pFileItem->m_bMakeCopy;

            eri.m_FileItems[fi.m_sDestFile] = fi;

            m_SharedMem.DestroyView((LPBYTE)pFileItem);
        }
        else if(memcmp(pHeader->m_uchMagic, "CPR",3 )==0)
        {
            // Custom prop entry
            CUSTOM_PROP* pProp = (CUSTOM_PROP*)m_SharedMem.CreateView(dwOffs, pHeader->m_wSize);

            CString sName;
            CString sValue;
            UnpackString(pProp->m_dwNameOffs, sName);
            UnpackString(pProp->m_dwValueOffs, sValue);      

            eri.m_Props[sName] = sValue;

            m_SharedMem.DestroyView((LPBYTE)pProp);
        }
        else if(memcmp(pHeader->m_uchMagic, "REG", 3)==0)
        {
            // Reg key entry
            REG_KEY* pKey = (REG_KEY*)m_SharedMem.CreateView(dwOffs, pHeader->m_wSize);

            CString sKeyName;
            CString sDstFile;
            UnpackString(pKey->m_dwRegKeyNameOffs, sKeyName);
            UnpackString(pKey->m_dwDstFileNameOffs, sDstFile);      

            eri.m_RegKeys[sKeyName] = sDstFile;

            m_SharedMem.DestroyView((LPBYTE)pKey);
        }
        else if(memcmp(pHeader->m_uchMagic, "STR", 3)==0)
        {
            // Skip string
        }
        else
        {
            ATLASSERT(0); // Unknown header
            return 1;
        }

        dwOffs += pHeader->m_wSize;

        m_SharedMem.DestroyView(pView);
    }

    // Perform some integrity checks

    if(m_pCrashDesc->m_uFileItems!=eri.m_FileItems.size())
    {
        ATLASSERT(0);
        return 1; 
    }

    if(m_pCrashDesc->m_uCustomProps!=eri.m_Props.size())
    {
        ATLASSERT(0);
        return 1; 
    }

    if(m_pCrashDesc->m_uRegKeyEntries!=eri.m_RegKeys.size())
    {
        ATLASSERT(0);
        return 1; 
    }

    // Success
    return 0;
}

int CCrashInfoReader::UnpackString(DWORD dwOffset, CString& str)
{
    STRING_DESC* pStrDesc = (STRING_DESC*)m_SharedMem.CreateView(dwOffset, sizeof(STRING_DESC));
    if(memcmp(pStrDesc, "STR", 3)!=0)
        return 1;

    WORD wLength = pStrDesc->m_wSize;
    if(wLength<=sizeof(STRING_DESC))
        return 2;

    WORD wStrLen = wLength-sizeof(STRING_DESC);

    m_SharedMem.DestroyView((LPBYTE)pStrDesc);
    LPBYTE pStrData = m_SharedMem.CreateView(dwOffset+sizeof(STRING_DESC), wStrLen);
    str = CString((LPCTSTR)pStrData, wStrLen/sizeof(TCHAR));
    m_SharedMem.DestroyView(pStrData);

    return 0;
}

ErrorReportInfo& CCrashInfoReader::GetReport(int nIndex)
{ 
    return m_Reports[nIndex]; 
}

int CCrashInfoReader::GetReportCount()
{ 
    return (int)m_Reports.size(); 
}

void CCrashInfoReader::CollectMiscCrashInfo(ErrorReportInfo& eri)
{   
    // Get crash time
    Utility::GetSystemTimeUTC(eri.m_sSystemTimeUTC);

    // Open parent process handle
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION, 
        FALSE, 
        m_dwProcessId);

    if(hProcess!=NULL)
    {
        // Get number of GUI resources in use  
        eri.m_dwGuiResources = GetGuiResources(hProcess, GR_GDIOBJECTS);

        // Determine if GetProcessHandleCount function available
        typedef BOOL (WINAPI *LPGETPROCESSHANDLECOUNT)(HANDLE, PDWORD);
        HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
        if(hKernel32!=NULL)
        {
            LPGETPROCESSHANDLECOUNT pfnGetProcessHandleCount = 
                (LPGETPROCESSHANDLECOUNT)GetProcAddress(hKernel32, "GetProcessHandleCount");
            if(pfnGetProcessHandleCount!=NULL)
            {    
                // Get count of opened handles
                DWORD dwHandleCount = 0;
                BOOL bGetHandleCount = pfnGetProcessHandleCount(hProcess, &dwHandleCount);
                if(bGetHandleCount)
                    eri.m_dwProcessHandleCount = dwHandleCount;
                else
                    eri.m_dwProcessHandleCount = 0;
            }

            FreeLibrary(hKernel32);
            hKernel32=NULL;
        }

        // Get memory usage info
        PROCESS_MEMORY_COUNTERS meminfo;
        BOOL bGetMemInfo = GetProcessMemoryInfo(hProcess, &meminfo, 
            sizeof(PROCESS_MEMORY_COUNTERS));
        if(bGetMemInfo)
        {    
            CString sMemUsage;
#ifdef _WIN64
            sMemUsage.Format(_T("%I64u"), meminfo.WorkingSetSize/1024);
#else
            sMemUsage.Format(_T("%lu"), meminfo.WorkingSetSize/1024);
#endif 
            eri.m_sMemUsage = sMemUsage;
        }

        // Determine the period of time the process is working.
        FILETIME CreationTime, ExitTime, KernelTime, UserTime;
        /*BOOL bGetTimes = */GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime);
        /*ATLASSERT(bGetTimes);*/
        SYSTEMTIME AppStartTime;
        FileTimeToSystemTime(&CreationTime, &AppStartTime);

        SYSTEMTIME CurTime;
        GetSystemTime(&CurTime);
        ULONG64 uCurTime = Utility::SystemTimeToULONG64(CurTime);
        ULONG64 uStartTime = Utility::SystemTimeToULONG64(AppStartTime);

        // Check that the application works for at least one minute before crash.
        // This might help to avoid cyclic error report generation when the applciation
        // crashes on startup.
        double dDiffTime = (double)(uCurTime-uStartTime)*10E-08;
        if(dDiffTime<60)
        {
            m_bAppRestart = FALSE; // Disable restart.
        } 
    }

    // Get operating system friendly name from registry.
    Utility::GetOSFriendlyName(eri.m_sOSName);

    // Determine if Windows is 64-bit.
    eri.m_bOSIs64Bit = Utility::IsOS64Bit();

    // Get geographic location.
    Utility::GetGeoLocation(eri.m_sGeoLocation);  
}

int CCrashInfoReader::ParseFileList(TiXmlHandle& hRoot, ErrorReportInfo& eri)
{
    strconv_t strconv;

    TiXmlHandle fl = hRoot.FirstChild("FileList");
    if(fl.ToElement()==0)
    {    
        return 1;
    }

    TiXmlHandle fi = fl.FirstChild("FileItem");
    while(fi.ToElement()!=0)
    {
        const char* pszDestFile = fi.ToElement()->Attribute("destfile");
        const char* pszSrcFile = fi.ToElement()->Attribute("srcfile");
        const char* pszDesc = fi.ToElement()->Attribute("description");
        const char* pszMakeCopy = fi.ToElement()->Attribute("makecopy");

        if(pszDestFile!=NULL)
        {
            CString sDestFile = strconv.utf82t(pszDestFile);      
            ERIFileItem item;
            item.m_sDestFile = sDestFile;
            if(pszSrcFile)
                item.m_sSrcFile = strconv.utf82t(pszSrcFile);
            if(pszDesc)
                item.m_sDesc = strconv.utf82t(pszDesc);

            if(pszMakeCopy)
            {
                if(strcmp(pszMakeCopy, "1")==0)
                    item.m_bMakeCopy = TRUE;
                else
                    item.m_bMakeCopy = FALSE;
            }
            else
                item.m_bMakeCopy = FALSE;

            eri.m_FileItems[sDestFile] = item;
        }

        fi = fi.ToElement()->NextSibling("FileItem");
    }

    return 0;
}

int CCrashInfoReader::ParseRegKeyList(TiXmlHandle& hRoot, ErrorReportInfo& eri)
{
    strconv_t strconv;

    TiXmlHandle fl = hRoot.FirstChild("RegKeyList");
    if(fl.ToElement()==0)
    {    
        return 1;
    }

    TiXmlHandle fi = fl.FirstChild("RegKey");
    while(fi.ToElement()!=0)
    {
        const char* pszDestFile = fi.ToElement()->Attribute("destfile");
        const char* pszRegKey = fi.ToElement()->Attribute("name");

        if(pszDestFile!=NULL && pszRegKey!=NULL)
        {
            CString sDestFile = strconv.utf82t(pszDestFile);      
            CString sRegKey = strconv.utf82t(pszRegKey);

            eri.m_RegKeys[sRegKey] = sDestFile;
        }

        fi = fi.ToElement()->NextSibling("RegKey");
    }

    return 0;
}

int CCrashInfoReader::ParseCrashDescription(CString sFileName, BOOL bParseFileItems, ErrorReportInfo& eri)
{
    strconv_t strconv;
    FILE* f = NULL; 

#if _MSC_VER<1400
    f = _tfopen(sFileName, _T("rb"));
#else
    _tfopen_s(&f, sFileName, _T("rb"));
#endif

    if(f==NULL)
        return 1;

    TiXmlDocument doc;
    bool bOpen = doc.LoadFile(f);
    if(!bOpen)
        return 1;

    TiXmlHandle hRoot = doc.FirstChild("CrashRpt");
    if(hRoot.ToElement()==NULL)
    {
        fclose(f);
        return 1;
    }

    {
        TiXmlHandle hCrashGUID = hRoot.FirstChild("CrashGUID");
        if(hCrashGUID.ToElement()!=NULL)
        {
            if(hCrashGUID.FirstChild().ToText()!=NULL)
            {
                const char* szCrashGUID = hCrashGUID.FirstChild().ToText()->Value();
                if(szCrashGUID!=NULL)
                    eri.m_sCrashGUID = strconv.utf82t(szCrashGUID);
            }
        }
    }

    {
        TiXmlHandle hAppName = hRoot.FirstChild("AppName");
        if(hAppName.ToElement()!=NULL)
        {
            const char* szAppName = hAppName.FirstChild().ToText()->Value();
            if(szAppName!=NULL)
                eri.m_sAppName = strconv.utf82t(szAppName);
        }
    }

    {
        TiXmlHandle hAppVersion = hRoot.FirstChild("AppVersion");
        if(hAppVersion.ToElement()!=NULL)
        {
            TiXmlText* pText = hAppVersion.FirstChild().ToText();
            if(pText!=NULL)
            {
                const char* szAppVersion = pText->Value();
                if(szAppVersion!=NULL)
                    eri.m_sAppVersion = strconv.utf82t(szAppVersion);
            }
        }
    }

    {
        TiXmlHandle hImageName = hRoot.FirstChild("ImageName");
        if(hImageName.ToElement()!=NULL)
        {
            TiXmlText* pText = hImageName.FirstChild().ToText();
            if(pText!=NULL)
            {
                const char* szImageName = pText->Value();
                if(szImageName!=NULL)
                    eri.m_sImageName = strconv.utf82t(szImageName);
            }
        }
    }

    {
        TiXmlHandle hSystemTimeUTC = hRoot.FirstChild("SystemTimeUTC");
        if(hSystemTimeUTC.ToElement()!=NULL)
        {
            TiXmlText* pText = hSystemTimeUTC.FirstChild().ToText();
            if(pText!=NULL)
            {
                const char* szSystemTimeUTC = pText->Value();
                if(szSystemTimeUTC!=NULL)
                    eri.m_sSystemTimeUTC = strconv.utf82t(szSystemTimeUTC);
            }
        }
    }

    if(bParseFileItems)
    {
        // Get directory name
        CString sReportDir = sFileName;
        int pos = sFileName.ReverseFind('\\');
        if(pos>=0)
            sReportDir = sFileName.Left(pos);
        if(sReportDir.Right(1)!=_T("\\"))
            sReportDir += _T("\\");

        TiXmlHandle fl = hRoot.FirstChild("FileList");
        if(fl.ToElement()==0)
        {    
            fclose(f);
            return 1;
        }

        TiXmlHandle fi = fl.FirstChild("FileItem");
        while(fi.ToElement()!=0)
        {
            const char* pszDestFile = fi.ToElement()->Attribute("name");      
            const char* pszDesc = fi.ToElement()->Attribute("description");      

            if(pszDestFile!=NULL)
            {
                CString sDestFile = strconv.utf82t(pszDestFile);      
                ERIFileItem item;
                item.m_sDestFile = sDestFile;
                item.m_sSrcFile = sReportDir + sDestFile;
                if(pszDesc)
                    item.m_sDesc = strconv.utf82t(pszDesc);
                item.m_bMakeCopy = FALSE;

                // Check that file really exists
                DWORD dwAttrs = GetFileAttributes(item.m_sSrcFile);
                if(dwAttrs!=INVALID_FILE_ATTRIBUTES &&
                    (dwAttrs&FILE_ATTRIBUTE_DIRECTORY)==0)
                {
                    eri.m_FileItems[sDestFile] = item;
                }
            }

            fi = fi.ToElement()->NextSibling("FileItem");
        }    
    }

    fclose(f);
    return 0;
}

BOOL CCrashInfoReader::AddUserInfoToCrashDescriptionXML(CString sEmail, CString sDesc)
{ 
    strconv_t strconv;

    TiXmlDocument doc;

    CString sFileName = g_CrashInfo.m_Reports[0].m_sErrorReportDirName + _T("\\crashrpt.xml");

    FILE* f = NULL; 
#if _MSC_VER<1400
    f = _tfopen(sFileName, _T("rb"));
#else
    _tfopen_s(&f, sFileName, _T("rb"));
#endif

    if(f==NULL)
        return FALSE;

    bool bLoad = doc.LoadFile(f);
    fclose(f);
    if(!bLoad)
    {    
        return FALSE;
    }

    TiXmlNode* root = doc.FirstChild("CrashRpt");
    if(!root)
    {    
        return FALSE;
    }

    // Write user e-mail

    TiXmlElement* email = new TiXmlElement("UserEmail");
    root->LinkEndChild(email);

    TiXmlText* email_text = new TiXmlText(strconv.t2utf8(sEmail));
    email->LinkEndChild(email_text);              

    // Write problem description

    TiXmlElement* desc = new TiXmlElement("ProblemDescription");
    root->LinkEndChild(desc);

    TiXmlText* desc_text = new TiXmlText(strconv.t2utf8(sDesc));
    desc->LinkEndChild(desc_text);              

#if _MSC_VER<1400
    f = _tfopen(sFileName, _T("w"));
#else
    _tfopen_s(&f, sFileName, _T("w"));
#endif

    if(f==NULL)
        return FALSE;

    bool bSave = doc.SaveFile(f); 
    fclose(f);
    if(!bSave)
        return FALSE;
    return TRUE;
}

BOOL CCrashInfoReader::AddFilesToCrashDescriptionXML(std::vector<ERIFileItem> FilesToAdd)
{   
    strconv_t strconv;

    TiXmlDocument doc;

    CString sFileName = g_CrashInfo.m_Reports[0].m_sErrorReportDirName + _T("\\crashrpt.xml");

    FILE* f = NULL; 
#if _MSC_VER<1400
    f = _tfopen(sFileName, _T("rb"));
#else
    _tfopen_s(&f, sFileName, _T("rb"));
#endif

    if(f==NULL)
    {    
        return FALSE;
    }

    bool bLoad = doc.LoadFile(f);  
    fclose(f);
    if(!bLoad)
    { 
        return FALSE;
    }

    TiXmlNode* root = doc.FirstChild("CrashRpt");
    if(!root)
    { 
        return FALSE;
    }

    TiXmlHandle hFileItems = root->FirstChild("FileList");
    if(hFileItems.ToElement()==NULL)
    {
        hFileItems = new TiXmlElement("FileList");
        root->LinkEndChild(hFileItems.ToNode());
    }

    unsigned i;
    for(i=0; i<FilesToAdd.size(); i++)
    { 
        if(m_Reports[0].m_FileItems.find(FilesToAdd[i].m_sDestFile)!=m_Reports[0].m_FileItems.end())
            continue; // Such file item already exists, skip

        TiXmlHandle hFileItem = new TiXmlElement("FileItem");
        hFileItem.ToElement()->SetAttribute("name", strconv.t2utf8(FilesToAdd[i].m_sDestFile));
        hFileItem.ToElement()->SetAttribute("description", strconv.t2utf8(FilesToAdd[i].m_sDesc));
        hFileItems.ToElement()->LinkEndChild(hFileItem.ToNode());              

        m_Reports[0].m_FileItems[FilesToAdd[i].m_sDestFile] = FilesToAdd[i];
    }

#if _MSC_VER<1400
    f = _tfopen(sFileName, _T("w"));
#else
    _tfopen_s(&f, sFileName, _T("w"));
#endif

    if(f==NULL)
        return FALSE;

    bool bSave = doc.SaveFile(f); 
    if(!bSave)
        return FALSE;
    fclose(f);
    return TRUE;
}

BOOL CCrashInfoReader::GetLastRemindDate(SYSTEMTIME& LastDate)
{  
    CString sDate = Utility::GetINIString(m_sINIFile, _T("General"), _T("LastRemindDate"));
    if(sDate.IsEmpty())
        return FALSE;

    Utility::UTC2SystemTime(sDate, LastDate);
    return TRUE;
}

BOOL CCrashInfoReader::SetLastRemindDateToday()
{
    // Get current time
    CString sTime;
    Utility::GetSystemTimeUTC(sTime);

    // Write it to INI  
    Utility::SetINIString(m_sINIFile, _T("General"), _T("LastRemindDate"), sTime);

    return TRUE;
}

REMIND_POLICY CCrashInfoReader::GetRemindPolicy()
{  
    CString sPolicy = Utility::GetINIString(m_sINIFile, _T("General"), _T("RemindPolicy"));

    if(sPolicy.Compare(_T("RemindLater"))==0)
        return REMIND_LATER;
    else if(sPolicy.Compare(_T("NeverRemind"))==0)
        return NEVER_REMIND;

    Utility::SetINIString(m_sINIFile, _T("General"), _T("RemindPolicy"), _T("RemindLater"));
    return REMIND_LATER;
}

BOOL CCrashInfoReader::SetRemindPolicy(REMIND_POLICY Policy)
{
    CString sPolicy;
    if(Policy==REMIND_LATER)
        sPolicy = _T("RemindLater");
    else if(Policy==NEVER_REMIND)
        sPolicy = _T("NeverRemind");

    Utility::SetINIString(m_sINIFile, _T("General"), _T("RemindPolicy"), sPolicy);

    return TRUE;
}

BOOL CCrashInfoReader::IsRemindNowOK()
{
    if(GetRemindPolicy()!=REMIND_LATER)
        return FALSE; // User doesn want us to remind him

    // Get last remind date
    SYSTEMTIME LastRemind;
    if(!GetLastRemindDate(LastRemind))
    {    
        return TRUE;
    }

    // Determine the period of time elapsed since the last remind.
    SYSTEMTIME CurTimeUTC, CurTimeLocal;
    GetSystemTime(&CurTimeUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &CurTimeUTC, &CurTimeLocal);
    ULONG64 uCurTime = Utility::SystemTimeToULONG64(CurTimeLocal);
    ULONG64 uLastRemindTime = Utility::SystemTimeToULONG64(LastRemind);

    // Check that at lease one week elapsed
    double dDiffTime = (double)(uCurTime-uLastRemindTime)*10E-08;
    if(dDiffTime<7*24*60*60)
    {
        return FALSE;
    }

    return TRUE;
}

LONG64 CCrashInfoReader::GetUncompressedReportSize(ErrorReportInfo& eri)
{
    LONG64 lTotalSize = 0;
    std::map<CString, ERIFileItem>::iterator it;
    HANDLE hFile = INVALID_HANDLE_VALUE;  
    CString sMsg;
    BOOL bGetSize = FALSE;
    LARGE_INTEGER lFileSize;

    for(it=eri.m_FileItems.begin(); it!=eri.m_FileItems.end(); it++)
    {   
        CString sFileName = it->second.m_sSrcFile.GetBuffer(0);
        hFile = CreateFile(sFileName, 
            GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL); 
        if(hFile==INVALID_HANDLE_VALUE)
            continue;

        bGetSize = GetFileSizeEx(hFile, &lFileSize);
        if(!bGetSize)
        {
            CloseHandle(hFile);
            continue;
        }

        lTotalSize += lFileSize.QuadPart;
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    return lTotalSize;
}

HICON CCrashInfoReader::GetCustomIcon()
{
    if(!g_CrashInfo.m_sCustomSenderIcon.IsEmpty())
    {
        CString sResourceFile;
        CString sIconIndex;
        int nIconIndex = 0;

        int nComma = g_CrashInfo.m_sCustomSenderIcon.ReverseFind(',');    
        if(nComma>=0)
        {
            sResourceFile = g_CrashInfo.m_sCustomSenderIcon.Left(nComma);      
            sIconIndex = g_CrashInfo.m_sCustomSenderIcon.Mid(nComma+1);
            sIconIndex.TrimLeft();
            sIconIndex.TrimRight();
            nIconIndex = _ttoi(sIconIndex);      
        }
        else
        {
            sResourceFile = g_CrashInfo.m_sCustomSenderIcon;
        }

        sResourceFile.TrimRight();        

        if(nIconIndex==-1)
        {      
            return NULL;
        }

        // Check that custom icon can be loaded
        HICON hIcon = ExtractIcon(NULL, sResourceFile, nIconIndex);
        if(hIcon==NULL || hIcon==(HICON)1)
        { 
            return NULL;
        }

        return hIcon;
    }

    return NULL;
}