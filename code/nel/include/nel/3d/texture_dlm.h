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

#ifndef NL_TEXTURE_DLM_H
#define NL_TEXTURE_DLM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/3d/texture.h"
#include "nel/3d/landscape_def.h"


#ifdef	NL_DLM_TILE_RES
// Size of a Block in the texture. Must be 18.
#define NL_DLM_BLOCK_SIZE	18
#else
// Size of a Block in the texture. Must be 10.
#define NL_DLM_BLOCK_SIZE	10
#endif

// Number of lightmap type. 4*4
#define NL_DLM_LIGHTMAP_TYPE_SIZE	16


namespace NL3D
{


using NLMISC::CRGBA;

// ***************************************************************************
/**
 * This texture is used by landscape to perform Dynamic LightMap (DLM).
 *	Actually a CTextureDLM handle many block of lightmap in one single big texture.
 *	If NL_DLM_TILE_RES, then
 *	Block can be of size of 2,3,5 or 9 * 2,3,5 or 9  (eg 2x9, or 5x5 texture).
 *	else
 *	Block can be of size of 3,5,9 or 17 * 3,5,9 or 17  (eg 3x17, or 9x9 texture).
 *
 *	Implementation note (NL_DLM_TILE_RES not defined):
 *	To make this possible easily, blocks of 10x10 are created and placed in the texture.
 *	Hence a 9x9 texture lies in a single block, 3 textures of 3x9 lies in a block etc...
 *	In worst case, lost space is 19%  (1 - 9*9 / 10*10).
 *
 *	If NL_DLM_TILE_RES is defined, then, same reasoning, with blocks of 18x18. In worst case,
 *	space lost is 70%: (1 - 15*15 / 18*18). But others cases are pretty good (90% to 100%)
 *
 * 	NB: TextureDLM ensure that point (MaxX,MaxY) of this texture is black. Useful for patch who
 *	want default black color
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CTextureDLM : public ITexture
{
public:

	/// Constructor
	CTextureDLM(uint width, uint height);

	/// Since texture is always in memory...
	void			doGenerate(bool /* async */ = false)
	{
		// Do nothing. texture still in memory... :o)
	}

	/// TextureDLM are system. Do not need to serialize them...
	// default ctor is required for compilation with NLMISC_DECLARE_CLASS, but never called...
	CTextureDLM() {nlstop;}
	virtual void	serial(NLMISC::IStream &/* f */) throw(NLMISC::EStream) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureDLM);



	/// \name Lightmap mgt.
	// @{

	/// return true if can create a texture of this size.
	bool			canCreateLightMap(uint w, uint h);

	/** create a space for a lightmap. NB: texture space is not filled with black.
	 *	return false if cannot, else return true, and return in x/y the position in the texture.
	 */
	bool			createLightMap(uint w, uint h, uint &x, uint &y);

	/** refill the texture with raw data. NB: no check is made on x,y,w,h lightmap validity.
	 *	CRGBA are transformed to texture format (16 bits or better)
	 *	The texture is invalidate (on this part only...)
	 *	\param map is the raw array of RGBA colors to fills. must be of w*h size
	 */
	void			copyRect(uint x, uint y, uint w, uint h, CRGBA  *textMap);

	/** same as copyRect(), but fill a RGBA(value, value, value, value)
	 */
	void			fillRect(uint x, uint y, uint w, uint h, uint8 value);

	/** same as copyRect(), but modulate textMap with an array of 565 color, before copying.
	 */
	void			modulateAndfillRect565(uint x, uint y, uint w, uint h, CRGBA  *textMap, uint16 *modColor);

	/** same as copyRect(), but modulate textMap with an array of CRGBA color, before copying.
	 */
	void			modulateAndfillRect8888(uint x, uint y, uint w, uint h, CRGBA  *textMap, CRGBA *modColor);

	/** same as copyRect(), but modulate textMap with a cte color, before copying.
	 */
	void			modulateConstantAndfillRect(uint x, uint y, uint w, uint h, CRGBA  *textMap, CRGBA modColor);

	/// Set a lightmap as free for use. It is an error to free a not allocated lightmap. (nlassert!!)
	void			releaseLightMap(uint x, uint y);

	// @}



// *****************************
private:

	/// A block descriptor.
	struct	CBlock
	{
		// Size of a lightmap in the block. eg: 9x9. Not relevant if FreeSpace==0 (because block completely free).
		uint8		Width, Height;

		// Position of block in texture, in pixels.
		uint16		PosX, PosY;

		/* BitField of Space free (1 if not free).
			NL_DLM_TILE_RES defined: since 3x3 is the minimum size, there is at max 6*6=36 lightmaps in a blocks.
			Hence a uint64.
			NL_DLM_TILE_RES defined: since 2x2 is the minimum size, there is at max 5*5=25 lightmaps in a blocks.
			(NB: a uint32 would be sufficient, but never mind)
		*/
		uint64		FreeSpace;

		/// Free List.
		CBlock		*FreePrec, *FreeNext;

		CBlock()
		{
			FreeSpace= 0;
			// No List
			FreePrec= FreeNext= NULL;
		}
	};

private:

	/// Number of block per line
	uint				_WBlock;

	/** The list of blocks. There is TextureWidth/NL_DLM_BLOCK_SIZE * TextureHeight/NL_DLM_BLOCK_SIZE blocks,
	 *	ranged from left to right then top to bottom.
	 */
	std::vector<CBlock>	_Blocks;


	/// The list of available Blocks, ie Blocks with FreeSpace==0
	std::vector<uint>	_EmptyBlocks;


	/// For each type of lightmaps (2x2, 2x3 etc...), list of blocks which are not full
	CBlock				*_FreeBlocks[NL_DLM_LIGHTMAP_TYPE_SIZE];

	/// get the lightmap type id according to lightmap size.
	uint				getTypeForSize(uint width, uint height);

	/// FreeBlock list mgt.
	void				linkFreeBlock(uint lMapType, CBlock *block);
	void				unlinkFreeBlock(uint lMapType, CBlock *block);

};


} // NL3D


#endif // NL_TEXTURE_DLM_H

/* End of texture_dlm.h */
