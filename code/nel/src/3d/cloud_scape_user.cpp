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

#include "nel/3d/u_cloud_scape.h"
#include "nel/3d/cloud_scape_user.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/cloud_scape.h"
#include "nel/3d/scene.h"


namespace NL3D
{

//===========================================================================
CCloudScapeUser::CCloudScapeUser (CScene *scene) : UCloudScape ()
{
	nlassert(scene);
	_Scene = scene;
	_CS = new CCloudScape(_Scene->getDriver());
}

//===========================================================================
CCloudScapeUser::~CCloudScapeUser ()
{
	delete _CS;
}

//===========================================================================
void CCloudScapeUser::init (SCloudScapeSetup *pCSS)
{
	_CS->init (pCSS, _Scene->getCam());
}

//===========================================================================
void CCloudScapeUser::set (SCloudScapeSetup &css)
{
	_CS->set (css);
}

//===========================================================================
void CCloudScapeUser::anim (double dt)
{
	_CS->anim (dt, _Scene->getCam());
}

//===========================================================================
void CCloudScapeUser::render ()
{
	_CS->render ();
}

//===========================================================================
uint32 CCloudScapeUser::getMemSize()
{
	return _CS->getMemSize();
}

//===========================================================================
void CCloudScapeUser::setQuality (float threshold)
{
	_CS->setQuality (threshold);
}

//===========================================================================
void CCloudScapeUser::setNbCloudToUpdateIn80ms (uint32 n)
{
	_CS->setNbCloudToUpdateIn80ms (n);
}

//===========================================================================
bool CCloudScapeUser::isDebugQuadEnabled ()
{
	return _CS->isDebugQuadEnabled ();
}

//===========================================================================
void CCloudScapeUser::setDebugQuad (bool b)
{
	_CS->setDebugQuad (b);
}

} // NL3D

