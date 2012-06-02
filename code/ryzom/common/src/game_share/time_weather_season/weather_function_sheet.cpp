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
//
#include "nel/misc/stream.h"
#include "nel/misc/debug.h"
#include "nel/georges/u_form_elm.h"


/** tool fct to get a float value from weather form, and to display an error msg if prb
  */
template <class T> static void getWeatherFuncFormValue(const NLGEORGES::UFormElm &item, T &dest, const char *name)
{
	if (!item.getValueByName(dest, name)) nlwarning("Couldn't get %s from weather function form", name);
}

//=============================================================================
CWeatherFunctionParameters::CWeatherFunctionParameters() : VegetableMinBendIntensity(0.1f),
														   VegetableMaxBendIntensity(1.f),
														   VegetableMinWindFrequency(0.1f),
														   VegetableMaxWindFrequency(5.f),
														   VegetableMaxBendOffset(0.5f),
														   VegetableWindIntensityThatStartBendOffset(0.6f),
														   TreeMinWindIntensity(0.1f),
														   TreeMaxWindIntensity(1.f)
{
}

//=============================================================================
CWeatherFunctionSheet::CWeatherFunctionSheet()
{
}

//=============================================================================
void CWeatherFunctionSheet::build(const NLGEORGES::UFormElm &item)
{
	const NLGEORGES::UFormElm *elm;
	uint numSetups = 0;
	// get list of weather setups from the form
	if(item.getNodeByName (&elm, "WeatherSetups") && elm)
	{
		// Get number of setups
		nlverify (elm->getArraySize (numSetups));
		SetupNames.resize(numSetups);
		// For each setup
		for(uint k = 0; k < numSetups; ++k)
		{
			if (!elm->getArrayValue(SetupNames[k], k))
			{
				nlwarning("Can't read weather setup from form");
			}
		}
	}
	uint numWeights = 0;
	SetupWeights.resize(numSetups);
	// get weight of each weather setup. Setup that are not given are assumed to be 1
	if(item.getNodeByName (&elm, "SetupsWeights") && elm)
	{
		// Get number of setups
		nlverify (elm->getArraySize (numWeights));
		numWeights = std::min(numSetups, numWeights);
		// For each setup
		for(uint k = 0; k < numWeights; ++k)
		{
			if (!elm->getArrayValue(SetupWeights[k], k))
			{
				nlwarning("Can't read weather setup from form");
			}
			SetupWeights[k] = std::max((uint32) 1, SetupWeights[k]);
		}
	}
	// complete other weights if not same size
	std::fill(SetupWeights.begin() + numWeights, SetupWeights.begin() + numSetups, 1);
	//
	getWeatherFuncFormValue(item, VegetableMinBendIntensity, "Visual.VegetableMinBendIntensity");
	getWeatherFuncFormValue(item, VegetableMaxBendIntensity, "Visual.VegetableMaxBendIntensity");
	getWeatherFuncFormValue(item, VegetableMinWindFrequency, "Visual.VegetableMinWindFrequency");
	getWeatherFuncFormValue(item, VegetableMaxWindFrequency, "Visual.VegetableMaxWindFrequency");
	getWeatherFuncFormValue(item, VegetableMaxBendOffset, "Visual.VegetableMaxBendOffset");
	getWeatherFuncFormValue(item, VegetableWindIntensityThatStartBendOffset, "Visual.VegetableWindIntensityThatStartBendOffset");
	//
	getWeatherFuncFormValue(item, TreeMinWindIntensity, "Visual.TreeMinWindIntensity");
	getWeatherFuncFormValue(item, TreeMaxWindIntensity, "Visual.TreeMaxWindIntensity");

}


//=============================================================================
void CWeatherFunctionSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(VegetableMinBendIntensity);
	f.serial(VegetableMaxBendIntensity);
	f.serial(VegetableMinWindFrequency);
	f.serial(VegetableMaxWindFrequency);
	f.serial(VegetableMaxBendOffset);
	f.serial(VegetableWindIntensityThatStartBendOffset);
	//
	f.serial(TreeMinWindIntensity);
	f.serial(TreeMaxWindIntensity);
	//
	f.serialCont(SetupNames);
	f.serialCont(SetupWeights);
}
