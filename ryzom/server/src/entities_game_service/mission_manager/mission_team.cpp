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
	// a mission hidden from the journal has no entry (0xFF)
	if ( _ClientIndex == 0xFF )
		return;

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

		clearGroupJournalEntry(*user, _ClientIndex);
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
	// a child's failure actions can remove this mission: nothing of it is read in the loop
	const uint16 teamId = _TeamId;
	for ( uint i = 0; i < templ->ChildrenMissions.size(); i++ )
	{
		const CMissionTemplate * child = CMissionManager::getInstance()->getTemplate( templ->ChildrenMissions[i] );
		if ( child )
		{
			// a failed instance leaves Instances (see CMissionTemplate::isInstance)
			const std::vector<CMission *> instances = child->Instances;
			for (uint j = 0; j < instances.size(); j++)
			{
				if (!child->isInstance(instances[j]))
					continue;
				CMissionTeam *mission = dynamic_cast<CMissionTeam *>(instances[j]);
				if (mission && mission->_TeamId == teamId)
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
	// the events of the failure actions can remove this mission
	NLMISC::CRefPtr<CMissionBaseBehaviour> self = this;
	CMission::onFailure(ignoreJumps);
	if (self == NULL)
		return;
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
		// the failure actions of the other instances or of the children can remove this mission or end it: the event is
		// still processed, and nothing of this mission is used
		NLMISC::CRefPtr<CMissionBaseBehaviour> self = this;
		CMissionManager::getInstance()->missionDoneOnce(templ, this);
		if (self != nullptr)
			stopChildren();
		if (self == nullptr || getFinished())
		{
			team->processTeamMissionEvent(eventList, CAIAliasTranslator::Invalid);
			return;
		}

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

		// The mission is done before the event, as when its last step ends: the event can end a parent mission, which
		// removes its children, this one among them. Nothing of this mission is used from here
		team->processTeamMissionEvent(eventList, CAIAliasTranslator::Invalid);
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
