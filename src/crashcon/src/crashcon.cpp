// crashcon.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <assert.h>
#include <process.h>
#include "CrashRpt.h"

LPVOID lpvState = NULL;

int main(int argc, char* argv[])
{
  argc; // this is to avoid C4100 unreferenced formal parameter warning
  argv; // this is to avoid C4100 unreferenced formal parameter warning
  
  // Install crash reporting

#ifdef TEST_DEPRECATED_FUNCS
  lpvState = Install(
    NULL, 
    _T("test@hotmail.com"), 
    NULL);
#else
  CR_INSTALL_INFO info;
  memset(&info, 0, sizeof(CR_INSTALL_INFO));
  info.cb = sizeof(CR_INSTALL_INFO);
  info.pszAppName = _T("CrashRpt Console Test");
  info.pszAppVersion = _T("1.0.0");
  info.pszEmailSubject = _T("CrashRpt Console Test 1.0.0 Error Report");
  info.pszEmailTo = _T("test@hotmail.com");

  int nInstResult = crInstall(&info);
  assert(nInstResult==0);
  
  if(nInstResult!=0)
  {
    TCHAR buff[256];
    crGetLastErrorMsg(buff, 256);
    _tprintf(_T("%s\n"), buff);
    return FALSE;
  }

#endif //TEST_DEPRECATED_FUNCS
  
  printf("Press Enter to simulate a null pointer exception or any other key to exit...\n");
  int n = _getch();
  if(n==13)
  {

#ifdef _DEBUG
     __try
     {
        RaiseException(123, EXCEPTION_NONCONTINUABLE, 0, NULL);
     } 
     __except(crExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
     {
     }
#else
     int *p = 0;
     *p = 0;
#endif // _DEBUG
  
  }

#ifdef TEST_DEPRECATED_FUNCS
  Uninstall(lpvState);
#else
  int nUninstRes = crUninstall();
  assert(nUninstRes==0);
  nUninstRes;
#endif //TEST_DEPRECATED_FUNCS

  return 0;
}

