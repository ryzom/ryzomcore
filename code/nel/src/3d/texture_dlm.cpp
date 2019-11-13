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

#include "nel/3d/texture_dlm.h"
#include "nel/misc/common.h"
#include "nel/misc/fast_mem.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
CTextureDLM::CTextureDLM(uint width, uint height)
{
	nlassert(width>=NL_DLM_BLOCK_SIZE);
	nlassert(height>=NL_DLM_BLOCK_SIZE);
	nlassert(NLMISC::isPowerOf2(width));
	nlassert(NLMISC::isPowerOf2(height));

	// verify there is sufficient blocks.
	_WBlock= width/NL_DLM_BLOCK_SIZE;
	uint	nBlocks= _WBlock * (height/NL_DLM_BLOCK_SIZE);
	nlassert(nBlocks>=NL_DLM_LIGHTMAP_TYPE_SIZE);


	// The DLM texture always reside in memory...
	// NB: this is simplier like that, and this is not a problem, since actually only 256Ko is allocated :o)
	setReleasable(false);
	// create the bitmap.
	CBitmap::resize(width, height, CBitmap::RGBA);
	// Format of texture, 32 bits and no mipmaps.
	// NB: 16 bits is not a good idea, because implies lot of flicking
	setUploadFormat(ITexture::RGBA8888);
	setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);

	// Fill the array of blocks.
	_Blocks.resize(nBlocks);
	_EmptyBlocks.resize(nBlocks);
	uint i;
	for(i = 0; i < _Blocks.size(); i++)
	{
		// compute position of the block in the texture
		_Blocks[i].PosX = (i%_WBlock) * NL_DLM_BLOCK_SIZE;
		_Blocks[i].PosY = (i/_WBlock) * NL_DLM_BLOCK_SIZE;

		// This block is free!!
		_EmptyBlocks[i] = i;
	}

	// init list to NULL.
	for(i = 0; i < NL_DLM_LIGHTMAP_TYPE_SIZE; i++)
	{
		_FreeBlocks[i] = NULL;
	}

	// Since NL_DLM_BLOCK_SIZE is 10 or 18 (a factor of prime number 5 or 3 respectively), we are sure there is
	// at least one pixel which is not used by blcks. The last pixel is filled with black (see CTextureDLm doc)
	nlassert(NL_DLM_BLOCK_SIZE==10 || NL_DLM_BLOCK_SIZE==18);
	CRGBA *ptr = (CRGBA*)(&CBitmap::getPixels(0)[0]);
	// fill last pixel with black.
	ptr[width*height-1]= CRGBA::Black;
	// Also, to ensure the texture do not wrap around, disable Tiling.
	ITexture::setWrapS(ITexture::Clamp);
	ITexture::setWrapT(ITexture::Clamp);
}


// ***************************************************************************
uint				CTextureDLM::getTypeForSize(uint width, uint height)
{
#ifdef NL_DLM_TILE_RES
	nlassert(width==3 || width==5 || width==9 || width==17);
	nlassert(height==3 || height==5 || height==9 || height==17);
#else
	nlassert(width==2 || width==3 || width==5 || width==9);
	nlassert(height==2 || height==3 || height==5 || height==9);
#endif

	// 0 for 2, 1 for 3, 2 for 5, and 3 for 9, and 4 for 17
	width= getPowerOf2(width-1);
	height= getPowerOf2(height-1);
#ifdef NL_DLM_TILE_RES
	// 0 for 3, 1 for 5, 2, for 9, and 3 for 17
	width--; height--;
#endif

	uint id= width + height*4;
	nlassert(id<NL_DLM_LIGHTMAP_TYPE_SIZE);

	return id;
}


// ***************************************************************************
bool			CTextureDLM::canCreateLightMap(uint w, uint h)
{
	// First test if the list is not empty.
	if(_FreeBlocks[getTypeForSize(w,h)])
		return true;

	// If empty, test if there is an empty block.
	return !_EmptyBlocks.empty();
}


// ***************************************************************************
void			CTextureDLM::linkFreeBlock(uint lMapType, CBlock *block)
{
	// link me to others
	block->FreeNext= _FreeBlocks[lMapType];
	block->FreePrec= NULL;
	// link other to me
	if(_FreeBlocks[lMapType])
		_FreeBlocks[lMapType]->FreePrec= block;
	_FreeBlocks[lMapType]= block;
}

