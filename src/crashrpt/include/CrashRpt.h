/*! \file   CrashRpt.h
 *  \brief  Defines the interface for the CrashRpt.DLL.
 *  \date   2003-2009
 *  \author Michael Carruth (mcarruth@gmail.com)
 *  \author zeXspectrum (zexspectrum@gmail.com)
 */

#ifndef _CRASHRPT_H_
#define _CRASHRPT_H_

#include <windows.h>

// This is needed for exporting/importing functions from/to CrashRpt.dll
#ifdef CRASHRPT_EXPORTS
 #define CRASHRPTAPI extern "C" __declspec(dllexport) 
#else 
 #define CRASHRPTAPI extern "C" __declspec(dllimport) 
#endif

//! Current CrashRpt version
#define CRASHRPT_VER 1102

/*! \defgroup CrashRptAPI CrashRpt Functions */
/*! \defgroup DeprecatedAPI Obsolete Functions */
/*! \defgroup CrashRptStructs CrashRpt Structures */
/*! \defgroup CrashRptWrappers CrashRpt Wrapper Classes */

/*! \ingroup CrashRptAPI
 *  \brief Client crash callback function prototype
 *  \param[in] lpvState Not used, always equals to NULL.
 *
 *  \remarks
 *  The crash callback function is called when crash occurs. This way client application is
 *  notified about the crash.
 *  In crash callback, you can use crAddFile() to add a custom file to the error report.
 *  
 *  The crash callback function should return TRUE to allow generate error report. It should 
 *  return FALSE to prevent crash report generation.
 *
 *  The following example shows how to use crash callback function.
 *
 *  \code
 *  // define the crash callback
 *  BOOL CALLBACK CrashCallback(LPVOID lpvState)
 *  {
 *    // add custom log file to crash report
 *    crAddFile(
 *       _T("C:\\Documents and Settings\\Application Data\\UserName\\MyApp\\Logs\\MyLog.txt"), 
 *       _T("My custom log file"));
 *
 *    return TRUE;
 *  }
 *
 *  \endcode
 */
typedef BOOL (CALLBACK *LPGETLOGFILE) (LPVOID lpvState);

#ifndef _CRASHRPT_REMOVE_DEPRECATED

/*! \ingroup DeprecatedAPI
 *  \brief Installs exception handlers for the current process.
 *
 *  \param[in] pfnCallback Client crash callback.
 *  \param[in] pszEmailTo Email address to send crash report to.
 *  \param[in] pszEmailSubject Subject of the E-mail message.
 *
 *  \return Always returns NULL.
 *
 *  \deprecated
 *    This function is deprecated. It is still supported for compatiblity with
 *    older versions of CrashRpt, however consider using crInstall() function instead.
 *    This function is currently implemented as a wrapper for crInstall().    
 *
 *  \remarks
 *
 *    This function installs unhandled exception filter for all threads of calling process.
 *    It also installs various C++ exception/error handlers. For the list of handlers,
 *    please see crInstall().
 *
 *    \c pfnCallback defines the callback function that is called on crash. The callback can be
 *    used to add a custom file to crash report using AddFile() function. This parameter can be NULL.
 *
 *    \c pszEmailTo should be the valid email address of recipient. 
 *
 *    \c pszEmailSubject is the email subject. If this parameter is NULL, the default subject is 
 *    generated.
 *
 *    This function assumes that CrashSender.exe is located in the same directory as CrashRpt.dll loaded
 *    by the caller process. To specify different directory, use crInstall().
 *
 *    On crash, the error report is sent as E-mail message using address and subject passed to the
 *    function as \c pszEmailTo and \c pszEmailSubject parameters, respectively.
 *    
 *    InstallW() and InstallA() are wide-character and multibyte-character versions of Install(). 
 *    The Install() macro defines character set independent mapping for these functions. 
 * 
 */

CRASHRPTAPI 
LPVOID 
InstallW(
   LPGETLOGFILE pfnCallback,
   LPCWSTR pszEmailTo,    
   LPCWSTR pszEmailSubject
   );

/*! \ingroup DeprecatedAPI
 *  \copydoc InstallW()
 */

CRASHRPTAPI 
LPVOID 
InstallA(
   LPGETLOGFILE pfnCallback,
   LPCSTR pszEmailTo,    
   LPCSTR pszEmailSubject
   );

/*! \brief Character set-independent mapping of InstallW() and InstallA() functions. 
 *  \ingroup DeprecatedAPI
 */
#ifdef UNICODE
#define Install InstallW 
#else
#define Install InstallA
#endif //UNICODE


/*! \ingroup DeprecatedAPI
 *  \brief Uninstalls the exception filters set up by Install().
 *
 *  \param[in] lpState State information returned from Install(), ignored and should be NULL.
 *
 *  \deprecated
 *    This function is deprecated. It is still supported for compatiblity with
 *    older versions of CrashRpt, however consider using crUninstall() function instead.
 *    This function is implemented as a wrapper for crUninstall().
  *
 *  \remarks
 *
 *    Call this function on application exit to uninstall all previously installed exception
 *    handlers.
 *
 *    The \c lpState parameter is unused and can be NULL.
 */

