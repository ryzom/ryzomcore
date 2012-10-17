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
#include "weather_setup_client.h"
#include "precipitation.h"
#include "nel/georges/u_form_elm.h"
#include "nel/3d/u_particle_system_instance.h"

using namespace NLMISC;


H_AUTO_DECL(RZ_WeatherSetupClient)

//==================================================================================
/** Tool fct to blend 2 floats
  */
static inline void blendValue(float &dest, float f1, float f2, float blendFactor)
{
	H_AUTO_USE(RZ_WeatherSetupClient)
	dest = blendFactor * f2 + (1.f - blendFactor) * f1;
}


//==================================================================================
void CWeatherStateClient::setup(const CWeatherState &ws, std::map<std::string, CPrecipitation> &precipitationMap)
{
	FXs.resize(ws.FXInfos.size());
	for(uint k = 0; k < FXs.size(); ++k)
	{
		if (!ws.FXInfos[k].Name.empty())
		{
			FXs[k].Precipitation = &(precipitationMap[ws.FXInfos[k].Name]);
			FXs[k].Ratio = ws.FXInfos[k].Ratio;
		}
		else
		{
			FXs[k].Precipitation = NULL;
			FXs[k].Ratio = 0.f;
		}
	}
}

//==================================================================================
void CWeatherStateClient::blend(CWeatherStateClient &dest, const CWeatherStateClient &s1, const CWeatherStateClient &s2, float blendFactor)
{
	H_AUTO_USE(RZ_WeatherSetupClient)
	// FXs. Blend FXs ofthe 2 vectors togethers
	dest.FXs.resize(s1.FXs.size());
	//
	uint numCommonFXs = 0; // number of fx in array 1 that are also in array 2
	// Add / Blend FXs that are only in array 1, or in both arrays
	uint l;
	uint k;
	for(k = 0; k < s1.FXs.size(); ++k)
	{
		dest.FXs[k].Precipitation  = s1.FXs[k].Precipitation;
		bool found = false;
		// see if FX is in both vectors
		for(l = 0; l < s2.FXs.size(); ++l)
		{
			if (s1.FXs[k].Precipitation == s2.FXs[l].Precipitation)
			{
				// yes, so blend the ratios
				blendValue(dest.FXs[k].Ratio, s1.FXs[k].Ratio, s2.FXs[k].Ratio, blendFactor);
				found = true;
				++ numCommonFXs;
				break;
			}
		}
		if (!found)
		{
			// this FX is only in array 1
			blendValue(dest.FXs[k].Ratio, s1.FXs[k].Ratio, 0, blendFactor);
		}
	}

	// Add FXs that are only in array 2
	dest.FXs.reserve(dest.FXs.size() + s2.FXs.size() - numCommonFXs);
	for(k = 0; k < s2.FXs.size(); ++k)
	{
		bool inBothArrays = false;
		// see if FX is in both vectors
		for(l = 0; l < s1.FXs.size(); ++l)
		{
			if (s1.FXs[k].Precipitation == s2.FXs[l].Precipitation)
			{
				inBothArrays = true;
				break;
			}
		}
		if (!inBothArrays)
		{
			dest.FXs.push_back(s2.FXs[k]);
			// this FX is only in array 2
			blendValue(dest.FXs.back().Ratio, 0.f, s2.FXs[k].Ratio, blendFactor);
		}
	}
}
