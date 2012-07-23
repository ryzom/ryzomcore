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
#include "scalable_time.h"
#include "time_client.h"
#include "nel/misc/time_nl.h"

// *************************************************************************************************
CScalableTime::CScalableTime()
{
	_LastPerformanceTime = NLMISC::CTime::getPerformanceTime();
	_ScaledPerformanceTime = (double) _LastPerformanceTime;
	_TimeScale = 1.f;
}

// *************************************************************************************************
TTime CScalableTime::getScaledLocalTime()
{
	update();
	return (TTime) (NLMISC::CTime::ticksToSecond ((NLMISC::TTicks) _ScaledPerformanceTime) * 1000.0);
}

// *************************************************************************************************
NLMISC::TTicks CScalableTime::getScaledPerformanceTime()
{
	update();
	return (NLMISC::TTicks) _ScaledPerformanceTime;
}

// *************************************************************************************************
void CScalableTime::setTimeScale(float scale)
{
	_TimeScale = std::max(0.f, scale);
}

// *************************************************************************************************
void CScalableTime::update()
{
	NLMISC::TTicks dt = NLMISC::CTime::getPerformanceTime() - _LastPerformanceTime;
	_LastPerformanceTime = NLMISC::CTime::getPerformanceTime();
	_ScaledPerformanceTime += (double) dt * (double) _TimeScale;
}