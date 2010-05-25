#include "stdafx.h"
#include "FilePreviewCtrl.h"
#include <set>
#include "png.h"

#pragma warning(disable:4611)
// DIBSIZE calculates the number of bytes required by an image

#define WIDTHBYTES(bits) ((DWORD)(((bits)+31) & (~31)) / 8)
#define DIBWIDTHBYTES(bi) (DWORD)WIDTHBYTES((DWORD)(bi).biWidth * (DWORD)(bi).biBitCount)
#define _DIBSIZE(bi) (DIBWIDTHBYTES(bi) * (DWORD)(bi).biHeight)
#define DIBSIZE(bi) ((bi).biHeight < 0 ? (-1)*(_DIBSIZE(bi)) : _DIBSIZE(bi))

//-----------------------------------------------------------------------------
// CFileMemoryMapping implementation
//-----------------------------------------------------------------------------

CFileMemoryMapping::CFileMemoryMapping()  
{
  m_hFile = INVALID_HANDLE_VALUE;
  m_uFileLength = 0;
  m_hFileMapping = NULL;  
  
  SYSTEM_INFO si;  
  GetSystemInfo(&si);
  m_dwAllocGranularity = si.dwAllocationGranularity;  
}

CFileMemoryMapping::~CFileMemoryMapping()
{
  Destroy();
}


BOOL CFileMemoryMapping::Init(LPCTSTR szFileName)
{
  if(m_hFile!=INVALID_HANDLE_VALUE)
  {
    Destroy();    
  }

	m_hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	
	m_hFileMapping = CreateFileMapping(m_hFile, 0, PAGE_READONLY, 0, 0, 0);
  LARGE_INTEGER size;
	GetFileSizeEx(m_hFile, &size);	
  m_uFileLength = size.QuadPart; 

	return TRUE;
}

BOOL CFileMemoryMapping::Destroy()
{
  std::map<DWORD, LPBYTE>::iterator it;
  for(it=m_aViewStartPtrs.begin(); it!=m_aViewStartPtrs.end(); it++)
  {
    if(it->second != NULL)
      UnmapViewOfFile(it->second);    
  }
  m_aViewStartPtrs.clear();
  
  if(m_hFileMapping!=NULL)
  {
	  CloseHandle(m_hFileMapping);    
  }

  if(m_hFile!=INVALID_HANDLE_VALUE)
  {
	  CloseHandle(m_hFile);    
  }

	m_hFileMapping = NULL;
	m_hFile = INVALID_HANDLE_VALUE;		
	m_uFileLength = 0;  

	return TRUE;
}

ULONG64 CFileMemoryMapping::GetSize()
{
	return m_uFileLength;
}

LPBYTE CFileMemoryMapping::CreateView(DWORD dwOffset, DWORD dwLength)
{
  DWORD dwThreadId = GetCurrentThreadId();
  DWORD dwBaseOffs = dwOffset-dwOffset%m_dwAllocGranularity;
  DWORD dwDiff = dwOffset-dwBaseOffs;
  LPBYTE pPtr = NULL;

  CAutoLock lock(&m_csLock);

  std::map<DWORD, LPBYTE>::iterator it = m_aViewStartPtrs.find(dwThreadId);
  if(it!=m_aViewStartPtrs.end())
  {
    UnmapViewOfFile(it->second);
  }
  
  pPtr = (LPBYTE)MapViewOfFile(m_hFileMapping, FILE_MAP_READ, 0, dwBaseOffs, dwLength+dwDiff);
  if(it!=m_aViewStartPtrs.end())
  {
    it->second = pPtr;
  }
  else
  {
    m_aViewStartPtrs[dwThreadId] = pPtr;
  }
  
  return (pPtr+dwDiff);
}

CImage::CImage()
{
  m_hBitmap = NULL;
  m_hPalette = NULL;
  m_bLoadCancelled = FALSE;
}

CImage::~CImage()
{
  Destroy();
}

BOOL CImage::IsBitmap(FILE* f)
{
  rewind(f);
  
  BITMAPFILEHEADER bfh;
  size_t n = fread(&bfh, sizeof(bfh), 1, f);
  if(n!=1)
    return FALSE;

  if(memcmp(&bfh.bfType, "BM", 2)!=0)
    return FALSE;
  
  return TRUE;  
}

