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




#ifndef RY_WEATHER_PREDICT_H
#define RY_WEATHER_PREDICT_H

#include "../season.h"

class CWeatherFunction;
struct CLightCycle;

namespace NLGEORGES
{
	class UFormElm;
}

class CWeatherFunctionParamsSheetBase;

// utility class to predict weather
struct CPredictWeather
{
	enum EWeatherCycleType { HighPressure = 0, LowPressure, SeasonTransition, WeatherCycleCount };

	/** Get the value for weather at the given date.
	  * wf is an array with 4 weather functions: one for each season
	  * They gives intervals in which a random offset can be taken and added to the weather value.
	  * If a NULL pointer is provided, no offset is added.
	  * See season.h in game_share for season ordering
	  * \param ls Decription of light cycle for each season. It is used to avoid weather transition when night is falling or at the start of the day
	  */
	static float predictWeather(uint64 day,
						        float hour,
						        const CWeatherFunctionParamsSheetBase &wfp,
								const CWeatherFunction wf[EGSPD::CSeason::Invalid]
						       );
private:
	static float getCycleWeatherValue(uint64 cycle, const CWeatherFunction &wf);
};





#endif


