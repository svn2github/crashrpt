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
#include "DetailDlg.h"
#include "Utility.h"
#include "CrashInfoReader.h"
#include "ErrorReportSender.h"
#include "strconv.h"

LRESULT CDetailDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  DlgResize_Init();

  CString sRTL = Utility::GetINIString(g_CrashInfo.m_sLangFileName, 
    _T("Settings"), _T("RTLReading"));
  if(sRTL.CompareNoCase(_T("1"))==0)
  {
    Utility::SetLayoutRTL(m_hWnd);
  }

  SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, 
    _T("DetailDlg"), _T("DlgCaption")));

  m_previewMode = PREVIEW_AUTO;
  m_filePreview.SubclassWindow(GetDlgItem(IDC_PREVIEW));
  m_filePreview.SetBytesPerLine(10);
  m_filePreview.SetEmptyMessage(Utility::GetINIString(g_CrashInfo.m_sLangFileName, 
    _T("DetailDlg"), _T("NoDataToDisplay")));

  m_linkPrivacyPolicy.SubclassWindow(GetDlgItem(IDC_PRIVACYPOLICY));
  m_linkPrivacyPolicy.SetHyperLink(g_CrashInfo.m_sPrivacyPolicyURL);
  m_linkPrivacyPolicy.SetLabel(Utility::GetINIString(g_CrashInfo.m_sLangFileName, 
    _T("DetailDlg"), _T("PrivacyPolicy")));

  if(!g_CrashInfo.m_sPrivacyPolicyURL.IsEmpty())
    m_linkPrivacyPolicy.ShowWindow(SW_SHOW);
  else
    m_linkPrivacyPolicy.ShowWindow(SW_HIDE);

  CStatic statHeader = GetDlgItem(IDC_HEADERTEXT);
  statHeader.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, 
    _T("DetailDlg"), _T("DoubleClickAnItem")));  

  m_list = GetDlgItem(IDC_FILE_LIST);
  m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

  m_list.InsertColumn(0, Utility::GetINIString(g_CrashInfo.m_sLangFileName, 
    _T("DetailDlg"), _T("FieldName")), LVCFMT_LEFT, 130);
  m_list.InsertColumn(1, Utility::GetINIString(g_CrashInfo.m_sLangFileName, 
    _T("DetailDlg"), _T("FieldDescription")), LVCFMT_LEFT, 110);
  m_list.InsertColumn(2, Utility::GetINIString(g_CrashInfo.m_sLangFileName, 
    _T("DetailDlg"), _T("FieldType")), LVCFMT_LEFT, 90);
  m_list.InsertColumn(3, Utility::GetINIString(g_CrashInfo.m_sLangFileName, 
    _T("DetailDlg"), _T("FieldSize")), LVCFMT_RIGHT, 60);

  m_iconList.Create(16, 16, ILC_COLOR32|ILC_MASK, 3, 1);
  m_list.SetImageList(m_iconList, LVSIL_SMALL);

  // Insert items
  WIN32_FIND_DATA   findFileData   = {0};
  HANDLE            hFind          = NULL;
  CString           sSize;
  
  std::map<CString, FileItem>::iterator p;
  unsigned i;
  for (i = 0, p = g_CrashInfo.GetReport(m_nCurReport).m_FileItems.begin(); 
    p != g_CrashInfo.GetReport(m_nCurReport).m_FileItems.end(); p++, i++)
  {     
	  CString sDestFile = p->first;
    CString sSrcFile = p->second.m_sSrcFile;
    CString sFileDesc = p->second.m_sDesc;

    SHFILEINFO sfi;
    SHGetFileInfo(sSrcFile, 0, &sfi, sizeof(sfi),
      SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_TYPENAME | SHGFI_SMALLICON);

    int iImage = -1;
    if(sfi.hIcon)
    {
      iImage = m_iconList.AddIcon(sfi.hIcon);
      DestroyIcon(sfi.hIcon);
    }

    int nItem = m_list.InsertItem(i, sDestFile, iImage);
    
	  CString sFileType = sfi.szTypeName;
    m_list.SetItemText(nItem, 1, sFileDesc);
    m_list.SetItemText(nItem, 2, sFileType);

    hFind = FindFirstFile(sSrcFile, &findFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
      FindClose(hFind);
      ULARGE_INTEGER lFileSize;
      lFileSize.LowPart = findFileData.nFileSizeLow;
      lFileSize.HighPart = findFileData.nFileSizeHigh;
      sSize = Utility::FileSizeToStr(lFileSize.QuadPart);
      m_list.SetItemText(nItem, 3, sSize);
    }    
  }

  m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

  m_statPreview = GetDlgItem(IDC_PREVIEWTEXT);
  m_statPreview.SetWindowText(Utility::GetINIString(
    g_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("Preview")));  

  m_btnClose = GetDlgItem(IDOK);
  m_btnClose.SetWindowText(Utility::GetINIString(
    g_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("Close")));  

  m_btnExport = GetDlgItem(IDC_EXPORT);
  m_btnExport.SetWindowText(Utility::GetINIString(
    g_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("Export")));  

  

  // center the dialog on the screen
	CenterWindow();  

  return TRUE;
}

