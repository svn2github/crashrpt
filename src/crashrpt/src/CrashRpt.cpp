#include "stdafx.h"
#include "CrashRpt.h"
#include "CrashHandler.h"

WTL::CAppModule _Module;

CComAutoCriticalSection g_cs; // Critical section for thread-safe accessing error messages
std::map<DWORD, CString> g_sErrorMsg; // Last error messages for each calling thread.

CRASHRPTAPI LPVOID InstallW(LPGETLOGFILE pfnCallback, LPCWSTR pszEmailTo, LPCWSTR pszEmailSubject)
{
  CR_INSTALL_INFOW info;
  memset(&info, 0, sizeof(CR_INSTALL_INFO));
  info.cb = sizeof(CR_INSTALL_INFO);
  info.pfnCrashCallback = pfnCallback;
  info.pszEmailTo = pszEmailTo;
  info.pszEmailSubject = pszEmailSubject;

  crInstallW(&info);

  return NULL;
}

CRASHRPTAPI LPVOID InstallA(LPGETLOGFILE pfnCallback, LPCSTR pszEmailTo, LPCSTR pszEmailSubject)
{
  USES_CONVERSION;
  LPWSTR lpwszEmailTo = A2W(pszEmailTo);
  LPWSTR lpwszEmailSubject = A2W(pszEmailSubject);

  return InstallW(pfnCallback, lpwszEmailTo, lpwszEmailSubject);
}

CRASHRPTAPI void Uninstall(LPVOID lpState)
{
  lpState;
  crUninstall();  
}

CRASHRPTAPI void AddFileW(LPVOID lpState, LPCWSTR lpFile, LPCWSTR lpDesc)
{ 
  lpState;
  crAddFileW(lpFile, lpDesc);
}

CRASHRPTAPI void AddFileA(LPVOID lpState, LPCSTR lpFile, LPCSTR lpDesc)
{
  USES_CONVERSION;
  LPWSTR lpwszFile = A2W(lpFile);
  LPWSTR lpwszDesc = A2W(lpDesc);
  AddFileW(lpState, lpwszFile, lpwszDesc);
}

CRASHRPTAPI void GenerateErrorReport(LPVOID lpState, PEXCEPTION_POINTERS pExInfo)
{
  lpState;

  CR_EXCEPTION_INFO ei;
  memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
  ei.cb = sizeof(CR_EXCEPTION_INFO);
  ei.exctype = CR_WIN32_STRUCTURED_EXCEPTION;
  ei.pexcptrs = pExInfo;
  
  crGenerateErrorReport(&ei);
}

CRASHRPTAPI int crInstallW(CR_INSTALL_INFOW* pInfo)
{
  crSetErrorMsg(_T("Success."));

  USES_CONVERSION;

  // Validate input parameters.
  if(pInfo==NULL || 
     pInfo->cb!=sizeof(CR_INSTALL_INFOW))
  {
    ATLASSERT(pInfo->cb==sizeof(CR_INSTALL_INFO));
    ATLASSERT(pInfo != NULL);        
    crSetErrorMsg(_T("pInfo is NULL or pInfo->cb member is not valid."));
    return 1; 
  }

  // Check if crInstall() already was called for current process.
  CCrashHandler *pCrashHandler = 
    CCrashHandler::GetCurrentProcessCrashHandler();

  if(pCrashHandler!=NULL)
  {
    ATLASSERT(pCrashHandler==NULL);
    crSetErrorMsg(_T("Can't install crash handlers to the same process twice."));
    return 2; 
  }
  
  pCrashHandler = new CCrashHandler();
  if(pCrashHandler==NULL)
  {
    ATLASSERT(pCrashHandler!=NULL);
    crSetErrorMsg(_T("Error allocating memory for crash handler."));
    return 3; 
  }

  LPTSTR ptszAppName = W2T((LPWSTR)pInfo->pszAppName);
  LPTSTR ptszAppVersion = W2T((LPWSTR)pInfo->pszAppVersion);
  LPTSTR ptszCrashSenderPath = W2T((LPWSTR)pInfo->pszCrashSenderPath);
  LPTSTR ptszEmailTo = W2T((LPWSTR)pInfo->pszEmailTo);
  LPTSTR ptszEmailSubject = W2T((LPWSTR)pInfo->pszEmailSubject);
  LPTSTR ptszUrl = W2T((LPWSTR)pInfo->pszUrl);

  int nInitResult = pCrashHandler->Init(
    ptszAppName, 
    ptszAppVersion, 
    ptszCrashSenderPath,
    pInfo->pfnCrashCallback,
    ptszEmailTo,
    ptszEmailSubject,
    ptszUrl,
    &pInfo->uPriorities);
  
  if(nInitResult!=0)
  {
    ATLASSERT(nInitResult==0);        
    return 4;
  }

  // OK.  
  return 0;
}

