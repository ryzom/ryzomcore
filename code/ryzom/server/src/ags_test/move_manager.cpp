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



#include "move_manager.h"
#include "nel/misc/config_file.h"
#include "nel/net/service.h"
#include "sheets.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "game_share/ryzom_entity_id.h"

//
//#include "nel/3d/u_instance_group.h"

#include "sheets.h"

// ugly
#include "game_share/ig_list.h"
#include "game_share/used_continent.h"

using namespace std;
using namespace NLMISC;
using namespace NLPACS;
using namespace NLNET;

namespace AGS_TEST
{

// the actor grid
CMoveManager::TObstacleGrid	CMoveManager::Grid;

// the continents
CContinentContainer			CMoveManager::Continents;

//
CMoveManager::TObstacleMap	CMoveManager::ObstacleMap;

//
CMoveManager::TPrimZoneMap	CMoveManager::PrimZoneMap;


// init
void	CMoveManager::init()
{
	// init continents
	Continents.init(30, 30, 8.0, 2, IService::getInstance()->WriteFilesDirectory);

	// load continents
	CUsedContinent::TUsedContinentCont usedCont = CUsedContinent::instance().getContinents();

	for (uint i=0; i<usedCont.size(); ++i)
	{
		Continents.loadContinent(usedCont[i].ContinentName, usedCont[i].ContinentName, usedCont[i].ContinentInstance);
		Continents.getMoveContainer(usedCont[i].ContinentInstance)->setAsStatic(0);
	}
/*	CConfigFile::CVar& cvUsedContinents = IService::getInstance()->ConfigFile.getVar("UsedContinents");
	uint	i;
	for (i=0; (sint)i<cvUsedContinents.size(); ++i)
		if (cvUsedContinents.asString(i) != "")
		{
			Continents.loadContinent(cvUsedContinents.asString(i), cvUsedContinents.asString(i), i);
			Continents.getMoveContainer(i)->setAsStatic(0);
		}
*/
/*
	// load trees ig
	for (i=sizeof(IGFiles)/sizeof(IGFiles[0]);i--;)
	{
		try
		{
			NL3D::UInstanceGroup *ig = NL3D::UInstanceGroup::createInstanceGroup(IGFiles[i]);
			if (ig == NULL)
				continue;

			uint k;
			for(k = 0; k < ig->getNumInstance(); ++k)
			{
				const string	&name = ig->getShapeName(k);
				sint			sz = name.size();
				if (sz >= 6 && strlwr(name.substr(sz-6, 6)) == ".shape")
				{
					const CSheets::CSheet	*sheet = CSheets::lookup(CSheetId(ig->getInstanceName(k)));
					CObstacle	obstacle;
					obstacle.Id = CEntityId(RYZOMID::flora, 0xDEADF00D, IService::getInstance()->getServiceId(), IService::getInstance()->getServiceId());
					obstacle.Position = ig->getInstancePos(k);
					obstacle.Radius = (sheet != NULL) ? sheet->Radius : 0.5f;
					Grid.insert(obstacle, obstacle.Position);
				}
			}

			delete ig;
		}
		catch (std::exception &e)
		{
			nlinfo(e.what());
		}
	}
*/
}

// update
void	CMoveManager::update()
{
	uint	i;
	for (i=0; (sint)i<Continents.size(); ++i)
		if (Continents.getMoveContainer(i) != NULL)
		{
			Continents.getMoveContainer(i)->evalCollision(1, 0);
			Continents.getMoveContainer(i)->evalCollision(1, 1);
		}
}


// process ai vision
void	CMoveManager::processAIVision(CMessage &msg)
{
	TObstacleMap::iterator	it;
	for (it=ObstacleMap.begin(); it!=ObstacleMap.end(); ++it)
		(*((*it).second)).Updated = false;

	uint64	AgentId;		// should be fakeid...
	msg.serial( AgentId );

	while (msg.getPos() < (sint32)(msg.length()))
	{
		CEntityId	id;
		TTime		time;
		CVectorD	pos;
		CVector		heading;
		float		speed;
		string		sheetId;

		// get all from message
		msg.serial(id, time, pos, heading, speed, sheetId);

		it = ObstacleMap.find(id);
		if (it == ObstacleMap.end())
		{
			// if not exists yet, creates an obstacle
			CObstacle	obstacle;
			obstacle.Id = id;
			obstacle.Position = CVector(pos);
			obstacle.Radius = 0.5f;
			obstacle.External = true;

			// get sheet from this sheetId
			const CSheets::CSheet	*sheet = CSheets::lookup(CSheetId(sheetId));
			if (sheet != NULL)
				obstacle.Radius = sheet->Radius;

			//
			nlinfo("New Object %s in IA vision at (%.1f,%.1f,%.1f), radius %.1f", id.toString().c_str(), pos.x, pos.y, pos.z, obstacle.Radius);

			// insert in grid
			TObstacleGrid::CIterator	ito = Grid.insert(obstacle, obstacle.Position);

			// insert in map
			pair<TObstacleMap::iterator, bool> res = ObstacleMap.insert(make_pair<CEntityId, TObstacleGrid::CIterator>(id, ito));
/*
			it = ObstacleMap.find(id);
			nlassert(it != ObstacleMap.end());
*/
			it = res.first;
		}
		else
		{
			(*((*it).second)).Position = pos;
			Grid.move((*it).second, pos);
		}

		(*((*it).second)).Updated = true;
	}

	// erase not updated obstacles
	for (it=ObstacleMap.begin(); it!=ObstacleMap.end(); )
	{
		if (!(*((*it).second)).Updated && (*((*it).second)).External)
		{
			nlinfo("Remove old object %s in IA vision", (*it).first.toString().c_str());
			TObstacleMap::iterator	itr = it++;
			Grid.remove((*itr).second);
			ObstacleMap.erase(itr);
		}
		else
		{
			++it;
		}
	}
}

// release
void	CMoveManager::release()
{
	// remove all continents
	uint	i;
	for (i=0; (sint)i<Continents.size(); ++i)
		Continents.removeContinent(i);

	Continents.clear();
}

} // AGS_TEST

/* End of move_manager.cpp */
