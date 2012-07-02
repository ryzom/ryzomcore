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

//
#include "game_share/light_cycle.h"
#include "game_share/time_weather_season/weather_predict.h"
//
#include "client_sheets/weather_function_params_sheet.h"
//
#include "weather.h"
#include "weather_manager_client.h"
#include "weather_setup_client.h"
#include "precipitation.h"
#include "continent.h"
#include "sheet_manager.h"
#include "sound_manager.h"
#include "client_cfg.h"

H_AUTO_DECL(RZ_WeatherManagerClient)

using namespace NLMISC;
using namespace std;


extern NL3D::UScene *Scene;
extern NL3D::ULandscape *Landscape;

//================================================================================================
CWeatherManagerClient::CWeatherManagerClient() : _WindDir(0, 0, 0),
												 _WeatherValue(0),
												 _ThunderLevel(0),
												 _ThunderStrike(false),
												 _LastEvalHour(0),
												 _LastEvalDay(0),
												 _LocalPrecipitationFactor(0.f)
{
}


//================================================================================================
void CWeatherManagerClient::init()
{
	// Get the list of the weather setups sheets
	std::vector<CSheetId> result;
	CSheetId::buildIdVector(result, "weather_setup");
	std::vector<const CWeatherSetupSheetBase *> wsSheets;
	std::vector<std::string> wsSheetsNames;

	for(uint k = 0; k < result.size(); ++k)
	{
		CEntitySheet *sheet = SheetMngr.get(result[k]);
		if (sheet && sheet->type() == CEntitySheet::WEATHER_SETUP)
		{
			//nlwarning("Sheet name = %s, weather setup = %s", result[k].toString().c_str(), NLMISC::CStringMapper::unmap((dynamic_cast<CWeatherSetupSheetBase *>(sheet))->SetupName).c_str());
			wsSheetsNames.push_back(result[k].toString());
			wsSheets.push_back(dynamic_cast<CWeatherSetupSheetBase *>(sheet));
		}
	}
	init(wsSheets, wsSheetsNames);
}

//================================================================================================
CWeatherSetup *CWeatherManagerClient::newWeatherSetup() const
{
	// client version of weather setup
	return new CWeatherSetupClient;
}

//================================================================================================
void CWeatherManagerClient::setupLoaded(CWeatherSetup *setup)
{
	CWeatherSetupClient *wsc = NLMISC::safe_cast<CWeatherSetupClient *>(setup);
	wsc->WeatherStateClient.setup(setup->WeatherState, _PrecipitationMap);
}


//================================================================================================
void CWeatherManagerClient::init(const std::vector<const CWeatherSetupSheetBase *> &sheets, const std::vector<std::string> &sheetNames)
{
	CWeatherManager::init(sheets, sheetNames);
	initPrecipitationFXs();
}

//================================================================================================
void CWeatherManagerClient::initPrecipitationFXs()
{
	CPrecipitationDesc desc;
	for(TPrecipitationMap::iterator it = _PrecipitationMap.begin(); it != _PrecipitationMap.end(); ++it)
	{
		desc.FxName = it->first + ".ps";
		desc.ReleasableModel = true;
		desc.GridSize = 7;
		desc.UseBBoxSize = true;
		it->second.init(desc);
	}
}

//================================================================================================
/** Update a vector of precipitation with the given pos
  */
static void updatePrecipitationVect(std::vector<CPrecipitation *> &vect, const NLMISC::CMatrix &camMat, NLPACS::UGlobalRetriever *gr)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	for(std::vector<CPrecipitation *>::iterator it = vect.begin(); it != vect.end(); ++it)
	{
		(*it)->update(camMat, gr);
	}
}

//================================================================================================
void CWeatherManagerClient::update(uint64 day, float hour, const CWeatherContext &wc)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	// get the weather value for the current date
	nlassert(wc.WFP);
	float weatherValue = ::getBlendedWeather(day, hour, *(wc.WFP), wc.WF);
	// build current weather state
	EGSPD::CSeason::TSeason season = CRyzomTime::getSeasonByDay((uint32)day);
	//
	manualUpdate(day, hour, wc, weatherValue, season);
	_LastEvalHour = hour;
	_LastEvalDay  = day;
}

