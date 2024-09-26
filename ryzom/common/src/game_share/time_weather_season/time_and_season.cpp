// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2016  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"
#include "time_and_season.h"


namespace MONTH
{
	NL_BEGIN_STRING_CONVERSION_TABLE (EMonth)
		NL_STRING_CONVERSION_TABLE_ENTRY(Winderly)
		NL_STRING_CONVERSION_TABLE_ENTRY(Germinally)
		NL_STRING_CONVERSION_TABLE_ENTRY(Folially)
		NL_STRING_CONVERSION_TABLE_ENTRY(Floris)
		NL_STRING_CONVERSION_TABLE_ENTRY(Medis)
		NL_STRING_CONVERSION_TABLE_ENTRY(Thermis)
		NL_STRING_CONVERSION_TABLE_ENTRY(Harvestor)
		NL_STRING_CONVERSION_TABLE_ENTRY(Frutor)
		NL_STRING_CONVERSION_TABLE_ENTRY(Fallenor)
		NL_STRING_CONVERSION_TABLE_ENTRY(Pluvia)
		NL_STRING_CONVERSION_TABLE_ENTRY(Mystia)
		NL_STRING_CONVERSION_TABLE_ENTRY(Nivia)
		NL_STRING_CONVERSION_TABLE_ENTRY(UNKNOWN)
	NL_END_STRING_CONVERSION_TABLE(EMonth, ConversionType, UNKNOWN)

	///
	EMonth toMonth( const std::string &str )
	{
		return ConversionType.fromString(str);
	}

	///
	const std::string& toString( EMonth month )
	{
		return ConversionType.toString(month);
	}

}; // MONTH


namespace WEEKDAY
{
	NL_BEGIN_STRING_CONVERSION_TABLE (EWeekDay)
		NL_STRING_CONVERSION_TABLE_ENTRY(Prima)
		NL_STRING_CONVERSION_TABLE_ENTRY(Dua)
		NL_STRING_CONVERSION_TABLE_ENTRY(Tria)
		NL_STRING_CONVERSION_TABLE_ENTRY(Quarta)
		NL_STRING_CONVERSION_TABLE_ENTRY(Quinteth)
		NL_STRING_CONVERSION_TABLE_ENTRY(Holeth)
		NL_STRING_CONVERSION_TABLE_ENTRY(UNKNOWN)
	NL_END_STRING_CONVERSION_TABLE(EWeekDay, ConversionType, UNKNOWN)

	///
	EWeekDay toWeekDay( const std::string &str )
	{
		return ConversionType.fromString(str);
	}

	///
	const std::string& toString( EWeekDay day )
	{
		return ConversionType.toString(day);
	}

}; // WEEKDAY


void CRyzomTime::updateRyzomClock(uint32 gameCyle)
{
	static const float ticksPerHour = (float)RYZOM_HOURS_IN_TICKS;

	uint32 totalTicks = gameCyle + _TickOffset;
	uint32 days = totalTicks / RYZOM_DAY_IN_TICKS;
	uint32 dayCycle = totalTicks - (days * RYZOM_DAY_IN_TICKS);
	// Avoid rollover for low amount of days
	if(days >= RYZOM_START_SPRING)
		days -= RYZOM_START_SPRING;
	else
		days = 0;
	float hours = (float)dayCycle / ticksPerHour;

	_RyzomDay = days;
	_RyzomTime = hours;
}

/* end of file */
