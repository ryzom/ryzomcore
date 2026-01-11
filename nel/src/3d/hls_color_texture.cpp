// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "std3d.h"
#include "nel/3d/hls_color_texture.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/fasthls_modifier.h"
#include "nel/misc/stream.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/system_info.h"
#include "nel/misc/algo.h"


using	namespace std;
using	namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


#define	BLOCK_NUM_PIXEL		16
#define	BLOCK_DXTC_SIZE		16
#define	BLOCK_ALPHA_SIZE	16


// ***************************************************************************
void	CHLSColorDelta::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);
	f.serial(DHue, DLum, DSat);
}


// ***************************************************************************
void			CHLSColorTexture::CMask::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);

	f.serial(FullBlockIndex);
	f.serial(MixtBlockIndex);
	f.serialCont(Data);
}


// ***************************************************************************
void			CHLSColorTexture::CMask::setBit(uint bitId)
{
	uint8	&b= Data[bitId/8];
	b|= 1<<(bitId&7);
}


// ***************************************************************************
CHLSColorTexture::CHLSColorTexture()
{
	reset();
}

// ***************************************************************************
void			CHLSColorTexture::reset()
{
	_Width= 0;
	_Height= 0;
	_NumMipMap= 0;
	contReset(_Texture);
	contReset(_Masks);
}

// ***************************************************************************
void			CHLSColorTexture::setBitmap(const NLMISC::CBitmap &bmp)
{
	nlassert(bmp.getPixelFormat()==CBitmap::DXTC5);
	uint	width= bmp.getWidth();
	uint	height= bmp.getHeight();
	uint	mmCount= bmp.getMipMapCount();
	nlassert(width>=1 && height>=1);
	nlassert(mmCount>1 || width*height==1);

	// restart
	reset();

	// resize.
	uint	m;
	uint	pixelSize= 0;
	uint	numTotalBlock= 0;
	for(m=0;m<mmCount;m++)
	{
		pixelSize+= bmp.getPixels(m).size();
		uint	mmWidth= bmp.getWidth(m);
		uint	mmHeight= bmp.getHeight(m);
		uint	wBlock= (mmWidth+3)/4;
		uint	hBlock= (mmHeight+3)/4;
		numTotalBlock+= wBlock*hBlock;
	}
	// add the info for the "Block to compress"
	uint	blockToCompressSize= 4*((numTotalBlock+31)/32);
	// allocate good size, and reset to 0 => no block to re-compress.
	_Texture.resize(pixelSize+blockToCompressSize, 0);

	// fill texture
	uint8	*ptr= &_Texture[0];
	for(m=0;m<mmCount;m++)
	{
		uint	mSize= bmp.getPixels(m).size();
		memcpy(ptr, &bmp.getPixels(m)[0], mSize);
		ptr+= mSize;
	}

	// header
	_BlockToCompressIndex= pixelSize;
	_Width= width;
	_Height= height;
	_NumMipMap= mmCount;
}


// ***************************************************************************
#define	MASK_BLOCK_EMPTY	0
#define	MASK_BLOCK_FULL		1
#define	MASK_BLOCK_MIXT		2
struct	CMaskInfo
{
	// list of block
	uint				WBlock, HBlock;
	uint				NumBlock;
	vector<uint8>		Blocks;	// 0 empty, 1. Full. 2. Mixt.
};

