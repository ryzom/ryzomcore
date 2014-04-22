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

#include "nel/3d/shape.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/scene.h"

#include <string>


using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// IShape
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTransformShape		*IShape::createInstance(CScene &scene)
{
	CTransformShape		*mo= (CTransformShape*)scene.createModel(NL3D::TransformShapeId);
	mo->Shape= this;
	return mo;
}


// ***************************************************************************
IShape::IShape()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_DistMax= -1;
}


// ***************************************************************************
void			IShape::setDistMax(float distMax)
{
	_DistMax= distMax;
	// normalize infinite setup.
	if(distMax<0)
		_DistMax= -1;
}


// ***************************************************************************
void			IShape::getAABBox(CAABBox &bbox) const
{
	bbox.setCenter(CVector::Null);
	bbox.setHalfSize(CVector::Null);
}


// ***************************************************************************
// ***************************************************************************
// CShapeStream
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CShapeStream::CShapeStream ()
{
	_Shape=NULL;
}


// ***************************************************************************
CShapeStream::CShapeStream (IShape* shape)
{
	// Set the pointer
	setShapePointer (shape);
}


// ***************************************************************************
void CShapeStream::setShapePointer (IShape* shape)
{
	_Shape=shape;
}


// ***************************************************************************
IShape*	CShapeStream::getShapePointer () const
{
	return _Shape;
}


// ***************************************************************************
void CShapeStream::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// First, serial an header or checking if it is correct
	f.serialCheck (NELID("PAHS"));

	// Then, serial the shape
	f.serialPolyPtr (_Shape);

	// Ok, it's done
}

} // NL3D
