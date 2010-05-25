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

#include "stdafx.h"
#include <windows.h>
#include "resource.h"
#include "ErrorReportDlg.h"
#include "Utility.h"
#include "tinyxml.h"
#include "zip.h"
#include "unzip.h"
#include "CrashInfoReader.h"
#include "strconv.h"

BOOL CErrorReportDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CErrorReportDlg::OnIdle()
{
	return FALSE;
}

LRESULT CErrorReportDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{   
  CString sRTL = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("Settings"), _T("RTLReading"));
  if(sRTL.CompareNoCase(_T("1"))==0)
  {
    Utility::SetLayoutRTL(m_hWnd);
  }

  SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("DlgCaption")));

	// center the dialog on the screen
	CenterWindow();
	
  // Set window icon
  SetIcon(::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME)), 0);

  // Load heading icon
  int nCurReport = g_CrashInfo.m_nCurrentReport;
  HMODULE hExeModule = LoadLibrary(g_CrashInfo.m_Reports[nCurReport].m_sImageName);
  if(hExeModule)
  {
    // Use IDR_MAINFRAME icon which is the default one for the crashed application.
    m_HeadingIcon = ::LoadIcon(hExeModule, MAKEINTRESOURCE(IDR_MAINFRAME));
  }  

  // If there is no IDR_MAINFRAME icon in crashed EXE module, use IDI_APPLICATION system icon
  if(m_HeadingIcon == NULL)
  {
    m_HeadingIcon = ::LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
  }  

  m_statSubHeader = GetDlgItem(IDC_SUBHEADER);
  
  m_link.SubclassWindow(GetDlgItem(IDC_LINK));   
  m_link.SetHyperLinkExtendedStyle(HLINK_COMMANDBUTTON);
  m_link.SetLabel(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("WhatDoesReportContain")));
    
  m_linkMoreInfo.SubclassWindow(GetDlgItem(IDC_MOREINFO));
  m_linkMoreInfo.SetHyperLinkExtendedStyle(HLINK_COMMANDBUTTON);
  m_linkMoreInfo.SetLabel(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("ProvideAdditionalInfo")));
    
  m_statEmail = GetDlgItem(IDC_STATMAIL);
  m_statEmail.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("YourEmail")));
  
  m_editEmail = GetDlgItem(IDC_EMAIL);
  
  m_statDesc = GetDlgItem(IDC_DESCRIBE);
  m_statDesc.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("DescribeProblem")));

  m_editDesc = GetDlgItem(IDC_DESCRIPTION);

  m_statIndent =  GetDlgItem(IDC_INDENT);
  
  m_chkRestart = GetDlgItem(IDC_RESTART);
  CString sCaption;
  sCaption.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("RestartApp")), g_CrashInfo.m_sAppName);
  m_chkRestart.SetWindowText(sCaption);
  m_chkRestart.SetCheck(BST_CHECKED);
  m_chkRestart.ShowWindow(g_CrashInfo.m_bAppRestart?SW_SHOW:SW_HIDE);

  m_statConsent = GetDlgItem(IDC_CONSENT);
  
  LOGFONT lf;
  memset(&lf, 0, sizeof(LOGFONT));
  lf.lfHeight = 11;
  lf.lfWeight = FW_NORMAL;
  lf.lfQuality = ANTIALIASED_QUALITY;
  _TCSCPY_S(lf.lfFaceName, 32, _T("Tahoma"));
  CFontHandle hConsentFont;
  hConsentFont.CreateFontIndirect(&lf);
  m_statConsent.SetFont(hConsentFont);

  if(g_CrashInfo.m_sPrivacyPolicyURL.IsEmpty())
    m_statConsent.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("MyConsent2")));
  else
    m_statConsent.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("MyConsent")));

  m_linkPrivacyPolicy.SubclassWindow(GetDlgItem(IDC_PRIVACYPOLICY));
  m_linkPrivacyPolicy.SetHyperLink(g_CrashInfo.m_sPrivacyPolicyURL);
  m_linkPrivacyPolicy.SetLabel(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("PrivacyPolicy")));
  
  BOOL bShowPrivacyPolicy = !g_CrashInfo.m_sPrivacyPolicyURL.IsEmpty();  
  m_linkPrivacyPolicy.ShowWindow(bShowPrivacyPolicy?SW_SHOW:SW_HIDE);
  
  m_statCrashRpt = GetDlgItem(IDC_CRASHRPT);
  m_statHorzLine = GetDlgItem(IDC_HORZLINE);  

  m_btnOk = GetDlgItem(IDOK);
  m_btnOk.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("SendReport")));

  m_btnCancel = GetDlgItem(IDCANCEL);
  m_btnCancel.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("CloseTheProgram")));

  memset(&lf, 0, sizeof(LOGFONT));
  lf.lfHeight = 25;
  lf.lfWeight = FW_NORMAL;
  lf.lfQuality = ANTIALIASED_QUALITY;
  _TCSCPY_S(lf.lfFaceName, 32, _T("Tahoma"));
  m_HeadingFont.CreateFontIndirect(&lf);

  m_Layout.SetContainerWnd(m_hWnd);
  m_Layout.Insert(m_linkMoreInfo);
  m_Layout.Insert(m_statIndent);
  m_Layout.Insert(m_statEmail, TRUE);
  m_Layout.Insert(m_editEmail, TRUE);
  m_Layout.Insert(m_statDesc, TRUE);
  m_Layout.Insert(m_editDesc, TRUE);
  m_Layout.Insert(m_chkRestart);
  m_Layout.Insert(m_statConsent);
  m_Layout.Insert(m_linkPrivacyPolicy);
  m_Layout.Insert(m_statHorzLine);
  m_Layout.Insert(m_statCrashRpt, TRUE);
  m_Layout.Insert(m_btnOk);
  m_Layout.Insert(m_btnCancel, TRUE);

  ShowMoreInfo(FALSE);

  m_dlgProgress.Create(m_hWnd);
  m_dlgProgress.Start(TRUE);
  
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	return TRUE;
}

