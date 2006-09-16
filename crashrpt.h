/*
 *	CrashRpt.h
 *
 * Crash reporting library.
 *
 * Copyright 2003-2005 Michael Carruth. All Rights Reserved.
 *
 * Contact 
 *    mikecarruth(at)gmail(dot)com
 *  
 * Purpose
 *    This file defines the public interface of crashrpt.dll which
 *    provides unhandled exception data collection and reporting facilities.
 *
 * License
 *    This software is provided 'as-is', without any express or implied 
 *    warranty. In no event will the authors be held liable for any damages 
 *    arising from the use of this software.
 *    
 *    Permission is granted to anyone to use this software for any purpose, 
 *    including commercial applications, and to alter it and redistribute it 
 *    freely, subject to the following restrictions:
 *    
 *       1. The origin of this software must not be misrepresented; you must 
 *       not claim that you wrote the original software. If you use this 
 *       software in a product, an acknowledgment in the product documentation 
 *       would be appreciated but is not required.
 *       
 *       2. Altered source versions must be plainly marked as such, and must 
 *       not be misrepresented as being the original software.
 *       
 *       3. This notice may not be removed or altered from any source 
 *       distribution.
 */


#ifndef CRASHRPT_H
#define CRASHRPT_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Types
 */

#ifndef __LHANDLE
#define __LHANDLE
typedef ULONG_PTR LHANDLE, FAR * LPLHANDLE;
#endif;

/* Callback function */
typedef LONG (__stdcall *LPCRASHREPORT_EXCEPTION_CALLBACK)(
   LHANDLE hReport, 
   EXCEPTION_POINTERS* pExceptionPtrs
   );

/* CRAddMiniDump() type flags - see MINIDUMP_TYPE in dbghelp.h */
typedef enum _CR_MINIDUMP_TYPE {
    CRMiniDumpNormal                         = 0x0000,
    CRMiniDumpWithDataSegs                   = 0x0001,
    CRMiniDumpWithFullMemory                 = 0x0002,
    CRMiniDumpWithHandleData                 = 0x0004,
    CRMiniDumpFilterMemory                   = 0x0008,
    CRMiniDumpScanMemory                     = 0x0010,
    CRMiniDumpWithUnloadedModules            = 0x0020,
    CRMiniDumpWithIndirectlyReferencedMemory = 0x0040,
    CRMiniDumpFilterModulePaths              = 0x0080,
    CRMiniDumpWithProcessThreadData          = 0x0100,
    CRMiniDumpWithPrivateReadWriteMemory     = 0x0200,
} CR_MINIDUMP_TYPE;

/* Return codes */
#define CRASH_SUCCESS                        0
#define CRASH_E_INVALID_HANDLE               1


#ifdef CRASHRPT_SOURCE
#define CRASHRPTAPI CR_RETURN_CODES __stdcall
#define CRASHRPTAPI_(type) type __stdcall
#else
#define CRASHRPTAPI __declspec(dllimport) CR_RETURN_CODES __stdcall
#define CRASHRPTAPI_(type) __declspec(dllimport) type __stdcall
#endif

/*
 * Entry points
 */

/* ----------------------------------------------------------------------------
 * CRInitCrashHandler
 *    Registers unhandled exception filter.
 *
 * Parameters
 *    pCallback         (optional) Pointer to user-defined callback.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    Initializes the crash report library, registers an
 *    unhandled exception filter, and sets the optional 
 *    user-defined callback function.
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRInitCrashHandler(
   LPCRASHREPORT_EXCEPTION_CALLBACK pCallback,
   );

/* ----------------------------------------------------------------------------
 * CRCreateReport
 *    Creates the crash report object.
 *
 * Parameters
 *    lpszFile          (optional) INI settings file.
 *
 * Return Values
 *    Void pointer (handle) to the crash report object.
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI_(LHANDLE)
CRCreateReportA(
   LPCSTR lpszFile
   );

CRASHRPTAPI_(LHANDLE)
CRCreateReportW(
   LPCWSTR lpwszFile
   );

/* ----------------------------------------------------------------------------
 * CRDestroyReport
 *    Deallocates the crash report object and frees all associated resources.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by 
 *                      CRCreateReport.
 *
 * Return Values
 *    Void
 *
 * Remarks   
 *    Uninitializes the crash report library.  After calling CRDestroyReport
 *    the crash report object will be set to NULL.
 * --------------------------------------------------------------------------*/
CRASHRPTAPI_(void)
CRDestroyReport(
   LPLHANDLE lphReport
   );

