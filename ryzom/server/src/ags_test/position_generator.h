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




#ifndef GD_POSITION_GENERATOR_H
#define GD_POSITION_GENERATOR_H


#include <string>
#include "nel/misc/types_nl.h"
#include "nel/ligo/primitive.h"


namespace AGS_TEST
{

class CPositionGenerator
{
public:
	static void setPositionInMM(int x, int y);			// x and y are in milimeters
	static void setPosition(int x, int y);				// x and y are in meters
	static void setPosition(std::string spawnName);		// name of a spawn point or spawn zone
	static void setPattern(const std::string &pattern);
	static void setSpacing(int spacing);

	static void addSpawnPoint(const NLLIGO::CPrimPoint &point)	{ _spawnPoints[point.getName()]=point; }
	static void addSpawnZone(const NLLIGO::CPrimZone &zone);

	static void init();
	static void next(int &x, int &y);

	static std::string getSpawnName();

private:
	class CSpawnZone
	{
	public:
		uint Next;
		std::vector<NLMISC::CVector> Points;

		CSpawnZone(): Next(0) {}
		const NLMISC::CVector& getNext()
		{
			if (Next>=Points.size()) Next=0;
			return Points[Next++];
		}
	};

private:
	static int _x, _y;
	static int _dx, _dy;
	static int _spacing;
	static std::string _pattern;
	static std::string _spawnZoneName;

	typedef std::map<std::string, CSpawnZone> TSpawnZoneMap;
	typedef std::map<std::string, NLLIGO::CPrimPoint> TSpawnPointMap;

	static TSpawnZoneMap	_spawnZones;
	static TSpawnPointMap	_spawnPoints;

	// for grid
	static int _linelen, _leftonline;
};


} // end of namespace AGS_TEST

#endif // GD_POSITION_GENERATOR_H
