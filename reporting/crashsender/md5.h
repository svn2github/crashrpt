
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

/*
 *	This is the C++ implementation of the MD5 Message-Digest
 *	Algorithm desrcipted in RFC 1321.
 *	I translated the C code from this RFC to C++.
 *	There is no warranty.
 *
 *	Feb. 12. 2005
 *	Benjamin Grüdelbach
 */

/*
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 * 
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 * 
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 * 
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 * 
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */

//---------------------------------------------------------------------- 
//include protection
#ifndef MD5_H
#define MD5_H

//---------------------------------------------------------------------- 
//STL includes
#include <string>

//---------------------------------------------------------------------- 
//typedefs
typedef unsigned char *POINTER;

/*
 * MD5 context.
 */
typedef struct 
{
	unsigned long int state[4];   	      /* state (ABCD) */
	unsigned long int count[2]; 	      /* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];	      /* input buffer */
} MD5_CTX;

/*
 * MD5 class
 */
class MD5
{

	private:

		void MD5Transform (unsigned long int state[4], unsigned char block[64]);
		void Encode (unsigned char*, unsigned long int*, unsigned int);
		void Decode (unsigned long int*, unsigned char*, unsigned int);
		void MD5_memcpy (POINTER, POINTER, unsigned int);
		void MD5_memset (POINTER, int, unsigned int);

	public:
	
		void MD5Init (MD5_CTX*);
		void MD5Update (MD5_CTX*, unsigned char*, unsigned int);
		void MD5Final (unsigned char [16], MD5_CTX*);

	MD5(){};
};

//---------------------------------------------------------------------- 
//End of include protection
#endif

/*
 * EOF
 */
