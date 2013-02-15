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
#include "weather_setup_sheet_base.h"
#include "weather_manager.h"
#include "nel/misc/path.h"


using namespace NLMISC;
using namespace std;


//================================================================================================
void CWeatherManager::init(const std::vector<const CWeatherSetupSheetBase *> &sheets, const std::vector<std::string> &sheetNames)
{
	nlassert(sheets.size() == sheetNames.size());
	for(uint k = 0; k < sheets.size(); ++k)
	{
		if (sheets[k])
		{
			std::string id = NLMISC::strlwr(NLMISC::CFile::getFilenameWithoutExtension(sheetNames[k]));
			CWeatherSetup *ws = newWeatherSetup();
			if (ws)
			{
				if (_WeatherSetupMap[id] != 0)
				{
					nlwarning("Duplicated weather setup : %s", id.c_str());
				}
				_WeatherSetupMap[id] = ws;
				(CCloudStateSheet &) ws->CloudState   = sheets[k]->CloudState; // copy parent obj part
				(CWeatherStateSheet &) ws->WeatherState = sheets[k]->WeatherState;
				ws->SetupName    = sheets[k]->SetupName;
				// copy the setup name in the weather state for blending ops
				ws->WeatherState.BestSetupName = ws->SetupName;
				// do additionnal init if needed
				setupLoaded(ws);
			}
		}
	}
}

//================================================================================================
CWeatherManager::~CWeatherManager()
{
	release();
}

//================================================================================================
void CWeatherManager::release()
{
	NLMISC::contReset(_WeatherSetupMap);
}
//================================================================================================
const CWeatherSetup *CWeatherManager::getSetup(const char *name) const
{
	std::string id = NLMISC::strlwr(CFile::getFilenameWithoutExtension(name));
	TWeatherSetupMap::const_iterator it = _WeatherSetupMap.find(id);
	if (it == _WeatherSetupMap.end()) return NULL;
	return it->second;
}
