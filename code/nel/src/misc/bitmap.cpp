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

#include "stdmisc.h"

#include "nel/misc/bitmap.h"
#include "nel/misc/stream.h"
#include "nel/misc/file.h"
#include "nel/misc/system_info.h"

// Define this to force all bitmap white (debug)
// #define NEL_ALL_BITMAP_WHITE


using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

struct EDDSBadHeader : public NLMISC::EStream
{
	EDDSBadHeader() : EStream( "Bad or unrecognized DDS file header" ) {}
};

struct ESeekFailed : public NLMISC::EStream
{
	ESeekFailed() : EStream( "Seek failed" ) {}
};

struct EAllocationFailure : public Exception
{
	EAllocationFailure() : Exception( "Can't allocate memory" ) {}
};

void blendFromui(NLMISC::CRGBA &c0, NLMISC::CRGBA &c1, uint coef);
uint32 blend(uint32 &n0, uint32 &n1, uint32 coef0);

const uint32 CBitmap::bitPerPixels[ModeCount]=
{
	32,		// RGBA
	8,		// Luminance
	8,		// Alpha
	16,		// AlphaLuminance
	4,		// DXTC1
	4,		// DXTC1Alpha
	8,		// DXTC3
	8,		// DXTC5
	16		// DsDt
};

const uint32 CBitmap::DXTC1HEADER = NL_MAKEFOURCC('D','X', 'T', '1');
const uint32 CBitmap::DXTC3HEADER = NL_MAKEFOURCC('D','X', 'T', '3');
const uint32 CBitmap::DXTC5HEADER = NL_MAKEFOURCC('D','X', 'T', '5');


#ifdef NEL_ALL_BITMAP_WHITE
// Make all the textures white
void MakeWhite(CBitmap &bitmaps)
{
	for (uint i=0; i<bitmaps.getMipMapCount (); i++)
	{
		uint size = bitmaps.getPixels (i).size ();
		bitmaps.getPixels (i).resize (0);
		bitmaps.getPixels (i).resize (size);
		bitmaps.getPixels (i).fill (0xff);
	}
}
#endif // NEL_ALL_BITMAP_WHITE

/*-------------------------------------------------------------------*\
								load
\*-------------------------------------------------------------------*/
uint8 CBitmap::load(NLMISC::IStream &f, uint mipMapSkip)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	nlassert(f.isReading());

	// testing if DDS
	uint32 fileType = 0;;
	f.serial(fileType);
	if(fileType == DDS_HEADER)
	{
#ifdef NEL_ALL_BITMAP_WHITE
		uint8 result = readDDS(f, mipMapSkip);
		MakeWhite (*this);
		return result;
#else // NEL_ALL_BITMAP_WHITE
		return readDDS(f, mipMapSkip);
#endif // NEL_ALL_BITMAP_WHITE
	}

	if (fileType == PNG_HEADER)
	{
#ifdef NEL_ALL_BITMAP_WHITE
		uint8 result = readPNG(f);
		MakeWhite (*this);
		return result;
#else // NEL_ALL_BITMAP_WHITE
		return readPNG(f);
#endif // NEL_ALL_BITMAP_WHITE
	}

#ifdef USE_JPEG
	if (fileType == JPG_HEADER)
	{
#ifdef NEL_ALL_BITMAP_WHITE
		uint8 result = readJPG(f);
		MakeWhite (*this);
		return result;
#else // NEL_ALL_BITMAP_WHITE
		return readJPG(f);
#endif // NEL_ALL_BITMAP_WHITE
	}
#endif // USE_JPEG

	// assuming it's TGA
	NLMISC::IStream::TSeekOrigin origin= f.begin;
	if(!f.seek (0, origin))
	{
		throw ESeekFailed();
	}

	// Reading header,
	// To make sure that the bitmap is TGA, we check imageType and imageDepth.
	uint8	lengthID;
	uint8	cMapType;
	uint8	imageType;
	uint16	tgaOrigin;
	uint16	length;
	uint8	depth;
	uint16	xOrg;
	uint16	yOrg;
	uint16	width;
	uint16	height;
	uint8	imageDepth;
	uint8	desc;

	f.serial(lengthID);
	f.serial(cMapType);
	f.serial(imageType);
	if(imageType!=2 && imageType!=3 && imageType!=10 && imageType!=11) return 0;
	f.serial(tgaOrigin);
	f.serial(length);
	f.serial(depth);
	f.serial(xOrg);
	f.serial(yOrg);
	f.serial(width);
	f.serial(height);
	f.serial(imageDepth);
	if(imageDepth!=8 && imageDepth!=16 && imageDepth!=24 && imageDepth!=32) return 0;
	f.serial(desc);

	if(!f.seek (0, origin))
	{
		throw ESeekFailed();
	}
#ifdef NEL_ALL_BITMAP_WHITE
	uint8 result = readTGA(f);
	MakeWhite (*this);
	return result;
#else // NEL_ALL_BITMAP_WHITE
	return readTGA(f);
#endif // NEL_ALL_BITMAP_WHITE
}


/*-------------------------------------------------------------------*\
								makeDummy
\*-------------------------------------------------------------------*/
void	CBitmap::makeDummy()
{
	static	const uint8	bitmap[1024]= {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	};

	makeDummyFromBitField(bitmap);
}


/*-------------------------------------------------------------------*\
								makeDummy
\*-------------------------------------------------------------------*/
void	CBitmap::makeNonPowerOf2Dummy()
{
	static	const uint8	bitmap[1024]= {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,0,0,0,0,
		0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
		0,0,0,0,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,0,0,0,0,
		0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,
		0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
		0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
		0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	};

	makeDummyFromBitField(bitmap);
}


/*-------------------------------------------------------------------*\
						makeDummyFromBitField
\*-------------------------------------------------------------------*/
void	CBitmap::makeDummyFromBitField(const uint8	bitmap[1024])
{
	PixelFormat = RGBA;
	_MipMapCount = 1;
	_Width= 32;
	_Height= 32;
	_Data[0].resize(_Width*_Height*sizeof(NLMISC::CRGBA));
	for(sint m=1;m<MAX_MIPMAP;m++)
		_Data[m].clear();
	NLMISC::CRGBA	*pix= (NLMISC::CRGBA*)(_Data[0].getPtr());

	for(sint i=0;i<(sint)(_Width*_Height);i++)
	{
		if(bitmap[i])
			pix[i].set(255,255,255,255);
		else
			pix[i].set(0x80,0x80,0x80,0x40);
	}
}




/*-------------------------------------------------------------------*\
								readDDS
\*-------------------------------------------------------------------*/
uint8 CBitmap::readDDS(NLMISC::IStream &f, uint mipMapSkip)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	//------------------ Reading Header ------------------------

	//-------------- reading entire header

	uint32 size = 0;
	f.serial(size); // size in Bytes of header(without "DDS")
	 uint32 * _DDSSurfaceDesc = new uint32[size];
	_DDSSurfaceDesc[0]= size;

#ifdef NL_LITTLE_ENDIAN
	f.serialBuffer((uint8*)(_DDSSurfaceDesc+1), size-4);
#else
	for(uint i= 0; i<size/4 - 1; i++)
	{
		f.serial(_DDSSurfaceDesc[i+1]);
	}
#endif

	// flags determines which members of the header structure contain valid data
	uint32 flags = _DDSSurfaceDesc[1];

	//verify if file have linearsize set
	if(!(flags & DDSD_LINEARSIZE))
    {
		nlwarning("A DDS doesn't have the flag DDSD_LINEARSIZE");
		//delete [] _DDSSurfaceDesc;
		//throw EDDSBadHeader();
	}

	//-------------- extracting and testing useful info

	_Height = _DDSSurfaceDesc[2];
	_Width  = _DDSSurfaceDesc[3];
	_MipMapCount= (uint8) _DDSSurfaceDesc[6];
	// If no mipmap.
	if(_MipMapCount==0)
		_MipMapCount=1;
	switch (_DDSSurfaceDesc[20])
	{
	case DXTC1HEADER:
		PixelFormat=DXTC1;
		break;
	case DXTC3HEADER:
		PixelFormat=DXTC3;
		break;
	case DXTC5HEADER:
		PixelFormat=DXTC5;
		break;
	}

	flags = _DDSSurfaceDesc[19]; //PixelFormat flags

/*	ace: I changed this code because it's not a way to detect if DXTC1 has a alpha channel or not
		There's no easy way to detect if the DXTC1 has an alpha channel or not, so, for now, we'll suppose
		that all DXTC1 has alpha channel per default.

		"There is no flag unfortunately, you need to read each block of DXT1 data, check if one of the colors
		contains alpha, and check if that color is used in the data.
		It's not that hard to write, but it IS a pain that it's the only way that I've found to check for alpha."
		http://www.gamedev.net/community/forums/topic.asp?topic_id=177475

		UPDATE: worst... on linux/opengl, it generates random alpha values
		if we use alpha dxtc1 by default. So I only to that for windows
		and leave the old test on linux

		kervala: some used textures don't have an alpha channel and they are
		(very) bad rendered with this fix	so we have to deactivate it the for moment
*/

//#ifdef NL_OS_WINDOWS
//	if(PixelFormat==DXTC1) //AlphaBitDepth
//	{
//		PixelFormat = DXTC1Alpha;
//	}
//#else
	if(PixelFormat==DXTC1 && _DDSSurfaceDesc[21]>0) //AlphaBitDepth
	{
		PixelFormat = DXTC1Alpha;
	}
//#endif

	if(PixelFormat!= DXTC1 && PixelFormat!= DXTC1Alpha && PixelFormat!= DXTC3 && PixelFormat!= DXTC5)
	{
		delete [] _DDSSurfaceDesc;
		throw EDDSBadHeader();
	}

	// compute the min power of 2 between width and height
	uint	minSizeLevel= min(_Width, _Height);
	minSizeLevel= getPowerOf2(minSizeLevel);

	//------------- manage mipMapSkip
	if(_MipMapCount>1 && mipMapSkip>0 && minSizeLevel>2)
	{
		// Keep at least the level where width and height are at least 4.
		mipMapSkip= min(mipMapSkip, minSizeLevel-2);
		// skip any mipmap
		uint	seekSize= 0;
		while(mipMapSkip>0 && _MipMapCount>1)
		{
			// raise to next multiple of 4
			uint32 wtmp= (_Width+3)&(~3);
			uint32 htmp= (_Height+3)&(~3);
			wtmp= max(wtmp, uint32(4));
			htmp= max(htmp, uint32(4));

			uint32 mipMapSz;
			if(PixelFormat==DXTC1 || PixelFormat==DXTC1Alpha)
				mipMapSz = wtmp*htmp/2;
			else
				mipMapSz = wtmp*htmp;

			// add to how many to skip
			seekSize+= mipMapSz;

			// Size of final bitmap is reduced.
			_Width>>=1;
			_Height>>=1;
			_MipMapCount--;
			mipMapSkip--;
		}
		// skip data in file
		if(seekSize>0)
		{
			if(!f.seek(seekSize, IStream::current))
			{
				delete [] _DDSSurfaceDesc;
				throw ESeekFailed();
			}
		}

	}

	//------------- preload all the mipmaps (one serialBuffer() is faster)
	uint32 w = _Width;
	uint32 h = _Height;
	uint32	totalSize= 0;

	uint8	m;
	for(m= 0; m<_MipMapCount; m++)
	{
		// raise to next multiple of 4
		uint32 wtmp= (w+3)&(~3);
		uint32 htmp= (h+3)&(~3);
		wtmp= max(wtmp, uint32(4));
		htmp= max(htmp, uint32(4));

		uint32 mipMapSz;
		if(PixelFormat==DXTC1 || PixelFormat==DXTC1Alpha)
			mipMapSz = wtmp*htmp/2;
		else
			mipMapSz = wtmp*htmp;


		_Data[m].resize(mipMapSz);
		totalSize+= mipMapSz;

	  	w = (w+1)/2;
		h = (h+1)/2;
	}

	// Read all the data in one block.
	vector<uint8>	pixData;
	pixData.resize(totalSize);
	f.serialBuffer(&(*pixData.begin()), totalSize);


	//------------- reading mipmap levels from pixData

	uint32 pixIndex= 0;

	for(m= 0; m<_MipMapCount; m++)
	{
		uint32	mipMapSz= _Data[m].size();
		memcpy(_Data[m].getPtr(), &(pixData[pixIndex]), mipMapSz);
		pixIndex+= mipMapSz;
	}

	//------------- End

	delete [] _DDSSurfaceDesc;

	switch(PixelFormat)
	{
		case DXTC1  : return 24;
		case DXTC1Alpha : return 32;
		case DXTC3  : return 32;
		case DXTC5  : return 32;
		default  : break;
	}

	return 0;
}




/*-------------------------------------------------------------------*\
							convertToDXTC5
\*-------------------------------------------------------------------*/
bool CBitmap::convertToDXTC5()
{
	/* Yoyo: RGB encoding for DXTC1 and DXTC5/3 are actually different!!
		DXTC3/5 don't rely on sign of color0>color1 to setup special encoding (ie use a special compression for Black)
		Since this can arise if the src is DXTC1 , we can't simply compress it into DXTC5 without doing a
		heavy compression...
		(the inverse is false: DXTC5 to DXTC1 is possible, with maybe swap color0/color1 and bits).
	*/

	return false;

/*	uint32 i,j;

	if(PixelFormat!=DXTC1) return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(2*_Data[m].size());
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i+=8)
		{
			//64 bits alpha
			for(j=0; j<8; j++)
			{
				dataTmp[dstId++]= 255;
			}

			//64 bits RGB
			for(j=0; j<8; j++)
			{
				dataTmp[dstId++]= _Data[m][i+j];
			}
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = DXTC5;
	return true;
*/
}



/*-------------------------------------------------------------------*\
							luminanceToRGBA()
\*-------------------------------------------------------------------*/
bool CBitmap::luminanceToRGBA()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()*4);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp[dstId++]= _Data[m][i];
			dataTmp[dstId++]= _Data[m][i];
			dataTmp[dstId++]= _Data[m][i];
			dataTmp[dstId++]= 255;
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = RGBA;
	return true;
}

