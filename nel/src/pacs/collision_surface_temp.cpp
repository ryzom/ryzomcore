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

#include "stdpacs.h"

#include "nel/pacs/collision_surface_temp.h"


namespace NLPACS
{


// ***************************************************************************
const	uint32	StartEdgeCollideNodeSize= 128;		// mem: 48*128= 6144
const	uint32	StartCollisionChainSize= 128;		// mem: 24*128= 3072
const	uint32	StartEdgeChainEntrySize= 1024;		// mem: 6*1024= 6144
const	uint32	StartCollisionDescSize= 64;			// mem: 36*64= 2304
const	uint32	StartCollisionInstanceSize= 64;		// mem: 4*64= 256
// Total default memory cost by CCollisionSurfaceTemp: 18Ko + 128Ko  (OChainLUT).


// ***************************************************************************
CCollisionSurfaceTemp::CCollisionSurfaceTemp()
{
	memset(OChainLUT, 0xFF, 65536*sizeof(uint16));
	memset(SurfaceLUT, 0x00, 65536*sizeof(CSurfaceLUTEntry));
	_EdgeCollideNodes.reserve(StartEdgeCollideNodeSize);
	CollisionChains.reserve(StartCollisionChainSize);
	EdgeChainEntries.reserve(StartEdgeChainEntrySize);
	CollisionDescs.reserve(StartCollisionDescSize);
	MoveDescs.reserve(StartCollisionDescSize);
	RotDescs.reserve(StartCollisionDescSize);
	CollisionInstances.reserve(StartCollisionInstanceSize);
	PossibleSurfaces.reserve(32);
	PrecValid= false;
	OutCounter = 0;
}


// ***************************************************************************
void				CCollisionSurfaceTemp::resetEdgeCollideNodes()
{
	_EdgeCollideNodes.clear();
}
// ***************************************************************************
uint32				CCollisionSurfaceTemp::allocEdgeCollideNode(uint32 size)
{
	uint32	id= (uint32)_EdgeCollideNodes.size();
	_EdgeCollideNodes.resize(id+size);
	return id;
}
// ***************************************************************************
CEdgeCollideNode	&CCollisionSurfaceTemp::getEdgeCollideNode(uint32 id)
{
	return _EdgeCollideNodes[id];
}


} // NLPACS