void CErrorReportDlg::ShowMoreInfo(BOOL bShow)
{
  CRect rc1, rc2;

  m_statEmail.ShowWindow(bShow?SW_SHOW:SW_HIDE);
  m_editEmail.ShowWindow(bShow?SW_SHOW:SW_HIDE);
  m_statDesc.ShowWindow(bShow?SW_SHOW:SW_HIDE);
  m_editDesc.ShowWindow(bShow?SW_SHOW:SW_HIDE);
  m_statIndent.ShowWindow(bShow?SW_SHOW:SW_HIDE);

  m_Layout.Update();

  //GetClientRect(&rc1);
  //rc1.bottom += k*nDeltaY;
  // ResizeClient(rc1.Width(), rc1.Height());

  if(bShow)
    m_editEmail.SetFocus();
  else
    m_btnOk.SetFocus();
}

LRESULT CErrorReportDlg::OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  CDCHandle dc((HDC)wParam);

  RECT rcClient;
  GetClientRect(&rcClient);

  RECT rc;
  CStatic statUpperHorzLine = GetDlgItem(IDC_UPPERHORZ);
  statUpperHorzLine.GetWindowRect(&rc);
  ScreenToClient(&rc);

  COLORREF cr = GetSysColor(COLOR_3DFACE);
  CBrush brush;
  brush.CreateSolidBrush(cr);  

  RECT rcHeading = {0, 0, rcClient.right, rc.bottom};
  dc.FillRect(&rcHeading, (HBRUSH)GetStockObject(WHITE_BRUSH));

  RECT rcBody = {0, rc.bottom, rcClient.right, rcClient.bottom};
  dc.FillRect(&rcBody, brush);

  rcHeading.left = 60;
  rcHeading.right -= 10;

  CString sHeading;
  sHeading.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("HeaderText")), g_CrashInfo.m_sAppName);
  dc.SelectFont(m_HeadingFont);
  dc.DrawTextEx(sHeading.GetBuffer(0), sHeading.GetLength(), &rcHeading, 
    DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);  

  if(m_HeadingIcon)
  {
    ICONINFO ii;
    m_HeadingIcon.GetIconInfo(&ii);
    dc.DrawIcon(16, rcHeading.bottom/2 - ii.yHotspot, m_HeadingIcon);
  }

  return TRUE;
}

LRESULT CErrorReportDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  // If needed, restart the application
  g_ErrorReportSender.DoWork(RESTART_APP);
	CloseDialog(wID);  
	return 0;
}

LRESULT CErrorReportDlg::OnCompleteCollectCrashInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if(!g_CrashInfo.m_bSilentMode)
  {
    LONG64 lTotalSize = g_ErrorReportSender.GetUncompressedReportSize()/1024;  
    CString sTotalSize;
    sTotalSize.Format(_T("%I64d KB"), lTotalSize);
    CString sSubHeader;
    sSubHeader.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("SubHeaderText")), sTotalSize);
    m_statSubHeader.SetWindowText(sSubHeader);
    ShowWindow(SW_SHOW);
  }
  else
    g_ErrorReportSender.DoWork(COMPRESS_REPORT|SEND_REPORT);
  
  return 0;
}

LRESULT CErrorReportDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  CreateTrayIcon(FALSE, m_hWnd);
  CloseDialog(0);  
  return 0;
}

void CErrorReportDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

LRESULT CErrorReportDlg::OnLinkClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{  
  CDetailDlg dlg;
  dlg.DoModal();
  return 0;
}

LRESULT CErrorReportDlg::OnMoreInfoClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
 m_linkMoreInfo.EnableWindow(0);
 ShowMoreInfo(TRUE);
 return 0;
}

