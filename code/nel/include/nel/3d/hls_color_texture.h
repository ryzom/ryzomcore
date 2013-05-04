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

#ifndef NL_HLS_COLOR_TEXTURE_H
#define NL_HLS_COLOR_TEXTURE_H

#include "nel/misc/types_nl.h"
#include <vector>
#include <string>


namespace NLMISC
{
	class	CBitmap;
	class	IStream;
	class	CRGBA;
}


namespace NL3D
{


using NLMISC::CRGBA;


// ***************************************************************************
class CHLSColorDelta
{
public:
	uint8	DHue;
	// -128..+127. to be extended to -255..+255
	sint8	DLum;
	// -128..+127. to be extended to -255..+255
	sint8	DSat;

	void	serial(NLMISC::IStream &f);
};


// ***************************************************************************
/**
 * A colorisable texture
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CHLSColorTexture
{
public:
	CHLSColorTexture();

	/// reset the build
	void			reset();

	/** setup the un-colored bitmap for the texture.
	 *	\param bmp a bitmap which must be a DXTC5 with all mipmaps.
	 */
	void			setBitmap(const NLMISC::CBitmap &bmp);

	/** add a mask to the texture.
	 *	R is taken as the mask value. must be same size as in setBitmap()
	 *	\param threshold used to know if a pixel mask value is or not an intermediate (ie not 0 or 255)
	 */
	void			addMask(const NLMISC::CBitmap &bmp, uint threshold= 15);

	/// get num of masks
	uint			getNumMasks() const {return (uint)_Masks.size();}

	/** build a texture with a HLS Color Delta
	 *	\param colDelta array of delta to apply to the bitmap (must be of numMasks entries)
	 *	\param out a colorised bitmap with DXTC5/mipMaps generated
	 */
	void			buildColorVersion(const CHLSColorDelta *colDeltaList, NLMISC::CBitmap &out);


	void			serial(NLMISC::IStream &f);


// **************
private:
	struct		CMask
	{
		uint32					FullBlockIndex;
		uint32					MixtBlockIndex;
		/// Raw Data. First come the Mixt block (16*uint8). Then come the FullBlockIndex bits, then the MixtBlockIndex bits
		std::vector<uint8>		Data;

		// set the Data bit to 1
		void			setBit(uint bitId);

		void			serial(NLMISC::IStream &f);
	};


	struct	CVectorInt
	{
		sint	x,y,z;
	};


private:
	uint32					_Width, _Height, _NumMipMap;
	uint32					_BlockToCompressIndex;
	// The DXTC5 data. Also contains bits from _BlockToCompressIndex to end (rounded to uint32)
	std::vector<uint8>		_Texture;
	// Masks
	std::vector<CMask>		_Masks;


private:
	/// uncompress DXTC5 RGB only block, into a RGBA raw array. Alpha is setup with undefined values
	static void			uncompressBlockRGB(const uint8* srcDXTC, CRGBA *dstRGBA);

	/// used by compressBlockRGB()
	static void			computeMinMax(sint *diffBlock, CVectorInt &v, sint mean[3], sint rgb0[3], sint rgb1[3]);

	/// apply colDelta to the block. Alpha part is not modified. MMX with no EMMS called here !!!
	static void			colorizeDXTCBlockRGB(const uint8 *srcPtr, uint8 *dstPtr, uint8 dHue, uint dLum, uint dSat);

public:
	/// compress DXTC5 RGB only block, from a RGBA raw array. dstDXTC Alpha part is not modified. srcRGBA->A are setup to 0!!
	static void			compressBlockRGB(CRGBA *srcRGBA, uint8* dstDXTC);

};



} // NL3D


#endif // NL_HLS_COLOR_TEXTURE_H

/* End of hls_color_texture.h */