/*-------------------------------------------------------------------*\
							alphaToRGBA()
\*-------------------------------------------------------------------*/
bool CBitmap::alphaToRGBA()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()*4);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp[dstId++]= 255;
			dataTmp[dstId++]= 255;
			dataTmp[dstId++]= 255;
			dataTmp[dstId++]= _Data[m][i];
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = RGBA;
	return true;
}


/*-------------------------------------------------------------------*\
							alphaLuminanceToRGBA()
\*-------------------------------------------------------------------*/
bool CBitmap::alphaLuminanceToRGBA()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()*2);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i+=2)
		{
			dataTmp[dstId++]= _Data[m][i];
			dataTmp[dstId++]= _Data[m][i];
			dataTmp[dstId++]= _Data[m][i];
			dataTmp[dstId++]= _Data[m][i+1];
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = RGBA;
	return true;
}




/*-------------------------------------------------------------------*\
							rgbaToAlphaLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::rgbaToAlphaLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()/2);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i+=4)
		{
			dataTmp[dstId++]= (_Data[m][i]*77 + _Data[m][i+1]*150 + _Data[m][i+2]*28)/255;
			dataTmp[dstId++]= _Data[m][i+3];
		}
		NLMISC::contReset(_Data[m]);
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = AlphaLuminance;
	return true;
}


/*-------------------------------------------------------------------*\
							luminanceToAlphaLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::luminanceToAlphaLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()*2);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp[dstId++]= _Data[m][i];
			dataTmp[dstId++]= 255;
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = AlphaLuminance;
	return true;
}



/*-------------------------------------------------------------------*\
							alphaToAlphaLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::alphaToAlphaLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()*2);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp[dstId++]= 0;
			dataTmp[dstId++]= _Data[m][i];
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = AlphaLuminance;
	return true;
}



/*-------------------------------------------------------------------*\
							rgbaToLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::rgbaToLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()/4);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i+=4)
		{
			dataTmp[dstId++]= (_Data[m][i]*77 + _Data[m][i+1]*150 + _Data[m][i+2]*28)/255;
		}
		NLMISC::contReset(_Data[m]);
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = Luminance;
	return true;
}



/*-------------------------------------------------------------------*\
							alphaToLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::alphaToLuminance()
{
	if(_Width*_Height == 0)  return false;

	PixelFormat = Luminance;
	return true;
}



/*-------------------------------------------------------------------*\
							alphaLuminanceToLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::alphaLuminanceToLuminance()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()/2);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i+=2)
		{
			dataTmp[dstId++]= 0;
			dataTmp[dstId++]= 0;
			dataTmp[dstId++]= 0;
			dataTmp[dstId++]= _Data[m][i];
		}
		NLMISC::contReset(_Data[m]);
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = Luminance;
	return true;
}


/*-------------------------------------------------------------------*\
							rgbaToAlpha
\*-------------------------------------------------------------------*/
bool CBitmap::rgbaToAlpha()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()/4);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i+=4)
		{
			dataTmp[dstId++]= _Data[m][i+3];
		}
		NLMISC::contReset(_Data[m]);
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = Alpha;
	return true;
}


/*-------------------------------------------------------------------*\
							luminanceToAlpha
\*-------------------------------------------------------------------*/
bool CBitmap::luminanceToAlpha()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size());
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i++)
		{
			dataTmp[dstId++]= _Data[m][i];
		}
		_Data[m] = dataTmp;
	}
	PixelFormat = Alpha;
	return true;
}


/*-------------------------------------------------------------------*\
							alphaLuminanceToAlpha
\*-------------------------------------------------------------------*/
bool CBitmap::alphaLuminanceToAlpha()
{
	uint32 i;

	if(_Width*_Height == 0)  return false;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		CObjectVector<uint8> dataTmp;
		dataTmp.resize(_Data[m].size()/2);
		uint	dstId= 0;

		for(i=0; i<_Data[m].size(); i+=2)
		{
			dataTmp[dstId++]= _Data[m][i+1];
		}
		NLMISC::contReset(_Data[m]);
		_Data[m].resize(0);
		_Data[m] = dataTmp;
	}
	PixelFormat = Alpha;
	return true;
}


/*-------------------------------------------------------------------*\
							convertToLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::convertToLuminance()
{
	switch(PixelFormat)
	{
		case RGBA :
			return rgbaToLuminance();
			break;

		case Luminance :
			return true;
			break;

		case Alpha :
			return alphaToLuminance();
			break;

		case AlphaLuminance :
			return alphaLuminanceToLuminance();
			break;

		default:
			break;
	}
	return false;
}



/*-------------------------------------------------------------------*\
							convertToAlpha
\*-------------------------------------------------------------------*/
bool CBitmap::convertToAlpha()
{
	switch(PixelFormat)
	{
		case RGBA :
			return rgbaToAlpha();
			break;

		case Luminance :
			return luminanceToAlpha();
			break;

		case Alpha :
			return true;
			break;

		case AlphaLuminance :
			return alphaLuminanceToAlpha();
			break;

		default:
			break;
	}
	return false;
}



/*-------------------------------------------------------------------*\
							convertToAlphaLuminance
\*-------------------------------------------------------------------*/
bool CBitmap::convertToAlphaLuminance()
{
	switch(PixelFormat)
	{
		case RGBA :
			return rgbaToAlphaLuminance();
			break;

		case Luminance :
			return luminanceToAlphaLuminance();
			break;

		case Alpha :
			return alphaToAlphaLuminance();
			break;

		case AlphaLuminance :
			return true;
			break;

		default:
			break;
	}
	return false;
}


/*-------------------------------------------------------------------*\
							convertToRGBA
\*-------------------------------------------------------------------*/
bool CBitmap::convertToRGBA()
{
	switch(PixelFormat)
	{
		case DXTC1 :
			return decompressDXT1(false);
			break;

		case DXTC1Alpha :
			return decompressDXT1(true);
			break;

		case DXTC3 :
			return decompressDXT3();
			break;

		case DXTC5 :
			return decompressDXT5();
			break;

		case Luminance :
			return luminanceToRGBA();
			break;

		case Alpha :
			return alphaToRGBA();
			break;

		case AlphaLuminance :
			return alphaLuminanceToRGBA();
			break;
		case RGBA:
			return true;
		break;
		default:
			break;
	}
	return false;
}


/*-------------------------------------------------------------------*\
							convertToType
\*-------------------------------------------------------------------*/
bool CBitmap::convertToType(CBitmap::TType type)
{
	if(PixelFormat==type) return true;

	switch(type)
	{
		case RGBA :
			return convertToRGBA();
			break;

		case DXTC5 :
			return convertToDXTC5();
			break;

		case Luminance :
			return convertToLuminance();
			break;

		case Alpha :
			return convertToAlpha();
			break;

		case AlphaLuminance :
			return convertToAlphaLuminance();
			break;

		default:
			break;
	}

	return false;
}




/*-------------------------------------------------------------------*\
							decompressDXT1
\*-------------------------------------------------------------------*/
bool CBitmap::decompressDXT1(bool alpha)
{
	uint32 i,j,k;
	NLMISC::CRGBA	c[4];
	CObjectVector<uint8> dataTmp[MAX_MIPMAP];

	uint32 width= _Width;
	uint32 height= _Height;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		uint32 wtmp, htmp;
		if(width<4)
			wtmp = 4;
		else
			wtmp = width;
		if(height < 4)
			htmp = 4;
		else
			htmp = height;
		uint32 mipMapSz = wtmp*htmp*4;
		dataTmp[m].resize(mipMapSz);
		if(dataTmp[m].size()<mipMapSz)
		{
			throw EAllocationFailure();
		}
		uint32 wBlockCount= wtmp/4;



		for(i=0; i < _Data[m].size(); i+=8)
		{
			uint16 color0;
			uint16 color1;
			uint32 bits;
			memcpy(&color0,&_Data[m][i],2);
			memcpy(&color1,&_Data[m][i+2],2);
			memcpy(&bits,&_Data[m][i+4],4);

			uncompress(color0,c[0]);
			uncompress(color1,c[1]);

			if (alpha)
			{
				c[0].A= 0;
				c[1].A= 0;
				c[2].A= 0;
				c[3].A= 0;
			}
			else
			{
				c[0].A= 255;
				c[1].A= 255;
				c[2].A= 255;
				c[3].A= 255;
			}

			if(color0>color1)
			{
				c[2].blendFromui(c[0],c[1],85);
				if(alpha) c[2].A= 255;

				c[3].blendFromui(c[0],c[1],171);
				if(alpha) c[3].A= 255;
			}
			else
			{
				c[2].blendFromui(c[0],c[1],128);
				if(alpha) c[2].A= 255;

				c[3].set(0,0,0,0);
			}

			// computing the 16 RGBA of the block

			uint32 blockNum= i/8; //(64 bits)
			// <previous blocks in above lines> * 4 (rows) * _Width (columns) + 4pix*4rgba*<same line previous blocks>
			uint32 pixelsCount= 4*(blockNum/wBlockCount)*wtmp*4 + 4*4*(blockNum%wBlockCount);
			for(j=0; j<4; j++)
			{
				for(k=0; k<4; k++)
				{
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k]= c[bits&3].R;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+1]= c[bits&3].G;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+2]= c[bits&3].B;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+3]= c[bits&3].A;
					bits>>=2;
				}
			}
		}

		// Copy result into the mipmap level.
		if(wtmp==width && htmp==height)
		{
			// For mipmaps level >4 pixels.
			_Data[m]= dataTmp[m];
		}
		else
		{
			// For last mipmaps, level <4 pixels.
			_Data[m].resize(width*height*4);
			CRGBA	*src= (CRGBA*)&dataTmp[m][0];
			CRGBA	*dst= (CRGBA*)&_Data[m][0];
			uint	x,y;
			for(y=0;y<height;y++)
			{
				for(x=0;x<width;x++)
					dst[y*width+x]= src[y*wtmp+x];
			}
		}

		// Next mipmap size.
		width = (width+1)/2;
		height = (height+1)/2;
	}
	PixelFormat = RGBA;
	return true;
}




/*-------------------------------------------------------------------*\
							decompressDXT3
\*-------------------------------------------------------------------*/
bool CBitmap::decompressDXT3()
{
	uint32 i,j,k;
	NLMISC::CRGBA	c[4];
	CObjectVector<uint8> dataTmp[MAX_MIPMAP];

	uint32 width= _Width;
	uint32 height= _Height;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		uint32 wtmp, htmp;
		if(width<4)
			wtmp = 4;
		else
			wtmp = width;
		if(height < 4)
			htmp = 4;
		else
			htmp = height;
		uint32 mipMapSz = wtmp*htmp*4;
		dataTmp[m].resize(mipMapSz);
		if(dataTmp[m].size()<mipMapSz)
		{
			throw EAllocationFailure();
		}
		uint32 wBlockCount= wtmp/4;


		for(i=0; i < _Data[m].size(); i+=16)
		{
			uint8 alpha[16];
			uint64 alphatmp;
			memcpy(&alphatmp,&_Data[m][i],8);

			for(j=0; j<16; j++)
			{
				uint8	a= (uint8)(alphatmp&15);
				// expand to 0-255.
				alpha[j]= a+(a<<4);
				alphatmp>>=4;
			}


			uint16 color0;
			uint16 color1;
			uint32 bits;
			memcpy(&color0,&_Data[m][i+8],2);
			memcpy(&color1,&_Data[m][i+10],2);
			memcpy(&bits,&_Data[m][i+12],4);

			uncompress(color0,c[0]);
			uncompress(color1,c[1]);

			// ignore color0>color1 for DXT3 and DXT5.
			c[2].blendFromui(c[0],c[1],85);
			c[3].blendFromui(c[0],c[1],171);

			// computing the 16 RGBA of the block

			uint32 blockNum= i/16; //(128 bits)
			// <previous blocks in above lines> * 4 (rows) * wtmp (columns) + 4pix*4rgba*<same line previous blocks>
			uint32 pixelsCount= 4*(blockNum/wBlockCount)*wtmp*4 + 4*4*(blockNum%wBlockCount);
			for(j=0; j<4; j++)
			{
				for(k=0; k<4; k++)
				{
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k]= c[bits&3].R;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+1]= c[bits&3].G;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+2]= c[bits&3].B;
					dataTmp[m][pixelsCount + j*wtmp*4 + 4*k+3]= alpha[4*j+k];
					bits>>=2;
				}
			}
		}

		// Copy result into the mipmap level.
		if(wtmp==width && htmp==height)
		{
			// For mipmaps level >4 pixels.
			_Data[m]= dataTmp[m];
		}
		else
		{
			// For last mipmaps, level <4 pixels.
			_Data[m].resize(width*height*4);
			CRGBA	*src= (CRGBA*)&dataTmp[m][0];
			CRGBA	*dst= (CRGBA*)&_Data[m][0];
			uint	x,y;
			for(y=0;y<height;y++)
			{
				for(x=0;x<width;x++)
					dst[y*width+x]= src[y*wtmp+x];
			}
		}

		// Next mipmap size.
		width = (width+1)/2;
		height = (height+1)/2;
	}
	PixelFormat = RGBA;
	return true;
}