CRASHRPTAPI 
void 
Uninstall(
   IN LPVOID lpState                            
   );

/*! \ingroup DeprecatedAPI 
 *  \brief Adds a file to the crash report.
 *  
 *  \param[in] lpState State information returned from Install(), ignored and should be NULL.
 *  \param[in] pszFile  Fully qualified file name.
 *  \param[in] pszDesc  Description of file, used by Error Report Details dialog.
 *
 *  \deprecated
 *    This function is deprecated. It is still supported for compatiblity with
 *    older versions of CrashRpt, however consider using crAddFile() function instead.
 *    This function is implemented as a wrapper for crAddFile().
 *
 *  \remarks
 *
 *    This function can be called anytime after Install() to add one or more
 *    files to the generated crash report. However, the recommended way is to 
 *    call this function in crash callback.
 *
 *    \c pszFile should be a valid absolute path of a file to add to crash report. It
 *    is recommended to add small files (several KB in size). If a large file is added,
 *    the crash report sending procedure may fail.
 *
 *    \c pszDesc is a description of a file. It can be NULL.
 *
 *    Function fails if \c pszFile doesn't exist at the moment of function call.
 *
 *    AddFileW() and AddFileA() are wide-character and multibyte-character versions of AddFile(). 
 *    The AddFile() macro defines character set independent mapping for these functions. 
 *
 */

CRASHRPTAPI 
void 
AddFileW(
   IN LPVOID lpState,                         
   IN LPCWSTR pszFile,                         
   IN LPCWSTR pszDesc                          
   );

/*! \ingroup DeprecatedAPI
 *
 *  \copydoc AddFileW()
 */

CRASHRPTAPI 
void 
AddFileA(
   IN LPVOID lpState,                         
   IN LPCSTR pszFile,                         
   IN LPCSTR pszDesc                          
   );

/*! \brief Character set-independent mapping of AddFileW() and AddFileA() functions. 
 *  \ingroup DeprecatedAPI
 */
#ifdef UNICODE
#define AddFile AddFileW
#else
#define AddFile AddFileA
#endif //UNICODE



/*! \ingroup DeprecatedAPI 
 *  \brief Generates the crash report.
 *  
 *  \param[in] lpState     State information returned from Install(), ignored and should be NULL.
 *  \param[in] pExInfo     Pointer to an EXCEPTION_POINTERS structure, can be NULL.
 * 
 *  \deprecated
 *    This function is deprecated. It is still supported for compatiblity with
 *    older versions of CrashRpt, however consider using crGenerateErrorReport() function instead.
 *    This function is implemented as a wrapper for crGenerateErrorReport().
 *
 *  \remarks
 *
 *    Call this function to manually generate a crash report.
 *
 *    The crash report contains the crash minidump, crash log in XML format and
 *    additional optional files added with AddFile().
 *
 *    \c pExInfo defines the exception pointers for generating crash minidump file.
 *    If \c pExInfo is NULL, current CPU state is used to create exception pointers.
 *
 *    This function generates the error report and returns control to the caller. It doesn't
 *    terminate the caller process.
 *
 */

CRASHRPTAPI 
void 
GenerateErrorReport(
   LPVOID lpState,
   PEXCEPTION_POINTERS pExInfo
   );

#endif //_CRASHRPT_REMOVE_DEPRECATED


#define CR_HTTP 0 //!< Send error report via HTTP connection
#define CR_SMTP 1 //!< Send error report via SMTP connection
#define CR_SMAPI 2 //!< Send error report via simple MAPI (using default mail client)

/*! \ingroup CrashRptStructs
 *  \struct CR_INSTALL_INFOW()
 *  \brief Crash reporting general info used by crInstall()
 *
 *  \remarks
 *
 *    - \c cb should always contain size of this structure in bytes. 
 *
 *    - \c pszAppName is a friendly name of client application. The application name is
 *         displayed in Error Report dialog. This parameter can be NULL.
 *         If this parameter is NULL, the name of EXE file that was used to start caller
 *         process becomes the application name.
 *
 *    - \c pszAppVersion should be the application version. Example: "1.0.1". This parameter can be NULL.
 *         If it equals to NULL, product version extracted from executable file which started the process and 
 *         the product version is used as application version.
 * 
 *    - \c pszEmailTo is the email address of the recipient of error reports, for example
 *         "name@example.com". 
 *         This parameter can be NULL. If it equals to NULL, the crash report won't be sent using
 *         E-mail client.
 *
 *    - \c pszEmailSubject is the subject of the email message. If this parameter is NULL,
 *         the default subject of form '<app_name> <app_version> Error Report' is generated.
 *
 *    - \c pszUrl is the URL of a server-side script that would receive crash report data via HTTP
 *         connection. If this parameter is NULL, HTTP connection won't be used to send crash reports.
 *
 *    - \c pszCrashSenderPath is the absolute path to the directory where CrashSender.exe is located. 
 *         The crash sender process is responsible for letting end user know about the crash and 
 *         sending the error report.
 *         This parameter can be NULL. If NULL, it is assumed that CrashRpt.exe is located in
 *         the same directory as CrashRpt.dll.
 *
 *    - \c pfnCrashCallback is a pointer to the LPGETLOGFILE() crash callback function. The crash callback function is
 *         called by CrashRpt when crash occurs and allows user to add custom files to the 
 *         error report or perform other actions. This parameter can be NULL.
 *         If NULL, crash callback is not called.
 *
 *    - \c uPriorities is an array that defines the preferred ways of sending error reports. 
 *         The available ways are: HTTP connection, SMTP connection or simple MAPI (default mail client).
 *         A priority may be an integer number greater or equal to zero.
 *         The element having index CR_HTML defines priority for using HTML connection.
 *         The element having index CR_SMTP defines priority for using SMTP connection.
 *         The element having index CR_SMAPI defines priority for using the default mail client.
 *         The ways having greater priority will be tried first. If priorities are equal to each other, HTML
 *         connection will be tried first, SMTP connection will be tried second and simple MAPI will be tried
 *         last. 
 *
 *    CR_INSTALL_INFOW and CR_INSTALL_INFOA structures are wide-character and multi-byte character 
 *    versions of CR_INSTALL_INFO(). CR_INSTALL_INFO() typedef defines character set independent mapping.
 *        
 */

