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

#ifndef NL_VEGETABLE_SORT_BLOCK_H
#define NL_VEGETABLE_SORT_BLOCK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/object_vector.h"
#include "nel/3d/tess_list.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/vegetable_instance_group.h"


namespace NL3D
{


using NLMISC::CVector;

class	CSortVSB;
class	CVegetableBlendLayerModel;

// ***************************************************************************
/**
 *	A block of vegetable instance groups.
 *	CVegetableSortBlock are sorted in Z order.
 *	NB: for speed and convenience, only the RdrPass NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT is sorted.
 *	A block have a number of quadrants (8). Each quadrant has an array of triangles to render.
 *	Internal to VegetableManager. Just an Handle for public.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableSortBlock : public CTessNodeList
{
public:

	/// Constructor
	CVegetableSortBlock();

	// get the center of the sort Block.
	const CVector	&getCenter() const {return _Center;}

	/** After adding some instance to any instance group of a sorted block, you must recall this method
	 *	NB: CVegetableManager::addInstance() and CVegetableManager::deleteIg() flag this SB as Dirty, when
	 *	needed only. if !Dirty, updateSortBlock() is a no-op.
	 *	/see CVegetableManager::addInstance()  CVegetableManager::deleteIg()
	 *	Warning! Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	 */
	void			updateSortBlock(CVegetableManager &vegetManager);


// ***************
private:
	friend class	CVegetableManager;
	friend class	CSortVSB;
	friend class	CVegetableClipBlock;
	friend class	CVegetableBlendLayerModel;


	// Who owns us.
	CVegetableClipBlock		*_Owner;

	// This flag is set to true by CVegetableManager in addInstance() or deleteIg() if the ig impact on me.
	// If false, updateSortBlock() is a no-op.
	bool					_Dirty;

	// This flag is cahnged by CVegetableManager in addInstance(). false by default
	bool					_UnderWater;

	/// \name Fast sorting.
	// @{
	/// center of the sort block.
	CVector			_Center;
	/// approximate Radius of the sort block.
	float			_Radius;

	/// Positive value used for sort. (square of distance to viewer + threshold). temp computed at each render()
	float			_SortKey;
	///	current quadrant used. computed at each render.
	uint			_QuadrantId;

	/// Quadrants.
	/// the big array of indices, for the NL3D_VEGETABLE_NUM_QUADRANT quadrants.
	CIndexBuffer	_SortedTriangleArray;
	/// start ptr.
	uint32					_SortedTriangleIndices[NL3D_VEGETABLE_NUM_QUADRANT];
	/// number of triangles.
	uint					_NTriangles;
	/// number of indeices= numTriangles*3.
	uint					_NIndices;

	// @}

	// List of Igs.
	CTessList<CVegetableInstanceGroup>		_InstanceGroupList;

	// ZSort rdrPass Igs must all be in same hardMode. This is the state.
	// NB: this restriction does not apply to other rdrPass
	bool					ZSortHardMode;

};


} // NL3D


#endif // NL_VEGETABLE_SORT_BLOCK_H

/* End of vegetable_sort_block.h */