/*-------------------------------------------------------------------*\
							decompressDXT5
\*-------------------------------------------------------------------*/
bool CBitmap::decompressDXT5()
{
	uint32 i,j,k;
	NLMISC::CRGBA	c[4];
	CObjectVector<uint8> dataTmp[MAX_MIPMAP];

	uint32 width= _Width;
	uint32 height= _Height;

	for(uint8 m= 0; m<_MipMapCount; m++)
	{
		uint32 wtmp, htmp;
		if(width<4)
			wtmp = 4;
		else
			wtmp = width;
		if(height < 4)
			htmp = 4;
		else
			htmp = height;
		uint32 mipMapSz = wtmp*htmp*4;
		dataTmp[m].resize(mipMapSz);
		if(dataTmp[m].size()<mipMapSz)
		{
			throw EAllocationFailure();
		}
		uint32 wBlockCount= wtmp/4;



		for(i=0; i < _Data[m].size(); i+=16)
		{
			uint64 bitsAlpha;
			memcpy(&bitsAlpha,&_Data[m][i],8);
			bitsAlpha>>= 16;

			uint32 alpha[8];
			alpha[0]= _Data[m][i+0];
			alpha[1]= _Data[m][i+1];

			if(alpha[0]>alpha[1])
			{
				alpha[2]= blend(alpha[0], alpha[1], 219);
				alpha[3]= blend(alpha[0], alpha[1], 183);
				alpha[4]= blend(alpha[0], alpha[1], 146);
				alpha[5]= blend(alpha[0], alpha[1], 110);
				alpha[6]= blend(alpha[0], alpha[1], 73);
				alpha[7]= blend(alpha[0], alpha[1], 37);
			}
			else
			{
				alpha[2]= blend(alpha[0], alpha[1], 204);
				alpha[3]= blend(alpha[0], alpha[1], 154);
				alpha[4]= blend(alpha[0], alpha[1], 102);
				alpha[5]= blend(alpha[0], alpha[1], 51);
				alpha[6]= 0;
				alpha[7]= 255;
			}

			uint8 codeAlpha[16];
			for(j=0; j<16; j++)
			{
				codeAlpha[j] = (uint8)(bitsAlpha & 7);
				bitsAlpha>>=3;
			}


			uint16 color0;
			uint16 color1;
			uint32 bits;
			memcpy(&color0,&_Data[m][i+8],2);
			memcpy(&color1,&_Data[m][i+10],2);
			memcpy(&bits,&_Data[m][i+12],4);

			uncompress(color0,c[0]);
			uncompress(color1,c[1]);

			// ignore color0>color1 for DXT3 and DXT5.
			c[2].blendFromui(c[0],c[1],85);
			c[3].blendFromui(c[0],c[1],171);

			// computing the 16 RGBA of the block

			uint32 blockNum= i/16; //(128 bits)

			// <previous blocks in above lines> * 4 (rows) * wtmp (columns) + 4pix*<same line previous blocks>
			uint32 pixelsCount= (blockNum/wBlockCount)*wtmp*4 + 4*(blockNum%wBlockCount);
			// *sizeof(RGBA)
			pixelsCount*=4;
			for(j=0; j<4; j++)
			{
				for(k=0; k<4; k++)
				{
					dataTmp[m][pixelsCount + (j*wtmp+k)*4 +0]= c[bits&3].R;
					dataTmp[m][pixelsCount + (j*wtmp+k)*4 +1]= c[bits&3].G;
					dataTmp[m][pixelsCount + (j*wtmp+k)*4 +2]= c[bits&3].B;
					dataTmp[m][pixelsCount + (j*wtmp+k)*4 +3]= (uint8) alpha[codeAlpha[4*j+k]];
					bits>>=2;
				}
			}

		}

		// Copy result into the mipmap level.
		if(wtmp==width && htmp==height)
		{
			// For mipmaps level >4 pixels.
			_Data[m]= dataTmp[m];
		}
		else
		{
			// For last mipmaps, level <4 pixels.
			_Data[m].resize(width*height*4);
			CRGBA	*src= (CRGBA*)&dataTmp[m][0];
			CRGBA	*dst= (CRGBA*)&_Data[m][0];
			uint	x,y;
			for(y=0;y<height;y++)
			{
				for(x=0;x<width;x++)
					dst[y*width+x]= src[y*wtmp+x];
			}
		}

		// Next mipmap size.
		width = (width+1)/2;
		height = (height+1)/2;
	}
	PixelFormat = RGBA;
	return true;

}




/*-------------------------------------------------------------------*\
							blend
\*-------------------------------------------------------------------*/
uint32 CBitmap::blend(uint32 &n0, uint32 &n1, uint32 coef0)
{
	int	a0 = coef0;
	int	a1 = 256-a0;
	return ((n0*a0 + n1*a1) >>8);
}



/*-------------------------------------------------------------------*\
							uncompress
\*-------------------------------------------------------------------*/
inline void CBitmap::uncompress(uint16 color, NLMISC::CRGBA &r)
{
	r.A= 0;
	r.R= ((color>>11)&31) << 3; r.R+= r.R>>5;
	r.G= ((color>>5)&63) << 2;  r.G+= r.G>>6;
	r.B= ((color)&31) << 3;     r.B+= r.B>>5;
}



/*-------------------------------------------------------------------*\
							getWidth
\*-------------------------------------------------------------------*/
uint32 CBitmap::getWidth(uint32 mipMap) const
{
	if(mipMap==0) return _Width;

	uint32 w = _Width;
	uint32 h = _Height;
	uint32 m = 0;

	do
	{
		m++;
		w = (w+1)/2;
		h = (h+1)/2;
		if(m==mipMap) return w;
	}
	while(w!=1 || h!=1);

	return 0;
}



/*-------------------------------------------------------------------*\
							getHeight
\*-------------------------------------------------------------------*/
uint32 CBitmap::getHeight(uint32 mipMap) const
{
	if(mipMap==0) return _Height;

	uint32 w = _Width;
	uint32 h = _Height;
	uint32 m = 0;

	do
	{
		m++;
		w = (w+1)/2;
		h = (h+1)/2;
		if(m==mipMap) return h;
	}
	while(w!=1 || h!=1);

	return 0;
}


/*-------------------------------------------------------------------*\
							getSize
\*-------------------------------------------------------------------*/
uint32 CBitmap::getSize(uint32 numMipMap) const
{
	return getHeight(numMipMap)*getWidth(numMipMap);
}



/*-------------------------------------------------------------------*\
							buildMipMaps
\*-------------------------------------------------------------------*/
void CBitmap::buildMipMaps()
{
	uint32 i,j;

	if(PixelFormat!=RGBA) return;
	if(_MipMapCount!=1) return;
	if(!NLMISC::isPowerOf2(_Width)) return;
	if(!NLMISC::isPowerOf2(_Height)) return;

	uint32 w = _Width;
	uint32 h = _Height;

	while(w>1 || h>1)
	{
		uint32 precw = w;
		uint32 prech = h;
		w = (w+1)/2;
		h = (h+1)/2;
		uint32	mulw= precw/w;
		uint32	mulh= prech/h;

		_Data[_MipMapCount].resize(w*h*4);

		NLMISC::CRGBA *pRgba = (NLMISC::CRGBA*)&_Data[_MipMapCount][0];
		NLMISC::CRGBA *pRgbaPrev = (NLMISC::CRGBA*)&_Data[_MipMapCount-1][0];
		for(i=0; i<h; i++)
		{
			sint	i0= mulh*i;
			sint	i1= mulh*i+1;
			if(mulh==1)
				i1=i0;
			i0*=precw;
			i1*=precw;
			for(j=0; j<w; j++)
			{
				sint	j0= mulw*j;
				sint	j1= mulw*j+1;
				if(mulw==1)
					j1=j0;
				CRGBA	&c0= pRgbaPrev[i0+j0];
				CRGBA	&c1= pRgbaPrev[i0+j1];
				CRGBA	&c2= pRgbaPrev[i1+j0];
				CRGBA	&c3= pRgbaPrev[i1+j1];
				pRgba[i*w + j].R = (c0.R +
									c1.R +
									c2.R +
									c3.R + 2 ) /4;
				pRgba[i*w + j].G = (c0.G +
									c1.G +
									c2.G +
									c3.G + 2 ) /4;
				pRgba[i*w + j].B = (c0.B +
									c1.B +
									c2.B +
									c3.B + 2 ) /4;
				pRgba[i*w + j].A = (c0.A +
									c1.A +
									c2.A +
									c3.A + 2 ) /4;
			}
		}

		_MipMapCount++;
	}
}

/*-------------------------------------------------------------------*\
							computeNeededMipMapCount
\*-------------------------------------------------------------------*/
uint32 CBitmap::computeNeededMipMapCount() const
{
	if(_MipMapCount == 0) return 0;
	if(!NLMISC::isPowerOf2(_Width)) return 1;
	if(!NLMISC::isPowerOf2(_Height)) return 1;

	uint32 mipMapCount = 1;
	uint32 w = _Width;
	uint32 h = _Height;

	while(w>1 || h>1)
	{
		w = (w+1)/2;
		h = (h+1)/2;
		++mipMapCount;
	}
	return mipMapCount;
}

/*-------------------------------------------------------------------*\
							releaseMipMaps
\*-------------------------------------------------------------------*/
void CBitmap::releaseMipMaps()
{
	if(_MipMapCount<=1) return;

	_MipMapCount=1;
	for(sint i=1;i<MAX_MIPMAP;i++)
	{
		NLMISC::contReset(_Data[i]);
	}
}

/*-------------------------------------------------------------------*\
							resample
\*-------------------------------------------------------------------*/
void CBitmap::resample(sint32 nNewWidth, sint32 nNewHeight)
{
	nlassert(PixelFormat == RGBA);
	bool needRebuild = false;

	// Deleting mipmaps
	//logResample("Resample: 10");
	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();
	//logResample("Resample: 20");

	if(nNewWidth==0 || nNewHeight==0)
	{
		_Width = _Height = 0;
		//logResample("Resample: 25");
		return;
	}

	//logResample("Resample: 30");
	CObjectVector<uint8> pDestui;
	pDestui.resize(nNewWidth*nNewHeight*4);
	//logResample("Resample: 40");
	NLMISC::CRGBA *pDestRgba = (NLMISC::CRGBA*)&pDestui[0];
	//logResample("Resample: 50");

	resamplePicture32 ((NLMISC::CRGBA*)&_Data[0][0], pDestRgba, _Width, _Height, nNewWidth, nNewHeight);
	//logResample("Resample: 60");

	NLMISC::contReset(_Data[0]); // free memory
	//logResample("Resample: 70");

	_Data[0] =  pDestui;
	//logResample("Resample: 80");
	_Width= nNewWidth;
	_Height= nNewHeight;

	// Rebuilding mipmaps
	//logResample("Resample: 90");
	if(needRebuild)
	{
		buildMipMaps();
		//logResample("Resample: 95");
	}
	//logResample("Resample: 100");
}


/*-------------------------------------------------------------------*\
							resize
\*-------------------------------------------------------------------*/
void CBitmap::resize (sint32 nNewWidth, sint32 nNewHeight, TType newType, bool resetTo0)
{
	// Deleting mipmaps
	releaseMipMaps();

	// Change type of bitmap ?
	if (newType!=DonTKnow)
		PixelFormat=newType;

	_Width = nNewWidth;
	_Height = nNewHeight;

	// resize the level 0 only.
	resizeMipMap(0, nNewWidth, nNewHeight, resetTo0);
}


/*-------------------------------------------------------------------*\
							resizeMipMap
\*-------------------------------------------------------------------*/
void CBitmap::resizeMipMap (uint32 numMipMap, sint32 nNewWidth, sint32 nNewHeight, bool resetTo0)
{
	nlassert(numMipMap<MAX_MIPMAP);

	// free memory
	NLMISC::contReset(_Data[numMipMap]);

	// DXTC compressed??
	//bool	isDXTC= PixelFormat==DXTC1 || PixelFormat==DXTC1Alpha || PixelFormat==DXTC3 || PixelFormat==DXTC5;
	// if yes, must round up width and height to 4, for allocation
	nNewWidth= 4*((nNewWidth+3)/4);
	nNewHeight= 4*((nNewHeight+3)/4);

	// resize the buffer
	_Data[numMipMap].resize (((uint32)(nNewWidth*nNewHeight)*bitPerPixels[PixelFormat])/8);

	// Fill 0?
	if( resetTo0 )
		_Data[numMipMap].fill(0);
}


/*-------------------------------------------------------------------*\
							reset
\*-------------------------------------------------------------------*/
void CBitmap::setMipMapCount(uint32 mmc)
{
	_MipMapCount= uint8(mmc);
}


/*-------------------------------------------------------------------*\
							reset
\*-------------------------------------------------------------------*/
void CBitmap::reset(TType type)
{
	for(uint i=0; i<_MipMapCount; i++)
	{
		NLMISC::contReset(_Data[i]);
		_Data[i].resize(0);
	}
	_Width = _Height = 0;
	_MipMapCount= 1;

	// Change pixel format
	PixelFormat=type;
}



