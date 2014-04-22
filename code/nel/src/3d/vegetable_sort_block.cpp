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

#include "std3d.h"

#include "nel/3d/vegetable_sort_block.h"
#include "nel/3d/radix_sort.h"
#include "nel/misc/fast_floor.h"
#include "nel/misc/hierarchical_timer.h"


using namespace std;
using namespace NLMISC;

namespace NL3D
{


// sort triangles with 2 mm of precision. It leaves 64 m with an uint16, which is really sufficient :)
#define	NL3D_VEGETABLE_TRI_RADIX_KEY_PRECISION	512


// ***************************************************************************
CVegetableSortBlock::CVegetableSortBlock()
{
	ZSortHardMode= true;
	_NTriangles= 0;
	_NIndices= 0;
	_Dirty= false;
	_UnderWater= false;
	NL_SET_IB_NAME(_SortedTriangleArray, "CVegetableSortBlock");
}


// ***************************************************************************
// the struct to sort a triangle.
struct	CSortTri
{
	// index of the triangle.
	uint16	TriIndex;

	// QSort only.
	// distance.
	sint16	Dist;
	bool	operator<(const CSortTri &o) const
	{
		return Dist>o.Dist;
	}
};


// ***************************************************************************
void			CVegetableSortBlock::updateSortBlock(CVegetableManager &vegetManager)
{
	H_AUTO( NL3D_Vegetable_Update_SortBlock );

	// if nothing to update (ie instance added/deleted do not impact me).
	if(!_Dirty)
	{
		// nothing to do.
		return;
	}
	else
	{
		// Ok clean me now.
		_Dirty= false;
	}

	// compute number of triangles.
	_NTriangles= 0;
	CVegetableInstanceGroup		*ptrIg= _InstanceGroupList.begin();
	while(ptrIg)
	{
		// add only zsort rdrPass triangles.
		_NTriangles+= ptrIg->_RdrPass[NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT].NTriangles;

		// next Ig in the SortBlock
		ptrIg= (CVegetableInstanceGroup*)(ptrIg->Next);
	}
	// compute number of indices
	_NIndices= _NTriangles*3;


	// if no triangles, clear and go
	if(_NTriangles == 0)
	{
		// reset the array of indices.
		_SortedTriangleArray.deleteAllIndexes();
		// bye
		return;
	}
	else
	{
		// else, re-allocate the array
		_SortedTriangleArray.setFormat(ZSortHardMode ? CIndexBuffer::Indices16 : CIndexBuffer::Indices32);
		_SortedTriangleArray.setNumIndexes(_NIndices * NL3D_VEGETABLE_NUM_QUADRANT);
	}

	// resize an array for sorting.
	static	std::vector<CSortTri>	triSort;
	static	std::vector<uint32>		triIndices32;
	static	std::vector<uint16>		triIndices16;
	// don't use resize because fill with default values. size() is the capacity here.
	if(triSort.size()<_NTriangles)
	{
		triSort.resize(_NTriangles);
	}

	if (ZSortHardMode)
	{
		nlassert(_NTriangles < 65536);
		if (triIndices16.size() < _NIndices) triIndices16.resize(_NIndices);
	}
	else
	{
		if (triIndices32.size() < _NIndices) triIndices32.resize(_NIndices);
	}

	// fill indices with all ig info.
	//-------------
	if (ZSortHardMode)
	{
		uint16		*triIdxPtr= &triIndices16[0];
		// for all igs in the sortBlock.
		ptrIg= _InstanceGroupList.begin();
		while(ptrIg)
		{
			CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ptrIg->_RdrPass[NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT];
			nlassert(vegetRdrPass.HardMode);
			CIndexBufferRead ibaRead;
			vegetRdrPass.TriangleIndices.lock (ibaRead);
			nlassert(ibaRead.getFormat() == CIndexBuffer::Indices16);
			const uint16		*triSrcPtr= (const uint16 *) ibaRead.getPtr();
			// add only zsort rdrPass triangles.
			for(uint i=0; i<vegetRdrPass.NTriangles; i++)
			{
				// fill the triangle indices.
				*(triIdxPtr++)= *(triSrcPtr++);
				*(triIdxPtr++)= *(triSrcPtr++);
				*(triIdxPtr++)= *(triSrcPtr++);
			}
			// next Ig in the SortBlock
			ptrIg= (CVegetableInstanceGroup*)(ptrIg->Next);
		}
	}
	else
	{
		uint32		*triIdxPtr= &triIndices32[0];
		// for all igs in the sortBlock.
		ptrIg= _InstanceGroupList.begin();
		while(ptrIg)
		{
			CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ptrIg->_RdrPass[NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT];
			nlassert(!vegetRdrPass.HardMode);
			CIndexBufferRead ibaRead;
			vegetRdrPass.TriangleIndices.lock (ibaRead);
			nlassert(ibaRead.getFormat() == CIndexBuffer::Indices32);
			const uint32		*triSrcPtr= (const uint32 *) ibaRead.getPtr();
			// add only zsort rdrPass triangles.
			for(uint i=0; i<vegetRdrPass.NTriangles; i++)
			{
				// fill the triangle indices.
				*(triIdxPtr++)= *(triSrcPtr++);
				*(triIdxPtr++)= *(triSrcPtr++);
				*(triIdxPtr++)= *(triSrcPtr++);
			}
			// next Ig in the SortBlock
			ptrIg= (CVegetableInstanceGroup*)(ptrIg->Next);
		}
	}

	// Sort for all quadrants
	//-------------
	for(uint quadrant=0; quadrant<NL3D_VEGETABLE_NUM_QUADRANT; quadrant++)
	{
		// ReFill SortTris with Start Data
		//-------------
		CSortTri	*triPtr= &triSort[0];
		uint		triId= 0;
		// for all igs in the sortBlock.
		ptrIg= _InstanceGroupList.begin();
		while(ptrIg)
		{
			CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ptrIg->_RdrPass[NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT];

			// add only zsort rdrPass triangles.
			for(uint i=0; i<vegetRdrPass.NTriangles; i++, triPtr++, triId++)
			{
				// QSort.
				triPtr->Dist = ptrIg->_TriangleQuadrantOrders[quadrant][i];

				// copy tri info
				triPtr->TriIndex= triId;
			}

			// next Ig in the SortBlock
			ptrIg= (CVegetableInstanceGroup*)(ptrIg->Next);
		}

		// sort the array according to distance
		//-------------
		// QSort.
		sort(triSort.begin(), triSort.begin()+_NTriangles);


		// Fill result.
		//-------------
		// init quadrant ptr.
		_SortedTriangleIndices[quadrant]= quadrant * _NIndices;

		// fill the indices.
		CIndexBufferReadWrite ibaReadWrite;
		_SortedTriangleArray.lock (ibaReadWrite);
		if (ZSortHardMode)
		{
			nlassert(ibaReadWrite.getFormat() == CIndexBuffer::Indices16);
			uint16	*pIdx= (uint16 *) ibaReadWrite.getPtr() + _SortedTriangleIndices[quadrant];
			for(uint i=0; i<_NTriangles; i++)
			{
				uint32	idTriIdx= triSort[i].TriIndex * 3;
				*(pIdx++)= (uint16) triIndices16[idTriIdx+0];
				*(pIdx++)= (uint16) triIndices16[idTriIdx+1];
				*(pIdx++)= (uint16) triIndices16[idTriIdx+2];
			}
		}
		else
		{
			nlassert(ibaReadWrite.getFormat() == CIndexBuffer::Indices32);
			uint32	*pIdx= (uint32 *) ibaReadWrite.getPtr() + _SortedTriangleIndices[quadrant];
			for(uint i=0; i<_NTriangles; i++)
			{
				uint32	idTriIdx= triSort[i].TriIndex * 3;
				*(pIdx++)= triIndices32[idTriIdx+0];
				*(pIdx++)= triIndices32[idTriIdx+1];
				*(pIdx++)= triIndices32[idTriIdx+2];
			}
		}
	}
}




} // NL3D
