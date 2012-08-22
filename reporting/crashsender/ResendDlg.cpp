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

#include "stdafx.h"
#include "resource.h"
#include "CrashInfoReader.h"
#include "ResendDlg.h"
#include "Utility.h"
#include "strconv.h"
#include "DetailDlg.h"
#include "ErrorReportSender.h"


LRESULT CActionProgressDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{   
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

	// Determine window mirroring settings.
	CString sRTL = pSender->GetLangStr(_T("Settings"), _T("RTLReading"));
    if(sRTL.CompareNoCase(_T("1"))==0)
    {
        Utility::SetLayoutRTL(m_hWnd);
    }

	// The dialog will be resizable, so init resize map now.
    DlgResize_Init(false);

	// Init "current action" static
    m_statCurAction = GetDlgItem(IDC_CURRENTACTION);
    m_statCurAction.SetWindowText(
		pSender->GetLangStr(_T("ResendDlg"), _T("CurrentAction")));

	// Init "description" static
    m_statActionDesc = GetDlgItem(IDC_ACTIONDESC);

	// Init progress bar.
    m_prgProgress = GetDlgItem(IDC_PROGRESS);
    m_prgProgress.SetRange(0, 100);

    return 0;
}

LRESULT CActionProgressDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    return 0;
}


BOOL CResendDlg::PreTranslateMessage(MSG* pMsg)
{
    return CWindow::IsDialogMessage(pMsg);
}

