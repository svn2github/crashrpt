#pragma once
#include "stdafx.h"
#include <map>
#include "tinyxml.h"

class CCrashDescReader
{
public:

  CCrashDescReader();
  ~CCrashDescReader();

  int Load(CString sFileName);
  
  bool m_bLoaded;

  DWORD m_dwGeneratorVersion;
  
  CString m_sCrashGUID;
  CString m_sAppName;
  CString m_sAppVersion;
  CString m_sImageName;
  CString m_sOperatingSystem;
  CString m_sSystemTimeUTC;
  
  DWORD m_dwExceptionType;
  DWORD m_dwExceptionCode;
  
  DWORD m_dwFPESubcode;
  
  CString m_sInvParamExpression;
  CString m_sInvParamFunction;
  CString m_sInvParamFile;
  DWORD m_dwInvParamLine;

  CString m_sUserEmail;
  CString m_sProblemDescription;

  CString m_sMemoryUsageKbytes;
  CString m_sGUIResourceCount;
  CString m_sOpenHandleCount;

  std::map<CString, CString> m_aFileItems;
  std::map<CString, CString> m_aCustomProps;

private:

  int LoadXmlv10(TiXmlHandle hDoc);
};