// ***************************************************************************
void			CHLSColorTexture::addMask(const NLMISC::CBitmap &bmpIn, uint threshold)
{
	// copy the bitmap and set RGBA/mipmaps.
	CBitmap		bmp= bmpIn;
	bmp.convertToType(CBitmap::RGBA);
	bmp.buildMipMaps();

	// verify widht...
	nlassert(bmp.getWidth()== _Width);
	nlassert(bmp.getHeight()== _Height);
	nlassert(bmp.getMipMapCount()== _NumMipMap);

	// ***** build the information for all mipmaps
	vector<CMaskInfo>	masks;
	masks.resize(_NumMipMap);
	uint	m;
	uint	numMixtBlock= 0;
	uint	numTotalBlock= 0;
	for(m=0;m<_NumMipMap;m++)
	{
		CMaskInfo	&mask= masks[m];
		uint	mmWidth= bmp.getWidth(m);
		uint	mmHeight= bmp.getHeight(m);
		mask.WBlock= (mmWidth+3)/4;
		mask.HBlock= (mmHeight+3)/4;
		mask.NumBlock= mask.WBlock*mask.HBlock;
		mask.Blocks.resize(mask.NumBlock);

		numTotalBlock+= mask.NumBlock;

		CRGBA	*src= (CRGBA*)(&bmp.getPixels(m)[0]);

		for(uint yB=0;yB<mask.HBlock;yB++)
		{
			for(uint xB=0;xB<mask.WBlock;xB++)
			{
				uint	accum= 0;
				uint	w= min(mmWidth, 4U);
				uint	h= min(mmHeight, 4U);
				for(uint y= 0;y< h;y++)
				{
					for(uint x= 0;x< w;x++)
					{
						uint	yPix= yB*4+y;
						uint	xPix= xB*4+x;
						// read the color
						uint8	alphaMask = src[yPix*mmWidth+xPix].R;
						// remove some dummy precision.
						if(alphaMask<threshold)
							alphaMask= 0;
						if(alphaMask>255-threshold)
							alphaMask= 255;
						// Add to the accum
						accum+= alphaMask;
					}
				}

				// full black?
				if(accum==0)
					mask.Blocks[yB*mask.WBlock+xB]= MASK_BLOCK_EMPTY;
				else if(accum==w*h*255)
					mask.Blocks[yB*mask.WBlock+xB]= MASK_BLOCK_FULL;
				// if not full white or full black, mixt block
				else
				{
					mask.Blocks[yB*mask.WBlock+xB]= MASK_BLOCK_MIXT;
					numMixtBlock++;
				}
			}
		}
	}

	// ***** compress into CMask
	CMask		newMask;
	uint		newMaskDataSize= 0;

	// add the mixt block data size (16*uint8 per block)
	newMaskDataSize+= numMixtBlock*BLOCK_ALPHA_SIZE;
	// compute the bit size. NB: use uint32 to blocks bits. => data is aligned.
	uint	bitDataSize= 4*((numTotalBlock+31)/32);
	// add fullBlock bits
	newMask.FullBlockIndex= newMaskDataSize;
	newMaskDataSize+= bitDataSize;
	// add mixtBlock bits
	newMask.MixtBlockIndex= newMaskDataSize;
	newMaskDataSize+= bitDataSize;

	// allocate. Fill with 0 to initialize bits per default EMPTY value
	newMask.Data.resize(newMaskDataSize, 0);

	// compress each mipMaps from bigger to smaller
	uint	bitId= 0;
	uint	mixtBlockId= 0;
	for(m=0;m<_NumMipMap;m++)
	{
		CMaskInfo	&mask= masks[m];

		// ---- build the mixtBlock alpha Mask
		for(uint yB=0;yB<mask.HBlock;yB++)
		{
			for(uint xB=0;xB<mask.WBlock;xB++)
			{
				uint	id= yB*mask.WBlock+xB;
				// if mixt block
				if(mask.Blocks[id]==MASK_BLOCK_MIXT)
				{
					nlassert(mixtBlockId<numMixtBlock);
					// Fill Alpha data.
					uint8	*dst= &newMask.Data[mixtBlockId*BLOCK_ALPHA_SIZE];
					uint	mmWidth= bmp.getWidth(m);
					uint	mmHeight= bmp.getHeight(m);
					// point to the src alpha color
					CRGBA	*src= (CRGBA*)(&bmp.getPixels(m)[0]);
					src= src + yB*4*mmWidth + xB*4;

					// for the 4*4 pixels
					uint	w= min(mmWidth, 4U);
					uint	h= min(mmHeight, 4U);
					for(uint y=0;y<h;y++)
					{
						for(uint x=0;x<w;x++)
						{
							dst[y*4+x]= src[y*mmWidth+x].R;
						}
					}

					// inc
					mixtBlockId++;
				}
			}
		}

		// ---- build the fullBlock and mixtBlocks bits.
		for(uint i=0; i<mask.NumBlock; i++)
		{
			nlassert(bitId<numTotalBlock);

			// fill bits
			if(mask.Blocks[i]==MASK_BLOCK_FULL)
				newMask.setBit(newMask.FullBlockIndex*8 + bitId);
			else if(mask.Blocks[i]==MASK_BLOCK_MIXT)
				newMask.setBit(newMask.MixtBlockIndex*8 + bitId);

			// inc
			bitId++;
		}
	}

	// ***** Add the CMask
	_Masks.push_back(newMask);

	// Or the BlockToCompress info with the MixtBlocks bits.
	nlassert(bitDataSize==_Texture.size()-_BlockToCompressIndex);
	for(uint i=0;i<bitDataSize;i++)
	{
		_Texture[_BlockToCompressIndex+i]|= newMask.Data[newMask.MixtBlockIndex+i];
	}
}


