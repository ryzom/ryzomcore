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

#ifndef NL_MESH_CAMERA_COL_MANAGER_H
#define NL_MESH_CAMERA_COL_MANAGER_H

#include "nel/misc/types_nl.h"
#include "ig_enum.h"
#include <vector>
#include <map>


// ***************************************************************************
/**
 * Manager to Add Mesh to the collision manager for camera collision
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CMeshCameraColManager : public IIGObserver
{
public:

	/// Constructor
	CMeshCameraColManager();
	~CMeshCameraColManager();

private:
	virtual void instanceGroupLoaded(NL3D::UInstanceGroup *ig);
	virtual void instanceGroupAdded(NL3D::UInstanceGroup *ig);
	virtual void instanceGroupRemoved(NL3D::UInstanceGroup *ig);

	// A collection of collision mesh
	class CMeshGroup
	{
	public:
		std::vector<uint32>		Meshs;
	};

	// mesh added to the collision manager per instance group
	typedef	std::map<NL3D::UInstanceGroup*, CMeshGroup>	TIgMap;
	TIgMap						_IgMap;
};

// The instance
extern CMeshCameraColManager	MeshCameraColManager;


#endif // NL_MESH_CAMERA_COL_MANAGER_H

/* End of mesh_camera_col_manager.h */
