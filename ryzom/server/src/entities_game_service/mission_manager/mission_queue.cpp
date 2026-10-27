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

#include "stdpch.h"
#include "mission_queue.h"
#include "mission_queue_manager.h"
#include "player_manager/character.h"
#include "mission_step_template.h"
#include "mission_template.h"
#include "mission_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "team_manager/team_manager.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "mission_team.h"
#include "mission_guild.h"
#include "mission_step_queue.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern CPlayerManager	PlayerManager;

CWaitingEntity CWaitingEntity::Unknown;

CVariable<uint32> MaxDisconnectTimeForQueue("egs", "MaxDisconnectTimeForQueue", "max time a player can stay disconnected wihtout being removed from it's mission queues, in seconds (default = 1 week)", 604800, true );
CVariable<uint32> MinSleepingTime("egs", "MinSleepingTime", "min 'sleeping' time, the duration after which a player who has set himself to sleep mode cannot awake. in seconds (default = 2 min)", 120, true );
CVariable<uint32> MaxEnterCriticalAnswerTime("egs", "MaxEnterCriticalAnswerTime", "time the player has to answer to the server request about entering a mission critical ppart, in ticks (default = 60 s)", 600, true );

// ----------------------------------------------------------------------------
// whether one of a mission's active steps is the queue_start step of a queue
static bool isWaitingIn(const CMission *mission, const std::string &queueName)
{
	const CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(mission->getTemplateId());
	if (templ == nullptr)
		return false;

	for (map<uint32, EGSPD::CActiveStepPD>::const_iterator it = mission->getStepsBegin(); it != mission->getStepsEnd(); ++it)
	{
		const uint32 index = (*it).second.getIndexInTemplate() - 1;
		if (index >= templ->Steps.size())
			continue;
		const CMissionStepQueueStart *step = dynamic_cast<const CMissionStepQueueStart *>(templ->Steps[index]);
		if (step != nullptr && step->QueueName == queueName)
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
// whether a mission has done a queue_end step of a queue
static bool hasLeftQueue(CMission *mission, CMissionTemplate *templ, const std::string &queueName)
{
	for (map<uint32, EGSPD::CDoneStepPD>::iterator it = mission->getStepsDoneBegin(); it != mission->getStepsDoneEnd(); ++it)
	{
		const uint32 index = (*it).first - 1;
		if (index >= templ->Steps.size())
			continue;
		const CMissionStepQueueEnd *step = dynamic_cast<const CMissionStepQueueEnd *>(templ->Steps[index]);
		if (step != nullptr && step->QueueName == queueName)
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
// The mission a player waits in a queue for: its own, its team's or its guild's mission whose queue_start step of the
// queue is active. The step tells it, not the mission's waiting queue id, which keeps only the last of two queues a
// mission waits in at once, nor the alias of the mission that created the queue: missions of other templates with the
// same name share the queue.
static CMission *getMissionWaitingIn(CCharacter *c, const std::string &queueName)
{
	for (map<TAIAlias, CMission *>::iterator it = c->getMissionsBegin(); it != c->getMissionsEnd(); ++it)
	{
		if ((*it).second != nullptr && isWaitingIn((*it).second, queueName))
			return (*it).second;
	}

	CTeam *team = TeamManager.getRealTeam(c->getTeamId());
	if (team != nullptr)
	{
		const vector<CMissionTeam *> &missions = team->getMissions();
		for (uint i = 0; i < missions.size(); ++i)
		{
			if (missions[i] != nullptr && isWaitingIn(missions[i], queueName))
				return missions[i];
		}
	}

	CGuild *guild = CGuildManager::getInstance()->getGuildFromId(c->getGuildId());
	if (guild != nullptr)
	{
		const vector<CMissionGuild *> &missions = guild->getMissions();
		for (uint i = 0; i < missions.size(); ++i)
		{
			if (missions[i] != nullptr && isWaitingIn(missions[i], queueName))
				return missions[i];
		}
	}
	return nullptr;
}

// ----------------------------------------------------------------------------
// show a queued mission's journal entry asleep or awake
static void setJournalSleep(CCharacter *character, const CMissionTemplate *templ, CMission *mission, bool sleep)
{
	// a mission hidden from the journal has no entry (0xFF)
	if (mission->getClientIndex() == 0xFF)
		return;

	if (templ->Type == MISSION_DESC::Solo)
		CBankAccessor_PLR::getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(character->_PropertyDatabase, sleep);
	else
		CBankAccessor_PLR::getGROUP().getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(character->_PropertyDatabase, sleep);
}

// ----------------------------------------------------------------------------
bool CMissionQueue::characterLoadedCallback(CCharacter *c)
{
	nlassert(c);

	bool retValue = false;

	// check if we need to change this player mission steps
	for (uint i = 0 ; i < _PlayersToRollBackInSteps.size() ; ++i)
	{
		if (_PlayersToRollBackInSteps[i] == c->getId())
		{
			// The player was in the critical part when the service stopped: its solo mission goes back to its queue_start
			// step, and the player to the head of the queue. A team does not outlive a restart, and a guild mission is
			// shared by members who may have gone on with it: those are left as they are.
			// The mission is in the critical part as saved when its queue_start step is done, or rolled back already, by a
			// login whose character was saved, when it is active. It has not left the queue at a queue_end, which clears
			// its waiting queue id: this queue's, or another one's (a mission waiting in two queues keeps only the last)
			// with no queue_end of this queue done (done steps are never cleared, so only then). The step at the queue's
			// index must still be its queue_start: the primitives can change across a restart.
			CMission *mission = c->getMission(_MissionAlias);
			CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
			CMissionStepQueueStart *queueStart = nullptr;
			if (templ != nullptr && _CreateQueueStepIndex < templ->Steps.size())
				queueStart = dynamic_cast<CMissionStepQueueStart *>(templ->Steps[_CreateQueueStepIndex]);
			const uint32 queueStartKey = (uint32)_CreateQueueStepIndex + 1; // a step's key is its index in the template + 1
			const bool inCriticalPart = mission != nullptr && mission->getStepsDone(queueStartKey) != nullptr;
			const bool rolledBack = mission != nullptr && mission->getSteps(queueStartKey) != nullptr;
			const bool isQueueStart = queueStart != nullptr && queueStart->QueueName == _QueueName;
			const bool inQueue = mission != nullptr
			    && (mission->getWaitingQueueId() == _QueueId
			        || (mission->getWaitingQueueId() != 0 && !hasLeftQueue(mission, templ, _QueueName)));
			if (isQueueStart && inQueue && inCriticalPart && queueStart->getOOOStepIndex() != 0xFFFFFFFF && !queueStart->isAny())
			{
				// queue_start alone would complete its out of order block at the acceptance, skipping the others
				nlwarning("%s was in the critical part of queue %u '%s', whose queue_start is in an out of order block: its mission is not rolled back",
				    c->getId().toString().c_str(), _QueueId, _QueueName.c_str());
				_PlayersToRollBackInSteps[i] = _PlayersToRollBackInSteps.back();
				_PlayersToRollBackInSteps.pop_back();
			}
			else if (isQueueStart && inQueue && (inCriticalPart || rolledBack))
			{
				if (inCriticalPart)
				{
					// The steps of the critical part go, done or active, and queue_start is active again with its initial
					// state, as a jump activates a step: an active step without its state fails the mission as the
					// character's missions are restored (checkConsistencyWithTemplate). The missions are not in game yet,
					// so nothing of the active steps is left.
					vector<uint32> doneSteps;
					for (map<uint32, EGSPD::CDoneStepPD>::iterator it = mission->getStepsDoneBegin(); it != mission->getStepsDoneEnd(); ++it)
					{
						if ((*it).first >= queueStartKey)
							doneSteps.push_back((*it).first);
					}
					for (uint j = 0; j < doneSteps.size(); ++j)
						mission->deleteFromStepsDone(doneSteps[j]);

					while (mission->getStepsBegin() != mission->getStepsEnd())
						mission->deleteFromSteps((*mission->getStepsBegin()).first);

					EGSPD::CActiveStepPD *step = mission->addToSteps(queueStartKey);
					vector<uint32> initState;
					templ->Steps[_CreateQueueStepIndex]->getInitState(initState);
					for (uint j = 0; j < initState.size(); ++j)
						step->addToStates(j + 1)->setState(initState[j]);
				}

				// The critical part starts again when the player enters it: its end date would fail the mission at a later
				// login. One that has passed is kept: the mission fails as the character's missions are restored
				// (CCharacter::updateSavedMissions), which takes the entry out again.
				if (mission->getCriticalPartEndDate() > CTickEventHandler::getGameCycle())
					mission->setCriticalPartEndDate(0);

				// Offline, as the loop below expects, which also gives the entry its mission
				bool queued = false;
				for (list<CWaitingEntity>::const_iterator itEntity = _Entities.begin(); itEntity != _Entities.end(); ++itEntity)
				{
					if ((*itEntity).Id == c->getId())
					{
						queued = true;
						break;
					}
				}
				if (!queued)
				{
					CWaitingEntity entity;
					entity.Id = c->getId();
					entity.Awake = true;
					entity.Online = false;
					entity.LastConnectionDate = CTime::getSecondsSince1970();
					_Entities.push_front(entity);
				}

				// The rollback is in memory until the character is saved, and a character that leaves before entering
				// the game is not, while this queue is: the id stays until the character is saved on disconnection
				// (CMissionQueueManager::disconnectPlayer), enters the critical part again or leaves the queue.
			}
			else
			{
				if (mission != nullptr && !isQueueStart)
					nlwarning("%s was in the critical part of queue %u '%s', whose mission no longer has its queue_start at step %u: its mission is not rolled back",
					    c->getId().toString().c_str(), _QueueId, _QueueName.c_str(), (uint)queueStartKey);
				else
					nldebug("%s was in the critical part of queue %u, with no solo mission in it", c->getId().toString().c_str(), _QueueId);
				_PlayersToRollBackInSteps[i] = _PlayersToRollBackInSteps.back();
				_PlayersToRollBackInSteps.pop_back();
			}
			break;
		}
	}

	list<CWaitingEntity>::iterator it = _Entities.begin();
	for ( ; it != _Entities.end() ; ++it)
	{
		if ( (*it).Id == c->getId() )
		{
			retValue = true;
			(*it).Online = true;
			(*it).LastConnectionDate = CTime::getSecondsSince1970();
			// A disconnection clears the entry's mission. queue_start's onActivation gives it back only for a mission of
			// the player itself (putInGame, as the character's missions are restored), not for a guild's, whose
			// acceptance would then find no mission.
			(*it).Mission = getMissionWaitingIn(c, _QueueName);
			++it;
			break;
		}
	}

	//increase the online position of remaining players
	if (it != _Entities.end())
		changeNbOnlineWaiters( ++it, true);

	return retValue;
}

// ----------------------------------------------------------------------------
void CMissionQueue::forgetRollBack(const NLMISC::CEntityId &id)
{
	for (uint i = 0; i < _PlayersToRollBackInSteps.size(); ++i)
	{
		if (_PlayersToRollBackInSteps[i] == id)
		{
			_PlayersToRollBackInSteps[i] = _PlayersToRollBackInSteps.back();
			_PlayersToRollBackInSteps.pop_back();
			return;
		}
	}
}

// ----------------------------------------------------------------------------
void CMissionQueue::clearOfflinePlayer()
{
	uint16 nbRemoved = 0;

	const uint32 time = CTime::getSecondsSince1970();
	for (list<CWaitingEntity>::iterator it = _Entities.begin() ; it != _Entities.end() ; )
	{
		if ( !(*it).Online && (time - (*it).LastConnectionDate > MaxDisconnectTimeForQueue) )
		{
			forgetRollBack((*it).Id);
			it = _Entities.erase(it);
			++nbRemoved;
		}
		else
		{
			(*it).Position -= nbRemoved;

			if ( (*it).Mission != NULL )
				(*it).Mission->updateUsersJournalEntry();

			++it;
		}
	}
}

// ----------------------------------------------------------------------------
NLMISC::CEntityId CMissionQueue::getEntityWaitingFor(const CMission *mission) const
{
	if (mission == NULL)
		return NLMISC::CEntityId::Unknown;

	const CMission *criticalPartMission = m_CriticalPartMission;
	if (_CriticalPartEntityId != CEntityId::Unknown && criticalPartMission == mission)
		return _CriticalPartEntityId;

	for (list<CWaitingEntity>::const_iterator it = _Entities.begin(); it != _Entities.end(); ++it)
	{
		const CMission *waitingMission = (*it).Mission;
		if (waitingMission == mission)
			return (*it).Id;
	}
	return NLMISC::CEntityId::Unknown;
}

// ----------------------------------------------------------------------------
bool CMissionQueue::waitsWithoutMission(const NLMISC::CEntityId &id) const
{
	if (id == CEntityId::Unknown)
		return false;

	if (_CriticalPartEntityId == id)
		return m_CriticalPartMission == NULL;

	for (list<CWaitingEntity>::const_iterator it = _Entities.begin(); it != _Entities.end(); ++it)
	{
		if ((*it).Id == id)
			return (*it).Mission == NULL;
	}
	return false;
}

// ----------------------------------------------------------------------------
void CMissionQueue::removePlayer(const NLMISC::CEntityId &id)
{
	forgetRollBack(id);

	// if removed entity is the one in critical part, choose another one and exit
	if (_CriticalPartEntityId == id)
	{
		playerLeavesCriticalArea();
		return;
	}

	// parse all entities in queue until we found the removed one
	bool wasOnline = false;
	bool wasAwake = false;

	list<CWaitingEntity>::iterator it = _Entities.begin();
	for ( ; it != _Entities.end() ; ++it)
	{
		if ( (*it).Id == id )
		{
			wasOnline = (*it).Online;
			wasAwake =  (*it).Awake;
			it = _Entities.erase(it);
			break;
		}
	}
	
	//decrease the position of remaining players
	for ( ; it != _Entities.end() ; ++it)
	{
		--(*it).Position;

		if (wasOnline && wasAwake)
			--(*it).PositionOnline;
	
		if ( (*it).Mission != NULL )
			(*it).Mission->updateUsersJournalEntry();
	}

	// decrease the number of queues in which is this player (so he must have been online)
	if (wasOnline)
	{
		CCharacter *player = PlayerManager.getChar(id);
		if (!player)
		{
			nlwarning("Cannot find player id %s", id.toString().c_str());
			return;
		}
		player->removeFromQueue(_QueueId);

		// if it was the player currently being requested to enter, choose another one
		if (id == _AskedEntityId)
		{
			_AskedEntityId = CEntityId::Unknown;
			_StepAnswerTimer = 0;
			// every queue skips a player whose proposal is still set
			player->setEnterCriticalZoneProposalQueueId(0);

			// send message to close proposal window
			CMessage msgout( "IMPULSION_ID" );
			msgout.serial( const_cast<CEntityId&> (id) );
			CBitMemStream bms;
			if ( ! GenericMsgManager.pushNameToStream( "MISSION:CLOSE_ENTER_CRITICAL", bms) )
			{
				nlwarning("Msg name MISSION:CLOSE_ENTER_CRITICAL not found");
				return;
			}
			msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
			CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(id.getDynamicId()), msgout );			
		}
	}

}

// ----------------------------------------------------------------------------
void CMissionQueue::disconnectPlayer(const NLMISC::CEntityId &id)
{
	list<CWaitingEntity>::iterator it = _Entities.begin();
	for ( ; it != _Entities.end() ; ++it)
	{
		if ( (*it).Id == id )
		{
			(*it).Online = false;
			(*it).Mission = NULL;
			break;
		}
	}
	
	//decrease the online position of remaining players
	if ( it != _Entities.end() && (*it).Awake )
		changeNbOnlineWaiters( ++it, false);

	// if it was the player currently being requested to enter, choose another one
	if (id == _AskedEntityId)
	{
		_AskedEntityId = CEntityId::Unknown;
		_StepAnswerTimer = 0;
	}
	// if it was the player in critical zone, remove it from zone and from queue
	else if (id == _CriticalPartEntityId)
	{
		playerLeavesCriticalArea();
	}

	CMissionQueueManager::getInstance()->saveToFile();
}

// ----------------------------------------------------------------------------
void CMissionQueue::tickUpdate()
{
	// manage waiting request if any
	if (_StepAnswerTimer > 0)
	{
		--_StepAnswerTimer;
	}
	else if (_AskedEntityId != CEntityId::Unknown)
	{
		// player did not answer in time, set him to sleep mode, and tell next player in queue
		CMission *mission = NULL;
		list<CWaitingEntity>::iterator it;
		for ( it = _Entities.begin() ; it != _Entities.end() ; ++it)
		{
			if ( (*it).Id == _AskedEntityId )
			{
				(*it).Awake = false;

				// send message to close proposal window
				CMessage msgout( "IMPULSION_ID" );
				msgout.serial( const_cast<CEntityId&> (_AskedEntityId) );
				CBitMemStream bms;
				if ( ! GenericMsgManager.pushNameToStream( "MISSION:CLOSE_ENTER_CRITICAL", bms) )
				{
					nlwarning("Msg name MISSION:CLOSE_ENTER_CRITICAL not found");
					return;
				}
				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_AskedEntityId.getDynamicId()), msgout );
				
				//
				CCharacter *character = PlayerManager.getChar(_AskedEntityId);
				if (character)
				{
					// reset asked queue id
					character->setEnterCriticalZoneProposalQueueId(0);

					mission = (*it).Mission;
					if (mission != NULL)
					{
						// indicate to player he is now in sleep mode
						CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
						if (!templ)
						{
							nlwarning("Failed to find mission template for alias %u", _MissionAlias);
						}
						else
						{
							setJournalSleep(character, templ, mission, true);
						}
					}
				}
				else
				{
					nlwarning("Failed to find player character %s", _AskedEntityId.toString().c_str());
				}
				
				break;
			}
		}

		//decrease the online position of remaining players
		if (it != _Entities.end())
			changeNbOnlineWaiters( ++it, false);

		// send player a message
		if (mission != NULL)
		{
			CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
			if (!templ)
			{
				nlwarning("Failed to find mission template for alias %u", _MissionAlias);
			}
			else
			{
				const TDataSetRow rowId = TheDataset.getDataSetRow(_AskedEntityId);
				const TDataSetRow giverRow = TheDataset.getDataSetRow( CAIAliasTranslator::getInstance()->getEntityId( mission->getGiver() ) );

				SM_STATIC_PARAMS_1(params, STRING_MANAGER::dyn_string_id);
				params[0].StringId = templ->sendTitleText(rowId, giverRow);
				CCharacter::sendDynamicSystemMessage( _AskedEntityId, "MISSION_ENTER_CRITITAL_PART_TIMEOUT" ,params );
			}
		}

		_AskedEntityId = CEntityId::Unknown;
		
		playerLeavesCriticalArea();
	}

	// if there is no entity in critical part and no asked entity, try again
	if (_CriticalPartEntityId == CEntityId::Unknown && _AskedEntityId == CEntityId::Unknown)
	{
		playerLeavesCriticalArea();
	}

	// manage current player in step
	if ( _CriticalPartTimer > 0 )
	{
		--_CriticalPartTimer;
	}
	else if (_CriticalPartEntityId != CEntityId::Unknown)
	{
		// player failed his mission !!
		const CEntityId criticalPartEntityId = _CriticalPartEntityId;
		CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
		if (!templ)
		{
			nlwarning("Failed to find mission template for alias %u", _MissionAlias);
		}
		else
		{
			const TDataSetRow rowId = TheDataset.getDataSetRow(_CriticalPartEntityId);
			// a failed instance leaves Instances (see CMissionTemplate::isInstance)
			const std::vector<CMission *> instances = templ->Instances;
			for (uint i = 0; i < instances.size(); ++i)
			{
				if (!templ->isInstance(instances[i]))
					continue;
				if (instances[i])
				{
					vector<TDataSetRow> entities;
					instances[i]->getEntities(entities);
					for ( uint j = 0 ; j < entities.size() ; ++j )
					{
						if( entities[j] == rowId )
						{
							instances[i]->onFailure(false);
							break;
						}
					}
				}
				else
					nlwarning( "mission %u  has a NULL instance (index %u) ", _MissionAlias, i);
			}
		}

		// get next entity in queue, unless the failure took the player out of the critical part (removePlayer), which
		// already asked the next one
		if (_CriticalPartEntityId == criticalPartEntityId)
			playerLeavesCriticalArea();
	}


	// once each day, remove players who have been offline for too long (24*60*60 s = 86400)
	if ( (CTime::getSecondsSince1970() % 86400) == 0 )
	{
		clearOfflinePlayer();
	}
}

// ----------------------------------------------------------------------------
void CMissionQueue::addPlayer( const NLMISC::CEntityId &id, CMission *mission, bool forceTopOfQueue )
{
	CCharacter *player = PlayerManager.getChar(id);
	if (!player)
	{
		nlwarning("Cannot find player id %s", id.toString().c_str());
		return;
	}

	if (mission)
		mission->setWaitingQueueId(_QueueId);

	player->addInQueue(_QueueId);

	uint16 nbOnline = 0;
	uint16 nbTotal = 0;
	for ( list<CWaitingEntity>::iterator it = _Entities.begin() ; it != _Entities.end() ; ++it)
	{
		// if found player, do not add entry, just refresh infos of previous one
		if ( (*it).Id == id )
		{
			//(*it).Awake = true;
			CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
			if (!templ)
			{
				nlwarning("Failed to find mission template for alias %u", _MissionAlias);
			}
			else if (mission)
			{
				setJournalSleep(player, templ, mission, !(*it).Awake);
			}

			(*it).LastConnectionDate = CTime::getSecondsSince1970();
			(*it).Online = true;
			(*it).PositionOnline = nbOnline;
			(*it).Position = nbTotal;
			(*it).Mission = mission;
						
			if (forceTopOfQueue)
			{
				_Entities.push_front(*it);
				_Entities.erase(it);
				it = _Entities.begin();
			}

			//increase the online position of remaining players
			if ( (*it).Awake )
				changeNbOnlineWaiters( ++it, true);
				
			return;
		}
	
		if ((*it).Online && (*it).Awake)
			++nbOnline;

		++nbTotal;
	}

	CWaitingEntity entity;
	entity.Awake = true;
	entity.Id = id;
	entity.LastConnectionDate = CTime::getSecondsSince1970();
	entity.Online = true;
	entity.Mission = mission;

	CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
	if (!templ)
	{
		nlwarning("Failed to find mission template for alias %u", _MissionAlias);
	}
	else if (mission)
	{
		setJournalSleep(player, templ, mission, false);
	}

	if (!forceTopOfQueue)
	{
		entity.Position = (uint16)_Entities.size();
		entity.PositionOnline = nbOnline;

		_Entities.push_back(entity);
	}
	else
	{
		entity.Position = 0;
		entity.PositionOnline = 0;

		_Entities.push_front(entity);
	}
}

// ----------------------------------------------------------------------------
void CMissionQueue::playerEntersCriticalArea(const NLMISC::CEntityId &id, bool accept)
{
	if (_AskedEntityId != id)
		return;

	CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
	if (!templ)
	{
		nlwarning("Failed to find mission template for alias %u", _MissionAlias);
		return;
	}

	_StepAnswerTimer = 0;
	_AskedEntityId = CEntityId::Unknown;

	CCharacter *player = PlayerManager.getChar(id);
	if (player)
		player->setEnterCriticalZoneProposalQueueId(0);

	if (accept)
	{
		CMission *mission = NULL;
		list<CWaitingEntity>::iterator itPlayerCritical;
		for ( itPlayerCritical = _Entities.begin() ; itPlayerCritical != _Entities.end() ; ++itPlayerCritical)
		{
			if ( (*itPlayerCritical).Id == id )
			{
				// remove entry
				mission = (*itPlayerCritical).Mission;
				BOMB_IF(mission == NULL, "mission == NULL, should not happen",;);
				break;
			}
		}

		BOMB_IF( itPlayerCritical == _Entities.end() , "Cannot find asked entity in queue !! BUG", return);

		// update waiters number for all remaining entities
		{
			list<CWaitingEntity>::iterator it = itPlayerCritical;
			for (  ++it ; it != _Entities.end() ; ++it)
			{
				--(*it).Position;
				--(*it).PositionOnline;
	
				if ( (*it).Mission != NULL )
					(*it).Mission->updateUsersJournalEntry();
			}
		}

		//remove entry
		_Entities.erase(itPlayerCritical);

		// step is succesfull, generate event
		if (player)
		{
			// set new mission end date
			if (mission != NULL)
			{
				mission->setCriticalPartEndDate(CTickEventHandler::getGameCycle() + _MaxTimeInCriticalPart);
			}

			// The player is in the critical part before the event: processing it can end the mission (a queue_end
			// step right after, a failure, a success that removes it), which takes the player out of the critical part
			// (removePlayer), the entry already removed
			_CriticalPartTimer = _MaxTimeInCriticalPart;
			_CriticalPartEntityId = id;
			m_CriticalPartMission = mission;
			forgetRollBack(id);

			// Mission event
			CMissionEventQueueEntryOk event(_QueueId);
			player->processMissionEvent( event );
		}
	}
	else
	{
		//put this player to sleep mode, get next one in queue
		CMission *mission = NULL;
		for ( list<CWaitingEntity>::iterator it = _Entities.begin() ; it != _Entities.end() ; ++it)
		{
			if ( (*it).Id == id )
			{
				(*it).Awake = false;
				mission = (*it).Mission;
				BOMB_IF(mission == NULL, "mission == NULL, should not happen",;);
				// indicate to player he is now in sleep mode		
				if (mission && player)
				{
					setJournalSleep(player, templ, mission, true);
				}

				//decrease the online position of remaining players
				changeNbOnlineWaiters( ++it, false);

				break;
			}
		}
		
		// send player a message
		if (mission != NULL)
		{
			const TDataSetRow rowId = TheDataset.getDataSetRow(_AskedEntityId);
			const TDataSetRow giverRow = TheDataset.getDataSetRow( CAIAliasTranslator::getInstance()->getEntityId( mission->getGiver() ) );
			
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::dyn_string_id);
			params[0].StringId = templ->sendTitleText(rowId, giverRow);
			CCharacter::sendDynamicSystemMessage( _AskedEntityId, "MISSION_ENTER_CRITITAL_PART_DECLINE" ,params );
		}
		
		playerLeavesCriticalArea();
	}
}

