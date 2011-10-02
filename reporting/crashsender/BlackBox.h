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

// File: BlackBox.h
// Description: Designed to record the application performance information and/or desktop state snapshots before the accident.
// Authors: zexspectrum
// Date: 2010

#pragma once
#include "stdafx.h"
#include "CrashRpt.h"

typedef struct tagCR_BLACK_BOX_INFO
{
  UINT  cb;                    //!< Size of this structure, in bytes.  
  int   nMemUsageSnapMaxCount; //!< Maximum memory usage snapshot count.
  int   nMemUsageSnapInterval; //!< Memory usage snapshot taking interval, in miliseconds.
  int   nCPUsageSnapMaxCount;  //!< CPU usage snapshot max count.
  int   nCPUsageSnapInterval;  //!< CPU usage snapshot max interval. 
  DWORD dwScreenshotFlags;     //!< Desktop screenshot flags.
  int   nScreenshotMaxCount;   //!< Maximum desktop screenshot count.
  int   nScreenshotInterval;   //!< Desktop screenshot taking interval, in miliseconds.
  float fScrenshotJpegQuality; //!< Desktop screenshot JPEG image quality (between zero and one).
  BOOL  bScreenshotGrayscale;  //!< If TRUE, generates grayscale screenshots (this decreases total file size).
}
CR_BLACK_BOX_INFO, *PCR_BLACK_BOX_INFO;


CRASHRPTAPI(int)
crEnableBlackBox(   
   BOOL bEnable,
   PCR_BLACK_BOX_INFO pInfo
   );

class CBlackBox
{
public:

  CBlackBox();
  ~CBlackBox();

  BOOL Init(PCR_BLACK_BOX_INFO pInfo);
  void Destroy();

private:

  static DWORD WINAPI ThreadProc(LPVOID lpParam);

  BOOL JpegWrite(CString sFileName);

  CR_BLACK_BOX_INFO m_Info;
};
