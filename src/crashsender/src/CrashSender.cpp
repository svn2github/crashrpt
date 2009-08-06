// CrashSender.cpp : main source file for CrashSender.exe
//

#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"
#include "tinyxml.h"
#include "smtpclient.h"
#include "httpsend.h"
#include "unzip.h"
#include "utility.h"
#include "CrashRpt.h"

CAppModule _Module;

int ParseCrashInfo(LPCSTR text, CString& sAppName, CString& sAppVersion, CString& sImageName,
  CString& sSubject, CString& sMailTo, CString& sUrl, UINT (*puPriorities)[3], CString& sZipName)
{  
  TiXmlDocument doc;
  doc.Parse(text);
  if(doc.Error())
    return 1;

  TiXmlHandle hRoot = doc.FirstChild("crashrpt");
  if(hRoot.ToElement()==NULL)
    return 1;

  const char* pszAppName = hRoot.ToElement()->Attribute("appname");
  const char* pszAppVersion= hRoot.ToElement()->Attribute("appver");
  const char* pszImageName = hRoot.ToElement()->Attribute("imagename");
  const char* pszSubject = hRoot.ToElement()->Attribute("subject");
  const char* pszMailTo = hRoot.ToElement()->Attribute("mailto");
  const char* pszUrl = hRoot.ToElement()->Attribute("url");
  const char* pszZipName = hRoot.ToElement()->Attribute("zipname");
  const char* pszHttpPriority = hRoot.ToElement()->Attribute("http_priority");
  const char* pszSmtpPriority = hRoot.ToElement()->Attribute("smtp_priority");
  const char* pszMapiPriority = hRoot.ToElement()->Attribute("mapi_priority");

  if(pszAppName)
    sAppName = pszAppName;

  if(pszAppVersion)
    sAppVersion = pszAppVersion;

  if(pszImageName)
    sImageName = pszImageName;

  if(pszSubject)
    sSubject = pszSubject;

  if(pszMailTo!=NULL)
    sMailTo = pszMailTo;

  if(pszUrl!=NULL)
    sUrl = pszUrl;

  if(pszZipName!=NULL)
    sZipName = pszZipName;

  if(pszHttpPriority!=NULL)
    (*puPriorities)[CR_HTTP] = atoi(pszHttpPriority);
  else
    (*puPriorities)[CR_HTTP] = 0;

  if(pszSmtpPriority!=NULL)
    (*puPriorities)[CR_SMTP] = atoi(pszSmtpPriority);
  else
    (*puPriorities)[CR_SMTP] = 0;

  if(pszMapiPriority!=NULL)
    (*puPriorities)[CR_SMAPI] = atoi(pszMapiPriority);
  else
    (*puPriorities)[CR_SMAPI] = 0;
  
  return 0;
}

int 
GetFileList(CString sZipName, std::map<std::string, std::string>& file_list)
{
  USES_CONVERSION;

  HZIP hz = OpenZip(sZipName, NULL);
  if(hz==NULL)
    return 1;

  int index = -1;
  ZIPENTRY ze;
  ZRESULT zr = FindZipItem(hz, _T("crashrpt.xml"), false, &index, &ze);
  if(zr!=ZR_OK)
  {
    CloseZip(hz);
    return 2;
  }

  CString sTempFileName = CUtility::getTempFileName();
  zr = UnzipItem(hz, index, sTempFileName);
  if(zr!=ZR_OK)
  {
    CloseZip(hz);
    return 2;
  }

  CString sTempDir = CUtility::getTempFileName();
  DeleteFile(sTempDir);

  BOOL bCreateDir = CreateDirectory(sTempDir, NULL);  
  ATLASSERT(bCreateDir);
  bCreateDir;
  
  LPSTR lpszTempFileName = T2A(sTempFileName.GetBuffer(0));

  TiXmlDocument doc;
  bool bLoad = doc.LoadFile(lpszTempFileName);
  if(!bLoad)
  {
    CloseZip(hz);
    return 3;
  }

  TiXmlHandle hRoot = doc.FirstChild("CrashRpt");
  if(hRoot.ToElement()==NULL)
  {
    CloseZip(hz);
    return 4;
  }
  
  TiXmlHandle fl = hRoot.FirstChild("FileList");
  if(fl.ToElement()==0)
  {
    CloseZip(hz);
    return 5;
  }

  TiXmlHandle fi = fl.FirstChild("FileItem");
  while(fi.ToElement()!=0)
  {
    const char* pszName = fi.ToElement()->Attribute("name");
    const char* pszDesc = fi.ToElement()->Attribute("description");

    if(pszName!=NULL && pszDesc!=NULL)
    {
	    CString sFileName = pszName;
      CString sFilePathName = sTempDir + _T("\\") + CString(pszName);      
      int index = -1;
      ZIPENTRY ze;	  
      ZRESULT zr = FindZipItem(hz, sFileName, false, &index, &ze);
      zr = UnzipItem(hz, index, sFilePathName);
	    LPSTR pszFilePathName = T2A(sFilePathName.GetBuffer(0));
      file_list[pszFilePathName]=pszDesc;
    }

    fi = fi.ToElement()->NextSibling("FileItem");
  }

  CloseZip(hz);

  return 0;
}

