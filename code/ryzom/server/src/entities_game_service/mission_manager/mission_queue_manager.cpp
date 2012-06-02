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

#include "mission_queue_manager.h"
#include "player_manager/character.h"
#include "game_share/backup_service_interface.h"


// ----------------------------------------------------------------------------

using namespace NLMISC;
using namespace NLNET;
using namespace std;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


// ----------------------------------------------------------------------------

CVariable<std::string>	MissionQueueFile("egs", "MissionQueueFile", "file holding all mission queues stuff", std::string("mission_queues.txt"), 0, true );
CVariable<uint32>		MissionQueueSavePeriod("egs", "MissionQueueSavePeriod", "interval between saves in ticks (default = 90s)", 90, 0, true );

CMissionQueueManager *CMissionQueueManager::_Instance = NULL;

// ----------------------------------------------------------------------------
CMissionQueueManager::CMissionQueueManager()
{
	_QueueIdCounter = 0;
	_InitOk = false;
}

// ----------------------------------------------------------------------------
void CMissionQueueManager::init()
{
	string sFilename = MissionQueueFile;
	sFilename = Bsi.getLocalPath() + sFilename;
	
	if (CFile::fileExists(sFilename))
	{
		static CPersistentDataRecord	pdr;
		pdr.clear();
		pdr.readFromTxtFile(sFilename.c_str());
		apply(pdr);
	}
	_InitOk = true;
}

// ----------------------------------------------------------------------------
void CMissionQueueManager::characterLoadedCallback(CCharacter *c)
{
	H_AUTO(CMissionQueueManagerCharacterLoadedCallback);

	nlassert(c != NULL);

	vector<uint32> playerQueues = c->getMissionQueues();

	// get queues for this player
	TPlayerQueues::const_iterator itPQ =  _PlayerQueues.find(c->getId());
	if (itPQ != _PlayerQueues.end())
	{
		const CQueueVect &queues = (*itPQ).second;
		for ( uint i = 0 ; i < queues.QueueIds.size() ; )
		{
			// check coherency with infos kept in manager
			const uint size2 = (uint)playerQueues.size();
			uint j;
			for ( j = 0 ; j < size2 ; ++j)
			{
				if (playerQueues[j] == queues.QueueIds[i])
				{
					playerQueues[j] = playerQueues.back();
					playerQueues.pop_back();
					break;
				}
			}

			if (j == size2)
			{
				// queue not found in player queues, remove it and go to next
				removePlayerFromQueue(c->getId(), queues.QueueIds[i]);
				continue;
			}
			else
			{
				map<uint32,CMissionQueue>::iterator itMQ = _Queues.find(queues.QueueIds[i]);
				BOMB_IF(itMQ == _Queues.end(),"Failed to find queue from it's id, but referenced in _PlayerQueues queues list, BUG", continue);
				(*itMQ).second.characterLoadedCallback(c);

				++i;
			}
		}
	}

	/*for ( map<uint32,CMissionQueue>::iterator it = _Queues.begin() ; it != _Queues.end() ; ++it )
	{
		(*it).second.characterLoadedCallback(c);
	}
	*/
}


// ----------------------------------------------------------------------------
void CMissionQueueManager::tickUpdate()
{
	if( IsRingShard ) // Temporary Fix potential problem with multi shard instance Ring unification: 
		return;	// Mission saved tick must be adapted for have relative value saved

	H_AUTO(CMissionQueueManagerUpdate);

	for ( map<uint32,CMissionQueue>::iterator it = _Queues.begin() ; it != _Queues.end() ; ++it )
	{
		(*it).second.tickUpdate();
	}

	// save file when time has come
	if (CTickEventHandler::getGameCycle() % MissionQueueSavePeriod.get() == 0)
		saveToFile();
}