LRESULT CResendDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{ 
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

	// Determine window mirroring settings.
	CString sRTL = pSender->GetLangStr(_T("Settings"), _T("RTLReading"));
    if(sRTL.CompareNoCase(_T("1"))==0)
    {
        Utility::SetLayoutRTL(m_hWnd);
    }

	// Init flags.
    m_bSendingNow = FALSE;
    m_bCancelled = FALSE;
    m_MailClientConfirm = NOT_CONFIRMED_YET;

	// Set dialog caption
    CString sTitle;
	sTitle.Format(pSender->GetLangStr(_T("ResendDlg"), _T("DlgCaption")), 
		pSender->GetCrashInfo()->m_sAppName);
    SetWindowText(sTitle);

    // center the dialog on the screen
    CenterWindow();

    // Set window icon
	HICON hIcon = pSender->GetCrashInfo()->GetCustomIcon();
    if(!hIcon)
        hIcon = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
    SetIcon(hIcon, 0);

    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);

	// Set up controls
    m_statText = GetDlgItem(IDC_TEXT);
	m_statText.SetWindowText(pSender->GetLangStr(_T("ResendDlg"), _T("ClickForDetails")));

    m_statSize = GetDlgItem(IDC_SELSIZE);
	m_statSize.SetWindowText(pSender->GetLangStr(_T("ResendDlg"), _T("SelectedSize")));

    m_btnSendNow = GetDlgItem(IDOK);
	m_btnSendNow.SetWindowText(pSender->GetLangStr(_T("ResendDlg"), _T("SendNow")));

    m_btnOtherActions = GetDlgItem(IDC_OTHERACTIONS);
	m_btnOtherActions.SetWindowText(pSender->GetLangStr(_T("MainDlg"), _T("OtherActions")));  

    m_btnShowLog = GetDlgItem(IDC_SHOWLOG);
	m_btnShowLog.SetWindowText(pSender->GetLangStr(_T("ResendDlg"), _T("ShowLog")));
    m_btnShowLog.ShowWindow(SW_HIDE);

    // Init list control
    m_listReportsSort.SubclassWindow(GetDlgItem(IDC_LIST));  
    m_listReports.SubclassWindow(m_listReportsSort.m_hWnd);
	m_listReports.InsertColumn(0, pSender->GetLangStr(_T("ResendDlg"), _T("ColumnCreationDate")), LVCFMT_LEFT, 170);
	m_listReports.InsertColumn(1, pSender->GetLangStr(_T("ResendDlg"), _T("ColumnSize")), LVCFMT_RIGHT, 90);
	m_listReports.InsertColumn(2, pSender->GetLangStr(_T("ResendDlg"), _T("ColumnStatus")), LVCFMT_LEFT, 170);
    m_listReports.ModifyStyleEx(0, LVS_EX_FULLROWSELECT);
    m_listReportsSort.SetSortColumn(0); // Sort by creation date

	// Add items to the list.
    int i;
	for(i=0; i<pSender->GetCrashInfo()->GetReportCount(); i++)
    {
        ErrorReportInfo& eri = pSender->GetCrashInfo()->GetReport(i);

        SYSTEMTIME st;
        Utility::UTC2SystemTime(eri.m_sSystemTimeUTC, st);
        CString sCreationDate;
        sCreationDate.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), 
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        int nItem = m_listReports.InsertItem(i, sCreationDate);
        m_listReports.SetItemData(nItem, i);

        CString sTotalSize = Utility::FileSizeToStr(eri.m_uTotalSize);

        m_listReports.SetItemText(nItem, 1, sTotalSize);

        m_listReports.SetCheckState(nItem, eri.m_bSelected);
    }

    UpdateSelectionSize();

    m_statConsent = GetDlgItem(IDC_CONSENT);

	// Create font used for the static
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = 11;
    lf.lfWeight = FW_NORMAL;
    lf.lfQuality = ANTIALIASED_QUALITY;
    _TCSCPY_S(lf.lfFaceName, 32, _T("Tahoma"));
    CFontHandle hConsentFont;
    hConsentFont.CreateFontIndirect(&lf);
    m_statConsent.SetFont(hConsentFont);

	if(pSender->GetCrashInfo()->m_sPrivacyPolicyURL.IsEmpty())
    	m_statConsent.SetWindowText(pSender->GetLangStr(_T("ResendDlg"), _T("MyConsent2")));
    else
    	m_statConsent.SetWindowText(pSender->GetLangStr(_T("ResendDlg"), _T("MyConsent")));
    
	// Init Privacy Policy link
    m_linkPrivacyPolicy.SubclassWindow(GetDlgItem(IDC_PRIVACYPOLICY));
	m_linkPrivacyPolicy.SetHyperLink(pSender->GetCrashInfo()->m_sPrivacyPolicyURL);
	m_linkPrivacyPolicy.SetLabel(pSender->GetLangStr(_T("MainDlg"), _T("PrivacyPolicy")));
    m_linkPrivacyPolicy.ShowWindow(pSender->GetCrashInfo()->m_sPrivacyPolicyURL.IsEmpty()?SW_HIDE:SW_SHOW);

	// Init progress bar
    m_dlgProgress.Create(m_hWnd);
    m_dlgProgress.ShowWindow(SW_HIDE);

	// Init child dialog
    m_dlgActionProgress.m_pParent = this;
    m_dlgActionProgress.Create(m_hWnd);
    m_dlgActionProgress.SetWindowLong(GWL_ID, IDD_PROGRESSMULTI); 

	// Position child dialog.
    CRect rc;
    m_listReports.GetWindowRect(&rc);
    ScreenToClient(&rc);
    m_dlgActionProgress.SetWindowPos(HWND_TOP, rc.left, rc.bottom, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Init resize map.
    DlgResize_Init();
	
	// Init handle to log file
    m_fileLog = NULL;

	// By default, we will exit if user closes the dialog.
    m_ActionOnClose = EXIT;

	// Show balloon in 3 seconds.
    m_nTick = 0;
    SetTimer(0, 3000);
    
    return TRUE;
}

