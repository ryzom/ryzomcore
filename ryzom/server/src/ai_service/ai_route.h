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



//class CAIRoute;

#ifndef RYAI_ROUTE_H
#define RYAI_ROUTE_H

//#include "nel/misc/types_nl.h"
#include "ai_share/ai_coord.h"
#include "ai_pos.h"

/*
	This is the virtual base class for abstracting pathfinding algorithms
	Routes are generaly spawned by CAIPlace classes (the 'place' is the destination)
*/


class CAIRoute
{
public:
	virtual ~CAIRoute() {}
	// get direction vector from current coordinate
	virtual double getDirection(CAIPos currentCoord)=0;

	// have we arrived at the destination
	// - dist defines the distance that the entity moves/ tick
	// returns true if distance from currentCoord to dest <= dist
	virtual bool haveArrived(CAIPos currentCoord,uint dist)=0;
};

#endif
