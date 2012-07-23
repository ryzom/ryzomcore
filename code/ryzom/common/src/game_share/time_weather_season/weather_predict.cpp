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
#include "weather_predict.h"
#include "weather_setup.h"
#include "weather_function.h"
#include "time_and_season.h"
//
#include "../light_cycle.h"
#include "../season.h"
//
#include "weather_function_params_sheet_base.h"
#include "nel/misc/algo.h"
#include "nel/misc/random.h"
#include "nel/misc/noise_value.h"
#include "nel/misc/fast_floor.h"
//


static NLMISC::CNoiseValue nv;

float CPredictWeather::getCycleWeatherValue(uint64 cycle, const CWeatherFunction &wf)
{
	uint numWS = wf.getNumWeatherSetups();
	if (!numWS) return 0.f;
	NLMISC::CRandom rnd;
	NLMISC::OptFastFloorBegin();
	float noiseValue = nv.eval(NLMISC::CVector(cycle * 0.99524f, cycle * 0.85422f, cycle * -0.45722f));
	NLMISC::OptFastFloorEnd();
	noiseValue = fmodf(noiseValue * 10.f, 1.f); // make distribution more uniform
	uint32 value = (uint32) (noiseValue * (float) wf.getWeatherSetupsTotalWeight());
	uint32 currWeight = 0;
	for(uint k = 0; k < numWS; ++k)
	{
		if (value >= currWeight && value < currWeight + wf.getWeatherSetupWeight(k))
		{
			return (((float) (value - currWeight) / (float) wf.getWeatherSetupWeight(k)) + (float) k) / numWS;
		}
		currWeight += wf.getWeatherSetupWeight(k);
	}
	return 1.f;
}


float CPredictWeather::predictWeather(uint64 day, float hour, const CWeatherFunctionParamsSheetBase &wfp, const CWeatherFunction wf[EGSPD::CSeason::Invalid])
{
	if (!wf)
	{
		return 1.f;
	}
	if (wfp.DayLength <= 0.f || wfp.CycleLength == 0) return 0.f;
	nlassert(hour >= 0);
	day += (uint64) (hour / (float) wfp.DayLength);
	hour = fmodf(hour, (float) wfp.DayLength);
	// test in which cycle we are, we use this as a seed to a random fct to get reproductible behaviour
	nlassert(wfp.CycleLength != 0.f);
	float weatherValue;
	uint64 currHour = (day * wfp.DayLength) + (uint) hour;
	uint64 cycle = currHour / wfp.CycleLength;
	uint64 cycleStartHour = cycle * wfp.CycleLength; // global start hour of the cycle
	// the last hour of each cycle does a transition
	if (currHour - cycleStartHour < wfp.CycleLength - 1)
	{
		// not a transition
		EGSPD::CSeason::TSeason season = CRyzomTime::getSeasonByDay((uint32) day);
		weatherValue = getCycleWeatherValue(cycle, wf[season]);
	}
	else
	{
		// this is a transition
		EGSPD::CSeason::TSeason season = CRyzomTime::getSeasonByDay((uint32) day);
		EGSPD::CSeason::TSeason nextSeason = CRyzomTime::getSeasonByDay((uint32) ((cycleStartHour + wfp.CycleLength) & 0xFFFFFFFF) / wfp.DayLength);
		float blendFactor = (float) fmod(hour, 1);
		weatherValue = blendFactor * getCycleWeatherValue(cycle + 1, wf[nextSeason]) + (1.f - blendFactor) * getCycleWeatherValue(cycle, wf[season]);
	}
	return weatherValue;
}





/*
#include <utility>

using namespace std::rel_ops;
using namespace std;

inline bool operator == (const CWeatherFunctionParamsSheetBase &lhs, const CWeatherFunctionParamsSheetBase &rhs)
{
	return    lhs.CycleLength == rhs.CycleLength &&
		      lhs.MaximaRatio == rhs.MaximaRatio &&
			  lhs.DayLength == rhs.DayLength     &&
			  lhs.MaxARatio == rhs.MaxARatio     &&
			  lhs.MaxDRatio == rhs.MaxDRatio     &&
			  lhs.MinDRatio == rhs.MinDRatio;
}


struct CFctCtrlPoint
{
	float X;
	float Y;
	CFctCtrlPoint(float x = 0.f, float y = 0.f) : X(x), Y(y) {}
};
*/