BOOL CImage::IsPNG(FILE* f)
{
  png_byte header[8];
  rewind(f);
  
  fread(header, 1, 8, f);
  if(png_sig_cmp(header, 0, 8))  
    return FALSE;

  return TRUE;
}

BOOL CImage::IsImageFile(CString sFileName)
{
  FILE* f = NULL;
  _TFOPEN_S(f, sFileName.GetBuffer(0), _T("rb"));
  if(f==NULL)
    return FALSE;

  if(IsBitmap(f) || IsPNG(f))
  {
    fclose(f);
    return TRUE;
  }

  fclose(f);
  return FALSE;
}

void CImage::Destroy()
{
  CAutoLock lock(&m_csLock);

  if(m_hBitmap)
  {
    DeleteObject(m_hBitmap);
    m_hBitmap = NULL;
  }

  if(m_hPalette)
  {
    DeleteObject(m_hPalette);
    m_hPalette = NULL;
  }

  m_bLoadCancelled = FALSE;
}

BOOL CImage::IsValid()
{
  CAutoLock lock(&m_csLock);
  return m_hBitmap!=NULL;
}

BOOL CImage::Load(CString sFileName)
{
  Destroy();

  FILE* f = NULL;
  _TFOPEN_S(f, sFileName.GetBuffer(0), _T("rb"));
  if(f==NULL)
    return FALSE;
  if(IsBitmap(f))
  {
    fclose(f);
    return LoadBitmapFromBMPFile(sFileName.GetBuffer(0));
  }
  else if(IsPNG(f))
  {
    fclose(f);
    return LoadBitmapFromPNGFile(sFileName.GetBuffer(0));
  }

  fclose(f);
  return FALSE;  
}

void CImage::Cancel()
{
  CAutoLock lock(&m_csLock);
  m_bLoadCancelled = TRUE;
}

// The following code was taken from http://support.microsoft.com/kb/158898
BOOL CImage::LoadBitmapFromBMPFile(LPTSTR szFileName)
{
  CAutoLock lock(&m_csLock);

  BITMAP  bm;

  // Use LoadImage() to get the image loaded into a DIBSection
  m_hBitmap = (HBITMAP)LoadImage( NULL, szFileName, IMAGE_BITMAP, 0, 0,
               LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE );
  if( m_hBitmap == NULL )
    return FALSE;

  // Get the color depth of the DIBSection
  GetObject(m_hBitmap, sizeof(BITMAP), &bm );
  // If the DIBSection is 256 color or less, it has a color table
  if( ( bm.bmBitsPixel * bm.bmPlanes ) <= 8 )
  {
    HDC           hMemDC = NULL;
    HBITMAP       hOldBitmap = NULL;
    RGBQUAD       rgb[256];
    LPLOGPALETTE  pLogPal = NULL;
    WORD          i = 0;

    // Create a memory DC and select the DIBSection into it
    hMemDC = CreateCompatibleDC( NULL );
    hOldBitmap = (HBITMAP)SelectObject( hMemDC, m_hBitmap );
    
    // Get the DIBSection's color table
    GetDIBColorTable( hMemDC, 0, 256, rgb );
    
    // Create a palette from the color tabl
    pLogPal = (LOGPALETTE *)malloc( sizeof(LOGPALETTE) + (256*sizeof(PALETTEENTRY)) );
    pLogPal->palVersion = 0x300;
    pLogPal->palNumEntries = 256;
    
    for(i=0;i<256;i++)
    {
      pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
      pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
      pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
      pLogPal->palPalEntry[i].peFlags = 0;
    }
    
    m_hPalette = CreatePalette( pLogPal );
   
    // Clean up
    free( pLogPal );
    SelectObject( hMemDC, hOldBitmap );
    DeleteDC( hMemDC );
  }
  else   // It has no color table, so use a halftone palette
  {
    HDC    hRefDC = NULL;

    hRefDC = GetDC( NULL );
    m_hPalette = CreateHalftonePalette( hRefDC );
    ReleaseDC( NULL, hRefDC );
  }
  return TRUE;
}

//void png_error_fn(png_struct *png_ptr, char *message)
//{	
//	longjmp(png_ptr->jmpbuf, 1);
//}

