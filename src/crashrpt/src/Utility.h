/************************************************************************************* 
  This file is a part of CrashRpt library.

  CrashRpt is Copyright (c) 2003, Michael Carruth
  All rights reserved.
 
  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:
 
   * Redistributions of source code must retain the above copyright notice, this 
     list of conditions and the following disclaimer.
 
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.
 
   * Neither the name of the author nor the names of its contributors 
     may be used to endorse or promote products derived from this software without 
     specific prior written permission.
 

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************************/

// File: Utility.h
// Description: Miscellaneous helper functions
// Authors: mikecarruth, zexspectrum
// Date: 

#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <shlobj.h>

namespace Utility  
{
  // Returns base name of the EXE file that launched current process.
  CString getAppName();
   
  // Returns the unique tmp file name.
  CString getTempFileName();

  // Returns the path to the temporary directory.
  int getTempDirectory(CString& strTemp);

  // Returns path to directory where EXE or DLL module is located.
  CString GetModulePath(HMODULE hModule);

  // Returns the absolute path and name of the module
  CString GetModuleName(HMODULE hModule);

  // Generates unique identifier (GUID)
  int GenerateGUID(CString& sGUID);  

  // Returns current system time as string (uses UTC time format).
  int GetSystemTimeUTC(CString& sTime); 

  // Converts UTC string to local time.
  void UTC2SystemTime(CString sUTC, SYSTEMTIME& st);

  // Returns friendly name of operating system (name, version, service pack)
  int GetOSFriendlyName(CString& sOSName);  

  // Returns path to a special folder (for example %LOCAL_APP_DATA%)
  int GetSpecialFolder(int csidl, CString& sFolderPath);

  // Replaces restricted characters in file name
  CString ReplaceInvalidCharsInFileName(CString sFileName);

  // Moves a file to the Recycle Bin or removes the file permanently
  int RecycleFile(CString sFilePath, bool bPermanentDelete);

  // Retrieves a string from INI file.
  CString GetINIString(LPCTSTR pszFileName, LPCTSTR pszSection, LPCTSTR pszName);
  
  // Adds a string to INI file.
  void SetINIString(LPCTSTR pszFileName, LPCTSTR pszSection, LPCTSTR pszName, LPCTSTR pszValue);

  // Mirrors the content of a window.
  void SetLayoutRTL(HWND hWnd);

  // Formats the error message.
  CString FormatErrorMsg(DWORD dwErrorCode);

  // Parses file path and returns file name.
  CString GetFileName(CString sPath);

  // Parses file path and returns file name without extension.
  CString GetBaseFileName(CString sFileName);

  // Parses file path and returns file extension.
  CString GetFileExtension(CString sFileName);

  // This helper function looks for the app's main window.
  HWND FindAppWindow();

  // Retrieves product version info from resources embedded into EXE or DLL
  CString GetProductVersion(CString sModuleName);
  
  // Creates a folder. If some intermediate folders in the path do not exist,
  // it creates them.
  BOOL CreateFolder(CString sFolderName);

  // Converts system time to UINT64
  ULONG64 SystemTimeToULONG64( const SYSTEMTIME& st );

  CString FileSizeToStr(ULONG64 uFileSize);
};

#endif	// _UTILITY_H_