// fct to order ctrl point by date
/*
static inline bool operator <  (const CFctCtrlPoint &lhs, const CFctCtrlPoint &rhs) { return lhs.X < rhs.X; }
static inline bool operator == (const CFctCtrlPoint &lhs, const CFctCtrlPoint &rhs) { return lhs.X == rhs.X; }
*/

/** Tool fct to evaluate the value of a function f(x) defined by a set of points joined by segments
  * \param x point at which the function must be evaluated
  * \param points A set of (x, y) pair which define the function
  * \param numPoints The number of points if the set
  */
/*
static float evalSegFunction(float x, const CFctCtrlPoint *points, uint numPoints)
{
	nlassert(points);
	nlassert(numPoints > 0);
	// extremes cases
	if (x <= points[0].X) return points[0].Y;
	if (x >= points[numPoints - 1].X) return points[numPoints - 1].Y;
	// find the first previous point
	uint index = NLMISC::searchLowerBound(points, numPoints, CFctCtrlPoint(x));
	nlassert(index < numPoints - 1);
	// slope is oo ? If yes return next value. (arbitrary)
	if (points[index].X == points[index + 1].X) return points[index + 1].Y;
	// interpolate linearly
	float lambda = (x - points[index].X) / (points[index + 1].X - points[index].X);
	return lambda * points[index + 1].Y + (1.f - lambda)  * points[index].Y;
}
*/


//================================================================
/** Init a random generator from a cycle number
  */
/*
static inline void initRand(NLMISC::CRandom &rg, uint64 cycleNumber)
{
	rg.srand((sint32) ((cycleNumber & 0xffffffff) ^ (cycleNumber >> 32)));
	// make some iteration to have a more natural result
	uint v = rg.rand();
	v = rg.rand();
	v = rg.rand();
	v = rg.rand();
}
*/



//================================================================
/** Get value for fair weather in the range [0, 1] for the given season
  */
/*
static float getFairWeatherValue(EGSPD::CSeason::TSeason season, const CWeatherFunction wf[EGSPD::CSeason::Invalid])
{
	nlassert(season < EGSPD::CSeason::Invalid);
	if (!wf) return 0.f;
	if (wf[season].getNumWeatherSetups() == 0.f) return 0.f;
	return wf[season].getFairWeatherValue() / (float) wf[season].getNumWeatherSetups();
}
*/

//================================================================
/** Get start value for a weather cycle
  * NB : During a day->night or night->day transition, the weather should be fair
  * NB : During the first cycle of the first day of the season, the weather should start in a fair way (because seasons don't have the same weather setups, we must be sure that the transition is correct)
  */
/*
static float getCycleStartValue(uint64 day, uint64 totalHour, const CWeatherFunctionParamsSheetBase &wfp, const CWeatherFunction wf[EGSPD::CSeason::Invalid])
{
	uint64 cycle = totalHour / wfp.CycleLength;
	uint64 cycleStartHour = cycle * wfp.CycleLength;
	uint64 dayStartHour = day * wfp.DayLength; // the global hour at which the day starts

	EGSPD::CSeason::TSeason season = CRyzomTime::getSeasonByDay((uint32)day);


	// When a weather cycle starts at a season, and end at another one, this is a special case where Weather value must be set to "fair weather"
	uint64 dayForEndCycle = (cycleStartHour + wfp.CycleLength) / wfp.DayLength;
	if (CRyzomTime::getSeasonByDay((uint32)day) != season)
	{
		// yes this is a transition cycle, so return the fair weather value for the previous season
		return getFairWeatherValue(season, wf);
	}


	// standard case, see if it is a low presure or hight presure cycle
	NLMISC::CRandom randomGenerator;
	initRand(randomGenerator, cycle);
	bool isLowPressure;
	if (wf)
	{
		 float randValue = randomGenerator.frand(100.f);
		float lowPressurePercent = wf[season].LowPressurePercent;
		//nldebug("%f < %f ?", randValue, lowPressurePercent);
		isLowPressure =  randValue >= lowPressurePercent;
	}
	else
	{
		isLowPressure = (randomGenerator.rand() & 1) != 0;
	}
	if (!isLowPressure)
	{
		// high pressure cycle : always starts with fair weather
		return getFairWeatherValue(season, wf);
	}
	else
	{
		// low pressure cycle
		initRand(randomGenerator, ~cycle);
		if (wf)
		{
			if (wf[season].getNumWeatherSetups() == 0.f) return 0.f;
			float ratio = randomGenerator.frand(1.f);
			CWeatherFunction::TInterval inter = wf[season].getPressureInterval(0);
			// delta interval in forms is expressed in the range [0, numWeatherSetups]
			float delta = ratio * inter.second + (1.f - ratio) * inter.first;
			return delta / (float) wf[season].getNumWeatherSetups();
		}
		else
		{
			return 0;
		}
	}
}
*/

