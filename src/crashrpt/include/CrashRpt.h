/*! \file   CrashRpt.h
 *  \brief  Defines the interface for the CrashRpt.DLL.
 *  \date   2003-2009
 *  \author Michael Carruth (mcarruth@gmail.com)
 *  \author zeXspectrum (zexspectrum@gmail.com)
 */

#ifndef _CRASHRPT_H_
#define _CRASHRPT_H_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CRASHRPT_LIB
#define CRASHRPT_DECLSPEC_DLLEXPORT __declspec(dllexport) 
#define CRASHRPT_DECLSPEC_DLLIMPORT __declspec(dllimport) 
#else
#define CRASHRPT_DECLSPEC_DLLEXPORT 
#define CRASHRPT_DECLSPEC_DLLIMPORT
#endif

// This is needed for exporting/importing functions from/to CrashRpt.dll
#ifdef CRASHRPT_EXPORTS
 #define CRASHRPTAPI CRASHRPT_DECLSPEC_DLLEXPORT WINAPI 
#else 
 #define CRASHRPTAPI CRASHRPT_DECLSPEC_DLLIMPORT WINAPI
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
 *  The crash callback function should return \c TRUE to allow generate error report. It should 
 *  return \c FALSE to prevent crash report generation.
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

LPVOID 
CRASHRPTAPI 
InstallW(
   LPGETLOGFILE pfnCallback,
   LPCWSTR pszEmailTo,    
   LPCWSTR pszEmailSubject
   );

/*! \ingroup DeprecatedAPI
 *  \copydoc InstallW()
 */

LPVOID 
CRASHRPTAPI 
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

void 
CRASHRPTAPI 
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

void 
CRASHRPTAPI 
AddFileW(
   IN LPVOID lpState,                         
   IN LPCWSTR pszFile,                         
   IN LPCWSTR pszDesc                          
   );

/*! \ingroup DeprecatedAPI
 *
 *  \copydoc AddFileW()
 */

void 
CRASHRPTAPI 
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

void 
CRASHRPTAPI 
GenerateErrorReport(
   LPVOID lpState,
   PEXCEPTION_POINTERS pExInfo
   );

#endif //_CRASHRPT_REMOVE_DEPRECATED

// Array indices for CR_INSTALL_INFO::uPriorities
#define CR_HTTP 0 //!< Send error report via HTTP connection
#define CR_SMTP 1 //!< Send error report via SMTP connection
#define CR_SMAPI 2 //!< Send error report via simple MAPI (using default mail client)

// Flags for CR_INSTALL_INFO::dwFlags
#define CR_INST_STRUCTURED_EXCEPTION_HANDLER   1    //!< Install structured exception handler
#define CR_INST_TERMINATE_HANDLER              2    //!< Install terminate handler
#define CR_INST_UNEXPECTED_HANDLER             4    //!< Install unexpected handler
#define CR_INST_PURE_CALL_HANDLER              8    //!< Install pure call handler (VS .NET and later)
#define CR_INST_NEW_OPERATOR_ERROR_HANDLER     16   //!< Install new operator error handler (VS .NET and later)
#define CR_INST_SECURITY_ERROR_HANDLER         32   //!< Install security errror handler (VS .NET and later)
#define CR_INST_INVALID_PARAMETER_HANDLER      64   //!< Install invalid parameter handler (VS 2005 and later)
#define CR_INST_SIGABRT_HANDLER                128  //!< Install SIGABRT signal handler
#define CR_INST_SIGFPE_HANDLER                 256  //!< Install SIGFPE signal handler   
#define CR_INST_SIGILL_HANDLER                 512  //!< Install SIGILL signal handler  
#define CR_INST_SIGINT_HANDLER                 1024 //!< Install SIGINT signal handler  
#define CR_INST_SIGSEGV_HANDLER                2048 //!< Install SIGSEGV signal handler
#define CR_INST_SIGTERM_HANDLER                4096 //!< Install SIGTERM signal handler  

#define CR_INST_ALL_EXCEPTION_HANDLERS 0      //!< Install all possible exception handlers
#define CR_INST_CRT_EXCEPTION_HANDLERS 0x1FFE //!< Install exception handlers for the linked CRT module

