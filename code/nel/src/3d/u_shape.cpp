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
#include "nel/3d/u_shape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_multi_lod.h"
#include "nel/3d/u_visual_collision_mesh.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{

// ***************************************************************************
bool		UShape::getMeshTriangles(std::vector<NLMISC::CVector> &vertices, std::vector<uint32> &indices) const
{
	if(!_Object)
		return false;

	// **** try to get a CMeshGeom
	CMesh				*mesh= dynamic_cast<CMesh*>(_Object);
	CMeshMultiLod		*meshMulti= dynamic_cast<CMeshMultiLod*>(_Object);
	const CMeshGeom		*meshGeom= NULL;
	if(mesh)
	{
		meshGeom= &mesh->getMeshGeom();
	}
	else if(meshMulti)
	{
		// get the first (bigger) meshGeom
		if(meshMulti->getNumSlotMesh())
		{
			meshGeom= dynamic_cast<const CMeshGeom*>(&meshMulti->getMeshGeom(0));
		}
	}

	if(!meshGeom)
		return false;

	// **** try to retrieve data
	if(! (meshGeom->retrieveVertices(vertices) && meshGeom->retrieveTriangles(indices)) )
	{
		vertices.clear();
		indices.clear();
		return false;
	}

	// ok!
	return true;
}


// ***************************************************************************
void		UShape::getVisualCollisionMesh(UVisualCollisionMesh	&colMesh) const
{
	colMesh.attach(NULL);

	CMeshBase			*mesh= dynamic_cast<CMeshBase*>(_Object);
	if(mesh)
	{
		// attach the possible col mesh
		colMesh.attach(mesh->getVisualCollisionMesh());
	}
}

// ***************************************************************************
uint				UShape::getNumMaterials() const
{
	CMeshBase			*mesh= dynamic_cast<CMeshBase*>(_Object);
	if(mesh)
	{
		return mesh->getNbMaterial();
	}

	// fails => return 0
	return 0;
}

// ***************************************************************************
UMaterial		UShape::getMaterial(uint materialId) const
{
	CMeshBase			*mesh= dynamic_cast<CMeshBase*>(_Object);
	if(mesh)
	{
		if(materialId<mesh->getNbMaterial())
			return UMaterial(&mesh->getMaterial(materialId));
	}

	// fails => return NULL material
	return UMaterial();
}

// ***************************************************************************
bool			UShape::getDefaultOpacity() const
{
	CMeshBase			*mesh= dynamic_cast<CMeshBase*>(_Object);
	if(mesh)
	{
		return mesh->getDefaultOpacity();
	}

	return false;
}

// ***************************************************************************
bool			UShape::getDefaultTransparency() const
{
	CMeshBase			*mesh= dynamic_cast<CMeshBase*>(_Object);
	if(mesh)
	{
		return mesh->getDefaultTransparency();
	}

	return false;
}


} // NL3D
