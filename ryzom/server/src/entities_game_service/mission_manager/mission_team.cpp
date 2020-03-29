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
#include "mission_manager/mission_team.h"
#include "team_manager/team_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "server_share/msg_ai_service.h"
#include "world_instances.h"
#include "primitives_parser.h"
#include "mission_manager/mission_manager.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CMissionTeam);

//----------------------------------------------------------------------------
void CMissionTeam::updateUsersJournalEntry()
{
	CTeam * team = TeamManager.getRealTeam( _TeamId );
	nlassert(team);

	for ( list<CEntityId>::const_iterator it = team->getTeamMembers().begin(); it != team->getTeamMembers().end();++it )
	{
		CCharacter * user = PlayerManager.getChar( *it );
		if ( !user )
		{
			nlwarning( "<MISSIONS>cant find user %s", (*it).toString().c_str() );
			continue;
		}
		updateUserJournalEntry(*user,"GROUP:");
	}
}

//----------------------------------------------------------------------------
void CMissionTeam::clearUsersJournalEntry()
{
	CTeam * team = TeamManager.getRealTeam( _TeamId );
	if ( !team )
	{
		nlwarning( "<MISSIONS>cant find team ID : %d", _TeamId );
		return;
	}
	
	for ( list<CEntityId>::const_iterator it = team->getTeamMembers().begin(); it != team->getTeamMembers().end();++it )
	{
		CCharacter * user = PlayerManager.getChar( *it );
		if ( !user )
		{
			nlwarning( "<MISSIONS>cant find user %s", (*it).toString().c_str() );
			continue;
		}

		CBankAccessor_PLR::TGROUP::TMISSIONS::TArray &missionItem = CBankAccessor_PLR::getGROUP().getMISSIONS().getArray(_ClientIndex);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:TYPE",_ClientIndex), 0);
		missionItem.setTYPE(user->_PropertyDatabase, 0);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:ICON",_ClientIndex), 0);
		missionItem.setICON(user->_PropertyDatabase, CSheetId::Unknown);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:TITLE",_ClientIndex), 0);
		missionItem.setTITLE(user->_PropertyDatabase, 0);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:DETAIL_TEXT",_ClientIndex), 0);
		missionItem.setDETAIL_TEXT(user->_PropertyDatabase, 0);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:END_DATE",_ClientIndex), 0 );
		missionItem.setEND_DATE(user->_PropertyDatabase, 0);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:BEGIN_DATE",_ClientIndex), 0 );
		missionItem.setBEGIN_DATE(user->_PropertyDatabase, 0);
		for (uint i = 0; i < NB_JOURNAL_COORDS; i++)
		{
			CBankAccessor_PLR::TGROUP::TMISSIONS::TArray::TTARGET &targetItem = missionItem.getTARGET(i);
//			user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:TARGET%u:TITLE",_ClientIndex,i), 0);
			targetItem.setTITLE(user->_PropertyDatabase, 0);
//			user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:TARGET%u:X",_ClientIndex,i), 0);
			targetItem.setX(user->_PropertyDatabase, 0);
//			user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:TARGET%u:Y",_ClientIndex,i), 0);
			targetItem.setY(user->_PropertyDatabase, 0);
		}
		for (uint i = 0; i < NB_STEP_PER_MISSION; i++)
		{
//			user->_PropertyDatabase.setProp( NLMISC::toString( "GROUP:MISSIONS:%u:GOALS:%u:TEXT",_ClientIndex,i), 0);
			missionItem.getGOALS().getArray(i).setTEXT(user->_PropertyDatabase, 0);
		}	
	}
}

//----------------------------------------------------------------------------
void CMissionTeam::setupEscort(const std::vector<TAIAlias> & aliases)
{
	TDataSetRow row = TheDataset.getDataSetRow( CAIAliasTranslator::getInstance()->getEntityId( _Giver ) );
	if ( !TheDataset.isAccessible( row ) )
	{
		nlwarning("<MISSIONS> Cant setup escort : giver is invalid %s", CPrimitivesParser::aliasToString(_Giver).c_str() );
		return;
	}
	CSetEscortTeamId msg;
	CMirrorPropValueRO<uint32>	in(TheDataset, row, DSPropertyAI_INSTANCE);
	msg.InstanceNumber = in;
	msg.Groups = aliases;
	msg.TeamId = _TeamId;
	CWorldInstances::instance().msgToAIInstance(in, msg);
}

