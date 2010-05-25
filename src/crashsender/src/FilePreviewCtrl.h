#pragma once
#include "atlmisc.h"
#include <vector>
#include <map>
#include "CritSec.h"

// Preview mode
enum PreviewMode
{
  PREVIEW_AUTO = -1,  // Auto
  PREVIEW_HEX  = 0,   // Hex
  PREVIEW_TEXT = 1,   // Text
  PREVIEW_IMAGE = 2   // Image  
};

// Used to map file contents into memory
class CFileMemoryMapping
{
public:

	CFileMemoryMapping();  
  ~CFileMemoryMapping();  

	BOOL Init(LPCTSTR szFileName);
	BOOL Destroy();

  ULONG64 GetSize();
	LPBYTE CreateView(DWORD dwOffset, DWORD dwLength);

private:

  HANDLE m_hFile;		          // Handle to current file
	HANDLE m_hFileMapping;		  // Memory mapped object
  DWORD m_dwAllocGranularity; // System allocation granularity  	  
	ULONG64 m_uFileLength;		  // Size of the file.		
  CCritSec m_csLock;
  std::map<DWORD, LPBYTE> m_aViewStartPtrs; // Base of the view of the file.    
};

struct LineInfo
{
  DWORD m_dwOffsetInFile;
  DWORD m_cchLineLength;  
};

class CImage
{
public:

  CImage();
  ~CImage();
    
  void Destroy();

  static BOOL IsBitmap(FILE* f);
  static BOOL IsPNG(FILE* f);
  static BOOL IsImageFile(CString sFileName);

  BOOL Load(CString sFileName);
  void Cancel();
  BOOL IsValid();  
  void Draw(HDC hDC, LPRECT prcDraw);
  
private:
  
  BOOL LoadBitmapFromBMPFile(LPTSTR szFileName);
  BOOL LoadBitmapFromPNGFile(LPTSTR szFileName);

  CCritSec m_csLock;      // Critical section
  HBITMAP m_hBitmap;      // Handle to the bitmap.  
  HPALETTE m_hPalette;    // Palette
  BOOL m_bLoadCancelled;
};

#define WM_FPC_COMPLETE  (WM_APP+100)

class CFilePreviewCtrl : public CWindowImpl<CFilePreviewCtrl, CStatic>
{
public:
  
  CFilePreviewCtrl();
  ~CFilePreviewCtrl();

  DECLARE_WND_SUPERCLASS(NULL, CWindow::GetWndClassName())

  BEGIN_MSG_MAP(CFilePreviewCtrl)  
    if (uMsg == WM_NCHITTEST || 
        uMsg == WM_NCLBUTTONDOWN || 
        uMsg == WM_NCMBUTTONDOWN ||
        uMsg == WM_NCXBUTTONDOWN ||
        uMsg == WM_NCLBUTTONDBLCLK)
    {
      // This is to enable scroll bar messages
      bHandled = TRUE;
      lResult = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
      return TRUE;
    }

    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
    MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_FPC_COMPLETE, OnComplete)
    MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
  END_MSG_MAP()

  LPCTSTR GetFile();
  BOOL SetFile(LPCTSTR szFileName, PreviewMode mode=PREVIEW_AUTO);
  PreviewMode GetPreviewMode();
  void SetPreviewMode(PreviewMode mode);
  void SetEmptyMessage(CString sText);
  BOOL SetBytesPerLine(int nBytesPerLine);

  PreviewMode DetectPreviewMode(LPCTSTR szFileName);

  LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnComplete(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

  void SetupScrollbars();
  CString FormatHexLine(LPBYTE pData, int nBytesInLine, ULONG64 uLineOffset);
  void DrawHexLine(HDC hdc, DWORD nLineNo);
  void DrawTextLine(HDC hdc, DWORD nLineNo);  
  void DoPaintEmpty(HDC hDC);
  void DoPaintText(HDC hDC);
  void DoPaintBitmap(HDC hDC);
  void DoPaint(HDC hDC);

  static DWORD WINAPI WorkerThread(LPVOID lpParam);
  void DoInWorkerThread();
  void ParseText();
  void LoadBitmap();
    
  CString m_sFileName;
  PreviewMode m_PreviewMode;
  CCritSec m_csLock;
  CFileMemoryMapping m_fm;  // File mapping object.  
  HFONT m_hFont;            // Font in use.  
  int m_xChar;              // Size of character in x direction.
  int m_yChar;              // Size of character in y direction.
  int m_nMaxColsPerPage;    // Maximum columns per page.
  int m_nMaxLinesPerPage;   // Maximum count of lines per one page.
	int m_nMaxDisplayWidth;   
	ULONG64 m_uNumLines;      // Number of lines in the file.	
  int m_nBytesPerLine;      // Count of displayed bytes per line.
  int m_cchTabLength;
  CString m_sEmptyMsg;      // Text to display when file is empty
  int m_nHScrollPos;        // Horizontal scroll position.
	int m_nHScrollMax;        // Max horizontal scroll position.
	int m_nVScrollPos;        // Vertical scrolling position.
	int m_nVScrollMax;        // Maximum vertical scrolling position.  
  std::vector<DWORD> m_aTextLines; // The array of lines of text file.
  HANDLE m_hWorkerThread;   // Handle to the worker thread.
  BOOL m_bCancelled;        // Is worker thread cancelled?
  CImage m_bmp;          // Stores the bitmap
};