// ***************************************************************************
void			CHLSColorTexture::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);

	f.serial(_Width, _Height, _NumMipMap, _BlockToCompressIndex);
	f.serialCont(_Texture);
	f.serialCont(_Masks);
}


// ***************************************************************************
static inline	void	getBitPack(uint32 *bitPtr, uint32 &bitMask)
{
#ifdef NL_LITTLE_ENDIAN
	bitMask= *bitPtr;
#else
	bitMask = ((uint8*)bitPtr)[0];
	bitMask+= ((uint8*)bitPtr)[1]<<8;
	bitMask+= ((uint8*)bitPtr)[2]<<16;
	bitMask+= ((uint8*)bitPtr)[3]<<24;
#endif
}

// ***************************************************************************
void			CHLSColorTexture::buildColorVersion(const CHLSColorDelta *colDeltaList, NLMISC::CBitmap &out)
{
	// static to avoid realloc
	static	vector<uint8>	dstTexture;
	static	vector<CRGBA>	dstUnCompTexture;
	uint32	*bitPtr;
	uint8	*srcPtr;
	uint8	*dstPtr;
	CRGBA	*dstUnCompPtr;
	uint32	bitMask;

	// **** prepare Data

	// count number of DXTC5 block in _Texture.
	uint	numBlocks= _BlockToCompressIndex/BLOCK_DXTC_SIZE;

	// create a tmp compressed block array, copy of Texture.
	dstTexture.resize(numBlocks*BLOCK_DXTC_SIZE);
	// copy from texture (to have non colored version already copied, and also ALPHA ok)
	memcpy(&dstTexture[0], &_Texture[0], dstTexture.size());

	// create a tmp uncompressed block array, which will receive coloring of mixt blocks
	dstUnCompTexture.resize(numBlocks*BLOCK_NUM_PIXEL);

	// For all blockToCompress, uncompress them in dstUnCompTexture, because they will blend with future mask coloring
	uint	n= numBlocks;
	bitPtr= (uint32*)(&_Texture[_BlockToCompressIndex]);
	dstUnCompPtr= &dstUnCompTexture[0];
	srcPtr= &_Texture[0];
	while(n>0)
	{
		uint	nBits= min(n, 32U);
		getBitPack(bitPtr, bitMask);
		n-= nBits;
		bitPtr++;
		for(;nBits>0;nBits--)
		{
			// need to compress/uncompress ??
			if(bitMask&1)
			{
				// uncompress this block. ignore alpha
				uncompressBlockRGB(srcPtr, dstUnCompPtr);
			}
			bitMask>>=1;
			dstUnCompPtr+= BLOCK_NUM_PIXEL;
			srcPtr+= BLOCK_DXTC_SIZE;
		}
	}

	// **** build the color version for all masks.

	for(uint maskId= 0; maskId<_Masks.size();maskId++)
	{
		CMask			&mask= _Masks[maskId];
		// unpack colDelta, and prepare for use with CFastHLSModifier.
		uint8			dHue= colDeltaList[maskId].DHue;
		uint			dLum= 0xFFFFFF00 + colDeltaList[maskId].DLum*2;
		uint			dSat= 0xFFFFFF00 + colDeltaList[maskId].DSat*2;

		// get a ptr on alpha of mixt block.
		uint8			*alphaMixtBlock= &mask.Data[0];


		// ---- for all Fullblock ot this mask, color and store in dstTexture
		// start at full Block bits desc
		bitPtr= (uint32*)(&mask.Data[mask.FullBlockIndex]);
		uint32	*bitCompPtr= (uint32*)(&_Texture[_BlockToCompressIndex]);
		srcPtr= &_Texture[0];
		dstPtr= &dstTexture[0];
		dstUnCompPtr= &dstUnCompTexture[0];
		n= numBlocks;
		// run all blocks.
		while(n>0)
		{
			uint	nBits= min(n, 32U);
			// get Full block mask.
			getBitPack(bitPtr, bitMask);
			n-= nBits;
			bitPtr++;
			// get Compress mask.
			uint32	bitCompMask;
			getBitPack(bitCompPtr, bitCompMask);
			bitCompPtr++;
			// for all bits
			for(;nBits>0;nBits--)
			{
				// need to colorize??
				if(bitMask&1)
				{
					// colorize this block. ignore alpha
					colorizeDXTCBlockRGB(srcPtr, dstPtr, dHue, dLum, dSat);
					// If this block is "a block to recompress", then must uncompress it in dstUnCompPtr
					uncompressBlockRGB(dstPtr, dstUnCompPtr);
				}
				bitMask>>=1;
				bitCompMask>>=1;
				srcPtr+= BLOCK_DXTC_SIZE;
				dstPtr+= BLOCK_DXTC_SIZE;
				dstUnCompPtr+= BLOCK_NUM_PIXEL;
			}
		}

		// ---- for all mixtblock ot this mask, color, uncompress and blend in store in dstUnCompTexture
		static	uint8	tmpColoredBlockDXTC[BLOCK_NUM_PIXEL];
		static	CRGBA	tmpColoredBlockRGBA[BLOCK_NUM_PIXEL];
		// start at mixt Block bits desc
		bitPtr= (uint32*)(&mask.Data[mask.MixtBlockIndex]);
		srcPtr= &_Texture[0];
		dstUnCompPtr= &dstUnCompTexture[0];
		n= numBlocks;
		// run all blocks.
		while(n>0)
		{
			uint	nBits= min(n, 32U);
			getBitPack(bitPtr, bitMask);
			n-= nBits;
			bitPtr++;
			for(;nBits>0;nBits--)
			{
				// need to colorize??
				if(bitMask&1)
				{
					// colorize this block. store 2 colors in tmp
					colorizeDXTCBlockRGB(srcPtr, tmpColoredBlockDXTC, dHue, dLum, dSat);
					// copy RGB bits from src to tmp
					((uint32*)tmpColoredBlockDXTC)[3]= ((uint32*)srcPtr)[3];

					// uncompress the block.
					uncompressBlockRGB(tmpColoredBlockDXTC, tmpColoredBlockRGBA);

					// blend tmpColoredBlockRGBA into dstUnCompPtr, according to alphaMixtBlock.
					for(uint i=0;i<16;i++)
					{
						dstUnCompPtr[i].blendFromuiRGBOnly(dstUnCompPtr[i], tmpColoredBlockRGBA[i], *alphaMixtBlock);
						// next pixel
						alphaMixtBlock++;
					}
				}
				bitMask>>=1;
				srcPtr+= BLOCK_DXTC_SIZE;
				dstUnCompPtr+= BLOCK_NUM_PIXEL;
			}
		}

	}


	// Since colorizeDXTCBlockRGB() use MMX, must end with emms.
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	if(CSystemInfo::hasMMX())
		_asm	emms;
#endif


	// **** compress needed blocks
	n= numBlocks;
	bitPtr= (uint32*)(&_Texture[_BlockToCompressIndex]);
	dstUnCompPtr= &dstUnCompTexture[0];
	dstPtr= &dstTexture[0];
	while(n>0)
	{
		uint	nBits= min(n, 32U);
		getBitPack(bitPtr, bitMask);
		n-= nBits;
		bitPtr++;
		for(;nBits>0;nBits--)
		{
			// need to compress ??
			if(bitMask&1)
			{
				// uncompress this block. ignore alpha
				compressBlockRGB(dstUnCompPtr, dstPtr);
			}
			bitMask>>=1;
			dstUnCompPtr+= BLOCK_NUM_PIXEL;
			dstPtr+= BLOCK_DXTC_SIZE;
		}
	}

	// **** format bitmap out with dstTexture.
	out.reset(CBitmap::DXTC5);
	out.resize(_Width, _Height, CBitmap::DXTC5);

	// create and fill all the mipMaps
	uint	w= _Width, h=_Height;
	dstPtr= &dstTexture[0];
	for(uint m=0;m<_NumMipMap;m++)
	{
		// allocate.
		out.resizeMipMap(m, w, h);
		// get the size of this DXTC5 level.
		uint	size= out.getPixels(m).size();
		// fill
		memcpy(&out.getPixels(m)[0], dstPtr, size);
		// next mipmap
		dstPtr+= size;
		w= (w+1)/2;
		h= (h+1)/2;
	}
	// verify all filled
	nlassert( dstPtr== (&dstTexture[0] + dstTexture.size()) );

	// set the correct num of mipmap
	out.setMipMapCount(_NumMipMap);
}


