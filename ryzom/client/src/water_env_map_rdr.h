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

#ifndef CL_WATER_ENV_MAP_RDR_H
#define CL_WATER_ENV_MAP_RDR_H


#include "misc.h"
#include "time_client.h"
#include "nel/3d/u_water_env_map.h"
#include "nel/3d/animation_time.h"
#include "nel/misc/rgba.h"

class CSky;

namespace NLMISC
{
	class CMatrix;
}

namespace NL3D
{
	class UDriver;
}

// Render water environment map from a sky scene
class CWaterEnvMapRdr : public NL3D::CWaterEnvMapRenderHelper
{
public:
	CClientDate				   CurrDate;
	CClientDate				   AnimationDate;
	CRGBA					   CurrFogColor;
	float					   CurrWeather;
	NL3D::TGlobalAnimationTime CurrTime;
	CSky					   *Sky;
public:
	CWaterEnvMapRdr() : CurrTime(-1), _LastRenderStartTime(-1)
	{
		_CurrCanopyCamPos = CVector::Null;
	}
	virtual void doRender(const NLMISC::CMatrix &camMatrix, NL3D::TGlobalAnimationTime time, NL3D::UDriver &drv);
private:
	CClientDate				   _DateForRender;
	CClientDate				   _AnimationDateForRender;
	CRGBA					   _FogColorForRender;
	float					   _WeatherForRender;
	NL3D::TGlobalAnimationTime _LastRenderStartTime;
	NLMISC::CVector			   _CurrCanopyCamPos;
};



#endif
