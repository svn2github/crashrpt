///////////////////////////////////////////////////////////////////////////////
//
//  Module: Utility.h
//
//    Desc: Misc static helper methods
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _UTILITY_H_
#define _UTILITY_H_

#pragma once

#include <shlobj.h>


////////////////////////////// Class Definitions /////////////////////////////

// ===========================================================================
// CUtility
// 
// See the module comment at top of file.
//
class CUtility  
{
public:

   //-----------------------------------------------------------------------------
   // getLastWriteFileTime
   //    Returns the time the file was last modified in a FILETIME structure.
   //
   // Parameters
   //    sFile       Fully qualified file name
   //
   // Return Values
   //    FILETIME structure
   //
   // Remarks
   //
   static 
   FILETIME 
   getLastWriteFileTime(
      CString sFile
      );
   
   //-----------------------------------------------------------------------------
   // getAppName
   //    Returns the application module's file name
   //
   // Parameters
   //    none
   //
   // Return Values
   //    File name of the executable
   //
   // Remarks
   //    none
   //
   static 
   CString 
   getAppName();

   //-----------------------------------------------------------------------------
   // getSaveFileName
   //    Presents the user with a save as dialog and returns the name selected.
   //
   // Parameters
   //    none
   //
   // Return Values
   //    Name of the file to save to, or "" if the user cancels.
   //
   // Remarks
   //    none
   //
   static 
   CString 
   getSaveFileName();
	
   //-----------------------------------------------------------------------------
   // getTempFileName
   //    Returns a generated temporary file name
   //
   // Parameters
   //    none
   //
   // Return Values
   //    Temporary file name
   //
   // Remarks
   //
   static 
   CString 
   getTempFileName();

   //-----------------------------------------------------------------------------
   // getTempDirectory
   //  Returns value of TEMP environment variable
   //
   //  Return value
   //   0 if successful
   //
   static
   int 
   getTempDirectory(CString& strTemp);

   // Returns path to directory where EXE or DLL module is located.
   static CString GetModulePath(HMODULE hModule);

   // Generates unique identifier (GUID)
   static int GenerateGUID(CString& sGUID);  

   // Returns current system time as string (uses UTC time format).
   static int GetSystemTimeUTC(CString& sTime);  

   // Returns friendly name of operating system (name, version, service pack)
   static int GetOSFriendlyName(CString& sOSName);  

   // Returns path to a special folder (for example %LOCAL_APP_DATA%)
   static int GetSpecialFolder(int csidl, CString& sFolderPath);

   // Replaces restricted characters in file name
   static CString ReplaceInvalidCharsInFileName(CString sFileName);

   // Moves a file to the Recycle Bin or removes the file permanently
   static int RecycleFile(CString sFilePath, bool bPermanentDelete);
};

#include <vector>
class strconv_t
{
public:
  strconv_t(){}
  ~strconv_t()
  {
    unsigned i;
    for(i=0; i<m_ConvertedStrings.size(); i++)
    {
      delete [] m_ConvertedStrings[i];
    }
  }

  LPCWSTR a2w(LPCSTR lpsz)
  {
    int count = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpsz, -1, NULL, 0);
    if(count==0)
      return NULL;

    void* pBuffer = (void*) new wchar_t[count];
    int result = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpsz, -1, (LPWSTR)pBuffer, count);
    if(result==0)
    {
      delete [] pBuffer;
      return NULL;
    }    

    m_ConvertedStrings.push_back(pBuffer);
    return (LPCWSTR)pBuffer;
  }

  LPCSTR w2a(LPCWSTR lpsz)
  {    
    int count = WideCharToMultiByte(CP_ACP, 0, lpsz, -1, NULL, 0, NULL, NULL);
    if(count==0)
      return NULL;

    void* pBuffer = (void*) new char[count];
    int result = WideCharToMultiByte(CP_ACP, 0, lpsz, -1, (LPSTR)pBuffer, count, NULL, NULL);
    if(result==0)
    {
      delete [] pBuffer;
      return NULL;
    }    

    m_ConvertedStrings.push_back(pBuffer);
    return (LPCSTR)pBuffer;
  }

  LPCSTR t2a(LPCTSTR lpsz)
  {
#ifdef UNICODE    
    return w2a(lpsz);
#else
    return lpsz;
#endif
  }

LPCWSTR t2w(LPCTSTR lpsz)
  {
#ifdef UNICODE    
    return lpsz;
#else
    return a2w(lpsz);
#endif
  }

  LPCTSTR a2t(LPCSTR lpsz)
  {
#ifdef UNICODE    
    return a2w(lpsz);
#else
    return lpsz;
#endif
  }

LPCTSTR w2t(LPCWSTR lpsz)
  {
#ifdef UNICODE    
    return lpsz;
#else
    return w2a(lpsz);
#endif
  }

private:
  std::vector<void*> m_ConvertedStrings;  
};


#endif	// #ifndef _UTILITY_H_