// ***************************************************************************
void			CHLSColorTexture::colorizeDXTCBlockRGB(const uint8 *srcPtr, uint8 *dstPtr, uint8 dHue, uint dLum, uint dSat)
{
	// get modifier.
	CFastHLSModifier	&fastHLS= CFastHLSModifier::getInstance();

	// apply the color on the 2 DXTC colors
	*(uint16*)(dstPtr+8 )= fastHLS.applyHLSMod(*(uint16*)(srcPtr+8 ) , dHue, dLum, dSat);
	*(uint16*)(dstPtr+10)= fastHLS.applyHLSMod(*(uint16*)(srcPtr+10) , dHue, dLum, dSat);
}


// ***************************************************************************
void			CHLSColorTexture::uncompressBlockRGB(const uint8* srcDXTC, CRGBA *dstRGBA)
{
	CRGBA	c[4];

	uint16 color0;
	uint16 color1;
	uint32 bits;
	color0= *(uint16*)(srcDXTC+8);
	color1= *(uint16*)(srcDXTC+10);
	bits=   *(uint32*)(srcDXTC+12);

	c[0].set565(color0);
	c[1].set565(color1);

	// ignore color0>color1 for DXT3 and DXT5.
	c[2].blendFromui(c[0],c[1],85);
	c[3].blendFromui(c[0],c[1],171);

	// bits to color (ignore alpha result)
	for(uint n= 16;n>0;n--)
	{
		*dstRGBA= c[bits&3];
		bits>>=2;
		dstRGBA++;
	}
}


