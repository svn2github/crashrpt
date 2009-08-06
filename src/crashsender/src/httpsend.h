#pragma once
#include "stdafx.h"
#include <string>
#include "smtpclient.h"

class CHttpSender
{
public:
  
  BOOL SendAssync(CString sUrl, CString sFileName, AssyncNotification* an);

private:

  static void ParseURL(LPCTSTR szURL, LPTSTR szProtocol, UINT cbProtocol,
    LPTSTR szAddress, UINT cbAddress, DWORD &dwPort, LPTSTR szURI, UINT cbURI);

  static BOOL _Send(CString sURL, CString sFileName, AssyncNotification* an);

  static DWORD WINAPI HttpSendThread(VOID* pParam);

  struct HttpSendThreadParams
  {
    CString m_sURL;
    CString m_sFileName;
    AssyncNotification* an;
  };
};