BOOL CImage::LoadBitmapFromPNGFile(LPTSTR szFileName)
{
  BOOL bStatus = FALSE;

  FILE *fp = NULL;
  const int number = 8;
  png_byte header[number];
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_infop end_info = NULL;
  png_uint_32 rowbytes = 0;
  png_uint_32 width = 0;
  png_uint_32 height = 0;
  png_bytep row = NULL;
  int y = 0;
  BITMAPINFO bmi;
  HDC hDC = NULL;

  _TFOPEN_S(fp, szFileName, _T("rb"));
  if (!fp)
  {
    return FALSE;
  }
  
  fread(header, 1, number, fp);
  if(png_sig_cmp(header, 0, number))  
  {
    goto cleanup;
  }  

  png_ptr = png_create_read_struct
     (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
    goto cleanup;

  if (setjmp(png_ptr->jmpbuf)) 
    goto cleanup;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    goto cleanup;

  end_info = png_create_info_struct(png_ptr);
  if (!end_info)
    goto cleanup;

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, number);

  // Read PNG information
  png_read_info(png_ptr, info_ptr);

  // Get count of bytes per row
  rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  row = new png_byte[rowbytes];

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);

  png_set_strip_16(png_ptr);
  png_set_packing(png_ptr);
  png_set_bgr(png_ptr);
  
  hDC = GetDC(NULL);

  {
    CAutoLock lock(&m_csLock);
    if(m_bLoadCancelled)
      goto cleanup;
    m_hBitmap = CreateCompatibleBitmap(hDC, width, height);  
  }

  memset(&bmi, 0, sizeof(bmi));  
  bmi.bmiHeader.biSize = sizeof(bmi);
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = rowbytes*height;
  
  for(y=height-1; y>=0; y--)
  {
    png_read_rows(png_ptr, &row, png_bytepp_NULL, 1); 

    {
      CAutoLock lock(&m_csLock);
      int n = SetDIBits(hDC, m_hBitmap, y, 1, row, &bmi, DIB_RGB_COLORS);
      if(n==0)
        goto cleanup;
    }
  }

  /* Read rest of file, and get additional chunks in info_ptr - REQUIRED */
  png_read_end(png_ptr, info_ptr);

  bStatus = TRUE;

cleanup:

  if(fp!=NULL)
  {
    fclose(fp);
  }

  if(png_ptr)
  {
    png_destroy_read_struct(&png_ptr,
      (png_infopp)&info_ptr, (png_infopp)&end_info);
  }

  if(row)
  {
    delete [] row;
  }

  if(!bStatus)
  {
    Destroy();
  }

  return bStatus;
}


void CImage::Draw(HDC hDC, LPRECT prcDraw)
{
  CAutoLock lock(&m_csLock);
  HPALETTE hOldPalette = NULL;

  CRect rcDraw = prcDraw;
  BITMAP        bm;
  GetObject( m_hBitmap, sizeof(BITMAP), &bm );
  
  HDC hMemDC = CreateCompatibleDC( hDC );
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, m_hBitmap );
  if(m_hPalette)
  {
    hOldPalette = SelectPalette(hDC, m_hPalette, FALSE );
    RealizePalette( hDC );
  }
    
  if((float)rcDraw.Width()/(float)bm.bmWidth <
    (float)rcDraw.Height()/(float)bm.bmHeight)
  {    
    int nDstMid = rcDraw.top + rcDraw.Height()/2;
    int nDstHeight = (int)( rcDraw.Width()*(float)bm.bmHeight/bm.bmWidth ); 
    rcDraw.top = nDstMid - nDstHeight/2; 
    rcDraw.bottom = nDstMid + nDstHeight/2; 
  }
  else
  {
    int nDstMid = rcDraw.left + rcDraw.Width()/2;
    int nDstWidth = (int)( rcDraw.Height()*(float)bm.bmWidth/bm.bmHeight ); 
    rcDraw.left = nDstMid - nDstWidth/2; 
    rcDraw.right = nDstMid + nDstWidth/2; 
  }

  int nOldMode = SetStretchBltMode(hDC, HALFTONE);
  StretchBlt(hDC, rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height(), 
    hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );

  SetStretchBltMode(hDC, nOldMode);
  SelectObject( hMemDC, hOldBitmap );  
  if(m_hPalette)
  {
    SelectPalette( hDC, hOldPalette, FALSE ); 
  }
}


//-----------------------------------------------------------------------------
// CFilePreviewCtrl implementation
//-----------------------------------------------------------------------------

