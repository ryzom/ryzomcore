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



#ifndef RY_MISSION_QUEUE_H
#define RY_MISSION_QUEUE_H

#include "nel/misc/types_nl.h"
#include <list>
#include "game_share/persistent_data.h"
#include "ai_alias_translator.h"
#include "mission_step_queue.h"

class CCharacter;
class IMissionStepTemplate;

typedef NLMISC::CRefPtr<CMission> CMissionRefPtr;

/**
 * class used to manage a waiting entity in queue
 * \author David Fleury
 * \author Nevrax France
 * \date 2005
 */
class CWaitingEntity
{
public:
	/// \ctor
	CWaitingEntity(): PositionOnline(0), Position(0), Awake(true), Online(false), Mission(NULL)
	{}

	DECLARE_PERSISTENCE_METHODS

	static CWaitingEntity Unknown;
	
public:
	///id of the entity
	NLMISC::CEntityId	Id;
	///last connection date (computer time)
	uint32				LastConnectionDate;
	/// current online status
	bool				Online;
	/// current awake status
	bool				Awake;
	/// current number of online players before this one
	uint16				PositionOnline;
	/// current number of players before this one
	uint16				Position;
	/// pointer on related mission
	CMissionRefPtr		Mission;
};


/**
 * class used to manage a waiting queue in mission
 * \author David Fleury
 * \author Nevrax France
 * \date 2005
 */
class CMissionQueue
{
public:
	/// \ctor
	CMissionQueue() : _MaxTimeInCriticalPart(0), _CreateQueueStepIndex(0), _MissionAlias(0)
	{}

	/// \ctor
	CMissionQueue(const std::string &name, uint32 id, uint32 maxTime, TAIAlias missionAlias, uint16 stepIndex) : _QueueName(name), _QueueId(id),
		_MaxTimeInCriticalPart(maxTime), _CreateQueueStepIndex(stepIndex), _MissionAlias(missionAlias)
	{}

	/// \dtor
	~CMissionQueue()
	{}

	/// method called each tick
	void tickUpdate();

	/// called when a character has been loaded, return true if character was in queue
	bool characterLoadedCallback(CCharacter *c);

	/// clear queues from players who have been away for too long
	void clearOfflinePlayer();

	///remove a player from queue
	void removePlayer(const NLMISC::CEntityId &id);

	///remove a player from queue
	void disconnectPlayer(const NLMISC::CEntityId &id);

	/// add player in queue
	void addPlayer(const NLMISC::CEntityId &id, CMission *mission, bool forceTopOfQueue = false);

	/// a player enters a critical area 
	void playerEntersCriticalArea(const NLMISC::CEntityId &id, bool accept);

	/// player leave critical area
	void playerLeavesCriticalArea();

	/// players wakes up
	void changePlayerAwakeState(const NLMISC::CEntityId &id, bool wakeUp);

	/// get associated mission alias 
	inline TAIAlias getMissionAlias() const { return _MissionAlias; }

	/// get given player position and online position, return false if player cannot be found
	bool getPlayerPositions(const NLMISC::CEntityId &id, uint16 &position, uint16 &onlinePosition, bool &hasPlayerInCritZone) const;

	/// get queue id
	inline uint32 getId() const { return _QueueId; }

	/// get queue name
	inline const std::string &getName() const { return _QueueName; }

	/// DEBUG ! dump queue
	void dump();

	DECLARE_PERSISTENCE_METHODS

private:
	/// get next candidate entity
	const CWaitingEntity &getNextCandidateInQueue() const;

	/// post apply method
	void postApply();

	/// decrease/increase the nb online waiters for entities in queue, starts at given iterator
	void changeNbOnlineWaiters( std::list<CWaitingEntity>::iterator itStart, bool inc );

private:
	///\name queue parameters
	//@{
	/// queue name
	std::string			_QueueName;
	/// queue Id
	uint32				_QueueId;
	/// index of the create queue step in mission
	uint16				_CreateQueueStepIndex;
	/// related mission alias
	TAIAlias			_MissionAlias; 
	/// timer, max duration a player can be in the critical part, in ticks
	uint32				_MaxTimeInCriticalPart;
	//@}

	/// waiting entities
	std::list<CWaitingEntity>	_Entities;

	///\name asked entity
	//@{
	/// when asking a player if he wants to take the step, keep timer (he must anwser before timer ends)
	uint32				_StepAnswerTimer;
	/// id of the asked entity
	NLMISC::CEntityId	_AskedEntityId;
	//@}

	///\name Entity in critical area
	//@{
	/// Id of the player currently in critical area
	NLMISC::CEntityId	_CriticalPartEntityId;
	/// the date when the player in critical area will fail his mission
	uint32				_CriticalPartTimer;
	//@}

	/** 
	 * crash/shutdown handler, when an entity was in critical part and EGS has been shutdown for any reason
	 * we must move the player from critical part to head of the queue, and we keep here the entityId : when player will log again, 
	 * we also must update his current mission step to new one.
	 */
	std::vector<NLMISC::CEntityId>	_PlayersToRollBackInSteps;
};



#endif // RY_MISSION_QUEUE_H //
