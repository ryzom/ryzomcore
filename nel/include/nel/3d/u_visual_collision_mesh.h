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

#ifndef NL_U_VISUAL_COLLISION_MESH_H
#define NL_U_VISUAL_COLLISION_MESH_H

#include "nel/misc/types_nl.h"


namespace NL3D
{


// ***************************************************************************
/**
 * Proxy to a Collision Mesh possibly stored in a UShape
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class UVisualCollisionMesh
{
public:

	/// Proxy interface
	UVisualCollisionMesh();
	/// Attach a IShape to this proxy
	void			attach(class CVisualCollisionMesh	*mesh);
	/// return true if the proxy is empty() (not attached)
	bool			empty() const {return _Mesh==NULL;}
	/// For Advanced usage, get the Collision Mesh ptr
	class CVisualCollisionMesh	*getMeshPtr() const {return _Mesh;}
	// get vertices of the mesh
	const std::vector<NLMISC::CVector> &getVertices() const;
	// get triangles of the mesh
	const std::vector<uint16> &getTriangles() const;
private:
	class CVisualCollisionMesh	*_Mesh;

};


} // NL3D


#endif // NL_U_VISUAL_COLLISION_MESH_H

/* End of u_visual_collision_mesh.h */