LRESULT CErrorReportDlg::OnRestartClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  BOOL bRestart = m_chkRestart.GetCheck()==BST_CHECKED?TRUE:FALSE;
  g_CrashInfo.m_bAppRestart = bRestart;

  return 0;
}

LRESULT CErrorReportDlg::OnSend(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{  
  HWND     hWndEmail = GetDlgItem(IDC_EMAIL);
  HWND     hWndDesc = GetDlgItem(IDC_DESCRIPTION);
  int      nEmailLen = ::GetWindowTextLength(hWndEmail);
  int      nDescLen = ::GetWindowTextLength(hWndDesc);

  int nCurReport = g_CrashInfo.m_nCurrentReport;
  LPTSTR lpStr = g_CrashInfo.m_Reports[nCurReport].m_sEmailFrom.GetBufferSetLength(nEmailLen+1);
  ::GetWindowText(hWndEmail, lpStr, nEmailLen+1);
  g_CrashInfo.m_Reports[nCurReport].m_sEmailFrom.ReleaseBuffer();

  lpStr = g_CrashInfo.m_Reports[nCurReport].m_sDescription.GetBufferSetLength(nDescLen+1);
  ::GetWindowText(hWndDesc, lpStr, nDescLen+1);
  g_CrashInfo.m_Reports[nCurReport].m_sDescription.ReleaseBuffer();

  //
  // If an email address was entered, verify that
  // it [1] contains a @ and [2] the last . comes
  // after the @.
  //
  
  if (g_CrashInfo.m_Reports[nCurReport].m_sEmailFrom.GetLength() &&
      (g_CrashInfo.m_Reports[nCurReport].m_sEmailFrom.Find(_T('@')) < 0 ||
       g_CrashInfo.m_Reports[nCurReport].m_sEmailFrom.ReverseFind(_T('.')) < 
       g_CrashInfo.m_Reports[nCurReport].m_sEmailFrom.Find(_T('@'))))
  {
    DWORD dwFlags = 0;
    CString sRTL = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("Settings"), _T("RTLReading"));
    if(sRTL.CompareNoCase(_T("1"))==0)
      dwFlags = MB_RTLREADING;
  
     // alert user     
     MessageBox(
       Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("InvalidEmailText")), 
       Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("InvalidEmailCaption")), 
       MB_OK|dwFlags);
     
     // select email
     ::SetFocus(hWndEmail);

     return 0;
  }

  // Write user email and problem description to XML
  g_CrashInfo.AddUserInfoToCrashDescriptionXML(g_CrashInfo.m_Reports[nCurReport].m_sEmailFrom, g_CrashInfo.m_Reports[nCurReport].m_sDescription);
    
  ShowWindow(SW_HIDE);
  CreateTrayIcon(true, m_hWnd);
  g_ErrorReportSender.SetExportFlag(FALSE, _T(""));
  g_ErrorReportSender.DoWork(COMPRESS_REPORT|RESTART_APP|SEND_REPORT);
  m_dlgProgress.Start(FALSE);    
  SetTimer(0, 500);
  
  // Notify user has confirmed error report submission
  g_ErrorReportSender.FeedbackReady(0);

  return 0;
}

LRESULT CErrorReportDlg::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if(WaitForSingleObject(m_hSenderThread, 0)==WAIT_OBJECT_0 )
  {
    KillTimer(0);    
  }
  
  return 0;
}

LRESULT CErrorReportDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
  if((HWND)lParam!=m_statIcon)
    return 0;

  HDC hDC = (HDC)wParam;
  SetBkColor(hDC, RGB(0, 255, 255));
  SetTextColor(hDC, RGB(0, 255, 255));
  return (LRESULT)TRUE;
}

int CErrorReportDlg::CreateTrayIcon(bool bCreate, HWND hWndParent)
{
  NOTIFYICONDATA nf;
	memset(&nf,0,sizeof(NOTIFYICONDATA));
	nf.cbSize = sizeof(NOTIFYICONDATA);
	nf.hWnd = hWndParent;
	nf.uID = 0;
	
	if(bCreate==true) // add icon to tray
	{
		nf.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nf.uCallbackMessage = WM_TRAYICON;
#if _MSC_VER>=1300		
		nf.uVersion = NOTIFYICON_VERSION;
#endif

		nf.hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
	  _TCSCPY_S(nf.szTip, 128, _T("Sending Error Report"));
		
		Shell_NotifyIcon(NIM_ADD, &nf);
	}
	else // delete icon
	{
		Shell_NotifyIcon(NIM_DELETE, &nf);
	}
  return 0;
}

               
LRESULT CErrorReportDlg::OnTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
  UINT uMouseMsg = (UINT)lParam; 

	if(uMouseMsg==WM_LBUTTONDBLCLK)
	{
	  m_dlgProgress.ShowWindow(SW_SHOW);  	
    m_dlgProgress.SetFocus();
	}	
  return 0;
}