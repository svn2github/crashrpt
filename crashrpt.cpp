// #include <windows.h>
// #include <atlconv.h>

#define CRASHRPT_SOURCE
#include "crashrpt.h"

CRASHRPTAPI
CRInitCrashHandler(
   LPCRASHREPORT_EXCEPTION_CALLBACK pCallback)
{
   return 0;
}

CRASHRPTAPI_(LHANDLE)
CRCreateReportA(LPCSTR lpszFile)
{
   USES_CONVERSION;
   return ( CRCreateReportW(A2W(lpszFile)) );
}

CRASHRPTAPI_(LHANDLE)
CRCreateReportW(LPCWSTR lpwszFile)
{
   return ( new CRReport(lpwszFile) );
}

CRASHRPTAPI_(void)
CRDestroyReport(LPLHANDLE lphReport)
{
   if ( lphReport && *lphReport )
      delete (dynamic_cast<CRReport*>(*lphReport));

   *lphReport = NULL;
}

CRASHRPTAPI
CRAddFileA(
   IN LHANDLE hReport,
   IN LPCSTR pszFile,
   IN LPCSTR pszDesc)
{
   USES_CONVERSION;
   return ( CRAddFileW( hReport, A2W(pszFile), A2W(pszDesc) ) );
}

CRASHRPTAPI
CRAddFileW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszFile,
   IN LPCWSTR pwszDesc)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Add(CRFileItem(pwszFile, pwszDesc)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRRemoveFileA(
   IN LHANDLE hReport,
   IN LPCSTR pszFile)
{
   USES_CONVERSION;
   return ( CRRemoveFileW( hReport, A2W(pszFile) ) );
}

CRASHRPTAPI
CRRemoveFileW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszFile)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Remove(CRFileItem(pwszFile)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRAddRegistryHiveA(
   IN LHANDLE hReport,
   IN LPCSTR pszHive,
   IN LPCSTR pszDesc)
{
   USES_CONVERSION;
   return ( CRAddRegistryHiveW( hReport, A2W(pszHive), A2W(pszDesc) ) );
}

CRASHRPTAPI
CRAddRegistryHiveW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszHive,
   IN LPCWSTR pwszDesc)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Add(CRRegItem(pwszHive, pwszDesc)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRRemoveRegistryHiveA(
   IN LHANDLE hReport,
   IN LPCSTR pszHive)
{
   USES_CONVERSION;
   return ( CRRemoveRegistryHiveW( hReport, A2W(pszHive) ) );
}

CRASHRPTAPI
CRRemoveRegistryHiveW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszHive)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Remove(CRegItem(pwszHive)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRAddEventLogA(
   IN LHANDLE hReport,
   IN LPCSTR pszServer,
   IN LPCSTR pszSource)
{
   USES_CONVERSION;
   return ( CRAddEventLogW( hReport, A2W(pszServer), A2W(pszSource) ) );
}

CRASHRPTAPI
CRAddEventLogW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszServer,
   IN LPCWSTR pwszSource)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Add(CREventLogItem(pwszServer, pwszSource)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRRemoveEventLogA(
   IN LHANDLE hReport,
   IN LPCSTR pszSource)
{
   USES_CONVERSION;
   return ( CRRemoveEventLog( hReport, A2W(pszSource) ) );
}

CRASHRPTAPI
CRRemoveEventLogW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszSource)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Remove(CREventLogItem(pwszSource)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRAddCustomStringA(
   IN LHANDLE hReport,
   IN LPCSTR pszLabel,
   IN LPCSTR pszValue)
{
   USES_CONVERSION;
   return ( CRAddCustomString( hReport, A2W(pszLabel), A2W(pszValue) ) );
}

CRASHRPTAPI
CRAddCustomStringW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszLabel,
   IN LPCWSTR pwszValue)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Add(CRCustomItem(pwszLabel, pwszValue)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRRemoveCustomStringA(
   IN LHANDLE hReport,
   IN LPCSTR pszLabel)
{
   USES_CONVERSION;
   return ( CRRemoveCustomStringW( hReport, A2W(pszLabel) ) );
}

CRASHRPTAPI
CRRemoveCustomStringW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszLabel)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Remove(CRCustomItem(pwszLabel)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRAddWMIQuery(
   IN LHANDLE hReport,
   IN LPCSTR pszQuery)
{
   USES_CONVERSION;
   return ( CRAddWMIQueryW( hReport, A2W(pszQuery) ) );
}

CRASHRPTAPI
CRAddWMIQuery(
  IN LHANDLE hReport,
  IN LPCWSTR pwszQuery)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Add(CRWMIQueryItem(pwszQuery)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRRemoveWMIQueryA(
   IN LHANDLE hReport,
   IN LPCSTR pszQuery)
{
   USES_CONVERSION;
   return ( CRRemoveWMIQueryW( hReport, A2W(pszQuery) ) );
}

CRASHRPTAPI
CRRemoveWMIQuery(
  IN LHANDLE hReport,
  IN LPCWSTR pwszQuery)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Remove(CRWMIQueryItem(pwszQuery)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRAddMiniDump(
   IN LHANDLE hReport,
   IN EXCEPTION_POINTERS* pExceptionPtrs,
   IN CR_MINIDUMP_TYPE type)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))
                  ->Add(CRMiniDump(pExceptionPtrs, type)) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRRemoveMiniDump(
   IN LHANDLE hReport)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))->Remove(CRMiniDump()) );

   return CRASH_E_INVALID_HANDLE;
}