LRESULT CResendDlg::OnTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	// This method is called when user clicks the tray icon.

	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

	// If user clicks or right-clicks the tray icon or clicks the balloon,
	// then show the dialog.
    if(LOWORD(lParam)==WM_LBUTTONDOWN || 
        LOWORD(lParam)==WM_LBUTTONDBLCLK ||
        LOWORD(lParam)==NIN_BALLOONUSERCLICK)
    {
        KillTimer(0);
        ShowWindow(SW_SHOW);
    }

	// If user right-clicks the icon, display context menu.
    if(LOWORD(lParam)==WM_RBUTTONDOWN)
    {
        CPoint pt;
        GetCursorPos(&pt);
        CMenu menu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_POPUPMENU));
        CMenu submenu = menu.GetSubMenu(2);

        strconv_t strconv;
		CString sShow = pSender->GetLangStr(_T("ResendDlg"), _T("PopupShow"));
		CString sExit = pSender->GetLangStr(_T("ResendDlg"), _T("PopupExit"));

        MENUITEMINFO mii;
        memset(&mii, 0, sizeof(MENUITEMINFO));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_STRING;

        mii.dwTypeData = sShow.GetBuffer(0);  
        submenu.SetMenuItemInfo(ID_MENU3_SHOW, FALSE, &mii);

        mii.dwTypeData = sExit.GetBuffer(0);  
        submenu.SetMenuItemInfo(ID_MENU3_EXIT, FALSE, &mii);

        submenu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
    }

    return 0;
}

void CResendDlg::CloseDialog(int nVal)
{
	// Close the dialog and exit message loop.
    DestroyWindow();
    AddTrayIcon(FALSE);

    Utility::RecycleFile(m_sLogFile, true);

    ::PostQuitMessage(nVal);
}

LRESULT CResendDlg::OnPopupShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// User chosen "Show" item from the popup menu.
	// Show the dialog.
    KillTimer(0);
    ShowWindow(SW_SHOW);
    return 0;
}

LRESULT CResendDlg::OnPopupExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// User chosen "Exit" item from the popup menu.
	// Exit the app.
    KillTimer(0);
    CloseDialog(0);

    return 0;
}

LRESULT CResendDlg::OnListItemChanging(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{  
	// User clicks a list item's checkbox.
	// Depending on current state, we should
	// either allow or prevent changing the item
    NMLISTVIEW* pnmlv = (NMLISTVIEW *)pnmh;
    if(pnmlv->iItem>=0 && (pnmlv->uChanged&LVIF_STATE) && 
        ((pnmlv->uNewState&LVIS_STATEIMAGEMASK)!=(pnmlv->uOldState&LVIS_STATEIMAGEMASK)))
    {
		// If we are currently send error reports, than prevent 
		// modifying check box state.
        if(m_bSendingNow)
            return TRUE;    
    }

	// Allow modifying item state
    return 0;
}

LRESULT CResendDlg::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{  
	// User clicks a list item's checkbox.
	// We need to update total size of selected crash reports.
    NMLISTVIEW* pnmlv = (NMLISTVIEW *)pnmh;
    if(pnmlv->iItem>=0 && (pnmlv->uChanged&LVIF_STATE) && 
        ((pnmlv->uNewState&LVIS_STATEIMAGEMASK)!=(pnmlv->uOldState&LVIS_STATEIMAGEMASK)))
    {    
        UpdateSelectionSize();
    }
    return 0;
}

LRESULT CResendDlg::OnListDblClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	// User double-clicks a list item.
	// We need to open details dialog for the selected crash report.

    if(m_bSendingNow)
        return 0; // Do nothing if in progress of sending

    NMITEMACTIVATE* pia = (NMITEMACTIVATE*)pnmh;
    if(pia->iItem>=0)
    {
        int nReport = (int)m_listReports.GetItemData(pia->iItem);

        m_dlgProgress.Start(FALSE, FALSE);

        CDetailDlg dlg;
        dlg.m_nCurReport = nReport;
        dlg.DoModal(m_hWnd);

        m_dlgProgress.Stop();
    }
    return 0;
}

