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

#pragma once
#include "stdafx.h"
#include "resource.h"
#include "MailMsg.h"
#include "FilePreviewCtrl.h"

class CDetailDlg : 
	public CDialogImpl<CDetailDlg>,
    public CDialogResize<CDetailDlg>
{
public:
    enum { IDD = IDD_DETAILDLG };
	    
    BEGIN_DLGRESIZE_MAP(CProgressDlg)    
        DLGRESIZE_CONTROL(IDC_FILE_LIST, DLSZ_SIZE_X)
        DLGRESIZE_CONTROL(IDC_PREVIEW, DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_PRIVACYPOLICY, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDOK, DLSZ_CENTER_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EXPORT, DLSZ_MOVE_X|DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

    BEGIN_MSG_MAP(CDetailDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        NOTIFY_HANDLER(IDC_FILE_LIST, LVN_ITEMCHANGED, OnItemChanged)
        NOTIFY_HANDLER(IDC_FILE_LIST, NM_DBLCLK, OnItemDblClicked)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDCANCEL, OnOK)
        COMMAND_ID_HANDLER(IDC_EXPORT, OnExport)
        COMMAND_RANGE_HANDLER(ID_PREVIEW_AUTO, ID_PREVIEW_IMAGE, OnPreviewModeChanged)
        COMMAND_RANGE_HANDLER(ID_ENCODING_AUTO, ID_ENCODING_UTF16BE, OnTextEncodingChanged)
        NOTIFY_HANDLER(IDC_PREVIEW, NM_RCLICK, OnPreviewRClick)
		NOTIFY_HANDLER(IDC_FILE_LIST, NM_RCLICK, OnListRClick)	
		COMMAND_ID_HANDLER(ID_MENU7_OPEN, OnPopupOpen)
		COMMAND_ID_HANDLER(ID_MENU7_DELETESELECTEDFILE, OnPopupDeleteSelected)
		COMMAND_ID_HANDLER(ID_MENU7_ATTACHMOREFILES, OnPopupAddMoreFiles)

        CHAIN_MSG_MAP(CDialogResize<CDetailDlg>)
    END_MSG_MAP()

    // Handler prototypes (uncomment arguments if needed):
    //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  
    LRESULT OnExport(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnPreviewModeChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  
    LRESULT OnTextEncodingChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  
    LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
    LRESULT OnItemDblClicked(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
    LRESULT OnPreviewRClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnListRClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnPopupOpen(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  
	LRESULT OnPopupDeleteSelected(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  
	LRESULT OnPopupAddMoreFiles(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);  

	void FillFileItemList();

    void SelectItem(int iItem);

	CImageList  m_iconList;       // Shell icon list
    CListViewCtrl m_list;
    CHyperLink m_linkPrivacyPolicy;
    CButton m_btnClose;
    CButton m_btnExport;
    CStatic m_statPreview;     
    CFilePreviewCtrl m_filePreview; 
    PreviewMode m_previewMode;    // Current preview mode
    TextEncoding m_textEncoding;  // Current text encoding
    int m_nCurReport;             // Index of the error report currently being displayed

};


