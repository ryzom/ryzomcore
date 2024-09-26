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
#include "mission_step_queue.h"
#include "mission_manager.h"
#include "mission_queue_manager.h"
#include "mission_log.h"
#include "mission_parser.h"
#include "ai_alias_translator.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "team_manager/team_manager.h"

#include "server_share/msg_ai_service.h"

#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;

MISSION_REGISTER_STEP(CMissionStepQueueStart,"queue_start")
MISSION_REGISTER_STEP(CMissionStepQueueEnd,"queue_end")

//----------------------------------------------------------------------------
bool CMissionStepQueueStart::buildStep( uint32 line, const vector< string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{
	_SourceLine = line;
	bool ret = true;
	if ( script.size() != 3)
	{
		MISLOGSYNTAXERROR("<name> <timer in ticks>");
		return false;
	}
	else
	{
		BOMB_IF(missionData.Template == NULL, "missionData.Template should never be NULL", return false);

		if (missionData.Name.empty())
		{
			nlwarning("Problem : mission has no name");
			QueueName = "default_" + script[1];	
		}
		else
			QueueName = missionData.Name + "_" + script[1];
		
		// remove all blanks in name
		string::size_type pos = QueueName.find_first_of(" ");
		while ( pos != string::npos )
		{
			QueueName.erase(pos,1);
			pos = QueueName.find_first_of(" ");
		}
		
		NLMISC::fromString(script[2], Timer);
		
		return true;
	}
}

//----------------------------------------------------------------------------
uint CMissionStepQueueStart::processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
{
	if( event.Type == CMissionEvent::QueueEntryOk )
	{
		return 1;
	}
	return 0;
}
	
//----------------------------------------------------------------------------
void CMissionStepQueueStart::getInitState( std::vector<uint32>& ret )
{
	ret.clear();
	ret.resize( 1 );
	ret[0] = 1;
}

// -------------------------------------------------------------------------------------------
static void getDHMS(uint32 nSecondsIN, uint32 &nDays, uint32 &nHours, uint32 &nMinutes, uint32 &nSeconds)
{
	uint32 seconds = nSecondsIN;
	nMinutes = uint32(seconds/60);
	nHours = uint32(nMinutes/60);
	nDays = uint32(nHours/24);
	seconds = seconds%60;
	nMinutes = nMinutes%60;
	nHours = nHours%24;
	if (seconds > 0)
	{
		++nMinutes;
		if (nMinutes == 60)
		{
			--nMinutes;
			++nHours;
			if (nHours == 24)
			{
				--nHours;
				++nDays;
			}
		}
	}
	nSeconds = seconds;
}


//----------------------------------------------------------------------------
uint32 CMissionStepQueueStart::sendStepText(CCharacter * user,const std::vector<uint32>& stepStates,const NLMISC::CEntityId & giver)
{
	//---- update local parameters from user ----//
	nlassert(user);
	// get queueId from Name
	const uint32 queueId = CMissionQueueManager::getInstance()->getQueueId(QueueName);
	// get position for user and queueId and update local position
	CMissionQueueManager::getInstance()->getPlayerPositions(queueId, user->getId(), _NbWaiters, _NbOnlineWaiters, _HasPlayerInCriticalZone);

	// get text params
	return IMissionStepTemplate::sendStepText(user,stepStates,giver);
}

//----------------------------------------------------------------------------
void CMissionStepQueueStart::getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
{
	// get estimated remaining time
	uint32 days, hours, minutes, seconds;
	getDHMS(uint32(Timer*(_HasPlayerInCriticalZone?1+_NbOnlineWaiters:_NbOnlineWaiters)*CTickEventHandler::getGameTimeStep()),
		days, hours, minutes, seconds);
	
	static const std::string stepText = "MISSION_QUEUE_WAIT";
	textPtr = &stepText;
	nlassert( subStepStates.size() == 1);
	retParams.push_back(STRING_MANAGER::TParam());
	retParams.back().Type = STRING_MANAGER::integer;
	retParams.back().Int = _NbWaiters;
	
	retParams.push_back(STRING_MANAGER::TParam());
	retParams.back().Type = STRING_MANAGER::integer;
	retParams.back().Int = _NbOnlineWaiters;

	retParams.push_back(STRING_MANAGER::TParam());
	retParams.back().Type = STRING_MANAGER::integer;
	retParams.back().Int = days;

	retParams.push_back(STRING_MANAGER::TParam());
	retParams.back().Type = STRING_MANAGER::integer;
	retParams.back().Int = hours;

	retParams.push_back(STRING_MANAGER::TParam());
	retParams.back().Type = STRING_MANAGER::integer;
	retParams.back().Int = minutes;
}

//----------------------------------------------------------------------------
void CMissionStepQueueStart::onActivation(CMission* inst,uint32 stepIndex, std::list< CMissionEvent * > & eventList)
{
	IMissionStepTemplate::onActivation(inst,stepIndex,eventList);

	CCharacter * user = inst->getMainEntity();
	if ( user )
	{
		CMissionQueueManager::getInstance()->addPlayerInQueue( user->getId(), inst, (uint16)stepIndex, QueueName, Timer );
	}
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

//----------------------------------------------------------------------------
bool CMissionStepQueueEnd::buildStep( uint32 line, const vector< string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{
	_SourceLine = line;
	bool ret = true;
	if ( script.size() != 2)
	{
		MISLOGSYNTAXERROR("<name>");
		return false;
	}
	BOMB_IF(missionData.Template == NULL, "missionData.Template should never be NULL", return false);

	if (missionData.Name.empty())
	{
		nlwarning("Problem : mission has no name");
		QueueName = "default_" + script[1];	
	}
	else
		QueueName = missionData.Name + "_" + script[1];

	// remove all blanks in name
	string::size_type pos = QueueName.find_first_of(" ");
	while ( pos != string::npos )
	{
		QueueName.erase(pos,1);
		pos = QueueName.find_first_of(" ");
	}

	return true;
}

//----------------------------------------------------------------------------
uint CMissionStepQueueEnd::processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
{
	if( event.Type == CMissionEvent::QueueExit )
	{
		// remove player from queue
		uint32 queueId = CMissionQueueManager::getInstance()->getQueueId(QueueName);
		BOMB_IF(queueId == 0, "Bad id for queue", return 0);

		CEntityId userId = getEntityIdFromRow(userRow);
		CMissionQueueManager::getInstance()->removePlayerFromQueue( userId, queueId);

		return 1;
	}
	return 0;
}
	
//----------------------------------------------------------------------------
void CMissionStepQueueEnd::getInitState( std::vector<uint32>& ret )
{
	ret.clear();
	ret.resize( 1 );
	ret[0] = 1;
}

//----------------------------------------------------------------------------
void CMissionStepQueueEnd::onActivation(CMission* inst,uint32 stepIndex, std::list< CMissionEvent * > & eventList)
{
	IMissionStepTemplate::onActivation(inst,stepIndex,eventList);

	// set queue id to 0 in mission as it's no longer in a queue, and reset critical part end date
	if (inst)
	{
		inst->setWaitingQueueId(0);
		inst->setCriticalPartEndDate(0);
	}

	// as soon as this step is activated, it must be validated and we go to the next
	//->generate an event that validates this step
	CMissionEventQueueExit *event = new CMissionEventQueueExit();
	if (event != NULL)
		eventList.push_back(event);
}