//================================================================================================
void CWeatherManagerClient::update(uint64 day, float hour, const CWeatherContext &wc, const NLMISC::CMatrix &camMat, const CContinent &continent)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	// get the weather value for the current date
	nlassert(wc.WFP);
	float weatherValue = ::getBlendedWeather(day, hour, *(wc.WFP), wc.WF);
	// build current weather state
	EGSPD::CSeason::TSeason season = CRyzomTime::getSeasonByDay((uint32)day);
	//
	manualUpdate(day, hour, wc, weatherValue, season, camMat, continent);
	_LastEvalHour = hour;
	_LastEvalDay  = day;
}



//================================================================================================
void CWeatherManagerClient::manualUpdate(uint64 day, float hour, const CWeatherContext &wc, float weatherValue, EGSPD::CSeason::TSeason season, const NLMISC::CMatrix &camMat, const CContinent &continent)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	if (!wc.WF) return;
	manualUpdate(day, hour, wc, weatherValue, season);
	setupFXs(camMat, wc.GR, continent);
	setupWind(&(wc.WF[season]));
	float scaledWeatherValue = weatherValue * (wc.WF[season].getNumWeatherSetups() - 1);
	updateThunder(day, hour, wc, true, scaledWeatherValue, season);
	_LastEvalHour = hour;
	_LastEvalDay  = day;

	// Sound stuff
	if (SoundMngr != 0)
	{

		const CWeatherSetup *floorSetup, *ceilSetup;
		float blendFactor;
		wc.WF[season].getClosestWeatherSetups(scaledWeatherValue, floorSetup, ceilSetup, blendFactor);

		float userVarClouds1 = 0.f;
		float userVarClouds2 = 0.f;
		float userVarStorm = 0.f;
		//
		static TStringId strClouds1 = CStringMapper::map("clouds1");
		static TStringId strClouds2 = CStringMapper::map("clouds2");
		static TStringId strHumidity1 = CStringMapper::map("humidity1");
		static TStringId strHumidity2 = CStringMapper::map("humidity2");
		static TStringId strStorm= CStringMapper::map("storm");


		// contrib from previous setup
		if (_CurrWeatherState.FirstSetupName == strClouds1 ||
			_CurrWeatherState.FirstSetupName == strHumidity1)
		{
			if (floorSetup && !floorSetup->WeatherState.FXInfos.empty() && !floorSetup->WeatherState.FXInfos[0].Name.empty())
			{
				userVarClouds1 += (1.f - _CurrWeatherState.BlendFactor) * _LocalPrecipitationFactor * floorSetup->WeatherState.FXInfos[0].Ratio;
			}
		}
		else
		if (_CurrWeatherState.FirstSetupName == strClouds2 ||
			_CurrWeatherState.FirstSetupName == strHumidity2)
		{
			if (floorSetup && !floorSetup->WeatherState.FXInfos.empty() && !floorSetup->WeatherState.FXInfos[0].Name.empty())
			{
				userVarClouds2 += (1.f - _CurrWeatherState.BlendFactor) * _LocalPrecipitationFactor * floorSetup->WeatherState.FXInfos[0].Ratio;
			}
		}
		else
		if (_CurrWeatherState.FirstSetupName == strStorm)
		{
			if (floorSetup && !floorSetup->WeatherState.FXInfos.empty() && floorSetup->WeatherState.FXInfos[0].Name.empty())
			{
				userVarClouds2 += (1.f - _CurrWeatherState.BlendFactor) * _LocalPrecipitationFactor * floorSetup->WeatherState.FXInfos[0].Ratio;
			}
			userVarStorm += (1.f - _CurrWeatherState.BlendFactor) * _LocalPrecipitationFactor;
		}
		// contrib from next setup
		if (_CurrWeatherState.SecondSetupName == strClouds1 ||
			_CurrWeatherState.SecondSetupName == strHumidity1)
		{
			if (ceilSetup && !ceilSetup->WeatherState.FXInfos.empty() && !ceilSetup->WeatherState.FXInfos[0].Name.empty())
			{
				userVarClouds1 += _CurrWeatherState.BlendFactor * _LocalPrecipitationFactor * ceilSetup->WeatherState.FXInfos[0].Ratio;
			}
		}
		else
		if (_CurrWeatherState.SecondSetupName == strClouds2 ||
			_CurrWeatherState.SecondSetupName == strHumidity2)
		{
			if (ceilSetup && !ceilSetup->WeatherState.FXInfos.empty() && !ceilSetup->WeatherState.FXInfos[0].Name.empty())
			{
				userVarClouds2 += _CurrWeatherState.BlendFactor * _LocalPrecipitationFactor * ceilSetup->WeatherState.FXInfos[0].Ratio;
			}
		}
		else
		if (_CurrWeatherState.SecondSetupName == strStorm)
		{
			if (ceilSetup && !ceilSetup->WeatherState.FXInfos.empty() && !ceilSetup->WeatherState.FXInfos[0].Name.empty())
			{
				userVarClouds2 += _CurrWeatherState.BlendFactor * _LocalPrecipitationFactor * ceilSetup->WeatherState.FXInfos[0].Ratio;
			}
			userVarStorm += _CurrWeatherState.BlendFactor * _LocalPrecipitationFactor;
		}

		// update vars
		SoundMngr->getMixer()->setUserVar(strClouds1, 1.f - (1.f - userVarClouds1) * (1.f - userVarClouds1) * (1.f - userVarClouds1));
		SoundMngr->getMixer()->setUserVar(strClouds2, 1.f - (1.f - userVarClouds2) * (1.f - userVarClouds2) * (1.f - userVarClouds2));
		SoundMngr->getMixer()->setUserVar(strStorm, 1.f - (1.f - userVarStorm) * (1.f - userVarStorm) * (1.f - userVarStorm));


	}
}


