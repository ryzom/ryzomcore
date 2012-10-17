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

#ifndef NL_TILE_FAR_BANK_H
#define NL_TILE_FAR_BANK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/stream.h"
#include "nel/misc/common.h"

#include <vector>

// Pixel count in far tiles
#define NL_NUM_PIXELS_ON_FAR_TILE_EDGE_SHIFT 2											// a 128x128 far tile is a 4x4 bitmap in far 0
#define NL_NUM_PIXELS_ON_FAR_TILE_EDGE (1<<NL_NUM_PIXELS_ON_FAR_TILE_EDGE_SHIFT)	// a 128x128 far tile is a 4x4 bitmap in far 0

namespace NLMISC
{
	class IStream;
}

namespace NL3D
{


/**
 * A bank for the far textures
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileFarBank
{
public:
	// Order of the tile
	enum TFarOrder { order0=0, order1=1, order2=2, orderCount };
	enum TFarType { diffuse=0, additive=1, alpha=2, typeCount=2 };

	/// Constructor
	CTileFarBank();

	// Class for a tile far descriptor
	class CTileFar
	{
		// Friend of CTileFarBank
		friend class CTileFarBank;
	public:
		/// Default constructor
		CTileFar ()
		{
		}

		/// Return the pointer on the pixels data. Call this method only if isFill () returns true.
		const NLMISC::CRGBA*		getPixels (TFarType type, TFarOrder order) const
		{
			return &_Pixels[type][order][0];
		}

		/// Return true if pixel value are presents, else return false.
		bool				isFill (TFarType type) const
		{
			return _Pixels[type][0].begin()!=_Pixels[type][0].end();
		}

		/// Return the pixel array size. Should be 0 for empty, 64 for a 128x128 tile and 256 for a 256x256 tile.
		uint				getSize (TFarType type, TFarOrder order) const
		{
			return (uint)_Pixels[type][order].size();
		}

		/// Set the pixel array of a far Tile
		void				setPixels (TFarType type, TFarOrder order, NLMISC::CRGBA* pixels, uint size);

		/// Erase a pixel array type
		void				erasePixels (TFarType type)
		{
			NLMISC::contReset (_Pixels[type][order0]);
			NLMISC::contReset (_Pixels[type][order1]);
			NLMISC::contReset (_Pixels[type][order2]);
		}

		/// Serial this tile
		void				serial (class NLMISC::IStream &f) throw(NLMISC::EStream);

	private:
		/// RGBA Pixels vector
		std::vector<NLMISC::CRGBA>	_Pixels[typeCount][orderCount];

		/// The version of this class
		static const sint	_Version;
	};

	/// Get number of tile in this bank
	sint					getNumTile () const
	{
		return (sint)_TileVector.size();
	}

	/// Resize the tile bank
	void					setNumTile (sint numTile)
	{
		_TileVector.resize(numTile);
	}

	/// Get a read only far tile pointer. Return NULL if the tile doesn't exist.
	const CTileFar*			getTile (sint tile) const
	{
		if (tile>=(sint)_TileVector.size())
			return NULL;

		return &_TileVector[tile];
	}

	/// Get a far tile pointer. Return NULL if the tile doesn't exist.
	CTileFar*				getTile (sint tile)
	{
		if (tile>=(sint)_TileVector.size())
			return NULL;

		return &_TileVector[tile];
	}

	/// Get the tile size
	static uint				getFarTileEdgeSize (TFarOrder order)
	{
		return 4>>(uint)order;
	}

	/// Serial this bank
	void    serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/// The far tile vector
	std::vector<CTileFar>	_TileVector;

	/// The version of this class
	static const sint	_Version;
};


} // NL3D


#endif // NL_TILE_FAR_BANK_H

/* End of tile_far_bank.h */