// ----------------------------------------------------------------------------
void CMissionQueueManager::saveToFile()
{
	H_AUTO(CMissionQueueManagerSaveToFile);

	if( _InitOk )
	{
		string sFilename = MissionQueueFile;
		
		// save file via Backup Service (BS)
		try
		{
			static CPersistentDataRecordRyzomStore	pdr;
			pdr.clear();
			store(pdr);

			CBackupMsgSaveFile msg( sFilename, CBackupMsgSaveFile::SaveFile, Bsi );
			{
				std::string s;
				pdr.toString(s);
				msg.DataMsg.serialBuffer((uint8*)&s[0], (uint)s.size());
			}
			Bsi.sendFile( msg );
		}
		catch(const Exception &)
		{
			nlwarning("(EGS)<CMissionQueueManager::saveToFile>  :  Can't serial file %s (connection with BS service down ?)",sFilename.c_str());
			return;
		}
	}
}

// ----------------------------------------------------------------------------
void CMissionQueueManager::addPlayerInQueue(const CEntityId &id, CMission *mission, uint16 stepIndex, const string &queueName, uint32 timer)
{
	H_AUTO(CMissionQueueManagerAddPlayer);

	nlassert(mission != NULL);
		
	uint32 queueId = 0;

	map<uint32,CMissionQueue>::iterator it;

	const map< string, uint32 >::const_iterator itId = _QueueNamesToIds.find(queueName);
	if (itId == _QueueNamesToIds.end())
	{
		++_QueueIdCounter;
		if (!_QueueIdCounter) _QueueIdCounter = 1; // just to be sure _QueueIdCounter always != 0

		queueId = _QueueIdCounter;

		it = _Queues.insert( make_pair(_QueueIdCounter,CMissionQueue(queueName,queueId,timer,mission->getTemplateId(),stepIndex)) ).first;
		if (it == _Queues.end())
		{
			nlwarning("Failed to create new queue name %s in queue list", queueName.c_str());
			return;
		}
		
		// map new queue
		_QueueNamesToIds.insert( make_pair(queueName, queueId) );
	}
	else
	{
		queueId = (*itId).second;
		it = _Queues.find(queueId);
	}

	if ( it == _Queues.end() )
	{
		nlwarning("Failed to find queue name %s in map, but is referenced in _QueueNamesToIds, BUG", queueName.c_str());
		return;
	}

	(*it).second.addPlayer(id,mission);

	// manage the player queues map
	TPlayerQueues::iterator itPQ = _PlayerQueues.find(id);
	if (itPQ == _PlayerQueues.end())
	{
		CQueueVect list;
		list.addQueue(queueId);
		_PlayerQueues.insert( make_pair(id, list) );
	}
	else
	{
		(*itPQ).second.addQueue(queueId);
	}

	//saveToFile();
}

// ----------------------------------------------------------------------------
void CMissionQueueManager::removePlayerFromQueue(const CEntityId &id, uint32 queueId)
{
	H_AUTO(CMissionQueueManagerRemovePlayer);

	map<uint32,CMissionQueue>::iterator it = _Queues.find(queueId);
	if ( it == _Queues.end() )
	{
		nlwarning("Trying to remove %s from queue %u but this queue doesn't exist in manager !", id.toString().c_str(), queueId);
		return;
	}

	(*it).second.removePlayer(id);

	TPlayerQueues::iterator itPQ = _PlayerQueues.find(id);
	if (itPQ == _PlayerQueues.end())
	{
		nlwarning("Failed to find entry in _PlayerQueues for player ! BUG", id.toString().c_str());
		return;
	}

	(*itPQ).second.removeQueue(queueId);

	// remove entry if player is no longer in a queue
	if ((*itPQ).second.QueueIds.empty())
	{
		_PlayerQueues.erase(itPQ);
	}
}

// ----------------------------------------------------------------------------
void CMissionQueueManager::disconnectPlayer(const CCharacter *c)
{
	H_AUTO(CMissionQueueManagerDisconnectPlayer);

	if (!c)
		return;

	// get queues for this player
	TPlayerQueues::const_iterator itPQ =  _PlayerQueues.find(c->getId());
	if (itPQ != _PlayerQueues.end())
	{
		const CQueueVect &queues = (*itPQ).second;
		for ( uint i = 0 ; i < queues.QueueIds.size() ; ++i)
		{
			map<uint32,CMissionQueue>::iterator itMQ = _Queues.find(queues.QueueIds[i]);
			BOMB_IF(itMQ == _Queues.end(),"Failed to find queue from it's id, but referenced in _PlayerQueues queues list, BUG", continue);
			(*itMQ).second.disconnectPlayer(c->getId());
		}
	}
}