LRESULT CResendDlg::OnListRClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

	CPoint pt;
    GetCursorPos(&pt);
    CMenu menu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_POPUPMENU));  
    CMenu submenu = menu.GetSubMenu(5);

    strconv_t strconv;
	CString sSelectAll = pSender->GetLangStr(_T("ResendDlg"), _T("SelectAll"));
	CString sDeselectAll = pSender->GetLangStr(_T("ResendDlg"), _T("DeselectAll"));
	CString sDeleteSelected = pSender->GetLangStr(_T("ResendDlg"), _T("DeleteSelected"));
	CString sDeleteAll = pSender->GetLangStr(_T("ResendDlg"), _T("DeleteAll"));

    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING;

    mii.dwTypeData = sSelectAll.GetBuffer(0);  
    submenu.SetMenuItemInfo(ID_MENU6_SELECTALL, FALSE, &mii);

    mii.dwTypeData = sDeselectAll.GetBuffer(0);  
    submenu.SetMenuItemInfo(ID_MENU6_DESELECTALL, FALSE, &mii);

	mii.dwTypeData = sDeleteSelected.GetBuffer(0);  
    submenu.SetMenuItemInfo(ID_MENU6_DELETESELECTED, FALSE, &mii);

	mii.dwTypeData = sDeleteAll.GetBuffer(0);  
    submenu.SetMenuItemInfo(ID_MENU6_DELETEALL, FALSE, &mii);

	// Get count of checked list items
	int nItems = 0;
	int nChecked = 0;
	int i;
    for(i=0; i<m_listReports.GetItemCount(); i++)
    {
		nItems++;

		// If list item checked
		if(m_listReports.GetCheckState(i))
			nChecked++;
	}
	
	submenu.EnableMenuItem(ID_MENU6_SELECTALL, nItems>0?MF_ENABLED:MF_DISABLED);
	submenu.EnableMenuItem(ID_MENU6_DESELECTALL, nItems>0?MF_ENABLED:MF_DISABLED);	
	submenu.EnableMenuItem(ID_MENU6_DELETESELECTED, nChecked>0?MF_ENABLED:MF_DISABLED);
	submenu.EnableMenuItem(ID_MENU6_DELETEALL, nItems>0?MF_ENABLED:MF_DISABLED);

	submenu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
    return 0;

	return 0;
}

LRESULT CResendDlg::OnPopupSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i;
    for(i=0; i<m_listReports.GetItemCount(); i++)
    {
		m_listReports.SetCheckState(i, TRUE);
    }

	UpdateSelectionSize();
	return 0;
}

LRESULT CResendDlg::OnPopupDeselectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int i;
    for(i=0; i<m_listReports.GetItemCount(); i++)
    {
		m_listReports.SetCheckState(i, FALSE);
    }

	UpdateSelectionSize();	
	return 0;
}

LRESULT CResendDlg::OnPopupDeleteSelected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

	// Walk through error reports
	int i;
    for(i=0; i<m_listReports.GetItemCount(); i++)
    {
		// If list item checked
		if(m_listReports.GetCheckState(i))
		{
			// Get error report index from item data
			int nReport = (int)m_listReports.GetItemData(i);

			// Delete report
			pSender->GetCrashInfo()->DeleteReport(nReport);   			
		}
    }

	// Delete selected list items
	BOOL bFound = TRUE;
	while(bFound)
	{
		bFound = FALSE;

		int i;
		for(i=0; i<m_listReports.GetItemCount(); i++)
		{
			// If list item checked
			if(m_listReports.GetCheckState(i))
			{
				bFound = TRUE;
				m_listReports.DeleteItem(i);
				break;
			}
		}
	}

	UpdateSelectionSize();	
	return 0;
}

LRESULT CResendDlg::OnPopupDeleteAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Delete all error reports
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();
	pSender->GetCrashInfo()->DeleteAllReports();               		
    
	// Delete all list items
	m_listReports.DeleteAllItems();

	UpdateSelectionSize();	
	return 0;
}

