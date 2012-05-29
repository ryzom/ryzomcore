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

#ifndef NL_VEGETABLE_CLIP_BLOCK_H
#define NL_VEGETABLE_CLIP_BLOCK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/bsphere.h"
#include "nel/3d/tess_list.h"
#include "nel/3d/vegetable_sort_block.h"


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;


// ***************************************************************************
/**
 *	A block of vegetable instance groups (via sortBlocks) which are clipped in frustum together
 *	Internal to VegetableManager. Just an Handle for public.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableClipBlock : public CTessNodeList
{
public:

	/// Constructor
	CVegetableClipBlock();


// ***************
private:
	friend class	CVegetableManager;


	/// \name Fast clipping.
	// @{
	bool				_Empty;
	NLMISC::CAABBox		_BBox;
	NLMISC::CBSphere	_BSphere;

	// extend sphere
	void			extendSphere(const CVector &vec);
	// extend bbox. Must not be empty. NB: do not modify the sphere
	void			extendBBoxOnly(const CVector &vec);
	// compute the sphere according to the bbox.
	void			updateSphere();
	// return false if empty or out of frustum
	bool			clip(const std::vector<CPlane>	&pyramid);

	// @}


private:

	// List of SortBlocks.
	CTessList<CVegetableSortBlock>		_SortBlockList;

	// The number of instanceGroups created in this clipBlock.
	uint					_NumIgs;

	// RenderList
	CVegetableClipBlock		*_RenderNext;
};


} // NL3D


#endif // NL_VEGETABLE_CLIP_BLOCK_H

/* End of vegetable_clip_block.h */