typedef struct tagCR_INSTALL_INFOW
{
  WORD cb;                       //!< Size of this structure in bytes; must be initialized before using!
  LPCWSTR pszAppName;             //!< Name of application.
  LPCWSTR pszAppVersion;          //!< Application version.
  LPCWSTR pszEmailTo;             //!< E-mail address of crash reports recipient.
  LPCWSTR pszEmailSubject;        //!< Subject of crash report e-mail. 
  LPCWSTR pszUrl;                 //!< URL of server-side script (used in HTTP connection).
  LPCWSTR pszCrashSenderPath;     //!< Directory name where CrashSender.exe is located.
  LPGETLOGFILE pfnCrashCallback; //!< User crash callback.
  UINT uPriorities[3];           //!< Array of error sending transport priorities.
}
CR_INSTALL_INFOW;

typedef CR_INSTALL_INFOW *PCR_INSTALL_INFOW;

/*! \ingroup CrashRptStructs
 *  \struct CR_INSTALL_INFOA
 *  \copydoc CR_INSTALL_INFOW
 */

typedef struct tagCR_INSTALL_INFOA
{
  WORD cb;                      //!< Size of this structure in bytes; must be initialized before using!
  LPCSTR pszAppName;             //!< Name of application.
  LPCSTR pszAppVersion;          //!< Application version.
  LPCSTR pszEmailTo;             //!< E-mail address of crash reports recipient.
  LPCSTR pszEmailSubject;        //!< Subject of crash report e-mail. 
  LPCSTR pszUrl;                 //!< URL of server-side script (used in HTTP connection).
  LPCSTR pszCrashSenderPath;     //!< Directory name where CrashSender.exe is located.
  LPGETLOGFILE pfnCrashCallback; //!< User crash callback.
  UINT uPriorities[3];           //!< Array of error sending transport priorities.
}
CR_INSTALL_INFOA;

typedef CR_INSTALL_INFOA *PCR_INSTALL_INFOA;

/*! \brief Character set-independent mapping of CR_INSTALL_INFOW and CR_INSTALL_INFOA structures.
 *  \ingroup CrashRptStructs
 */
#ifdef UNICODE
typedef CR_INSTALL_INFOW CR_INSTALL_INFO;
typedef PCR_INSTALL_INFOW PCR_INSTALL_INFO;
#else
typedef CR_INSTALL_INFOA CR_INSTALL_INFO;
typedef PCR_INSTALL_INFOA PCR_INSTALL_INFO; 
#endif // UNICODE

