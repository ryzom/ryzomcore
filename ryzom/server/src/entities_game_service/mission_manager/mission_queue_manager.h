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



#ifndef EGS_MISSION_QUEUE_MANAGER_H
#define EGS_MISSION_QUEUE_MANAGER_H

// misc
#include "nel/misc/common.h"
#include "nel/misc/time_nl.h"
#include "mission_queue.h"
#include "mission_step_queue.h"
#include "ai_alias_translator.h"
#include <map>

#include "game_share/persistent_data.h"

class CCharacter;
class IMissionStepTemplate;

class CQueueVect
{
public:
	std::vector<uint32> QueueIds;

	void addQueue(uint32 id) 
	{ 
		if (find(id) == false)
			QueueIds.push_back(id);
	}

	void removeQueue(uint32 id) 
	{ 
#ifdef NL_DEBUG
		nlassert(find(id) == true);
#endif
		const uint size = (uint)QueueIds.size();
		for ( uint i = 0 ; i < size ; ++i )
		{
			if (QueueIds[i] == id)
			{
				QueueIds[i] = QueueIds.back();
				QueueIds.pop_back();
				return;
			}
		}
	}

	bool find(uint32 id) const
	{ 
		const uint size = (uint)QueueIds.size();
		for ( uint i = 0 ; i < size ; ++i )
		{
			if (QueueIds[i] == id)
			{
				return true;
			}
		}
		return false;
	}

	DECLARE_PERSISTENCE_METHODS
};

/**
 * CMissionQueueManager
 *
 * \author David 'Malkav' Fleury
 * \author Nevrax France
 * \date December 2005
 */
class CMissionQueueManager
{
	typedef std::map< NLMISC::CEntityId, CQueueVect> TPlayerQueues;
public:

	static CMissionQueueManager *getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CMissionQueueManager;
		return _Instance;
	}

	/// init methodd
	void init();

	/// called when a characater has been loaded
	void characterLoadedCallback(CCharacter *c);

	/// method called each tick
	void tickUpdate();

	/// save to file method
	void saveToFile();

	/// add a plyer in a queue or create a queue if not already exists
	void addPlayerInQueue(const NLMISC::CEntityId &id, CMission *mission, uint16 stepIndex, const std::string &queueName, uint32 timer);

	/// remove a player from queue (timer elapsed or he is out of the critical area)
	void removePlayerFromQueue(const NLMISC::CEntityId &id, uint32 queueId);

	/// a player enters a critical area in specified queue
	void playerEntersCriticalArea(const NLMISC::CEntityId &id, uint32 queueId, bool accept);

	/// player wakes up for missionAlias
	void playerWakesUp(const NLMISC::CEntityId &id, TAIAlias missionAlias);

	/// player wakes up for queueId
	//void playerWakesUp(const NLMISC::CEntityId &id, uint32 queueId);

	/// disconnect player
	void disconnectPlayer(const CCharacter *c);

	/// get queue If from it's name
	uint32 getQueueId( const std::string &name ) const;

	/// get player position in given queue, return false if player isn't in queue or queue not found
	bool getPlayerPositions(uint32 queueId, const NLMISC::CEntityId &id, uint16 &position, uint16 &positionOnline, bool &hasPlayerInCritZone) const;

	/// DEBUG ! dump all queues
	void dump();

	inline bool isInit() { return _InitOk; }

	DECLARE_PERSISTENCE_METHODS

private:
	/// private ctor (as it's a singleton)
	CMissionQueueManager();

private:
	// The Singleton instance
	static CMissionQueueManager *_Instance;

	/// map queue from their Id
	std::map< uint32, CMissionQueue >	_Queues;

	/// var used to attribute an id number to queues. Invalid ID = 0
	uint32									_QueueIdCounter;

	/// map queue name to id
	std::map< std::string, uint32 >	_QueueNamesToIds;

	/// Map here player to the queues in which they are referenced (for faster finds and coherency checks at character loading)
	TPlayerQueues					_PlayerQueues;

	// flag at true if init done
	bool							_InitOk;
};

#endif // EGS_MISSION_QUEUE_MANAGER_H

