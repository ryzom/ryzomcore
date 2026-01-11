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

#ifndef RY_VERBOSE_CLOCK_H
#define RY_VERBOSE_CLOCK_H

#include "nel/misc/time_nl.h"

namespace R2
{


class CVerboseClock
{
public:
	CVerboseClock(const std::string &msg);
	~CVerboseClock();
private:
	std::string _Msg;
	NLMISC::TTime _StartTime;
};

class CPreciseClock
{
public:
	CPreciseClock(const std::string &msg);
	~CPreciseClock();
private:
	std::string _Msg;
	NLMISC::TTicks _StartTime;
};


} // R2

#endif
