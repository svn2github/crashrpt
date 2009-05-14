///////////////////////////////////////////////////////////////////////////////
//
//  Module: maindlg.h
//
//    Desc: Main crash report dialog, responsible for gathering additional
//          user information and allowing user to examine crash report.
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MAINDLG_H_
#define _MAINDLG_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Utility.h"
#include "DeadLink.h"
#include "detaildlg.h"
#include "aboutdlg.h"

//
// RTF load callback
//
DWORD CALLBACK LoadRTFString(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
   CString *sText = (CString*)dwCookie;
   LONG lLen = sText->GetLength();

   for (*pcb = 0; *pcb < cb && *pcb < lLen; (*pcb)++)
   {  
      pbBuff[*pcb] = sText->GetAt(*pcb);
   }

   return 0;
}


////////////////////////////// Class Definitions /////////////////////////////

// ===========================================================================
// CMainDlg
// 
// See the module comment at top of file.
//
class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

   CString     m_sEmail;         // Email: From
   CString     m_sDescription;   // Email: Body
   CDeadLink   m_link;           // Dead link
   TStrStrMap  *m_pUDFiles;      // Files <name,desc>

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDC_LINK, OnLinkClick)
      MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
		COMMAND_ID_HANDLER(IDOK, OnSend)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

   
   //-----------------------------------------------------------------------------
   // CMainDlg
   //
   // Loads RichEditCtrl library
   //
   CMainDlg() 
   {
      LoadLibrary(CRichEditCtrl::GetLibraryName());
   };

	
   //-----------------------------------------------------------------------------
   // OnInitDialog
   //
   // 
   //
   LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

	   // Add "About..." menu item to system menu.

	   // IDM_ABOUTBOX must be in the system command range.
      ATLASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX); 
      ATLASSERT(IDM_ABOUTBOX < 0xF000); 

      CMenu sysMenu;
      sysMenu.Attach(GetSystemMenu(FALSE));
      if (sysMenu.IsMenu())
      {
		   CString strAboutMenu;
		   strAboutMenu.LoadString(IDS_ABOUTBOX);
		   if (!strAboutMenu.IsEmpty())
		   {
            sysMenu.AppendMenu(MF_SEPARATOR);
			   sysMenu.AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		   }
	   }
      //
      // Set title using app name
      //
      SetWindowText(CUtility::getAppName());

      //
      // Use app icon
      //
      CStatic icon;
      icon.Attach(GetDlgItem(IDI_APPICON));
      icon.SetIcon(::LoadIcon((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME)));
      icon.Detach();

      //
      // Set failure heading
      //
      EDITSTREAM es;
      es.pfnCallback = LoadRTFString;

      CString sText;
      sText.Format(IDS_HEADER, CUtility::getAppName());
      es.dwCookie = (DWORD)&sText;

      CRichEditCtrl re;
      re.Attach(GetDlgItem(IDC_HEADING_TEXT));
      re.StreamIn(SF_RTF, es);
      re.Detach();

      //
      // Hook dead link
      //
      m_link.SubclassWindow(GetDlgItem(IDC_LINK));   

      return TRUE;
	}

	
   //-----------------------------------------------------------------------------
   // OnLinkClick
   //
   // Display details dialog instead of opening URL
   //
   LRESULT OnLinkClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
   {
      CDetailDlg dlg;
      dlg.m_pUDFiles = m_pUDFiles;
      dlg.DoModal();
      return 0;
   }

   //-----------------------------------------------------------------------------
   // OnSysCommand
   //
   // 
   //
   LRESULT OnSysCommand(UINT, WPARAM wParam, LPARAM , BOOL& bHandled)
   {
      bHandled = FALSE;

      if ((wParam & 0xFFF0) == IDM_ABOUTBOX)
      {
         CAboutDlg dlg;
         dlg.DoModal();
         bHandled = TRUE;
      }

      return 0;
   }

	
   //-----------------------------------------------------------------------------
   // OnSend
   //
   // Send handler, validates email address entered, if one, and returns.
   //
   LRESULT OnSend(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
      HWND     hWndEmail = GetDlgItem(IDC_EMAIL);
      HWND     hWndDesc = GetDlgItem(IDC_DESCRIPTION);
	   int      nEmailLen = ::GetWindowTextLength(hWndEmail);
      int      nDescLen = ::GetWindowTextLength(hWndDesc);

      LPTSTR lpStr = m_sEmail.GetBufferSetLength(nEmailLen);
      ::GetWindowText(hWndEmail, lpStr, nEmailLen);
      m_sEmail.ReleaseBuffer();

      lpStr = m_sDescription.GetBufferSetLength(nDescLen);
      ::GetWindowText(hWndDesc, lpStr, nDescLen);
      m_sDescription.ReleaseBuffer();

      //
      // If an email address was entered, verify that
      // it [1] contains a @ and [2] the last . comes
      // after the @.
      //
      if (m_sEmail.GetLength() &&
          (m_sEmail.Find(_T('@')) < 0 ||
           m_sEmail.ReverseFind(_T('.')) < m_sEmail.Find(_T('@'))))
      {
         // alert user
         TCHAR szBuf[256];
		   ::LoadString(_Module.GetResourceInstance(), IDS_INVALID_EMAIL, szBuf, 255);
         MessageBox(szBuf, CUtility::getAppName(), MB_OK);
         // select email
         ::SetFocus(hWndEmail);
      }
      else
      {
         EndDialog(wID);
      }

      return 0;
   }

   //-----------------------------------------------------------------------------
   // OnCancel
   //
   // 
   //
   LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
      EndDialog(wID);
		return 0;
   }

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif	// #ifndef _MAINDLG_H_