// ***************************************************************************
void		CHLSColorTexture::computeMinMax(sint *diffBlock, CVectorInt &v, sint mean[3], sint rgb0[3], sint rgb1[3])
{
	// compute the min and max distance along the axis v.
	sint	mind= INT_MAX;
	sint	maxd= INT_MIN;
	sint	*srcDiff= diffBlock;
	// for the 16 pixels
	for(uint n=16;n>0;n--,srcDiff+=3)
	{
		sint	R= srcDiff[0];
		sint	G= srcDiff[1];
		sint	B= srcDiff[2];
		sint	d= R*v.x + G*v.y + B*v.z;
		if(d<mind)
			mind= d;
		if(d>maxd)
			maxd= d;
	}

	// avoid overflow. here, Higher possible bit is 16+8+2 (add of 3 values=> *4) == 26
	// 26-12= 14. 14+16=30 => ok.
	mind>>= 12;
	maxd>>= 12;

	// compute the 2 colors: rgb0 on the min, and rgb1 on the max
	rgb0[0]= mean[0]+ (mind*v.x>>20);
	rgb0[1]= mean[1]+ (mind*v.y>>20);
	rgb0[2]= mean[2]+ (mind*v.z>>20);
	rgb1[0]= mean[0]+ (maxd*v.x>>20);
	rgb1[1]= mean[1]+ (maxd*v.y>>20);
	rgb1[2]= mean[2]+ (maxd*v.z>>20);
	// clamp to 0..255
	fastClamp8(rgb0[0]);
	fastClamp8(rgb0[1]);
	fastClamp8(rgb0[2]);
	fastClamp8(rgb1[0]);
	fastClamp8(rgb1[1]);
	fastClamp8(rgb1[2]);
}


