/************************************************************************************* 
This file is a part of CrashRpt library.

Copyright (c) 2003, Michael Carruth
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
#include "stdafx.h"
#include "MailMsg.h"
#include "DetailDlg.h"
#include "ProgressDlg.h"

// This message is sent by the system to the Error Report dialog when user clicks the tray icon
#define WM_TRAYICON (WM_USER+128)

// class CSequenceLayout
// Implements vertical layout to position controls 
// on the dialog one below another. Some layout items 
// can be hidden/shown, and controls are automatically repositioned by the class.
// The layout class can also resize the container window to accomodate all the layout itmes.
class CSequenceLayout
{
	// Internal structure
    struct ItemInfo
    {
        BOOL m_bSecondary; // Is this item secondary?
        HWND m_hWnd;       // Handle to control's window.
        RECT m_rcInitial;  // Initial rectangle of the item within the parent dialog.
    };

public:

	// Constructor
    CSequenceLayout()
    {
        m_hWndContainer = NULL;
    }

	// Sets the container window for this layout
    void SetContainerWnd(HWND hWnd)
    {
        m_hWndContainer = hWnd;
        GetClientRect(m_hWndContainer, &m_rcContainer);
    }

	// Adds an item to layout
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

	// Updates item positions
    void Update()
    { 
        int nDeltaY = 0;

		// Walk through items
        int i;
        for(i=0; i<(int)m_aItems.size(); i++)
        {      
			// Get i-th item
            ItemInfo ii = m_aItems[i];
            if(GetWindowLong(ii.m_hWnd, GWL_STYLE)&WS_VISIBLE) // Skip invisible items
            {        
				// Position the item appropriately
                CWindow wnd = ii.m_hWnd;
                CRect rc = ii.m_rcInitial;
                rc.OffsetRect(0, nDeltaY);
                wnd.MoveWindow(&rc);
            }
            else 
            {
				// Skip secondary items
                if(ii.m_bSecondary)
                    continue;

				// Get the next primary item
                int nNext = GetNextPrimaryItem(i+1);
                if(nNext==-1)
                    continue;

				// Update vertical offset
                ItemInfo nextItem = m_aItems[nNext];
                nDeltaY -= nextItem.m_rcInitial.top - ii.m_rcInitial.top;
            }    
        }

		// Resize the container to accomodate all the layout items
        if(m_hWndContainer!=NULL)
        {
            CWindow wnd = m_hWndContainer;
            wnd.ResizeClient(m_rcContainer.Width(), m_rcContainer.Height()+nDeltaY);
        }
    }

private:

	// Returns the index of the next primary item
    int GetNextPrimaryItem(int nStart)
    {
		// Walk through layout items
        int i;
        for(i=nStart; i<(int)m_aItems.size(); i++)
        {
			// If this item is primary, return its index
            if(m_aItems[i].m_bSecondary==FALSE)
                return i;
        }

		// Return -1 to indicate the item not found
        return -1;
    }

	// Variables used internally
    HWND m_hWndContainer;           // Handle to the continer window.
    CRect m_rcContainer;            // Countainer window's rectangle.
    std::vector<ItemInfo> m_aItems; // The list of layout items.
};

// class CErrorReportDlg
// Implements Error Report dialog.
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
        COMMAND_HANDLER(IDC_EMAIL, EN_KILLFOCUS, OnEmailKillFocus)
        COMMAND_HANDLER(IDC_DESCRIPTION, EN_KILLFOCUS, OnEmailKillFocus)
        COMMAND_ID_HANDLER(IDOK, OnSend)
        COMMAND_ID_HANDLER(IDC_CANCEL, OnCancel)            
        COMMAND_ID_HANDLER(ID_MENU5_SENDREPORTLATER, OnPopupSendReportLater)            
        COMMAND_ID_HANDLER(ID_MENU5_CLOSETHEPROGRAM, OnPopupCloseTheProgram)            
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
    LRESULT OnEmailKillFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  
    LRESULT OnLinkClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnMoreInfoClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  
    LRESULT OnSend(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnRestartClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnPopupSendReportLater(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnPopupCloseTheProgram(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// This method is used to close this dialog with some return code.
    void CloseDialog(int nVal); 

	// This method hides or displays some input fields.
    void ShowMoreInfo(BOOL bShow);
	
	// This method creates or destroys the tray icon.
    int CreateTrayIcon(bool bCreate, HWND hWndParent);	
};
