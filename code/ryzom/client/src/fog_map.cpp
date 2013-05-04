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





#include "stdpch.h"
#include "nel/misc/file.h"
#include "zone_util.h"
#include "fog_map.h"
#include "client_cfg.h"
#include "nel/3d/u_driver.h"
#include <algorithm>
//
#include "interface_v3/interface_manager.h"
#include "interface_v3/group_map.h"
#include "client_cfg.h"
#include "global.h"

using namespace NLMISC;

H_AUTO_DECL(RZ_FogMap)

//===================================================================
void CFogState::setupInDriver(NL3D::UDriver &drv)
{
	H_AUTO_USE(RZ_FogMap)
	// Are we in wireframe ?
	bool filled = (drv.getPolygonMode() == NL3D::UDriver::Filled);

	if (FogEnabled && filled && ClientCfg.Fog)
	{
		drv.enableFog(true);
		drv.setupFog(FogStartDist, FogEndDist, FogColor);
	}
	else
	{
		drv.enableFog(false);
	}
}

//===================================================================
CFogMap::CFogMap() : _MinCoord(0, 0), _MaxCoord(0, 0)
{
}

//===================================================================
void CFogMap::worldPosToMapPos(float inX, float inY, float &outX, float &outY) const
{
	H_AUTO_USE(RZ_FogMap)
	outX = _MinCoord.x != _MaxCoord.x ? (inX - _MinCoord.x) / (_MaxCoord.x - _MinCoord.x) : _MinCoord.x;
	outY = _MinCoord.y != _MaxCoord.y ? 1.f - (inY - _MinCoord.y) / (_MaxCoord.y - _MinCoord.y) : 1.f - _MinCoord.y;
}

//===================================================================
void CFogMap::getFogParams(float startDist, float endDist, float x, float y, float dayNight, float duskRatio, CLightCycleManager::TLightState lightState, CFogState &result)
{
	H_AUTO_USE(RZ_FogMap)
	NLMISC::clamp(dayNight, 0.f, 1.f);
	//
	float mx, my;
	worldPosToMapPos(x, y, mx, my);
	// tmp patch  for ring island : lookup dist / depth ahead of current pos -> in matis cliffs, avoids to have too thick
	// fog when looking inside the lands
	float ddMx = x;
	float ddMy = y;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupMap *gm = dynamic_cast<CGroupMap *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:actual_map"));
	if (gm && gm->isIsland())
	{
		CVector front = MainCam.getMatrix().getJ();
		ddMx += ClientCfg.FogDistAndDepthLookupBias * front.x;
		ddMy += ClientCfg.FogDistAndDepthLookupBias * front.y;
	}
	//
	worldPosToMapPos(ddMx, ddMy, ddMx, ddMy);
	//
	NLMISC::CRGBAF col0;
	NLMISC::CRGBAF col1;
	float blendFactor;
	switch(lightState)
	{
		case CLightCycleManager::DayToNight:
			// See which transition it is : day->dusk or dusk->night ?
			if (dayNight < duskRatio)
			{
				col0 = getMapValueFromMapCoord(CFogMapBuild::Day, mx, my, CRGBAF(1.f, 1.f, 1.f));
				col1 = getMapValueFromMapCoord(CFogMapBuild::Dusk, mx, my, CRGBAF(1.f, 1.f, 1.f));
				blendFactor = duskRatio != 0.f ? dayNight / duskRatio : 0.f;
			}
			else
			{
				col0 = getMapValueFromMapCoord(CFogMapBuild::Dusk, mx, my, CRGBAF(1.f, 1.f, 1.f));
				col1 = getMapValueFromMapCoord(CFogMapBuild::Night, mx, my, CRGBAF(1.f, 1.f, 1.f));
				blendFactor = duskRatio != 1.f ? (dayNight -  duskRatio) / (1.f - duskRatio) : 0.f;
			}
		break;
		default:
			col0 = getMapValueFromMapCoord(CFogMapBuild::Day, mx, my, CRGBAF(1.f, 1.f, 1.f));
			col1 = getMapValueFromMapCoord(CFogMapBuild::Night, mx, my, CRGBAF(1.f, 1.f, 1.f));
			blendFactor = dayNight;
		break;
	}
	NLMISC::CRGBAF fogColor = col1 * blendFactor + (1.f - blendFactor) * col0;
	//
	result.FogColor = fogColor;
	//
	NLMISC::CRGBAF distanceCol = getMapValueFromMapCoord(CFogMapBuild::Distance, ddMx, ddMy, CRGBAF(0.f, 0.f, 0.f));
	result.FogStartDist = (distanceCol.R * (endDist - startDist)) + startDist;
	//
	NLMISC::CRGBAF depthCol = getMapValueFromMapCoord(CFogMapBuild::Depth,ddMx, ddMy, CRGBAF(1.f, 1.f, 1.f));
	result.FogEndDist = std::min(result.FogStartDist + depthCol.R * (endDist - startDist), endDist);
	result.FogEnabled = true;
}