/*! \ingroup CrashRptStructs
 *  \struct CR_INSTALL_INFOW()
 *  \brief This function defines general information used by crInstall()
 *
 *  \remarks
 *
 *    \a cb should always contain size of this structure in bytes. 
 *
 *    \a pszAppName is a friendly name of client application. The application name is
 *       displayed in Error Report dialog. This parameter can be NULL.
 *       If this parameter is NULL, the name of EXE file that was used to start caller
 *       process becomes the application name.
 *
 *    \a pszAppVersion should be the application version. Example: "1.0.1". This parameter can be NULL.
 *       If it equals to NULL, product version is extracted from the executable file which started 
 *       the process and this product version is used as application version.
 * 
 *    \a pszEmailTo is the email address of the recipient of error reports, for example
 *       "name@example.com". 
 *       This parameter can be NULL. If it equals to NULL, the crash report won't be sent using
 *       E-mail client.
 *
 *    \a pszEmailSubject is the subject of the email message. If this parameter is NULL,
 *       the default subject of form '<app_name> <app_version> Error Report' is generated.
 *
 *    \a pszUrl is the URL of a server-side script that would receive crash report data via HTTP
 *       connection. If this parameter is NULL, HTTP connection won't be used to send crash reports.
 *
 *    \a pszCrashSenderPath is the absolute path to the directory where CrashSender.exe is located. 
 *       The crash sender process is responsible for letting end user know about the crash and 
 *       sending the error report.
 *       This parameter can be NULL. If NULL, it is assumed that CrashRpt.exe is located in
 *       the same directory as CrashRpt.dll.
 *
 *    \a pfnCrashCallback is a pointer to the LPGETLOGFILE() crash callback function. The crash callback function is
 *         called by CrashRpt when crash occurs and allows user to add custom files to the 
 *         error report or perform other actions. This parameter can be NULL.
 *         If NULL, crash callback is not called.
 *
 *    \a uPriorities is an array that defines the preferred ways of sending error reports. 
 *         The available ways are: HTTP connection, SMTP connection or simple MAPI (default mail client).
 *         A priority may be an integer number greater or equal to zero.
 *         The element having index CR_HTML defines priority for using HTML connection.
 *         The element having index CR_SMTP defines priority for using SMTP connection.
 *         The element having index CR_SMAPI defines priority for using the default mail client.
 *         The ways having greater priority will be tried first. If priorities are equal to each other, HTML
 *         connection will be tried the first, SMTP connection will be tried the second and simple MAPI will be tried
 *         the last. 
 *
 *    <b>Since v1.1.2</b> \a dwFlags can be used to select what exception handlers to install. 
 *    Use zero value to install all possible exception handlers or
 *    use a combination of the following values:
 *       
 *      - \c CR_INST_STRUCTURED_EXCEPTION_HANDLER   Install structured exception handler
 *      - \c CR_INST_PURE_CALL_HANDLER              Install pure call handler (VS .NET and later)
 *      - \c CR_INST_NEW_OPERATOR_ERROR_HANDLER     Install new operator error handler (VS .NET and later)
 *      - \c CR_INST_SECURITY_ERROR_HANDLER         Install security errror handler (VS .NET and later)
 *      - \c CR_INST_INVALID_PARAMETER_HANDLER      Install invalid parameter handler (VS 2005 and later)
 *      - \c CR_INST_SIGABRT_HANDLER                Install SIGABRT signal handler
 *      - \c CR_INST_SIGINT_HANDLER                 Install SIGINT signal handler  
 *      - \c CR_INST_SIGTERM_HANDLER                Install SIGTERM signal handler  
 *
 *   <b>Since v1.1.2</b> \a pszPrivacyPolicyURL defines the URL for the Privacy Policy hyperlink of the 
 *   Error Report dialog. If this parameter is NULL, the link is not displayed.
 *
 *  \note
 *
 *    CR_INSTALL_INFOW and CR_INSTALL_INFOA structures are wide-character and multi-byte character 
 *    versions of CR_INSTALL_INFO(). CR_INSTALL_INFO() typedef defines character set independent mapping.
 *        
 */