CFilePreviewCtrl::CFilePreviewCtrl()
{   
  m_xChar = 10;
  m_yChar = 10;
  m_nHScrollPos = 0;
	m_nHScrollMax = 0;
	m_nMaxColsPerPage = 0;
  m_nMaxLinesPerPage = 0;
	m_nMaxDisplayWidth = 0;   
	m_uNumLines = 0;
  m_nVScrollPos = 0;
	m_nVScrollMax = 0;
  m_nBytesPerLine = 16; 
  m_cchTabLength = 4;
  m_sEmptyMsg = _T("No data to display");
  m_hFont = CreateFont(14, 7, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, 0, 0, 
    ANTIALIASED_QUALITY, FIXED_PITCH, _T("Courier"));

  m_hWorkerThread = NULL;
  m_bCancelled = FALSE;
  m_PreviewMode = PREVIEW_HEX;
}

CFilePreviewCtrl::~CFilePreviewCtrl()
{
  DeleteObject(m_hFont);  
}

LPCTSTR CFilePreviewCtrl::GetFile()
{
  if(m_sFileName.IsEmpty())
    return NULL;
  return m_sFileName;
}

BOOL CFilePreviewCtrl::SetFile(LPCTSTR szFileName, PreviewMode mode)
{
  // If we are currently processing some file in background,
  // stop the worker thread
  if(m_hWorkerThread!=NULL)
  {
    m_bCancelled = TRUE;
    m_bmp.Cancel();
    WaitForSingleObject(m_hWorkerThread, INFINITE);
    m_hWorkerThread = NULL;
  }

  CAutoLock lock(&m_csLock);

  m_sFileName = szFileName;
  
  if(mode==PREVIEW_AUTO)
    m_PreviewMode = DetectPreviewMode(m_sFileName);
  else
    m_PreviewMode = mode;
  
  if(szFileName==NULL)
  {
    m_fm.Destroy();
  }
  else
  {
    if(!m_fm.Init(m_sFileName))
    {
      m_sFileName.Empty();
      return FALSE;
    }
  }

  CRect rcClient;
  GetClientRect(&rcClient);

  HDC hDC = GetDC();
	HFONT hOldFont = (HFONT)SelectObject(hDC, m_hFont);
	
  LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));
	GetObject(m_hFont, sizeof(LOGFONT), &lf);			
  m_xChar = lf.lfWidth;
  m_yChar = lf.lfHeight;
	
  SelectObject(hDC, hOldFont);

	m_nVScrollPos = 0; 
  m_nVScrollMax = 0;
  m_nHScrollPos = 0;
  m_nHScrollMax = 0;
  m_aTextLines.clear();
  m_uNumLines = 0;
  m_nMaxDisplayWidth = 0;
  m_bmp.Destroy();

  if(m_PreviewMode==PREVIEW_HEX)
  {
    if(m_fm.GetSize()!=0)
    {
	    m_nMaxDisplayWidth = 
		    8 +				//adress
		    2 +				//padding
		    m_nBytesPerLine * 3 +	//hex column
		    1 +				//padding
		    m_nBytesPerLine;	//ascii column
    }

    m_uNumLines = m_fm.GetSize() / m_nBytesPerLine;
	
    if(m_fm.GetSize() % m_nBytesPerLine)
		  m_uNumLines++;
  }
  else if(m_PreviewMode==PREVIEW_TEXT)
  {       
    m_bCancelled = FALSE;
    m_hWorkerThread = CreateThread(NULL, 0, WorkerThread, this, 0, NULL);
    ::SetTimer(m_hWnd, 0, 250, NULL);
  }
  else if(m_PreviewMode==PREVIEW_IMAGE)
  {
    m_bCancelled = FALSE;
    m_hWorkerThread = CreateThread(NULL, 0, WorkerThread, this, 0, NULL);    
    ::SetTimer(m_hWnd, 0, 250, NULL);
  }

  SetupScrollbars();
	InvalidateRect(NULL, FALSE);
  UpdateWindow();

  return TRUE;
}

PreviewMode CFilePreviewCtrl::GetPreviewMode()
{
  return m_PreviewMode;
}

void CFilePreviewCtrl::SetPreviewMode(PreviewMode mode)
{
  SetFile(m_sFileName, mode);
}