int 
GetCrashInfoThroughPipe(
  CString& sAppName,
  CString& sAppVersion,
  CString& sImageName,
  CString& sSubject,
  CString& sMailTo,
  CString& sUrl,
  UINT (*puPriorities)[3],
  CString& sZipName,
  std::map<std::string, std::string> &file_list)
{
  // Create named pipe to get crash information from client process.
  
  DWORD dwProcessId = GetCurrentProcessId();
  CString szPipeName;
  szPipeName.Format(_T("\\\\.\\pipe\\CrashRpt_%lu"), dwProcessId);
  
  HANDLE hPipe = CreateNamedPipe(
    szPipeName, PIPE_ACCESS_INBOUND|FILE_FLAG_OVERLAPPED, 
    0, 1, 0, 1024, 0, NULL);
  
  if(hPipe==INVALID_HANDLE_VALUE)
  {
    ATLASSERT(hPipe!=INVALID_HANDLE_VALUE);
    return 1; // Couldn't create pipe
  }

  // Connect pipe
  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(OVERLAPPED));
  overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  BOOL bConnected = ConnectNamedPipe(hPipe, &overlapped);
  DWORD dwLastError = GetLastError();
  if(!bConnected && 
     dwLastError!=ERROR_IO_PENDING &&
     dwLastError!=ERROR_PIPE_CONNECTED)
  {
    ATLASSERT(bConnected);
    CloseHandle(hPipe);
    return 3; // Couldn't connect 
  }

  DWORD dwWaitResult = WaitForSingleObject(overlapped.hEvent, 60*1000);
  if(dwWaitResult!=WAIT_OBJECT_0)
  {    
    ATLASSERT(dwWaitResult==WAIT_OBJECT_0);
    CloseHandle(hPipe);
    return 4; // Time out interval exceeded
  }

  // Check connection result
  DWORD dwBytesTransferred = 0;
  BOOL bConnectionResult = GetOverlappedResult(hPipe, &overlapped, &dwBytesTransferred, TRUE); 
  if(!bConnectionResult)
  {
    ATLASSERT(bConnectionResult!=0);
    return 5; // Connection failed
  }

  // Read incoming data
  std::string sDataA;
  for(;;)
  {
    DWORD dwBytesRead = 0;
    BYTE buffer[1024];
    BOOL bRead = ReadFile(hPipe, buffer, 1024, &dwBytesRead, NULL);
    if(!bRead)
      break;
    sDataA += std::string((char*)buffer, dwBytesRead);
  }

  // Parse text  
  int nParseResult = ParseCrashInfo(sDataA.c_str(), sAppName, sAppVersion, sImageName, 
    sSubject, sMailTo, sUrl, puPriorities, sZipName);
  if(nParseResult!=0)
  {
    ATLASSERT(nParseResult==0);
    return 6;
  }

  // Disconnect
  BOOL bDisconnected = DisconnectNamedPipe(hPipe);
  ATLASSERT(bDisconnected);
  bDisconnected;

  CloseHandle(hPipe);
  CloseHandle(overlapped.hEvent); 

  if(0!=GetFileList(sZipName, file_list))
    return 7;

  // Success
  return 0;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

  CMainDlg dlgMain;

  if(GetCrashInfoThroughPipe(
    dlgMain.m_sAppName,
    dlgMain.m_sAppVersion,
    dlgMain.m_sImageName,
    dlgMain.m_sEmailSubject, 
    dlgMain.m_sEmailTo, 
    dlgMain.m_sUrl,
    &dlgMain.m_uPriorities,
    dlgMain.m_sZipName,    
    dlgMain.m_pUDFiles)!=0)
    return 1; 
  
	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

  int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
