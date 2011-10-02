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

#include "stdafx.h"
#include "BlackBox.h"
#include "jpeglib.h"
#include <setjmp.h>

CBlackBox::CBlackBox()
{
  ZeroMemory(&m_Info, sizeof(CR_BLACK_BOX_INFO));
}

CBlackBox::~CBlackBox()
{
  Destroy();
}

BOOL CBlackBox::Init(PCR_BLACK_BOX_INFO pInfo)
{
  if(pInfo==NULL || pInfo->cb!=sizeof(CR_BLACK_BOX_INFO))
    return FALSE;

  memcpy(&m_Info, pInfo, sizeof(CR_BLACK_BOX_INFO));

  return TRUE;
}

void CBlackBox::Destroy()
{
}

DWORD WINAPI CBlackBox::ThreadProc(LPVOID /*lpParam*/)
{
  return 0;
}

BOOL CBlackBox::JpegWrite(CString sFileName)
{
  int image_width = 0;
  int image_height = 0;
  int quality = 0;
  JSAMPLE * image_buffer = NULL;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */

  cinfo.err = jpeg_std_error(&jerr);  
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */

  _tfopen_s(&outfile, sFileName, _T("wb"));
  if(outfile==NULL)
    return FALSE;

  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  cinfo.image_width = image_width; 	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) 
  {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */
  jpeg_finish_compress(&cinfo);

  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);
  
  return TRUE;
}