PreviewMode CFilePreviewCtrl::DetectPreviewMode(LPCTSTR szFileName)
{
  PreviewMode mode = PREVIEW_HEX;
  CString sFileName;
  std::set<CString>::iterator it;
  std::set<CString> aTextFileExtensions;
  CString sExtension;

  sFileName = szFileName;
    
  if(CImage::IsImageFile(sFileName))
  {
    mode = PREVIEW_IMAGE;
    goto cleanup;
  }

  int backslash_pos = sFileName.ReverseFind('\\');
  if(backslash_pos>=0)
    sFileName = sFileName.Mid(backslash_pos+1);  
  int dot_pos = sFileName.ReverseFind('.');
  if(dot_pos>0)
    sExtension = sFileName.Mid(dot_pos+1);
  sExtension.MakeUpper();
  
  aTextFileExtensions.insert(_T("TXT"));
  aTextFileExtensions.insert(_T("INI"));
  aTextFileExtensions.insert(_T("LOG"));
  aTextFileExtensions.insert(_T("XML"));
  aTextFileExtensions.insert(_T("HTM"));
  aTextFileExtensions.insert(_T("HTML"));
  aTextFileExtensions.insert(_T("JS"));
  aTextFileExtensions.insert(_T("C"));
  aTextFileExtensions.insert(_T("H"));
  aTextFileExtensions.insert(_T("CPP"));
  aTextFileExtensions.insert(_T("HPP"));

  it = aTextFileExtensions.find(sExtension);
  if(it!=aTextFileExtensions.end())
  {
    mode = PREVIEW_TEXT;
  }

cleanup:

  
  return mode;
}


DWORD WINAPI CFilePreviewCtrl::WorkerThread(LPVOID lpParam)
{
  CFilePreviewCtrl* pCtrl = (CFilePreviewCtrl*)lpParam;
  pCtrl->DoInWorkerThread(); 
  return 0;
}

void CFilePreviewCtrl::DoInWorkerThread()
{
  if(m_PreviewMode==PREVIEW_TEXT)
    ParseText();
  else
    LoadBitmap();
}

void CFilePreviewCtrl::ParseText()
{
  DWORD dwFileSize = (DWORD)m_fm.GetSize();
  DWORD dwOffset = 0;
  DWORD dwPrevOffset = 0;
  int nTabs = 0;  

  if(dwFileSize!=0)
  {
      CAutoLock lock(&m_csLock);        
      m_aTextLines.push_back(0);
  }

  for(;;)
  {
    {
      CAutoLock lock(&m_csLock);        
      if(m_bCancelled)
        break;
    }

    DWORD dwLength = 4096;
    if(dwOffset+dwLength>=dwFileSize)
      dwLength = dwFileSize-dwOffset;

    if(dwLength==0)
      break;

    LPBYTE ptr = m_fm.CreateView(dwOffset, dwLength);

    UINT i;
    for(i=0; i<dwLength; i++)
    {
      {
        CAutoLock lock(&m_csLock);        
        if(m_bCancelled)
          break;
      }

      char c = ((char*)ptr)[i];

      if(c=='\t')
      {
        nTabs++;
      }
      else if(c=='\n')
      {
        CAutoLock lock(&m_csLock);        
        m_aTextLines.push_back(dwOffset+i+1);
        int cchLineLength = dwOffset+i+1-dwPrevOffset;
        if(nTabs!=0)
          cchLineLength += nTabs*(m_cchTabLength-1);

        m_nMaxDisplayWidth = max(m_nMaxDisplayWidth, cchLineLength);
        m_uNumLines++;
        dwPrevOffset = dwOffset+i+1;        
        nTabs = 0;
      }
    }

    dwOffset += dwLength;        
  }

  PostMessage(WM_FPC_COMPLETE);
}

void CFilePreviewCtrl::LoadBitmap()
{
  m_bmp.Load(m_sFileName);
  PostMessage(WM_FPC_COMPLETE);
}

void CFilePreviewCtrl::SetEmptyMessage(CString sText)
{
  m_sEmptyMsg = sText;
}

BOOL CFilePreviewCtrl::SetBytesPerLine(int nBytesPerLine)
{
  if(nBytesPerLine<0)
    return FALSE;

  m_nBytesPerLine = nBytesPerLine;
  return TRUE;
}