// ----------------------------------------------------------------------------
void CMissionQueueManager::playerEntersCriticalArea(const NLMISC::CEntityId &id, uint32 queueId, bool accept)
{
	H_AUTO(CMissionQueueManagerPlayerEntersCriticalArea);

	map<uint32,CMissionQueue>::iterator it = _Queues.find(queueId);
	if ( it == _Queues.end() )
	{
		nlwarning("Trying to make player %s enters critical area of queue %u but this queue doesn't exist in manager !", id.toString().c_str(), queueId);
		return;
	}

	(*it).second.playerEntersCriticalArea(id, accept);
}

// ----------------------------------------------------------------------------
uint32 CMissionQueueManager::getQueueId( const std::string &name ) const
{
	const map< string, uint32 >::const_iterator itId = _QueueNamesToIds.find(name);
	if (itId != _QueueNamesToIds.end())
	{
		return (*itId).second;
	}
	else
		return 0;
}

// ----------------------------------------------------------------------------
bool CMissionQueueManager::getPlayerPositions(uint32 queueId, const NLMISC::CEntityId &id, uint16 &position, uint16 &positionOnline, bool &hasPlayerInCritZone) const
{
	const map<uint32,CMissionQueue>::const_iterator it = _Queues.find(queueId);
	if ( it == _Queues.end() )
	{
		return false;
	}

	return (*it).second.getPlayerPositions(id, position, positionOnline, hasPlayerInCritZone);
}

// ----------------------------------------------------------------------------
void CMissionQueueManager::playerWakesUp(const NLMISC::CEntityId &id, TAIAlias missionAlias)
{
	H_AUTO(CMissionQueueManagerPlayerWakesUp);

#ifdef NL_DEBUG
	bool found = false;
#endif
	
	map<uint32,CMissionQueue>::iterator it;

	for ( it = _Queues.begin() ; it != _Queues.end() ; ++it )
	{
		if ((*it).second.getMissionAlias() == missionAlias)
		{
			(*it).second.changePlayerAwakeState(id, true);
#ifdef NL_DEBUG
			found = true;
#endif
		}
	}

#ifdef NL_DEBUG
	nlassert(found == true);
#endif
}

// ----------------------------------------------------------------------------
void CMissionQueueManager::dump()
{
	nlinfo("_QueueIdCounter = %u", _QueueIdCounter);
	nlinfo(" Nb queues = %u", _Queues.size());
	for ( map<uint32,CMissionQueue>::iterator it = _Queues.begin() ; it != _Queues.end() ; ++it )
	{
		nlinfo("-------------------------------------------------------");
		(*it).second.dump();
	}
}

// ----------------------------------------------------------------------------
// PERSISTENCE METHODS
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// If the following macro is defined then the macros such as PERSISTENT_CLASS, PERSISTENT_DATA, PERSISTENT_PRE_STORE, etc
// are undefined after inclusion of persistent_data_template.h
#define PERSISTENT_MACROS_AUTO_UNDEF

//-----------------------------------------------------------------------------
// Persistent data for CQueueList
//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS CQueueVect

#define PERSISTENT_PRE_STORE\
	H_AUTO(CQueueVectStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CQueueVectApply);\

#define PERSISTENT_DATA\
	PROP_VECT(uint32, QueueIds)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

//-----------------------------------------------------------------------------
// Persistent data for CMissionQueueManager
//-----------------------------------------------------------------------------
#define PERSISTENT_CLASS CMissionQueueManager

#define PERSISTENT_PRE_STORE\
	H_AUTO(CMissionQueueManagerStore);\
	
#define PERSISTENT_PRE_APPLY\
	H_AUTO(CMissionQueueManagerApply);\
	
#define PERSISTENT_DATA\
	PROP(uint32, _QueueIdCounter)\
	PROP_MAP(std::string, uint32, _QueueNamesToIds)\
	STRUCT_MAP(uint32, CMissionQueue, _Queues)\
	STRUCT_MAP(CEntityId, CQueueVect, _PlayerQueues)\
	

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

