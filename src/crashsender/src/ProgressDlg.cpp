#include "stdafx.h"
#include "ProgressDlg.h"


LRESULT CProgressDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{ 
  DlgResize_Init();

  HICON hIcon = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
  SetIcon(hIcon, FALSE);
  SetIcon(hIcon, TRUE);

  m_prgProgress = GetDlgItem(IDC_PROGRESS);
  m_prgProgress.SetRange(0, 100);

  m_listView = GetDlgItem(IDC_LIST); 
  m_listView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
  m_listView.InsertColumn(0, _T("Status"), LVCFMT_LEFT, 2048);
  
  return TRUE;
}

LRESULT CProgressDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{    
  if(m_bFinished)
  {
	HWND hWndParent = ::GetParent(m_hWnd);
	::PostMessage(hWndParent, WM_CLOSE, 0, 0);
    return 0;
  }

  AnimateWindow(m_hWnd, 200, AW_HIDE|AW_BLEND); 
  return 0;
}


LRESULT CProgressDlg::OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{ 
  if(m_bFinished)
  {
	HWND hWndParent = ::GetParent(m_hWnd);
	::PostMessage(hWndParent, WM_CLOSE, 0, 0);
    return 0;
  }

  CancelSenderThread();
  CButton m_btnCancel = GetDlgItem(IDCANCEL);
  m_btnCancel.EnableWindow(0);
  return 0;
}

void CProgressDlg::Start()
{ 
  // center the dialog on the screen
	CenterWindow();

  ShowWindow(SW_SHOW); 
  SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
  FlashWindow(FALSE);

  SetTimer(1, 3000);
  SetTimer(0, 200);

  m_bFinished = FALSE;
}

LRESULT CProgressDlg::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  WORD wTimerId = (WORD)wParam;

  if(wTimerId==0)
  {
    int nProgressPct = 0;
    std::vector<CString> messages;

    GetSenderThreadStatus(nProgressPct, messages);
    
    m_prgProgress.SetPos(nProgressPct);

    int attempt = 0;

    unsigned i;
    for(i=0; i<messages.size(); i++)
    {
      CStatic statText = GetDlgItem(IDC_TEXT);

      if(messages[i].CompareNoCase(_T("[status_success]"))==0)
      { 
        m_bFinished = TRUE;
        statText.SetWindowText(_T("Completed successfuly!"));
        HWND hWndParent = ::GetParent(m_hWnd);
        ::PostMessage(hWndParent, WM_CLOSE, 0, 0);
      }

      if(messages[i].CompareNoCase(_T("[status_failed]"))==0)
      { 
        m_bFinished = TRUE;
        KillTimer(1);
        statText.SetWindowText(_T("Completed with errors. Press Close to close this window."));
        
        CButton btnCancel = GetDlgItem(IDCANCEL);
        btnCancel.EnableWindow(1);
        btnCancel.SetWindowText(_T("Close"));
        ShowWindow(SW_SHOW);
      }

      if(messages[i].CompareNoCase(_T("[cancelled_by_user]"))==0)
      { 
        statText.SetWindowText(_T("Cancelling..."));
      }

      if(messages[i].CompareNoCase(_T("[sending_attempt]"))==0)
      {
        attempt ++;      
        CString str;
        str.Format(_T("The error report is now being sent (attempt %d of 3)..."), attempt);
        statText.SetWindowText(str);
      }
      
      if(messages[i].CompareNoCase(_T("[confirm_launch_email_client]"))==0)
      {       
        KillTimer(1);
        ShowWindow(SW_SHOW);
        INT_PTR result = MessageBox(_T("Error report can be sent using your default E-mail program.\nPress OK to run the E-mail program or press Cancel to cancel."), 
          _T("Send Error Report"), MB_OKCANCEL|MB_ICONQUESTION);
        FeedbackReady(result==IDOK?0:1);       
        ShowWindow(SW_HIDE);
      }

      int count = m_listView.GetItemCount();
      int indx = m_listView.InsertItem(count, messages[i]);
      m_listView.EnsureVisible(indx, TRUE);

    }
  }

  if(wTimerId==1)
  {
    AnimateWindow(m_hWnd, 200, AW_HIDE|AW_BLEND); 
    KillTimer(1);
  }

  return 0;
}

LRESULT CProgressDlg::OnListRClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{  
  LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) pnmh;

  POINT pt;
  GetCursorPos(&pt);

  CMenu popup_menu;
  popup_menu.LoadMenu(IDR_POPUPMENU);

  CMenu submenu = popup_menu.GetSubMenu(0);  

  if(lpnmitem->iItem<0)
  {
    submenu.EnableMenuItem(ID_MENU1_COPYSEL, MF_BYCOMMAND|MF_GRAYED);
  }

  submenu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, m_hWnd); 
  return 0;
}

LRESULT CProgressDlg::OnCopySel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  CString sData;
  int i;
  for(i=0; i<m_listView.GetItemCount(); i++)
  {
    DWORD dwState = m_listView.GetItemState(i, LVIS_SELECTED);
    if(dwState==0)
      continue;

    TCHAR buf[4096];
    buf[0]=0;
    int n = m_listView.GetItemText(i, 0, buf, 4095);
    sData += CString(buf,n);
    sData += "\r\n";
  }

  SetClipboard(sData);  

  return 0;
}

LRESULT CProgressDlg::OnCopyLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  CString sData;
  int i;
  for(i=0; i<m_listView.GetItemCount(); i++)
  {
    TCHAR buf[4096];
    buf[0]=0;
    int n = m_listView.GetItemText(i, 0, buf, 4095);
    sData += CString(buf,n);
    sData += "\r\n";
  }

  SetClipboard(sData);  

  return 0;
}

int CProgressDlg::SetClipboard(CString& sData)
{
  if (OpenClipboard())
  {
    EmptyClipboard();
    HGLOBAL hClipboardData;
    DWORD dwSize = (sData.GetLength()+1)*sizeof(TCHAR);
    hClipboardData = GlobalAlloc(GMEM_DDESHARE, dwSize);
    TCHAR* pszData = (TCHAR*)GlobalLock(hClipboardData);
    if(pszData!=NULL)
    {      
      _TCSNCPY_S(pszData, dwSize/sizeof(TCHAR), sData, sData.GetLength()*sizeof(TCHAR));
      GlobalUnlock(hClipboardData);
#ifdef _UNICODE
      SetClipboardData(CF_UNICODETEXT, hClipboardData);
#else
      SetClipboardData(CF_TEXT, hClipboardData);    
#endif
      CloseClipboard();
      return 0;
   }
   CloseClipboard();
  }

  return 1;
}