/*! \ingroup CrashRptAPI 
 *  \brief  Installs exception handlers for all threads of the caller process.
 *
 *  \param[in] pInfo General information.
 *
 *  \remarks
 *    This function installs unhandled exception filter for all threads of caller process.
 *    It also installs various C++ exception/error handlers that function for all threads of the caller process.
 *    For more information, see \ref exception_handling
 *
 *    Below is the list of installed handlers:
 *     - WIN32 top-level unhandled exception filter [ \c SetUnhandledExceptionFilter() ]
 *     - C++ pure virtual call handler (Visual Studio .NET 2003 and later) [ \c _set_purecall_handler() ]
 *     - C++ invalid parameter handler (Visual Studio .NET 2005 and later) [ \c _set_invalid_parameter_handler() ]
 *     - C++ new operator error handler (Visual Studio .NET 2003 and later) [ \c _set_new_handler() ]
 *     - C++ buffer overrun handler (Visual Studio .NET 2003 only) [ \c _set_security_error_handler() ]
 *     - C++ abort handler [ \c signal(SIGABRT) ]
 *     - C++ illegal instruction handler [ \c signal(SIGINT) ]
 *     - C++ termination request [ \c signal(SIGTERM) ]
 *
 *    In a multithreaded program, additionally use crInstallToCurrentThread() function for each execution
 *    thread, except the main thread.
 * 
 *    The \c pInfo parameter contains all required information needed to install CrashRpt.
 *
 *    This function fails when \c pInfo->pszCrashSenderPath doesn't contain valid path to CrashSender.exe
 *    or when \c pInfo->pszCrashSenderPath is equal to NULL, but CrashSender.exe is not located in the
 *    directory where CrashRpt.dll located.
 *
 *    On crash, the crash minidump file is created, which contains CPU and 
 *    stack state information. Also XML file is created that contains additional 
 *    information that may be helpful for crash analysis. These files along with several additional
 *    files added with crAddFile() are packed to a single ZIP file.
 *
 *    When crash information is collected, another process, CrashSender, is launched 
 *    and the process where crash had occured is terminated. The CrashSender process is 
 *    responsible for letting the user know about the crash and send the error report.
 * 
 *    The error report can be sent over E-mail using address and subject passed to the
 *    function as CR_INSTALL_INFO() structure members. Another way of sending error report is an HTTP 
 *    request using \c pszUrl member of CR_INSTALL_INFO(). If both the E-mail address and
 *    URL are not specified, this function fails.
 *
 *    crInstallW() and crInstallA() are wide-character and multi-byte character versions of crInstall()
 *    function. The crInstall() macro defines character set independent mapping for these functions.
 *
 *    The following example shows how to use crInstall() and crUninstall() functions.
 *   
 *    \code
 *    #include <windows.h>
 *    #include <assert.h>
 *    #include "CrashRpt.h"
 *
 *    void main()
 *    {
 *      // Install crash reporting
 *      CR_INSTALL_INFO info;
 *      memset(&info, 0, sizeof(CR_INSTALL_INFO));
 *      info.cb = sizeof(CR_INSTALL_INFO);  
 *      info.pszAppName = _T("My App Name");
 *      info.pszAppVersion = _T("1.2.3");
 *      info.pszEmailSubject = "Error Report from My App v.1.2.3";
 *      // The address to send reports by E-mail
 *      info.pszEmailTo = _T("myname@hotmail.com");  
 *      // The URL to send reports via HTTP connection
 *      info.pszUrl = _T("http://myappname.com/utils/crashrpt.php"); 
 *      info.pfnCrashCallback = CrashCallback; 
 *      info.uPriorities[CR_HTTP] = 3; // Try HTTP first
 *      info.uPriorities[CR_SMTP] = 2; // Try SMTP second
 *      info.uPriorities[CR_SMAPI] = 1; // Try system email program last
 *
 *      int nInstResult = crInstall(&info);
 *      assert(nInstResult==0);
 *   
 *      // Here follows your code..
 *
 *      // Uninstall crash reporting
 *      crUninstall();
 *    }
 *    \endcode
 *
 *  \sa crInstallW(), crInstallA(), crInstall(), CR_INSTALL_INFOW, 
 *      CR_INSTALL_INFOA, CR_INSTALL_INFO, crUninstall(), 
 *      CrAutoInstallHelper
 */

CRASHRPTAPI 
int
crInstallW(
  PCR_INSTALL_INFOW pInfo
);

/*! \ingroup CrashRptAPI
 *  \copydoc crInstallW()
 */

CRASHRPTAPI 
int
crInstallA(
  PCR_INSTALL_INFOA pInfo
);

/*! \brief Character set-independent mapping of crInstallW() and crInstallA() functions. 
 * \ingroup CrashRptAPI
 */
#ifdef UNICODE
#define crInstall crInstallW
#else
#define crInstall crInstallA
#endif //UNICODE

/*! \ingroup CrashRptAPI 
 *  \brief Unsinstalls exception handlers previously installed with crInstall().
 *
 *  \return
 *    This function returns zero if succeeded.
 *
 *  \remarks
 *
 *    Call this function on application exit to uninstall exception
 *    handlers previously installed with crInstall(). After function call, the exception handlers
 *    are restored to states that they had before calling crInstall().
 *
 *    This function fails if crInstall() wasn't previously called in context of
 *    current process.
 *
 *    When this function fails, use crGetLastErrorMsg() to retrieve the error message.
 *
 *  \sa crInstallW(), crInstallA(), crInstall(), crUninstall(),
 *      CrAutoInstallHelper
 */

CRASHRPTAPI 
int
crUninstall();


