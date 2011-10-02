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

#ifndef __SCREENCAP_H__
#define __SCREENCAP_H__

#include "stdafx.h"

extern "C" {
#include "png.h"
}
#include "jpeglib.h"

// Window information
struct WindowInfo
{
    CString m_sTitle; // Window title
    CRect m_rcWnd;    // Window rect
    DWORD dwStyle;
    DWORD dwExStyle;
};

// Monitor info
struct MonitorInfo
{
    CString m_sDeviceID; // Device ID
    CRect m_rcMonitor;   // Monitor rectangle in screen coordinates
    CString m_sFileName; // Image file name corresponding to this monitor
};

// Desktop screen shot info
struct ScreenshotInfo
{
    ScreenshotInfo()
    {
        m_bValid = FALSE;
    }

    BOOL m_bValid;
    CRect m_rcVirtualScreen;
    std::vector<MonitorInfo> m_aMonitors; // The list of monitors.
    std::vector<WindowInfo> m_aWindows; // The list of windows.
};

// What format to use when saving screenshots
enum SCREENSHOT_IMAGE_FORMAT
{
    SCREENSHOT_FORMAT_PNG = 0, // Use PNG format
    SCREENSHOT_FORMAT_JPG = 1, // Use JPG format
    SCREENSHOT_FORMAT_BMP = 2  // Use BMP format
};

// Desktop screenshot capture
class CScreenCapture
{
public:

    // Constructor
    CScreenCapture();
    ~CScreenCapture();

    // Returns virtual screen rectangle
    void GetScreenRect(LPRECT rcScreen);

    // Returns an array of visible windows for the specified process or 
    // the main window of the process.
    BOOL FindWindows(HANDLE hProcess, BOOL bAllProcessWindows, 
        std::vector<WindowInfo>* paWindows);

    // Captures the specified screen area and returns the list of image files
    BOOL CaptureScreenRect(
        std::vector<CRect> arcCapture, 
        CString sSaveDirName, 
        int nIdStartFrom, 
        SCREENSHOT_IMAGE_FORMAT fmt, 
        int nJpegQuality,
        BOOL bGrayscale,
        std::vector<MonitorInfo>& monitor_list,
        std::vector<CString>& out_file_list);

    /* PNG management functions */

    // Initializes PNG file header
    BOOL PngInit(int nWidth, int nHeight, BOOL bGrayscale, CString sFileName);
    // Writes a scan line to the PNG file
    BOOL PngWriteRow(LPBYTE pRow, int nRowLen, BOOL bGrayscale);
    // Closes PNG file
    BOOL PngFinalize();

    /* JPEG management functions */

    BOOL JpegInit(int nWidth, int nHeight, BOOL bGrayscale, int nQuality, CString sFileName);
    BOOL JpegWriteRow(LPBYTE pRow, int nRowLen, BOOL bGrayscale);
    BOOL JpegFinalize();

    /* Member variables. */

    CPoint m_ptCursorPos;                 // Current mouse cursor pos
    std::vector<CRect> m_arcCapture;      // Array of capture rectangles
    CURSORINFO m_CursorInfo;              // Cursor info  
    int m_nIdStartFrom;                   // An ID for the current screenshot image 
    CString m_sSaveDirName;               // Directory name to save screenshots to
    SCREENSHOT_IMAGE_FORMAT m_fmt;        // Image format
    int m_nJpegQuality;                   // Jpeg quality
    BOOL m_bGrayscale;                    // Create grayscale image or not
    FILE* m_fp;                           // Handle to the file
    png_structp m_png_ptr;                // libpng stuff
    png_infop m_info_ptr;                 // libpng stuff
    struct jpeg_compress_struct m_cinfo;  // libjpeg stuff
    struct jpeg_error_mgr m_jerr;         // libjpeg stuff
    std::vector<MonitorInfo> m_monitor_list; // The list of monitor devices
    std::vector<CString> m_out_file_list; // The list of output image files
};

#endif //__SCREENCAP_H__