//================================================================
/*
float CPredictWeather::predictWeather(uint64 day, float hour, const CWeatherFunctionParamsSheetBase &wfp, const CWeatherFunction *wf, EWeatherCycleType *cycleTypeDest)
{
	if (!wf)
	{
		return 1.f;
	}
	if (wfp.DayLength <= 0.f || wfp.CycleLength == 0) return 0.f;
	nlassert(hour >= 0);
	day += (uint64) (hour / (float) wfp.DayLength);
	hour = fmodf(hour, (float) wfp.DayLength);
	// test in which cycle we are, we use this as a seed to a random fct to get reproductible behaviour
	nlassert(wfp.CycleLength != 0.f);
	uint64 currHour = (day * wfp.DayLength) + (uint) hour;
	uint64 cycle = currHour / wfp.CycleLength;
	uint64 cycleStartHour = cycle * wfp.CycleLength; // global start hour of the cycle
	// cache previous results, this avoid to recompute the weather function
	static const CFctCtrlPoint	 *lastFct;
	static uint				     lastNumPoints;
	static CWeatherFunctionParamsSheetBase lastDesc;
	static uint64                lastCycle = std::numeric_limits<uint64>::max();
	static EWeatherCycleType     weatherCycle = HighPressure;
	static const CWeatherFunction *lastWf = NULL;

	if (cycle != lastCycle || lastDesc != wfp || lastWf != wf) // are we in a new cycle of weather
	{
		EGSPD::CSeason::TSeason season = CRyzomTime::getSeasonByDay((uint32)day);

		// must recompute fonction
		lastDesc  = wfp;
		lastCycle = cycle;
		lastWf = wf;

		// special case : see if the weather is at a transition of season
		uint64 endCycleDay = (cycleStartHour + wfp.CycleLength) / wfp.DayLength; // which day is it at the end of the cycle
		EGSPD::CSeason::TSeason nextSeason = CRyzomTime::getSeasonByDay((uint32)endCycleDay);
		if (nextSeason != season)
		{
			// special transition case : we switch between fair weather value for season 1 and for season 2
			// The function may not be continue at the point where the season change, if the value for fair weather isn't the same in both seasons
			static CFctCtrlPoint transitionFct[4];
			transitionFct[0].X = 0.f;
			transitionFct[0].Y = getFairWeatherValue(season, wf);
			transitionFct[1].X = (float) (endCycleDay * wfp.DayLength - cycleStartHour);
			transitionFct[1].Y = getFairWeatherValue(season, wf);;
			transitionFct[2].X = transitionFct[1].X;
			transitionFct[2].Y = getFairWeatherValue(nextSeason, wf);;
			transitionFct[3].X = (float) wfp.CycleLength;
			transitionFct[3].Y = getCycleStartValue(day, cycleStartHour + wfp.CycleLength, wfp, wf); // start value for the next cycle
			lastFct = transitionFct;
			lastNumPoints = sizeof(transitionFct) / sizeof(transitionFct[0]);
			weatherCycle = SeasonTransition;
		}
		else
		{
			uint64 dayStartHour = day * wfp.DayLength; // the global hour at which the day starts

			NLMISC::CRandom randomGenerator;
			// set seed
			initRand(randomGenerator, cycle);
			bool isLowPressure;
			if (wf)
			{
				float randValue = randomGenerator.frand(100.f);
				float lowPressurePercent = wf[season].LowPressurePercent;
				//nldebug("%f < %f ?", randValue, lowPressurePercent);
				isLowPressure =  randValue >= lowPressurePercent;
			}
			else
			{
				isLowPressure = (randomGenerator.rand() & 1) != 0;
			}
			if (!isLowPressure)
			{
				// high pressure. There is fair weather, or mist
				// The cycle always starts with a fair weather value
				// The mist value is constant for all the cycle
				// For each hour, we see if there is mist, and set the function accordingly

				static std::vector<CFctCtrlPoint> lpFct;
				lpFct.resize(wfp.CycleLength);

				float fairWeatherValue = getFairWeatherValue(season, wf);
				float A;
				// compute value of mist
				if (wf)
				{
					if (wf[season].getNumWeatherSetups() == 0.f)
					{
						A = fairWeatherValue;
					}
					else
					{
						float ratio = randomGenerator.frand(1.f);
						CWeatherFunction::TInterval inter = wf[season].getPressureInterval(1);
						// delta interval in forms is expressed in the range [0, numWeatherSetups]
						float weatherSetupValue = ratio * inter.second + (1.f - ratio) * inter.first;
						A = weatherSetupValue / (float) wf[season].getNumWeatherSetups();
					}
				}

				// starts with fair weather
				lpFct[0] = CFctCtrlPoint(0.f, fairWeatherValue);
				//
				uint currHour = (uint) (cycleStartHour - day * wfp.DayLength);
				// for each hour, see if mist is needed
				for(uint k = 1; k < wfp.CycleLength - 1; ++k)
				{
					++currHour;
					if (currHour == wfp.DayLength) currHour = 0;

					if (k == 0 || k == (wfp.CycleLength - 1))
					{
						lpFct[k] = CFctCtrlPoint((float) k, fairWeatherValue);
					}
					else
					{
						bool inMist = (wf[season].MistStartHour <= wf[season].MistEndHour) ? currHour >= wf[season].MistStartHour && currHour <= wf[season].MistEndHour
							                                                               : currHour >= wf[season].MistStartHour || currHour <= wf[season].MistEndHour;
						// is it a mist hour ?
						if (inMist)
						{
							lpFct[k] = CFctCtrlPoint((float) k, A);
						}
						else
						{
							lpFct[k] = CFctCtrlPoint((float) k, fairWeatherValue);
						}
						// take next hour

					}
				}

				// ends with start value of next cycle
				float endValue   = getCycleStartValue(day, cycleStartHour + wfp.CycleLength, wfp, wf);
				lpFct[wfp.CycleLength - 1] = CFctCtrlPoint((float) wfp.CycleLength - 1, endValue);

				lastFct = &lpFct[0];
				lastNumPoints = lpFct.size();
				weatherCycle = HighPressure;
			}
			else
			{
				// low pressure cycle
				// The fct has the following aspect :
				//
				//         D * F     D*(1-F)     A      D / 4   D / 4
				//0 | <-------------><-----> <------> <------><--------->
				//  | ***            ^     ***********        ^        *
				//  |    ***         |    *           *       |       *
				//  |       ***     E|   *             *      |      *
				//  |          ***   |  *               *     | C   *
				//  |             ***v *                 *    |    *
				//  |                **                   *   |   *
				//  |                                      *  |  *
				//  |                                       * | *
				//  |                                        *v*
				//  |                                         *
				//1 v <------------------------------------------------->
				//                                 B
				//
				//  With A from 0% to 90% of B
				//  C : 0-1 (random)
				//  D : 2 * B - A
				//  E : C * fixed factor alpha (< 1)
				//  F : random in [0.25f, 0.75f]
				//
				//

				float A = randomGenerator.frand(wfp.MaxARatio) * wfp.CycleLength;
				float C = randomGenerator.frand(1.f);
				if (wf)
				{
					C *= wf[season].LowPressureValueFactor;
				}
				float D = 2.f * (wfp.CycleLength - A) / 3.f;
				float E = C * wfp.MaximaRatio;
				float F = wfp.MinDRatio + randomGenerator.frand(wfp.MaxDRatio - wfp.MinDRatio);

				float startValue = getCycleStartValue(day, cycleStartHour, wfp, wf);
				float endValue   = getCycleStartValue(day, cycleStartHour + wfp.CycleLength, wfp, wf);

				static CFctCtrlPoint hpFct[6];
				hpFct[0].X = 0.f;
				hpFct[0].Y = startValue;
				hpFct[1].X = D * F;
				hpFct[1].Y = E + startValue;
				hpFct[2].X = D;
				hpFct[2].Y = startValue;
				hpFct[3].X = D + A;
				hpFct[3].Y = startValue;
				hpFct[4].X = 1.25f * D + A;
				hpFct[4].Y = startValue + C;
				hpFct[5].X = (float) wfp.CycleLength;
				hpFct[5].Y = endValue;

				lastFct = hpFct;
				lastNumPoints = sizeof(hpFct) / sizeof(hpFct[0]);

				weatherCycle = LowPressure;
			}
		}
	}
	if (cycleTypeDest)
	{
		*cycleTypeDest = weatherCycle;
	}
	float weatherValue = evalSegFunction((float) (currHour - cycleStartHour) + fmodf(hour, 1.f), lastFct, lastNumPoints);
	NLMISC::clamp(weatherValue, 0.f, 1.f);
	return weatherValue;
}
*/