// ***************************************************************************
void			CTextureDLM::unlinkFreeBlock(uint lMapType, CBlock *block)
{
	// unlink other from me
	if(block->FreeNext)
		block->FreeNext->FreePrec= block->FreePrec;
	if(block->FreePrec)
		block->FreePrec->FreeNext= block->FreeNext;
	else
		_FreeBlocks[lMapType]= block->FreeNext;
	// reset me
	block->FreePrec= NULL;
	block->FreeNext= NULL;
}


// ***************************************************************************
bool			CTextureDLM::createLightMap(uint w, uint h, uint &x, uint &y)
{
	// at least cna create it??
	if(!canCreateLightMap(w, h))
		return false;

	// the type of lightmap.
	uint	lMapType= getTypeForSize(w,h);

	// First manage case list is empty.
	//===================
	if(_FreeBlocks[lMapType]==NULL)
	{
		// list is empty => allocate a block from _EmptyBlocks.
		nlassert(!_EmptyBlocks.empty());

		// pop a block from empty list
		CBlock	*block= &_Blocks[_EmptyBlocks.back()];
		_EmptyBlocks.pop_back();

		// init this block.
		nlassert(block->FreeSpace==0);
		// set size of lightmaps for this blocks.
		block->Width= w;
		block->Height= h;

		// Link this block to the list.
		linkFreeBlock(lMapType, block);
	}

	// Get the block from the list.
	CBlock	*block= _FreeBlocks[lMapType];

	// Allocate block lightmap.
	//===================

	// compute block info.
	uint	nLMapOnX= NL_DLM_BLOCK_SIZE / block->Width;
	uint	nLMapOnY= NL_DLM_BLOCK_SIZE / block->Height;
	uint	nLMapPerBlock= nLMapOnX * nLMapOnY;
	// bit must fit in a uint64
	nlassert(nLMapPerBlock<=64);

	// get an id in the FreeSpace bitField.
	uint	i;
	for(i= 0;i<nLMapPerBlock; i++)
	{
		uint	mask= 1<<i;
		// If the bit is not set, then this id is free.
		if( (block->FreeSpace & mask)==0 )
		{
			// take this id, hence set this bit
			block->FreeSpace|= mask;
			// stop, found.
			break;
		}
	}
	nlassert(i<nLMapPerBlock);

	// compute x/y texture pos for this id.
	x= block->PosX + (i%nLMapOnX) * w;
	y= block->PosY + (i/nLMapOnX) * h;


	// if lightmap full
	//===================
	// if bitfield is full
	if( block->FreeSpace == (uint)(1<<nLMapPerBlock)-1 )
	{
		// Must remove it from free list.
		unlinkFreeBlock(lMapType, block);
	}

	return true;
}

// ***************************************************************************
void			CTextureDLM::copyRect(uint x, uint y, uint w, uint h, CRGBA  *textMap)
{
	// copy image.
	CRGBA	*src= textMap;
	CRGBA	*dst= (CRGBA*)getPixels().getPtr();
	dst+= y*getWidth()+x;
	for(sint n= h;n>0;n--, src+= w, dst+= getWidth())
	{
		memcpy(dst, src, w*sizeof(CRGBA));
	}

	// Invalidate the rectangle.
	ITexture::touchRect(CRect(x, y, w, h));
}

// ***************************************************************************
void			CTextureDLM::fillRect(uint x, uint y, uint w, uint h, uint8 value)
{
	// copy image.
	CRGBA	*dst= (CRGBA*)getPixels().getPtr();
	dst+= y*getWidth()+x;
	for(sint n= h;n>0;n--, dst+= getWidth())
	{
		memset(dst, value, w*sizeof(CRGBA));
	}

	// Invalidate the rectangle.
	ITexture::touchRect(CRect(x, y, w, h));
}


// ***************************************************************************
void			CTextureDLM::modulateAndfillRect565(uint x, uint y, uint w, uint h, CRGBA  *textMap, uint16 *modColor)
{
	// compute start dst to copy.
	CRGBA	*dst= (CRGBA*)getPixels().getPtr();
	dst+= y*getWidth()+x;

	// precahce Data in memory (best CPU support)
	CFastMem::precache(textMap, w*h*sizeof(CRGBA));
	CFastMem::precache(modColor, w*h*sizeof(uint16));

	// For all lines
	for(sint n= h;n>0;n--, dst+= (getWidth()-w) )
	{
		// For all the line.
		for(sint nc= w;nc>0;nc--, textMap++, modColor++, dst++)
		{
			uint16	tc= *modColor;
			// modulate R.
			dst->R= ( (tc>>11) * textMap->R)>>5;
			// modulate G.
			dst->G= (((tc>>5)&63) * textMap->G)>>6;
			// modulate B.
			dst->B= ( (tc&31) * textMap->B)>>5;
		}
	}

	// Invalidate the rectangle.
	ITexture::touchRect(CRect(x, y, w, h));
}


