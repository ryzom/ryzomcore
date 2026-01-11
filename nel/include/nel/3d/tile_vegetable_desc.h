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

#ifndef NL_TILE_VEGETABLE_DESC_H
#define NL_TILE_VEGETABLE_DESC_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/3d/landscape_def.h"
#include "nel/3d/vegetable.h"
#include "nel/3d/vegetable_def.h"
#include <vector>

namespace NL3D
{


class	CVegetableManager;


// ***************************************************************************
/**
 * A descriptor of vegetables for a tile.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTileVegetableDesc
{
public:

	/// Constructor
	CTileVegetableDesc();

	/// clear lists.
	void		clear();

	/// Build the CTileVegetableDesc.
	void		build(const std::vector<CVegetable> &vegetables);

	/// register all Vegetables in this to the manager.
	void		registerToManager(CVegetableManager *vegetableManager);

	/// serial.
	void		serial(NLMISC::IStream &f);

	/// get the vegetable list for a specific distanceType
	const	std::vector<CVegetable>		&getVegetableList(uint distType) const;

	/// get the vegetable seed for a specific distanceType
	uint		getVegetableSeed(uint distType) const;

	/// return true if no vegetable at all for any DistType
	bool		empty() const {return _Empty;}

private:

	/** List of vegetable to instanciate for a specific tile
	 *	Grouped by distance Type.
	 */
	std::vector<CVegetable>		_VegetableList[NL3D_VEGETABLE_BLOCK_NUMDIST];

	uint32						_VegetableSeed[NL3D_VEGETABLE_BLOCK_NUMDIST];

	// to know if no vegetable at all
	bool						_Empty;

	void	compileRunTime();
};


} // NL3D


#endif // NL_TILE_VEGETABLE_DESC_H

/* End of tile_vegetable_desc.h */
