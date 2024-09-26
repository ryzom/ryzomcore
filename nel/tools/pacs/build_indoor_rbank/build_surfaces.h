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

#ifndef NL_BUILD_SURFACES_H
#define NL_BUILD_SURFACES_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/pacs/collision_mesh_build.h"

// External class declaration
namespace NLPACS
{
	class CLocalRetriever;
};

/**
 * The interior surface class. Intermediate to compute real retriever surfaces
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CInteriorSurface
{
public:
	/// The collision mesh root object
	NLPACS::CCollisionMeshBuild		*CollisionMeshBuild;

	/// The faces that compose the surface
	std::vector<uint32>				Faces;

	/// The Id of the surface
	sint32							Id;

	/// The center of the surface
	NLMISC::CVector					Center;

	/// The material of the surface
	sint32							Material;

public:
	NLPACS::CCollisionFace			&getFace(uint face) { return CollisionMeshBuild->Faces[Faces[face]]; }
	NLPACS::CCollisionFace			&getNeighbor(uint face, uint edge)
	{
		return CollisionMeshBuild->Faces[getFace(face).Edge[edge]];
	}
};


/**
 * The border of interior surfaces.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CInteriorBorder
{
public:
	/// The vertices that compose the border
	std::vector<NLMISC::CVector>	Vertices;

	/// The left and right surfaces
	sint32							Left, Right;

public:
};


// how to build interior snapping data
void	buildSnapping(NLPACS::CCollisionMeshBuild &cmb, NLPACS::CLocalRetriever &lr);


// how to build surfaces
void	buildSurfaces(NLPACS::CCollisionMeshBuild &cmb, NLPACS::CLocalRetriever &lr);


#endif // NL_BUILD_SURFACES_H

/* End of build_surfaces.h */