// ----------------------------------------------------------------------------
void CMissionQueue::playerLeavesCriticalArea()
{
	_CriticalPartEntityId = CEntityId::Unknown;
	m_CriticalPartMission = NULL;
	_CriticalPartTimer = 0;

	// A candidate with no mission waiting here any more is taken out of the queue: it would hold the critical part until
	// it timed out. That is the entry of a player who was offline when the mission was removed (which then cannot tell
	// the entry belongs to it), or who left the team or guild whose mission the player waited for.
	for (;;)
	{
		const CEntityId candidate = getNextCandidateInQueue().Id;
		if (candidate == CEntityId::Unknown)
			break;
		CCharacter *c = PlayerManager.getChar(candidate);
		if (c == NULL || getMissionWaitingIn(c, _QueueName) != nullptr)
			break;
		nldebug("%s waits in queue %u with no mission waiting in it, taken out of the queue", candidate.toString().c_str(), _QueueId);
		CMissionQueueManager::getInstance()->removePlayerFromQueue(candidate, _QueueId);
		if (getNextCandidateInQueue().Id == candidate)
		{
			// the manager's queue of this id is another one (a damaged queue file): the loop would not end
			nlwarning("Queue %u '%s': the queue manager could not take %s out of it, taken out here", _QueueId, _QueueName.c_str(), candidate.toString().c_str());
			removePlayer(candidate);
		}
	}

	// get next candidate and send him an enter critical part request
	const CWaitingEntity &nextEntity = getNextCandidateInQueue();
	const CEntityId &id = nextEntity.Id;
	if (id != CEntityId::Unknown)
	{
		CCharacter *player = PlayerManager.getChar(id);
		if (player)
		{
			if (player->getEnterCriticalZoneProposalQueueId() != 0)
			{
				// player already has a request in progress, skip him
			}

			player->setEnterCriticalZoneProposalQueueId(_QueueId);
		}
		_AskedEntityId = id;
		_StepAnswerTimer = MaxEnterCriticalAnswerTime;

		uint32 textId = 0;
		// chat message
		CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
		if (!templ)
		{
			nlwarning("Failed to find mission template for alias %u", _MissionAlias);
		}
		else if (nextEntity.Mission != NULL)
		{
			const TDataSetRow rowId = TheDataset.getDataSetRow(_AskedEntityId);
			const TDataSetRow giverRow = TheDataset.getDataSetRow( CAIAliasTranslator::getInstance()->getEntityId( nextEntity.Mission->getGiver() ) );
						
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::dyn_string_id);
			params[0].StringId = templ->sendTitleText(rowId, giverRow);

			textId = STRING_MANAGER::sendStringToClient( TheDataset.getDataSetRow(id), "MISSION_ENTER_CRITITAL_PART_PROPOSAL",params );

			CMessage msgout( "IMPULSION_ID" );
			msgout.serial( const_cast<CEntityId&> (id) );
			CBitMemStream bms;
			if ( ! GenericMsgManager.pushNameToStream( "STRING:DYN_STRING", bms) )
			{
				nlwarning("<sendDynamicSystemMessage> Msg name STRING:DYN_STRING not found");
			}
			else
			{
				bms.serial( textId );
				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(id.getDynamicId()), msgout );
			}
		}

		// send message
		CMessage msgout( "IMPULSION_ID" );
		msgout.serial( const_cast<CEntityId&> (id) );
		CBitMemStream bms;
		if ( ! GenericMsgManager.pushNameToStream( "MISSION:ASK_ENTER_CRITICAL", bms) )
		{
			nlwarning("Msg name MISSION:ASK_ENTER_CRITICAL not found");
			return;
		}
		bms.serial(textId);
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(id.getDynamicId()), msgout );
	}
}