typedef struct tagCR_INSTALL_INFOW
{
  WORD cb;                        //!< Size of this structure in bytes; must be initialized before using!
  LPCWSTR pszAppName;             //!< Name of application.
  LPCWSTR pszAppVersion;          //!< Application version.
  LPCWSTR pszEmailTo;             //!< E-mail address of crash reports recipient.
  LPCWSTR pszEmailSubject;        //!< Subject of crash report e-mail. 
  LPCWSTR pszUrl;                 //!< URL of server-side script (used in HTTP connection).
  LPCWSTR pszCrashSenderPath;     //!< Directory name where CrashSender.exe is located.
  LPGETLOGFILE pfnCrashCallback;  //!< User crash callback.
  UINT uPriorities[5];            //!< Array of error sending transport priorities.
  DWORD dwFlags;                  //!< Flags.
  LPCWSTR pszPrivacyPolicyURL;    //!< URL of privacy policy agreement.
}
CR_INSTALL_INFOW;

typedef CR_INSTALL_INFOW *PCR_INSTALL_INFOW;

/*! \ingroup CrashRptStructs
 *  \struct CR_INSTALL_INFOA
 *  \copydoc CR_INSTALL_INFOW
 */

typedef struct tagCR_INSTALL_INFOA
{
  WORD cb;                       //!< Size of this structure in bytes; must be initialized before using!
  LPCSTR pszAppName;             //!< Name of application.
  LPCSTR pszAppVersion;          //!< Application version.
  LPCSTR pszEmailTo;             //!< E-mail address of crash reports recipient.
  LPCSTR pszEmailSubject;        //!< Subject of crash report e-mail. 
  LPCSTR pszUrl;                 //!< URL of server-side script (used in HTTP connection).
  LPCSTR pszCrashSenderPath;     //!< Directory name where CrashSender.exe is located.
  LPGETLOGFILE pfnCrashCallback; //!< User crash callback.
  UINT uPriorities[3];           //!< Array of error sending transport priorities.
  DWORD dwFlags;                 //!< Flags.
  LPCSTR pszPrivacyPolicyURL;    //!< URL of privacy policy agreement.
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
 *    It also installs various CRT exception/error handlers that function for all threads of the caller process.
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
 *    In a multithreaded program, additionally use crInstallToCurrentThread2() function for each execution
 *    thread, except the main one.
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
 *    When crash information is collected, another process, <b>CrashSender.exe</b>, is launched 
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
 *      info.dwFlags = 0; // Install all available exception handlers
 *      info.pszPrivacyPolicyURL = _T("http://myappname.com/privacy.html"); // Set URL for privacy policy
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

int
CRASHRPTAPI 
crInstallW(
  PCR_INSTALL_INFOW pInfo
);

/*! \ingroup CrashRptAPI
 *  \copydoc crInstallW()
 */

int
CRASHRPTAPI 
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
 *    are restored to states they had before calling crInstall().
 *
 *    This function fails if crInstall() wasn't previously called in context of
 *    current process.
 *
 *    When this function fails, use crGetLastErrorMsg() to retrieve the error message.
 *
 *  \sa crInstallW(), crInstallA(), crInstall(), crUninstall(),
 *      CrAutoInstallHelper
 */

int
CRASHRPTAPI 
crUninstall();


/*! \ingroup CrashRptAPI  
 *  \brief Installs exception handlers to the current thread.
 *
 *  \return This function returns zero if succeeded.
 *   
 *  \remarks
 *   
 *   This function sets exception handlers for the caller thread. If you have
 *   several execution threads, you ought to call the function for each thread,
 *   except the main one.
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
 *   The crInstallToCurrentThread2() function gives better control of what exception 
 *   handlers to install. 
 *
 *   \sa crInstallToCurrentThread(), crInstallToCurrentThread2(),
 *       crUninstallFromCurrentThread(), CrThreadAutoInstallHelper
 */

int 
CRASHRPTAPI 
crInstallToCurrentThread();

/*! \ingroup CrashRptAPI
 *  \brief Installs exception handlers to the caller thread.
 *  \return This function returns zero if succeeded.
 *  \param[in] dwFlags Flags.
 *
 *  \remarks
 *
 *  This function is available <b>since v.1.1.2</b>.
 *
 *  The function sets exception handlers for the caller thread. If you have
 *  several execution threads, you ought to call the function for each thread,
 *  except the main one.
 *   
 *  The function works the same way as crInstallToCurrentThread(), but provides
 *  an ability to select what exception handlers to install.
 *
 *  \a dwFlags defines what exception handlers to install. Use zero value
 *  to install all possible exception
 *  handlers. Or use a combination of the following constants:
 *
 *      - \c CR_INST_TERMINATE_HANDLER              Install terminate handler
 *      - \c CR_INST_UNEXPECTED_HANDLER             Install unexpected handler
 *      - \c CR_INST_SIGFPE_HANDLER                 Install SIGFPE signal handler   
 *      - \c CR_INST_SIGILL_HANDLER                 Install SIGILL signal handler  
 *      - \c CR_INST_SIGSEGV_HANDLER                Install SIGSEGV signal handler 
 * 
 *  Example:
 *
 *   \code
 *   DWORD WINAPI ThreadProc(LPVOID lpParam)
 *   {
 *     // Install exception handlers
 *     crInstallToCurrentThread2(0);
 *
 *     // Your code...
 *
 *     // Uninstall exception handlers
 *     crUninstallFromCurrentThread();
 *    
 *     return 0;
 *   }
 *   \endcode
 * 
 *  \sa 
 *    crInstallToCurrentThread()
 */

int 
CRASHRPTAPI 
crInstallToCurrentThread2(DWORD dwFlags);

/*! \ingroup CrashRptAPI  
 *  \brief Uninstalls C++ exception handlers from the current thread.
 *  \return This function returns zero if succeeded.
 *  
 *  \remarks
 *
 *    This function unsets C++ exception handlers for the caller thread. If you have
 *    several execution threads, you ought to call the function for each thread.
 *    After calling this function, the exception handlers for current thread are
 *    replaced with the handlers that were before call of crInstallToCurrentThread() 
 *    (or crInstallToCurrentThread2()).
 *
 *    This function fails if crInstallToCurrentThread() (or crInstallToCurrentThread2())
 *    wasn't called for current thread.
 *    When this function fails, use crGetLastErrorMsg() to retrieve the error message.
 *
 *    No need to call this function for the main execution thread. The crUninstall()
 *    will automatically uninstall C++ exception handlers for the main thread.
 *
 *   \sa crInstallToCurrentThread(), crInstallToCurrentThread2(),
 *       crUninstallFromCurrentThread(), CrThreadAutoInstallHelper
 */

int 
CRASHRPTAPI 
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

int
CRASHRPTAPI 
crAddFileW(
   LPCWSTR pszFile,
   LPCWSTR pszDesc 
   );

/*! \ingroup CrashRptAPI
 *  \copydoc crAddFileW()
 */


int
CRASHRPTAPI 
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
#define CR_CPP_PURE_CALL                3    //!< C++ pure virtual function call (VS .NET and later).
#define CR_CPP_NEW_OPERATOR_ERROR       4    //!< C++ new operator fault (VS .NET and later).
#define CR_CPP_SECURITY_ERROR           5    //!< Buffer overrun error (VS .NET only).
#define CR_CPP_INVALID_PARAMETER        6    //!< Invalid parameter exception (VS 2005 and later).
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
 *  \a cb must contain the size of this structure in bytes.
 *
 *  \a pexcptrs should contain the exception pointers. If this parameter is NULL, 
 *     the current CPU state is used to generate exception pointers.
 *
 *  \a exctype is the type of exception. This value can be used for crash report classification on developers' side. 
 *  This parameter may be one of the following:
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
 *   
 * 
 *   \a code is used if \a exctype is \c CR_WIN32_STRUCTURED_EXCEPTION and represents the structured exception code. 
 *   If \a pexptrs is NULL, this value is used when generating exception information for initializing
 *   \c pexptrs->ExceptionRecord->ExceptionCode member, otherwise it is ignored.
 *
 *   \a fpe_subcode is used if \a exctype is equal to \c CR_CPP_SIGFPE. It defines the floating point
 *   exception subcode (see \c signal() function ducumentation in MSDN).
 * 
 *   \a expression, \a function, \a file and \a line are used when \a exctype is \c CR_CPP_INVALID_PARAMETER.
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

int 
CRASHRPTAPI 
crGenerateErrorReport(   
   CR_EXCEPTION_INFO* pExceptionInfo
   );


/*! \ingroup CrashRptAPI 
 *  \brief Can be used as a structured exception filter.
 *
 *  \return This function returns \c EXCEPTION_EXECUTE_HANDLER if succeeds, else \c EXCEPTION_CONTINUE_SEARCH.
 *
 *  \param[in] code Exception code.
 *  \param[in] ep   Exception pointers.
 *
 *  \remarks
 *     
 *     This function can be called instead of a structured exception filter
 *     inside of __try __except(Expression) statement. The function generates a error report
 *     and returns control to the exception handler block.
 *
 *     The exception code is usually retrieved with \b GetExceptionCode() intrinsic function
 *     and the exception pointers are retrieved with \b GetExceptionInformation() intrinsic 
 *     function.
 *
 *     If an error occurs, this function returns \c EXCEPTION_CONTINUE_SEARCH.
 *     Use crGetLastErrorMsg() to retrieve the error message on fail.
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

int 
CRASHRPTAPI
crExceptionFilter(
  unsigned int code, 
  struct _EXCEPTION_POINTERS* ep);

#define CR_NONCONTINUABLE_EXCEPTION  32   

/*! \ingroup CrashRptAPI  
 *  \brief Emulates a predefined crash situation.
 *
 *  \return This function doesn't return if succeded. If failed, returns non-zero value. Call crGetLastErrorMsg()
 *   to get the last error message.
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
 *    CrashRpt will intercept an error or exception if crInstall() and/or crInstallToCurrentThread2() 
 *    were previously called. crInstall() installs exception handlers that function on per-process basis.
 *    crInstallToCurrentThread2() installs exception handlers that function on per-thread basis.
 *    
 *  \a ExceptionType can be one of the following constants:
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
 *  The \c CR_WIN32_STRUCTURED_EXCEPTION uses incorrect code to cause a null pointer write error.
 *
 *  The \c CR_CPP_NONCONTINUABLE_EXCEPTION has the same effect as \c CR_WIN32_STRUCTURED_EXCEPTION, but it uses
 *  \b RaiseException() function call to raise noncontinuable software exception.
 *
 *  The following example shows how to use crEmulateCrash() function.
 *
 *  \code
 *  // emulate null pointer exception (access violation)
 *  crEmulateCrash(CR_WIN32_STRUCTURED_EXCEPTION);
 *  \endcode
 *
 */

int
CRASHRPTAPI
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
 *  of crGetLastErrorMsg(). The crGetLastErrorMsg() macro defines character set independent mapping.
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

int
CRASHRPTAPI
crGetLastErrorMsgW(
  LPWSTR pszBuffer, 
  UINT uBuffSize);

/*! \ingroup CrashRptAPI
 *  \copydoc crGetLastErrorMsgW()
 *
 */

int
CRASHRPTAPI
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


#ifdef __cplusplus
}
#endif


//// Helper wrapper classes

#ifndef _CRASHRPT_NO_WRAPPERS

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
 *   This wrapper class calls crInstallToCurrentThread2() in its constructor and 
 *   calls crUninstallFromCurrentThread() in its destructor.
 *
 *   Use CrThreadAutoInstallHelper::m_nInstallStatus member to check 
 *   the return status of crInstallToCurrentThread2().
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
  CrThreadAutoInstallHelper(DWORD dwFlags=0)
  {
    m_nInstallStatus = crInstallToCurrentThread2(dwFlags);    
  }

  //! Uninstalls exception handlers from the caller thread
  ~CrThreadAutoInstallHelper()
  {
    crUninstallFromCurrentThread();
  }

  //! Install status
  int m_nInstallStatus;
};

#endif //!_CRASHRPT_NO_WRAPPERS

#endif //_CRASHRPT_H_


