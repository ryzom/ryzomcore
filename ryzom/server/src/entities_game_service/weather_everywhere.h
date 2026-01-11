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



#ifndef NL_WEATHER_EVERYWHERE_H
#define NL_WEATHER_EVERYWHERE_H

#include "game_share/time_weather_season/weather_function.h"
#include "game_share/time_weather_season/time_and_season.h"


/**
 * Utility class to query the weather at any time and position.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2004
 */
class CWeatherEverywhere
{
public:

	/// Constructor
	CWeatherEverywhere() : _WeatherFunctionParamsSheet(NULL) {}

	/// Destructor
	~CWeatherEverywhere();

	/// Initialization (must be called after the initialization of static sheets)
	void						init();

	/**
	 * Return the weather at the specified position & time.
	 * In case of failure (such as a position outside a continent), return unknownWeather.
	 */
	CRyzomTime::EWeather		getWeather( const NLMISC::CVector& pos, const CRyzomTime& ryzomTime ) const;

private:

	/// All weather functions (the vector contains a pointer to EGSPD::CSeason::Invalid values)
	std::vector< CWeatherFunction * >				_WeatherFunctionsBySeasonByContinent;

	/// Pointer to the weather function parameter sheet
	const class CWeatherFunctionParamsSheetBase		*_WeatherFunctionParamsSheet;
};


/// Singleton instance
extern CWeatherEverywhere	WeatherEverywhere;


#endif // NL_WEATHER_EVERYWHERE_H

/* End of weather_everywhere.h */