LRESULT CResendDlg::OnSendNow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// User clicked "Send Now"/"Cancel" button.

	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

    if(!m_bSendingNow) // "Send Now" clicked.
    {
        int i;
        for(i=0; i<m_listReports.GetItemCount(); i++)
        {
            BOOL bSelected = m_listReports.GetCheckState(i);
            if(bSelected)
            {
                int nReport = (int)m_listReports.GetItemData(i);        
				if(pSender->GetCrashInfo()->GetReport(nReport).m_DeliveryStatus == PENDING)
                {
                    m_listReports.SetItemText(i, 2, 
						Utility::GetINIString(pSender->GetCrashInfo()->m_sLangFileName, _T("ResendDlg"), _T("StatusPending")));
                }
            }      
        }

        m_bSendingNow = TRUE;
        m_bCancelled = FALSE;
        m_bErrors = FALSE;
        m_ActionOnClose = HIDE;

		m_statText.SetWindowText(pSender->GetLangStr(_T("ResendDlg"), _T("DeliveryingReports")));

        m_statSize.ShowWindow(SW_HIDE);
        m_statConsent.ShowWindow(SW_HIDE);
        m_linkPrivacyPolicy.ShowWindow(SW_HIDE);  
        m_btnOtherActions.ShowWindow(SW_HIDE);
        m_btnShowLog.ShowWindow(SW_HIDE);
        m_dlgActionProgress.ShowWindow(SW_SHOW);  
		m_btnSendNow.SetWindowText(pSender->GetLangStr(_T("ProgressDlg"), _T("Cancel")));

        SetTimer(1, 250); // Update this dialog every 250 ms
        SetTimer(2, 3000); // Hide this dialog in 3 sec.

        // Open log file
        Utility::RecycleFile(m_sLogFile, true);
        CString sCurTime;
        Utility::GetSystemTimeUTC(sCurTime);
        sCurTime.Replace(':', '-');
        m_sLogFile.Format(_T("%s\\CrashRpt-Log-%s.txt"), 
			pSender->GetCrashInfo()->m_sUnsentCrashReportsFolder, sCurTime);
#if _MSC_VER<1400
        m_fileLog = _tfopen(m_sLogFile, _T("wt"));
#else
        _tfopen_s(&m_fileLog, m_sLogFile.GetBuffer(0), _T("wt"));
#endif
        fprintf(m_fileLog, "%c%c%c", 0xEF, 0xBB, 0xBF); // UTF-8 signature

        SendNextReport();
    }
    else // "Cancel" clicked
    {
        m_bCancelled = TRUE;
        m_btnSendNow.EnableWindow(0);
        KillTimer(2); // Don't hide window 
        pSender->Cancel();    
    }

    return 0;
}

LRESULT CResendDlg::OnShowLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    HINSTANCE hInst = ShellExecute(m_hWnd, _T("open"), m_sLogFile, NULL, NULL, SW_SHOW);
    ATLASSERT((int)hInst>32);
    hInst;
    return 0;
}

int CResendDlg::FindListItemByReportIndex(int nReport)
{
    int i;
    for(i=0; i<m_listReports.GetItemCount(); i++)
    {
        int nData = (int)m_listReports.GetItemData(i);
        if(nData==nReport)
            return i;
    }

    return -1;
}

