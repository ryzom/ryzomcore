// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
//
#include "mesh_array.h"
//
#include "../global.h"
#include "../misc.h"


using namespace NLMISC;
using namespace NL3D;

namespace R2
{


// *********************************************************************************************************
CMeshArray::CMeshArray()
{
	_Active = true;
}

// *********************************************************************************************************
CMeshArray::~CMeshArray()
{
	clear();
}

// *********************************************************************************************************
void CMeshArray::setShapeName(const std::string &shapeName)
{
	//H_AUTO(R2_CMeshArray_setShapeName)
	resize(0);
	_ShapeName = shapeName;
}

// *********************************************************************************************************
void CMeshArray::resize(uint newSize)
{
	//H_AUTO(R2_CMeshArray_resize)
	if (newSize < _MeshInstances.size())
	{
		for(uint k = newSize; k < _MeshInstances.size(); ++k)
		{
			Scene->deleteInstance(_MeshInstances[k]);
		}
		_MeshInstances.resize(newSize);
	}
	else
	{
		uint oldSize = (uint)_MeshInstances.size();
		_MeshInstances.resize(newSize);
		for(uint k = oldSize; k < newSize; ++k)
		{
			_MeshInstances[k] = Scene->createInstance(_ShapeName);
			if (_Active)
			{
				_MeshInstances[k].show();
			}
			else
			{
				_MeshInstances[k].hide();
			}
		}
	}
}

// *********************************************************************************************************
UInstance &CMeshArray::getInstance(uint index)
{
	//H_AUTO(R2_CMeshArray_getInstance)
	nlassert(index < _MeshInstances.size());
	return _MeshInstances[index];
}

// *********************************************************************************************************
const UInstance &CMeshArray::getInstance(uint index) const
{
	//H_AUTO(R2_CMeshArray_getInstance)
	nlassert(index < _MeshInstances.size());
	return _MeshInstances[index];
}

// *********************************************************************************************************
void CMeshArray::setEmissive(NLMISC::CRGBA color)
{
	//H_AUTO(R2_CMeshArray_setEmissive)
	for(uint k = 0; k < _MeshInstances.size(); ++k)
	{
		::setEmissive(_MeshInstances[k], color);
	}
}

// *********************************************************************************************************
void CMeshArray::setActive(bool active)
{
	//H_AUTO(R2_CMeshArray_setActive)
	if (active == _Active) return;
	for(uint k = 0; k < _MeshInstances.size(); ++k)
	{
		if (active)
		{
			_MeshInstances[k].show();
		}
		else
		{
			_MeshInstances[k].hide();
		}
	}
	_Active = active;
}


} // R2