//================================================================================================
void CWeatherManagerClient::manualUpdate(uint64 day, float hour, const CWeatherContext &wc, float weatherValue, EGSPD::CSeason::TSeason season)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	if (!wc.WF) return;
	_WeatherValue = weatherValue;
	nlassert(season < EGSPD::CSeason::Invalid);
	float scaledWeatherValue = weatherValue * (wc.WF[season].getNumWeatherSetups() - 1);
	const CWeatherSetup *floorSetup, *ceilSetup;
	float blendFactor;
	wc.WF[season].getClosestWeatherSetups(scaledWeatherValue, floorSetup, ceilSetup, blendFactor);
	if (floorSetup && ceilSetup)
	{
		// blend general part
		CWeatherState::blend(_CurrWeatherState, floorSetup->WeatherState, ceilSetup->WeatherState, blendFactor);
		// blend client specific part
		CWeatherStateClient::blend(_CurrWeatherStateClient, safe_cast<const CWeatherSetupClient *>(floorSetup)->WeatherStateClient, safe_cast<const CWeatherSetupClient *>(ceilSetup)->WeatherStateClient, blendFactor);
	}
	_LastEvalHour = hour;
	_LastEvalDay  = day;
}


//================================================================================================
void CWeatherManagerClient::setupWind(const CWeatherFunction *wf)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	float wi = _CurrWeatherState.WindIntensity;
	NLMISC::clamp(_CurrWeatherState.WindIntensity, 0.f, 1.f);
	// TEMP
	// wind for static & personnal FXs
	NL3D::UParticleSystemInstance::setGlobalUserParamValue("WIND", wi);
	// wind for vegetables
	if (Landscape)
	{
		float vegetWindFreq;
		float vegetBendIntensity;
		float vegetBendOffset;

		if (!wf)
		{
			static const float windBendMin = 0.5f;  // static for tests in debug mode
			static const float startWindBendMin = 0.6f;
			vegetWindFreq = 5.f * wi;
			vegetBendIntensity = wi;
			vegetBendOffset = wi < startWindBendMin ? 0.f : windBendMin * (wi - startWindBendMin) / (1.f - startWindBendMin);
		}
		else
		{
			vegetWindFreq = wf->VegetableMinWindFrequency +  (wf->VegetableMaxWindFrequency - wf->VegetableMinWindFrequency) * wi;
			vegetBendIntensity = wf->VegetableMinBendIntensity + (wf->VegetableMaxBendIntensity - wf->VegetableMinBendIntensity) * wi;
			if (wi < wf->VegetableWindIntensityThatStartBendOffset)
			{
				vegetBendOffset = 0.f;
			}
			else
			{
				vegetBendOffset = wf->VegetableMaxBendOffset * (wi - wf->VegetableWindIntensityThatStartBendOffset) / (1.f - wf->VegetableWindIntensityThatStartBendOffset);
			}

		}
		Landscape->setVegetableWind(_WindDir, vegetWindFreq, vegetBendIntensity, vegetBendOffset);
	}
	// wind for trees
	if (Scene)
	{
		float windTree;
		if (wf)
		{
			windTree = wf->TreeMinWindIntensity + (wf->TreeMaxWindIntensity - wf->TreeMinWindIntensity) * wi;
		}
		else
		{
			windTree = 0.1f + 0.9f * wi;
		}
		Scene->setGlobalWindPower(windTree);
	}
}

