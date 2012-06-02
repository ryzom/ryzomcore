// Md5.cpp: implementation of the CMd5 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Md5.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define FF(a, b, c, d, x, s, ac) { \
  (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
 }
 
#define GG(a, b, c, d, x, s, ac) { \
  (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
 }
 
#define HH(a, b, c, d, x, s, ac) { \
  (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
 }
 
#define II(a, b, c, d, x, s, ac) { \
  (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
 }
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define MD5_PAD_FIRST_BYTE 0x80 // The first byte of padding.
#define MD5_PAD_SIZE    56      // The padding function pads out to 56 bytes.

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
 
#define S11 7
#define S12 12
#define S13 17
#define S14 22
 
#define S21 5
#define S22 9
#define S23 14
#define S24 20
 
#define S31 4
#define S32 11
#define S33 16
#define S34 23
 
#define S41 6
#define S42 10
#define S43 15
#define S44 21
 
#define T1  0xd76aa478
#define T2  0xe8c7b756
#define T3  0x242070db
#define T4  0xc1bdceee
#define T5  0xf57c0faf
#define T6  0x4787c62a
#define T7  0xa8304613
#define T8  0xfd469501
#define T9  0x698098d8
#define T10 0x8b44f7af
#define T11 0xffff5bb1
#define T12 0x895cd7be
#define T13 0x6b901122
#define T14 0xfd987193
#define T15 0xa679438e
#define T16 0x49b40821
   
#define T17 0xf61e2562
#define T18 0xc040b340
#define T19 0x265e5a51
#define T20 0xe9b6c7aa
#define T21 0xd62f105d
#define T22 0x02441453
#define T23 0xd8a1e681
#define T24 0xe7d3fbc8
#define T25 0x21e1cde6
#define T26 0xc33707d6
#define T27 0xf4d50d87
#define T28 0x455a14ed
#define T29 0xa9e3e905
#define T30 0xfcefa3f8
#define T31 0x676f02d9
#define T32 0x8d2a4c8a
 
#define T33 0xfffa3942
#define T34 0x8771f681
#define T35 0x6d9d6122
#define T36 0xfde5380c
#define T37 0xa4beea44
#define T38 0x4bdecfa9
#define T39 0xf6bb4b60
#define T40 0xbebfbc70
#define T41 0x289b7ec6
#define T42 0xeaa127fa
#define T43 0xd4ef3085
#define T44 0x04881d05
#define T45 0xd9d4d039
#define T46 0xe6db99e5
#define T47 0x1fa27cf8
#define T48 0xc4ac5665
 
#define T49 0xf4292244
#define T50 0x432aff97
#define T51 0xab9423a7
#define T52 0xfc93a039
#define T53 0x655b59c3
#define T54 0x8f0ccc92
#define T55 0xffeff47d
#define T56 0x85845dd1
#define T57 0x6fa87e4f
#define T58 0xfe2ce6e0
#define T59 0xa3014314
#define T60 0x4e0811a1
#define T61 0xf7537e82
#define T62 0xbd3af235
#define T63 0x2ad7d2bb
#define T64 0xeb86d391
 
#define A_INIT 0x67452301
#define B_INIT 0xefcdab89
#define C_INIT 0x98badcfe
#define D_INIT 0x10325476


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMD5::CMD5()
{
}

CMD5::CMD5(CFile& InputFile)
{
 // Initialize the digest.
 a = A_INIT;
 b = B_INIT;
 c = C_INIT;
 d = D_INIT;
 
 // Now init the bit length of the input.
 count[0] = 0;
 count[1] = 0;
 
 // Now perform transformation on every block of input.
 do
  {
  // Read a block from the input.
  block_idx = InputFile.Read( manip_block, MD5_BLOCK_SIZE);
  
  if (block_idx == MD5_BLOCK_SIZE)
   {
   // A full block is ready to be transformed.  Do so.
   MD5_IncCount( MD5_BLOCK_SIZE );
   MD5_Xform();
   }
  else
   {
   MD5_IncCount( block_idx );
   
   // If the count of bytes read is not a full block,
   // the end of the file has been found, and the ending should be prepared.
   if (block_idx < (MD5_PAD_SIZE) )
    {
    // There is enough room in the block, to pad up to the correct size.
    // Pad and then transform.
    MD5_Pad( normal );
    MD5_Xform();
    }
   else
    {
    // The block is already up to or beyond the pad size. Pad to the
    // end and of the current block, and then transform it.
    MD5_Pad( to_end );
    MD5_Xform();
    
    // Now create another block that is empty and pad all the way.
    block_idx = 0;
    MD5_Pad( finish );
    MD5_Xform();
    }
   }
  } while (block_idx == MD5_BLOCK_SIZE);
}

