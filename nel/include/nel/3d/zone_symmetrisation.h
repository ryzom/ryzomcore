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

#ifndef NL_ZONE_SYMMETRISATION_H
#define NL_ZONE_SYMMETRISATION_H

#include "nel/misc/types_nl.h"
#include "nel/3d/patchuv_locator.h"
#include "nel/3d/patch.h"


namespace NL3D
{

struct CPatchInfo;
class CTileBank;

/**
 * Environnement used to symmetrise zones
 *
 * This class build symmetry specific information needed to know how
 * transform tiles when a zone is symmetrise.
 *
 * There is two states for a tile : Regular or Goofy.
 * If the tile is regular, it doesn't need specific transformation when the zone is symmetrise.
 * If the tile is goofy, the tile must be rotate by 180deg more to be correct.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CZoneSymmetrisation
{
public:

	/// Symmetrie state
	enum TState
	{
		Nothing		=0,
		Regular		=1,
		Goofy		=2,
	};

	/// Error structure
	class CError
	{
	public:
		std::vector<std::string>	Errors;
	};

	/// Constructor
	CZoneSymmetrisation ();

	/**
	  * Get tile symmetry state
	  * Can return Regular, Goofy or Nothing.
	  */
	TState getTileState (uint patch, uint tile, uint layer) const;

	/**
	  * Get tile symmetry state
	  * Can return Regular, Goofy or Nothing.
	  */
	TState getTileBorderState (uint patch, uint tile) const;

	/**
	  * Get tile symmetry state for oriented tiles
	  * Can return Regular, Goofy or Nothing.
	  */
	TState getOrientedTileBorderState (uint patch, uint tile) const;

	/**
	  * Set oriented tile corner state
	  */
	bool getOrientedTileCorner (uint patch, uint tile);

	/**
	  * Build symmetry information
	  *
	  * \param zone is the zone to build symmetry information
	  * \param snapCell is the unit size of the zones
	  * \param weldThreshold is the threshold used to check vertex over snaped positions
	  * \param errorDesc is a structure used to return errors
	  * \param toOriginalSpace is the matrix used to transform back a vertex in its original position before symmetry / rotation
	  * \return false if the patch topology is invalid for this operations
	  */
	bool build (const std::vector<CPatchInfo> &patchInfo, float snapCell, float weldThreshold, const CTileBank &bank, CError &errorDesc, const NLMISC::CMatrix &toOriginalSpace);

private:

	/**
	  * Propagate tile state information for a given tile.
	  *
	  * \param i is the patch number in the zone that contain the tile to propagate
	  * \param u is the tile u coordinate in the patch
	  * \param v is the tile v coordinate in the patch
	  * \param patchInfo is a vector of patch information
	  * \param bank is the tile bank used by the landscape
	  * \param forceRegular must be true to force the first propagated tile to Regular if Nothing
	  * \return false if the patch topology is invalid for this operations true if succesful
	  */
	bool propagateTileState (uint i, uint s, uint t, const std::vector<CPatchInfo> &patchInfo, const CTileBank &bank, bool forceRegular);

	/**
	  * Set orientedtile symmetry state
	  */
	void setTileState (uint patch, uint tile, uint layer, TState state);

	/**
	  * Set border tile symmetry state
	  */
	void setTileBorderState (uint patch, uint tile, TState state);

	/**
	  * Set border oriented tile symmetry state
	  */
	void setOrientedTileBorderState (uint patch, uint tile, TState state);

	/**
	  * Set oriented tile corner state
	  */
	void setOrientedTileCorner (uint patch, uint tile, bool corner);

	/**
	  * Set tile state of a patch
	  */
	bool setTileState (const NL3D::CPatchInfo &patch, uint patchId, float snapCell, float weldThreshold, TState &state, const NLMISC::CMatrix &toOriginalSpace, const CTileBank &bank);
	bool setOrientedTileState (const NL3D::CPatchInfo &patch, uint patchId, float snapCell, float weldThreshold, TState &state, const NLMISC::CMatrix &toOriginalSpace, const CTileBank &bank);

	// Snap a position on the grid
	static bool snapOnGrid (float& value, float resolution, float snap);

private:

	// The patch state array
	std::vector<std::vector<uint16> >	_TilesLayerStates;
};


} // NL3D


#endif // NL_ZONE_SYMMETRISATION_H

/* End of zone_symmetrisation.h */