//================================================================================================
void CWeatherManagerClient::setupFXs(const NLMISC::CMatrix &camMat, NLPACS::UGlobalRetriever *gr, const CContinent &continent)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	static TPrecipitationVect askedPrecipitations;
	static TPrecipitationVect releasablePrecipitations;

	// Read the precipitation local ratio
	CVector pos = camMat.getPos();
	CRGBAF localNoPrecipitation = continent.FogMap.getMapValue (CFogMapBuild::NoPrecipitation, pos.x, pos.y, CRGBAF(0.f, 0.f, 0.f, 0.f));
	_LocalPrecipitationFactor = 1.f - localNoPrecipitation.R;
	//
	uint numAskedFXs = (uint)_CurrWeatherStateClient.FXs.size();
	askedPrecipitations.resize(numAskedFXs);
	uint k;
	for(k = 0; k < numAskedFXs; ++k)
	{
		askedPrecipitations[k] = _CurrWeatherStateClient.FXs[k].Precipitation;
		_CurrWeatherStateClient.FXs[k].Precipitation->setStrenght (_CurrWeatherStateClient.FXs[k].Ratio * _LocalPrecipitationFactor);
	}
	std::sort(askedPrecipitations.begin(), askedPrecipitations.end()); // sort em for std::set_
	// See which FXs where asked to be removed, but are now needed again (remove them from the waiting/shutting_down list)
	releasablePrecipitations.clear();
	std::set_difference(_WaitingPrecipitations.begin(), _WaitingPrecipitations.end(), askedPrecipitations.begin(), askedPrecipitations.end(), std::back_inserter(releasablePrecipitations));
	/*
	if (!releasablePrecipitations.empty())
	{
		for(uint k = 0; k < releasablePrecipitations.size(); ++k)
		{
			nlinfo("Shutting down precipitation reused : %s", releasablePrecipitations[k]->getDesc().FxName.c_str() );
		}
	}
	*/
	_WaitingPrecipitations.swap(releasablePrecipitations);
	// see which FXs are to be removed
	// NB : we are working on vectors, but they are likely to be very smalls
	releasablePrecipitations.clear();
	std::set_difference(_ActivePrecipitations.begin(), _ActivePrecipitations.end(), askedPrecipitations.begin(), askedPrecipitations.end(), std::back_inserter(releasablePrecipitations));
	//
	for(k = 0; k < releasablePrecipitations.size(); ++k)
	{
		releasablePrecipitations[k]->setStrenght(0);
	}
	/*
	if (!releasablePrecipitations.empty())
	{
		for(uint k = 0; k < releasablePrecipitations.size(); ++k)
		{
			nlinfo("Precipitation put in shutting down list : %s", releasablePrecipitations[k]->getDesc().FxName.c_str() );
		}
	}
	*/
	// put in the waiting precipitation list
	_WaitingPrecipitations.insert(_WaitingPrecipitations.end(), releasablePrecipitations.begin(), releasablePrecipitations.end());
	// set new active FX list
	_ActivePrecipitations.swap(askedPrecipitations);
	// tmp for debug : dump precipitation list if it has changed
	/*
	if (_ActivePrecipitations.size() != askedPrecipitations.size() ||
		!std::equal(_ActivePrecipitations.begin(), _ActivePrecipitations.end(), 	askedPrecipitations.begin()) )
	{
		for(uint k = 0; k < _ActivePrecipitations.size(); ++k)
		{
			nlinfo("New precipitation list : %s", _ActivePrecipitations[k]->getDesc().FxName.c_str() );
		}
	}
	*/
	// update precipitations
	updatePrecipitationVect(_ActivePrecipitations, camMat, gr);
	// update waiting precipitations
	updatePrecipitationVect(_WaitingPrecipitations, camMat, gr);
	// Remove waiting precipitation that are not running anymore (no more particles)
	TPrecipitationVect::iterator lastValid = std::remove_if(_WaitingPrecipitations.begin(), _WaitingPrecipitations.end(), std::not1(std::mem_fun(&CPrecipitation::isRunning)));
	_WaitingPrecipitations.erase(lastValid, _WaitingPrecipitations.end());
}