void CMD5::SetContent(CString csContent)
{
	int	iOffset	= 0;

	// Initialize the digest.
	a = A_INIT;
	b = B_INIT;
	c = C_INIT;
	d = D_INIT;

	// Now init the bit length of the input.
	count[0] = 0;
	count[1] = 0;

	// Now perform transformation on every block of input.
	do
	{
		strcpy((char*)manip_block, csContent.Mid(iOffset, MD5_BLOCK_SIZE));
		block_idx	= strlen((const char*)manip_block);

		if (block_idx == MD5_BLOCK_SIZE)
		{
			// A full block is ready to be transformed.  Do so.
			MD5_IncCount(MD5_BLOCK_SIZE);
			MD5_Xform();
		}
		else
		{
			MD5_IncCount( block_idx );

			// If the count of bytes read is not a full block,
			// the end of the file has been found, and the ending should be prepared.
			if (block_idx < (MD5_PAD_SIZE) )
			{
				// There is enough room in the block, to pad up to the correct size.
				// Pad and then transform.
				MD5_Pad( normal );
				MD5_Xform();
			}
			else
			{
				// The block is already up to or beyond the pad size. Pad to the
				// end and of the current block, and then transform it.
				MD5_Pad( to_end );
				MD5_Xform();

				// Now create another block that is empty and pad all the way.
				block_idx = 0;
				MD5_Pad( finish );
				MD5_Xform();
			}
		}
	} 
	while (block_idx == MD5_BLOCK_SIZE);
}

void CMD5::MD5_Pad( MD5_PadMode PadMode )
{
	switch( PadMode )
	{
		case normal:
			// Perform normal padding.
			manip_block[block_idx++] = MD5_PAD_FIRST_BYTE;
			while (block_idx < MD5_PAD_SIZE)
			manip_block[block_idx++] = 0;

			// Now store count in this last block.
			MD5_StoreCnt();
			break;

		case to_end:
			// Pad until the end of the block.
			manip_block[block_idx++] = MD5_PAD_FIRST_BYTE;
			while (block_idx < MD5_BLOCK_SIZE)
			manip_block[block_idx++] = 0;
			break;

		case finish:
			// Finish padding.
			while (block_idx < MD5_PAD_SIZE)
			manip_block[block_idx++] = 0;

			// Now store count in this last block.
			MD5_StoreCnt();
			break;
	}
}
  