CRASHRPTAPI int crInstallA(CR_INSTALL_INFOA* pInfo)
{
  if(pInfo==NULL)
    return crInstallW((CR_INSTALL_INFOW*)NULL);

  // Convert pInfo members to wide char

  USES_CONVERSION;
  LPWSTR lpwszAppName = NULL;
  LPWSTR lpwszAppVersion = NULL;
  LPWSTR lpwszCrashSenderPath = NULL;
  LPWSTR lpwszEmailSubject = NULL;
  LPWSTR lpwszEmailTo = NULL;
  LPWSTR lpwszUrl = NULL;

  CR_INSTALL_INFOW ii;
  memset(&ii, 0, sizeof(CR_INSTALL_INFOW));
  ii.cb = sizeof(CR_INSTALL_INFOW);
  ii.pfnCrashCallback = pInfo->pfnCrashCallback;

  if(pInfo->pszAppName!=NULL)
  {
    lpwszAppName = A2W(pInfo->pszAppName);
    ii.pszAppName = lpwszAppName;
  }

  if(pInfo->pszAppVersion!=NULL)
  {
    lpwszAppVersion = A2W(pInfo->pszAppVersion);
    ii.pszAppVersion = lpwszAppVersion;
  }

  if(pInfo->pszCrashSenderPath!=NULL)
  {
    lpwszCrashSenderPath = A2W(pInfo->pszCrashSenderPath);
    ii.pszCrashSenderPath = lpwszCrashSenderPath;
  }

  if(pInfo->pszEmailSubject!=NULL)
  {
    lpwszEmailSubject = A2W(pInfo->pszEmailSubject);
    ii.pszEmailSubject = lpwszEmailSubject;
  }

  if(pInfo->pszEmailTo!=NULL)
  {
    lpwszEmailTo = A2W(pInfo->pszEmailTo);
    ii.pszEmailTo = lpwszEmailTo;
  }

  if(pInfo->pszUrl!=NULL)
  {
    lpwszUrl = A2W(pInfo->pszUrl);
    ii.pszUrl = lpwszUrl;
  }

  memcpy(&ii.uPriorities, pInfo->uPriorities, 3*sizeof(UINT));

  return crInstallW(&ii);
}

CRASHRPTAPI int crUninstall()
{
  crSetErrorMsg(_T("Success."));

  CCrashHandler *pCrashHandler = 
    CCrashHandler::GetCurrentProcessCrashHandler();
  
  if(pCrashHandler==NULL)
  {    
    ATLASSERT(pCrashHandler!=NULL);
    crSetErrorMsg(_T("Crash handler wasn't preiviously installed for this process."));
    return 1; 
  }

  // Uninstall main thread's C++ exception handlers
  int nUnset = pCrashHandler->UnSetThreadCPPExceptionHandlers();
  if(nUnset!=0)
    return 2;

  int nDestroy = pCrashHandler->Destroy();
  if(nDestroy!=0)
    return 3;

  delete pCrashHandler;
    
  return 0;
}

// Sets C++ exception handlers for the calling thread
CRASHRPTAPI int crInstallToCurrentThread()
{
  crSetErrorMsg(_T("Success."));

  CCrashHandler *pCrashHandler = 
    CCrashHandler::GetCurrentProcessCrashHandler();
  
  if(pCrashHandler==NULL)
  {
    ATLASSERT(pCrashHandler!=NULL);
    crSetErrorMsg(_T("Crash handler was already installed for current thread."));
    return 1; 
  }

  int nResult = pCrashHandler->SetThreadCPPExceptionHandlers();
  if(nResult!=0)
    return 2; // Error?

  // Ok.
  return 0;
}