void CFilePreviewCtrl::SetupScrollbars()
{
  CAutoLock lock(&m_csLock);
  CRect rcClient;
  GetClientRect(&rcClient);

  SCROLLINFO sInfo;

	//	Vertical scrollbar

  m_nMaxLinesPerPage = (int)min(m_uNumLines, rcClient.Height() / m_yChar);
	m_nVScrollMax = (int)max(0, m_uNumLines-1);
  m_nVScrollPos = (int)min(m_nVScrollPos, m_nVScrollMax-m_nMaxLinesPerPage+1);
	
	sInfo.cbSize = sizeof(SCROLLINFO);
	sInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	sInfo.nMin	= 0;
	sInfo.nMax	= m_nVScrollMax;
	sInfo.nPos	= m_nVScrollPos;
	sInfo.nPage	= min(m_nMaxLinesPerPage, m_nVScrollMax+1);
	SetScrollInfo (SB_VERT, &sInfo, TRUE);
	
	//	Horizontal scrollbar 
	
  m_nMaxColsPerPage = min(m_nMaxDisplayWidth+1, rcClient.Width() / m_xChar);	
  m_nHScrollMax = max(0, m_nMaxDisplayWidth-1);
  m_nHScrollPos = min(m_nHScrollPos, m_nHScrollMax-m_nMaxColsPerPage+1);
	
	sInfo.cbSize = sizeof(SCROLLINFO);
	sInfo.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
	sInfo.nMin	= 0;
	sInfo.nMax	= m_nHScrollMax;
	sInfo.nPos	= m_nHScrollPos;
	sInfo.nPage	= min(m_nMaxColsPerPage, m_nHScrollMax+1);

	SetScrollInfo (SB_HORZ, &sInfo, TRUE);

}	

//
//	Create 1 line of a hex-dump, given a buffer of BYTES
//
CString CFilePreviewCtrl::FormatHexLine(LPBYTE pData, int nBytesInLine, ULONG64 uLineOffset)
{
  CString sResult;
  CString str;
	int i;

	//print the hex address
  str.Format(_T("%08X  "), uLineOffset);
  sResult += str;

	//print hex data
	for(i = 0; i < nBytesInLine; i++)
	{
    str.Format(_T("%02X "), pData[i]);
    sResult += str;
	}

	//print some blanks if this isn't a full line
	for(; i < m_nBytesPerLine; i++)
	{
    str.Format(_T("   "));
    sResult += str;
	}

	//print a gap between the hex and ascii
	sResult += _T(" ");

	//print the ascii
	for(i = 0; i < nBytesInLine; i++)
	{
		BYTE c = pData[i];
		if(c < 32 || c > 128) c = '.';
    str.Format( _T("%c"), c);
    sResult += str;
	}

	//print some blanks if this isn't a full line
	for(; i < m_nBytesPerLine; i++)
	{
		sResult += _T(" ");
	}

  return sResult;
}

//
//	Draw 1 line to the display
//
void CFilePreviewCtrl::DrawHexLine(HDC hdc, DWORD nLineNo)
{	
  int nBytesPerLine = m_nBytesPerLine;

  if(m_fm.GetSize() - nLineNo * m_nBytesPerLine < (UINT)m_nBytesPerLine)
    nBytesPerLine= (DWORD)m_fm.GetSize() - nLineNo * m_nBytesPerLine;

	//get data from our file mapping
	LPBYTE ptr = m_fm.CreateView(nLineNo * m_nBytesPerLine, nBytesPerLine);
	
	//convert the data into a one-line hex-dump
	CString str = FormatHexLine(ptr, nBytesPerLine, nLineNo*m_nBytesPerLine );

	//draw this line to the screen
	TextOut(hdc, -(int)(m_nHScrollPos * m_xChar), 
    (nLineNo - m_nVScrollPos) * (m_yChar-1) , str, str.GetLength());
}

void CFilePreviewCtrl::DrawTextLine(HDC hdc, DWORD nLineNo)
{
  CRect rcClient;
  GetClientRect(&rcClient);

  DWORD dwOffset = 0;
  DWORD dwLength = 0;
  {
    CAutoLock lock(&m_csLock);
    dwOffset = m_aTextLines[nLineNo];
    if(nLineNo==m_uNumLines-1)
      dwLength = (DWORD)m_fm.GetSize() - dwOffset;
    else
      dwLength = m_aTextLines[nLineNo+1]-dwOffset-1;
  }

  if(dwLength==0)
    return;
  
  //get data from our file mapping
	LPBYTE ptr = m_fm.CreateView(dwOffset, dwLength);

  //draw this line to the screen
  CRect rcText;
  rcText.left = -(int)(m_nHScrollPos * m_xChar);
  rcText.top = (nLineNo - m_nVScrollPos) * m_yChar;
  rcText.right = rcClient.right;
  rcText.bottom = rcText.top + m_yChar;
  DRAWTEXTPARAMS params;
  memset(&params, 0, sizeof(DRAWTEXTPARAMS));
  params.cbSize = sizeof(DRAWTEXTPARAMS);
  params.iTabLength = m_xChar*m_cchTabLength;

  DWORD dwFlags = DT_LEFT|DT_TOP|DT_SINGLELINE|DT_NOPREFIX|DT_EXPANDTABS;
	  
  DrawTextExA(hdc, (char*)ptr, dwLength-1,  &rcText, 
    dwFlags, &params);
}

