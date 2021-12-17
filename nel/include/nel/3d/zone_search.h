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

#ifndef NL_ZONE_SEARCH_H
#define NL_ZONE_SEARCH_H

#include <list>
#include "nel/misc/types_nl.h"


namespace NL3D
{


/**
 * CZoneSearch is a class that determine which zone cooresponding to a coordinate
 * and zones are around a position
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2000
 */
class CZoneSearch
{
public:

	/// Constructor
	CZoneSearch();

	/**
	* Get the zone name corresponding to coordinate
	* \param x is axis X coordinate (in meters)
	* \param y is axis Y coordinate (in meters)
	* \param cx is axis X coordinate of center area (in meters)
	* \param cy is axis Y coordinate of center area (in meters)
	* \return a pair of the zone name and square distance between zone and center area (in zone unit)
	*/
	std::pair<std::string, uint32> getZoneName(uint x, uint y, uint cx, uint cy);

	/*
	* Get a list of zone name around a position
	* \param x is axis X ccordinate (in meter)
	* \param y is axis Y coordinate (in meter)
	* \param sizeArea is area of zone research (in meter)
	* \param l is a reference to a list of pair of string and uint32
	* \return a liste contained names of all zones around indicated position, square distance between zones and indicated position
	*/
	void getListZoneName(uint x, uint y, uint sizeArea, std::list< std::pair<std::string, uint32> >& l);

	uint16 getZoneId (uint x, uint y) const;
	void getZonePos (uint16 zoneId, uint &x, uint &y) const;
	void getListZoneId (uint x, uint y, uint sizeArea, std::vector<uint16> &l, const std::vector<uint16> *validZoneIds = NULL);
	std::string getZoneNameFromId (uint16 zoneid);

private:
	// Number zones on X axis of landscape
	uint _NbZoneX;

	/// Number zones on Y axis of landscape
	uint _NbZoneY;

	// Size X of one zone (in meters)
	uint _SizeZoneX;

	// Size X of one zone (in meters)
	uint _SizeZoneY;
};


} // NL3D


#endif // NL_ZONE_SEARCH_H

/* End of zone_search.h */