/* ----------------------------------------------------------------------------
 * CRAddFile
 *    Adds a file to the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszFile           Fully qualified path to the file to add.
 *    pszDesc           File description.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRAddFileA(
   IN LHANDLE hReport,
   IN LPCSTR pszFile,
   IN LPCSTR pszDesc
   );

CRASHRPTAPI
CRAddFileW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszFile,
   IN LPCWSTR pwszDesc
   );

/* ----------------------------------------------------------------------------
 * CRRemoveFile
 *    Removes a file from the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszFile           Fully qualified path to the file to remove.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRRemoveFileA(
   IN LHANDLE hReport,
   IN LPCSTR pszFile
   );

CRASHRPTAPI
CRRemoveFileW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszFile
   );

/* ----------------------------------------------------------------------------
 * CRAddRegistryHive
 *    Adds a registry hive to the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszHive           Registry hive to add.
 *    pszDesc           Description of the hive.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRAddRegistryHiveA(
   IN LHANDLE hReport,
   IN LPCSTR pszHive,
   IN LPCSTR pszDesc
   );

CRASHRPTAPI
CRAddRegistryHiveW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszHive,
   IN LPCWSTR pszDesc
   );

/* ----------------------------------------------------------------------------
 * CRRemoveRegistryHive
 *    Removes a registry hive from the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszHive           Registry have to remove.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRRemoveRegistryHiveA(
   IN LHANDLE hReport,
   IN LPCSTR pszHive
   );

CRASHRPTAPI
CRRemoveRegistryHiveW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszHive
   );

/* ----------------------------------------------------------------------------
 * CRAddEventLog
 *    Adds an event log to the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszSourceName     Name of the log to add.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRAddEventLogA(
   IN LHANDLE hReport,
   IN LPCSTR pszSourceName
   );

CRASHRPTAPI
CRAddEventLogW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszSourceName
   );

/* ----------------------------------------------------------------------------
 * CRRemoveEventLog
 *    Removes an event log from the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszSourceName     Name of the log to remove.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRRemoveEventLogA(
   IN LHANDLE hReport,
   IN LPCSTR pszSourceName
   );

CRASHRPTAPI
CRRemoveEventLogW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszSourceName
   );

/* ----------------------------------------------------------------------------
 * CRAddCustomString
 *    Adds a label value pair to the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszLabel          Label name.          
 *    pszValue          Value.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRAddCustomStringA(
   IN LHANDLE hReport,
   IN LPCSTR pszLabel,
   IN LPCSTR pszValue
   );

CRASHRPTAPI
CRAddCustomStringW(
   IN LHANDLE hReport,
   IN LPCWSTR pszLabel,
   IN LPCWSTR pwszValue
   );

/* ----------------------------------------------------------------------------
 * CRRemoveCustomString
 *    Removes a label value pair from the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszLabel          Label name.
 *    pszValue          Value
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRRemoveCustomStringA(
   IN LHANDLE hReport,
   IN LPCSTR pszLabel
   );

CRASHRPTAPI
CRRemoveCustomStringW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszLabel
   );

/* ----------------------------------------------------------------------------
 * CRAddWMIQuery
 *    Adds the results of the specified WMI query to the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszQuery          WMI query to execute.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRAddWMIQueryA(
   IN LHANDLE hReport,
   IN LPCSTR pszQuery
   );

CRASHRPTAPI
CRAddWMIQueryW(
  IN LHANDLE hReport,
  IN LPCWSTR pwszQuery
  );

/* ----------------------------------------------------------------------------
 * CRRemoveWMIQuery
 *    Removes the WMI query from the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszQuery          WMI query to remove from the report.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRRemoveWMIQueryA(
   IN LHANDLE hReport,
   IN LPCSTR pszQuery
   );

CRASHRPTAPI
CRRemoveWMIQueryW(
  IN LHANDLE hReport,
  IN LPCWSTR pwszQuery
  );

/* ----------------------------------------------------------------------------
 * CRAddMiniDump
 *    Adds a process minidump to the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pExceptionPtrs    Exception pointers to create the minidump.
 *    type              (optional) Mini dump type.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRAddMiniDump(
   IN LHANDLE hReport,
   EXCEPTION_POINTERS* pExceptionPtrs,
   CR_MINIDUMP_TYPE type
   );

/* ----------------------------------------------------------------------------
 * CRRemoveMiniDump
 *    Removes a process minidump from the crash report.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRRemoveMiniDump(
   IN LHANDLE hReport
   );

/* ----------------------------------------------------------------------------
 * CRSendReportViaHttp
 *    Sends the report over HTTP using the user's IE proxy settings.
 *
 * Parameters
 *    hReport           Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszURL            Name of the resource.         
 *    pszVerb           The HTTP verb ("GET","PUT","POST",...)
 *    pszHeaders        (optional) Comma-seperated list of HTTP headers in
 *                      the form "label=value,label2=value2,..."
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRSendReportViaHttpA(
   IN LHANDLE hReport,
   IN LPCSTR pszURL,
   IN LPCSTR pszVerb,
   IN LPCSTR pszHeaders
   );

CRASHRPTAPI
CRSendReportViaHttpW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszURL,
   IN LPCWSTR pwszVerb,
   IN LPCWSTR pwszHeaders
   );

/* ----------------------------------------------------------------------------
 * CRSendReportViaSmtp
 *    Sends the report over SMTP.
 *
 * Parameters
 *    hReport         Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszFrom           Sender's return email address
 *    pszTo             Receipient's email address
 *    pszSubject        Mail subject
 *    pszMessage        Mail body
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRSendReportViaSmtpA(
   IN LHANDLE hReport,
   IN LPCSTR pszFrom,
   IN LPCSTR pszTo,
   IN LPCSTR pszSubject,
   IN LPCSTR pszMessage,
   );

CRASHRPTAPI
CRSendReportViaSmtpW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszFrom,
   IN LPCWSTR pwszTo,
   IN LPCWSTR pwszSubject,
   IN LPCWSTR pwszMessage
   );

/* ----------------------------------------------------------------------------
 * CRSaveReport
 *    Saves the report.
 *
 * Parameters
 *    hReport         Handle to the crash report object returned by
 *                      CRCreateReport.
 *    pszFile           Fully qualifed file name.
 *
 * Return Values
 *    CR_SUCCESS, CR_ERROR_UNEXPECTED
 *
 * Remarks   
 *    None
 * --------------------------------------------------------------------------*/
CRASHRPTAPI
CRSaveReportA(
   IN LHANDLE hReport,
   IN LPCSTR pszFile
   );

CRASHRPTAPI
CRSaveReportW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszFile
   );

#ifdef __cplusplus
}
#endif
#endif /* CRASHRPT_H */
