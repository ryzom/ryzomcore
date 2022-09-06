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

/***************************************************************************************************
Steps linked with AI concepts/events
	- escort		: escort a bot 
	- wait_message	: wait for a specific AI event
***************************************************************************************************/

class CMissionStepEscort : public IMissionStepTemplate
{

	std::vector<TAIAlias>	Aliases;
	bool					SaveAll;
		
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );

	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow );

	void getInitState( std::vector<uint32>& ret );

	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates);

	virtual void onActivation(CMission* inst,uint32 stepIndex, std::list< CMissionEvent * > & eventList);

	virtual void getEscortGroups( std::vector< TAIAlias > & groups );
	
	virtual bool checkEscortFailure( bool groupWiped );

	bool checkTextConsistency();

	MISSION_STEP_GETNEWPTR(CMissionStepEscort)
	
};


class CMissionStepAIMsg : public IMissionStepTemplate
{
	std::string _Msg;

	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );

	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow );

	void getInitState( std::vector<uint32>& ret );

	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
	{}

	bool checkTextConsistency();

	MISSION_STEP_GETNEWPTR(CMissionStepAIMsg)
};


#endif