// ----------------------------------------------------------------------------
const CWaitingEntity &CMissionQueue::getNextCandidateInQueue() const
{
	if (_Entities.empty())
		return CWaitingEntity::Unknown;
	
	for ( list<CWaitingEntity>::const_iterator it = _Entities.begin() ; it != _Entities.end() ; ++it)
	{
		if ( (*it).Online && (*it).Awake )
		{
			// skip player if he already has a proposal in progress, or is still loading: the character is online from its
			// selection, and a client without its interface yet drops the proposal, then times out asleep
			CCharacter *player = PlayerManager.getChar((*it).Id);
			if (player && player->getEnterCriticalZoneProposalQueueId() == 0 && player->getEnterFlag())
			{
				return (*it);
			}
		}
	}

	return CWaitingEntity::Unknown;
}

// ----------------------------------------------------------------------------
void CMissionQueue::postApply()
{
	clearOfflinePlayer();

	if ( _CriticalPartEntityId != CEntityId::Unknown )
	{
		// The player was in the critical part: its mission goes back to its queue_start step, and the player to the head
		// of the queue, when the character comes back (characterLoadedCallback). Calling addPlayer here did nothing:
		// it needs the character, which is not loaded at startup, so the mission was rolled back without its entry and
		// waited at queue_start for a proposal that never came.

		// keep player Id to reset his mission step when he will be back
		// when he will be online
		// check the steps he has validated and come back in steps until we found the create_queue step
		forgetRollBack(_CriticalPartEntityId);
		_PlayersToRollBackInSteps.push_back(_CriticalPartEntityId);

		playerLeavesCriticalArea();
	}
}