//================================================================================================
void CWeatherManagerClient::setWindDir(const NLMISC::CVector &dir)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	_WindDir.set(dir.x, dir.y, 0.f);
	_WindDir.normalize();
	NL3D::UParticleSystemInstance::setGlobalVectorValue("WIND", _WindDir);
}

//================================================================================================
void CWeatherManagerClient::computeCloudState(uint64 day, float hour, const CWeatherContext &wc, CCloudState &dest) const
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	if (!wc.WF)
	{
		dest = CCloudState();
		return;
	}
	nlassert(wc.WFP);
	float weatherValue = ::getBlendedWeather(day, hour, *(wc.WFP), wc.WF);
	// build current weather state
	uint season = CRyzomTime::getSeasonByDay((uint32)day);
	computeCloudState(weatherValue, (EGSPD::CSeason::TSeason) season, dest, wc.WF);
}

//================================================================================================
void CWeatherManagerClient::computeCloudState(float weatherValue, EGSPD::CSeason::TSeason season, CCloudState &dest, const CWeatherFunction wf[EGSPD::CSeason::Invalid]) const
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	if (!wf)
	{
		dest = CCloudState();
		return;
	}
	nlassert(season < EGSPD::CSeason::Invalid);
	wf[season].getCloudState(weatherValue * (wf[season].getNumWeatherSetups() - 1), dest);
}

//================================================================================================
void CWeatherManagerClient::release()
{
	for(TPrecipitationMap::iterator it = _PrecipitationMap.begin(); it != _PrecipitationMap.end(); ++it)
	{
		it->second.release();
	}
	NLMISC::contReset(_PrecipitationMap);
	NLMISC::contReset(_ActivePrecipitations);
	NLMISC::contReset(_WaitingPrecipitations);
	CWeatherManager::release();
	_CurrWeatherStateClient = CWeatherStateClient();
}

//================================================================================================
void CWeatherManagerClient::drawPrecipitationClipGrids(NL3D::UDriver &drv) const
{
	for(TPrecipitationVect::const_iterator it = _ActivePrecipitations.begin(); it != _ActivePrecipitations.end(); ++it)
	{
		(*it)->drawClipGrid(drv);
	}
}

//================================================================================================

// alias for the thunder time measure
typedef CWeatherManagerClient::CThunderTimeMeasure CThunderTime;

static inline bool operator == (const CThunderTime &lhs, const CThunderTime &rhs)
{
	return lhs.Cycle == rhs.Cycle && lhs.SubCycle == rhs.SubCycle;
}

static inline bool operator != (const CThunderTime &lhs, const CThunderTime &rhs)
{
	return !(lhs == rhs);
}


