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



class CAIRouteBeeline;

#ifndef RYAI_ROUTE_BEELINE_H
#define RYAI_ROUTE_BEELINE_H

#include "nel/misc/types_nl.h"
#include "ai_route.h"

/*
	This class implements beeline routes towards points - it is the most
	basic form of pathfinding imaginable
*/


class CAIRouteBeeline: public CAIRoute
{
public:
	// ctors and dtors -------------------------------------------------
	CAIRouteBeeline(CAIPos destination)
	{
		_destination=destination;
	}

public:
	// respecting the inheritted interface -----------------------------

	virtual double getDirection(CAIPos currentCoord);
	virtual bool haveArrived(CAIPos currentCoord,uint dist);

private:
	CAIPos _destination;
};


//----------------------------------------------------------------------
// methods

double CAIRouteBeeline::getDirection(CAIPos currentCoord)
{
	return currentCoord.directionTo(_destination);
}

double CAIRouteBeeline::haveArrived(CAIPos currentCoord,uint dist)
{
	return currentCoord.distTo(_destination)<=dist;
}

#endif
