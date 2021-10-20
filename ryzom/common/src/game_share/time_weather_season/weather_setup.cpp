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
#include "weather_setup.h"
#include "nel/georges/u_form_elm.h"

using namespace NLMISC;


//==================================================================================
CWeatherState::CWeatherState()
{
}

//==================================================================================
/** Tool fct to blend a color
  */
static inline void blendValue(NLMISC::CRGBA &dest, NLMISC::CRGBA c1, NLMISC::CRGBA c2, float blendFactor)
{
	dest.blendFromui(c1, c2, (uint) (blendFactor * 256.f));
}

//==================================================================================
/** Tool fct to blend 2 floats
  */
static inline void blendValue(float &dest, float f1, float f2, float blendFactor)
{
	dest = blendFactor * f2 + (1.f - blendFactor) * f1;
}

//==================================================================================
/** Tool fct to blend 2 uint32
  */
static inline void blendValue(uint32 &dest, uint32 u1, uint32 u2, float blendFactor)
{
	dest = (uint32) (blendFactor * u2 + (1.f - blendFactor) * u1);
}

//==================================================================================
void CWeatherState::blend(CWeatherState &dest,const CWeatherState &s1,const CWeatherState &s2, float blendFactor)
{
	dest.BlendFactor = blendFactor;
	NLMISC::clamp(blendFactor, 0.f, 1.f);
	dest.FirstSetupName = s1.BestSetupName;
	dest.SecondSetupName = s2.BestSetupName;

	// Best aproximated setup name
	// NB nico : changed this because precipitation starts as soon as a setup starts to blend, so display works better
	// when using localised name of the next setup.
	if (blendFactor == 0.f)
	{
		dest.BestSetupName = s1.BestSetupName;
		dest.LocalizedName = s1.LocalizedName;
	}
	else
	{
		dest.BestSetupName = s2.BestSetupName;
		dest.LocalizedName = s2.LocalizedName;
	}


	// Fog
	blendValue(dest.FogRatio, s1.FogRatio, s2.FogRatio, blendFactor);
	blendValue(dest.FogColorDay, s1.FogColorDay, s2.FogColorDay, blendFactor);
	blendValue(dest.FogColorDusk, s1.FogColorDusk, s2.FogColorDusk, blendFactor);
	blendValue(dest.FogColorNight, s1.FogColorNight, s2.FogColorNight, blendFactor);
	blendValue(dest.FogGradientFactor, s1.FogGradientFactor, s2.FogGradientFactor, blendFactor);
	uint k;
	for(k = 0; k < NumFogType; ++k)
	{
		blendValue(dest.FogNear[k],  s1.FogNear[k],  s2.FogNear[k],  blendFactor);
		blendValue(dest.FogFar[k],   s1.FogFar[k],   s2.FogFar[k],   blendFactor);
	}
	// Lighting
	blendValue(dest.Lighting, s1.Lighting, s2.Lighting, blendFactor);
	// BackGround
	dest.DayBackgroundFileName1 = s1.DayBackground;
	dest.DuskBackgroundFileName1 = s1.DuskBackground;
	dest.NightBackgroundFileName1 = s1.NightBackground;
	dest.DayBackgroundFileName2 = s2.DayBackground;
	dest.DuskBackgroundFileName2 = s2.DuskBackground;
	dest.NightBackgroundFileName2 = s2.NightBackground;
	// Wind intensity
	blendValue(dest.WindIntensity, s1.WindIntensity, s2.WindIntensity, blendFactor);
	// Thunder color. NB : the thunder intensity is blended separatly
	blendValue(dest.ThunderColor, s1.ThunderColor, s2.ThunderColor, blendFactor);
}



//==================================================================================
void CCloudState::blend(CCloudState &dest, const CCloudState &s1, const CCloudState &s2, float blendFactor)
{
	NLMISC::clamp(blendFactor, 0.f, 1.f);
	blendValue(dest.AmbientDay, s1.AmbientDay, s2.AmbientDay, blendFactor);
	blendValue(dest.DiffuseDay, s1.DiffuseDay, s2.DiffuseDay, blendFactor);
	blendValue(dest.AmbientNight, s1.AmbientNight, s2.AmbientNight, blendFactor);
	blendValue(dest.DiffuseNight, s1.DiffuseNight, s2.DiffuseNight, blendFactor);
	blendValue(dest.AmbientDusk, s1.AmbientDusk, s2.AmbientDusk, blendFactor);
	blendValue(dest.DiffuseDusk, s1.DiffuseDusk, s2.DiffuseDusk, blendFactor);
	blendValue(dest.DiffusionSpeed, s1.DiffusionSpeed, s2.DiffusionSpeed, blendFactor);
	blendValue(dest.NumClouds, s1.NumClouds, s2.NumClouds, blendFactor);
}