/** Eval the thunder function value at the given date
  */
static float evalThunderFunction(const CThunderTime &tt)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	// cache previous result
	static CThunderTime lastTime;
	static float lastValue;
	if (tt != lastTime)
	{
		CRandom rnd;
		rnd.srand((uint32) (tt.Cycle & 0xFFFFFFFF));
		sint32 dummy = rnd.rand();
		dummy = rnd.rand();
		dummy = rnd.rand();
		float v0 = rnd.frand(1.f);
		rnd.srand((uint32) ((tt.Cycle + 1) & 0xFFFFFFFF));
		dummy = rnd.rand();
		dummy = rnd.rand();
		dummy = rnd.rand();
		float v1 = rnd.frand(1.f);
		lastValue = tt.SubCycle * v1 + (1.f - tt.SubCycle) * v0;
		lastTime = tt;
	}
	return lastValue;
}

/** Convert a day / hour to a CThunderTime
  */
static inline void toThunderTime(uint64 day, float hour, CThunderTime &dest, const CWeatherContext &wc)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	// convert day part to seconds
	nlassert(wc.LC);
	nlassert(wc.WFP);
	double timeInSeconds = day * (double) wc.LC->RealDayLength + ((double) hour / wc.LC->NumHours) * wc.LC->RealDayLength;

	// convert date in second to the thunder cycle date
	double cycle = timeInSeconds / wc.WFP->MinThunderPeriod;
	dest.Cycle = (uint64) cycle;
	dest.SubCycle = (float) fmod(cycle, 1);
}

/** Convert CThunderTime cycle number to a day / hour pair
  */
static inline void toGlobalTime(uint64 &day, float &hour, uint64 cycle, const CWeatherContext &wc)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	nlassert(wc.WFP);
	nlassert(wc.LC);
	// convert to global time in seconds
	double timeInSeconds = cycle * (double) wc.WFP->MinThunderPeriod;
	double dDay;
	dDay = timeInSeconds / wc.LC->RealDayLength;
	day = (uint64) floor(dDay);
	hour = (float) fmod((float)dDay, wc.LC->NumHours);
}

/** Difference between 2 thunder time, expressed in thunder cycles
  */
static inline float diff(const CThunderTime &t0, const CThunderTime &t1)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	if (t0.Cycle == t1.Cycle) return t1.SubCycle - t0.SubCycle;
	return 1.f - t0.SubCycle + t1.SubCycle + (float) (t1.Cycle - t0.Cycle - 1);
}

/** Add the given number of cycle to a CThunderTime
  */
static inline void add(CThunderTime &dest, float duration)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	dest.Cycle += (uint64) floorf(duration);
	dest.SubCycle += fmodf(duration, 1.f); // add fractionnal part
	dest.Cycle += (uint64) floorf(dest.SubCycle);                // add carry
	dest.SubCycle = fmodf(dest.SubCycle, 1.f);
}

// blend between 2 thunder times
static inline void blend(CThunderTime &dest, const CThunderTime &t0, const CThunderTime &t1, float blendFactor)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	dest = t0;
	add(dest, blendFactor * diff(t0, t1));
}

//================================================================================================
/** Generate a new thunder strike if the weather function goes over the threshold
  * \return true if a strike wa generated
  */
bool CWeatherManagerClient::updateThunderState(CThunderTime &t0, CThunderTime &t1, float thunderThreshold)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	float thunderValue0 = evalThunderFunction(t0);
	float thunderValue1 = evalThunderFunction(t1);
	if (thunderValue0 <= thunderThreshold && thunderValue1 > thunderThreshold)
	{
		// The value went over the threshold -> generate a thunder strike
		// See at which date the strike started exactly
		float blendFactor = (thunderThreshold - thunderValue0) / (thunderValue1 - thunderValue0);
		// set the strike date
		blend(_ThunderStrikeDate, t0, t1, blendFactor);
		_ThunderStrike = true;
		return true;
	}
	return false;
}