// ----------------------------------------------------------------------------
void CMissionQueue::changePlayerAwakeState(const NLMISC::CEntityId &id, bool wakeUp)
{
	list<CWaitingEntity>::iterator it;
	for ( it = _Entities.begin() ; it != _Entities.end() ; ++it)
	{
		if ( (*it).Id == id )
		{
			// return if state is already in specified mode
			if ((*it).Awake == wakeUp)
				return;

			(*it).Awake = wakeUp;
			
			CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
			if (!templ)
			{
				nlwarning("Failed to find mission template for alias %u", _MissionAlias);
			}
			else if ((*it).Mission != NULL)
			{
				CCharacter *character = PlayerManager.getChar(id);
				if (character)
				{
					setJournalSleep(character, templ, (*it).Mission, !wakeUp);
				}
			}

			break;
		}
	}

	//change the online position of remaining players
	if (it != _Entities.end())
		changeNbOnlineWaiters( ++it, wakeUp);
}

// ----------------------------------------------------------------------------
void CMissionQueue::changeNbOnlineWaiters( std::list<CWaitingEntity>::iterator itStart, bool inc )
{
	for ( list<CWaitingEntity>::iterator it = itStart ; it != _Entities.end() ; ++it)
	{
		if (inc)
			++(*it).PositionOnline;
		else
			--(*it).PositionOnline;

		if ( (*it).Mission != NULL )
			(*it).Mission->updateUsersJournalEntry();
	}
}

