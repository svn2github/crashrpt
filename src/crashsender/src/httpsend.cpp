#include "stdafx.h"
#include "httpsend.h"
#include <wininet.h>
#include <sys/stat.h>
#include "base64.h"
#include "md5.h"
#include <string>
#include "Utility.h"

BOOL CHttpSender::SendAssync(CString sUrl, CString sFileName, AssyncNotification* an)
{
  DWORD dwThreadId = 0;
    
  HttpSendThreadParams params; 
  params.m_sURL = sUrl;
  params.m_sFileName = sFileName;
  params.an = an;

  HANDLE hThread = CreateThread(NULL, 0, HttpSendThread, (void*)&params, 0, &dwThreadId);
  if(hThread==NULL)
    return FALSE;

  an->WaitForCompletion();

  return TRUE;
}

DWORD WINAPI CHttpSender::HttpSendThread(VOID* pParam)
{
  HttpSendThreadParams* params = (HttpSendThreadParams*)pParam;
   
  CString sURL = params->m_sURL;
  CString sFileName = params->m_sFileName;
  AssyncNotification* an = params->an;

  an->SetCompleted(0);

  int nResult = _Send(sURL, sFileName, an);

  an->SetCompleted( nResult?0:1 );  

  return nResult;
}

BOOL CHttpSender::_Send(CString sURL, CString sFileName, AssyncNotification* an)
{ 
  strconv_t strconv;
  BOOL bStatus = FALSE;
	TCHAR* hdrs = _T("Content-Type: application/x-www-form-urlencoded");
	LPCTSTR accept[2]={_T("*/*"), NULL};
  int uFileSize = 0;
  BYTE* uchFileData = NULL;
  HINTERNET hSession = NULL;
  HINTERNET hConnect = NULL;
  HINTERNET hRequest = NULL;
  TCHAR szProtocol[512];
  TCHAR szServer[512];
  TCHAR szURI[1024];
  DWORD dwPort;
  struct _stat st;
  int res = -1;
  FILE* f = NULL;
  BOOL bResult = FALSE;
  char* chPOSTRequest = NULL;
  CString sMD5Hash;
  CString sPOSTRequest;
  LPCSTR szPOSTRequest; // ASCII
  char* szPrefix="crashrpt=\"";
  char* szSuffix="\"";
  CString sErrorMsg;
  CHAR szResponce[1024];
  DWORD dwBufSize = 1024;
  MD5 md5;
  MD5_CTX md5_ctx;
  unsigned char md5_hash[16];
  int i=0;  
  CString msg; 

  an->SetProgress(_T("Start sending error report over HTTP"), 0, false);

  an->SetProgress(_T("Creating Internet connection"), 3, false);

  if(an->IsCancelled()){ goto exit; }

  // Create Internet session
	hSession = InternetOpen(_T("CrashRpt"),
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(hSession==NULL)
  {
    an->SetProgress(_T("Error creating Internet conection"), 0);
	  goto exit; // Couldn't create internet session
  }
  
  ParseURL(sURL, szProtocol, 512, szServer, 512, dwPort, szURI, 1024);

  an->SetProgress(_T("Connecting to server"), 5, false);

  // Connect to server
	hConnect = InternetConnect(
    hSession, 
    szServer,
		INTERNET_DEFAULT_HTTP_PORT, 
    NULL, 
    NULL, 
    INTERNET_SERVICE_HTTP, 
    0, 
    1);
	
	if(hConnect==NULL)
  {
    an->SetProgress(_T("Error connecting to server"), 0);
	  goto exit; // Couldn't connect
  }
	
  if(an->IsCancelled()){ goto exit; }

  an->SetProgress(_T("Preparing HTTP request data"), 7, false);

  // Load file data into memory
  res = _tstat(sFileName.GetBuffer(0), &st);
  if(res!=0)
  {
    an->SetProgress(_T("Error opening file"), 0);
    goto exit; // File not found
  }
  
  uFileSize = st.st_size;
  uchFileData = new BYTE[uFileSize];
#if _MSC_VER<1400
  f = _tfopen(sFileName.GetBuffer(0), _T("rb"));
#else
  _tfopen_s(&f, sFileName.GetBuffer(0), _T("rb"));
#endif
  if(!f || fread(uchFileData, uFileSize, 1, f)!=1)
  {
    an->SetProgress(_T("Error reading file"), 0);
    goto exit;  
  }
  fclose(f);

  md5.MD5Init(&md5_ctx);
  md5.MD5Update(&md5_ctx, uchFileData, uFileSize);
  md5.MD5Final(md5_hash, &md5_ctx);
  
  sMD5Hash = _T("&md5=");
  for(i=0; i<16; i++)
  {
    CString number;
    number.Format(_T("%02X"), md5_hash[i]);
    sMD5Hash += number;
  }
  
  sPOSTRequest = base64_encode(uchFileData, uFileSize).c_str();
  sPOSTRequest = szPrefix + sPOSTRequest + szSuffix;  
  sPOSTRequest.Replace(_T("+"), _T("%2B"));
  sPOSTRequest.Replace(_T("/"), _T("%2F"));  

  sPOSTRequest += sMD5Hash;
  
  an->SetProgress(_T("Opening HTTP request"), 10);

  if(an->IsCancelled()){ goto exit; }

  // Send POST request
  hRequest = HttpOpenRequest(hConnect, _T("POST"),
		                         szURI, NULL, NULL, accept, 0, 1);	
	if(hRequest==NULL)
  {
    an->SetProgress(_T("Error opening HTTP request"), 0);
	  goto exit; // Coudn't open request	
  }

  if(an->IsCancelled()){ goto exit; }

  szPOSTRequest = strconv.t2a(sPOSTRequest.GetBuffer(0));

  an->SetProgress(_T("Sending HTTP request"), 50);
  bResult = HttpSendRequest(hRequest, hdrs, (int)_tcslen(hdrs), 
    (void*)szPOSTRequest, (DWORD)strlen(szPOSTRequest));
    
  if(bResult == FALSE)
  {
    an->SetProgress(_T("Error sending HTTP request"), 100, false);
		goto exit; // Couldn't send request
  }
	  
  an->SetProgress(_T("Sending error report over HTTP completed OK"), 10, true);
    
  HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE, szResponce, &dwBufSize, NULL); 
  if(atoi(szResponce)!=200)
  {
    CString msg;
    msg.Format(_T("Error! The server returned code %s"), CString(szResponce));
    an->SetProgress(msg, 0);
    goto exit;
  }    

  InternetReadFile(hRequest, szResponce, 1024, &dwBufSize);
  szResponce[dwBufSize] = 0;
  msg = CString(szResponce, dwBufSize);
  msg = _T("Server returned:") + msg;
  an->SetProgress(msg, 0);
    
  if(atoi(szResponce)!=200)
  {
    an->SetProgress(_T("Failed"), 100, false);
    goto exit;
  }

  an->SetProgress(_T("Sent OK"), 100, false);
  bStatus = TRUE;

exit:

  // Clean up
	if(hRequest) 
    InternetCloseHandle(hRequest);

	if(hConnect) 
    InternetCloseHandle(hConnect);

	if(hSession) 
    InternetCloseHandle(hSession);

  if(chPOSTRequest)
    delete [] chPOSTRequest;
    
  if(uchFileData)
    delete [] uchFileData;

  if(f)
    fclose(f);

  return bStatus;
}


