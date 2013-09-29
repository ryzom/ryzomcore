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

#include "s3tc_compressor.h"
#include <squish.h>
#include <nel/3d/hls_color_texture.h>

using namespace std;


// ***************************************************************************
static void		compressMipMap(uint8 *pixSrc, sint width, sint height, vector<uint8>	&compdata, CS3TCCompressor::DDS_HEADER &dest, sint algo)
{
	// Filling DDSURFACEDESC structure for output
	//===========================================
	memset(&dest, 0, sizeof(dest));
	dest.dwSize = sizeof(dest);
	dest.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
	dest.dwHeight = height;
	dest.dwWidth = width;
	dest.ddpf.dwSize = sizeof(CS3TCCompressor::DDS_PIXELFORMAT);
	dest.ddpf.dwFlags = DDPF_FOURCC;
	dest.dwCaps = DDSCAPS_TEXTURE;

	// Setting flags
	int flags = squish::kColourIterativeClusterFit; // for best quality
	switch(algo)
	{
		case DXT1:
		case DXT1A:
			flags |= squish::kDxt1;
			dest.ddpf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '1');
			break;
		case DXT3:
			flags |= squish::kDxt3;
			dest.ddpf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '3');
			break;
		case DXT5:
			flags |= squish::kDxt5;
			dest.ddpf.dwFourCC = MAKEFOURCC('D', 'X', 'T', '5');
			break;
	}

	// Encoding
	//===========
	// resize dest.
	dest.dwLinearSize = squish::GetStorageRequirements(width, height, flags);
	compdata.resize(dest.dwLinearSize);
	// Go!
#ifdef SQUISH_COMPRESS_HAS_METRIC
	float weight[3] = {0.3086f, 0.6094f, 0.0820f};
	squish::CompressImage(pixSrc, width, height, &(*compdata.begin()), flags, weight);
#else
	squish::CompressImage(pixSrc, width, height, &(*compdata.begin()), flags);
#endif

#if 0
	// This code was for the previous compression library, if I remember correctly.
	// It should no longer be relevant.
	/* S3TC is a very good compressor, but make BIG mistakes in some case with  DXTC5 and DXTC3
	*/
	if( algo==DXT5 || algo==DXT3 )
	{
		sint	wBlock= max(1, width/4);
		sint	hBlock= max(1, height/4);
		uint	numTotalBlock= wBlock*hBlock;
		uint	numFixedBlock= 0;
		
		NLMISC::CRGBA	*rgbaSrc= (NLMISC::CRGBA*)pixSrc;
		for(sint y=0;y<hBlock;y++)
		{
			for(sint x=0;x<wBlock;x++)
			{
				// get comp dest
				uint8	*pixDst= &(*(compdata.begin() + (y*wBlock + x) * 16));
				uint16	rgb0= *(uint16*)(pixDst+8);
				uint16	rgb1= *(uint16*)(pixDst+10);
				/* If S3TC decided to use "50% decode table" (case rgb0<=rgb1), this is an error
					compress this block with our own compressor
				*/
				if(rgb0<=rgb1)
				{
					numFixedBlock++;

					// copy color block
					NLMISC::CRGBA	col[16];
					sint	x0, y0;
					NLMISC::CRGBA	precColor= NLMISC::CRGBA::Black;
					for(y0=0;y0<4;y0++)
					{
						for(x0=0;x0<4;x0++)
						{
							// manage case where height or width are <4
							if(y*4+y0<height && x*4+x0<width)
							{
								precColor= rgbaSrc[(y*4+y0)*width + x*4+x0];
								col[y0*4+x0]= precColor;
							}
							else
							{
								// copy preceding color, to allow correct compression 
								col[y0*4+x0]= precColor;
							}
						}
					}
					
					// compress
					NL3D::CHLSColorTexture::compressBlockRGB(col, pixDst);
					// get correct image under photoshop (swap color if our compressor invert them)
					rgb0= *(uint16*)(pixDst+8);
					rgb1= *(uint16*)(pixDst+10);
					if(rgb0<=rgb1)
					{
						*(uint16*)(pixDst+8)= rgb1;
						*(uint16*)(pixDst+10)= rgb0;
						uint32	&bits= *(uint32*)(pixDst+12);
						for(uint i=0;i<16; i++)
						{
							static uint8	invertTable[]= {1,0,3,2};
							uint	pixVal= (bits>>(i*2))&3;
							pixVal= invertTable[pixVal];
							bits&= ~(3<<(i*2));
							bits|= (pixVal<<(i*2));
						}
					}

					// Test: 05 to 1323
					/*
					uint32	&bits= *(uint32*)(pixDst+12);
					for(uint i=0;i<16; i++)
					{
						uint	pixVal= (bits>>(i*2))&3;
						if(pixVal==2)
						{
							uint r= (rand()&0xFF)>>7;
							pixVal= r+2;
							bits&= ~(3<<(i*2));
							bits|= (pixVal<<(i*2));
						}
					}*/
				}
			}
		}

		if(numFixedBlock && numTotalBlock)
		{
			nlinfo("Fix %d blocks on %d total ", numFixedBlock, numTotalBlock);
		}
	}
#endif
	
}



// ***************************************************************************
CS3TCCompressor::CS3TCCompressor()
{
}


// ***************************************************************************
void		CS3TCCompressor::compress(const NLMISC::CBitmap &bmpSrc, bool optMipMap, uint algo, NLMISC::IStream &output)
{
	vector<uint8>		CompressedMipMaps;
	DDS_HEADER			dest;
	NLMISC::CBitmap		picSrc= bmpSrc;


	// For all mipmaps, compress.
	if(optMipMap)
	{
		// Build the mipmaps.
		picSrc.buildMipMaps();
	}
	for(sint mp= 0;mp<(sint)picSrc.getMipMapCount();mp++)
	{
		uint8	*pixDest;
		uint8	*pixSrc= picSrc.getPixels(mp).getPtr();
		sint	w= picSrc.getWidth(mp);
		sint	h= picSrc.getHeight(mp);
		vector<uint8>	compdata;
		DDS_HEADER	temp;
		compressMipMap(pixSrc, w, h, compdata, temp, algo);
		// Copy the result of the base dds in the dest.
		if(mp==0)
			dest= temp;

		// Append this data to the global data.
		uint	delta= (uint)CompressedMipMaps.size();
		CompressedMipMaps.resize(CompressedMipMaps.size()+compdata.size());
		pixDest= &(*CompressedMipMaps.begin())+ delta;
		memcpy( pixDest, &(*compdata.begin()), compdata.size());
	}


	// Setting Nb MipMap.
	dest.dwFlags |= DDSD_MIPMAPCOUNT;
	dest.dwCaps |= DDSCAPS_MIPMAP;
	dest.dwMipMapCount = picSrc.getMipMapCount();


	// Saving DDS file
	//=================
	output.serialBuffer((uint8*)std::string("DDS ").c_str(),4);
	output.serialBuffer((uint8*) &dest, sizeof(dest));
	output.serialBuffer(&(*CompressedMipMaps.begin()), (uint)CompressedMipMaps.size());
}
