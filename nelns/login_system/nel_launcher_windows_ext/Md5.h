// Md5.h: interface for the CMd5 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MD5_H__02391EF3_6296_4823_BC8C_D8B3DFE719B2__INCLUDED_)
#define AFX_MD5_H__02391EF3_6296_4823_BC8C_D8B3DFE719B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define UINT4			unsigned long

#define MD5_BLOCK_SIZE  64      // Operate on the input in 64 byte chunks.
#define MD5_COUNT_SIZE   2      // The bit cnt of the input is this big.
#define MD5_XFORM_SIZE  16      // The xform buffer is this long.

enum MD5_PadMode{normal, to_end, finish};

class CMD5  
{
private:
	// Operate on the input file one block at a time.
	unsigned char manip_block[MD5_BLOCK_SIZE];

	// Index to next empty manip_block position.
	// If block_idx == MD5_BLOCK_SIZE, manip_block is full.
	int block_idx;

	// The bit count of the input file, mod 2^64, lsb first.
	UINT4 count[MD5_COUNT_SIZE];

	// The A,B,C,D, components of the digest in the making.
	UINT4 a,b,c,d;

	// The manip_block is converted into this array of unsigned integers
	// to me used by the transformation routine.
	UINT4 xform_buf[MD5_XFORM_SIZE];
    
public:
	CMD5();
	CMD5(CFile& InputFile);
	void SetContent(CString csContent);	
	void GetDigest(unsigned char*);		// Pointer to an array 16 bytes long.  Large enough to hold 128 bits of digest, ls byte first.
	CString ConvertToAscii(unsigned char* lpszBuff);

	// Helper functions
private:
	void MD5_Decode();			// Convert a manip_block of data from the input to the xform_buf to be transformed.
	void MD5_IncCount(int);		// Keep track of the bit length of the input, as successive blocks are read.
	void MD5_Pad(MD5_PadMode);	// Perform padding functions.
	void MD5_StoreCnt();		// Store the bit length of the input into the current manip_block.
	void MD5_Xform();			// Do the transformation.
};

#endif // !defined(AFX_MD5_H__02391EF3_6296_4823_BC8C_D8B3DFE719B2__INCLUDED_)
