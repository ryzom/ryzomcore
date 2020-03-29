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
bool CMissionQueue::characterLoadedCallback(CCharacter *c)
{
	nlassert(c);

	bool retValue = false;

	// check if we need to change this player mission steps
	for (uint i = 0 ; i < _PlayersToRollBackInSteps.size() ; ++i)
	{
		if (_PlayersToRollBackInSteps[i] == c->getId())
		{
			// update mission
			CMission *mission = c->getMission(_MissionAlias);
			if (mission)
			{
				vector<uint32> removedSteps;

				map<uint32, EGSPD::CDoneStepPD>::iterator it = mission->getStepsDoneBegin();
				for ( ; it != mission->getStepsDoneEnd() ; ++it )
				{
					if ( (*it).first > _CreateQueueStepIndex )
						removedSteps.push_back((*it).first);
				}

				for (uint j = 0 ; j < removedSteps.size() ; ++j)
				{
					mission->deleteFromStepsDone(removedSteps[j]);
					mission->addToSteps(removedSteps[j]);
				}
			}
			else
			{
				nlwarning("Cannot find mission alias %u in player %s, BUG", _MissionAlias, c->getId().toString().c_str());
			}

			_PlayersToRollBackInSteps[i] = _PlayersToRollBackInSteps.back();
			_PlayersToRollBackInSteps.pop_back();
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
void CMissionQueue::clearOfflinePlayer()
{
	uint16 nbRemoved = 0;

	const uint32 time = CTime::getSecondsSince1970();
	for (list<CWaitingEntity>::iterator it = _Entities.begin() ; it != _Entities.end() ; )
	{
		if ( !(*it).Online && (time - (*it).LastConnectionDate > MaxDisconnectTimeForQueue) )
		{
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
void CMissionQueue::removePlayer(const NLMISC::CEntityId &id)
{
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
							string prefix;
							if ( templ->Type  == MISSION_DESC::Solo )
//								prefix = "";
								CBankAccessor_PLR::getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(character->_PropertyDatabase, 1);
							else //if ( templ->Type  == MISSION_DESC::Group )
//								prefix = "GROUP:";
								CBankAccessor_PLR::getGROUP().getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(character->_PropertyDatabase, 1);

							//character->_PropertyDatabase.setProp( toString("%sMISSIONS:%u:SLEEP",prefix.c_str(),mission->getClientIndex()), 1);
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
		CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate(_MissionAlias);
		if (!templ)
		{
			nlwarning("Failed to find mission template for alias %u", _MissionAlias);
		}
		else
		{
			const TDataSetRow rowId = TheDataset.getDataSetRow(_CriticalPartEntityId);
			for ( uint i = 0; i < templ->Instances.size() ; ++i)
			{
				if ( templ->Instances[i] )
				{
					vector<TDataSetRow> entities;
					templ->Instances[i]->getEntities( entities );
					for ( uint j = 0 ; j < entities.size() ; ++j )
					{
						if( entities[j] == rowId )
						{
							templ->Instances[i]->onFailure(false);
						}
					}
				}
				else
					nlwarning( "mission %u  has a NULL instance (index %u) ", _MissionAlias, i);
			}
		}
			
		// get next entity in queue
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
				string prefix;
				
				if ( templ->Type  == MISSION_DESC::Solo )
//					prefix = "";
					CBankAccessor_PLR::getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(player->_PropertyDatabase, ((*it).Awake?0:1));
				else //if ( templ->Type  == MISSION_DESC::Group )
//					prefix = "GROUP:";
					CBankAccessor_PLR::getGROUP().getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(player->_PropertyDatabase, ((*it).Awake?0:1));
					
//				player->_PropertyDatabase.setProp( toString("%sMISSIONS:%u:SLEEP",prefix.c_str(),mission->getClientIndex()), ((*it).Awake?0:1) );
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
		string prefix;
		
		if ( templ->Type  == MISSION_DESC::Solo )
//			prefix = "";
			CBankAccessor_PLR::getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(player->_PropertyDatabase, 0);
		else //if ( templ->Type  == MISSION_DESC::Group )
//			prefix = "GROUP:";
			CBankAccessor_PLR::getGROUP().getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(player->_PropertyDatabase, 0);
		
//		player->_PropertyDatabase.setProp( toString("%sMISSIONS:%u:SLEEP",prefix.c_str(),mission->getClientIndex()), 0);
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

		// step is succesfull, generate event
		if (player)
		{
			// set new mission end date
			if (mission != NULL)
			{
				mission->setCriticalPartEndDate(CTickEventHandler::getGameCycle() + _MaxTimeInCriticalPart);
			}

			// Mission event
			CMissionEventQueueEntryOk event;
			player->processMissionEvent( event );

			_CriticalPartTimer = _MaxTimeInCriticalPart;
			_CriticalPartEntityId = id;
		}

		//remove entry
		_Entities.erase(itPlayerCritical);		
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
//					string prefix;
					if ( templ->Type  == MISSION_DESC::Solo )
//						prefix = "";
						CBankAccessor_PLR::getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(player->_PropertyDatabase, 1);
					else //if ( templ->Type  == MISSION_DESC::Group )
//						prefix = "GROUP:";
						CBankAccessor_PLR::getGROUP().getMISSIONS().getArray(mission->getClientIndex()).setSLEEP(player->_PropertyDatabase, 1);
						
//					if (player)
//					{
//						player->_PropertyDatabase.setProp( toString("%sMISSIONS:%u:SLEEP",prefix.c_str(),mission->getClientIndex()), 1);
//					}
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
	_CriticalPartTimer = 0;

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
			// skip player if he already has a proposal in progress
			CCharacter *player = PlayerManager.getChar((*it).Id);
			if (player && player->getEnterCriticalZoneProposalQueueId() == 0)
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
		// player was in critical part, add him at the head of the queue
		addPlayer( _CriticalPartEntityId, NULL, true );

		// keep player Id to reset his mission step when he will be back
		// when he will be online
		// check the steps he has validated and come back in steps until we found the create_queue step
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
//				string prefix;
				
				CCharacter *character = PlayerManager.getChar(id);
				if (character)
				{
					if ( templ->Type  == MISSION_DESC::Solo )
//						prefix = "";
						CBankAccessor_PLR::getMISSIONS().getArray((*it).Mission->getClientIndex()).setSLEEP(character->_PropertyDatabase, (wakeUp?0:1));
					else //if ( templ->Type  == MISSION_DESC::Group )
//						prefix = "GROUP:";
						CBankAccessor_PLR::getGROUP().getMISSIONS().getArray((*it).Mission->getClientIndex()).setSLEEP(character->_PropertyDatabase, (wakeUp?0:1));
					
//					CCharacter *character = PlayerManager.getChar(id);
//					if (character)
//					{
//						character->_PropertyDatabase.setProp( toString("%sMISSIONS:%u:SLEEP",prefix.c_str(),(*it).Mission->getClientIndex()), (wakeUp?0:1));
//					}
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