/*-------------------------------------------------------------------*\
							resamplePicture32
\*-------------------------------------------------------------------*/
void CBitmap::resamplePicture32 (const NLMISC::CRGBA *pSrc, NLMISC::CRGBA *pDest,
								 sint32 nSrcWidth, sint32 nSrcHeight,
								 sint32 nDestWidth, sint32 nDestHeight)
{
	//logResample("RP32: 0 pSrc=%p pDest=%p, Src=%d x %d Dest=%d x %d", pSrc, pDest, nSrcWidth, nSrcHeight, nDestWidth, nDestHeight);
	if ((nSrcWidth<=0)||(nSrcHeight<=0)||(nDestHeight<=0)||(nDestHeight<=0))
		return;

	// If we're reducing it by 2, call the fast resample
	if (((nSrcHeight / 2) == nDestHeight) && ((nSrcHeight % 2) == 0) &&
		((nSrcWidth  / 2) == nDestWidth)  && ((nSrcWidth  % 2) == 0))
	{
		resamplePicture32Fast(pSrc, pDest, nSrcWidth, nSrcHeight, nDestWidth, nDestHeight);
		return;
	}

	bool bXMag=(nDestWidth>=nSrcWidth);
	bool bYMag=(nDestHeight>=nSrcHeight);
	bool bXEq=(nDestWidth==nSrcWidth);
	bool bYEq=(nDestHeight==nSrcHeight);
	std::vector<NLMISC::CRGBAF> pIterm (nDestWidth*nSrcHeight);

	if (bXMag)
	{
		float fXdelta=(float)(nSrcWidth)/(float)(nDestWidth);
		NLMISC::CRGBAF *pItermPtr=&*pIterm.begin();
		sint32 nY;
		for (nY=0; nY<nSrcHeight; nY++)
		{
			const NLMISC::CRGBA *pSrcLine=pSrc;
			float fX=0.f;
			sint32 nX;
			for (nX=0; nX<nDestWidth; nX++)
			{
				float fVirgule=fX-(float)floor(fX);
				nlassert (fVirgule>=0.f);
				NLMISC::CRGBAF vColor;
				if (fVirgule>=0.5f)
				{
					if (fX<(float)(nSrcWidth-1))
					{
						NLMISC::CRGBAF vColor1 (pSrcLine[(sint32)floor(fX)]);
						NLMISC::CRGBAF vColor2 (pSrcLine[(sint32)floor(fX)+1]);
						vColor=vColor1*(1.5f-fVirgule)+vColor2*(fVirgule-0.5f);
					}
					else
						vColor=NLMISC::CRGBAF (pSrcLine[(sint32)floor(fX)]);
				}
				else
				{
					if (fX>=1.f)
					{
						NLMISC::CRGBAF vColor1 (pSrcLine[(sint32)floor(fX)]);
						NLMISC::CRGBAF vColor2 (pSrcLine[(sint32)floor(fX)-1]);
						vColor=vColor1*(0.5f+fVirgule)+vColor2*(0.5f-fVirgule);
					}
					else
						vColor=NLMISC::CRGBAF (pSrcLine[(sint32)floor(fX)]);
				}
				*(pItermPtr++)=vColor;
				fX+=fXdelta;
			}
			pSrc+=nSrcWidth;
		}
	}
	else if (bXEq)
	{
		NLMISC::CRGBAF *pItermPtr=&*pIterm.begin();
		for (sint32 nY=0; nY<nSrcHeight; nY++)
		{
			const NLMISC::CRGBA *pSrcLine=pSrc;
			sint32 nX;
			for (nX=0; nX<nDestWidth; nX++)
				*(pItermPtr++)=NLMISC::CRGBAF (pSrcLine[nX]);
			pSrc+=nSrcWidth;
		}
	}
	else
	{
		double fXdelta=(double)(nSrcWidth)/(double)(nDestWidth);
		nlassert (fXdelta>1.f);
		NLMISC::CRGBAF *pItermPtr=&*pIterm.begin();
		sint32 nY;
		for (nY=0; nY<nSrcHeight; nY++)
		{
			const NLMISC::CRGBA *pSrcLine=pSrc;
			double fX=0.f;
			sint32 nX;
			for (nX=0; nX<nDestWidth; nX++)
			{
				NLMISC::CRGBAF vColor (0.f, 0.f, 0.f, 0.f);
				double fFinal=fX+fXdelta;
				while ((fX<fFinal)&&((sint32)fX!=nSrcWidth))
				{
					double fNext=(double)floor (fX)+1.f;
					if (fNext>fFinal)
						fNext=fFinal;
					vColor+=((float)(fNext-fX))*NLMISC::CRGBAF (pSrcLine[(sint32)floor(fX)]);
					fX=fNext;
				}
				fX = fFinal; // ensure fX == fFinal
				vColor/=(float)fXdelta;
				*(pItermPtr++)=vColor;
			}
			pSrc+=nSrcWidth;
		}
	}

	if (bYMag)
	{
		double fYdelta=(double)(nSrcHeight)/(double)(nDestHeight);
		sint32 nX;
		for (nX=0; nX<nDestWidth; nX++)
		{
			double fY=0.f;
			sint32 nY;
			for (nY=0; nY<nDestHeight; nY++)
			{
				double fVirgule=fY-(double)floor(fY);
				nlassert (fVirgule>=0.f);
				NLMISC::CRGBAF vColor;
				if (fVirgule>=0.5f)
				{
					if (fY<(double)(nSrcHeight-1))
					{
						NLMISC::CRGBAF vColor1=pIterm[((sint32)floor(fY))*nDestWidth+nX];
						NLMISC::CRGBAF vColor2=pIterm[(((sint32)floor(fY))+1)*nDestWidth+nX];
						vColor=vColor1*(1.5f-(float)fVirgule)+vColor2*((float)fVirgule-0.5f);
					}
					else
						vColor=pIterm[((sint32)floor(fY))*nDestWidth+nX];
				}
				else
				{
					if (fY>=1.f)
					{
						NLMISC::CRGBAF vColor1=pIterm[((sint32)floor(fY))*nDestWidth+nX];
						NLMISC::CRGBAF vColor2=pIterm[(((sint32)floor(fY))-1)*nDestWidth+nX];
						vColor=vColor1*(0.5f+(float)fVirgule)+vColor2*(0.5f-(float)fVirgule);
					}
					else
						vColor=pIterm[((sint32)floor(fY))*nDestWidth+nX];
				}
				pDest[nX+nY*nDestWidth]=vColor;
				fY+=fYdelta;
			}
		}
	}
	else if (bYEq)
	{
		for (sint32 nX=0; nX<nDestWidth; nX++)
		{
			sint32 nY;
			for (nY=0; nY<nDestHeight; nY++)
			{
				pDest[nX+nY*nDestWidth]=pIterm[nY*nDestWidth+nX];
			}
		}
	}
	else
	{
		double fYdelta=(double)(nSrcHeight)/(double)(nDestHeight);
		nlassert (fYdelta>1.f);
		sint32 nX;
		for (nX=0; nX<nDestWidth; nX++)
		{
			double fY=0.f;
			sint32 nY;
			for (nY=0; nY<nDestHeight; nY++)
			{
				NLMISC::CRGBAF vColor (0.f, 0.f, 0.f, 0.f);
				double fFinal=fY+fYdelta;
				while ((fY<fFinal)&&((sint32)fY!=nSrcHeight))
				{
					double fNext=(double)floor (fY)+1.f;
					if (fNext>fFinal)
						fNext=fFinal;
					vColor+=((float)(fNext-fY))*pIterm[((sint32)floor(fY))*nDestWidth+nX];
					fY=fNext;
				}
				vColor/=(float)fYdelta;
				pDest[nX+nY*nDestWidth]=vColor;
			}
		}
	}
}

/*-------------------------------------------------------------------*\
							resamplePicture32Fast
\*-------------------------------------------------------------------*/
void CBitmap::resamplePicture32Fast (const NLMISC::CRGBA *pSrc, NLMISC::CRGBA *pDest,
									 sint32 nSrcWidth, sint32 nSrcHeight,
									 sint32 nDestWidth, sint32 nDestHeight)
{
	// the image is divided by two : 1 pixel in dest = 4 pixels in src
	// the resulting pixel in dest is an average of the four pixels in src

	nlassert(nSrcWidth  % 2 == 0);
	nlassert(nSrcHeight % 2 == 0);
	nlassert(nSrcWidth  / 2 == nDestWidth);
	nlassert(nSrcHeight / 2 == nDestHeight);

	sint32 x, y, twoX, twoSrcWidthByY;

	for (y=0 ; y<nDestHeight ; y++)
	{
		twoSrcWidthByY = 2*nSrcWidth*y;
		for (x=0 ; x<nDestWidth ; x++)
		{
			twoX = 2*x;
			pDest[x+y*nDestWidth].avg4( pSrc[twoX   + twoSrcWidthByY             ],
										pSrc[twoX   + twoSrcWidthByY + nSrcWidth ],
										pSrc[twoX+1 + twoSrcWidthByY             ],
										pSrc[twoX+1 + twoSrcWidthByY + nSrcWidth ]);
		}
	}
}



/*-------------------------------------------------------------------*\
							readTGA
\*-------------------------------------------------------------------*/
uint8 CBitmap::readTGA( NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	if(!f.isReading()) return 0;

	uint32			size;
	uint32			x,y;
	sint32			slsize;
	uint8			*scanline;
	uint8			r,g,b;
	sint32			i,j,k;

	// TGA file header fields
	uint8	lengthID;
	uint8	cMapType;
	uint8	imageType;
	uint16	origin;
	uint16	length;
	uint8	depth;
	uint16	xOrg;
	uint16	yOrg;
	uint16	width;
	uint16	height;
	uint8	imageDepth;
	uint8	desc;


	// Determining whether file is in Original or New TGA format

	bool newTgaFormat;
	uint32 extAreaOffset;
	uint32 devDirectoryOffset;
	char signature[16];

	f.seek (0, f.end);
	newTgaFormat = false;
	if (f.getPos() >= 26)
	{
		f.seek (-26, f.end);
		f.serial(extAreaOffset);
		f.serial(devDirectoryOffset);
		for(i=0; i<16; i++)
		{
			f.serial(signature[i]);
		}
		if(strncmp(signature,"TRUEVISION-XFILE",16)==0)
			newTgaFormat = true;
	}



	// Reading TGA file header
	f.seek (0, f.begin);

	f.serial(lengthID);
	f.serial(cMapType);
	f.serial(imageType);
	f.serial(origin);
	f.serial(length);
	f.serial(depth);
	f.serial(xOrg);
	f.serial(yOrg);
	f.serial(width);
	f.serial(height);
	f.serial(imageDepth);
	f.serial(desc);

	if(cMapType!=0)
	{
		nlinfo("readTga : color-map not supported");
	}

	if(lengthID>0)
	{
		uint8	dummy;
		for(i=0; i<lengthID; i++)
			f.serial(dummy);
	}



	// Reading TGA image data

	_Width = width;
	_Height = height;
	size = _Width * _Height * (imageDepth/8);

	switch(imageType)
	{
		// Uncompressed RGB or RGBA
		case 2:
		{
			_Data[0].resize(_Width*_Height*4);
			uint8 upSideDown = ((desc & (1 << 5))==0);
			slsize = _Width * imageDepth / 8;

			scanline = new uint8[slsize];
			if(!scanline)
			{
				throw EAllocationFailure();
			}

			for(y=0; y<_Height;y++)
			{
				// Serial buffer: more efficient way to load.
				f.serialBuffer (scanline, slsize);

				if(imageDepth==24 || imageDepth==32)
				{
					sint32 mult = 3;
					if(imageDepth==16)
					{
						mult = 2;
					}
					if(imageDepth==32)
					{
						mult = 4;
					}
					if(imageDepth!=16)
					{
						for(x=0; x<_Width; x++)
						{
							// RGB(A)
							r = scanline[x*mult+0];
							g = scanline[x*mult+1];
							b = scanline[x*mult+2];
							// Switching to BGR(A)
							scanline[x*mult+0] = b;
							scanline[x*mult+1] = g;
							scanline[x*mult+2] = r;
						}
					}
				}

				k=0;
				for(i=0; i<width; i++)
				{
					if(upSideDown)
					{
						if(imageDepth==16)
						{
							uint16 toto = (uint16)scanline[k++];
							toto |= scanline[k++]<<8;
							uint _r = toto>>10;
							uint _g = (toto>>5)&0x1f;
							uint _b = toto&0x1f;
							_Data[0][(height-y-1)*width*4 + 4*i] = uint8((_r<<3) | (_r>>2));
							_Data[0][(height-y-1)*width*4 + 4*i + 1] = uint8((_g<<3) | (_g>>2));
							_Data[0][(height-y-1)*width*4 + 4*i + 2] = uint8((_b<<3) | (_b>>2));
							_Data[0][(height-y-1)*width*4 + 4*i + 3] = 255;
						}
						else
						{
							_Data[0][(height-y-1)*width*4 + 4*i] = scanline[k++];
							_Data[0][(height-y-1)*width*4 + 4*i + 1] = scanline[k++];
							_Data[0][(height-y-1)*width*4 + 4*i + 2] = scanline[k++];
							if(imageDepth==32)
								_Data[0][(height-y-1)*width*4 + 4*i + 3] = scanline[k++];
							else
								_Data[0][(height-y-1)*width*4 + 4*i + 3] = 255;
						}
					}
					else
					{
						if(imageDepth==16)
						{
							uint16 toto = (uint16)scanline[k++];
							toto |= scanline[k++]<<8;
							int _r = toto>>10;
							int _g = toto&(0x3e0)>>5;
							int _b = toto&0x1f;
							_Data[0][y*width*4 + 4*i] = uint8((_r<<3) | (_r>>2));
							_Data[0][y*width*4 + 4*i + 1] = uint8((_g<<3) | (_g>>2));
							_Data[0][y*width*4 + 4*i + 2] = uint8((_b<<3) | (_b>>2));
							_Data[0][y*width*4 + 4*i + 3] = 255;
						}
						else
						{
							_Data[0][y*width*4 + 4*i] = scanline[k++];
							_Data[0][y*width*4 + 4*i + 1] = scanline[k++];
							_Data[0][y*width*4 + 4*i + 2] = scanline[k++];
							if(imageDepth==32)
								_Data[0][y*width*4 + 4*i + 3] = scanline[k++];
							else
								_Data[0][y*width*4 + 4*i + 3] = 255;
						}
					}
				}
			}

			PixelFormat = RGBA;
			delete []scanline;
		};
		break;

		// Uncompressed Grayscale bitmap
		case 3:
		{
			_Data[0].resize(_Width*_Height);
			uint8 upSideDown = ((desc & (1 << 5))==0);
			slsize = _Width;

			scanline = new uint8[slsize];
			if(!scanline)
			{
				throw EAllocationFailure();
			}

			for(y=0; y<_Height;y++)
			{
				// Serial buffer: more efficient way to load.
				f.serialBuffer (scanline, slsize);

				k=0;
				for(i=0; i<width; i++)
				{
					if(upSideDown)
						_Data[0][(height-y-1)*width + i] = scanline[k++];
					else
						_Data[0][y*width + i] = scanline[k++];
				}
			}

			PixelFormat = _LoadGrayscaleAsAlpha?Alpha:Luminance;
			delete []scanline;
		};
		break;

		// Compressed RGB or RGBA
		case 10:
		{
			uint8 packet;
			uint8 pixel[4];
			uint32 imageSize = width*height;
			uint32 readSize = 0;
			uint8 upSideDown = ((desc & (1 << 5))==0);
			_Data[0].resize(_Width*_Height*4);
			uint	dstId= 0;

			while(readSize < imageSize)
			{
				f.serial(packet);
				if((packet & 0x80) > 0) // packet RLE
				{
					for(i=0; i<imageDepth/8; i++)
					{
						f.serial(pixel[i]);
					}
					for (i=0; i < (packet & 0x7F) + 1; i++)
					{
						if(imageDepth==32)
						{
							_Data[0][dstId++]= pixel[2];
							_Data[0][dstId++]= pixel[1];
							_Data[0][dstId++]= pixel[0];
							_Data[0][dstId++]= pixel[3];
						}
						if(imageDepth==24)
						{
							_Data[0][dstId++]= pixel[2];
							_Data[0][dstId++]= pixel[1];
							_Data[0][dstId++]= pixel[0];
							_Data[0][dstId++]= 0;
						}
					}
				}
				else	// packet Raw
				{
					for(i=0; i<((packet & 0x7F) + 1); i++)
					{
						for(j=0; j<imageDepth/8; j++)
						{
							f.serial(pixel[j]);
						}
						if(imageDepth==32)
						{
							_Data[0][dstId++]= pixel[2];
							_Data[0][dstId++]= pixel[1];
							_Data[0][dstId++]= pixel[0];
							_Data[0][dstId++]= pixel[3];
						}
						if(imageDepth==24)
						{
							_Data[0][dstId++]= pixel[2];
							_Data[0][dstId++]= pixel[1];
							_Data[0][dstId++]= pixel[0];
							_Data[0][dstId++]= 0;
						}
					}
  				}
				readSize += (packet & 0x7F) + 1;
			}
			PixelFormat = RGBA;

			if (upSideDown) flipV();
		};
		break;

		// Compressed Grayscale bitmap (not tested)
		case 11:
		{
			uint8 packet;
			uint8 pixel[4];
			uint32 imageSize = width*height;
			uint32 readSize = 0;
			_Data[0].resize(_Width*_Height);
			uint	dstId= 0;

			while(readSize < imageSize)
			{
				f.serial(packet);
				if((packet & 0x80) > 0) // packet RLE
				{
					f.serial(pixel[0]);
					for (i=0; i < (packet & 0x7F) + 1; i++)
					{
						_Data[0][dstId++]= pixel[0];
					}
				}
				else	// packet Raw
				{
					for(i=0; i<((packet & 0x7F) + 1); i++)
					{
						f.serial(pixel[0]);
						_Data[0][dstId++]= pixel[0];
					}
  				}
				readSize += (packet & 0x7F) + 1;
			}
			PixelFormat = _LoadGrayscaleAsAlpha?Alpha:Luminance;
		};
		break;

		default:
			return 0;
	}

	_MipMapCount = 1;
	return(imageDepth);

}

