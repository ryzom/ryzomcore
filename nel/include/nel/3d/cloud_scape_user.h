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

#include "nel/3d/u_cloud_scape.h"

namespace NL3D
{

class UDriver;
class UCamera;
class CCloudScape;
class CScene;

/// implementation of UWaterInstance methods
class CCloudScapeUser : public UCloudScape
{
public:

	CCloudScapeUser(CScene *scene);
	virtual	~CCloudScapeUser();

	virtual void init (SCloudScapeSetup *pCSS = NULL);

	virtual void set (SCloudScapeSetup &css);

	virtual void anim (double dt);

	virtual void render ();

	virtual uint32 getMemSize();

	virtual void setQuality (float threshold);

	virtual void setNbCloudToUpdateIn80ms (uint32 n);

	virtual bool isDebugQuadEnabled ();

	virtual void setDebugQuad (bool b);

	CCloudScape *_CS;
	CScene *_Scene;
};


} // NL3D