void CMD5::MD5_Xform()
{
 // Save orig values of a,b,c,d.
 UINT4 a_orig = a;
 UINT4 b_orig = b;
 UINT4 c_orig = c;
 UINT4 d_orig = d;
 
 // Now decode the manip_block into xform_buf;
 MD5_Decode();
 
 // Round 1.
 // Let [abcd k s i] denote the operation
 // a = b + ((a + F(b,c,d) + xform_buf[k] + T[i]) <<< s).
 // Do the following 16 operations.
 //  [ABCD  0  7  1]  [DABC  1 12  2]  [CDAB  2 17  3]  [BCDA  3 22  4]
 //  [ABCD  4  7  5]  [DABC  5 12  6]  [CDAB  6 17  7]  [BCDA  7 22  8]
 //  [ABCD  8  7  9]  [DABC  9 12 10]  [CDAB 10 17 11]  [BCDA 11 22 12]
 //  [ABCD 12  7 13]  [DABC 13 12 14]  [CDAB 14 17 15]  [BCDA 15 22 16]
 
 FF (a, b, c, d, xform_buf[ 0], S11, T1);
 FF (d, a, b, c, xform_buf[ 1], S12, T2);
 FF (c, d, a, b, xform_buf[ 2], S13, T3);
 FF (b, c, d, a, xform_buf[ 3], S14, T4);
 FF (a, b, c, d, xform_buf[ 4], S11, T5);
 FF (d, a, b, c, xform_buf[ 5], S12, T6);
 FF (c, d, a, b, xform_buf[ 6], S13, T7);
 FF (b, c, d, a, xform_buf[ 7], S14, T8);
 FF (a, b, c, d, xform_buf[ 8], S11, T9);
 FF (d, a, b, c, xform_buf[ 9], S12, T10);
 FF (c, d, a, b, xform_buf[10], S13, T11);
 FF (b, c, d, a, xform_buf[11], S14, T12);
 FF (a, b, c, d, xform_buf[12], S11, T13);
 FF (d, a, b, c, xform_buf[13], S12, T14);
 FF (c, d, a, b, xform_buf[14], S13, T15);
 FF (b, c, d, a, xform_buf[15], S14, T16);
   
 // Round 2.
 // Let [abcd k s i] denote the operation
 // a = b + ((a + G(b,c,d) + xform_buf[k] + T[i]) <<< s).
 // Do the following 16 operations.
 //  [ABCD  1  5 17]  [DABC  6  9 18]  [CDAB 11 14 19]  [BCDA  0 20 20]
 //  [ABCD  5  5 21]  [DABC 10  9 22]  [CDAB 15 14 23]  [BCDA  4 20 24]
 //  [ABCD  9  5 25]  [DABC 14  9 26]  [CDAB  3 14 27]  [BCDA  8 20 28]
 //  [ABCD 13  5 29]  [DABC  2  9 30]  [CDAB  7 14 31]  [BCDA 12 20 32]
 
 GG (a, b, c, d, xform_buf[ 1], S21, T17);
 GG (d, a, b, c, xform_buf[ 6], S22, T18);
 GG (c, d, a, b, xform_buf[11], S23, T19);
 GG (b, c, d, a, xform_buf[ 0], S24, T20);
 GG (a, b, c, d, xform_buf[ 5], S21, T21);
 GG (d, a, b, c, xform_buf[10], S22, T22);
 GG (c, d, a, b, xform_buf[15], S23, T23);
 GG (b, c, d, a, xform_buf[ 4], S24, T24);
 GG (a, b, c, d, xform_buf[ 9], S21, T25);
 GG (d, a, b, c, xform_buf[14], S22, T26);
 GG (c, d, a, b, xform_buf[ 3], S23, T27);
 GG (b, c, d, a, xform_buf[ 8], S24, T28);
 GG (a, b, c, d, xform_buf[13], S21, T29);
 GG (d, a, b, c, xform_buf[ 2], S22, T30);
 GG (c, d, a, b, xform_buf[ 7], S23, T31);
 GG (b, c, d, a, xform_buf[12], S24, T32);
   
 // Round 3.
 // Let [abcd k s t] denote the operation
 // a = b + ((a + H(b,c,d) + xform_buf[k] + T[i]) <<< s).
 // Do the following 16 operations.
 //  [ABCD  5  4 33]  [DABC  8 11 34]  [CDAB 11 16 35]  [BCDA 14 23 36]
 //  [ABCD  1  4 37]  [DABC  4 11 38]  [CDAB  7 16 39]  [BCDA 10 23 40]
 //  [ABCD 13  4 41]  [DABC  0 11 42]  [CDAB  3 16 43]  [BCDA  6 23 44]
 //  [ABCD  9  4 45]  [DABC 12 11 46]  [CDAB 15 16 47]  [BCDA  2 23 48]
 
 HH (a, b, c, d, xform_buf[ 5], S31, T33);
 HH (d, a, b, c, xform_buf[ 8], S32, T34);
 HH (c, d, a, b, xform_buf[11], S33, T35);
 HH (b, c, d, a, xform_buf[14], S34, T36);
 HH (a, b, c, d, xform_buf[ 1], S31, T37);
 HH (d, a, b, c, xform_buf[ 4], S32, T38);
 HH (c, d, a, b, xform_buf[ 7], S33, T39);
 HH (b, c, d, a, xform_buf[10], S34, T40);
 HH (a, b, c, d, xform_buf[13], S31, T41);
 HH (d, a, b, c, xform_buf[ 0], S32, T42);
 HH (c, d, a, b, xform_buf[ 3], S33, T43);
 HH (b, c, d, a, xform_buf[ 6], S34, T44);
 HH (a, b, c, d, xform_buf[ 9], S31, T45);
 HH (d, a, b, c, xform_buf[12], S32, T46);
 HH (c, d, a, b, xform_buf[15], S33, T47);
 HH (b, c, d, a, xform_buf[ 2], S34, T48);
 
 // Round 4.
 // Let [abcd k s t] denote the operation
 // a = b + ((a + I(b,c,d) + xform_buf[k] + T[i]) <<< s).
 // Do the following 16 operations.
 //  [ABCD  0  6 49]  [DABC  7 10 50]  [CDAB 14 15 51]  [BCDA  5 21 52]
 //  [ABCD 12  6 53]  [DABC  3 10 54]  [CDAB 10 15 55]  [BCDA  1 21 56]
 //  [ABCD  8  6 57]  [DABC 15 10 58]  [CDAB  6 15 59]  [BCDA 13 21 60]
 //  [ABCD  4  6 61]  [DABC 11 10 62]  [CDAB  2 15 63]  [BCDA  9 21 64]
 
 II (a, b, c, d, xform_buf[ 0], S41, T49);
 II (d, a, b, c, xform_buf[ 7], S42, T50);
 II (c, d, a, b, xform_buf[14], S43, T51);
 II (b, c, d, a, xform_buf[ 5], S44, T52);
 II (a, b, c, d, xform_buf[12], S41, T53);
 II (d, a, b, c, xform_buf[ 3], S42, T54);
 II (c, d, a, b, xform_buf[10], S43, T55);
 II (b, c, d, a, xform_buf[ 1], S44, T56);
 II (a, b, c, d, xform_buf[ 8], S41, T57);
 II (d, a, b, c, xform_buf[15], S42, T58);
 II (c, d, a, b, xform_buf[ 6], S43, T59);
 II (b, c, d, a, xform_buf[13], S44, T60);
 II (a, b, c, d, xform_buf[ 4], S41, T61);
 II (d, a, b, c, xform_buf[11], S42, T62);
 II (c, d, a, b, xform_buf[ 2], S43, T63);
 II (b, c, d, a, xform_buf[ 9], S44, T64);
 
 // Now perform the following additions. (That is increment each
 // of the four registers by the value it had before this block
 // was started.)
 a += a_orig;
 b += b_orig;
 c += c_orig;
 d += d_orig;
}
 
