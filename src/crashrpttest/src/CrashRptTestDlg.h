// CrashRptTestDlg.h : header file
//

#if !defined(AFX_CRASHRPTTESTDLG_H__BBACA293_F00E_47DE_BC9D_CD93B6864747__INCLUDED_)
#define AFX_CRASHRPTTESTDLG_H__BBACA293_F00E_47DE_BC9D_CD93B6864747__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCrashRptTestDlg dialog

class CCrashRptTestDlg : public CDialog
{
// Construction
public:
	CCrashRptTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CCrashRptTestDlg)
	enum { IDD = IDD_CRASHRPTTEST_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCrashRptTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCrashRptTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CRASHRPTTESTDLG_H__BBACA293_F00E_47DE_BC9D_CD93B6864747__INCLUDED_)
