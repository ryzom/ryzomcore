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
#include "light_cycle_manager.h"
#include "weather_manager_client.h"
#include "game_share/light_cycle.h"
#include "game_share/time_weather_season/weather_predict.h"
#include "precipitation.h"
#include "sheet_manager.h"
#include "client_cfg.h"
#include "continent.h"
#include "time_client.h"
#include "interface_v3/interface_manager.h"
//
#include "nel/misc/sheet_id.h"
#include "nel/3d/u_particle_system_instance.h"
//
#include "client_sheets/weather_function_params_sheet.h"
//
#include "r2/editor.h"


H_AUTO_DECL(RZ_Weather)

CLightCycle		   WorldLightCycle;
CLightCycleManager LightCycleManager;
CWeatherManagerClient    WeatherManager;
CWeatherFunctionParamsSheet  *WeatherFunctionParams = NULL;

float               ManualWeatherValue = 0.f;
EGSPD::CSeason::TSeason	ManualSeasonValue = EGSPD::CSeason::Spring;
bool                ForceTrueWeatherValue = false;
float				DayNightCycleHour = 12.f;
float				ForcedDayNightCycleHour = -1.0f;

EGSPD::CSeason::TSeason CurrSeason = EGSPD::CSeason::Invalid;
EGSPD::CSeason::TSeason StartupSeason = EGSPD::CSeason::Invalid;

extern NLPACS::UGlobalRetriever	*GR;

using namespace NLMISC;

void releaseWeather()
{
	if( WeatherFunctionParams )
		delete WeatherFunctionParams;
	WeatherFunctionParams = NULL;
}

void buildLightCycleDesc(CLightCycleDesc &dest,EGSPD::CSeason::TSeason season)
{
	if (season >= EGSPD::CSeason::Invalid)
	{
		nlwarning("Invalid season");
		return;
	}
	dest.NumHours = WorldLightCycle.NumHours;
	dest.RealDayLength = WorldLightCycle.RealDayLength;
	dest.MaxNumColorSteps = WorldLightCycle.MaxNumColorSteps;
	const CSeasonLightCycle &slc = WorldLightCycle.SeasonLightCycle[season];
	dest.NightTransitionStartHour = slc.DayToDuskHour;
	dest.NightTransitionEndHour = slc.NightHour;
	dest.DawnTransitionStartHour = slc.NightToDayHour;
	dest.DawnTransitionEndHour = slc.DayHour;

	float intervalDayNight = slc.NightHour >= slc.DayToDuskHour ? slc.NightHour - slc.DayToDuskHour : WorldLightCycle.NumHours - slc.DayToDuskHour + slc.NightHour;
	float intervalDayDusk = slc.DuskToNightHour >= slc.DayToDuskHour ? slc.DuskToNightHour - slc.DayToDuskHour : WorldLightCycle.NumHours - slc.DayToDuskHour + slc.DuskToNightHour;

	if (intervalDayDusk != 0.f) dest.DuskRatio = intervalDayDusk / intervalDayNight;
	else dest.DuskRatio = 0.5f;
}

void loadWorldLightCycle(CSheetId lightCycle)
{
	nldebug("Load light cycle '%s'", lightCycle.toString().c_str());

	if (lightCycle == CSheetId::Unknown)
		lightCycle = CSheetId("ryzom.light_cycle");

	CEntitySheet *sheet = SheetMngr.get(lightCycle);
	nlassert(sheet);
	if (sheet->type() != CEntitySheet::LIGHT_CYCLE)
	{
		nlwarning("Bad type for light cycle sheet");
		return;
	}
	CLightCycleSheet *lightCycleSheet = (CLightCycleSheet *) sheet;
	WorldLightCycle = lightCycleSheet->LightCycle;
}

void loadWeatherFunctionParams()
{
	if (WeatherFunctionParams == NULL) WeatherFunctionParams = new CWeatherFunctionParamsSheet;

	if (ClientCfg.ManualWeatherSetup)
	{
		WeatherFunctionParams->CWeatherFunctionParamsSheetBase::build("ryzom.weather_function_params");
	}
	else
	{
		// Now, just get it from the sheet manager
		std::vector <CSheetId> result;
		CSheetId::buildIdVector(result, "weather_function_params");
		if (result.empty())
		{
			nlwarning("Couldn't load weather_function_params");
			return;
		}
		if (result.size() != 1)
		{
			nlwarning("Multiple .weather_function_params sheets available, taking first");
		}
		CEntitySheet *sheet = SheetMngr.get(result[0]);
		if (sheet->type() != CEntitySheet::WEATHER_FUNCTION_PARAMS)
		{
			nlwarning("Bad type for weather_function_params sheet");
			return;
		}
		WeatherFunctionParams = (CWeatherFunctionParamsSheet *) sheet;
	}
}

