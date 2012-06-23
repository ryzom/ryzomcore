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

#include "nel/3d/u_visual_collision_mesh.h"
#include "nel/3d/visual_collision_mesh.h"


namespace NL3D
{


// ***************************************************************************
UVisualCollisionMesh::UVisualCollisionMesh() : _Mesh(NULL)
{
}

// ***************************************************************************
void			UVisualCollisionMesh::attach(class CVisualCollisionMesh	*mesh)
{
	_Mesh= mesh;
}

// ***************************************************************************
const std::vector<NLMISC::CVector> &UVisualCollisionMesh::getVertices() const
{
	return _Mesh->getVertices();
}

// ***************************************************************************
const std::vector<uint16> &UVisualCollisionMesh::getTriangles() const
{
	return _Mesh->getTriangles();
}


} // NL3D