//================================================================================================
void CWeatherManagerClient::updateThunder(uint64 day, float hour, const CWeatherContext &wc, bool manual, float manualWeatherValue, EGSPD::CSeason::TSeason manualSeason)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	// we use a random function that trigger thunder.
	// A thunder strike happens when the function is above of a threshold.
	// The function needs to get back below the threshold before another thunder strike can happen

	// first, build the 2 dates between which we should evaluate the thunder value;
	CThunderTime t0;
	CThunderTime t1;


	float manualThunderThreshold = 0.f;
	if (manual)
	{
		if (wc.WF)
		{
			manualThunderThreshold = 1.f - 0.5f * wc.WF[manualSeason].getThunderIntensity(manualWeatherValue);
		}
		else
		{
			manualThunderThreshold = 0.5f;
		}
	}


	toThunderTime(_LastEvalDay, _LastEvalHour, t0, wc); // start date
	toThunderTime(day, hour, t1, wc); // end date

	if (diff(t0, t1) > 1) // we don't need to look further that 1 thunder cycle to the past, because there can be only one thunder strike every other cycles.
	{
		t0.Cycle = t1.Cycle - 1;
		t0.SubCycle = t1.SubCycle;
	}

	if (t0.Cycle == t1.Cycle) // we stay in the same cycle, so the function is linear
	{
		float threshold = manual ? manualThunderThreshold : getThunderThreshold(t0.Cycle, wc);
		updateThunderState(t0, t1, threshold);
	}
	else
	{
		// Change of thunder cycle between the 2 dates
		CThunderTime middleTime;
		middleTime.Cycle = t1.Cycle;
		middleTime.SubCycle = 0.f;
		float threshold = manual ? manualThunderThreshold : getThunderThreshold(t0.Cycle,wc);
		bool strike = updateThunderState(t0, middleTime, threshold); // eval end of previous cycle
		if (!strike) // we are sure there won't be another strike during the next cycle
		{
			threshold = manual ? manualThunderThreshold : getThunderThreshold(t1.Cycle,wc);
			updateThunderState(middleTime, t1, threshold); // eval start of current cycle
		}
	}

	// update the thunder value
	nlassert(wc.WFP);
	if (_ThunderStrike)
	{
		// if too much time has ellapsed since the last thunder strike, disable it
		float timeEllapsedSinceStrike = diff(_ThunderStrikeDate, t1) * wc.WFP->MinThunderPeriod ;
		if (timeEllapsedSinceStrike >= wc.WFP->ThunderLength)
		{
			_ThunderStrike = false;
			_ThunderLevel = 0.f;
		}
		else
		{
			_ThunderLevel = wc.WFP->ThunderLength != 0.f ? 1.f - (timeEllapsedSinceStrike / wc.WFP->ThunderLength)
				                                         : 0.f;
		}
	}
}

//================================================================================================
float CWeatherManagerClient::getThunderIntensity(uint64 day, float hour, const CWeatherContext &wc)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	if (!wc.WF) return 0.f;
	nlassert(wc.WFP);
	float weatherValue = ::getBlendedWeather(day, hour, *(wc.WFP), wc.WF);
	uint season = CRyzomTime::getSeasonByDay((uint32)day);
	return wc.WF[season].getThunderIntensity(weatherValue * (wc.WF[season].getNumWeatherSetups() - 1));
}

//================================================================================================
float CWeatherManagerClient::getThunderThreshold(uint64 thunderCycle, const CWeatherContext &wc)
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	// convert the thunder cycle to a day / hour pair
	uint64 day;
	float hour;
	toGlobalTime(day, hour, thunderCycle, wc);
	return 1.f - 0.5f * getThunderIntensity(day, hour, wc);
}

// ***************************************************************************

CWeatherContext::CWeatherContext ()
{
	H_AUTO_USE(RZ_WeatherManagerClient)
	if (WeatherFunctionParams == NULL) WeatherFunctionParams = new CWeatherFunctionParamsSheet;
	WFP = WeatherFunctionParams;
	LC = &WorldLightCycle;
	WF = NULL;
	GR = NULL;
}

// ***************************************************************************
