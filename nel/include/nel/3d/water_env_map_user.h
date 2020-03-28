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

#ifndef NL_WATER_ENVMAP_USER
#define NL_WATER_ENVMAP_USER

#include "nel/3d/u_water_env_map.h"
#include "nel/3d/water_env_map.h"
#include "nel/3d/driver_user.h"

namespace NL3D
{

struct IWaterEnvMapRender;


class CWaterEnvMapUser : public UWaterEnvMap
{
public:
	class CWaterEnvMapInternal : public CWaterEnvMap
	{
	public:
		CDriverUser		   *Driver;
		IWaterEnvMapRender *Rdr;
		// From CWaterEnvMap
		virtual void render(CTextureCube::TFace face, TGlobalAnimationTime time)
		{
			nlassert(Driver);
			if (!Rdr) return;
			Rdr->render((IWaterEnvMapRender::TFace) face, time, *Driver);
		}
	};
	CWaterEnvMapInternal EnvMap;
public:
	virtual ~CWaterEnvMapUser() { }
	virtual void				init(uint cubeMapSize, uint projection2DSize, TGlobalAnimationTime updateTime);
	// Set an external renderer that will update the envmap used for water rendering. The renderer will be called during the update as needed
	virtual	void			    setWaterEnvMapRenderCallback(IWaterEnvMapRender *rdr) { EnvMap.Rdr = rdr; }
	virtual IWaterEnvMapRender *getWaterEnvMapRenderCallback() const { return EnvMap.Rdr; }
	virtual void				invalidate();
	virtual void				setAlpha(uint8 alpha) { EnvMap.setAlpha(alpha); }
	virtual uint8				getAlpha() const { return EnvMap.getAlpha(); }
	virtual CWaterEnvMap		*getWaterEnvMap() { return &EnvMap; }
};


} // NL3D


#endif