/*! \ingroup CrashRptAPI  
 *  \brief Installs C++ exception/error handlers for the current thread.
 *
 *  \return This function returns zero if succeeded.
 *   
 *  \remarks
 *   
 *   This function sets C++ exception handlers for the caller thread. If you have
 *   several execution threads, you ought to call the function for each thread,
 *   except the main thread.
 *  
 *   The list of C++ exception\error handlers installed with this function:
 *    - terminate handler [ \c set_terminate() ]
 *    - unexpected handler [ \c set_unexpected() ]
 *    - floating point error handler [ \c signal(SIGFPE) ]
 *    - illegal instruction handler [ \c signal(SIGILL) ]
 *    - illegal storage access handler [ \c signal(SIGSEGV) ]    
 *
 *   The crInstall() function automatically installs C++ exception handlers for the
 *   main thread, so no need to call crInstallToCurrentThread() for the main thread.
 *
 *   This function fails if calling it twice for the same thread.
 *   When this function fails, use crGetLastErrorMsg() to retrieve the error message.
 * 
 *   Call crUninstallFromCurrentThread() to uninstall C++ exception handlers from
 *   current thread.
 *
 *   The following example shows how to use crInstallToCurrentThread() and crUninstallFromCurrentThread().
 *
 *   \code
 *
 *   DWORD WINAPI ThreadProc(LPVOID lpParam)
 *   {
 *     // Install exception handlers
 *     crInstallToCurrentThread();
 *
 *     // Your code...
 *
 *     // Uninstall exception handlers
 *     crUninstallFromCurrentThread();
 *    
 *     return 0;
 *   }
 *
 *   // .. Create a thread
 *   DWORD dwThreadId = 0;
 *   HANDLE hWorkingThread = CreateThread(NULL, 0, 
 *            ThreadProc, (LPVOID)NULL, 0, &dwThreadId);
 *
 *   \endcode
 *
 *   \sa crInstallToCurrentThread(), crUninstallFromCurrentThread(), CrThreadAutoInstallHelper
 */

CRASHRPTAPI 
int 
crInstallToCurrentThread();

/*! \ingroup CrashRptAPI  
 *  \brief Uninstalls C++ exception handlers from the current thread.
 *  \return This function returns zero if succeeded.
 *  
 *  \remarks
 *
 *    This function unsets C++ exception handlers for the caller thread. If you have
 *    several execution threads, you ought to call the function for each thread.
 *    After calling this function, the C++ exception handlers for current thread are
 *    replaced with the handlers that were before call of crInstallToCurrentThread().
 *
 *    This function fails if crInstallToCurrentThread() wasn't called for current thread.
 *    When this function fails, use crGetLastErrorMsg() to retrieve the error message.
 *
 *    No need to call this function for the main execution thread. The crUninstall()
 *    will automatically uninstall C++ exception handlers for the main thread.
 *
 *   \sa crInstallToCurrentThread(), crUninstallFromCurrentThread(), CrThreadAutoUninstallHelper
 */

CRASHRPTAPI 
int 
crUninstallFromCurrentThread();


/*! \ingroup CrashRptAPI  
 *  \brief Adds a file to crash report.
 * 
 *  \return This function returns zero if succeeded.
 *
 *  \param[in] pszFile Absolute path to the file to add.
 *  \param[in] pszDesc File description (used in Error Report Details dialog).
 *
 *    This function can be called anytime after crInstall() to add one or more
 *    files to the generated crash report. However, the recommended way is to 
 *    call this function in crash callback.
 *  
 *    \c pszFile should be a valid absolute path of a file to add to crash report. It
 *    is recommended to add small files (several KB in size). If a large file is added,
 *    the crash report sending procedure may fail.
 *
 *    \c pszDesc is a description of a file. It can be NULL.
 *
 *    Function fails if \c pszFile doesn't exist at the moment of function call. 
 * 
 *    The crAddFileW() and crAddFileA() are wide-character and multibyte-character
 *    versions of crAddFile() function. The crAddFile() macro defines character set
 *    independent mapping.
 *
 *  \sa crAddFileW(), crAddFileA(), crAddFile()
 */

CRASHRPTAPI 
int
crAddFileW(
   LPCWSTR pszFile,
   LPCWSTR pszDesc 
   );

/*! \ingroup CrashRptAPI
 *  \copydoc crAddFileW()
 */

CRASHRPTAPI 
int
crAddFileA(
   LPCSTR pszFile,
   LPCSTR pszDesc 
   );

/*! \brief Character set-independent mapping of crAddFileW() and crAddFileA() functions. 
 *  \ingroup CrashRptAPI
 */
#ifdef UNICODE
#define crAddFile crAddFileW
#else
#define crAddFile crAddFileA
#endif //UNICODE



// Exception types
#define CR_WIN32_STRUCTURED_EXCEPTION   0    //!< WIN32 structured exception.
#define CR_CPP_TERMINATE_CALL           1    //!< C++ terminate() call.
#define CR_CPP_UNEXPECTED_CALL          2    //!< C++ unexpected() call.

#if _MSC_VER>=1300
#define CR_CPP_PURE_CALL                3    //!< C++ pure virtual function call.
#define CR_CPP_NEW_OPERATOR_ERROR       4    //!< C++ new operator fault.
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400
#define CR_CPP_SECURITY_ERROR           5    //!< Buffer overrun error.
#endif

#if _MSC_VER>=1400
#define CR_CPP_INVALID_PARAMETER        6    //!< Invalid parameter exception.
#endif

#define CR_CPP_SIGABRT                  7    //!< C++ SIGABRT signal (abort).
#define CR_CPP_SIGFPE                   8    //!< C++ SIGFPE signal (flotating point exception).
#define CR_CPP_SIGILL                   9   //!< C++ SIGILL signal (illegal instruction).
#define CR_CPP_SIGINT                   10   //!< C++ SIGINT signal (CTRL+C).
#define CR_CPP_SIGSEGV                  11   //!< C++ SIGSEGV signal (invalid storage access).
#define CR_CPP_SIGTERM                  12   //!< C++ SIGTERM signal (termination request).

