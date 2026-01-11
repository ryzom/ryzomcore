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

#include "nel/3d/patch_rdr_pass.h"
#include "nel/3d/index_buffer.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
CRdrTileId::CRdrTileId()
{
	PatchRdrPass= NULL;
	TileMaterial= NULL;
	_Next= NULL;
}


// ***************************************************************************
CPatchRdrPass::CPatchRdrPass()
{
	RefCount= 0;

	clearAllRenderList();
}


// ***************************************************************************
void	CPatchRdrPass::clearAllRenderList()
{
	_MaxRenderedFaces= 0;
	_Far0ListRoot= NULL;
	_Far1ListRoot= NULL;
	for(uint i=0;i<NL3D_MAX_TILE_PASS; i++)
	{
		_TileListRoot[i]= NULL;
	}
}


} // NL3D
