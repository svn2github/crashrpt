// CrashRptTest.h : main header file for the CRASHRPTTEST application
//

#if !defined(AFX_CRASHRPTTEST_H__4F0635BB_296B_47B2_9345_D3FF96CE2A18__INCLUDED_)
#define AFX_CRASHRPTTEST_H__4F0635BB_296B_47B2_9345_D3FF96CE2A18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <afxmt.h>

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCrashRptTestApp:
// See CrashRptTest.cpp for the implementation of this class
//

class CCrashRptTestApp : public CWinApp
{
public:
	int m_nThreads;
   LPVOID m_lpvState;
	CCrashRptTestApp();
   void generateErrorReport();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCrashRptTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCrashRptTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	CMutex m_mutex;
	int m_nProc;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CRASHRPTTEST_H__4F0635BB_296B_47B2_9345_D3FF96CE2A18__INCLUDED_)