void CMD5::MD5_IncCount( int IncAmt)
{
	UINT4 old_cnt0	= count[0];
   
	count[0]	+= IncAmt*8;
	if(count[0] < old_cnt0) 
		count[1]++;      // Carry detection.
}
 
void CMD5::MD5_Decode()
{
	unsigned int i, j;

	for (i = 0, j = 0; j < MD5_BLOCK_SIZE; i++, j += sizeof(UINT4))
		xform_buf[i] = ((UINT4)manip_block[j]) | (((UINT4)manip_block[j+1]) << 8) |
		(((UINT4)manip_block[j+2]) << 16) | (((UINT4)manip_block[j+3]) << 24);
}
 
void CMD5::MD5_StoreCnt()
{
	manip_block[MD5_PAD_SIZE+0]= unsigned int( count[0] & 0xff);
	manip_block[MD5_PAD_SIZE+1]= unsigned int((count[0] >> 8)  & 0xff);
	manip_block[MD5_PAD_SIZE+2]= unsigned int((count[0] >> 16) & 0xff);
	manip_block[MD5_PAD_SIZE+3]= unsigned int((count[0] >> 24) & 0xff);

	manip_block[MD5_PAD_SIZE+4]= unsigned int(count[1] & 0xff);
	manip_block[MD5_PAD_SIZE+5]= unsigned int((count[1] >> 8)  & 0xff);
	manip_block[MD5_PAD_SIZE+6]= unsigned int((count[1] >> 16) & 0xff);
	manip_block[MD5_PAD_SIZE+7]= unsigned int((count[1] >> 24) & 0xff);
}
 
void CMD5::GetDigest( unsigned char* Dest)
{
	Dest[0] = unsigned int(a & 0xff);
	Dest[1] = unsigned int((a >> 8) & 0xff);
	Dest[2] = unsigned int((a >> 16) & 0xff);
	Dest[3] = unsigned int((a >> 24) & 0xff);

	Dest[4] = unsigned int(b & 0xff);
	Dest[5] = unsigned int((b >> 8) & 0xff);
	Dest[6] = unsigned int((b >> 16) & 0xff);
	Dest[7] = unsigned int((b >> 24) & 0xff);

	Dest[8] = unsigned int(c & 0xff);
	Dest[9] = unsigned int((c >> 8) & 0xff);
	Dest[10] = unsigned int((c >> 16) & 0xff);
	Dest[11] = unsigned int((c >> 24) & 0xff);

	Dest[12] = unsigned int(d & 0xff);
	Dest[13] = unsigned int((d >> 8) & 0xff);
	Dest[14] = unsigned int((d >> 16) & 0xff);
	Dest[15] = unsigned int((d >> 24) & 0xff);
}

CString CMD5::ConvertToAscii(unsigned char* lpszBuff)
{
	CString	csRet;
	CString csHex	= "0123456789abcdef";
	int	i, j;
	unsigned long	num;
	unsigned long*	plBuff	= (unsigned long*)lpszBuff;

	for(i = 0; i < 4; i++)
	{
		num	= plBuff[i];
		for(j = 0; j < 4; j++)
		{
			csRet	+= csHex[int((num >> (j * 8 + 4)) & 0x0F)];
			csRet	+= csHex[int((num >> (j * 8)) & 0x0F)];
		}
	}
	return csRet;
}

