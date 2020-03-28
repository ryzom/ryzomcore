// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef PHYSICS_H
#define PHYSICS_H

//
// Includes
//

#include <nel/misc/vector.h>
#include <nel/misc/time_nl.h>

//
// External classes
//

class CTrajectory
{
	NLMISC::CVector		_StartPosition;
	NLMISC::CVector		_EndPosition;
	float				_Speed;
	NLMISC::TTime		_StartTime;
	NLMISC::TTime		_StopTime;
	float				_Distance;

public:
	void				init(const NLMISC::CVector &position, const NLMISC::CVector &target, float speed, NLMISC::TTime startTime)
	{
		_StartPosition = position;
		_EndPosition = target;
		_Speed = speed;
		_StartTime = startTime;
		_Distance = (_EndPosition-_StartPosition).norm();
		_StopTime = (NLMISC::TTime)(_Distance/_Speed*1000.0+_StartTime);
	}

//	void				compute(const NLMISC::CVector &position, const NLMISC::CVector &target, float speed, NLMISC::TTime startTime);

	NLMISC::CVector		eval(NLMISC::TTime t) const;
	NLMISC::CVector		evalSpeed(NLMISC::TTime t) const;

	NLMISC::TTime		getStartTime() const { return _StartTime; }
	NLMISC::CVector		getStartPosition() const { return _StartPosition; }
	NLMISC::TTime		getStopTime() const { return _StopTime; }
};

#endif // PHYSICS_H

/* End of physics.h */
