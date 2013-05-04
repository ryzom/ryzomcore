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

#include "nel/3d/vegetable_instance_group.h"


namespace NL3D
{


// ***************************************************************************
CVegetableInstanceGroup::CVegetableInstanceGroup()
{
	_ClipOwner= NULL;
	_SortOwner= NULL;
	_HasZSortPassInstances= false;
	_TriangleQuadrantOrderNumTriangles= 0;
	_ULPrec= this;
	_ULNext= this;
	_ULNumVertices= 0;
}


// ***************************************************************************
CVegetableInstanceGroup::~CVegetableInstanceGroup()
{
	unlinkUL();
}


// ***************************************************************************
void			CVegetableInstanceGroup::linkBeforeUL(CVegetableInstanceGroup *igNext)
{
	nlassert(igNext);

	// first, unlink others from me. NB: works even if _ULPrec==_ULNext==this.
	_ULNext->_ULPrec= _ULPrec;
	_ULPrec->_ULNext= _ULNext;
	// link to igNext.
	_ULNext= igNext;
	_ULPrec= igNext->_ULPrec;
	// link others to me.
	_ULNext->_ULPrec= this;
	_ULPrec->_ULNext= this;
}

// ***************************************************************************
void			CVegetableInstanceGroup::unlinkUL()
{
	// unlink others from me. NB: works even if _ULPrec==_ULNext==this.
	_ULNext->_ULPrec= _ULPrec;
	_ULPrec->_ULNext= _ULNext;
	// reset
	_ULPrec= this;
	_ULNext= this;
}


// ***************************************************************************
CVegetableInstanceGroupReserve::CVegetableInstanceGroupReserve()
{
}


// ***************************************************************************
bool			CVegetableInstanceGroup::isEmpty() const
{
	for(uint i=0; i<NL3D_VEGETABLE_NRDRPASS; i++)
	{
		const CVegetableRdrPass	&vegetRdrPass= _RdrPass[i];
		// If some triangles to render, the ig is not empty
		if(vegetRdrPass.NTriangles != 0)
			return false;
	}

	// for all pass, no triangles to render => the ig is empty.
	return true;
}



} // NL3D
