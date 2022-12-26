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




#ifndef GD_ACTOR_MANAGER_H
#define GD_ACTOR_MANAGER_H


// Nel
#include "nel/misc/types_nl.h"
//#include "nel/net/message.h"
//#include "nel/net/unified_network.h"

// Local headers
#include "actor.h"
#include "actor_group.h"

#include <set>

namespace AGS_TEST
{

/**
 * Singleton, for managing the actors in a scene
 * \author Sadge
 * \author Nevrax France
 * \date 2002
 */
class CActorManager
{
public:
	// initialisation and housekeeping for the singleton
	static void init();
	static void release();

	// methods for dealing with tardy connection of a key service
	static void reconnectEGS(uint8 serviceId);
	static void reconnectIOS(uint8 serviceId);
	static void reconnectGPMS(uint8 serviceId);

	static void	addVisionService(uint serviceId) { _visionHandlingServices.insert(serviceId); }
	static void removeVisionService(uint serviceId) { _visionHandlingServices.erase(serviceId); }

	// methods managing the actor set
	// getActor() returns NULL if the actor doesn't exist
	static CActor *newActor(const std::string &type, const std::string &name);
	static void   killActor(const std::string &name);
	static CActor *getActor(const std::string &name);
	static CActor *getActor(const NLMISC::CEntityId &id);
	static CActor *getActor(unsigned index);
	static uint32 numActors() { return _actors.size(); }

	// Movement script management
	static void update();

private:
	// forbid instantiation
	CActorManager();

	// changed by ben -- std::vector<CActor> replaced by std::vector<CActor*> because highly unsafe (reallocs might occur)
	// to avoid memory fragmentation, use NLMISC::CBlockMemory<CActor> instead
	static std::vector<CActor*> _actors;
	static int _nextActorID;

	static std::set<uint>	_visionHandlingServices;

public:
	// changed by ben -- std::vector<CActorGroup> replaced by std::vector<CActorGroup*> because highly unsafe (reallocs might occur)
	// to avoid memory fragmentation, use NLMISC::CBlockMemory<CActorGroup> instead
	static std::vector<CActorGroup*> _actorGroups;
	static CActorGroup *			_defaultActorGroup;

	static CActorGroup * newActorGroup(const std::string &name);
	static CActorGroup * getActorGroup(const std::string &name);
	static CActorGroup * getActorGroup(unsigned index);
	static void removeActorGroup(const std::string &name);

	static uint32 numActorGroups() { return _actorGroups.size(); }

	static CActorGroup*	getDefaultActorGroup() { return _defaultActorGroup; }

	// move all actors from a group to another group
	static void			setActorsToGroup(const std::string &sourceActorGroup, const std::string &destActorGroup);

	static sint32	getNextActorId() { return ++_nextActorID; }

};

} // end of namespace AGS_TEST

#endif // GD_ACTOR_MANAGER_H
/* End of actor_manager.h */
