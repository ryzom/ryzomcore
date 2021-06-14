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
#include "mission_manager/mission_step_ai.h"
#include "mission_manager/mission_manager.h"
#include "mission_manager/mission_log.h"
#include "mission_manager/mission_parser.h"
#include "mission_manager/ai_alias_translator.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "team_manager/team_manager.h"

#include "server_share/msg_ai_service.h"

#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;

MISSION_REGISTER_STEP(CMissionStepEscort,"escort")
MISSION_REGISTER_STEP(CMissionStepAIMsg,"wait_msg")

//----------------------------------------------------------------------------
bool CMissionStepEscort::buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{
	_SourceLine = line;
	if ( script.size() != 2 && script.size() != 3 )
	{
		MISLOGSYNTAXERROR("<group_name>[:save_all]");
		return false;
	}
	string name = CMissionParser::getNoBlankString(script[1]);
	CAIAliasTranslator::getInstance()->getGroupAliasesFromName( name, Aliases  );
	if ( Aliases.empty() )
	{
		MISLOGERROR1("invalid group name %s", name.c_str());
		return false;
	}
	SaveAll = false;
	if ( script.size() == 3 )
	{
		if ( CMissionParser::getNoBlankString(script[2]) == "save_all" )
			SaveAll = true;
	}
	return true;
}

//----------------------------------------------------------------------------
uint CMissionStepEscort::processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
{
	// not check here : they are done before. If a talk event comes here, the step is complete
	if( event.Type == CMissionEvent::Escort )
	{
		CMissionEventEscort & eventSpe = (CMissionEventEscort &) event;
		const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate(eventSpe.Mission);
		if ( !templ )
		{
			LOGMISSIONSTEPERROR("escort : Invalid mission template " + toString(eventSpe.Mission));
			return 0;
		}

		/*vector<TAIAlias> groups;
		templ->getEscortGroups(groups);

		for ( uint i = 0;i < groups.size(); i++)
			CMissionManager::getInstance()->unregisterEscort( groups[i], TheDataset.getEntityId( userRow ) );
			*/
		for ( uint i = 0;i < Aliases.size(); i++)
			CMissionManager::getInstance()->unregisterEscort( Aliases[i], TheDataset.getEntityId( userRow ) );

		CCharacter * user = PlayerManager.getChar( userRow );
		if( user)
		{
			CTeam * team = TeamManager.getTeam( user->getTeamId() );
			if ( team->isFake() )
				TeamManager.removeFakeTeam(user);
		}
		
		LOGMISSIONSTEPSUCCESS("escort");

		return 1;
	}
	return 0;
}
	
//----------------------------------------------------------------------------
void CMissionStepEscort::getInitState( std::vector<uint32>& ret )
{
	ret.clear();
	ret.resize( 1 );
	ret[0] = 1;
}

//----------------------------------------------------------------------------
void CMissionStepEscort::getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates)
{
	/// to be overloaded in script
	/*
	static const std::string stepText = "MIS_ESCORT";
	if( subStepStates[0] != 0 )
	{
		nbSubSteps++;
		retParams.push_back(STRING_MANAGER::TParam());
		retParams.back().Type = STRING_MANAGER::string_id;
		retParams.back().Identifier = Name;
		///\todo nico that wont work. STRING_MANAGER must be upgraded...

	}
	textPtr = &stepText;	
	*/
}

//----------------------------------------------------------------------------
void CMissionStepEscort::onActivation(CMission* inst,uint32 stepIndex, std::list< CMissionEvent * > & eventList)
{
	inst->setupEscort( Aliases );
	if ( inst->getMainEntity() == NULL )
		return;
	for ( uint i = 0; i < Aliases.size(); i++ )
		CMissionManager::getInstance()->registerEscort( Aliases[i],inst->getTemplateId(), inst->getMainEntity()->getId() );
}

//----------------------------------------------------------------------------
void CMissionStepEscort::getEscortGroups( std::vector< TAIAlias > & groups )
{
	groups.insert(groups.end(),Aliases.begin(),Aliases.end());
}

//----------------------------------------------------------------------------
bool CMissionStepEscort::checkEscortFailure( bool groupWiped )
{
	if ( groupWiped )
		return true;
	return SaveAll;
}

//----------------------------------------------------------------------------
bool CMissionStepEscort::checkTextConsistency()
{
	if ( !_IsInOverridenOOO && isDisplayed() && _OverridenText.empty() )
	{
		MISLOG("non overridden escort text");
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
bool CMissionStepAIMsg::buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{
	_SourceLine = line;
	if ( script.size() != 2 )
	{
		MISLOGSYNTAXERROR("<msg name>");
		return false;
	}
	_Msg = CMissionParser::getNoBlankString(script[1]);
	return true;
}

//----------------------------------------------------------------------------
uint CMissionStepAIMsg::processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow )
{
	// not check here : they are done befor. If a talk event comes here, the step is complete
	if( event.Type == CMissionEvent::AIMsg )
	{
		CMissionEventAIMsg & eventSpe = (CMissionEventAIMsg &) event;
		nlwarning("CMissionStepAIMsg : Message from event = '%s', message of mission = '%s'",  eventSpe.Msg.c_str(), _Msg.c_str());
		if ( eventSpe.Msg == _Msg )
		{
			LOGMISSIONSTEPSUCCESS("wait_msg");
			return 1;
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
void CMissionStepAIMsg::getInitState( std::vector<uint32>& ret )
{
	ret.clear();
	ret.resize( 1 );
	ret[0] = 1;
}

//----------------------------------------------------------------------------
bool CMissionStepAIMsg::checkTextConsistency()
{
	if ( !_IsInOverridenOOO && isDisplayed() && _OverridenText.empty() )
	{
		MISLOG("non overridden wait_msg text");
		return false;
	}
	return true;
}
