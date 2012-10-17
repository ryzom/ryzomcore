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

#include "nel/3d/root_model.h"
#include "nel/3d/scene.h"


namespace NL3D {


// ***************************************************************************
void	CRootModel::registerBasic()
{
	CScene::registerModel( RootModelId, 0, CRootModel::creator);
}


// ***************************************************************************
void	CRootModel::traverseHrc()
{
	// Traverse the Hrc sons.
	uint	num= hrcGetNumChildren();
	for(uint i=0;i<num;i++)
		hrcGetChild(i)->traverseHrc();
}

// ***************************************************************************
void	CRootModel::traverseClip()
{
	// Traverse the Clip sons.
	uint	num= clipGetNumChildren();
	for(uint i=0;i<num;i++)
		clipGetChild(i)->traverseClip();
}

// ***************************************************************************
void	CRootModel::traverseAnimDetail()
{
	// no-op
}

// ***************************************************************************
void	CRootModel::traverseLoadBalancing()
{
	// no-op
}

// ***************************************************************************
void	CRootModel::traverseLight()
{
	// no-op
}

// ***************************************************************************
void	CRootModel::traverseRender()
{
	// no-op
}


} // NL3D