/*! \ingroup CrashRptStructs
 *  \brief Extended exception info used by crGenerateErrorReport().
 *
 *  \remarks
 *
 *  This structure contains essential information needed to generate crash minidump file and
 *  provide the developer with other information about the error.
 *
 *  \c cb must contain the size of this structure in bytes.
 *
 *  \c pexcptrs should contain the exception pointers. If this parameter is NULL, 
 *     the current CPU state is used to generate exception pointers.
 *
 *  \c exctype is the type of exception. This parameter may be one of the following:
 *     - \c CR_WIN32_STRUCTURED_EXCEPTION Win32 structured exception
 *     - \c CR_CPP_TERMINATE_CALL        C++ terminate() function call
 *     - \c CR_CPP_UNEXPECTED_CALL       C++ unexpected() function call
 *     - \c CR_CPP_PURE_CALL Pure virtual method call (Visual Studio .NET 2003 and later) 
 *     - \c CR_CPP_NEW_OPERATOR_ERROR C++ 'new' operator error (Visual Studio .NET 2003 and later)
 *     - \c CR_CPP_SECURITY_ERROR Buffer overrun (Visual Studio .NET 2003 only) 
 *     - \c CR_CPP_INVALID_PARAMETER Invalid parameter error (Visual Studio 2005 and later) 
 *     - \c CR_CPP_SIGABRT C++ SIGABRT signal 
 *     - \c CR_CPP_SIGFPE  C++ floating point exception
 *     - \c CR_CPP_SIGILL  C++ illegal instruction
 *     - \c CR_CPP_SIGINT  C++ SIGINT signal
 *     - \c CR_CPP_SIGSEGV C++ invalid storage access
 *     - \c CR_CPP_SIGTERM C++ termination request
 * 
 *   The \c exctype can be used for crash report classification on developers' side.
 * 
 *   \c code is used if \c exctype is CR_WIN32_STRUCTURED_EXCEPTION and represents the structured exception code. 
 *   If \c pexptrs is NULL, this value is used to initialize \c pexptrs->ExceptionCode member, otherwise it is ignored.
 *
 *   \c fpe_subcode is used if \c exctype is equal to CR_CPP_SIGFPE. It defines the floating point
 *   exception subcode (see \c signal() function ducumentation in MSDN).
 * 
 *   \c expression, \c function, \c file and \c line are used when \c exctype is CR_CPP_INVALID_PARAMETER.
 *   These members are typically non-zero when using debug version of CRT.
 *
 * 
 */

typedef struct tagCR_EXCEPTION_INFO
{
  WORD cb;                   //!< Size of this structure in bytes; should be initialized before using.
  PEXCEPTION_POINTERS pexcptrs; //!< Exception pointers.
  int exctype;               //!< Exception type.
  DWORD code;                //!< Code of structured exception.
  unsigned int fpe_subcode;  //!< Floating point exception subcode.
  const wchar_t* expression; //!< Assertion expression.
  const wchar_t* function;   //!< Function in which assertion happened.
  const wchar_t* file;       //!< File in which assertion happened.
  unsigned int line;         //!< Line number.
}
CR_EXCEPTION_INFO;

typedef CR_EXCEPTION_INFO *PCR_EXCEPTION_INFO;

/*! \ingroup CrashRptAPI  
 *  \brief Manually generates an errror report.
 *
 *  \return This function returns zero if succeeded. When failed, it returns a non-zero value.
 *     Use crGetLastErrorMsg() to retrieve the error message.
 *  
 *  \param[in] pExceptionInfo Exception information. 
 *
 *  \remarks
 *
 *    Call this function to manually generate a crash report. When crash information is collected,
 *    control is returned to the caller. The crGenerateErrorReport() doesn't terminate the caller process.
 *
 *    The crash report contains crash minidump, crash descriptor in XML format and
 *    additional custom files added with crAddFile().
 *
 *    The exception information should be passed using CR_EXCEPTION_INFO structure. 
 *
 *    The following example shows how to use crGenerateErrorReport() function.
 *
 *    \code
 *    CR_EXCEPTION_INFO ei;
 *    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
 *    ei.cb = sizeof(CR_EXCEPTION_INFO);
 *    ei.exctype = CR_WIN32_STRUCTURED_EXCEPTION;
 *    ei.code = 1234;
 *    ei.pexcptrs = NULL;
 *
 *    int result = crGenerateErrorReport(&ei);
 *
 *    if(result!=0)
 *    {
 *      // If goes here, crGenerateErrorReport() has failed
 *      // Get the last error message
 *      TCHAR szErrorMsg[256];
 *      crGetLastErrorMsg(szErrorMsg, 256);
 *    }
 *   
 *    // Manually terminate program
 *    ExitProcess(0);
 *
 *    \endcode
 */

CRASHRPTAPI 
int 
crGenerateErrorReport(   
   CR_EXCEPTION_INFO* pExceptionInfo
   );


