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



#ifndef RY_LIGHT_CYCLE_H
#define RY_LIGHT_CYCLE_H

#include "season.h"

namespace NLGEORGES
{
	class UFormElm;
}

/** Description of light cycle for a single season
  */
struct CSeasonLightCycle
{
	float DayHour;
	float DayToDuskHour;
	float DuskToNightHour;
	float NightHour;
	float NightToDayHour;
///////////////////////////////////////////////////////////////
	// ctor
	CSeasonLightCycle();
	// build from a Georges form
	void  build(const NLGEORGES::UFormElm &item);
	// serial in a stream
	void  serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(DayHour, DayToDuskHour, DuskToNightHour, NightHour, NightToDayHour);
	}
};


/** Description of a complete light cycle for each season
  */
struct CLightCycle
{
	float             RealDayLength;	 // real length of the day, in seconds
	float             NumHours;          // number of ryzom hours in a day
	uint32			  MaxNumColorSteps;  // the max number of color steps
	CSeasonLightCycle SeasonLightCycle[EGSPD::CSeason::Invalid]; // description of each season
///////////////////////////////////////////////////////////////
	// ctor
	CLightCycle();
	// build from a Georges form
	void  build(const NLGEORGES::UFormElm &item);
	// Build from a sheet file
	void  build(const char *sheetName);
	//
	void  serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(RealDayLength, NumHours, MaxNumColorSteps);
		for(uint k = 0; k < EGSPD::CSeason::Invalid; ++k)
		{
			f.serial(SeasonLightCycle[k]);
		}
	}

};




#endif