void CFilePreviewCtrl::DoPaintEmpty(HDC hDC)
{
  RECT rcClient;
  GetClientRect(&rcClient);

  HFONT hOldFont = (HFONT)SelectObject(hDC, m_hFont);

  FillRect(hDC, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH));

  CRect rcText;
  DrawTextEx(hDC, m_sEmptyMsg.GetBuffer(0), -1, &rcText, DT_CALCRECT, NULL);

  rcText.MoveToX(rcClient.right/2-rcText.right/2);
  DrawTextEx(hDC, m_sEmptyMsg.GetBuffer(0), -1, &rcText, DT_LEFT, NULL);

  SelectObject(hDC, hOldFont);
}

void CFilePreviewCtrl::DoPaintText(HDC hDC)
{
  HFONT hOldFont = (HFONT)SelectObject(hDC, m_hFont);
  
  RECT rcClient;
  GetClientRect(&rcClient);

  HRGN hRgn = CreateRectRgn(0, 0, rcClient.right, rcClient.bottom);
  SelectClipRgn(hDC, hRgn);
	  
  FillRect(hDC, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH));

	int iPaintBeg = max(0, m_nVScrollPos);			//only update the lines that 
	int iPaintEnd = (int)min(m_uNumLines, m_nVScrollPos + rcClient.bottom / m_yChar);		//need updating!!!!!!!!!!!!!
	
  
	if(rcClient.bottom % m_yChar) iPaintEnd++;
	if(iPaintEnd > m_uNumLines) iPaintEnd--;
	
	//
	//	Only paint what needs to be!
	//
  int i;
	for(i = iPaintBeg; i < iPaintEnd; i++)
	{
    if(m_PreviewMode==PREVIEW_HEX)
      DrawHexLine(hDC, i);
    else
      DrawTextLine(hDC, i);				
	}
		
	SelectObject(hDC, hOldFont);
}

void CFilePreviewCtrl::DoPaintBitmap(HDC hDC)
{
  RECT rcClient;
  GetClientRect(&rcClient);

  HRGN hRgn = CreateRectRgn(0, 0, rcClient.right, rcClient.bottom);
  SelectClipRgn(hDC, hRgn);
	  
  FillRect(hDC, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH));

  if(m_bmp.IsValid())
  {
    m_bmp.Draw(hDC, &rcClient);
  }
  else
  {
    DoPaintEmpty(hDC);
  }
}

void CFilePreviewCtrl::DoPaint(HDC hDC)
{
  if(m_PreviewMode==PREVIEW_TEXT ||
     m_PreviewMode==PREVIEW_HEX)
  {
    DoPaintText(hDC);
  }
  else
  {
    DoPaintBitmap(hDC);
  }
}

LRESULT CFilePreviewCtrl::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
  m_fm.Destroy();
  bHandled = FALSE;  
  return 0;
}

LRESULT CFilePreviewCtrl::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	SetupScrollbars();

	InvalidateRect(NULL, FALSE);
	UpdateWindow();

  return 0;
}

