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
#include "sky_sheet.h"

// *****************************************************************************************************
CSkySheet::CSkySheet()
{
	AnimLengthInSeconds = 3; // by default, anim last 3 secconds (over 90 frames)
	Type = SKY;
	WaterEnvMapCameraHeight = 0.f;
	WaterEnvMapAlpha = 255;
	SunClipZ = -1.0f;
}

// *****************************************************************************************************
void CSkySheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	item.getValueByName(InstanceGroupName, (prefix + "InstanceGroupName").c_str());
	item.getValueByName(AnimationName, (prefix + "AnimationName").c_str());
	item.getValueByName(AnimLengthInSeconds, (prefix + "AnimLengthInSeconds").c_str());
	const NLGEORGES::UFormElm *elm = NULL;
	if(item.getNodeByName (&elm, "Objects") && elm)
	{
		uint numObjects;
		nlverify (elm->getArraySize (numObjects));
		Objects.resize(numObjects);
		// For each sky object
		for(uint k = 0; k < numObjects; ++k)
		{
			const NLGEORGES::UFormElm *objectForm;
			if (elm->getArrayNode (&objectForm, k) && objectForm)
			{
				Objects[k].build(*objectForm, "");
			}
		}
	}
	item.getValueByName(AmbientSunLightBitmap, (prefix + "AmbientSunLightBitmap").c_str());
	item.getValueByName(DiffuseSunLightBitmap, (prefix + "DiffuseSunLightBitmap").c_str());
	item.getValueByName(FogColorBitmap, (prefix + "FogColorBitmap").c_str());
	item.getValueByName(WaterEnvMapCameraHeight, (prefix + "WaterEnvMapCameraHeight").c_str());
	item.getValueByName(WaterEnvMapAlpha, (prefix + "WaterEnvMapAlpha").c_str());
	item.getValueByName(SunSource, (prefix + "SunSource").c_str());
	item.getValueByName(SunClipZ, (prefix + "SunClipZ").c_str());
}

// *****************************************************************************************************
void CSkySheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(InstanceGroupName);
	f.serial(AnimationName);
	f.serial(AnimLengthInSeconds);
	f.serialCont(Objects);
	f.serial(AmbientSunLightBitmap);
	f.serial(DiffuseSunLightBitmap);
	f.serial(FogColorBitmap);
	f.serial(WaterEnvMapCameraHeight);
	f.serial(WaterEnvMapAlpha);
	f.serial(SunSource);
	f.serial(SunClipZ);
}

// *****************************************************************************************************
void CSkySheet::build(const NLGEORGES::UFormElm &item)
{
	build(item, "");
}
