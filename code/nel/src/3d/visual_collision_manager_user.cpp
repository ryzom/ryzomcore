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

#include "nel/3d/visual_collision_manager_user.h"
#include "nel/3d/u_visual_collision_mesh.h"


namespace NL3D {


// ***************************************************************************
uint		CVisualCollisionManagerUser::addMeshInstanceCollision(const UVisualCollisionMesh &mesh, const NLMISC::CMatrix &instanceMatrix, bool avoidCollisionWhenInside, bool avoidCollisionWhenOutside)
{
	// if empty proxy abort
	if(mesh.empty())
		return 0;
	return _Manager.addMeshInstanceCollision(mesh.getMeshPtr(), instanceMatrix, avoidCollisionWhenInside, avoidCollisionWhenOutside);
}

// ***************************************************************************
void CVisualCollisionManagerUser::getMeshs(const NLMISC::CAABBox &aabbox, std::vector<CMeshInstanceColInfo> &dest)
{
	static std::vector<CVisualCollisionManager::CMeshInstanceColInfo> colInfos;
	_Manager.getMeshs(aabbox, colInfos);
	dest.resize(colInfos.size());
	for(uint k = 0; k < colInfos.size(); ++k)
	{
		dest[k].Mesh.attach(colInfos[k].Mesh);
		dest[k].WorldMatrix = colInfos[k].WorldMatrix;
		dest[k].ID = colInfos[k].ID;
		dest[k].WorldBBox = colInfos[k].WorldBBox;
	}
}


} // NL3D





