/*! \ingroup CrashRptAPI 
 *  \brief Can be used as a structured exception filter.
 *
 *  \return This function returns EXCEPTION_EXECUTE_HANDLER if succeeds, else EXCEPTION_CONTINUE_SEARCH.
 *
 *  \param[in] code Exception code.
 *  \param[in] ep   Exception pointers.
 *
 *  \remarks
 *     
 *     This function can be called instead of C++ structured exception filter
 *     inside of __try __except(Expression) statement. The function generates a error report
 *     and returns control to the exception handler block.
 *
 *     The exception code is usually retrieved with GetExceptionCode() intrinsic function
 *     and the exception pointers are retrieved with GetExceptionInformation() intrinsic 
 *     function.
 *
 *     If an error occurs, this function returns EXCEPTION_CONTINUE_SEARCH.
 *     Use crGetLastErrorMsg() to retrieve the error message on fail.
 *
 *     
 *
 *     The following example shows how to use crExceptionFilter().
 *    
 *     \code
 *     int* p = 0x00000000;   // pointer to NULL
 *     __try
 *     {
 *        *p = 13; // causes an access violation exception;
 *     }
 *     __except(crExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
 *     {   
 *       // Terminate program
 *       ExitProcess(1);
 *     }
 *
 *     \endcode 
 */

CRASHRPTAPI
int 
crExceptionFilter(
  unsigned int code, 
  struct _EXCEPTION_POINTERS* ep);

#define CR_NONCONTINUABLE_EXCEPTION  32   

/*! \ingroup CrashRptAPI  
 *  \brief Emulates a predefined crash situation.
 *
 *  \return This function doesn't return if succeded. If failed, returns non-zero value. Call crGetLastErrorMsg()
 *   to get the last error message().
 *
 *  \param[in] ExceptionType Type of crash.
 *
 *  \remarks
 *
 *    This function uses some a priori incorrect or vulnerable code or raises a C++ signal or raises an uncontinuable
 *    software exception to cause crash.
 *
 *    This function can be used to test if CrashRpt handles a crash situation correctly.
 *    
 *    CrashRpt will intercept an error or exception if crInstall() and/or crInstallToCurrentThread() 
 *    were previously called. crInstall() installs exception handlers that function on per-process basis.
 *    crInstallToCurrentThread() installs exception handlers that function on per-thread basis.
 *    
 *  \c ExceptionType can be one of the following constants:
 *    - \c CR_WIN32_STRUCTURED_EXCEPTION  This will generate a null pointer exception.
 *    - \c CR_CPP_TERMINATE_CALL This results in call of terminate() C++ function.
 *    - \c CR_CPP_UNEXPECTED_CALL This results in call of unexpected() C++ function.
 *    - \c CR_CPP_PURE_CALL This emulates a call of pure virtual method call of a C++ class instance (Visual Studio .NET 2003 and later).
 *    - \c CR_CPP_NEW_OPERATOR_ERROR This emulates C++ new operator failure (Visual Studio .NET 2003 and later).
 *    - \c CR_CPP_SECURITY_ERROR This emulates copy of large amount of data to a small buffer (Visual Studio .NET 2003 only).
 *    - \c CR_CPP_INVALID_PARAMETER This emulates an invalid parameter C++ exception (Visual Studio 2005 and later). 
 *    - \c CR_CPP_SIGABRT This raises SIGABRT signal (abnormal program termination).
 *    - \c CR_CPP_SIGFPE This causes floating point exception.
 *    - \c CR_CPP_SIGILL This raises SIGILL signal (illegal instruction signal).
 *    - \c CR_CPP_SIGINT This raises SIGINT signal.
 *    - \c CR_CPP_SIGSEGV This raises SIGSEGV signal.
 *    - \c CR_CPP_SIGTERM This raises SIGTERM signal (program termination request).
 *    - \c CR_CPP_NONCONTINUABLE_EXCEPTION This raises a noncontinuable software exception (expected result is the same as in CR_WIN32_STRUCTURED_EXCEPTION).
 *
 *  The CR_WIN32_STRUCTURED_EXCEPTION uses incorrect code to cause a null pointer write error.
 *
 *  The CR_CPP_NONCONTINUABLE_EXCEPTION has the same effect as CR_WIN32_STRUCTURED_EXCEPTION, but it uses
 *  \b RaiseException() function call to raise noncontinuable software exception.
 *
 *  The following example shows how to use crEmulateCrash() function.
 *
 *  \code
 *  // emulate null pointer exception (access violation)
 *  crEmulateCrash(CR_WIN32_UNHANDLED_EXCEPTION);
 *  \endcode
 *
 */

CRASHRPTAPI
int
crEmulateCrash(
  unsigned ExceptionType);



/*! \ingroup CrashRptAPI 
 *  \brief Gets the last CrashRpt error message.
 *
 *  \return This function returns length of error message in characters.
 *
 *  \param[out] pszBuffer Pointer to the buffer.
 *  \param[in]  uBuffSize Size of buffer in characters.
 *
 *  \remarks
 *
 *    This function gets the last CrashRpt error message. You can use this function
 *    to retrieve the text status of the last called CrashRpt function.
 *
 *    If buffer is too small for the error message, the message is truncated.
 *
 *  crGetLastErrorMsgW() and crGetLastErrorMsgA() are wide-character and multi-byte character versions
 *  of crGetLastError(). The crGetLastErrorMsg() macro defines character set independent mapping.
 *
 *  The following example shows how to use crGetLastErrorMsg() function.
 *
 *  \code
 *  
 *  // .. call some CrashRpt function
 *
 *  // Get the status message
 *  TCHAR szErrorMsg[256];
 *  crGetLastErrorMsg(szErrorMsg, 256);
 *  \endcode
 *
 *  \sa crGetLastErrorMsgA(), crGetLastErrorMsgW(), crGetLastErrorMsg()
 */