CRASHRPTAPI
CRSendReportViaHttpA(
   IN LHANDLE hReport,
   IN LPCSTR pszURL,
   IN LPCSTR pszVerb,
   IN LPCSTR pszHeaders)
{
   USES_CONVERSION;
   return ( CRSendReportViaHttpW( hReport, A2W(pszURL), A2W(pszVerb), 
            A2W(pszHeaders) ) );
}

CRASHRPTAPI
CRSendReportViaHttpW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszURL,
   IN LPCWSTR pwszVerb,
   IN LPCWSTR pwszHeaders)
{
   CRReport pReport = (dynamic_cast<CRReport*>(hReport));
   if ( !pReport )
      return CRASH_E_INVALID_HANDLE;

   CRAddCustomString( hReport, L"SendVia", L"HTTP" );
   CRAddCustomString( hReport, L"URL", pwszURL );
   CRAddCustomString( hReport, L"Verb", pwszVerb );
   CRAddCustomString( hreport, L"Headers", pwszHeaders );

   WCHAR pwszPath[_MAX_PATH] = {0};
   if ( 0 == GetTempPathW( _MAX_PATH, pwszPath ) )
      return CRASH_E_SYSTEM_ERROR;

   WCHAR pwszFile[_MAX_FILE] = {0};
   if ( 0 == GetModuleFileName( NULL, &pwszFile, _MAX_FILE ) )
      return CRASH_E_SYSTEM_ERROR;
   
   if ( !PathRemoveFileSpec( pwszFile ) )
      return CRASH_E_SYSTEM_ERROR;         
   
   StringCchCat( pwszPath, _MAX_PATH, pwszFile );
   CRSaveReportW( hReport, pwszPath );
   // Save report to temp directory
   // Call RunDll32 to invoke crashrpt.dll GUI entry point
   //  passing in the path to the temp file
}

CRASHRPTAPI
CRSendReportViaSmtpA(
   IN LHANDLE hReport,
   IN LPCSTR pszFrom,
   IN LPCSTR pszTo,
   IN LPCSTR pszSubject,
   IN LPCSTR pszMessage)
{
   USES_CONVERSION;
   return ( CRSendReportViaSmtpW( hReport, A2W(pszFrom), A2W(pszTo), 
            A2W(pszSubject), A2W(pszMessage) ) );
}

CRASHRPTAPI
CRSendReportViaSmtpW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszFrom,
   IN LPCWSTR pwszTo,
   IN LPCWSTR pwszSubject,
   IN LPCWSTR pwszMessage) 
{
   CRReport pReport = (dynamic_cast<CRReport*>(hReport));
   if ( !pReport )
      return CRASH_E_INVALID_HANDLE;

   CRAddCustomString( hReport, L"SendVia", L"SMTP" );
   CRAddCustomString( hReport, L"From", pwszFrom );
   CRAddCustomString( hReport, L"To", pwszTo );
   CRAddCustomString( hReport, L"Verb", pwszVerb );
   CRAddCustomString( hreport, L"Subject", pwszSubject );
   CRAddCustomString( hreport, L"Message", pwszMessage );

   WCHAR pwszPath[_MAX_PATH] = {0};
   if ( 0 == GetTempPathW( _MAX_PATH, pwszPath ) )
      return CRASH_E_SYSTEM_ERROR;

   WCHAR pwszFile[_MAX_FILE] = {0};
   if ( 0 == GetModuleFileName( NULL, &pwszFile, _MAX_FILE ) )
      return CRASH_E_SYSTEM_ERROR;
   
   if ( !PathRemoveFileSpec( pwszFile ) )
      return CRASH_E_SYSTEM_ERROR;         
   
   StringCchCat( pwszPath, _MAX_PATH, pwszFile );
   CRSaveReportW( hReport, pwszPath );
   // Save report to temp directory
   // Call RunDll32 to invoke crashrpt.dll GUI entry point
   //  passing in the path to the temp file
}

CRASHRPTAPI
CRSaveReportA(
   IN LHANDLE hReport,
   IN LPCSTR pszFile) 
{
   USES_CONVERSION;
   return ( CRSaveReportW( hReport, A2W(pszFile) ) );
}

CRASHRPTAPI
CRSaveReportW(
   IN LHANDLE hReport,
   IN LPCWSTR pwszFile)
{
   if ( hReport )
      return ( (dynamic_cast<CRReport*>(hReport))->Save(pwszFile) );

   return CRASH_E_INVALID_HANDLE;
}
