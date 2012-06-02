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



#ifndef RY_WEATHER_FUNCTION_SHEET
#define RY_WEATHER_FUNCTION_SHEET

#include "nel/misc/types_nl.h"
#include <vector>

namespace NLGEORGES
{
	class UFormElm;
	class IStream;
}
namespace NLMISC
{
	struct EStream;
	class IStream;
}

// parameters of weather function, not including the list of setups
struct CWeatherFunctionParameters
{
	// Parameters from form related to wind
	float	VegetableMinBendIntensity;
	float	VegetableMaxBendIntensity;
	float	VegetableMinWindFrequency;
	float	VegetableMaxWindFrequency;
	float   VegetableMaxBendOffset;
	float   VegetableWindIntensityThatStartBendOffset; // The wind intensity at which the vegetable start to have a bending offset
	//
	float   TreeMinWindIntensity;
	float   TreeMaxWindIntensity;
	// ctor
	CWeatherFunctionParameters();
};

/**
 * Class to manage weather function sheets
 * sheet defining a weather function
 * Please note that we dont derive from CEntitySheet because CWeatherFunctionSheet instances are aggregated in a CContinentSheetInstance
 *
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */
class CWeatherFunctionSheet : public CWeatherFunctionParameters
{
public:
	std::vector<std::string> SetupNames;
	std::vector<uint32>		 SetupWeights;
public:
	// ctor
	CWeatherFunctionSheet();

	void build(const NLGEORGES::UFormElm &item);
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};



#endif