// Unsets C++ exception handlers from the calling thread
CRASHRPTAPI int crUninstallFromCurrentThread()
{
  crSetErrorMsg(_T("Success."));

  CCrashHandler *pCrashHandler = 
    CCrashHandler::GetCurrentProcessCrashHandler();

  if(pCrashHandler==NULL)
  {
    ATLASSERT(pCrashHandler!=NULL);
    crSetErrorMsg(_T("Crash handler wasn't previously installed for current thread."));
    return 1; // Invalid parameter?
  }

  int nResult = pCrashHandler->UnSetThreadCPPExceptionHandlers();
  if(nResult!=0)
    return 2; // Error?

  // OK.
  return 0;
}

CRASHRPTAPI int crAddFileW(PCWSTR pszFile, PCWSTR pszDesc)
{
  crSetErrorMsg(_T("Success."));

  USES_CONVERSION;

  CCrashHandler *pCrashHandler = 
    CCrashHandler::GetCurrentProcessCrashHandler();

  if(pCrashHandler==NULL)
  {
    ATLASSERT(pCrashHandler!=NULL);
    crSetErrorMsg(_T("Crash handler wasn't previously installed for current process."));
    return 1; // No handler installed for current process?
  }
  
  LPTSTR lptszFile = W2T((LPWSTR)pszFile);
  LPTSTR lptszDesc = W2T((LPWSTR)pszDesc);

  int nAddResult = pCrashHandler->AddFile(lptszFile, lptszDesc);
  if(nAddResult!=0)
  {
    ATLASSERT(nAddResult==0);
    return 2; // Couldn't add file?
  }

  // OK.
  return 0;
}

CRASHRPTAPI int crAddFileA(PCSTR pszFile, PCSTR pszDesc)
{
  // Convert parameters to wide char

  USES_CONVERSION;

  LPWSTR pwszFile = NULL;
  LPWSTR pwszDesc = NULL;
  
  if(pszFile)
  {
    pwszFile = A2W(pszFile);
  }

  if(pszDesc)
  {
    pwszDesc = A2W(pszDesc);    
  }

  return crAddFileW(pwszFile, pwszDesc);
}

CRASHRPTAPI int crGenerateErrorReport(
  CR_EXCEPTION_INFO* pExceptionInfo)
{
  crSetErrorMsg(_T("Unspecified error."));

  if(pExceptionInfo==NULL || 
     pExceptionInfo->cb!=sizeof(CR_EXCEPTION_INFO))
  {
    crSetErrorMsg(_T("Exception info is NULL or invalid."));
    ATLASSERT(pExceptionInfo!=NULL);
    ATLASSERT(pExceptionInfo->cb==sizeof(CR_EXCEPTION_INFO));
    return 1;
  }

  CCrashHandler *pCrashHandler = 
    CCrashHandler::GetCurrentProcessCrashHandler();

  if(pCrashHandler==NULL)
  {    
    // Handler is not installed for current process 
    crSetErrorMsg(_T("Crash handler wasn't previously installed for current process."));
    ATLASSERT(pCrashHandler!=NULL);
    return 2;
  } 

  return pCrashHandler->GenerateErrorReport(pExceptionInfo);  
}

CRASHRPTAPI int crGetLastErrorMsgW(LPWSTR pszBuffer, UINT uBuffSize)
{
  if(pszBuffer==NULL || uBuffSize==0)
    return -1; // Null pointer to buffer

  USES_CONVERSION;

  g_cs.Lock();

  DWORD dwThreadId = GetCurrentThreadId();
  std::map<DWORD, CString>::iterator it = g_sErrorMsg.find(dwThreadId);

  if(it==g_sErrorMsg.end())
  {
    // No error message for current thread.
    CString sErrorMsg = _T("No error.");
	  LPWSTR pwszErrorMsg = T2W(sErrorMsg.GetBuffer(0));
	  WCSNCPY_S(pszBuffer, uBuffSize, pwszErrorMsg, sErrorMsg.GetLength());
    int size =  sErrorMsg.GetLength();
    g_cs.Unlock();
    return size;
  }
  
  LPWSTR pwszErrorMsg = T2W(it->second.GetBuffer(0));
  WCSNCPY_S(pszBuffer, uBuffSize, pwszErrorMsg, uBuffSize-1);
  int size = it->second.GetLength();
  g_cs.Unlock();
  return size;
}