BOOL CResendDlg::SendNextReport()
{  
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

    if(!m_bSendingNow)
        return FALSE;

    if(!m_bCancelled)
    {
        int i;
        for(i=0; i<m_listReports.GetItemCount(); i++)
        {
            BOOL bSelected = m_listReports.GetCheckState(i);    
            if(bSelected)
            {
                int nReport = (int)m_listReports.GetItemData(i);

				DELIVERY_STATUS status = pSender->GetCrashInfo()->GetReport(nReport).m_DeliveryStatus;
                if(status!=PENDING)
                    continue;

                m_nCurReport = nReport;
				m_listReports.SetItemText(i, 2, pSender->GetLangStr(_T("ResendDlg"), _T("StatusInProgress")));
                m_listReports.EnsureVisible(i, TRUE);
                m_listReports.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);

                pSender->SetCurReport(nReport);
                pSender->DoWork(COMPRESS_REPORT|SEND_REPORT);      
                return TRUE;
            }
        }
    }

    if(m_fileLog!=NULL)
        fclose(m_fileLog);
    m_fileLog = NULL;

    m_bSendingNow = FALSE;
    m_ActionOnClose = EXIT;
    KillTimer(1);
    KillTimer(2);

	// Update control states.
    m_btnSendNow.EnableWindow(1);
    m_btnOtherActions.ShowWindow(SW_SHOW);
    m_btnShowLog.ShowWindow(SW_SHOW);  
    m_statSize.ShowWindow(SW_SHOW);
    m_statConsent.ShowWindow(SW_SHOW);
    m_linkPrivacyPolicy.ShowWindow(SW_SHOW);  
    m_btnOtherActions.ShowWindow(SW_SHOW);
    m_dlgActionProgress.ShowWindow(SW_HIDE);  
    m_btnSendNow.SetWindowText(pSender->GetLangStr(_T("ResendDlg"), _T("SendNow")));
	m_statText.SetWindowText(pSender->GetLangStr(_T("ResendDlg"), _T("ClickForDetails")));

	// Update Status column of the list view
    int i;
    for(i=0; i<m_listReports.GetItemCount(); i++)
    {
        BOOL bSelected = m_listReports.GetCheckState(i);  
        if(bSelected)
        {
            int nReport = (int)m_listReports.GetItemData(i);

			DELIVERY_STATUS status = pSender->GetCrashInfo()->GetReport(nReport).m_DeliveryStatus;
            if(status==PENDING)
            {
                m_listReports.SetItemText(i, 2, _T(""));
            }
            else if(status!=DELIVERED)
            {
				pSender->GetCrashInfo()->GetReport(nReport).m_DeliveryStatus = PENDING;
            }
        }
    }


    DWORD dwFlags = 0;
	CString sRTL = pSender->GetLangStr(_T("Settings"), _T("RTLReading"));
    if(sRTL.CompareNoCase(_T("1"))==0)
        dwFlags = MB_RTLREADING;

    CString sCaption;
	sCaption.Format(pSender->GetLangStr(_T("ResendDlg"), _T("DlgCaption")), 
		pSender->GetCrashInfo()->m_sAppName);

    if(m_bErrors && !pSender->GetCrashInfo()->m_bSilentMode)
    {
        ShowWindow(SW_SHOW);
        SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
        SetFocus();  
        RedrawWindow(0, 0, RDW_ERASE|RDW_FRAME|RDW_INVALIDATE);
        m_listReports.RedrawWindow(0, 0, RDW_ERASE|RDW_FRAME|RDW_INVALIDATE);  

		MessageBox(pSender->GetLangStr(_T("ResendDlg"), _T("DeliveryFailed")), 
            sCaption, 
            MB_OK|MB_ICONINFORMATION|dwFlags);  

    }
    else if(!m_bErrors)
    {
        if(IsWindowVisible())
        {
            MessageBox(pSender->GetLangStr(_T("ResendDlg"), _T("DeliverySucceeded")), 
                sCaption, 
                MB_OK|MB_ICONINFORMATION|dwFlags);
        }

        SendMessage(WM_CLOSE);
    }

    return FALSE;
}

LRESULT CResendDlg::OnOtherActions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

    CPoint pt;
    GetCursorPos(&pt);
    CMenu menu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_POPUPMENU));  
    CMenu submenu = menu.GetSubMenu(3);

    strconv_t strconv;
	CString sRemindLater = pSender->GetLangStr(_T("ResendDlg"), _T("PopupRemindLater"));
	CString sNeverRemind = pSender->GetLangStr(_T("ResendDlg"), _T("PopupNeverRemind"));

    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING;

    mii.dwTypeData = sRemindLater.GetBuffer(0);  
    submenu.SetMenuItemInfo(ID_MENU4_REMINDLATER, FALSE, &mii);

    mii.dwTypeData = sNeverRemind.GetBuffer(0);  
    submenu.SetMenuItemInfo(ID_MENU4_NEVERREMIND, FALSE, &mii);

    submenu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
    return 0;
}