// This method's code was taken from 
// http://www.codeproject.com/KB/IP/simplehttpclient.aspx
void CHttpSender::ParseURL(LPCTSTR szURL, LPTSTR szProtocol, UINT cbProtocol, 
                           LPTSTR szAddress, UINT cbAddress, DWORD &dwPort, 
                           LPTSTR szURI, UINT cbURI)
{  
	cbURI;
	cbAddress;
	cbProtocol;
	
	DWORD dwPosition=0;
	BOOL bFlag=FALSE;

	while(_tcslen(szURL)>0 && dwPosition<_tcslen(szURL) && _tcsncmp((szURL+dwPosition), _T(":"), 1))
		++dwPosition;

	if(!_tcsncmp((szURL+dwPosition+1), _T("/"), 1)){	// is PROTOCOL
		if(szProtocol){
			_TCSNCPY_S(szProtocol, cbProtocol, szURL, dwPosition);
			szProtocol[dwPosition]=0;
		}
		bFlag=TRUE;
	}else{	// is HOST 
		if(szProtocol){
			_TCSNCPY_S(szProtocol, cbProtocol, _T("http"), 4);
			szProtocol[5]=0;
		}
	}

	DWORD dwStartPosition=0;
	
	if(bFlag){
		dwStartPosition=dwPosition+=3;				
	}else{
		dwStartPosition=dwPosition=0;
	}

	bFlag=FALSE;
	while(_tcslen(szURL)>0 && dwPosition<_tcslen(szURL) && _tcsncmp(szURL+dwPosition, _T("/"), 1))
			++dwPosition;

	DWORD dwFind=dwStartPosition;

	for(;dwFind<=dwPosition;dwFind++){
		if(!_tcsncmp((szURL+dwFind), _T(":"), 1)){ // find PORT
			bFlag=TRUE;
			break;
		}
	}

	if(bFlag)
  {
		TCHAR sztmp[256]=_T("");
		_TCSNCPY_S(sztmp, 256, (szURL+dwFind+1), dwPosition-dwFind);
		dwPort=_ttol(sztmp);
    int len = dwFind-dwStartPosition;
		_TCSNCPY_S(szAddress, cbAddress, (szURL+dwStartPosition), len);
    szAddress[len]=0;
	}
  else if(!_tcscmp(szProtocol,_T("https")))
  {
		dwPort=INTERNET_DEFAULT_HTTPS_PORT;
    int len = dwPosition-dwStartPosition;
		_TCSNCPY_S(szAddress, cbAddress, (szURL+dwStartPosition), len);
    szAddress[len]=0;
	}
  else 
  {
		dwPort=INTERNET_DEFAULT_HTTP_PORT;
    int len = dwPosition-dwStartPosition;
		_TCSNCPY_S(szAddress, cbAddress, (szURL+dwStartPosition), len);    
    szAddress[len]=0;
	}

	if(dwPosition<_tcslen(szURL))
  { 
    // find URI
    int len = (int)(_tcslen(szURL)-dwPosition);
		_TCSNCPY_S(szURI, cbURI, (szURL+dwPosition), len);
    szURI[len] = 0;
	}
  else
  {
		szURI[0]=0;
	}

	return;
}