// ***************************************************************************

void updateWeatherManager(CContinent *continent)
{
	H_AUTO_USE(RZ_Weather)
	// build a weather context
	CWeatherContext wc;
	wc.GR   = NULL;
	wc.LC   = &WorldLightCycle;
	wc.WF	= continent->WeatherFunction;

	// Update the weather manager
	// NB nico : even when light hour is forced, we use the real time to compute the weather (so that the weather will reflect the real one, even if hour & light is fixed)
	if(ClientCfg.ManualWeatherSetup && !ForceTrueWeatherValue)
	{
		WeatherManager.manualUpdate(RT.getRyzomDay(), (float) RT.getRyzomTime(), wc, ManualWeatherValue, CurrSeason);
	}
	else
	{
		WeatherManager.update(RT.getRyzomDay(), (float) RT.getRyzomTime(), wc);
	}
}

// ***************************************************************************

void updateWeatherManager(const NLMISC::CMatrix &camMatrix, CContinent *continent)
{
	H_AUTO_USE(RZ_Weather)
	// build a weather context
	CWeatherContext wc;
	wc.GR   = GR;
	if(continent)
		wc.WF = continent->WeatherFunction;
	else
		wc.WF = NULL;

	if (continent)
	{
		if(ClientCfg.ManualWeatherSetup && !ForceTrueWeatherValue)
		{
			WeatherManager.manualUpdate(RT.getRyzomDay(), (float) RT.getRyzomTime(), wc, ManualWeatherValue, CurrSeason, camMatrix, *continent);
		}
		else
		{
			// NB nico : even when light hour is forced, we use the real time to compute the weather (so that the weather will reflect the real one, even if hour & light is fixed)
			WeatherManager.update(RT.getRyzomDay(), RT.getRyzomTime(), wc, camMatrix, *continent);
		}
	}
}


///////////////////
// WEATHER VALUE //
///////////////////


float LocalServerWeather = 0.f;       // 'local' server driven weather, trying to reach the weather value given in the database as time passes (for smooth transition)
float ServerWeatherBlendFactor = 0.f; // blend factor between server driven weather & local computed pseudo random weather (based on the current date, gives same result for all clients)
bool  ServerDrivenWeather = false;

const float WEATHER_BLEND_SPEED = 1.f / 8.f; // number of seconds to blend betwen weather states

static NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> s_ServerWeatherValueDB;

