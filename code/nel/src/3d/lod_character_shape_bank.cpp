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

#include "nel/3d/lod_character_shape_bank.h"
#include "nel/3d/lod_character_shape.h"


using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{



// ***************************************************************************
CLodCharacterShapeBank::CLodCharacterShapeBank()
{
}

// ***************************************************************************
void						CLodCharacterShapeBank::reset()
{
	contReset(_ShapeArray);
	contReset(_ShapeMap);
}

// ***************************************************************************
uint32						CLodCharacterShapeBank::addShape()
{
	// Alloc a new shape
	_ShapeArray.resize(_ShapeArray.size()+1);

	return (uint32)_ShapeArray.size()-1;
}

// ***************************************************************************
sint32						CLodCharacterShapeBank::getShapeIdByName(const std::string &name) const
{
	CstItStrIdMap	it= _ShapeMap.find(name);
	if(it==_ShapeMap.end())
		return -1;
	else
		return it->second;
}

// ***************************************************************************
const CLodCharacterShape	*CLodCharacterShapeBank::getShape(uint32 shapeId) const
{
	if(shapeId >= _ShapeArray.size())
		return NULL;
	else
		return &_ShapeArray[shapeId];
}

// ***************************************************************************
CLodCharacterShape			*CLodCharacterShapeBank::getShapeFullAcces(uint32 shapeId)
{
	if(shapeId >= _ShapeArray.size())
		return NULL;
	else
		return &_ShapeArray[shapeId];
}

// ***************************************************************************
bool						CLodCharacterShapeBank::compile()
{
	bool	error= false;

	// clear the map
	contReset(_ShapeMap);

	// build the map
	for(uint i=0; i<_ShapeArray.size(); i++)
	{
		const string &name= _ShapeArray[i].getName();
		ItStrIdMap	it= _ShapeMap.find(name);
		if(it == _ShapeMap.end())
			_ShapeMap.insert(make_pair(name, i));
		else
		{
			error= true;
			nlwarning("Found a CLod with same name in the bank: %s", name.c_str());
		}
	}

	return error;
}

// ***************************************************************************
uint						CLodCharacterShapeBank::getNumShapes() const
{
	return (uint)_ShapeArray.size();
}

// ***************************************************************************
void			CLodCharacterShapeBank::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serialCont(_ShapeArray);
	f.serialCont(_ShapeMap);
}



} // NL3D
