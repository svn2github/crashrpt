// CrashRptTest.cpp : main source file for CrashRptTest.exe
//

#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"
#include "CrashThread.h"


CAppModule _Module;

LPVOID g_pCrashRptState = NULL;
HANDLE g_hWorkingThread = NULL;
CrashThreadInfo g_CrashThreadInfo;

// Helper function that returns path to application directory
CString GetAppDir()
{
	CString string;
	LPTSTR buf = string.GetBuffer(_MAX_PATH);
	GetModuleFileName(NULL, buf, _MAX_PATH);
	*(_tcsrchr(buf,'\\'))=0; // remove executable name
	string.ReleaseBuffer();
	return string;
}

BOOL WINAPI CrashCallback(LPVOID lpvState)
{
  CString sLogFile = GetAppDir() + _T("\\dummy.log");
  CString sIniFile = GetAppDir() + _T("\\dummy.ini");

#ifdef TEST_DEPRECATED_FUNCS
  AddFile(lpvState, sLogFile, _T("Dummy Log File"));
  AddFile(lpvState, sLogFile, _T("Dummy INI File"));
#else
  lpvState;
  
  int nAddFile = crAddFile(sLogFile, _T("Dummy Log File"));
  ATLASSERT(nAddFile==0);

  nAddFile = crAddFile(sIniFile, _T("Dummy INI File"));
  ATLASSERT(nAddFile==0);
#endif

  return TRUE;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

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

  // Install crash reporting
#ifdef TEST_DEPRECATED_FUNCS

  g_pCrashRptState = Install(
    CrashCallback, 
    _T("test@hotmail.com"), 
    _T("Crash"));

#else

  CR_INSTALL_INFO info;
  memset(&info, 0, sizeof(CR_INSTALL_INFO));
  info.cb = sizeof(CR_INSTALL_INFO);  
  info.pszAppName = _T("CrashRpt Tests");
  info.pszAppVersion = _T("1.0.1");
  info.pszEmailSubject = _T("Error from CrashRptTests v1.0.1");
  info.pszEmailTo = _T("test@hotmail.com");
  info.pszUrl = _T("http://test.com/test.php");
  info.pfnCrashCallback = CrashCallback;    
  info.uPriorities[CR_HTTP] = 3;
  info.uPriorities[CR_SMTP] = 2;
  info.uPriorities[CR_SMAPI] = 1;  
    
  int nInstResult = crInstall(&info);
  ATLASSERT(nInstResult==0);
  nInstResult;

#endif //TEST_DEPRECATED_FUNCS

  /* Create another thread */
  g_CrashThreadInfo.m_pCrashRptState = g_pCrashRptState;
  g_CrashThreadInfo.m_bStop = false;
  g_CrashThreadInfo.m_hWakeUpEvent = CreateEvent(NULL, FALSE, FALSE, _T("WakeUpEvent"));
  ATLASSERT(g_CrashThreadInfo.m_hWakeUpEvent!=NULL);

  DWORD dwThreadId = 0;
  g_hWorkingThread = CreateThread(NULL, 0, CrashThread, (LPVOID)&g_CrashThreadInfo, 0, &dwThreadId);
  ATLASSERT(g_hWorkingThread!=NULL);

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();

  // Close another thread
  g_CrashThreadInfo.m_bStop = true;
  SetEvent(g_CrashThreadInfo.m_hWakeUpEvent);
  // Wait until thread terminates
  WaitForSingleObject(g_hWorkingThread, INFINITE);

  // Uninstall crash reporting
  
#ifdef TEST_DEPRECATED_FUNCS

  Uninstall(g_pCrashRptState);

#else
  
  int nUninstResult = crUninstall();
  ATLASSERT(nUninstResult==0);
  nUninstResult;

#endif //TEST_DEPRECATED_FUNCS

	::CoUninitialize();

	return nRet;
}
