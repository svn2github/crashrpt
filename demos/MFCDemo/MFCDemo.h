
// MFCDemo.h : main header file for the MFCDemo application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMFCDemoApp:
// See MFCDemo.cpp for the implementation of this class
//

class CMFCDemoApp : public CWinAppEx
{
public:
	CMFCDemoApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	// Overload Run() method to install crash handler in it.
	virtual int Run();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMFCDemoApp theApp;
