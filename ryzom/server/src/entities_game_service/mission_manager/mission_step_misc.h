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



#ifndef RY_MISSION_STEP_MISC_H
#define RY_MISSION_STEP_MISC_H

#include "mission_step_template.h"

/*
 * 
 */
class CMissionStepHandleCreate : public IMissionStepTemplate
{
	TAIAlias	GroupAlias;
	uint32		DespawnTime;
	
	virtual bool buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );
	void onActivation(CMission* instance,uint32 stepIndex,std::list< CMissionEvent * > & eventList);
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow );
	void getInitState( std::vector<uint32>& ret );
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates);
	
	MISSION_STEP_GETNEWPTR(CMissionStepHandleCreate)
};

/*
 *
 */
class CMissionStepHandleRelease : public IMissionStepTemplate
{
	TAIAlias	GroupAlias;
	
	virtual bool buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );
	void onActivation(CMission* instance,uint32 stepIndex,std::list< CMissionEvent * > & eventList);
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow );
	void getInitState( std::vector<uint32>& ret );
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates);
	
	MISSION_STEP_GETNEWPTR(CMissionStepHandleRelease)
};

#endif
