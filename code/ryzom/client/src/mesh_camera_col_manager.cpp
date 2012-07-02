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
#include "mesh_camera_col_manager.h"
#include "precipitation_clip_grid.h"
#include "nel/3d/u_visual_collision_manager.h"
#include "nel/3d/u_shape_bank.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/system_info.h"
#include "nel/3d/u_visual_collision_mesh.h"

using namespace NL3D;
using namespace NLMISC;
using namespace std;


// ***************************************************************************
CMeshCameraColManager	MeshCameraColManager;


// ***************************************************************************
extern UVisualCollisionManager		*CollisionManager;
extern UDriver						*Driver;


// ***************************************************************************
CMeshCameraColManager::CMeshCameraColManager()
{
}

// ***************************************************************************
CMeshCameraColManager::~CMeshCameraColManager()
{
	if(!CollisionManager)
		return;

	// reset any remaining mesh
	TIgMap::iterator	it;
	for(it= _IgMap.begin();it!=_IgMap.end();it++)
	{
		CMeshGroup	&mg= it->second;
		for(uint i=0;i<mg.Meshs.size();i++)
		{
			CollisionManager->removeMeshCollision(mg.Meshs[i]);
		}
	}
	_IgMap.clear();
}

// ***************************************************************************
void CMeshCameraColManager::instanceGroupLoaded(NL3D::UInstanceGroup * /* ig */)
{
	// no op
}

// ***************************************************************************
void CMeshCameraColManager::instanceGroupAdded(NL3D::UInstanceGroup *ig)
{
	if(!ig)
		return;

	// TestYoyo
	/*CSimpleClock	clock;
	clock.start();*/

	// Build a list of Mesh to add to the collision manager
	CMeshGroup	mg;

	// for all instance of the ig
	for(uint i=0;i<ig->getNumInstance();i++)
	{
		string shapeName= ig->getShapeName(i);
		if (!shapeName.empty())
		{
			// well... get the actual name (not transformed)
			if (shapeName.find('.') == std::string::npos)
				shapeName += ".shape";

			// try to get the shape
			UShape shape= Driver->getShapeBank()->getShape(shapeName);
			// if found
			if(!shape.empty())
			{
				UVisualCollisionMesh	colMesh;
				shape.getVisualCollisionMesh(colMesh);
				// if this mesh has a collision
				if(!colMesh.empty())
				{
					// get the instance matrix
					CMatrix	mat;
					ig->getInstanceMatrix(i, mat);

					// special code for matis serre. Use the same flags as for shadows
					bool	avoidCollisionInside= ig->dontCastShadowForInterior(i);
					bool	avoidCollisionOutside= ig->dontCastShadowForExterior(i);
					// very special patch for the matis serre (grrrrrrrrrrrrr)
					avoidCollisionOutside= avoidCollisionOutside || strlwr(shapeName)== "ma_serre_interieur.shape";

					// then send the result to the collision manager, and keep the mesh col id if succeed
					uint32	meshId= CollisionManager->addMeshInstanceCollision(colMesh, mat, avoidCollisionInside, avoidCollisionOutside);
					if(meshId)
						mg.Meshs.push_back(meshId);
				}
			}
		}
	}

	// if mesh group not empty, append to the map (for future remove)
	if(!mg.Meshs.empty())
	{
		// should not be present
		nlassert(_IgMap.find(ig)==_IgMap.end());
		// insert
		_IgMap[ig]= mg;
	}

	// TestYoyo
	/*clock.stop();
	double freq = (double) CSystemInfo::getProcessorFrequency(false);
	double msPerTick = 1000 / (double) freq;
	nlinfo("IG Camera Added: {%x, %d}: %.3f ms", (uint32)ig, ig->getNumInstance(), msPerTick * clock.getNumTicks());*/
}

// ***************************************************************************
void CMeshCameraColManager::instanceGroupRemoved(NL3D::UInstanceGroup *ig)
{
	// find the ig in the map
	TIgMap::iterator	it= _IgMap.find(ig);
	if(it!=_IgMap.end())
	{
		// TestYoyo
		/*CSimpleClock	clock;
		clock.start();*/

		// remove the mesh collision from the manager
		CMeshGroup	&mg= it->second;
		for(uint i=0;i<mg.Meshs.size();i++)
		{
			CollisionManager->removeMeshCollision(mg.Meshs[i]);
			//HeightGrid.removeCollisionMesh(mg.Meshs[i]);
		}
		// remove
		_IgMap.erase(it);

		// TestYoyo
		/*clock.stop();
		double freq = (double) CSystemInfo::getProcessorFrequency(false);
		double msPerTick = 1000 / (double) freq;
		nlinfo("IG Camera Removed: %.3f ms", msPerTick * clock.getNumTicks());*/
	}
}

