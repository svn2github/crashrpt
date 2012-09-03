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

class CMFCDemoApp : public CWinApp
{
public:
	CMFCDemoApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int Run();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMFCDemoApp theApp;