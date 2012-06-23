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



#ifndef RY_FOG_MAP_BUILD_H
#define RY_FOG_MAP_BUILD_H


/** Description of fog map parameters
  */
class CFogMapBuild
{
public:
	// Various kind of fog maps
	enum TMapType { Day = 0, Night, Dusk, Distance, Depth, NoPrecipitation, NumMap };
	// Name of each map
	std::string Map[NumMap];
	// Corner zone giving extent of the maps
	std::string ZoneMin;
	std::string ZoneMax;
public:
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

#endif