// ***************************************************************************
void			CHLSColorTexture::compressBlockRGB(CRGBA *srcRGBA, uint8* dstDXTC)
{
	// skip alpha part.
	uint8	*dstBlock= dstDXTC+8;


	// **** compute RGB0 and RGB1.
	uint	i,j,n;

	// compute the mean color of 16 pixels
	sint	mean[3];
	mean[0]= 0;
	mean[1]= 0;
	mean[2]= 0;
	CRGBA	*src= srcRGBA;
	for(n=16;n>0;n--,src++)
	{
		mean[0]+= src->R;
		mean[1]+= src->G;
		mean[2]+= src->B;
		// at same time, setup alpha to 0. Important for "compute bits" part (see MMX)!!
		src->A= 0;
	}
	mean[0]>>= 4;
	mean[1]>>= 4;
	mean[2]>>= 4;

	// compute col-mean
	sint	diffBlock[16*3];
	src= srcRGBA;
	sint	*srcDiff= diffBlock;
	for(n=16;n>0;n--,src++,srcDiff+=3)
	{
		srcDiff[0]= (sint)src->R - mean[0];
		srcDiff[1]= (sint)src->G - mean[1];
		srcDiff[2]= (sint)src->B - mean[2];
	}


	// compute the covariant matrix.
	sint	coMat[3][3];
	// Apply std RGB factor (0.3, 0.56, 0.14) to choose the best Axis. This give far much best results.
	sint	rgbFact[3]= {77, 143, 36};
	for(i=0;i<3;i++)
	{
		// OPTIMIZE SINCE SYMETRIX MATRIX
		for(j=i;j<3;j++)
		{
			sint32	factor= 0;
			// divide / 16 to avoid overflow sint32
			uint	colFactor= (rgbFact[i]*rgbFact[j]) >> 4;
			// run all 16 pixels.
			sint	*srcDiff= diffBlock;
			for(n=16;n>0;n--,srcDiff+=3)
			{
				factor+= srcDiff[i] * srcDiff[j] * colFactor;
			}
			coMat[i][j]= factor;
		}
	}
	// Fill symetrix matrix
	coMat[1][0]= coMat[0][1];
	coMat[2][0]= coMat[0][2];
	coMat[2][1]= coMat[1][2];


	// take the bigger vector
	sint	maxSize= 0;
	uint	axis= 0;
	for(i=0;i<3;i++)
	{
		// Use abs since sqr fails because all sint32 range may be used.
		sint	size= abs(coMat[i][0]) + abs(coMat[i][1]) + abs(coMat[i][2]);
		if(size>maxSize)
		{
			maxSize= size;
			axis= i;
		}
	}

	// normalize this vector
	CVector	v;
	// remove some rgb factor...
	v.x= (float)coMat[axis][0]/rgbFact[0];
	v.y= (float)coMat[axis][1]/rgbFact[1];
	v.z= (float)coMat[axis][2]/rgbFact[2];
	v.normalize();
	// set a Fixed 16:16.
	CVectorInt	vInt;
	// don't bother if OptFastFloorBegin() has been called. 16:16 precision is sufficient.
	vInt.x= OptFastFloor(v.x*65536);
	vInt.y= OptFastFloor(v.y*65536);
	vInt.z= OptFastFloor(v.z*65536);


	// For all pixels, choose the 2 colors along the axis
	sint	rgb0[3];
	sint	rgb1[3];
	computeMinMax(diffBlock, vInt, mean, rgb0, rgb1);

	// Average to 16 bits. NB: correclty encode 0..255 to 0.31 or 0..63.
	uint	R,G,B;
	R= ((rgb0[0]*7967+32768)>>16);
	G= ((rgb0[1]*16191+32768)>>16);
	B= ((rgb0[2]*7967+32768)>>16);
	uint16	rgb016= (R<<11) + (G<<5) + (B);
	R= ((rgb1[0]*7967+32768)>>16);
	G= ((rgb1[1]*16191+32768)>>16);
	B= ((rgb1[2]*7967+32768)>>16);
	uint16	rgb116= (R<<11) + (G<<5) + (B);
	// copy to block
	((uint16*)dstBlock)[0]= rgb016;
	((uint16*)dstBlock)[1]= rgb116;


	// **** compute bits
	CRGBA	c[4];
	c[0].set565(rgb016);
	c[1].set565(rgb116);
	c[2].blendFromui(c[0],c[1],85);
	c[3].blendFromui(c[0],c[1],171);
	// it is important that c[] and src Alpha are set to 0, because of "pmaddwd" use in MMX code...
	c[0].A= 0;
	c[1].A= 0;
	c[2].A= 0;
	c[3].A= 0;
	CRGBA	*cPtr= c;

	// result.
	uint32	bits= 0;

#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	if(CSystemInfo::hasMMX())
	{
		// preapre mmx
		uint64	blank= 0;
		__asm
		{
			movq		mm7, blank
		}

		// for 16 pixels
		src= srcRGBA;
		for(n=16;n>0;n--,src++)
		{
			/* // C Version (+ little asm).
			uint	minDist= 0xFFFFFFFF;
			uint	id= 0;
			for(i=0;i<4;i++)
			{
				// applying factors such *23, *80, *6 gives better results, but slower (in MMX).
				uint	dist= sqr((sint)src->R-(sint)c[i].R);
				dist+= sqr((sint)src->G-(sint)c[i].G);
				dist+= sqr((sint)src->B-(sint)c[i].B);
				if(dist<minDist)
				{
					minDist= dist;
					id= i;
				}
			}
			bits|=id;
			__asm
			{
				mov	eax, bits
				ror eax, 2
				mov bits, eax
			}*/
			__asm
			{
				mov			esi, src
				mov			edi, cPtr

				mov			ecx, 4
				mov			edx, 0xFFFFFFFF	// edx= minDist

				movd		mm0, [esi]
				punpcklbw	mm0, mm7

				mov			esi, 4			// esi= id MinDist (inverted)

				// compare 4 cases.
			myLoop:
				movd		mm1, [edi]
				punpcklbw	mm1, mm7
				psubsw		mm1, mm0
				pmaddwd		mm1, mm1
				movd		eax, mm1
				psrlq       mm1, 32
				movd		ebx, mm1
				add			eax, ebx

				// take smaller of A and B. here: eax= A, edx= B
				sub			eax, edx		// eax= A-B
				sbb			ebx, ebx		// ebx= FF if A<B.
				and			eax, ebx		// eax= A-B if A<B
				add			edx, eax		// if A<B, edx= B+A-B= A, else, edx= B. => minimum
				// setup the "smaller" id. here esi= iB, ecx= iA
				not			ebx				// ebx= 0 if A<B, FF else
				sub			esi, ecx		// esi= iB-iA
				and			esi, ebx		// esi= 0 if A<B, iB-iA else
				add			esi, ecx		// esi= 0+iA= iA if A<B, else esi= iB-iA+iA= iB

				add			edi, 4
				dec			ecx
				jnz			myLoop

				// reverse id
				mov			edx, 4
				mov			eax, bits
				sub			edx, esi
				// and store into bits
				or			eax, edx
				ror			eax, 2
				mov			bits, eax
			}
		}


		// end MMX block.
		__asm	emms;
	}
	else
#endif	// NL_OS_WINDOWS
	{
		src= srcRGBA;
		for(n=16;n>0;n--,src++)
		{
			// C Version (+ little asm).
			uint	minDist= 0xFFFFFFFF;
			uint	id= 0;
			for(i=0;i<4;i++)
			{
				// applying factors such *23, *80, *6 gives better results, but slower (in MMX).
				uint	dist= sqr((sint)src->R-(sint)c[i].R);
				dist+= sqr((sint)src->G-(sint)c[i].G);
				dist+= sqr((sint)src->B-(sint)c[i].B);
				if(dist<minDist)
				{
					minDist= dist;
					id= i;
				}
			}
			// a ror is faster, but full C version
			bits|= id<<30;
			// don't do it for the last.
			if(n>1)
				bits>>=2;
		}
	}

	// copy
	((uint32*)dstBlock)[1]= bits;
}


} // NL3D