/*-------------------------------------------------------------------*\
							writeTGA
\*-------------------------------------------------------------------*/
bool CBitmap::writeTGA( NLMISC::IStream &f, uint32 d, bool upsideDown)
{
	if(f.isReading()) return false;
	if (d==0)
	{
		switch (PixelFormat)
		{
		case RGBA:
			d = 32;
			break;
		case Luminance:
			d = 8;
			break;
		case Alpha:
			d = 8;
			break;
		default:
			;
		}
	}
	if(d!=24 && d!=32 && d!=16 && d!=8) return false;
	if ((PixelFormat != RGBA)&&(PixelFormat != Alpha)&&(PixelFormat != Luminance)) return false;
	if ((PixelFormat == Alpha) && (d != 8)) return false;
	if ((PixelFormat == Luminance) && (d != 8)) return false;

	sint32	i,j,x,y;
	uint8	* scanline;
	uint8	r,g,b,a;

	uint8	lengthID = 0;
	uint8	cMapType = 0;
	uint8	imageType = 2;
	uint16	origin = 0;
	uint16	length = 0;
	uint8	depth = 0;
	uint16	xOrg = 0;
	uint16	yOrg = 0;
	uint16	width = (uint16)_Width;
	uint16	height = (uint16)_Height;
	uint8	imageDepth = (uint8)d;
	uint8	desc = 0;
	if (upsideDown)
		desc |= 1<<5;

	if ((PixelFormat == Alpha) || (PixelFormat == Luminance))
		imageType = 3; // Uncompressed grayscale

	f.serial(lengthID);
	f.serial(cMapType);
	f.serial(imageType);
	f.serial(origin);
	f.serial(length);
	f.serial(depth);
	f.serial(xOrg);
	f.serial(yOrg);
	f.serial(width);
	f.serial(height);
	f.serial(imageDepth);
	f.serial(desc);

	if ((PixelFormat == Alpha)||(PixelFormat == Luminance))
		scanline = new uint8[width];
	else
		scanline = new uint8[width*4];
	if(!scanline)
	{
		throw EAllocationFailure();
	}

	for(y=0; y<(sint32)height; y++)
	{

		uint32 k=0;
		if (PixelFormat == Alpha)
		{
			for(i=0; i<width; ++i) // Alpha
			{
				scanline[k++] = _Data[0][(height-y-1)*width + i];
			}
		}
		else if (PixelFormat == Luminance)
		{
			for(i=0; i<width; ++i) // Luminance
			{
				scanline[k++] = _Data[0][(height-y-1)*width + i];
			}
		}
		else
		{
			for(i=0; i<width*4; i+=4) // 4:RGBA
			{
				if(d==16)
				{
					for(j=0; j<(sint32)4; j++)
					{
						scanline[k++] = _Data[0][(height-y-1)*width*4 + i + j];
					}
				}
				else
				{
					for(j=0; j<(sint32)d/8; j++)
					{
						scanline[k++] = _Data[0][(height-y-1)*width*4 + i + j];
					}
				}
			}
		}

		if(d==16)
		{
			for(x=0; x<(sint32)width; x++)
			{
				r = scanline[x*4+0];
				g = scanline[x*4+1];
				b = scanline[x*4+2];
				int rr = r >>3;
				int gg = g >>3;
				int bb = b >>3;
				uint16 c16 = uint16((rr<<10) | (gg<<5) | bb);
				scanline[x*2+0] = c16&0xff;
				scanline[x*2+1] = c16>>8;
			}
		}
		if(d==24)
		{
			for(x=0; x<(sint32)width; x++)
			{
				r = scanline[x*3+0];
				g = scanline[x*3+1];
				b = scanline[x*3+2];
				scanline[x*3+0] = b;
				scanline[x*3+1] = g;
				scanline[x*3+2] = r;
			}
		}
		if(d==32)
		{
			for(x=0; x<(sint32)width; x++)
			{
				r = scanline[x*4+0];
				g = scanline[x*4+1];
				b = scanline[x*4+2];
				a= scanline[x*4+3];
				scanline[x*4+0] = b;
				scanline[x*4+1] = g;
				scanline[x*4+2] = r;
				scanline[x*4+3] = a;
			}
		}

		int finaleSize=width*d/8;
		for(i=0; i<finaleSize; i++)
		{
			f.serial(scanline[i]);
		}
	}
	delete []scanline;
	return true;
}


template<class T>
void rotateCCW (const T* src, T* dst, uint srcWidth, uint srcHeight)
{
	for (uint y=0; y<srcHeight; y++)
	for (uint x=0; x<srcWidth; x++)
	{
		uint dstX=y;
		uint dstY=srcWidth-x-1;
		dst[dstX+dstY*srcHeight]=src[x+y*srcWidth];
	}
}

/*template<class T>
void rotateCCW (const vector<T>& src, vector<T>& dst, uint srcWidth, uint srcHeight)
{
	for (uint y=0; y<srcHeight; y++)
	for (uint x=0; x<srcWidth; x++)
	{
		uint dstX=y;
		uint dstY=srcWidth-x;
		dst[dstX+dstY*srcHeight]=src[x+y*srcWidth];
	}
}
*/
void CBitmap::rotateCCW()
{
	// Copy the array
	CObjectVector<uint8> copy=_Data[0];

	switch (PixelFormat)
	{
	case RGBA:
		NLMISC::rotateCCW ((uint32*)&(_Data[0][0]), (uint32*)&(copy[0]), _Width, _Height);
		break;
	case Luminance:
	case Alpha:
		NLMISC::rotateCCW (&_Data[0][0], &copy[0], _Width, _Height);
		break;
	case AlphaLuminance:
		NLMISC::rotateCCW ((uint16*)&(_Data[0][0]), (uint16*)&(copy[0]), _Width, _Height);;
		break;
	default: break;
	}

	uint32 tmp=_Width;
	_Width=_Height;
	_Height=tmp;
	_Data[0]=copy;
}

void CBitmap::blit(const CBitmap &src, sint srcX, sint srcY, sint srcWidth, sint srcHeight, sint destX, sint destY)
{
	nlassert(PixelFormat == RGBA);
	nlassert(src.PixelFormat == RGBA);
	// clip x
	if (srcX < 0)
	{
		srcWidth += srcX;
		if (srcWidth <= 0) return;
		destX -= srcX;
		srcX = 0;
	}
	if (srcX + srcWidth > (sint) src.getWidth())
	{
		srcWidth = src.getWidth() - srcX;
		if (srcWidth <= 0) return;
	}
	if (destX < 0)
	{
		srcWidth += destX;
		if (srcWidth <= 0) return;
		srcX -= destX;
		destX = 0;
	}
	if (destX + srcWidth > (sint) getWidth())
	{
		srcWidth = getWidth() - destX;
		if (srcWidth <= 0) return;
	}
	// clip y
	if (srcY < 0)
	{
		srcHeight += srcY;
		if (srcHeight <= 0) return;
		destY -= srcY;
		srcY = 0;
	}
	if (srcY + srcHeight > (sint) src.getHeight())
	{
		srcHeight = src.getHeight() - srcY;
		if (srcHeight <= 0) return;
	}
	if (destY < 0)
	{
		srcHeight += destY;
		if (srcHeight <= 0) return;
		srcY -= destY;
		destY = 0;
	}
	if (destY + srcHeight > (sint) getHeight())
	{
		srcHeight = getHeight() - destY;
		if (srcHeight <= 0) return;
	}
	uint32 *srcPixels = (uint32 *) &src.getPixels()[0];
	uint32 *srcPtr = &(srcPixels[srcX + srcY * src.getWidth()]);
	uint32 *srcEndPtr = srcPtr + srcHeight * src.getWidth();
	uint32 *destPixels = (uint32 *) &getPixels()[0];
	uint32 *destPtr = 	&(destPixels[destX + destY * getWidth()]);
	while (srcPtr != srcEndPtr)
	{
		memcpy(destPtr, srcPtr, sizeof(uint32) * srcWidth);
		srcPtr += src.getWidth();
		destPtr += getWidth();
	}

}


bool CBitmap::blit(const CBitmap *src, sint32 x, sint32 y)
{

	nlassert(this->PixelFormat == src->PixelFormat);
	if (this->PixelFormat != src->PixelFormat)
	{
		return false;
	}


	// check for dxtc use

	const bool useDXTC   =  PixelFormat == DXTC1 || PixelFormat == DXTC1Alpha || PixelFormat == DXTC3 || PixelFormat ==	DXTC5;

	// number of bits for a 4x4 pix block
	const uint dxtcNumBits  =  PixelFormat == DXTC1 || PixelFormat == DXTC1Alpha ? 64 : 128;


	if (useDXTC)
	{
		// blit pos must be multiple of 4

		nlassert(! (x & 3 || y & 3) );
		if (x & 3 || y & 3) return false;

	}

	nlassert(PixelFormat != DonTKnow);

	// the width to copy
	sint width = src->_Width;
	// the height to copy
	sint height = src->_Height;

	uint destStartX, destStartY;
	uint srcStartX, srcStartY;


	// clip against left
	if (x < 0)
	{
		width += x;
		if (width <= 0) return true;
		destStartX = 0;
		srcStartX = -x;
	}
	else
	{
		destStartX = x;
		srcStartX = 0;
	}

	// clip against top
	if (y < 0)
	{
		height += y;
		if (height <= 0) return true;
		srcStartY = -y;
		destStartY = 0;
	}
	else
	{
		destStartY = y;
		srcStartY = 0;
	}

	// clip against right
	if ((destStartX + width - 1) >= _Width)
	{
		width = _Width - destStartX;
		if (width <= 0) return true;
	}

	// clip against bottom
	if ((destStartY + height - 1) >= _Height)
	{
		height = _Height - destStartY;
		if (width <= 0) return true;
	}


	// divide all distance by 4 when using DXTC
	if (useDXTC)
	{
		destStartX >>= 2;
		destStartY >>= 2;
		srcStartX >>= 2;
		srcStartY >>= 2;
		width >>= 2;
		height >>= 2;
	}


	// bytes per pixs is for either one pixel or 16 (a 4x4 block in DXTC)
	const uint bytePerPixs = ( useDXTC ? dxtcNumBits : bitPerPixels[PixelFormat] ) >> 3 /* divide by 8 to get the number of bytes */;


	const uint destRealWidth = useDXTC ? (_Width >> 2) : _Width;
	const uint srcRealWidth = useDXTC ? (src->_Width >> 2) : src->_Width;


	// size to go to the next line in the destination
	const uint destStride = destRealWidth * bytePerPixs;

	// size to go to the next line in the source
	const uint srcStride = srcRealWidth * bytePerPixs;

	// length in bytes of a line to copy
	const uint lineLength = width * bytePerPixs;


	uint8  *destPos = &(_Data[0][0]) + destStride * destStartY + bytePerPixs * destStartX;
	const uint8 *srcPos = &(src->_Data[0][0]) + srcStride * srcStartY + bytePerPixs * srcStartX;

	// copy each hline
	for (sint k = 0; k < height; ++k)
	{
		::memcpy(destPos, srcPos, lineLength);
		destPos += destStride;
		srcPos += srcStride;
	}


	return true;
}

// Private :
float CBitmap::getColorInterp (float x, float y, float colorInXY00, float colorInXY10, float colorInXY01, float colorInXY11) const
{
	float res =	colorInXY00*(1.0f-x)*(1.0f-y) +
				colorInXY10*(     x)*(1.0f-y) +
				colorInXY01*(1.0f-x)*(     y) +
				colorInXY11*(     x)*(     y);
	clamp (res, 0.0f, 255.0f);
	return res;
}

