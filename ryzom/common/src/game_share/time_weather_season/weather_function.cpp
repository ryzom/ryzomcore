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
#include "weather_function_sheet.h"
#include "weather_function.h"
#include "weather_manager.h"
#include "weather_setup.h"
#include "nel/georges/u_form_elm.h"

NL_INSTANCE_COUNTER_IMPL(CWeatherFunction);

//==================================================================
CWeatherFunction::CWeatherFunction()
{
	_TotalWeight = 0;
}



//==================================================================
void CWeatherFunction::buildFromSheet(const CWeatherFunctionSheet &sheet, const CWeatherManager &wm)
{
	// copy common part of objects (parameters)
	*static_cast<CWeatherFunctionParameters *>(this) = *static_cast<const CWeatherFunctionParameters *>(&sheet);
	// get pointer on the setup from their names
	_WeatherSetups.resize(sheet.SetupNames.size());
	nlassert(sheet.SetupWeights.size() == sheet.SetupNames.size());
	for(uint k = 0; k < sheet.SetupNames.size(); ++k)
	{
		_WeatherSetups[k].WS = wm.getSetup(sheet.SetupNames[k].c_str());
		if(!_WeatherSetups[k].WS)
		{
			nlwarning("Unknown weather setup : %s", sheet.SetupNames[k].c_str());
		}
		_WeatherSetups[k].Weight = sheet.SetupWeights[k];
		_TotalWeight += _WeatherSetups[k].Weight;
	}
}


//==================================================================
void CWeatherFunction::getWeatherState(float weatherValue, CWeatherState &dest) const
{
	const CWeatherSetup *floorSetup, *ceilSetup;
	float blendFactor;
	getClosestWeatherSetups(weatherValue, floorSetup, ceilSetup, blendFactor);
	if (!floorSetup && !ceilSetup)
	{
		dest = CWeatherState();
		return;
	}
	if (!floorSetup)
	{
		dest = ceilSetup->WeatherState;
	}
	else
	if (!ceilSetup)
	{
		dest = floorSetup->WeatherState;
	}
	else
	{
		CWeatherState::blend(dest, floorSetup->WeatherState, ceilSetup->WeatherState, blendFactor);
	}
}

//==================================================================
void CWeatherFunction::getClosestWeatherSetups(float weatherValue, const CWeatherSetup *&floorSetup, const CWeatherSetup *&ceilSetup, float &blendValue) const
{
	if (_WeatherSetups.empty())
	{
		floorSetup = NULL;
		ceilSetup = NULL;
		blendValue = 0.f;
		return;
	}
	if (_WeatherSetups.size() == 1)
	{
		floorSetup = _WeatherSetups[0].WS;
		ceilSetup = _WeatherSetups[0].WS;
		blendValue = 0.f;
		return;
	}
	NLMISC::clamp(weatherValue, 0.f, (float) (_WeatherSetups.size() - 1));
	uint start = (uint) weatherValue;
	uint end = start + 1;
	end = std::min((uint)_WeatherSetups.size() - 1, end);
	floorSetup = _WeatherSetups[start].WS;
	ceilSetup = _WeatherSetups[end].WS;
	blendValue = weatherValue - (float) start;
}


//==================================================================
void CWeatherFunction::getCloudState(float weatherValue, CCloudState &dest) const
{
	const CWeatherSetup *floorSetup, *ceilSetup;
	float blendFactor;
	getClosestWeatherSetups(weatherValue, floorSetup, ceilSetup, blendFactor);
	if (!floorSetup && !ceilSetup)
	{
		dest = CCloudState();
		return;
	}
	if (!floorSetup)
	{
		dest = ceilSetup->CloudState;
	}
	else
	if (!ceilSetup)
	{
		dest = floorSetup->CloudState;
	}
	else
	{
		CCloudState::blend(dest, floorSetup->CloudState, floorSetup->CloudState, blendFactor);
	}
}

//==================================================================
float CWeatherFunction::getThunderIntensity(float weatherValue) const
{
	const CWeatherSetup *floorSetup, *ceilSetup;
	float blendFactor;
	getClosestWeatherSetups(weatherValue, floorSetup, ceilSetup, blendFactor);
	if (!floorSetup && !ceilSetup) return 0.f;
	if (!floorSetup) return ceilSetup->WeatherState.ThunderIntensity;
	if (!ceilSetup) return floorSetup->WeatherState.ThunderIntensity;
	return blendFactor * ceilSetup->WeatherState.ThunderIntensity + (1.f - blendFactor) * floorSetup->WeatherState.ThunderIntensity;
}