CRASHRPTAPI int crGetLastErrorMsgA(LPSTR pszBuffer, UINT uBuffSize)
{  
  if(pszBuffer==NULL)
    return -1;

  USES_CONVERSION;

  WCHAR* pwszBuffer = new WCHAR[uBuffSize];
    
  int res = crGetLastErrorMsgW(pwszBuffer, uBuffSize);
  
  LPSTR paszBuffer = W2A(pwszBuffer);  

  STRCPY_S(pszBuffer, uBuffSize, paszBuffer);

  delete [] pwszBuffer;

  return res;
}

int crSetErrorMsg(PTSTR pszErrorMsg)
{  
  g_cs.Lock();
  DWORD dwThreadId = GetCurrentThreadId();
  g_sErrorMsg[dwThreadId] = pszErrorMsg;
  g_cs.Unlock();
  return 0;
}


CRASHRPTAPI int crExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
  crSetErrorMsg(_T("Unspecified error."));

  CCrashHandler *pCrashHandler = 
    CCrashHandler::GetCurrentProcessCrashHandler();

  if(pCrashHandler==NULL)
  {    
    crSetErrorMsg(_T("Crash handler wasn't previously installed for current process."));
    return EXCEPTION_CONTINUE_SEARCH; 
  }

  CR_EXCEPTION_INFO ei;
  memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
  ei.cb = sizeof(CR_EXCEPTION_INFO);  
  ei.exctype = CR_WIN32_STRUCTURED_EXCEPTION;
  ei.pexcptrs = ep;
  ei.code = code;

  int res = pCrashHandler->GenerateErrorReport(&ei);
  if(res!=0)
  {
    // If goes here than GenerateErrorReport() failed  
    return EXCEPTION_CONTINUE_SEARCH;  
  }  
  
  crSetErrorMsg(_T("Success."));
  return EXCEPTION_EXECUTE_HANDLER;  
}

//-----------------------------------------------------------------------------------------------
// Below crEmulateCrash() related stuff goes 


class CDerived;
class CBase
{
public:
   CBase(CDerived *derived): m_pDerived(derived) {};
   ~CBase();
   virtual void function(void) = 0;

   CDerived * m_pDerived;
};

class CDerived : public CBase
{
public:
   CDerived() : CBase(this) {};   // C4355
   virtual void function(void) {};
};

CBase::~CBase()
{
   m_pDerived -> function();
}

#include <float.h>
void sigfpe_test()
{ 
  // Code taken from http://www.devx.com/cplus/Article/34993/1954

  //Set the x86 floating-point control word according to what
  //exceptions you want to trap. 
  _clearfp(); //Always call _clearfp before setting the control
              //word
  //Because the second parameter in the following call is 0, it
  //only returns the floating-point control word
  unsigned int cw; 
#if _MSC_VER<1400
  cw = _controlfp(0, 0); //Get the default control
#else
  _controlfp_s(&cw, 0, 0); //Get the default control
#endif 
                                      //word
  //Set the exception masks off for exceptions that you want to
  //trap.  When a mask bit is set, the corresponding floating-point
  //exception is //blocked from being generating.
  cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_ZERODIVIDE|
         EM_DENORMAL|EM_INVALID);
  //For any bit in the second parameter (mask) that is 1, the 
  //corresponding bit in the first parameter is used to update
  //the control word.  
  unsigned int cwOriginal;
#if _MSC_VER<1400
  cwOriginal = _controlfp(cw, MCW_EM); //Set it.
