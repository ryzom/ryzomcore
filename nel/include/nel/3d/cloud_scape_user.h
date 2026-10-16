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
	virtual	~CCloudScapeUser() NL_OVERRIDE;

	virtual void init (SCloudScapeSetup *pCSS = nullptr) NL_OVERRIDE;

	virtual void set (SCloudScapeSetup &css) NL_OVERRIDE;

	virtual void anim (double dt) NL_OVERRIDE;

	virtual void render () NL_OVERRIDE;

	virtual uint32 getMemSize() NL_OVERRIDE;

	virtual void setQuality (float threshold) NL_OVERRIDE;

	virtual void setNbCloudToUpdateIn80ms (uint32 n) NL_OVERRIDE;

	virtual bool isDebugQuadEnabled () NL_OVERRIDE;

	virtual void setDebugQuad (bool b) NL_OVERRIDE;

	CCloudScape *_CS;
	CScene *_Scene;
};


} // NL3D