// ----------------------------------------------------------------------------
bool CMissionQueue::getPlayerPositions(const NLMISC::CEntityId &id, uint16 &position, uint16 &onlinePosition, bool &hasPlayerInCritZone) const
{
	for ( list<CWaitingEntity>::const_iterator it = _Entities.begin() ; it != _Entities.end() ; ++it)
	{
		if ((*it).Id == id)
		{
			if (_CriticalPartEntityId != CEntityId::Unknown)
				hasPlayerInCritZone = true;
			else
				hasPlayerInCritZone = false;

			position = (*it).Position;
			onlinePosition = (*it).PositionOnline;
			
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------------
void CMissionQueue::dump()
{
	nlinfo("Queue name = %s, Id = %u", _QueueName.c_str(), _QueueId);
	nlinfo("\tsize = %u",_Entities.size());
	
	for ( list<CWaitingEntity>::iterator it = _Entities.begin() ; it != _Entities.end() ; ++it)
	{
		nlinfo("\tPlayer %s, awake = %u, online = %u, position = %u", (*it).Id.toString().c_str(), uint((*it).Awake), uint((*it).Online),(*it).Position );
	}
	nlinfo("\n");
}	

// ----------------------------------------------------------------------------
// If the following macro is defined then the macros such as PERSISTENT_CLASS, PERSISTENT_DATA, PERSISTENT_PRE_STORE, etc
// are undefined after inclusion of persistent_data_template.h
#define PERSISTENT_MACROS_AUTO_UNDEF


//-----------------------------------------------------------------------------
// Persistent data for CWaitingEntity
//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS CWaitingEntity

#define PERSISTENT_PRE_STORE\
	H_AUTO(CWaitingEntityStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CWaitingEntityApply);\

#define PERSISTENT_DATA\
	PROP(CEntityId, Id)\
	PROP(uint32, LastConnectionDate)\
	PROP(bool, Awake)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CMissionQueue
//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS CMissionQueue

#define PERSISTENT_PRE_STORE\
	H_AUTO(CMissionQueueStore);\
	
#define PERSISTENT_PRE_APPLY\
	H_AUTO(CMissionQueueApply);\

#define PERSISTENT_POST_APPLY\
	postApply();\
	
#define PERSISTENT_DATA\
	PROP(string, _QueueName)\
	PROP(uint32, _QueueId)\
	PROP(uint16, _CreateQueueStepIndex)\
	PROP_GAME_CYCLE_COMP(_MaxTimeInCriticalPart)\
	PROP(TAIAlias, _MissionAlias)\
	PROP(CEntityId, _CriticalPartEntityId)\
	STRUCT_LIST(CWaitingEntity,_Entities)\
	PROP_VECT(CEntityId, _PlayersToRollBackInSteps)\
	
//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"
