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



#ifndef NL_MOVE_MANAGER_H
#define NL_MOVE_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/net/message.h"
#include "nel/misc/entity_id.h"

#include "nel/ligo/primitive.h"

#include "move_grid.h"
#include "game_share/continent_container.h"

#include <map>

namespace AGS_TEST
{

class CObstacle
{
public:
	CObstacle() : External(false), Updated(true) {}
	NLMISC::CEntityId	Id;
	NLMISC::CVector		Position;
	float				Radius;
	bool				External;
	bool				Updated;
};

 /**
 * Manager of a unique move grid
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2002
 */
class CMoveManager
{
public:
	/// The type of grid
	typedef CMoveGrid<CObstacle, 256, 16000>				TObstacleGrid;

	/// A map of obstacle
	typedef std::map<NLMISC::CEntityId, TObstacleGrid::CIterator>	TObstacleMap;

	/// A map of prim zone
	typedef std::map<std::string, NLLIGO::CPrimZone>		TPrimZoneMap;

public:
	/// The grid
	static TObstacleGrid		Grid;

	/// The continent container
	static CContinentContainer	Continents;

	/// The obstacle map
	static TObstacleMap			ObstacleMap;

	/// The prim zone map
	static TPrimZoneMap			PrimZoneMap;

public:

	/// init
	static void			init();

	/// update
	static void			update();

	/// process ia vision from gpms
	static void			processAIVision(NLNET::CMessage &msg);

	/// release
	static void			release();

	/// Add a prim zone in map
	static void			addPrimZone(const NLLIGO::CPrimZone &primZone)
	{
		if (PrimZoneMap.find(primZone.getName()) == PrimZoneMap.end())
		{
			PrimZoneMap.insert(std::make_pair<std::string, NLLIGO::CPrimZone>(primZone.getName(), primZone));
			nlinfo("Using PrimZone '%s'", primZone.getName().c_str());
		}
		else
		{
			nlwarning("PrimZone '%s' already added, abort...", primZone.getName().c_str());
		}
	}

	/// get a prim zone by its name
	static NLLIGO::CPrimZone *getPrimZone(const std::string &zoneName)
	{
		TPrimZoneMap::iterator	it = PrimZoneMap.find(zoneName);
		if (it == PrimZoneMap.end())
		{
			nlwarning("Couldn't find PrimZone '%s', abort...", zoneName.c_str());
			return NULL;
		}
		return &((*it).second);
	}


public:
};

} // AGS_TEST

#endif // NL_MOVE_MANAGER_H

/* End of move_manager.h */