LRESULT CFilePreviewCtrl::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  SCROLLINFO info;
	int nHScrollInc = 0;
	int  nOldHScrollPos = m_nHScrollPos;
		
	switch (LOWORD(wParam))
	{
	case SB_LEFT:
		m_nHScrollPos = 0;
		break;

	case SB_RIGHT:
		m_nHScrollPos = m_nHScrollMax + 1;
		break;

	case SB_LINELEFT:
		if(m_nHScrollPos > 0) --m_nHScrollPos;
		break;

	case SB_LINERIGHT:
		m_nHScrollPos++;
		break;

	case SB_PAGELEFT:
		m_nHScrollPos -= m_nMaxColsPerPage;
		if(m_nHScrollPos > nOldHScrollPos) 
      m_nHScrollPos = 0;
		break;

	case SB_PAGERIGHT:
		m_nHScrollPos += m_nMaxColsPerPage;
		break;

	case SB_THUMBPOSITION:
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_HORZ, &info);
		m_nHScrollPos = info.nTrackPos;
		break;

	case SB_THUMBTRACK:
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_HORZ, &info);
		m_nHScrollPos = info.nTrackPos;
		break;

	default:
		nHScrollInc = 0;
	}

	//keep scroll position in range
	if(m_nHScrollPos  > m_nHScrollMax - m_nMaxColsPerPage + 1)
		m_nHScrollPos = m_nHScrollMax - m_nMaxColsPerPage + 1;
  
  nHScrollInc = m_nHScrollPos - nOldHScrollPos;
		
	if (nHScrollInc)
	{	
		
		//finally setup the actual scrollbar!
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_POS;
		info.nPos = m_nHScrollPos;
		SetScrollInfo(SB_HORZ, &info, TRUE);
				
		InvalidateRect(NULL);
	}
		
  return 0;
}

LRESULT CFilePreviewCtrl::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  // React to the various vertical scroll related actions.
	// CAUTION:
	// All sizes are in unsigned values, so be carefull
	// when testing for < 0 etc

	SCROLLINFO info;
	int nVScrollInc;
	int  nOldVScrollPos = m_nVScrollPos;
		
	//HideCaret();

	switch (LOWORD(wParam))
	{
	case SB_TOP:
		m_nVScrollPos = 0;
		break;

	case SB_BOTTOM:
		m_nVScrollPos = m_nVScrollMax - m_nMaxLinesPerPage + 1;
		break;

	case SB_LINEUP:
		if(m_nVScrollPos > 0) --m_nVScrollPos;
		break;

	case SB_LINEDOWN:
		m_nVScrollPos++;
		break;

	case SB_PAGEUP:
		m_nVScrollPos -= max(1, m_nMaxLinesPerPage);
		if(m_nVScrollPos > nOldVScrollPos) m_nVScrollPos = 0;
		break;

	case SB_PAGEDOWN:
		m_nVScrollPos += max(1, m_nMaxLinesPerPage);
		break;

	case SB_THUMBPOSITION:
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &info);
		m_nVScrollPos = info.nTrackPos;
		break;

	case SB_THUMBTRACK:
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &info);
		m_nVScrollPos = info.nTrackPos;
		break;

	default:
		nVScrollInc = 0;
	}

	//keep scroll position in range
	if(m_nVScrollPos > m_nVScrollMax - m_nMaxLinesPerPage+1)
		m_nVScrollPos = m_nVScrollMax - m_nMaxLinesPerPage+1;

  if(m_nVScrollPos<0)
    m_nVScrollPos = 0;

	nVScrollInc = m_nVScrollPos - nOldVScrollPos;
		
	if (nVScrollInc)
	{	
		
		//finally setup the actual scrollbar!
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_POS;
		info.nPos = m_nVScrollPos;
		SetScrollInfo(SB_VERT, &info, TRUE);
				
		InvalidateRect(NULL);
	}
	
	return 0;
}

LRESULT CFilePreviewCtrl::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  // Do nothing
	return 0;   
}

LRESULT CFilePreviewCtrl::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{ 
  PAINTSTRUCT ps;
	HDC hDC = BeginPaint(&ps);

  {
    CMemoryDC memDC(hDC, ps.rcPaint);

    if(m_fm.GetSize()==0)
      DoPaintEmpty(memDC);
    else
      DoPaint(memDC);
  }
  	
	EndPaint(&ps);
	
  return 0;
}

LRESULT CFilePreviewCtrl::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  SetupScrollbars();
  InvalidateRect(NULL);
  return 0;
}

LRESULT CFilePreviewCtrl::OnComplete(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  KillTimer(0);
  SetupScrollbars();
  InvalidateRect(NULL);
  return 0;
}

LRESULT CFilePreviewCtrl::OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  NMHDR nmhdr;
  nmhdr.hwndFrom = m_hWnd;
  nmhdr.code = NM_RCLICK;
  nmhdr.idFrom = GetWindowLong(GWL_ID);

  HWND hWndParent = GetParent();
  ::SendMessage(hWndParent, WM_NOTIFY, 0, (LPARAM)&nmhdr);
  return 0;
}