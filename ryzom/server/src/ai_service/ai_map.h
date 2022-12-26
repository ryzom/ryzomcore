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



class CAIMap;

#ifndef RYAI_MAP_H
#define RYAI_MAP_H

#include "nel/misc/types_nl.h"
#include "ai_route.h"

/*
	This class implements the data set and related methods for a map.
	Map data includes collision maps and so on.
*/


class CAIMap  
{
public:
	// a method for releasing self cleanly before destruction
	void release() {}

	// the name of the map
	std::string Name;
};

#endif
