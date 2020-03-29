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



#ifndef RY_WEATHER_H
#define RY_WEATHER_H

#include "game_share/cdb.h"
#include "game_share/cdb_leaf.h"
#include "game_share/cdb_branch.h"
#include "game_share/cdb_synchronised.h"

#include "game_share/weather_cfg.h"

#include "game_share/property_base_type.h"
#include "game_share/property_container.h"
#include "game_share/container_property_receiver.h"

class CWeather
{
public:
	enum EDayCycle
	{
		dawn = 0,
		day,
		twilight,
		night
	};

	// Constructor
	static void init();

	// Update Ryzom time
	static void updateRyzomTime();

	// Get Ryzom Time
	static float getRyzomTime() { return _RyzomTime.getValue(); }

	// Get Day Cycle
	static uint8 getDayCycle() { return _DayCycle.getValue(); } 

	// get string of day cycle
	static const std::string& toString( uint8 );
	
private:
	static CPropertyBaseType< float >	_RyzomTime;
	static CPropertyBaseType< uint8 >	_DayCycle;
};
#endif // RY_WEATHER_H

/* End of file weather.h */
