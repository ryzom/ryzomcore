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

#ifndef NL_VEGETABLE_INSTANCE_GROUP_H
#define NL_VEGETABLE_INSTANCE_GROUP_H

#include "nel/misc/types_nl.h"
#include "nel/misc/object_vector.h"
#include "nel/misc/matrix.h"
#include "nel/misc/rgba.h"
#include "nel/3d/tess_list.h"
#include "nel/3d/vegetable_instance_group.h"
#include "nel/3d/vegetable_def.h"
#include "nel/3d/vegetable_light_ex.h"
#include "nel/3d/vegetable_uv8.h"
#include "nel/3d/index_buffer.h"
#include <vector>


namespace NL3D
{


class	CVegetableManager;
class	CVegetableClipBlock;
class	CVegetableSortBlock;
class	CVegetableVBAllocator;
class	CVegetableShape;


// ***************************************************************************
/**
 *	A block of vegetable instances. Instaces are added to an IG, and deleted when the ig is deleted.
 *	Internal to VegetableManager. Just an Handle for public.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableInstanceGroup : public CTessNodeList
{
public:

	/// Constructor
	CVegetableInstanceGroup();
	~CVegetableInstanceGroup();

	/// tells if the instanceGroup has no faces at all.
	bool			isEmpty() const;

	/** The Vegetable Light Information. Contains list of PointLights which influence the vegetIg.
	 *	NB: PointLights ptrs are kept so the ig must be deleted when the lights are deleted.
	 */
	CVegetableLightEx		VegetableLightEx;

// ***************
private:
	friend class	CVegetableManager;
	friend class	CVegetableClipBlock;
	friend class	CVegetableSortBlock;


	// Who owns us.
	CVegetableClipBlock		*_ClipOwner;
	CVegetableSortBlock		*_SortOwner;


	/** a reference to an instance which is lighted (precomputed or not).
	 *	Useful for Lighting updates.
	 */
	struct	CVegetableLightedInstance
	{
		// The shape use to create the instance
		CVegetableShape		*Shape;
		// the matrix to multiply normal (useful for precomputeLighting only)
		NLMISC::CMatrix		NormalMat;
		// The color (not modulated by global ambients/diffuses).
		NLMISC::CRGBA		MatAmbient;
		NLMISC::CRGBA		MatDiffuse;
		// The UV to lookup in landscape Dynamic Lightmap
		CVegetableUV8		DlmUV;
		// The index on the first index in CVegetableRdrPass::Vertices array.
		uint				StartIdInRdrPass;
	};


	// a rdrPass to render pack of triangles in one time.
	struct	CVegetableRdrPass
	{
		// vertices are in VBSoft or VBHard ??
		bool					HardMode;
		// List of vertices used (used for deletion of the ig, and also for change of HardMode).
		NLMISC::CObjectVector<uint32, false>	Vertices;
		// List of faces indices to render. They points to vertex in VBuffer.
		CIndexBuffer							TriangleIndices;
		// List of faces indices to render. They points to Vertices in this.
		NLMISC::CObjectVector<uint32, false>	TriangleLocalIndices;
		// List of faces indices to render. for lighting updates.
		NLMISC::CObjectVector<CVegetableLightedInstance>	LightedInstances;
		// the number of triangles currently setuped, ie _TriangleIndices.size()/3.
		uint32					NTriangles;
		// the number of vertices currently setuped.
		uint32					NVertices;
		// the number of Lighted instances currently setuped.
		uint32					NLightedInstances;


		CVegetableRdrPass()
		{
			HardMode= true;
			NTriangles= 0;
			NVertices= 0;
			NLightedInstances= 0;
			NL_SET_IB_NAME(TriangleIndices, "CVegetableRdrPass");
		}
	};


	// For now, there is only 5 render pass: Lighted and Unlit combined with 2Sided or not + the ZSort one.
	CVegetableRdrPass			_RdrPass[NL3D_VEGETABLE_NRDRPASS];

	// list of triangles order, for quadrant ZSorting. only for NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT rdrpass.
	// this is why this don't appear in CVegetableRdrPass
	NLMISC::CObjectVector<sint16, false>			_TriangleQuadrantOrderArray;
	uint						_TriangleQuadrantOrderNumTriangles;
	sint16						*_TriangleQuadrantOrders[NL3D_VEGETABLE_NUM_QUADRANT];
	// If the Igs contains some instance in NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT rdrpass, this flag is true.
	bool						_HasZSortPassInstances;


	/// \name UpdateLighting management
	// @{

	// Circular list. at ctor, init to this. At dtor, unlinkUL.
	CVegetableInstanceGroup		*_ULPrec;
	CVegetableInstanceGroup		*_ULNext;
	// Sum of all lighted vertices in this IG.
	uint						_ULNumVertices;

	/// insert this before igNext.
	void			linkBeforeUL(CVegetableInstanceGroup *igNext);
	/// unlink
	void			unlinkUL();

	// @}


};


// ***************************************************************************
/**
 *	Mirror struct of CVegetableInstanceGroup, for reserveIg system in CVegetableManager.
 *	Internal to VegetableManager.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableInstanceGroupReserve
{
public:

	/// Constructor
	CVegetableInstanceGroupReserve();


// ********************
private:
	friend class	CVegetableManager;


	// For each rdrPass, the number of Vertices and Triangles to reserve.
	struct	CVegetableRdrPass
	{
		uint		NTriangles;
		uint		NVertices;
		uint		NLightedInstances;

		CVegetableRdrPass()
		{
			NVertices= 0;
			NTriangles= 0;
			NLightedInstances= 0;
		}
	};

	// space to be reserved for all rdrPass.
	CVegetableRdrPass			_RdrPass[NL3D_VEGETABLE_NRDRPASS];

};


} // NL3D


#endif // NL_VEGETABLE_INSTANCE_GROUP_H

/* End of vegetable_instance_group.h */