// Public:
CRGBAF CBitmap::getColor (float x, float y) const
{
		if (x < 0.0f) x = 0.0f;
	if (x > 1.0f) x = 1.0f;
	if (y < 0.0f) y = 0.0f;
	if (y > 1.0f) y = 1.0f;

	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);

	if (nWidth == 0 || nHeight == 0) return CRGBAF(0, 0, 0, 0);

	const CObjectVector<uint8> &rBitmap = getPixels(0);
	sint32 nX[4], nY[4];

	x *= nWidth-1;
	y *= nHeight-1;

	// Integer part of (x,y)
	//nX[0] = ((sint32)floor(x-0.5f));
	//nY[0] = ((sint32)floor(y-0.5f));
	nX[0] = ((sint32)floor(x));
	nY[0] = ((sint32)floor(y));

	nX[1] = (nX[0] < (nWidth-1) ? nX[0]+1 : nX[0]);
	nY[1] = nY[0];

	nX[2] = nX[0];
	nY[2] = (nY[0] < (nHeight-1) ? nY[0]+1 : nY[0]);

	nX[3] = nX[1];
	nY[3] = nY[2];

	uint32 i;

	for (i = 0; i < 4; ++i)
	{
		nlassert (nX[i] >= 0);
		nlassert (nY[i] >= 0 );
		nlassert (nX[i] < nWidth);
		nlassert (nY[i] < nHeight);
	}

	// Decimal part of (x,y)
	x = x - (float)nX[0];
	y = y - (float)nY[0];

	switch (this->PixelFormat)
	{
		case RGBA:
		case DXTC1:
		case DXTC1Alpha:
		case DXTC3:
		case DXTC5:
		{
			CRGBAF finalVal;
			CRGBA val[4];

			if (this->PixelFormat == RGBA)
			{
				for (i = 0; i < 4; ++i)
				{
					val[i] = CRGBA (rBitmap[(nX[i]+nY[i]*nWidth)*4+0],
									rBitmap[(nX[i]+nY[i]*nWidth)*4+1],
									rBitmap[(nX[i]+nY[i]*nWidth)*4+2],
									rBitmap[(nX[i]+nY[i]*nWidth)*4+3]);
				}
			}
			else
			{
				// slower version : get from DXT
				for (i = 0; i < 4; ++i)
				{
					val[i] = getPixelColor(nX[i], nY[i]);
				}
			}

			finalVal.R = getColorInterp (x, y, val[0].R, val[1].R, val[2].R, val[3].R);
			finalVal.G = getColorInterp (x, y, val[0].G, val[1].G, val[2].G, val[3].G);
			finalVal.B = getColorInterp (x, y, val[0].B, val[1].B, val[2].B, val[3].B);
			finalVal.A = getColorInterp (x, y, val[0].A, val[1].A, val[2].A, val[3].A);
			finalVal /= 255.f;

			return finalVal;
		}
		break;
		case Alpha:
		case Luminance:
		{

			float finalVal;
			float val[4];

			for (i = 0; i < 4; ++i)
				val[i] = rBitmap[(nX[i]+nY[i]*nWidth)];

			finalVal = getColorInterp (x, y, val[0], val[1], val[2], val[3]);
			finalVal /= 255.f;

			if (this->PixelFormat == Alpha)
				return CRGBAF (1.f, 1.f, 1.f, finalVal);
			else // Luminance
				return CRGBAF (finalVal, finalVal, finalVal, 1.f);
		}
		break;
		default: break;
	}

	return CRGBAF (0.0f, 0.0f, 0.0f, 0.0f);
}

// wrap a value inside the given range (for positive value it is like a modulo)
static inline uint32 wrap(sint32 value, uint32 range)
{
	return value >= 0 ? (value % range) : range - 1 - (- value - 1) % range;
}


CRGBAF CBitmap::getColor(float x, float y, bool tileU, bool tileV) const
{
	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	if (nWidth == 0 || nHeight == 0) return CRGBAF(0, 0, 0, 0);

	sint32 nX[4], nY[4];

	if (!tileU)
	{
		if (x < 0.0f) x = 0.0f;
		if (x > 1.0f) x = 1.0f;
		x *= nWidth-1;
		nX[0] = ((sint32)floor(x));
		nX[1] = (nX[0] < (nWidth-1) ? nX[0]+1 : nX[0]);
		nX[2] = nX[0];
		nX[3] = nX[1];
		uint32 i;
		for (i = 0; i < 4; ++i)
		{
			nlassert (nX[i] >= 0);
			nlassert (nX[i] < nWidth);
		}
	}
	else
	{
		x *= nWidth;
		nX[0] = wrap((sint32)floorf(x), nWidth);
		nX[1] = wrap(nX[0] + 1, nWidth);
		nX[2] = nX[0];
		nX[3] = nX[1];
	}
	//
	if (!tileV)
	{
		if (y < 0.0f) y = 0.0f;
		if (y > 1.0f) y = 1.0f;
		y *= nHeight-1;
		nY[0] = ((sint32)floor(y));
		nY[1] = nY[0];
		nY[2] = (nY[0] < (nHeight-1) ? nY[0]+1 : nY[0]);
		nY[3] = nY[2];
		uint32 i;
		for (i = 0; i < 4; ++i)
		{
			nlassert (nY[i] >= 0 );
			nlassert (nY[i] < nHeight);
		}
	}
	else
	{
		y *= nHeight;
		nY[0] = wrap((sint32)floorf(y), nHeight);
		nY[1] = nY[0];
		nY[2] = wrap(nY[0] + 1, nHeight);
		nY[3] = nY[2];
	}
	// Decimal part of (x,y)
	x = x - (float)nX[0];
	y = y - (float)nY[0];
	const CObjectVector<uint8> &rBitmap = getPixels(0);
	switch (this->PixelFormat)
	{
		case RGBA:
		case DXTC1:
		case DXTC1Alpha:
		case DXTC3:
		case DXTC5:
		{
			CRGBAF finalVal;
			CRGBA val[4];

			if (this->PixelFormat == RGBA)
			{
				for (uint32 i = 0; i < 4; ++i)
				{
					val[i] = CRGBA (rBitmap[(nX[i]+nY[i]*nWidth)*4+0],
									rBitmap[(nX[i]+nY[i]*nWidth)*4+1],
									rBitmap[(nX[i]+nY[i]*nWidth)*4+2],
									rBitmap[(nX[i]+nY[i]*nWidth)*4+3]);
				}
			}
			else
			{
				// slower version : get from DXT
				for (uint32 i = 0; i < 4; ++i)
				{
					val[i] = getPixelColor(nX[i], nY[i]);
				}
			}

			finalVal.R = getColorInterp (x, y, val[0].R, val[1].R, val[2].R, val[3].R);
			finalVal.G = getColorInterp (x, y, val[0].G, val[1].G, val[2].G, val[3].G);
			finalVal.B = getColorInterp (x, y, val[0].B, val[1].B, val[2].B, val[3].B);
			finalVal.A = getColorInterp (x, y, val[0].A, val[1].A, val[2].A, val[3].A);
			finalVal /= 255.f;

			return finalVal;
		}
		break;
		case Alpha:
		case Luminance:
		{

			float finalVal;
			float val[4];

			for (uint32 i = 0; i < 4; ++i)
				val[i] = rBitmap[(nX[i]+nY[i]*nWidth)];

			finalVal = getColorInterp (x, y, val[0], val[1], val[2], val[3]);
			finalVal /= 255.f;

			if (this->PixelFormat == Alpha)
				return CRGBAF (1.f, 1.f, 1.f, finalVal);
			else // Luminance
				return CRGBAF (finalVal, finalVal, finalVal, 1.f);
		}
		break;
		default: break;
	}
	return CRGBAF (0.0f, 0.0f, 0.0f, 0.0f);
}



void	CBitmap::loadSize(NLMISC::IStream &f, uint32 &retWidth, uint32 &retHeight)
{
	retWidth= 0;
	retHeight= 0;

	nlassert(f.isReading());

	// testing if DDS
	uint32 fileType = 0;
	f.serial(fileType);
	if(fileType == DDS_HEADER)
	{
		// read entire DDS header.
		uint32 size = 0;
		f.serial(size); // size in Bytes of header(without "DDS")
		uint32 * _DDSSurfaceDesc = new uint32[size];
		_DDSSurfaceDesc[0]= size;

		for(uint i= 0; i<size/4 - 1; i++)
		{
			f.serial(_DDSSurfaceDesc[i+1]);
		}

		// flags determines which members of the header structure contain valid data
		uint32 flags = _DDSSurfaceDesc[1];

		//verify if file have linearsize set
		if(!(flags & DDSD_LINEARSIZE))
		{
			nlwarning("A DDS doesn't have the flag DDSD_LINEARSIZE");
			//delete [] _DDSSurfaceDesc;
			//throw EDDSBadHeader();
		}

		//-------------- extracting and testing useful info
		retHeight  = _DDSSurfaceDesc[2];
		retWidth = _DDSSurfaceDesc[3];

		delete [] _DDSSurfaceDesc;
	}
	else if(fileType == PNG_HEADER)
	{
		// check second part of header
		f.serialCheck(0x0a1a0a0d);

		uint32 chunkLength = 0;
		uint32 chunkName = 0;
		bool eof = false;

		do
		{
			try
			{
				// length of chunk data
				f.serial(chunkLength);
				NLMISC_BSWAP32(chunkLength);

				// name of chunk
				f.serial(chunkName);

				// size of image is a part of IHDR chunk
				if (chunkName == NL_MAKEFOURCC('I', 'H', 'D', 'R'))
				{
					uint32 val;
					f.serial(val);
					NLMISC_BSWAP32(val);
					retWidth = val;

					f.serial(val);
					NLMISC_BSWAP32(val);
					retHeight = val;

					break;
				}
				// end of file chunk
				else if (chunkName == NL_MAKEFOURCC('I', 'E', 'N', 'D'))
				{
					break;
				}

				// skip data of this chunk and CRC32
				f.seek(chunkLength+4, IStream::current);
			}
			catch(...)
			{
				eof = true;
			}
		}
		while(!eof);
	}
	else if(fileType == JPG_HEADER)
	{
		uint8 blockMarker1 = 0;
		uint8 blockMarker2 = 0;
		uint16 blockSize = 0;
		bool eof = false;

		do
		{
			try
			{
				// marker of a block
				f.serial(blockMarker1);

				if (blockMarker1 == 0xff)
				{
					// marker of a block
					f.serial(blockMarker2);

					// 0xff00 is only found in image data
					if (blockMarker2 == 0x00)
					{
						// image data 0xff
					}
					// 0xffda is image data
					else if (blockMarker2 == 0xda)
					{
						// next data is image data which must end with 0xffd9
					}
					// 0xffd9 is the end of an image
					else if (blockMarker2 == 0xd9)
					{
						// real end of file
						break;
					}
					else if (blockMarker2 == 0xdd || blockMarker2 == 0xdc)
					{
						f.seek(4, IStream::current);
					}
					else if (blockMarker2 == 0xdf)
					{
						f.seek(3, IStream::current);
					}
					else if (blockMarker2 >= 0xd0 && blockMarker2 <= 0xd8)
					{
						// no content
					}
					else
					{
						// size of a block
						f.serial(blockSize);
						NLMISC_BSWAP16(blockSize);

						// frame marker (which contains image width and height)
						if (blockMarker2 >= 0xc0 && blockMarker2 <= 0xc3)
						{
							uint8 imagePrecision = 0; // sample precision
							uint32 imageSize = 0; // width and height
							f.serial(imagePrecision); 
							f.serial(imageSize);
							NLMISC_BSWAP32(imageSize);

							retWidth = imageSize & 0xffff;
							retHeight = (imageSize & 0xffff0000) >> 16;

							break;
						}

						// skip the block
						f.seek(blockSize - 2, IStream::current);
					}
				}
			}
			catch(...)
			{
				eof = true;
			}
		}
		while(!eof);
	}
	// assuming it's TGA
	else
	{
		if(!f.seek (0, NLMISC::IStream::begin))
		{
			throw ESeekFailed();
		}

		// Reading header,
		// To make sure that the bitmap is TGA, we check imageType and imageDepth.
		uint8	lengthID;
		uint8	cMapType;
		uint8	imageType;
		uint16	tgaOrigin;
		uint16	length;
		uint8	depth;
		uint16	xOrg;
		uint16	yOrg;
		uint16	width;
		uint16	height;
		uint8	imageDepth;
		uint8	desc;

		f.serial(lengthID);
		f.serial(cMapType);
		f.serial(imageType);
		if(imageType!=2 && imageType!=3 && imageType!=10 && imageType!=11)
		{
			nlwarning("Invalid TGA format, type %u in not supported (must be 2,3,10 or 11)", imageType);
			return;
		}
		f.serial(tgaOrigin);
		f.serial(length);
		f.serial(depth);
		f.serial(xOrg);
		f.serial(yOrg);
		f.serial(width);
		f.serial(height);
		f.serial(imageDepth);
		if(imageDepth!=8 && imageDepth!=16 && imageDepth!=24 && imageDepth!=32)
		{
			nlwarning("Invalid TGA format, bit depth %u in not supported (must be 8,16,24 or 32)", imageDepth);
			return;
		}
		f.serial(desc);

		// Ok, we have width and height.
		retWidth= width;
		retHeight= height;
	}

	// reset stream.
	if(!f.seek (0, NLMISC::IStream::begin))
	{
		throw ESeekFailed();
	}
}


void	CBitmap::loadSize(const std::string &path, uint32 &retWidth, uint32 &retHeight)
{
	retWidth= 0;
	retHeight= 0;

	CIFile		f(path);
	if(f.open(path))
		loadSize(f, retWidth, retHeight);
}

// ***************************************************************************
void	CBitmap::flipHDXTCBlockColor(uint8 *bitColor, uint32 w)
{
	// pack each line in a u32 (NB: the following works either in Little and Big Endian)
	uint32	bits= *(uint32*)bitColor;

	// swap in X for each line
	uint32	res;
	if(w!=2)
	{
		res = (bits & 0xC0C0C0C0) >> 6;
		res+= (bits & 0x30303030) >> 2;
		res+= (bits & 0x0C0C0C0C) << 2;
		res+= (bits & 0x03030303) << 6;
	}
	// special case where w==2
	else
	{
		res = (bits & 0x0C0C0C0C) >> 2;
		res+= (bits & 0x03030303) << 2;
	}

	// copy
	*((uint32*)bitColor)= res;
}

// ***************************************************************************
void	CBitmap::flipVDXTCBlockColor(uint8 *bitColor, uint32 h)
{
	// swap just bytes (work either in Little and Big Endian)
	if(h!=2)
	{
		std::swap(bitColor[0], bitColor[3]);
		std::swap(bitColor[1], bitColor[2]);
	}
	// special case where h==2)
	else
	{
		// whatever Little or Big endian, the first byte is the first line, and the second byte is the second line
		std::swap(bitColor[0], bitColor[1]);
	}
}

