// crashcon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <process.h>

#ifndef _CRASH_RPT_
#include "../../crashrpt/src/crashrpt.h"
#pragma comment(lib, "../../crashrpt/lib/crashrpt.lib")
#endif

int main(int argc, char* argv[])
{
   LPVOID lpvState = Install(NULL, NULL, NULL);
#ifdef _DEBUG
   printf("Press a ENTER to simulate a null pointer exception...\n");
   getchar();
   __try {
      RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
   } __except(GenerateErrorReport(lpvState, GetExceptionInformation())){}
#else
   printf("Press a ENTER to generate a null pointer exception...\n");
   getchar();
   int *p = 0;
   *p = 0;
#endif // _DEBUG
	return 0;
}

