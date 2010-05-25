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
#include "ScreenCap.h"
#include <tchar.h>
#include "Utility.h"

// Disable warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable:4611)

// This function is used for monitor enumeration
BOOL CALLBACK EnumMonitorsProc(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM dwData)
{	
  CScreenCapture* psc = (CScreenCapture*)dwData;

	MONITORINFOEX mi;
	HDC hDC = NULL;  
  HDC hCompatDC = NULL;
  HBITMAP hBitmap = NULL;
  BITMAPINFO bmi;
  int nLeft = 0;
  int nTop = 0;
  int nWidth = 0;
  int nHeight = 0;
  int nRowWidth = 0;
  LPBYTE pRowBits = NULL;
  CString sFileName;

  // Get monitor size
  nLeft = lprcMonitor->left;
  nTop = lprcMonitor->top;
	nWidth = lprcMonitor->right - lprcMonitor->left;
	nHeight = lprcMonitor->bottom - lprcMonitor->top;
	
  // Get monitor info
  mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &mi);
    
	// Get the device context for this monitor
	hDC = CreateDC(_T("DISPLAY"), mi.szDevice, NULL, NULL); 
	if(hDC==NULL)
    goto cleanup;

  hCompatDC = CreateCompatibleDC(hDC);
  if(hCompatDC==NULL)
    goto cleanup;

  hBitmap = CreateCompatibleBitmap(hDC, nWidth, nHeight);
  if(hBitmap==NULL)
    goto cleanup;

  SelectObject(hCompatDC, hBitmap);

  BOOL bBitBlt = BitBlt(hCompatDC, 0, 0, nWidth, nHeight, hDC, nLeft, nTop, SRCCOPY|CAPTUREBLT);
  if(!bBitBlt)
    goto cleanup;
  
  // Draw mouse cursor.
  if(PtInRect(lprcMonitor, psc->m_ptCursorPos))
	{						
		if(psc->m_CursorInfo.flags == CURSOR_SHOWING)
		{
      ICONINFO IconInfo;
		  GetIconInfo((HICON)psc->m_CursorInfo.hCursor, &IconInfo);
			int x = psc->m_ptCursorPos.x - nLeft - IconInfo.xHotspot;
			int y = psc->m_ptCursorPos.y - nTop  - IconInfo.yHotspot;
			DrawIcon(hCompatDC, x, y, (HICON)psc->m_CursorInfo.hCursor);
      DeleteObject(IconInfo.hbmMask);
			DeleteObject(IconInfo.hbmColor);
		}				
	}

  /* Write screenshot bitmap to a PNG file. */

  // Init PNG writer
  sFileName.Format(_T("%s\\screenshot%d.png"), psc->m_sSaveDirName, psc->m_nIdStartFrom++);
  BOOL bInit = psc->PngInit(nWidth, nHeight, sFileName);
  if(!bInit)
    goto cleanup;

  // We will get bitmap bits row by row
  nRowWidth = nWidth*3 + 10;
  pRowBits = new BYTE[nRowWidth];
  if(pRowBits==NULL)
    goto cleanup;
    
  memset(&bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
  bmi.bmiHeader.biWidth = nWidth;
  bmi.bmiHeader.biHeight = -nHeight;
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biPlanes = 1;  

  int i;
  for(i=nHeight-1; i>=0; i--)
  {    
    int nFetched = GetDIBits(hCompatDC, hBitmap, i, 1, pRowBits, &bmi, DIB_RGB_COLORS);
    if(nFetched!=1)
      break;

    BOOL bWrite = psc->PngWriteRow(pRowBits);
    if(!bWrite)
      goto cleanup;   
  }
  
  psc->PngFinalize();
  
cleanup:

  // Clean up
  if(hDC)
    DeleteDC(hDC);

  if(hCompatDC)
    DeleteDC(hCompatDC);

  if(hBitmap)
    DeleteObject(hBitmap);

  if(pRowBits)
    delete [] pRowBits;

  // Next monitor
	return TRUE;
}