LRESULT CResendDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

    if(m_ActionOnClose==EXIT)
    {        
		pSender->GetCrashInfo()->SetLastRemindDateToday();
        CloseDialog(0);
        return 0;
    }
    else if(m_ActionOnClose==HIDE)
    {
        AnimateWindow(m_hWnd, 200, AW_HIDE|AW_BLEND); 	  
        return 0;
    }

    return 0;
}

LRESULT CResendDlg::OnRemindLater(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{  
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();
    pSender->GetCrashInfo()->SetLastRemindDateToday();
    pSender->GetCrashInfo()->SetRemindPolicy(REMIND_LATER);

    KillTimer(0);
    CloseDialog(0);
    return 0;
}

LRESULT CResendDlg::OnNeverRemind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();
	pSender->GetCrashInfo()->SetLastRemindDateToday();
	pSender->GetCrashInfo()->SetRemindPolicy(NEVER_REMIND);

    KillTimer(0);
    CloseDialog(0);
    return 0;
}

void CResendDlg::AddTrayIcon(BOOL bAdd)
{
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();
    
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
		sTip.Format(pSender->GetLangStr(_T("ResendDlg"), _T("DlgCaption")), pSender->GetCrashInfo()->m_sAppName);
        _TCSCPY_S(nf.szTip, 64, sTip);
		HICON hIcon = pSender->GetCrashInfo()->GetCustomIcon();
        if(!hIcon)
            hIcon = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));    
        nf.hIcon = hIcon;


        CString sInfo;
		sInfo.Format(pSender->GetLangStr(_T("ResendDlg"), _T("BalloonText")), 
			pSender->GetCrashInfo()->m_sAppName, pSender->GetCrashInfo()->m_sAppName);
        _TCSCPY_S(nf.szInfo, 200, sInfo.GetBuffer(0));

        CString sInfoTitle;
		sInfoTitle.Format(pSender->GetLangStr(_T("ResendDlg"), _T("BalloonCaption")), 
			pSender->GetCrashInfo()->m_sAppName);
        _TCSCPY_S(nf.szInfoTitle, 64, sInfoTitle.GetBuffer(0));

        Shell_NotifyIcon(NIM_ADD,&nf);
    }
    else // Delete icon
    {
        Shell_NotifyIcon(NIM_DELETE,&nf);
    }	
}

void CResendDlg::UpdateSelectionSize()
{
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();
    int nItemsSelected = 0;
    ULONG64 uSelectedFilesSize = 0;

    int i;
    for(i=0; i<m_listReports.GetItemCount(); i++)
    {
        if(m_listReports.GetCheckState(i))
        {
            int nReport = (int)m_listReports.GetItemData(i);

            nItemsSelected++;
			uSelectedFilesSize += pSender->GetCrashInfo()->GetReport(nReport).m_uTotalSize;
        }
    }

    CString sText;
	sText.Format(pSender->GetLangStr(_T("ResendDlg"), _T("SelectedSize")), nItemsSelected, 
        Utility::FileSizeToStr(uSelectedFilesSize).GetBuffer(0));
    m_statSize.SetWindowText(sText);

    m_btnSendNow.EnableWindow(nItemsSelected>0?TRUE:FALSE);
}

LRESULT CResendDlg::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{ 
    if(wParam==0)
        DoBalloonTimer();
    else if(wParam==1)
        DoProgressTimer();
    else if(wParam==2)
        DoHideWindowTimer();

    return 0;
}

void CResendDlg::DoBalloonTimer()
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
}

