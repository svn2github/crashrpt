#pragma once
#include "stdafx.h"
#include "CrashRpt.h"


struct CrashThreadInfo
{
  LPVOID m_pCrashRptState;
  HANDLE m_hWakeUpEvent;
  bool m_bStop;
  int m_ExceptionType;
};

DWORD WINAPI CrashThread(LPVOID pParam);

void test_seh();
void test_generate_report();

