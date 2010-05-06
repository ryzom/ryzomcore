/** \file physics.h
 * Physics computation for throwing snowballs
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef PHYSICS_H
#define PHYSICS_H

//
// Includes
//

#include <nel/misc/types_nl.h>

#include <nel/misc/vector.h>
#include <nel/misc/time_nl.h>

namespace SBCLIENT {

//
// External classes
//

class CTrajectory
{
	NLMISC::CVector		_StartPosition;
	NLMISC::CVector		_EndPosition;
	float				_Speed;
	NLMISC::TLocalTime		_StartTime;
	NLMISC::TLocalTime		_StopTime;
	float				_Distance;

public:
	void				init(const NLMISC::CVector &position, const NLMISC::CVector &target, float speed, NLMISC::TLocalTime startTime)
	{
		_StartPosition = position;
		_EndPosition = target;
		_Speed = speed;
		_StartTime = startTime;
		_Distance = (_EndPosition-_StartPosition).norm();
		_StopTime = (NLMISC::TLocalTime)(_Distance / _Speed + _StartTime);
	}

//	void				compute(const NLMISC::CVector &position, const NLMISC::CVector &target, float speed, NLMISC::TTime startTime);

	NLMISC::CVector		eval(NLMISC::TLocalTime t) const;
	NLMISC::CVector		evalSpeed(NLMISC::TLocalTime t) const;

	NLMISC::TLocalTime	getStartTime() const { return _StartTime; }
	NLMISC::CVector		getStartPosition() const { return _StartPosition; }
	NLMISC::TLocalTime	getStopTime() const { return _StopTime; }
};

} /* namespace SBCLIENT */

#endif // PHYSICS_H

/* End of physics.h */