#else
  _controlfp_s(&cwOriginal, cw, MCW_EM); //Set it.
#endif
                              //MCW_EM is defined in float.h.
                              //Restore the original value when done:
                              //_controlfp(cwOriginal, MCW_EM);

  // Divide by zero

  float a = 1;
  float b = 0;
  float c = a/b;
  c; 
}

#define BIG_NUMBER 0x1fffffff
#pragma warning(disable: 4717) // avoid C4717 warning
int RecurseAlloc() 
{
   int *pi = new int[BIG_NUMBER];
   pi;
   RecurseAlloc();
   return 0;
}

CRASHRPTAPI int crEmulateCrash(unsigned ExceptionType)
{
  crSetErrorMsg(_T("Unspecified error."));

  switch(ExceptionType)
  {
  case CR_WIN32_STRUCTURED_EXCEPTION:
    {
      // Access violation
      int *p = 0;
      *p = 0;
    }
    break;
  case CR_CPP_TERMINATE_CALL:
    {
      // Call terminate
      terminate();
    }
    break;
  case CR_CPP_UNEXPECTED_CALL:
    {
      // Call unexpected
      unexpected();
    }
    break;
#if _MSC_VER>=1300
  case CR_CPP_PURE_CALL:
    {
      // pure virtual method call
      CDerived derived;
    }
    break;
#endif
#if _MSC_VER>=1300 && _MSC_VER<1400
  case CR_CPP_SECURITY_ERROR:
    {
      // Cause buffer overrun (/GS compiler option)

      char large_buffer[] = "This string is longer than 10 characters!!!";
      // vulnerable code
      char buffer[10];
#pragma warning(disable:4996) // avoid C4996 warning
      strcpy(buffer, large_buffer); // overrun buffer !!!      
    }
    break;
#endif //_MSC_VER>=1300 && _MSC_VER<1400
#if _MSC_VER>=1400
  case CR_CPP_INVALID_PARAMETER:
    {      
      char* formatString;
      // Call printf_s with invalid parameters.
      formatString = NULL;
      printf(formatString);
    }
    break;
#endif
#if _MSC_VER>=1300
  case CR_CPP_NEW_OPERATOR_ERROR:
    {
      // Cause memory allocation error
      RecurseAlloc();
    }
    break;
#endif //_MSC_VER>=1300
  case CR_CPP_SIGABRT: 
    {
      // Call abort
      abort();
    }
    break;
  case CR_CPP_SIGFPE:
    {
      // floating point exception ( /fp:except compiler option)
      sigfpe_test();
      return 1;
    }    
  case CR_CPP_SIGILL: 
    {
      int result = raise(SIGILL);  
      ATLASSERT(result==0);
      crSetErrorMsg(_T("Error raising SIGILL."));
      return result;
    }    
  case CR_CPP_SIGINT: 
    {
      int result = raise(SIGINT);  
      ATLASSERT(result==0);
      crSetErrorMsg(_T("Error raising SIGINT."));
      return result;
    }    
  case CR_CPP_SIGSEGV: 
    {
      int result = raise(SIGSEGV);  
      ATLASSERT(result==0);
      crSetErrorMsg(_T("Error raising SIGSEGV."));
      return result;
    }    
  case CR_CPP_SIGTERM: 
    {
     int result = raise(SIGTERM);  
     crSetErrorMsg(_T("Error raising SIGTERM."));
	   ATLASSERT(result==0);     
     return result;
    }
  case CR_NONCONTINUABLE_EXCEPTION: 
    {
      // Raise noncontinuable software exception
      RaiseException(123, EXCEPTION_NONCONTINUABLE, 0, NULL);        
    }
    break;
  default:
    {
      crSetErrorMsg(_T("Unknown exception type specified."));    
      ATLASSERT(0); // unknown type?
    }
    break;
  }
 
  return 1;
}


// --------------------------------------------------------------
// DllMain()

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD /*fdwReason*/, LPVOID /*lpvReserved*/)
{
  HRESULT hRes = _Module.Init(NULL, hinstDLL);
  ATLASSERT(SUCCEEDED(hRes));
  hRes;

  return TRUE;
}