// ***************************************************************************
void			CTextureDLM::modulateAndfillRect8888(uint x, uint y, uint w, uint h, CRGBA  *textMap, CRGBA *modColor)
{
	// compute start dst to copy.
	CRGBA	*dst= (CRGBA*)getPixels().getPtr();
	dst+= y*getWidth()+x;

	// precahce Data in memory (best CPU support)
	CFastMem::precache(textMap, w*h*sizeof(CRGBA));
	CFastMem::precache(modColor, w*h*sizeof(CRGBA));

	// For all lines
	for(sint n= h;n>0;n--, dst+= (getWidth()-w) )
	{
		// For all the line.
		for(sint nc= w;nc>0;nc--, textMap++, modColor++, dst++)
		{
			CRGBA		mc= *modColor;
			// modulate RGB only
			dst->R= ( mc.R * textMap->R)>>8;
			dst->G= ( mc.G * textMap->G)>>8;
			dst->B= ( mc.B * textMap->B)>>8;
		}
	}

	// Invalidate the rectangle.
	ITexture::touchRect(CRect(x, y, w, h));
}


// ***************************************************************************
void			CTextureDLM::modulateConstantAndfillRect(uint x, uint y, uint w, uint h, CRGBA  *textMap, CRGBA mc)
{
	// compute start dst to copy.
	CRGBA	*dst= (CRGBA*)getPixels().getPtr();
	dst+= y*getWidth()+x;

	// precahce Data in memory (best CPU support)
	CFastMem::precache(textMap, w*h*sizeof(CRGBA));

	// For all lines
	for(sint n= h;n>0;n--, dst+= (getWidth()-w) )
	{
		// For all the line.
		for(sint nc= w;nc>0;nc--, textMap++, dst++)
		{
			// modulate RGB only
			dst->R= ( mc.R * textMap->R)>>8;
			dst->G= ( mc.G * textMap->G)>>8;
			dst->B= ( mc.B * textMap->B)>>8;
		}
	}

	// Invalidate the rectangle.
	ITexture::touchRect(CRect(x, y, w, h));
}


// ***************************************************************************
void			CTextureDLM::releaseLightMap(uint x, uint y)
{
	// Search what block is under this pos.
	uint	blockId= (y/NL_DLM_BLOCK_SIZE)*_WBlock + (x/NL_DLM_BLOCK_SIZE);
	nlassert(blockId<_Blocks.size());
	CBlock	*block= &_Blocks[blockId];

	// compute block info.
	uint	nLMapOnX= NL_DLM_BLOCK_SIZE / block->Width;
	uint	nLMapOnY= NL_DLM_BLOCK_SIZE / block->Height;
	uint	nLMapPerBlock= nLMapOnX * nLMapOnY;
	// was Full (ie all bits set) before this release
	bool	wasFull= (block->FreeSpace == (uint)(1<<nLMapPerBlock)-1);
	// the type of lightmap.
	uint	lMapType= getTypeForSize(block->Width, block->Height);


	// get relative pos to the block.
	x-= block->PosX;
	y-= block->PosY;

	// compute bit number.
	uint	bitX= x/block->Width;
	uint	bitY= y/block->Height;
	// assert good pos param.
	nlassert(x == bitX*block->Width);
	nlassert(y == bitY*block->Height);

	// compute bitId, as done in createLightMap()
	uint	bitId= bitY * nLMapOnX + bitX;
	uint	mask= 1<<bitId;

	// Free this bit in the block.
	nlassert(block->FreeSpace & mask);
	block->FreeSpace&= (~mask & std::numeric_limits<uint>::max());


	// Free the block if necessary.
	//=======================
	bool	isEmpty= block->FreeSpace==0;

	// If wasFull and now it is empty (nLMapPerBlock==1 case), just append to EmptyBlocks.
	if(wasFull && isEmpty)
	{
		// add the id to the empty list.
		_EmptyBlocks.push_back(blockId);
	}
	// if wasFull, but still have some lightmap now, must insert into FreeList
	else if(wasFull && !isEmpty)
	{
		linkFreeBlock(lMapType, block);
	}
	// if was not full but now it is empty, must remove from free list and insert into EmptyBlocks
	else if(!wasFull && isEmpty)
	{
		// remove the block from Free List
		unlinkFreeBlock(lMapType, block);
		// add the id to the empty list.
		_EmptyBlocks.push_back(blockId);
	}
	// else (!wasFull && !isEmpty) no-op, since must be kept in the FreeList.

}


} // NL3D
