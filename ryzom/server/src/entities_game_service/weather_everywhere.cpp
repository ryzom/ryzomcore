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
#include "weather_everywhere.h"
#include "nel/misc/sheet_id.h"
#include "nel/georges/load_form.h"
#include "nel/georges/u_form_elm.h"
#include "zone_manager.h"
#include "egs_sheets/egs_sheets.h"
#include "game_share/time_weather_season/weather_manager.h"
#include "game_share/time_weather_season/weather_setup_sheet_base.h"
#include "game_share/time_weather_season/weather_function_params_sheet_base.h"
#include "game_share/time_weather_season/weather_predict.h"
#include "game_share/season.h"

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;


/// Singleton instance
CWeatherEverywhere	WeatherEverywhere;

/// Number of possible weather setups
const uint NB_WEATHER_SETUPS = 6;

/// Mapping from weather setups to EWeather value
const CRyzomTime::EWeather WeatherSetupsToValue [NB_WEATHER_SETUPS] =
{
	CRyzomTime::best,
	CRyzomTime::good, CRyzomTime::good,
	CRyzomTime::bad, CRyzomTime::bad,
	CRyzomTime::worst
};


/*
 * Initialization
 */
void CWeatherEverywhere::init()
{
	// Init weather manager (with weather setup sheets)
	std::vector<CSheetId> sheetVec;
	CSheetId::buildIdVector( sheetVec, "weather_setup" ); // list weather_setup sheets
	std::vector<const CWeatherSetupSheetBase *> weatherSheets;
	std::vector<std::string> weatherSheetNames;
	for ( uint k=0; k!=sheetVec.size(); ++k )
	{
		const CWeatherSetupSheetBase *sheet = CSheets::getWeatherSetupSheet( sheetVec[k] ); // get static sheet
		if ( sheet )
		{		
			weatherSheetNames.push_back( sheetVec[k].toString() );
			weatherSheets.push_back( sheet );
		}
	}
	CWeatherManager weatherManager;
	weatherManager.init( weatherSheets, weatherSheetNames );

	// Load the continents (george forms). The sorted list is found in CStaticWorld.
	sheetVec.clear();
	const CStaticWorld *staticWorld = CSheets::getWorldForm( CSheetId("ryzom.world") );
	nlassert( staticWorld );
	nlassertex( staticWorld->Continents.size() == CONTINENT::NB_CONTINENTS, ("ryzom.world should contain exactly the same number of continents as the enum size CONTINENT::TContinent (=%u), not %u", (uint)(CONTINENT::NB_CONTINENTS), staticWorld->Continents.size()) ); 
	UFormLoader *formLoader = UFormLoader::createLoader();
	_WeatherFunctionsBySeasonByContinent.resize( staticWorld->Continents.size() );
	for ( uint iContinent=0; iContinent!=CONTINENT::NB_CONTINENTS; ++iContinent )
	{
		_WeatherFunctionsBySeasonByContinent[iContinent] = new CWeatherFunction [EGSPD::CSeason::Invalid];
		CSmartPtr<UForm> continentForm = formLoader->loadForm( staticWorld->Continents[iContinent].toString().c_str() ); // load continent form
		if ( continentForm == NULL )
		{
			nlwarning( "Continent sheet '%s' not found", staticWorld->Continents[iContinent].toString().c_str() );
		}
		else
		{
			for ( uint iSeason=0; iSeason!=EGSPD::CSeason::Invalid; ++iSeason )
			{
				string seasonStr = EGSPD::CSeason::toString( (EGSPD::CSeason::TSeason) iSeason);
				const UFormElm *seasonWFNode = NULL;
				if ( continentForm->getRootNode().getNodeByName( &seasonWFNode, (seasonStr + string("WeatherFunction")).c_str() ) && seasonWFNode )
				{
					CWeatherFunctionSheet wfs;
					wfs.build( *seasonWFNode );
					_WeatherFunctionsBySeasonByContinent[iContinent][iSeason].buildFromSheet( wfs, weatherManager );
				}
				else
				{
					// Not a warning because indoor continent does not have any weather function
					nlinfo( "%s has no node %s", staticWorld->Continents[iContinent].toString().c_str(), (seasonStr + string("WeatherFunction")).c_str() );
				}
			}
		}
	}
	UFormLoader::releaseLoader(formLoader);
	
	// Use always first weather function param sheet
	if ( CSheets::getWeatherFunctionParamsSheets().begin() != CSheets::getWeatherFunctionParamsSheets().end() )
	{
		_WeatherFunctionParamsSheet = &(*CSheets::getWeatherFunctionParamsSheets().begin()).second;
	}
}


/*
 * Destructor
 */
CWeatherEverywhere::~CWeatherEverywhere()
{
	for ( uint i=0; i!=_WeatherFunctionsBySeasonByContinent.size(); ++i )
	{
		delete [] _WeatherFunctionsBySeasonByContinent[i];
	}
}


/*
 * Return the weather at the specified position & time.
 * In case of failure (such as a position outside a continent), return unknownWeather.
 */
CRyzomTime::EWeather CWeatherEverywhere::getWeather( const NLMISC::CVector& pos, const CRyzomTime& ryzomTime ) const
{
	if ( ! _WeatherFunctionParamsSheet )
		return CRyzomTime::unknownWeather;

	// Get continent of which the position belongs
	CContinent *continent = CZoneManager::getInstance().getContinent( pos );
	if ( ! continent )
		return CRyzomTime::unknownWeather;

	// Predict weather
	float weatherFloatValue = CPredictWeather::predictWeather(
		ryzomTime.getRyzomDay(),
		ryzomTime.getRyzomTime(),
		*_WeatherFunctionParamsSheet,
		_WeatherFunctionsBySeasonByContinent[continent->getId()] );

	// Map from weather setup to EWeather value
	uint weatherSetupIndex = min( (uint)(weatherFloatValue*((float)NB_WEATHER_SETUPS)), NB_WEATHER_SETUPS-1 );
	//nldebug( "Weather setup of position is %u", weatherSetupIndex );
	return WeatherSetupsToValue[weatherSetupIndex];
}