CScreenCapture::CScreenCapture()
{
  m_fp = NULL;
  m_png_ptr = NULL;
  m_info_ptr = NULL;
  m_nIdStartFrom = 0;
}

BOOL CScreenCapture::CaptureScreenRect(RECT rcCapture, POINT ptCursorPos, 
      CString sSaveDirName, int nIdStartFrom, std::vector<CString>& out_file_list)
{	
  // Get cursor information
  m_ptCursorPos = ptCursorPos;
  m_CursorInfo.cbSize = sizeof(CURSORINFO);
  GetCursorInfo(&m_CursorInfo);

  m_nIdStartFrom = nIdStartFrom;
  m_sSaveDirName = sSaveDirName;
	EnumDisplayMonitors(NULL, &rcCapture, EnumMonitorsProc, (LPARAM)this);	
  out_file_list = m_out_file_list;
	return TRUE;
}

void CScreenCapture::GetScreenRect(LPRECT rcScreen)
{
	int nWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int nHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	rcScreen->left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	rcScreen->top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	rcScreen->right = rcScreen->left + nWidth;
	rcScreen->bottom = rcScreen->top + nHeight;
}

BOOL CScreenCapture::PngInit(int nWidth, int nHeight, CString sFileName)
{  
  m_fp = NULL;
  m_png_ptr = NULL;
  m_info_ptr = NULL;

  m_out_file_list.push_back(sFileName);

#if _MSC_VER>=1400
  _tfopen_s(&m_fp, sFileName.GetBuffer(0), _T("wb"));
#else
  m_fp = _tfopen(sFileName.GetBuffer(0), _T("wb"));
#endif

  if (!m_fp)
  {    
    return FALSE;
  }

  m_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
    (png_voidp)NULL, NULL, NULL);
  if (!m_png_ptr)
    return FALSE;

  m_info_ptr = png_create_info_struct(m_png_ptr);
  if (!m_info_ptr)
  {
    png_destroy_write_struct(&m_png_ptr, (png_infopp)NULL);
    return FALSE;
  }

  /* Error handler*/
  if (setjmp(png_jmpbuf(m_png_ptr)))
  {
    png_destroy_write_struct(&m_png_ptr, &m_info_ptr);
    fclose(m_fp);
    return FALSE;
  }

  png_init_io(m_png_ptr, m_fp);

  /* set the zlib compression level */
  png_set_compression_level(m_png_ptr, Z_BEST_COMPRESSION);

  /* set other zlib parameters */
  png_set_compression_mem_level(m_png_ptr, 8);
  png_set_compression_strategy(m_png_ptr, Z_DEFAULT_STRATEGY);
  png_set_compression_window_bits(m_png_ptr, 15);
  png_set_compression_method(m_png_ptr, 8);
  png_set_compression_buffer_size(m_png_ptr, 8192);

  png_set_IHDR(
    m_png_ptr, 
    m_info_ptr, 
    nWidth, //width, 
    nHeight, //height,
    8, // bit_depth
    PNG_COLOR_TYPE_RGB, // color_type
    PNG_INTERLACE_NONE, // interlace_type
    PNG_COMPRESSION_TYPE_DEFAULT, 
    PNG_FILTER_TYPE_DEFAULT);

  png_set_bgr(m_png_ptr);

  /* write the file information */
  png_write_info(m_png_ptr, m_info_ptr);

  return TRUE;
}

BOOL CScreenCapture::PngWriteRow(LPBYTE pRow)
{
  png_bytep rows[1] = {pRow};
  png_write_rows(m_png_ptr, (png_bytepp)&rows, 1);
  return TRUE;
}

BOOL CScreenCapture::PngFinalize()
{
  /* end write */
  png_write_end(m_png_ptr, m_info_ptr);

  /* clean up */
  png_destroy_write_struct(&m_png_ptr, (png_infopp)&m_info_ptr);
  
  if(m_fp)
    fclose(m_fp);

  return TRUE;
}







