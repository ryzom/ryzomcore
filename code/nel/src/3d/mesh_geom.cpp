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

#include "nel/3d/mesh_geom.h"
#include "nel/3d/mesh_block_manager.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
IMeshGeom::IMeshGeom()
{
	_RootInstanceId= -1;
	_MeshBlockManager= NULL;
	_MeshVBHeapId= 0;
	_MeshVBHeapNumVertices= 0;
}

// ***************************************************************************
IMeshGeom::~IMeshGeom()
{
	// If still have a VBHeap data in the manager, must free it.
	if(_MeshVBHeapId)
	{
		nlassert(_MeshBlockManager);
		_MeshBlockManager->freeMeshVBHeap(this);
	}
}


} // NL3D