// ***************************************************************************
void	CBitmap::flipHDXTCBlockAlpha3(uint8 *blockAlpha, uint32 w)
{
#ifdef NL_LITTLE_ENDIAN
	uint64	bits= *(uint64*)blockAlpha;
#else
	uint64	bits= (uint64)blockAlpha[0] + ((uint64)blockAlpha[1]<<8) +
		((uint64)blockAlpha[2]<<16) + ((uint64)blockAlpha[3]<<24) +
		((uint64)blockAlpha[4]<<32) + ((uint64)blockAlpha[5]<<40) +
		((uint64)blockAlpha[6]<<48) + ((uint64)blockAlpha[7]<<56);
#endif

	// swap in X for each line
	uint64	res;
	if(w!=2)
	{
		res = (bits & INT64_CONSTANT(0xF000F000F000F000)) >> 12;
		res+= (bits & INT64_CONSTANT(0x0F000F000F000F00)) >> 4;
		res+= (bits & INT64_CONSTANT(0x00F000F000F000F0)) << 4;
		res+= (bits & INT64_CONSTANT(0x000F000F000F000F)) << 12;
	}
	// special case where w==2
	else
	{
		res = (bits & INT64_CONSTANT(0x00F000F000F000F0)) >> 4;
		res+= (bits & INT64_CONSTANT(0x000F000F000F000F)) << 4;
	}

	// copy
#ifdef NL_LITTLE_ENDIAN
	*((uint64*)blockAlpha)= res;
#else
	blockAlpha[0]= res & 255;
	blockAlpha[1]= (res>>8) & 255;
	blockAlpha[2]= (res>>16) & 255;
	blockAlpha[3]= (res>>24) & 255;
	blockAlpha[4]= (res>>32) & 255;
	blockAlpha[5]= (res>>40) & 255;
	blockAlpha[6]= (res>>48) & 255;
	blockAlpha[7]= (res>>56) & 255;
#endif
}

// ***************************************************************************
void	CBitmap::flipVDXTCBlockAlpha3(uint8 *blockAlpha, uint32 h)
{
	uint16	*wAlpha= (uint16*)blockAlpha;

	// swap just words (work either in Little and Big Endian)
	if(h!=2)
	{
		std::swap(wAlpha[0], wAlpha[3]);
		std::swap(wAlpha[1], wAlpha[2]);
	}
	// special case where h==2)
	else
	{
		// whatever Little or Big endian, the first byte is the first line, and the second byte is the second line
		std::swap(wAlpha[0], wAlpha[1]);
	}
}

// ***************************************************************************
void	CBitmap::flipHDXTCBlockAlpha5(uint8 *bitAlpha, uint32 w)
{
	// pack into bits. Little Indian in all cases
	uint64	bits= (uint64)bitAlpha[0] + ((uint64)bitAlpha[1]<<8) +
		((uint64)bitAlpha[2]<<16) + ((uint64)bitAlpha[3]<<24) +
		((uint64)bitAlpha[4]<<32) + ((uint64)bitAlpha[5]<<40);

	// swap in X for each line
	uint64	res;
	if(w!=2)
	{
		res = (bits & INT64_CONSTANT(0xE00E00E00E00)) >> 9;
		res+= (bits & INT64_CONSTANT(0x1C01C01C01C0)) >> 3;
		res+= (bits & INT64_CONSTANT(0x038038038038)) << 3;
		res+= (bits & INT64_CONSTANT(0x007007007007)) << 9;
	}
	// special case where w==2
	else
	{
		res = (bits & INT64_CONSTANT(0x038038038038)) >> 3;
		res+= (bits & INT64_CONSTANT(0x007007007007)) << 3;
	}

	// copy. Little Indian in all cases
	bitAlpha[0]= uint8(res & 255);
	bitAlpha[1]= uint8((res>>8) & 255);
	bitAlpha[2]= uint8((res>>16) & 255);
	bitAlpha[3]= uint8((res>>24) & 255);
	bitAlpha[4]= uint8((res>>32) & 255);
	bitAlpha[5]= uint8((res>>40) & 255);
}

// ***************************************************************************
void	CBitmap::flipVDXTCBlockAlpha5(uint8 *bitAlpha, uint32 h)
{
	// pack into bits. Little Indian in all cases
	uint64	bits= (uint64)bitAlpha[0] + ((uint64)bitAlpha[1]<<8) +
		((uint64)bitAlpha[2]<<16) + ((uint64)bitAlpha[3]<<24) +
		((uint64)bitAlpha[4]<<32) + ((uint64)bitAlpha[5]<<40);

	// swap in Y
	uint64	res;
	if(h!=2)
	{
		res = (bits & INT64_CONSTANT(0xFFF000000000)) >> 36;
		res+= (bits & INT64_CONSTANT(0x000FFF000000)) >> 12;
		res+= (bits & INT64_CONSTANT(0x000000FFF000)) << 12;
		res+= (bits & INT64_CONSTANT(0x000000000FFF)) << 36;
	}
	// special case where h==2
	else
	{
		res = (bits & INT64_CONSTANT(0x000000FFF000)) >> 12;
		res+= (bits & INT64_CONSTANT(0x000000000FFF)) << 12;
	}

	// copy. Little Indian in all cases
	bitAlpha[0]= uint8(res & 255);
	bitAlpha[1]= uint8((res>>8) & 255);
	bitAlpha[2]= uint8((res>>16) & 255);
	bitAlpha[3]= uint8((res>>24) & 255);
	bitAlpha[4]= uint8((res>>32) & 255);
	bitAlpha[5]= uint8((res>>40) & 255);
}

// ***************************************************************************
void	CBitmap::flipDXTCMipMap(bool vertical, uint mm, uint type)
{
	nlassert(mm<MAX_MIPMAP);
	// size of a DXTC block. 64 bits (2 U32) for DXTC1, else 128 bits (4*U32)
	uint	blockSizeU32= type==1? 2 : 4;
	// get size in block
	sint32	width = getWidth(mm);
	sint32	height = getHeight(mm);
	if(width==0 || height==0)
		return;
	uint32	wBlock= (width+3)/4;
	uint32	hBlock= (height+3)/4;
	// get data ptr and check size.
	uint32	*data= (uint32*)(&_Data[mm][0]);
	nlassert(_Data[mm].size()==wBlock*hBlock*blockSizeU32*4);

	// get the offset (in bytes) to the start of color pixels bits
	uint32	offsetColorBits= type==1? 4 : 12;

	// abort if swap is nonsense
	if(vertical && height==1)
		return;
	if(!vertical && width==1)
		return;

	// *** First reverse Blocks
	if(vertical)
	{
		// reverse vertical
		for(uint yBlock=0;yBlock<hBlock/2;yBlock++)
		{
			uint32	*src0= data + (yBlock*wBlock)*blockSizeU32;
			uint32	*src1= data + ((hBlock-yBlock-1)*wBlock)*blockSizeU32;
			for(uint xBlock=0;xBlock<wBlock;xBlock++, src0+=blockSizeU32, src1+=blockSizeU32)
			{
				uint32	*block0= src0;
				uint32	*block1= src1;
				// swap the blocks
				for(uint i=0;i<blockSizeU32;i++, block0++, block1++)
				{
					std::swap(*block0, *block1);
				}
			}
		}
	}
	else
	{
		// reverse horizontal
		for(uint yBlock=0;yBlock<hBlock;yBlock++)
		{
			uint32	*src0= data + (yBlock*wBlock)*blockSizeU32;
			uint32	*src1= data + (yBlock*wBlock + wBlock-1)*blockSizeU32;
			for(uint xBlock=0;xBlock<wBlock/2;xBlock++, src0+=blockSizeU32, src1-=blockSizeU32)
			{
				uint32	*block0= src0;
				uint32	*block1= src1;
				// swap the blocks
				for(uint i=0;i<blockSizeU32;i++, block0++, block1++)
				{
					std::swap(*block0, *block1);
				}
			}
		}
	}

	// *** Then reverse Bits
	for(uint yBlock=0;yBlock<hBlock;yBlock++)
	{
		uint32	*src= data + (yBlock*wBlock)*blockSizeU32;
		for(uint xBlock=0;xBlock<wBlock;xBlock++, src+=blockSizeU32)
		{
			uint8	*block= (uint8*)src;

			// flip color bits
			if(vertical)	flipVDXTCBlockColor(block+offsetColorBits, height);
			else			flipHDXTCBlockColor(block+offsetColorBits, width);

			// flip alpha bits if any
			if(type==3)
			{
				// point to the alpha part (16*4 bits)
				if(vertical)	flipVDXTCBlockAlpha3(block, height);
				else			flipHDXTCBlockAlpha3(block, width);
			}
			else if(type==5)
			{
				// point to the bit alpha part (16*3 bits)
				if(vertical)	flipVDXTCBlockAlpha5(block+2, height);
				else			flipHDXTCBlockAlpha5(block+2, width);
			}
		}
	}

}


// ***************************************************************************
void	CBitmap::flipDXTC(bool vertical)
{
	// get type
	uint	type;
	if(PixelFormat == DXTC1 || PixelFormat == DXTC1Alpha )
		type=1;
	else if(PixelFormat == DXTC3)
		type=3;
	else if(PixelFormat == DXTC5)
		type=5;
	else
		return;

	// correct width/height?
	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	if(!isPowerOf2(nWidth) || !isPowerOf2(nHeight))
		return;

	// flip all mipmaps
	for(uint mm=0;mm<_MipMapCount;mm++)
	{
		flipDXTCMipMap(vertical, mm, type);
	}
}


// ***************************************************************************
void	CBitmap::flipH()
{
	if (PixelFormat != RGBA)
	{
		// try for DXTC
		flipDXTC(false);

		// then quit (whether it worked or not)
		return;
	}

	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	sint32 i, j;
	NLMISC::CRGBA *pBitmap = (NLMISC::CRGBA*)&_Data[0][0];
	bool needRebuild = false;
	CRGBA temp;

	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();

	for( i = 0; i < nHeight; ++i )
		for( j = 0; j < nWidth/2; ++j )
		{
			temp = pBitmap[i*nWidth+j];
			pBitmap[i*nWidth+j] = pBitmap[i*nWidth+nWidth-j-1];
			pBitmap[i*nWidth+nWidth-j-1] = temp;
		}

	// Rebuilding mipmaps
	if(needRebuild)
	{
		buildMipMaps();
	}
}


// ***************************************************************************
void	CBitmap::flipV()
{
	if (PixelFormat != RGBA)
	{
		// try for DXTC
		flipDXTC(true);

		// then quit (whether it worked or not)
		return;
	}

	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	sint32 i, j;
	NLMISC::CRGBA *pBitmap = (NLMISC::CRGBA*)&_Data[0][0];
	bool needRebuild = false;
	CRGBA temp;

	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();

	for( j = 0; j < nHeight/2; ++j )
		for( i = 0; i < nWidth; ++i )
		{
			temp = pBitmap[j*nWidth+i];
			pBitmap[j*nWidth+i] = pBitmap[(nHeight-j-1)*nWidth+i];
			pBitmap[(nHeight-j-1)*nWidth+i] = temp;
		}

	// Rebuilding mipmaps
	if(needRebuild)
	{
		buildMipMaps();
	}
}


void	CBitmap::rot90CW()
{
	if (PixelFormat != RGBA)
		return;
	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	sint32 i, j;
	NLMISC::CRGBA *pSrcRgba = (NLMISC::CRGBA*)&_Data[0][0];
	bool needRebuild = false;

	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();

	CObjectVector<uint8> pDestui;
	pDestui.resize(nWidth*nHeight*4);
	NLMISC::CRGBA *pDestRgba = (NLMISC::CRGBA*)&pDestui[0];

	for( j = 0; j < nHeight; ++j )
	for( i = 0; i < nWidth;  ++i )
		pDestRgba[j+i*nHeight] = pSrcRgba[i+(nHeight-1-j)*nWidth];

	uint32 nTemp = _Width;
	_Width = _Height;
	_Height = nTemp;

	NLMISC::contReset(_Data[0]); // free memory
	_Data[0] =  pDestui;
	// Rebuilding mipmaps
	if(needRebuild)
	{
		buildMipMaps();
	}
}

void	CBitmap::rot90CCW()
{
	if (PixelFormat != RGBA)
		return;
	sint32 nWidth = getWidth(0);
	sint32 nHeight = getHeight(0);
	sint32 i, j;
	NLMISC::CRGBA *pSrcRgba = (NLMISC::CRGBA*)&_Data[0][0];
	bool needRebuild = false;

	if(_MipMapCount>1)
		needRebuild = true;
	releaseMipMaps();

	CObjectVector<uint8> pDestui;
	pDestui.resize(nWidth*nHeight*4);
	NLMISC::CRGBA *pDestRgba = (NLMISC::CRGBA*)&pDestui[0];

	for( j = 0; j < nHeight; ++j )
	for( i = 0; i < nWidth;  ++i )
		pDestRgba[j+i*nHeight] = pSrcRgba[nWidth-1-i+j*nWidth];

	uint32 nTemp = _Width;
	_Width = _Height;
	_Height = nTemp;

	NLMISC::contReset(_Data[0]); // free memory
	_Data[0] =  pDestui;
	// Rebuilding mipmaps
	if(needRebuild)
	{
		buildMipMaps();
	}
}