LRESULT CDetailDlg::OnItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  LPNMLISTVIEW lpItem           = (LPNMLISTVIEW)pnmh; 
  int iItem                     = lpItem->iItem;

  if (lpItem->uChanged & LVIF_STATE
     && lpItem->uNewState & LVIS_SELECTED)
  {
     SelectItem(iItem);
  }

  return 0;
}

LRESULT CDetailDlg::OnItemDblClicked(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  LPNMLISTVIEW lpItem           = (LPNMLISTVIEW)pnmh; 
  int iItem                     = lpItem->iItem;
  DWORD_PTR dwRet               = 0;
  
  if (iItem < 0 || (int)g_CrashInfo.GetReport(m_nCurReport).m_FileItems.size() < iItem)
     return 0;

  std::map<CString, FileItem>::iterator p = g_CrashInfo.GetReport(m_nCurReport).m_FileItems.begin();
  for (int i = 0; i < iItem; i++, p++);

  CString sFileName = p->second.m_sSrcFile;
  dwRet = (DWORD_PTR)::ShellExecute(0, _T("open"), sFileName,
    0, 0, SW_SHOWNORMAL);
  ATLASSERT(dwRet > 32);

  return 0;
}

void CDetailDlg::SelectItem(int iItem)
{
  // Sanity check  
  if (iItem < 0 || (int)g_CrashInfo.GetReport(m_nCurReport).m_FileItems.size() < iItem)
      return;

  std::map<CString, FileItem>::iterator p = g_CrashInfo.GetReport(m_nCurReport).m_FileItems.begin();
  for (int i = 0; i < iItem; i++, p++);

  m_previewMode = PREVIEW_AUTO;
  m_filePreview.SetFile(p->second.m_sSrcFile, m_previewMode);
}

LRESULT CDetailDlg::OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  EndDialog(0);
  return 0;
}

LRESULT CDetailDlg::OnExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  CString sFileName = g_CrashInfo.GetReport(m_nCurReport).m_sCrashGUID + _T(".zip");

  CFileDialog dlg(FALSE, _T("*.zip"), sFileName,
      OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,
      _T("ZIP Files (*.zip)\0*.zip\0All Files (*.*)\0*.*\0\0"), m_hWnd);
  
  INT_PTR result = dlg.DoModal();
  if(result==IDOK)
  {
    CString sExportFileName = dlg.m_szFileName;
    g_ErrorReportSender.SetExportFlag(TRUE, sExportFileName);
    g_ErrorReportSender.DoWork(COMPRESS_REPORT);    
  }

  return 0;
}

LRESULT CDetailDlg::OnPreviewRClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
  CPoint pt;
  GetCursorPos(&pt);

  CMenu menu;
  menu.LoadMenu(IDR_POPUPMENU);
  
  CMenu submenu = menu.GetSubMenu(1);
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(MENUITEMINFO));
  mii.cbSize = sizeof(MENUITEMINFO);
  mii.fMask = MIIM_STRING;
  
  strconv_t strconv;
  CString sAuto = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("PreviewAuto"));
  CString sText = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("PreviewText"));
  CString sHex = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("PreviewHex"));
  CString sImage = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("DetailDlg"), _T("PreviewImage"));

  mii.dwTypeData = sAuto.GetBuffer(0);  
  submenu.SetMenuItemInfo(ID_PREVIEW_AUTO, FALSE, &mii);
  
  mii.dwTypeData = sHex.GetBuffer(0);
  submenu.SetMenuItemInfo(ID_PREVIEW_HEX, FALSE, &mii);

  mii.dwTypeData = sText.GetBuffer(0);
  submenu.SetMenuItemInfo(ID_PREVIEW_TEXT, FALSE, &mii);

  mii.dwTypeData = sImage.GetBuffer(0);
  submenu.SetMenuItemInfo(ID_PREVIEW_IMAGE, FALSE, &mii);

  UINT uItem = ID_PREVIEW_AUTO;
  if(m_previewMode==PREVIEW_HEX)
    uItem = ID_PREVIEW_HEX;
  else if(m_previewMode==PREVIEW_TEXT)
    uItem = ID_PREVIEW_TEXT;
  else if(m_previewMode==PREVIEW_IMAGE)
    uItem = ID_PREVIEW_IMAGE;

  submenu.CheckMenuRadioItem(ID_PREVIEW_AUTO, ID_PREVIEW_IMAGE, uItem, MF_BYCOMMAND); 

  submenu.TrackPopupMenu(TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd);

  return 0;
}

LRESULT CDetailDlg::OnPreviewModeChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  PreviewMode mode = PREVIEW_AUTO;
  if(wID==ID_PREVIEW_TEXT)
    mode = PREVIEW_TEXT;
  else if(wID==ID_PREVIEW_HEX)
    mode = PREVIEW_HEX;
  else if(wID==ID_PREVIEW_IMAGE)
    mode = PREVIEW_IMAGE;
  m_previewMode = mode;
  m_filePreview.SetPreviewMode(mode);
  return 0;
}