void CResendDlg::DoProgressTimer()
{   
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();

    // Get current progress
    int nProgressPct = 0;
    std::vector<CString> messages;
    pSender->GetStatus(nProgressPct, messages);

    // Update progress bar
    m_dlgActionProgress.m_prgProgress.SetPos(nProgressPct);

    int nCurItem = FindListItemByReportIndex(m_nCurReport);

    unsigned i;
    for(i=0; i<messages.size(); i++)
    { 
        m_dlgActionProgress.m_statActionDesc.SetWindowText(messages[i]);

        if(m_fileLog)
        {
            strconv_t strconv;
            LPCSTR szLine = strconv.t2utf8(messages[i]);
            fprintf(m_fileLog, szLine);
            fprintf(m_fileLog, "\n");
        }


        if(messages[i].CompareNoCase(_T("[status_success]"))==0)
        {
            m_listReports.SetItemText(nCurItem, 2, 
				pSender->GetLangStr(_T("ResendDlg"), _T("StatusSucceeded")));

            SendNextReport();
        }
        else if(messages[i].CompareNoCase(_T("[status_failed]"))==0)
        {
            m_bErrors = TRUE;
            m_listReports.SetItemText(nCurItem, 2, 
				pSender->GetLangStr(_T("ResendDlg"), _T("StatusFailed")));
            SendNextReport();
        }
        else if(messages[i].CompareNoCase(_T("[exit_silently]"))==0)
        {
            m_bErrors = TRUE;
            SendNextReport();
        }
        else if(messages[i].CompareNoCase(_T("[cancelled_by_user]"))==0)    
        { 
            /*m_bErrors = TRUE;
            m_listReports.SetItemText(nCurItem, 2, 
            Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("StatusFailed")));

            SendNextReport();      */
        }        
        else if(messages[i].CompareNoCase(_T("[confirm_launch_email_client]"))==0)
        {       
            KillTimer(0);        
			if(!pSender->GetCrashInfo()->m_bSilentMode)
            {
                if(m_MailClientConfirm==NOT_CONFIRMED_YET)
                {
                    BOOL bVisible = IsWindowVisible();
                    ShowWindow(SW_SHOW);
                    SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
                    SetFocus();  
                    RedrawWindow(0, 0, RDW_ERASE|RDW_FRAME|RDW_INVALIDATE);
                    m_listReports.RedrawWindow(0, 0, RDW_ERASE|RDW_FRAME|RDW_INVALIDATE);  

                    DWORD dwFlags = 0;
					CString sRTL = Utility::GetINIString(pSender->GetCrashInfo()->m_sLangFileName, _T("Settings"), _T("RTLReading"));
                    if(sRTL.CompareNoCase(_T("1"))==0)
                        dwFlags = MB_RTLREADING;

                    CString sMailClientName;        
                    CMailMsg::DetectMailClient(sMailClientName);
                    CString msg;
					msg.Format(pSender->GetLangStr(_T("ProgressDlg"), _T("ConfirmLaunchEmailClient")), sMailClientName);

                    CString sCaption = pSender->GetLangStr(_T("ProgressDlg"), _T("DlgCaption"));
                    CString sTitle;
					sTitle.Format(sCaption, pSender->GetCrashInfo()->m_sAppName);
                    INT_PTR result = MessageBox(msg, 
                        sTitle,
                        MB_OKCANCEL|MB_ICONQUESTION|dwFlags);

                    RedrawWindow();

                    if(result==IDOK)
                        m_MailClientConfirm = ALLOWED;
                    else
                        m_MailClientConfirm = NOT_ALLOWED;

                    pSender->FeedbackReady(result==IDOK?0:1);       
                    ShowWindow(bVisible?SW_SHOW:SW_HIDE);
                }
                else
                {
                    pSender->FeedbackReady(m_MailClientConfirm==ALLOWED?0:1);       
                }
            }      
            else
            { 
                // In silent mode, assume user provides his/her consent
                pSender->FeedbackReady(0);       
            }        
        }    
    }
}

void CResendDlg::DoHideWindowTimer()
{
	CErrorReportSender* pSender = CErrorReportSender::GetInstance();
	if(!pSender->GetCrashInfo()->m_bSilentMode)
    {
        AnimateWindow(m_hWnd, 200, AW_HIDE|AW_BLEND);         
    }
    KillTimer(2);
}