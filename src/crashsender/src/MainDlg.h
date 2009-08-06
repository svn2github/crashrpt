// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlctrls.h>
#include <atlctrlx.h>
#include "MailMsg.h"
#include "DetailDlg.h"
#include "ProgressDlg.h"
#include "SenderThread.h"

#define WM_TRAYICON (WM_USER+128)

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

  CString     m_sAppName;
  CString     m_sAppVersion;
  CString     m_sImageName;
  CString     m_sEmailSubject;
  CString     m_sEmailFrom;     // Email: From  
  CString     m_sEmailTo;
  CString     m_sDescription;   // Email: Body
  CString     m_sZipName;
  CString     m_sUrl;
  UINT        m_uPriorities[3];

  TStrStrMap  m_pUDFiles;      // Files <name,desc>

  CStatic m_statIcon;  
  CStatic m_statEmail;
  CEdit m_editEmail;
  CStatic m_statDesc;
  CEdit m_editDesc;
  CButton m_btnOk;
  CButton m_btnCancel;
  CStatic m_statHorzLine;
  CStatic m_statCrashRpt; 
  CHyperLink  m_link;           
  CHyperLink m_linkMoreInfo;
  int m_nDeltaY;
  CFont m_HeadingFont;
  CIcon m_HeadingIcon;

  CProgressDlg m_dlgProgress;
  HANDLE m_hSenderThread;
  SenderThreadContext m_ctx;

  
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)    
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)

    COMMAND_ID_HANDLER(IDC_LINK, OnLinkClick)
    COMMAND_ID_HANDLER(IDC_MOREINFO, OnMoreInfoClick)    
		COMMAND_ID_HANDLER(IDOK, OnSend)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)    
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);	
  LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);	
  LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);	

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnLinkClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnMoreInfoClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnSend(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);
  void ShowMoreInfo(BOOL bShow);
  void AddUserInfoToCrashDescriptorXML(CString sEmail, CString sDesc);
  int CreateTrayIcon(bool bCreate, HWND hWndParent);
  
};
