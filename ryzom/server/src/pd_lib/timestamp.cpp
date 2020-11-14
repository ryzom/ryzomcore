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

#include "timestamp.h"


/*
 * Constructor
 */
CTimestamp::CTimestamp()
{
	//setToCurrent();
	//memset(&_Timestamp, 0, sizeof(_Timestamp));
	_Time = 0;
}

uint	gmtTime(uint year, uint month, uint day, uint hour, uint minute, uint second)
{
	const uint	nbisyear = 365*86400;			// non bissextile years
	const uint	bisyear = 366*86400;			// bissextile years

	const uint	nbismonths[] = {
									86400*31,	// jan
									86400*28,	// feb
									86400*31,	// mar
									86400*30,	// apr
									86400*31,	// may
									86400*30,	// jun
									86400*31,	// jul
									86400*31,	// aug
									86400*30,	// sep
									86400*31,	// oct
									86400*30,	// nov
									86400*31	// dec
								};
	const uint	bismonths[] = {
									86400*31,	// jan
									86400*29,	// feb
									86400*31,	// mar
									86400*30,	// apr
									86400*31,	// may
									86400*30,	// jun
									86400*31,	// jul
									86400*31,	// aug
									86400*30,	// sep
									86400*31,	// oct
									86400*30,	// nov
									86400*31	// dec
								};

	uint		gmt = 0;

	uint	i;
	for (i=1970; i<year; ++i)
		gmt += (  ((i&3) == 0 && ((i%100) != 0 || (i%400) == 0))  ?  bisyear : nbisyear  );

	bool	bissext = ( (year&3) == 0 && ((year%100) != 0 || (year%400) == 0));
	for (i=0; i<month-1; ++i)
		gmt += (bissext ? bismonths[i] : nbismonths[i]);

	gmt += 86400*(day-1);
	gmt += 3600*hour;
	gmt += 60*minute;
	gmt += second;

	return gmt;
}


bool CTimestamp::fromString(const char* str)
{
	uint	year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	uint	t;

	if (str[0] == '#' || str[0] == '+' || str[0] == '-')
	{
		if (str[0] == '#')
		{
			setToCurrent();
			++str;
		}

		if (_Time == 0)
			setToCurrent();

		sint	dt;
		NLMISC::fromString(std::string(str), dt);

		++str;
		while (*str >= '0' && *str <= '9')
			++str;

		if (*str == 'd' || *str == 'D')
			dt *= 86400;
		else if (*str == 'h' || *str == 'H')
			dt *= 3600;
		else if (*str == 'm' || *str == 'M')
			dt *= 60;

		_Time += dt;
	}
	else if (sscanf(str, "%d.%d.%d.%d.%d.%d", &year, &month, &day, &hour, &minute, &second) == 6 ||
			 sscanf(str, "%d.%d.%d.%d.%d", &year, &month, &day, &hour, &minute) == 5)
	{
		_Time = gmtTime(year, month, day, hour, minute, second);
	}
	else if (sscanf(str, "%8X", &t) == 1)
	{
		_Time = t;
	}

	return validate();
}

