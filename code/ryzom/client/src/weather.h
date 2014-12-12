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



#ifndef CL_WORLD_LIGHT_CYCLE_H
#define CL_WORLD_LIGHT_CYCLE_H


#include <nel/misc/sheet_id.h>
#include "game_share/season.h"

struct CLightCycleDesc;
struct CLightCycle;
class  CLightCycleManager;
class  CWeatherManagerClient;
class  CWeatherFunctionParamsSheet;

// release memory
void releaseWeather();

// Description of light cycle in ryzom (for each season) , should be initialized from a georges sheet
extern CLightCycle		WorldLightCycle;
/** This setup light of objects in the world depending on the hour. It has a light cycle descriptor which must be setuped at each season change (see buildLightCycleDesc).
  * It may interract with a weather manager to know when to override lighting of the scene (in case of bad weather).
  */
extern CLightCycleManager LightCycleManager;
// This manage precipitations and blend weather setups. This allow to know the current weather (if fog or light must be overriden)
extern CWeatherManagerClient    WeatherManager;
// The last season computed from computeCurrSeason, updated in the main loop / in teleport. Update code based on season should use this value
extern EGSPD::CSeason::TSeason   CurrSeason;
// The startup season, updated at init and when a new continent is selected
extern EGSPD::CSeason::TSeason   StartupSeason;
// Parameter for weather function generation
extern CWeatherFunctionParamsSheet  *WeatherFunctionParams;

// load the world light cycle from a sheet
void loadWorldLightCycle(NLMISC::CSheetId lightCycleSheet);

// load the weather function params
void loadWeatherFunctionParams();

// Tool fct : build a CSeasonLightCycle for the given season
void buildLightCycleDesc(CLightCycleDesc &dest, EGSPD::CSeason::TSeason season);

// Small update of the weather manager
void updateWeatherManager(class CContinent *continent);

// Full update of the weather manager
void updateWeatherManager(const NLMISC::CMatrix &camMatrix, CContinent *continent);

// Some extern value
extern float			ManualWeatherValue;
extern EGSPD::CSeason::TSeason	ManualSeasonValue;
extern bool				ForceTrueWeatherValue;
extern float			DayNightCycleHour;
extern float			ForcedDayNightCycleHour;

class CWeatherFunctionParamsSheetBase;
class CWeatherFunction;


// update server driven weather value
void updateDBDrivenWeatherValue();

float getBlendedWeather(uint64 day,
						float hour,
						const CWeatherFunctionParamsSheetBase &wfp,
						const CWeatherFunction wf[EGSPD::CSeason::Invalid]
					   );

EGSPD::CSeason::TSeason computeCurrSeason();

#endif

