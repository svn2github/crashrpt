#include "stdafx.h"
#include "CrashThread.h"
#include <exception>
#include <signal.h>
#include <assert.h>

// Tests crExceptionFilter
void test_seh()
{
  __try
  {
    int nResult = crEmulateCrash(CR_NONCONTINUABLE_EXCEPTION);
    if(nResult!=0)
    {      
      MessageBox(NULL, _T("Error raising noncontinuable exception!"), _T("Error"), 0);    
    }
  }
  __except(crExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
  {    
    // Terminate program
    ExitProcess(1);
  }
}

// Tests crGenerateErrorReport
void test_generate_report()
{
  CR_EXCEPTION_INFO ei;
  memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
  ei.cb = sizeof(CR_EXCEPTION_INFO);
  ei.exctype = CR_WIN32_STRUCTURED_EXCEPTION;
  ei.code = 0x1234;
  ei.pexcptrs = NULL;

  int nResult = crGenerateErrorReport(&ei);
  if(nResult!=0)
  {
    TCHAR szErrorMsg[256];
    CString sError = _T("Error generating error report!\nErrorMsg:");
    crGetLastErrorMsg(szErrorMsg, 256);
    sError+=szErrorMsg;
    MessageBox(NULL, sError, 0, 0);
  }
}

DWORD WINAPI CrashThread(LPVOID pParam)
{
  CrashThreadInfo* pInfo = (CrashThreadInfo*)pParam;

  // Install per-thread exception handlers
  CrThreadAutoInstallHelper cr_install_helper(0);

  for(;;)
  {
    // Wait until wake up event is signaled
    WaitForSingleObject(pInfo->m_hWakeUpEvent, INFINITE);   

    if(pInfo->m_bStop)
      break; // Exit the loop

    if(pInfo->m_ExceptionType==128)
    {
      // Test generate report manually
      test_generate_report();
    }
    else if(pInfo->m_ExceptionType==CR_NONCONTINUABLE_EXCEPTION)
    {
      // Test crExceptionFilter
      test_seh();
    }
    else if(crEmulateCrash(pInfo->m_ExceptionType)!=0)
    {
      // Test exception handler
      TCHAR szErrorMsg[256];
      CString sError = _T("Error creating exception situation!\nErrorMsg:");
      crGetLastErrorMsg(szErrorMsg, 256);
      sError+=szErrorMsg;
      MessageBox(NULL, sError, _T("Error"), 0);    
    }
  }

  // Exit this thread
  return 0;
}




