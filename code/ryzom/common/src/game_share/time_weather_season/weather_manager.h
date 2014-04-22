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





#ifndef CL_WEATHER_MANAGER_H
#define CL_WEATHER_MANAGER_H

#include "weather_setup.h"
#include <map>

class CWeatherSetupSheetBase;

/** Weather manager gather all weather setups (or derived class)
  */
class CWeatherManager
{
public:
	// dtor
	virtual ~CWeatherManager();
	/** Setup the manager from a set of weather sheets and their name
	  * NB : arrays of names and sheets must have the same size or an assertion occurs
	  */
	virtual void         init(const std::vector<const CWeatherSetupSheetBase *> &sheets, const std::vector<std::string> &sheetNames);
	/// Release all datas (fx models..). Should be called before deleting the scene
	void				 release();
	/// get a weather setup from its name, and return a pointer to the setup or NULL
	const CWeatherSetup  *getSetup(const char *name) const;
////////////////////////////////////////////////////////////////////////////////////
protected:
	/** For derivers : Build a new CWeatherSetup or derived class to be used by this manager
	  * Derivers may add additionnal infos into setups
	  */
	virtual CWeatherSetup *newWeatherSetup() const { return new CWeatherSetup; }
	/** For derivers : once a setup has been loaded, 'init' is called with that setup as a parameter, so
	  * additionnal setup can be done
	  */
	virtual void setupLoaded(CWeatherSetup * /* setup */) {}
private:
	typedef std::map<std::string, NLMISC::CSmartPtr<CWeatherSetup> >  TWeatherSetupMap;
	TWeatherSetupMap _WeatherSetupMap;
};

#endif