//===================================================================
NLMISC::CRGBAF CFogMap::getMapValue(TMapType type, float x, float y, NLMISC::CRGBAF defaultValue) const
{
	H_AUTO_USE(RZ_FogMap)
	nlassert(type < CFogMapBuild::NumMap);
	if (_Map[type].getWidth() == 0) return defaultValue;
	float mx, my;
	worldPosToMapPos(x, y, mx, my);
	return _Map[type].getColor(mx, my);
}

//===================================================================
NLMISC::CRGBAF CFogMap::getMapValueFromMapCoord(TMapType type, float x, float y, NLMISC::CRGBAF defaultValue) const
{
	H_AUTO_USE(RZ_FogMap)
	nlassert(type < CFogMapBuild::NumMap);
	if (_Map[type].getWidth() == 0) return defaultValue;
	return _Map[type].getColor(x, y);
}

//===================================================================
void CFogMap::release()
{
	H_AUTO_USE(RZ_FogMap)
	for(uint k = 0; k < CFogMapBuild::NumMap; ++k)
	{
		_Map[k].reset();
	}
	_MinCoord.set(0.f, 0.f);
	_MaxCoord.set(0.f, 0.f);
}

//===================================================================
bool CFogMap::init(const CFogMapBuild &desc)
{
	H_AUTO_USE(RZ_FogMap)
	release();
	CVector2f minCoord, maxCoord;
	if (!getPosFromZoneName(desc.ZoneMin, minCoord) || !getPosFromZoneName(desc.ZoneMax, maxCoord))
	{
		release();
		return false;
	}
	return init(desc, minCoord, maxCoord);
}

//===================================================================
bool CFogMap::init(const CFogMapBuild &desc, const NLMISC::CVector &minCoord, const NLMISC::CVector &maxCoord)
{
	_MinCoord = minCoord;
	_MaxCoord = maxCoord;
	if (_MinCoord.x > _MaxCoord.x) std::swap(_MinCoord.x, _MaxCoord.x);
	if (_MinCoord.y > _MaxCoord.y) std::swap(_MinCoord.y, _MaxCoord.y);
	_MaxCoord.set(_MaxCoord.x + 160.f, _MaxCoord.y + 160.f);
	for(uint k = 0; k < CFogMapBuild::NumMap; ++k)
	{
		_Map[k].resize(0, 0);
		if (!desc.Map[k].empty())
		{
			load(_Map[k], desc.Map[k]);
		}
	}
	// Exception for dusk map : it has been added later, so if the filename is empty, we use the day and night map to get the dusk map
	if (desc.Map[CFogMapBuild::Dusk].empty())
	{
		if (_Map[CFogMapBuild::Day].getWidth() != 0 &&
			_Map[CFogMapBuild::Day].getHeight() != 0 &&
			_Map[CFogMapBuild::Night].getWidth() != 0 &&
			_Map[CFogMapBuild::Night].getHeight() != 0)
		{
			_Map[CFogMapBuild::Dusk].blend(_Map[CFogMapBuild::Day], _Map[CFogMapBuild::Night], 127);
		}
	}
	return true;
}

//===================================================================
void CFogMap::load(NLMISC::CBitmap &bm, const std::string &filename)
{
	H_AUTO_USE(RZ_FogMap)
	std::string lookupName = NLMISC::CPath::lookup(filename, false, true);
	if (lookupName.empty())
	{
		nlwarning("couldn't find %s", filename.c_str());
		return;
	}
	NLMISC::CIFile inputFile;
	inputFile.open(lookupName);
	try
	{
		if (!bm.load(inputFile))
		{
			nlwarning ("Error while loading fog map %s (bitmap size is multiple of 4?)", lookupName.c_str());
			inputFile.close();
		}
	}
	catch(const NLMISC::EStream &e)
	{
		nlwarning(e.what());
		inputFile.close();
	}
}























