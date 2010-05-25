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

// File: ErrorReportDlg.h
// Description: Error Report Dialog.
// Authors: mikecarruth, zexspectrum
// Date: 

#pragma once

#include <atlctrls.h>
#include <atlctrlx.h>
#include "MailMsg.h"
#include "DetailDlg.h"
#include "ProgressDlg.h"

#define WM_TRAYICON (WM_USER+128)

class CSequenceLayout
{
  struct ItemInfo
  {
    BOOL m_bSecondary;
    HWND m_hWnd;
    RECT m_rcInitial;
  };

public:
  
  CSequenceLayout()
  {
    m_hWndContainer = NULL;
  }

  void SetContainerWnd(HWND hWnd)
  {
    m_hWndContainer = hWnd;
    GetClientRect(m_hWndContainer, &m_rcContainer);
  }

  void Insert(HWND hWnd, BOOL bSecondary=FALSE)
  {
    RECT rc;
    GetWindowRect(hWnd, &rc);
    MapWindowPoints(0, GetParent(hWnd), (LPPOINT)&rc, 2);
    ItemInfo ii;
    ii.m_hWnd = hWnd;
    ii.m_bSecondary = bSecondary;
    ii.m_rcInitial = rc;
    m_aItems.push_back(ii);
  }
  
  void Update()
  { 
    int nDeltaY = 0;
    
    int i;
    for(i=0; i<(int)m_aItems.size(); i++)
    {      
      ItemInfo ii = m_aItems[i];
      if(GetWindowLong(ii.m_hWnd, GWL_STYLE)&WS_VISIBLE)
      {        
        CWindow wnd = ii.m_hWnd;
        CRect rc = ii.m_rcInitial;
        rc.OffsetRect(0, nDeltaY);
        wnd.MoveWindow(&rc);
      }
      else 
      {
        if(ii.m_bSecondary)
          continue;

        int nNext = GetNextPrimaryItem(i+1);
        if(nNext==-1)
          continue;

        ItemInfo nextItem = m_aItems[nNext];
        nDeltaY -= nextItem.m_rcInitial.top - ii.m_rcInitial.top;
      }    
    }

    if(m_hWndContainer!=NULL)
    {
      CWindow wnd = m_hWndContainer;
      wnd.ResizeClient(m_rcContainer.Width(), m_rcContainer.Height()+nDeltaY);
    }
  }

private:

  int GetNextPrimaryItem(int nStart)
  {
    int i;
    for(i=nStart; i<(int)m_aItems.size(); i++)
    {
      if(m_aItems[i].m_bSecondary==FALSE)
        return i;
    }

    return -1;
  }

  HWND m_hWndContainer;
  CRect m_rcContainer;
  std::vector<ItemInfo> m_aItems;
};

class CErrorReportDlg : 
  public CDialogImpl<CErrorReportDlg>, 
  public CUpdateUI<CErrorReportDlg>,
	public CMessageFilter, 
  public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

  CStatic m_statIcon; 
  CStatic m_statSubHeader;
  CHyperLink  m_link;           
  CHyperLink m_linkMoreInfo;
  CStatic m_statIndent;
  CStatic m_statEmail;
  CEdit m_editEmail;
  CStatic m_statDesc;  
  CEdit m_editDesc;
  CButton m_chkRestart;
  CStatic m_statConsent;
  CHyperLink  m_linkPrivacyPolicy;           
  CStatic m_statHorzLine;
  CStatic m_statCrashRpt;
  CButton m_btnOk;
  CButton m_btnCancel;    
  CFont m_HeadingFont;
  CIcon m_HeadingIcon;
  CSequenceLayout m_Layout;

  CProgressDlg m_dlgProgress;
  HANDLE m_hSenderThread;
  
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CErrorReportDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CErrorReportDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)    
    MESSAGE_HANDLER(WM_COMPLETECOLLECT, OnCompleteCollectCrashInfo)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)

    COMMAND_ID_HANDLER(IDC_LINK, OnLinkClick)
    COMMAND_ID_HANDLER(IDC_MOREINFO, OnMoreInfoClick)    
    COMMAND_ID_HANDLER(IDC_RESTART, OnRestartClick)    
		COMMAND_ID_HANDLER(IDOK, OnSend)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)        
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnCompleteCollectCrashInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);	
  LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);	
  LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);	

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnLinkClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnMoreInfoClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  
  LRESULT OnSend(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnRestartClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);
  void ShowMoreInfo(BOOL bShow);
  int CreateTrayIcon(bool bCreate, HWND hWndParent);
  
};
