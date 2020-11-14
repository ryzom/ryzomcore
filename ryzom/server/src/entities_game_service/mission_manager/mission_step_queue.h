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



#ifndef RY_MISSION_STEP_AI_H
#define RY_MISSION_STEP_AI_H

#include "mission_step_template.h"
#include "nel/misc/smart_ptr.h"

/***************************************************************************************************
Steps linked with waiting queues
	- queue_start	: create a queue or insert player in queue
	- queue_end 	: remove player from queue, next waiting player can validate the queue_start step
***************************************************************************************************/

/**
 * class used for queue start steps
 * \author David Fleury
 * \author Nevrax France
 * \date 2005
 */
class CMissionStepQueueStart : public IMissionStepTemplate, public NLMISC::CRefCount
{
public:
	virtual bool buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );
	
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow );
	
	void getInitState( std::vector<uint32>& ret );
	
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates);

	virtual uint32	sendStepText(CCharacter * user,const std::vector<uint32>& stepStates,const NLMISC::CEntityId & giver);
	
	virtual void onActivation(CMission* inst,uint32 stepIndex, std::list< CMissionEvent * > & eventList);

	MISSION_STEP_GETNEWPTR(CMissionStepQueueStart)

	std::string QueueName;
	uint32		Timer; // in ticks

private:
	uint16		_NbWaiters;
	uint16		_NbOnlineWaiters;
	bool		_HasPlayerInCriticalZone;
};

typedef NLMISC::CRefPtr<CMissionStepQueueStart> CMissionStepQueueStartRefPtr;

/**
 * class used for queue end steps
 * \author David Fleury
 * \author Nevrax France
 * \date 2005
 */
class CMissionStepQueueEnd : public IMissionStepTemplate
{
public:

	virtual bool buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );
	
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow );
	
	void getInitState( std::vector<uint32>& ret );
	
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{}
	
	virtual void onActivation(CMission* inst,uint32 stepIndex, std::list< CMissionEvent * > & eventList);

	virtual bool isDisplayed() const { return false; }

	std::string QueueName;
		
	MISSION_STEP_GETNEWPTR(CMissionStepQueueEnd)	
};

#endif // RY_MISSION_STEP_AI_H //
