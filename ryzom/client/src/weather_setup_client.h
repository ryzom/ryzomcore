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



#ifndef CL_WEATHER_SETUP
#define CL_WEATHER_SETUP

#include "nel/misc/rgba.h"
#include "client_sheets/weather_setup_sheet.h"
#include "nel/misc/string_mapper.h"
#include "game_share/fog_type.h"
#include "game_share/time_weather_season/weather_setup.h"
#include <vector>

namespace NLGEORGES
{
	class UFormElm;
}


class CPrecipitation;


/** Weather state client part.
  * This includes pointer on precipitation fxs, that are not needed on server side
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CWeatherStateClient
{
public:
	// struct describing a fx and its intensity
	struct CFXDesc
	{
		float		   Ratio;
		CPrecipitation *Precipitation; // the precipitation object
	};
	// FX
	std::vector<CFXDesc> FXs;
	// create vector of fx by using precipitation in the map
	void setup(const CWeatherState &ws, std::map<std::string, CPrecipitation> &precipitationMap);
	/** Blend 2 weather state together
	  * Blend factor is clamped to [0, 1]
	  */
	static void          blend(CWeatherStateClient &dest, const CWeatherStateClient &s1, const CWeatherStateClient &s2, float blendFactor);
};

class CWeatherSetupClient : public CWeatherSetup
{
public:
	CWeatherStateClient WeatherStateClient;
};



#endif