CRASHRPTAPI
int
crGetLastErrorMsgW(
  LPWSTR pszBuffer, 
  UINT uBuffSize);

/*! \ingroup CrashRptAPI
 *  \copydoc crGetLastErrorMsgW()
 *
 */

CRASHRPTAPI
int
crGetLastErrorMsgA(
  LPSTR pszBuffer, 
  UINT uBuffSize);

/*! \brief Defines character set-independent mapping for crGetLastErrorMsgW() and crGetLastErrorMsgA().
 *  \ingroup CrashRptAPI
 */

#ifdef UNICODE
#define crGetLastErrorMsg crGetLastErrorMsgW
#else
#define crGetLastErrorMsg crGetLastErrorMsgA
#endif //UNICODE


//// Helper wrapper classes

/*! \class CrAutoInstallHelper
 *  \ingroup CrashRptWrappers
 *  \brief Installs exception handlers in constructor and uninstalls in destructor.
 *  \remarks
 *    Use this class to easily install/uninstall exception handlers in you \b main()
 *    or \b WinMain() function.
 *
 *    This wrapper class calls crInstall() in its constructor and calls crUninstall() in
 *    its destructor.
 *
 *    Use CrAutoInstallHelper::m_nInstallStatus member to check the return status of crInstall().
 *   
 *    Example:
 *
 *    \code
 *    #include <CrashRpt.h>
 *
 *    void main()
 *    {      
 *      CR_INSTALL_INFO info;
 *      memset(&info, 0, sizeof(CR_INSTALL_INFO));
 *      info.cb = sizeof(CR_INSTALL_INFO);  
 *      info.pszAppName = _T("My App Name");
 *      info.pszAppVersion = _T("1.2.3");
 *      info.pszEmailSubject = "Error Report from My App v.1.2.3";
 *      // The address to send reports by E-mail
 *      info.pszEmailTo = _T("myname@hotmail.com");  
 *      // The URL to send reports via HTTP connection
 *      info.pszUrl = _T("http://myappname.com/utils/crashrpt.php"); 
 *      info.pfnCrashCallback = CrashCallback; 
 *      info.uPriorities[CR_HTTP] = 3; // Try HTTP first
 *      info.uPriorities[CR_SMTP] = 2; // Try SMTP second
 *      info.uPriorities[CR_SMAPI] = 1; // Try system email program last
 *
 *      // Install crash reporting
 *      CrAutoInstallHelper cr_install_helper(&info);
 *      // Check that installed OK
 *      assert(cr_install_helper.m_nInstallStatus==0);
 *
 *      // Your code follows here ...
 *
 *    }
 *    \endcode
 */

class CrAutoInstallHelper
{
public:

  //! Installs exception handlers to the caller process
  CrAutoInstallHelper(PCR_INSTALL_INFOA pInfo)
  {
    m_nInstallStatus = crInstallA(pInfo);
  }

  //! Installs exception handlers to the caller process
  CrAutoInstallHelper(PCR_INSTALL_INFOW pInfo)
  {
    m_nInstallStatus = crInstallW(pInfo);
  }

  //! Uninstalls exception handlers from the caller process
  ~CrAutoInstallHelper()
  {
    crUninstall();
  }

  //! Install status
  int m_nInstallStatus;
};

/*! \class CrThreadAutoInstallHelper
 *  \ingroup CrashRptWrappers
 *  \brief Installs (uninstalls) exception handlers for the caller thread in class' constructor (destructor).
 *  
 *  \remarks
 *
 *   This wrapper class calls crInstallToCurrentThread() in its constructor and 
 *   calls crUninstallFromCurrentThread() in its destructor.
 *
 *   Use CrThreadAutoInstallHelper::m_nInstallStatus member to check 
 *   the return status of crInstallToCurrentThread().
 *
 *   Example:
 *
 *   \code
 *   DWORD WINAPI ThreadProc(LPVOID lpParam)
 *   {
 *     CrThreadAutoInstallHelper cr_thread_install_helper();
 *     assert(cr_thread_install_helper.m_nInstallStatus==0);
 *    
 *     // Your code follows here ...
 *   }
 *   \endcode
 */

class CrThreadAutoInstallHelper
{
public:

  //! Installs exception handlers to the caller thread
  CrThreadAutoInstallHelper()
  {
    m_nInstallStatus = crInstallToCurrentThread();
  }

  //! Uninstalls exception handlers from the caller thread
  ~CrThreadAutoInstallHelper()
  {
    crUninstallFromCurrentThread();
  }

  //! Install status
  int m_nInstallStatus;
};


#endif //_CRASHRPT_H_


