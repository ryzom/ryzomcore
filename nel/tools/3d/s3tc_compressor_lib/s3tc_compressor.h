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

#ifndef NL_S3TC_COMPRESSOR_H
#define NL_S3TC_COMPRESSOR_H

#include <nel/misc/types_nl.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/stream.h>


// ***************************************************************************
#define	DXT1	1
#define	DXT1A	11
#define	DXT3	3
#define	DXT5	5

#ifndef DDSD_CAPS

// flags to be used for DDS_HEADER
#define DDSD_CAPS			0x00000001l	// default
#define DDSD_WIDTH			0x00000004l
#define DDSD_HEIGHT			0x00000002l
#define DDSD_PIXELFORMAT	0x00001000l
#define DDSD_MIPMAPCOUNT	0x00020000l

#define DDPF_FOURCC			0x00000004l

#define DDSCAPS_TEXTURE		0x00001000l
#define DDSCAPS_MIPMAP		0x00400000l

#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
		((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
		((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))

#endif

// ***************************************************************************
/**
 * From a bitmap, build the high quality DDS, and output in a stream
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CS3TCCompressor
{
public:
	/// Constructor
	CS3TCCompressor();

	void		compress(const NLMISC::CBitmap &bmpSrc, bool optMipMap, uint algo, NLMISC::IStream &output);

	typedef struct
	{
	    uint32 dwSize;
	    uint32 dwFlags;
	    uint32 dwFourCC;
	    uint32 dwRGBBitCount;
	    uint32 dwRBitMask;
	    uint32 dwGBitMask;
	    uint32 dwBBitMask;
	    uint32 dwABitMask;
	} DDS_PIXELFORMAT;

	typedef struct
	{
	    uint32 dwSize;
	    uint32 dwFlags;
	    uint32 dwHeight;
	    uint32 dwWidth;
	    uint32 dwLinearSize;
	    uint32 dwDepth;
	    uint32 dwMipMapCount;
	    uint32 dwReserved1[11];
	    DDS_PIXELFORMAT ddpf;
	    uint32 dwCaps;
	    uint32 dwCaps2;
	    uint32 dwCaps3;
	    uint32 dwCaps4;
	    uint32 dwReserved2;
	} DDS_HEADER;

};


#endif // NL_S3TC_COMPRESSOR_H

/* End of s3tc_compressor.h */