//================================================================
// used by the stats generation
/*
struct CFloatValue
{
	double Value;
	CFloatValue() : Value(0) {}
};
*/

//================================================================
/*
void CPredictWeather::generateWeatherStats(const std::string &fileName, const CWeatherFunctionParamsSheetBase &wfp, const CWeatherFunction wf[EGSPD::CSeason::Invalid])
{
	if (!wf)
	{
		nlwarning("CWeatherFunctionParams::generateWeatherStats : no weather function has been supplied!");
		return;
	}
	if (fileName.empty())
	{
		nlwarning("CWeatherFunctionParams::generateWeatherStats : the stats fileName is empty!");
		return;
	}
	// give the amount for each weather setup
	typedef std::map<std::string, CFloatValue> TWeatherStatMap;
	// stats for each season
	TWeatherStatMap stats[EGSPD::CSeason::Invalid];
	//
	uint k, l;
	const uint numSamples = 1000;
	static uint64 startDay = 0;
	// make stats for 1000 days
	for(uint64 day = startDay; day < startDay + numSamples; ++day)
	{
		EGSPD::CSeason::TSeason season = CRyzomTime::getSeasonByDay((uint32)day);
		const uint numSamples = 500;
		// Take 2000 sample of weather state along the day
		for(k = 0; k < numSamples; ++k)
		{
			float hour = wfp.DayLength / (float) numSamples;
			float weatherValue = predictWeather(day, hour, wfp, wf);
			if (wf[season].getNumWeatherSetups() == 0.f) continue;
			if (wf[season].getNumWeatherSetups() == 1.f)
			{
				std::string wsName = NLMISC::CStringMapper::unmap(wf[season].getWeatherSetup(0)->SetupName);
				stats[season][wsName].Value += 1;
			}
			else
			{
				float weatherSetupValue = weatherValue * (wf[season].getNumWeatherSetups() - 1);
				std::string wsName0 = NLMISC::CStringMapper::unmap(wf[season].getWeatherSetup((uint) weatherSetupValue)->SetupName);
				std::string wsName1 = NLMISC::CStringMapper::unmap(wf[season].getWeatherSetup(std::min(((uint) weatherSetupValue) + 1, wf[season].getNumWeatherSetups() - 1))->SetupName);
				stats[season][wsName0].Value += 1.f - fmodf(weatherSetupValue, 1.f);
				stats[season][wsName1].Value += fmodf(weatherSetupValue, 1.f);
			}
		}
	}
	//
	startDay += numSamples;

	// Get all setups names
	std::set<string> seasonsNames;
	for(k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		for(l = 0; l < wf[k].getNumWeatherSetups(); ++l)
		{
			seasonsNames.insert(NLMISC::CStringMapper::unmap(wf[k].getWeatherSetup(l)->SetupName));
		}
	}

	// make sum of setups contributions for each seasons
	double setupContributionSum[EGSPD::CSeason::Invalid];
	std::fill(setupContributionSum, setupContributionSum + EGSPD::CSeason::EndSeason, 0);
	for(k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		for(TWeatherStatMap::const_iterator statIt = stats[k].begin(); statIt != stats[k].end(); ++statIt)
		{
			setupContributionSum[k] += statIt->second.Value;
		}
	}

	// create output string

	// write seasons name
	std::string output = " ;";
	for(k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		output += EGSPD::CSeason::toString(k) + ";";
	}
	output +="\n";

	// write percents for each weather setups
	for(std::set<string>::const_iterator it = seasonsNames.begin(); it != seasonsNames.end(); ++it)
	{
		output += *it + ";";
		for(k = 0; k < EGSPD::CSeason::Invalid; ++k)
		{
			output += NLMISC::toString("%.1f;", (float) (100 * stats[k][*it].Value / setupContributionSum[k]));
		}
		output +="\n";
	}

	NLMISC::COFile outputFile;
	if (!outputFile.open(fileName))
	{
		nlwarning("Couldn't open %s to write weather statistics", fileName.c_str());
		return;
	}
	try
	{
		for(uint n = 0; n < output.size(); ++n)
		{
			outputFile.serial(output[n]);
		}
	}
	catch (const NLMISC::EStream &e)
	{
		nlwarning(e.what());
	}
	outputFile.close();
}
*/