//===========================================================================
void CBitmap::blend(CBitmap &Bm0, CBitmap &Bm1, uint16 factor, bool inputBitmapIsMutable /*= false*/)
{
	nlassert(factor <= 256);

	nlassert(Bm0._Width != 0 && Bm0._Height != 0
			 && Bm1._Width != 0 && Bm1._Height != 0);

	nlassert(Bm0._Width  == Bm1._Width);	// the bitmap should have the same size
	nlassert(Bm0._Height == Bm1._Height);

	const CBitmap *nBm0, *nBm1; // pointer to the bitmap that is used for blending, or to a copy is a conversion wa required

	CBitmap cp0, cp1; // these bitmap are copies of Bm1 and Bm0 if a conversion was needed

	if (Bm0.PixelFormat != RGBA)
	{
		if (inputBitmapIsMutable)
		{
			Bm0.convertToRGBA();
			nBm0 = &Bm0;
		}
		else
		{
			cp0 = Bm0;
			cp0.convertToRGBA();
			nBm0 = &cp0;
		}
	}
	else
	{
		nBm0 = &Bm0;
	}


	if (Bm1.PixelFormat != RGBA)
	{
		if (inputBitmapIsMutable)
		{
			Bm1.convertToRGBA();
			nBm1 = &Bm1;
		}
		else
		{
			cp1 = Bm1;
			cp1.convertToRGBA();
			nBm1 = &cp1;
		}
	}
	else
	{
		nBm1 = &Bm1;
	}

	if (this != nBm0 && this != nBm1)
	{
		// if source is the same than the dets, don't resize because this clear the bitmap
		this->resize(Bm0._Width, Bm0._Height, RGBA);
	}

	uint numPix = _Width * _Height; // 4 component per pixels


	const uint8 *src0		= &(nBm0->_Data[0][0]);
	const uint8 *src1		= &(nBm1->_Data[0][0]);
	uint8 *dest				= &(this->_Data[0][0]);


	#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	if (CSystemInfo::hasMMX())
	{
		// On a P4 2GHz, with a 256x256 texture, I got the following results :
		// without mmx : 5.2 ms
		// with mmx    : 1.7 ms
		// I'm sure this can be further optimized..

		uint numPixLeft = numPix & 1; // process 2 pixels at once, so special case for odd number
		numPix = numPix & ~1;
		// do fast blend with mmx
		uint64 blendFactor0;
		uint64 blendFactor1;
		uint16 *bf0 = (uint16 *) &blendFactor0;
		uint16 *bf1 = (uint16 *) &blendFactor1;
		bf0[0] = bf0[1] = bf0[2] = bf0[3] = (1 << 6) * (factor);
		bf1[0] = bf1[1] = bf1[2] = bf1[3] = (1 << 6) * (256 - factor);
		__asm
		{
			mov esi, src0
			mov eax, src1
			mov edi, dest
			mov ebx, -8
			mov ecx, numPix
			shr ecx, 1 // process pixels 2 by 2
			movq mm1, blendFactor0
			movq mm0, blendFactor1

		myLoop:
			pxor mm6, mm6
			lea  ebx, [ebx + 8] // points next location
			pxor mm7, mm7
			movq mm2, [esi + ebx]
			movq mm3, [eax + ebx]
			// do blend
			punpckhbw mm7, mm2  // mm7 contains src0 color 0 in high bytes
			punpckhbw mm6, mm3  // mm6 contains src1 color 0 in high bytes
			psrl	  mm7, 1
			pxor mm4, mm4       // mm4 = 0
			psrl	  mm6, 1
			pmulhw mm7, mm0     // src0 = src0 * blendFactor
			pxor mm5, mm5       // mm5 = 0
			pmulhw mm6, mm1     // src1 = src1 * (1 - blendfactor)
			punpcklbw mm4, mm2  // mm4 contains src0 color 1 in high bytes
			paddusw mm6, mm7    // mm6 = src0[0] blended with src1[0]
			psrl      mm4, 1
			psrlw     mm6, 5
			punpcklbw mm5, mm3  // mm4 contains src1 color 1 in high bytes
			psrl      mm5, 1
			pmulhw    mm4, mm0     // src0 = src0 * blendFactor
			pmulhw    mm5, mm1     // src1 = src1 * (1 - blendfactor)
			paddusw   mm4, mm5    // mm6 = src0[1] blended with src1[1]
			psrlw     mm4, 5
			// pack result
			packuswb  mm4, mm6
			dec		  ecx
			movq      [edi + ebx], mm4  // store result
			jne myLoop
			emms
		}
		if (numPixLeft)
		{
			// case of odd number of pixels
			src0 += 4 * numPix;
			src1 += 4 * numPix;
			dest += 4 * numPix;
			uint blendFact    = (uint) factor;
			uint invblendFact = 256 - blendFact;
			*dest = (uint8) (((blendFact * *src1)		+ (invblendFact * *src0)) >> 8);
			*(dest + 1) = (uint8) (((blendFact * *(src1 + 1)) + (invblendFact * *(src0 + 1))) >> 8);
			*(dest + 2) = (uint8) (((blendFact * *(src1 + 2)) + (invblendFact * *(src0 + 2))) >> 8);
			*(dest + 3)  = (uint8) (((blendFact * *(src1 + 3)) + (invblendFact * *(src0 + 3))) >> 8);
		}
	}
	else
	#endif //#ifdef NL_OS_WINDOWS
	{
		uint8 *endPix			= dest + (numPix << 2);
		// no mmx version
		uint blendFact    = (uint) factor;
		uint invblendFact = 256 - blendFact;
		do
		{
			/// blend 4 component at each pass
			*dest = (uint8) (((blendFact * *src1)		+ (invblendFact * *src0)) >> 8);
			*(dest + 1) = (uint8) (((blendFact * *(src1 + 1)) + (invblendFact * *(src0 + 1))) >> 8);
			*(dest + 2) = (uint8) (((blendFact * *(src1 + 2)) + (invblendFact * *(src0 + 2))) >> 8);
			*(dest + 3)  = (uint8) (((blendFact * *(src1 + 3)) + (invblendFact * *(src0 + 3))) >> 8);

			src0 = src0 + 4;
			src1 = src1 + 4;
			dest = dest + 4;
		}
		while (dest != endPix);
	}
}



//-----------------------------------------------
CRGBA CBitmap::getRGBAPixel(sint x, sint y, uint32 numMipMap /*=0*/) const
{
	uint w = getWidth(numMipMap);
	uint h = getHeight(numMipMap);
	if (w == 0 || (uint) x >= w || (uint) y >= h) return CRGBA::Black; // include negative cases
	const uint8 *pix = &getPixels(numMipMap)[(x + y * w) << 2];
	return CRGBA(pix[0], pix[1], pix[2], pix[3]);
}

//-----------------------------------------------
CRGBA CBitmap::getDXTCColorFromBlock(const uint8 *block, sint x, sint y)
{
	uint16  col0;
	uint16  col1;
	memcpy(&col0, block, sizeof(uint16));
	memcpy(&col1, block + 2, sizeof(uint16));
	uint	colIndex = (block[4 + (y & 3)] >> ((x & 3) << 1)) & 3;
	CRGBA   result, c0, c1;
	if (col0 > col1)
	{
		switch(colIndex)
		{
			case 0:
				uncompress(col0, result);
			break;
			case 1:
				uncompress(col1, result);
			break;
			case 2:
				uncompress(col0, c0);
				uncompress(col1, c1);
				result.blendFromui(c0, c1, 85);
			break;
			case 3:
				uncompress(col0, c0);
				uncompress(col1, c1);
				result.blendFromui(c0, c1, 171);
			break;
		default:
			;
		}
		result.A = 255;
	}
	else
	{
		switch(colIndex)
		{
			case 0:
				uncompress(col0, result);
				result.A = 255;
			break;
			case 1:
				uncompress(col1, result);
				result.A = 255;
			break;
			case 2:
				uncompress(col0, c0);
				uncompress(col1, c1);
				result.blendFromui(c0, c1, 128);
				result.A = 255;
			break;
			case 3:
				result.set(0, 0, 0, 0);
			break;
		}
	}
	return result;
}

//-----------------------------------------------
CRGBA CBitmap::getDXTC1Texel(sint x, sint y, uint32 numMipMap) const
{
	uint w = getWidth(numMipMap);
	uint h = getHeight(numMipMap);
	if (w == 0 || h == 0 || (uint) x >= w || (uint) y >= h) return CRGBA::Black; // include negative cases
	uint numRowBlocks   = std::max((w + 3) >> 2, 1u);
	const uint8 *pix    = &getPixels(numMipMap)[0];
	const uint8 *block  = pix + ((y >> 2) * (numRowBlocks << 3) + ((x >> 2) << 3));
	return getDXTCColorFromBlock(block, x, y);
}


//-----------------------------------------------
CRGBA CBitmap::getDXTC3Texel(sint x, sint y, uint32 numMipMap) const
{
	uint w = getWidth(numMipMap);
	uint h = getHeight(numMipMap);
	if (w == 0 || h == 0 || (uint) x >= w || (uint) y >= h) return CRGBA::Black; // include negative cases
	uint numRowBlocks   = std::max((w + 3) >> 2, 1u);
	const uint8 *pix    = &getPixels(numMipMap)[0];
	const uint8 *block  = pix + ((y >> 2) * (numRowBlocks << 4) + ((x >> 2) << 4));
	CRGBA result = getDXTCColorFromBlock(block + 8, x, y);
	// get alpha part
	uint8 alphaByte = block[((y & 3) << 1) + ((x & 2) >> 1)];
	result.A = (x & 1) ?  (alphaByte & 0xf0) : ((alphaByte & 0x0f) << 4);
	return result;
}

//-----------------------------------------------
CRGBA CBitmap::getDXTC5Texel(sint x, sint y, uint32 numMipMap) const
{
	uint w = getWidth(numMipMap);
	uint h = getHeight(numMipMap);
	if (w == 0 || h == 0 || (uint) x >= w || (uint) y >= h) return CRGBA::Black; // include negative cases
	uint numRowBlocks   = std::max((w + 3) >> 2, 1u);
	const uint8 *pix    = &getPixels(numMipMap)[0];
	const uint8 *block  = pix + ((y >> 2) * (numRowBlocks << 4) + ((x >> 2) << 4));
	CRGBA result = getDXTCColorFromBlock(block + 8, x, y);
	// get alpha part
	uint8 alpha0 = block[0];
	uint8 alpha1 = block[1];

	uint alphaIndex;
	uint tripletIndex = (x & 3) + ((y & 3) << 2);
	if (tripletIndex < 8)
	{
		alphaIndex = (((uint32 &) block[2]) >> (tripletIndex * 3)) & 7;
	}
	else
	{
		alphaIndex = (((uint32 &) block[5]) >> ((tripletIndex - 8) * 3)) & 7; // we can read a dword there because there are color datas following he alpha datas
	}

	if (alpha0 > alpha1)
	{
		switch (alphaIndex)
		{
			case 0: result.A = alpha0; break;
			case 1: result.A = alpha1; break;
			case 2: result.A = (uint8) ((6 * (uint) alpha0 + (uint) alpha1) / 7); break;
			case 3: result.A = (uint8) ((5 * (uint) alpha0 + 2 * (uint) alpha1) / 7); break;
			case 4: result.A = (uint8) ((4 * (uint) alpha0 + 3 * (uint) alpha1) / 7); break;
			case 5: result.A = (uint8) ((3 * (uint) alpha0 + 4 * (uint) alpha1) / 7); break;
			case 6: result.A = (uint8) ((2 * (uint) alpha0 + 5 * (uint) alpha1) / 7); break;
			case 7: result.A = (uint8) (((uint) alpha0 + (uint) 6 * alpha1) / 7); break;
		}
	}
	else
	{
		switch (alphaIndex)
		{
			case 0: result.A = alpha0; break;
			case 1: result.A = alpha1; break;
			case 2: result.A = (uint8) ((4 * (uint) alpha0 + (uint) alpha1) / 5); break;
			case 3: result.A = (uint8) ((3 * (uint) alpha0 + 2 * (uint) alpha1) / 5); break;
			case 4: result.A = (uint8) ((2 * (uint) alpha0 + 3 * (uint) alpha1) / 5); break;
			case 5: result.A = (uint8) (((uint) alpha0 + 4 * (uint) alpha1) / 5); break;
			case 6: result.A = 0;	break;
			case 7: result.A = 255; break;
		}
	}
	return result;
}


//-----------------------------------------------
CRGBA CBitmap::getPixelColor(sint x, sint y, uint32 numMipMap /*=0*/) const
{

	switch (PixelFormat)
	{
		case RGBA:
			return getRGBAPixel(x, y, numMipMap);
		case DXTC1:
		case DXTC1Alpha:
			return getDXTC1Texel(x, y, numMipMap);
		case DXTC3:
			return getDXTC3Texel(x, y, numMipMap);
		case DXTC5:
			return getDXTC5Texel(x, y, numMipMap);
		default:
			nlstop;
		break;
	}
	return CRGBA::Black;
}


//-----------------------------------------------
void CBitmap::swap(CBitmap &other)
{
	std::swap(PixelFormat, other.PixelFormat);
	std::swap(_MipMapCount, other._MipMapCount);
	std::swap(_LoadGrayscaleAsAlpha, other._LoadGrayscaleAsAlpha);
	std::swap(_Width, other._Width);
	std::swap(_Height, other._Height);
	for(uint k = 0; k < MAX_MIPMAP; ++k)
	{
		_Data[k].swap(other._Data[k]);
	}
}

//-----------------------------------------------
void CBitmap::unattachPixels(CObjectVector<uint8> *mipmapDestArray, uint maxMipMapCount /*=MAX_MIPMAP*/)
{
	if (!mipmapDestArray) return;
	uint k;
	for(k = 0; k < std::min((uint) _MipMapCount, maxMipMapCount); ++k)
	{
		mipmapDestArray[k].swap(_Data[k]);
		_Data[k].clear();
	}
	for(; k < _MipMapCount; ++k)
	{
		_Data[k].clear();
	}
	#ifdef NL_DEBUG
		// check that remaining mipmaps are empty
		for(; k < _MipMapCount; ++k)
		{
			nlassert(_Data[k].empty());
		}
	#endif
	_MipMapCount = 1;
	_Width = 0;
	_Height = 0;
	PixelFormat = RGBA;
	_LoadGrayscaleAsAlpha = true;
}




void CBitmap::getData(uint8*& extractData)
{

	uint32 size=0;
	if(PixelFormat==RGBA)
		size=_Width*_Height*4;
	else if(PixelFormat==Alpha||PixelFormat==Luminance)
		size=_Width*_Height;
	else
	{
		nlstop;
	}

	for(uint32 pix=0;pix<size;pix++)
		extractData[pix]=_Data[0][pix];

}

void CBitmap::getDibData(uint8*& extractData)
{

	uint32 lineSize=0,size;
	uint8** buf;
	buf=new uint8*[_Height];
	if(PixelFormat==RGBA)
	{
		lineSize=_Width*4;


	}
	else if(PixelFormat==Alpha||PixelFormat==Luminance)
	{
		lineSize=_Width;
	}
	else
	{
		nlstop;
	}

	for(sint32 i=_Height-1;i>=0;i--)
	{
		buf[_Height-1-i]=&_Data[0][i*lineSize];
	}

	size=lineSize*_Height;

	for(uint32 line=0;line<_Height;line++)
	{
		for(uint32 pix=0;pix<lineSize;pix++)
			extractData[line*lineSize+pix]=_Data[0][size-(line+1)*lineSize+pix];
	}
	delete []buf;

}

} // NLMISC

