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



#ifndef CL_WEATHER_FUNCTION_H
#define CL_WEATHER_FUNCTION_H

#include "nel/misc/debug.h"
#include "weather_function_sheet.h"

namespace NLGEORGES
{
	class UFormElm;
}

class CWeatherState;
class CWeatherSetup;
class CCloudState;
class CWeatherManager;
class CWeatherSetupSheetBase;
class CWeatherFunctionSheetBase;



/** A weather function is a set of weather setup. Depending on the weather function, it blends between 2 setups to get the current state of the weather
  * It should be initialized after all weather setup have been loaded, because it builds references on the weather setups.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CWeatherFunction : public CWeatherFunctionParameters
{
	NL_INSTANCE_COUNTER_DECL(CWeatherFunction);
public:

	// ctor
	CWeatherFunction();
	// Build the weather function from the matching sheet
	// NB : all weather setups should have been built before this is called

	void				buildFromSheet(const CWeatherFunctionSheet &sheet, const CWeatherManager &wm);
	/** From an index value, return a blend of weather states. The index value is clamped to [0, numberOfSetups - 1]
	  * e.g If the index value is 1.5, this will return setup 1 & 2 equally blended together.
	  * NB : The thunder intensity is not interpolated during this call. see getThunderIntensity()
	  * \param dest The result weather setup
	  */
	void				getWeatherState(float weatherValue, CWeatherState &dest) const;
	// get 2 much closest weather setups for a given weather value (ordered by weather value), and the blend factor needed to get the setup that march the given weather value.
	void				getClosestWeatherSetups(float weatherValue, const CWeatherSetup *&floorSetup, const CWeatherSetup *&ceilSetup, float &blendValue) const;
	/** From an index value, return a blend of cloud states. The index value is clamped to [0, numberOfSetups - 1]
	  * e.g If the index value is 1.5, this will return setup 1 & 2 equally blended togather
	  * \param dest The result weather setup
	  */
	void				getCloudState(float weatherValue, CCloudState &dest) const;
	/** Get the thunder intensity for the given weather index value. The index value is clamped to [0, numberOfSetups - 1]
	  */
	float				getThunderIntensity(float weatherValue) const;
	/** get interval for low or high pressure
	  * The maxima and minima are in the range [0, numWeatherSetups]
	  * \param pressure 0 for low pressure and 1 for high pressure
	  */
	uint                getNumWeatherSetups() const { return (uint)_WeatherSetups.size(); }
	const CWeatherSetup *getWeatherSetup(uint index) const { return _WeatherSetups.empty() ? NULL : _WeatherSetups[index].WS; }
	uint32              getWeatherSetupWeight(uint index) const { return _WeatherSetups[index].Weight; }
	uint32				getWeatherSetupsTotalWeight() const { return _TotalWeight; }

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
private:
	class CWSInfo
	{
	public:
		const CWeatherSetup *WS;
		uint32				 Weight; // the more high the weight, the more frequent this weather setup happens
	public:
		CWSInfo() : WS(NULL), Weight(1) {}
	};
	std::vector<CWSInfo> _WeatherSetups;
	uint32 _TotalWeight;
};




#endif
