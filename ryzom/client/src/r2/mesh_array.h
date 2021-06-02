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

#ifndef R2_MESH_ARRAY_H
#define R2_MESH_ARRAY_H


#include "nel/3d/u_instance.h"


class CEntityCL;


namespace R2
{

// An array of identical meshs instances.
class CMeshArray
{
public:
	CMeshArray();
	~CMeshArray();
	// set the shape to be used. This reset the array
	void setShapeName(const std::string &shapeName);
	const std::string &getShapeName() const{ return _ShapeName; }
	void resize(uint newSize);
	uint getSize() const { return (uint)_MeshInstances.size(); }
	NL3D::UInstance &getInstance(uint index);
	const NL3D::UInstance &getInstance(uint index) const;
	NL3D::UInstance operator[](uint index) { return getInstance(index); }
	const NL3D::UInstance operator[](uint index) const { return getInstance(index); }
	void clear() { resize(0); }
	void setEmissive(NLMISC::CRGBA color);
	bool empty() const { return _MeshInstances.empty(); }
	// show / hide
	void setActive(bool active);
	bool getActive() const { return _Active; }
private:
	std::string					 _ShapeName;
	std::vector<NL3D::UInstance> _MeshInstances;
	bool						 _Active;
};

} // R2

#endif