//----------------------------------------------------------------------------
void CMissionTeam::getEntities(std::vector<TDataSetRow>& entities)
{
	CTeam * team = TeamManager.getRealTeam( _TeamId );
	if ( team )
	{
		std::list<CEntityId>::const_iterator it = team->getTeamMembers().begin();
		for (;it != team->getTeamMembers().end(); ++it )
			entities.push_back( TheDataset.getDataSetRow(*it) );
	}
	else
		nlwarning("<MISSIONS> Invalid team %u", _TeamId);
}

//----------------------------------------------------------------------------
void CMissionTeam::stopChildren()
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	for ( uint i = 0; i < templ->ChildrenMissions.size(); i++ )
	{
		const CMissionTemplate * child = CMissionManager::getInstance()->getTemplate( templ->ChildrenMissions[i] );
		if ( child )
		{
			for ( uint j = 0; j < child->Instances.size(); j++ )
			{
				CMissionTeam * mission = dynamic_cast<CMissionTeam*>( child->Instances[j] );
				if ( mission && mission->_TeamId == _TeamId )
					mission->onFailure( true,false );
			}
		}
		else
			nlwarning("<MISSIONS> : invalid child template %u",templ->ChildrenMissions[i] );
	}
}

//----------------------------------------------------------------------------
void CMissionTeam::onFailure(bool ignoreJumps, bool sendMessage)
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	CMission::onFailure(ignoreJumps);
	sendMessage = ( sendMessage && !templ->Tags.NoList );
	if ( getProcessingState() == CMissionBaseBehaviour::Normal  )
	{
		CTeam * team = TeamManager.getRealTeam( _TeamId );
		if ( team )
		{
			if (sendMessage && !templ->Tags.NoList && !templ->Tags.AutoRemove)
				team->sendDynamicMessageToMembers("MISSION_FAILED",TVectorParamCheck());

			if ( templ->Tags.NoList || isChained() || templ->Tags.AutoRemove )
				team->removeMission(this, mr_fail);
			else
				setFailureFlag();
		}
		return;
	}
	if ( _ProcessingState == CMissionBaseBehaviour::InJump )
	{
		_ProcessingState = Normal;
		return;
	}
	else if ( _ProcessingState == CMissionBaseBehaviour::Complete )
	{
		forceSuccess();
		return;
	}
}

//----------------------------------------------------------------------------
void CMissionTeam::forceSuccess()
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	CTeam * team = TeamManager.getRealTeam( _TeamId );
	if ( team )
	{
		_ProcessingState = Normal;
		CMissionEventMissionDone  event(templ->Alias);
		std::list< CMissionEvent * > eventList;
		eventList.push_back( &event );


		const list<CEntityId> & members = team->getTeamMembers();
		for (list<CEntityId>::const_iterator it = members.begin() ; it != members.end() ; ++it)
		{
			CCharacter * c = PlayerManager.getChar( (*it) );
			if (c )
			{
				c->addSuccessfulMissions( *templ );
				if ( !templ->Tags.NoList && !templ->Tags.AutoRemove )
					CCharacter::sendDynamicSystemMessage( c->getId(), isChained()?"EGS_MISSION_STEP_SUCCESS":"EGS_MISSION_SUCCESS");
			}
		}	
		CMissionManager::getInstance()->missionDoneOnce(templ);
		stopChildren();
		team->processTeamMissionEvent( eventList, CAIAliasTranslator::Invalid  );
		
		// only remove no list missions, other must be manually removed by user
		if ( templ->Tags.NoList || isChained() || templ->Tags.AutoRemove )
		{
			updateEncyclopedia();
			team->removeMission(this, mr_success);
		}
		else
		{
			setSuccessFlag();
		}
	}
	else
		nlwarning("<MISSIONS> alias %s : invalid team", CPrimitivesParser::aliasToString(templ->Alias).c_str());
}

//----------------------------------------------------------------------------
CCharacter* CMissionTeam::getMainEntity()
{
	CTeam * team = TeamManager.getRealTeam(_TeamId);
	if ( team )
		return PlayerManager.getChar( team->getLeader() );
	nlwarning( "<MISSIONS> invalid team id '%u' ",_TeamId );
	return NULL;
}
