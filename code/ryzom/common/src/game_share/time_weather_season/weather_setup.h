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



#ifndef RY_WEATHER_SETUP
#define RY_WEATHER_SETUP

#include "nel/misc/rgba.h"
#include "weather_setup_sheet_base.h"
#include "../fog_type.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/smart_ptr.h"
#include <vector>

namespace NLGEORGES
{
	class UFormElm;
}


class CPrecipitation;

/** Description of weather state, not including clouds
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CWeatherState : public CWeatherStateSheet
{
public:
	/// The first blended setup name (contribute for 1-blendFactor)
	NLMISC::TStringId	FirstSetupName;
	/// The second blended setup name (contribute for blendFactor)
	NLMISC::TStringId	SecondSetupName;
	// Best aprox setup name when blended (contain the original name in reference setup)
	NLMISC::TStringId	BestSetupName;
	// 0 by default, it contains the parameters passed to a call to blend
	float				BlendFactor;

	/** Result of the blend
	  */
	std::string	  DayBackgroundFileName1;
	std::string	  DuskBackgroundFileName1;
	std::string	  NightBackgroundFileName1;
	std::string	  DayBackgroundFileName2;
	std::string	  DuskBackgroundFileName2;
	std::string	  NightBackgroundFileName2;
public:
	// default ctor
	CWeatherState();
	/** Blend 2 weather state together
	  * Blend factor is clamped to [0, 1]
	  */
	static void   blend(CWeatherState &dest, const CWeatherState &s1, const CWeatherState &s2, float blendFactor);
};

/** Description of clouds state
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CCloudState : public CCloudStateSheet
{
public:
	/** Blend 2 clouds state
	  * Blend factor is clamped to [0, 1]
	  */
	static void          blend(CCloudState &dest, const CCloudState &s1, const CCloudState &s2, float blendFactor);
};


/** Weather setup base class
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CWeatherSetup : public NLMISC::CRefCount
{
public:
	CWeatherState       WeatherState;
	CCloudState         CloudState;
	NLMISC::TStringId	SetupName;
	virtual ~CWeatherSetup() {}
};



#endif