// ***************************************************************************
static uint16 getServerWeather()
{
	CCDBNodeLeaf *node = s_ServerWeatherValueDB ? &*s_ServerWeatherValueDB
		: &*(s_ServerWeatherValueDB = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:WEATHER:VALUE"));
	if (!node) return 0;
	return (uint16) node->getValue16();
}

// ***************************************************************************
void updateDBDrivenWeatherValue()
{
	uint16 dbWeather = getServerWeather();
	if (dbWeather == 0)
	{
		// not server driven
		ServerDrivenWeather = false;
		NLMISC::incrementalBlend(ServerWeatherBlendFactor, 0.f, WEATHER_BLEND_SPEED * DT);
	}
	else
	{
		float targetWeather = (dbWeather - 1) / 1022.f;
		if (!ServerDrivenWeather)
		{
			// just switched to server driven weather ?
			LocalServerWeather = targetWeather;
			ServerDrivenWeather = true;
		}
		// slowly blend from predicted weather to server driven weather
		NLMISC::incrementalBlend(ServerWeatherBlendFactor, 1.f, WEATHER_BLEND_SPEED * DT);

		// slowly blend between server driven weathers as values in the db changes
		NLMISC::incrementalBlend(LocalServerWeather, targetWeather, WEATHER_BLEND_SPEED * DT);

	}
}

// ***************************************************************************
float getBlendedWeather(uint64 day,
						float hour,
						const CWeatherFunctionParamsSheetBase &wfp,
						const CWeatherFunction wf[EGSPD::CSeason::Invalid]
						)
{
	return NLMISC::blend(CPredictWeather::predictWeather(day, hour, wfp, wf), LocalServerWeather, ServerWeatherBlendFactor);
}

uint8 getSeasonDBValue()
{
	CCDBNodeLeaf *serverSeason = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:SEASON:VALUE");
	return serverSeason ? serverSeason->getValue8() : 0;
}

////////////
// SEASON //
////////////

uint8 ServerSeasonValue = 0;
bool ServerSeasonReceived = false;
static bool WaitingServerSeason = false;
static R2::CEditor::TMode OldR2EDMode = R2::CEditor::NotInitialized;
static EGSPD::CSeason::TSeason LastSeason = EGSPD::CSeason::Unknown;

// ***************************************************************************
EGSPD::CSeason::TSeason computeCurrSeason()
{
	// Note : when season isn't imposed by the server or by the editor, we use
	// the season found in StartupSeason
	// This mean that when the season change due to time, it won't be updated on the client
	// This is a deliberate choice to avoid possibly disadvantaging freeze on the client while in combat
	// To have this done correctly, the server should be aware of the change, and freeze action accordingly.
	// When entering a new area, the 'StartupSeason' variable is updated however, because the player is safe at that time.
	// (see Continent::select)

	if (!ClientCfg.R2EDEnabled || !ClientCfg.Local) // ... Standard client
	{
		// standard case
		if (ServerSeasonValue != 0)
		{
			ServerSeasonReceived = false;
			// server driven season
			LastSeason = (EGSPD::CSeason::TSeason) ((ServerSeasonValue - 1) & 3);
		}
		else
		{
			LastSeason = ClientCfg.ManualWeatherSetup ? ManualSeasonValue : StartupSeason;
		}
		return LastSeason;
	}

	// When we change act( different location) the Server TP the player at the good location
	// So the manual setting of season is useless (R2::getEditor().getSeason()) because
	// there is a huge risk to (Change to 2 times season)
	// eg Act1 Season1, Island1, Act2 Season2, Island2
	// If server tp occurs before, client received change of Season: there will be a unecessary TP



	// When in edition mode, the editor tell what is the season (it is local and given
	// by each act of the current scenario, so ignore server value here...)
	if (R2::getEditor().getMode() == R2::CEditor::EditionMode)
	{
		OldR2EDMode = R2::getEditor().getMode();
		switch(R2::getEditor().getSeason())
		{
			case R2::CEditor::Automatic: LastSeason = StartupSeason; break;
			case R2::CEditor::Spring:	 LastSeason = EGSPD::CSeason::Spring; break;
			case R2::CEditor::Summer:	 LastSeason = EGSPD::CSeason::Summer; break;
			case R2::CEditor::Autumn:	 LastSeason = EGSPD::CSeason::Autumn; break;
			case R2::CEditor::Winter:	 LastSeason = EGSPD::CSeason::Winter; break;
			case R2::CEditor::UnknownSeason:
				if (LastSeason == EGSPD::CSeason::Unknown)
				{
					// don't change the season unless this is the first call
					LastSeason = StartupSeason; // default
				}
			break;
			default:
				nlassert(0);
			break;
		}
	}
	// Leaving edit -> Test 'mode' or GoingToTest 'mode' :
	if (OldR2EDMode == R2::CEditor::EditionMode)
	{
		// a transition from edition mode to test mode has been detected :
		// At some point the server will impulse the current season, be it the same than the current one
		// Until this occur, we just retain the last season that was set in the editor, because
		// season change are VERY costly (several seconds freeze) and we don't want the season to change twice in a row just because we don't
		// know its real value at this point.
		WaitingServerSeason = true;
	}
	OldR2EDMode = R2::getEditor().getMode();
	if (WaitingServerSeason && !ServerSeasonReceived)
	{
		return LastSeason;
	}
	ServerSeasonReceived = false;
	WaitingServerSeason = false;
	// back to standard case
	// At this point we ignore the 'ClientCfg.ManualWeatherSetup' flag, however, because it may be set
	// when the "change weather" dialog is shown, but we only want to preview weather change, not the season change.
	if (ServerSeasonValue != 0)
	{
		// server driven season
		LastSeason = (EGSPD::CSeason::TSeason) ((ServerSeasonValue - 1) & 3);
	}
	else
	{
		LastSeason = StartupSeason;
	}
	return LastSeason;
}
