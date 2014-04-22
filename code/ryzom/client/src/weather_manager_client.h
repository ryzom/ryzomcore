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





#ifndef CL_WEATHER_MANAGER_CLIENT_H
#define CL_WEATHER_MANAGER_CLIENT_H

#include "game_share/time_weather_season/weather_setup.h"
#include "game_share/time_weather_season/weather_function.h"
#include "game_share/season.h"
#include "game_share/time_weather_season/weather_manager.h"
#include "weather_setup_client.h"
#include "nel/misc/vector.h"
#include <map>
#include <list>

namespace NLMISC
{
	class CMatrix;
}
namespace NL3D
{
	class UScene;
	class UDriver;
}
namespace NLPACS
{
	class UGlobalRetriever;
}

class CWeatherFunctionParamsSheet;
struct CLightCycle;

// a context to pass to the weather manager methods
struct CWeatherContext
{
	CWeatherContext ();
	const CWeatherFunction       *WF;
	const CWeatherFunctionParamsSheet *WFP;
	const CLightCycle            *LC;
	NLPACS::UGlobalRetriever     *GR;
};

class CWeatherManagerClient : public CWeatherManager
{
public:
	// ctor
	CWeatherManagerClient();
	//
	void init();
	/// From CWeatherManager : Loads all the weather setups
	virtual void         init(const std::vector<const CWeatherSetupSheetBase *> &sheets, const std::vector<std::string> &sheetNames);
	/// From CWeatherManager : Release all datas (fx models..). Should be called before deleting the scene
	virtual void		 release();
	/// Set the global direction of the wind. Only XY direction is kept
	void				 setWindDir(const NLMISC::CVector &dir);
	/// Update the weather, small update, only to update current weather state
	void				 update(uint64 day, float hour, const CWeatherContext &wc);
	/// Update the weather, full update, with 3d FX thunder update
	void				 update(uint64 day, float hour, const CWeatherContext &wc, const NLMISC::CMatrix &camMat, const class CContinent &continent);
	/// Get the current weather value. Updated after each call to 'update'
	float				 getWeatherValue() const { return _WeatherValue; }
	/** Does the same than 'update', but let the user choose the weather value.	The weather value ranges from 0 to 1.
	  * The day and hour are needed only to manage phenomena like thunder (need a clock to know when there are thunder strikes)
	  * Small update, only to update current weather state
	  */
	void                 manualUpdate(uint64 day, float hour, const CWeatherContext &wc, float weatherValue, EGSPD::CSeason::TSeason season);
	/** Does the same than 'update', but let the user choose the weather value.	The weather value ranges from 0 to 1.
	  * The day and hour are needed only to manage phenomena like thunder (need a clock to know when there are thunder strikes)
	  * Full update, with 3d FX thunder update
	  */
	void                 manualUpdate(uint64 day, float hour, const CWeatherContext &wc, float weatherValue, EGSPD::CSeason::TSeason season, const NLMISC::CMatrix &camMat, const class CContinent &continent);
	// get the current weather setup. It is updated at each call to 'update'
	const CWeatherState  &getCurrWeatherState() const { return _CurrWeatherState; }
	// Compute the state of clouds at the given date.
	void				 computeCloudState(uint64 day, float hour, const CWeatherContext &wc, CCloudState &dest) const;
	// Compute the cloud state for the given weather value.
	void				 computeCloudState(float weatherValue, EGSPD::CSeason::TSeason season, CCloudState &dest, const CWeatherFunction wf[EGSPD::CSeason::Invalid]) const;
	// For debug : draw the clip grids of currently active precipitations
	void				 drawPrecipitationClipGrids(NL3D::UDriver &drv) const;
	// Get the 'thunder level', that is how bright the thunder lighting is. It ranges from 0 (no thunder) to 1 (thunder impact)
	float				 getThunderLevel() const { return _ThunderLevel; }
	// To avoid rain in town, There's a Noprecipitation map that give a factor to stop rain at some places
	float				getLocalPrecipitationFactor() const { return _LocalPrecipitationFactor; }
/////////////////////////////////////////////////////////////////
protected:
	// from CWeatherManager
	virtual CWeatherSetup *newWeatherSetup() const;
	// from CWeatherManager
	virtual void setupLoaded(CWeatherSetup *setup);
private:
	typedef std::map<std::string, CPrecipitation> TPrecipitationMap;
	// A vector of precipitation pointers
	typedef std::vector<CPrecipitation *> TPrecipitationVect;
private:
	CWeatherState				  _CurrWeatherState;
	CWeatherStateClient			  _CurrWeatherStateClient;
	TPrecipitationMap			  _PrecipitationMap;
	TPrecipitationVect			  _ActivePrecipitations;
	TPrecipitationVect			  _WaitingPrecipitations;
	NLMISC::CVector				  _WindDir;
	float						  _WeatherValue;
	float						  _ThunderLevel;
	bool                          _ThunderStrike; // true if a thunder strike is currently happening
	float						  _LastEvalHour;
	uint64                        _LastEvalDay;
	float						  _LocalPrecipitationFactor;
private:
	void	initPrecipitationFXs();
	void    setupFXs(const NLMISC::CMatrix &camMat, NLPACS::UGlobalRetriever *gr, const class CContinent &continent);
	void    setupWind(const CWeatherFunction *wf);
public:
	/** A time measurment for thunder function. Each cycle of thunder last a period like 0.5 s. One thunder
	  * strike can happen only each 2 cycles
	  * PRIVATE use by implementation
	  */
	struct CThunderTimeMeasure
	{
		uint64 Cycle;
		float  SubCycle;
		CThunderTimeMeasure() : Cycle(~0), SubCycle(0)
		{}
	};
private:
	CThunderTimeMeasure _ThunderStrikeDate;
private:
	// Use the curr weather state to update the thunder.
	void    updateThunder(uint64 day, float hour, const CWeatherContext &wc, bool manual = false, float manualWeatherValue = 0.f, EGSPD::CSeason::TSeason manualSeason = EGSPD::CSeason::Spring);
	// The thunder intensity at the given day and hour (value of the 'ThunderIntensity' field in the weather setups)
	float   getThunderIntensity(uint64 day, float hour, const CWeatherContext &wc);
	// Compute the minimum threshold that must be reached to create a thunder strike
	float   getThunderThreshold(uint64 thunderCycle, const CWeatherContext &wc);
	// update the thunder state between 2 times between which the thunder function is linear
	bool    updateThunderState(CThunderTimeMeasure &t0, CThunderTimeMeasure &t1, float thunderThreshold);
};

#endif
