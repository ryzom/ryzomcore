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

#ifndef RY_BUILDING_ENUMS_H
#define RY_BUILDING_ENUMS_H

namespace ROOM_RESTRICTION
{
	// restriction to have a valid destination
	enum TRestriction
	{
		Rm_Fight,
		Rm_Magic,
		Rm_Harvest,
		Rm_Craft,
		Unknown,
	};
	
	TRestriction fromString( const std::string & str );
}

namespace BUILDING_TYPES
{
	// restriction to have a valid destination
	enum TBuildingType
	{
		Common,
		Player,
		Guild,
		Unknown,
	};
	
	TBuildingType fromString( const std::string & str );
}



#endif // RY_BUILDING_ENUMS_H

/* End of building_enums.h */

