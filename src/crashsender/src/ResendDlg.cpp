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
#include "CrashInfoReader.h"
#include "ResendDlg.h"
#include "Utility.h"
#include "strconv.h"

BOOL CResendDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

LRESULT CResendDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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
  
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
 
  // Show balloon in 3 seconds.
  m_nTick = 0;
  SetTimer(0, 3000);

  return TRUE;
}

LRESULT CResendDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  CloseDialog(0);  
  return 0;
}

LRESULT CResendDlg::OnTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
  if(lParam==WM_LBUTTONDOWN || lParam==WM_LBUTTONDBLCLK)
  {
    KillTimer(0);
    ShowWindow(SW_SHOW);
  }

  if(lParam==WM_RBUTTONDOWN)
  {
    
  }

  return 0;
}

void CResendDlg::CloseDialog(int nVal)
{
	DestroyWindow();
  AddTrayIcon(FALSE);
	::PostQuitMessage(nVal);
}

LRESULT CResendDlg::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{  
  if(m_nTick==0)
  {
    // Show tray icon and balloon.
    AddTrayIcon(TRUE);

    KillTimer(0);

    // Wait for one minute. If user doesn't want to click us, exit.
    SetTimer(0, 60000);
  }
  else if(m_nTick==1)
  {
    KillTimer(0);

    CloseDialog(0);
  }

  m_nTick ++;

  return 0;
}

void CResendDlg::AddTrayIcon(BOOL bAdd)
{
  NOTIFYICONDATA nf;
	memset(&nf,0,sizeof(NOTIFYICONDATA));
	nf.cbSize = sizeof(NOTIFYICONDATA);
	nf.hWnd = m_hWnd;
	nf.uID = 0;

  if(bAdd) // Add icon to tray
	{	
		nf.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO ;
		nf.uCallbackMessage = WM_RESENDTRAYICON;
		nf.uVersion = NOTIFYICON_VERSION;

    CString sTip; 
    sTip.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("DlgCaption")), g_CrashInfo.m_sAppName);
		nf.hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
    _TCSCPY_S(nf.szTip, 64, sTip);
	
    CString sInfo;
    sInfo.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("BalloonText")), 
      g_CrashInfo.m_sAppName, g_CrashInfo.m_sAppName);
    _TCSCPY_S(nf.szInfo, 200, sInfo.GetBuffer(0));

    CString sInfoTitle;
    sInfoTitle.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("BalloonCaption")), 
      g_CrashInfo.m_sAppName);
    _TCSCPY_S(nf.szInfoTitle, 64, sInfoTitle.GetBuffer(0));

		Shell_NotifyIcon(NIM_ADD,&nf);
	}
	else // Delete icon
	{
		Shell_NotifyIcon(NIM_DELETE,&nf);
	}	
}
