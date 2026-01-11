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



#ifndef CL_SCALABLE_TIME_H
#define CL_SCALABLE_TIME_H

#include "nel/misc/time_nl.h"

/** Allow to get the current time, and to change the time factor
  */
class CScalableTime
{
public:
	CScalableTime();
	// get local time with time scale applied
	NLMISC::TTime  getScaledLocalTime();
	// get performance time with time scale applied;
	NLMISC::TTicks getScaledPerformanceTime();
	void		   setTimeScale(float scale);
	float		   getTimeScale() const { return _TimeScale; }
private:
	NLMISC::TTicks  _LastPerformanceTime;
	double          _ScaledPerformanceTime;
	float			_TimeScale;
	void		   update();
};






#endif