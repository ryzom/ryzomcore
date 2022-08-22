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
#include "mission_manager/mission_solo.h"
#include "server_share/msg_ai_service.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "team_manager/team_manager.h"
#include "mission_manager/mission_manager.h"
#include "world_instances.h"
#include "zone_manager.h"
#include "creature_manager/creature_manager.h"
#include "primitives_parser.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CMissionSolo);

//----------------------------------------------------------------------------
void CMissionSolo::updateUsersJournalEntry()
{
	CCharacter * user = PlayerManager.getChar( _Taker );
	if ( !user )
	{
		nlwarning( "<MISSIONS>cant find user %u", _Taker.getIndex() );
		return;
	}
	updateUserJournalEntry( *user, "" );
}

void CMissionSolo::clearUsersJournalEntry()
{	
	CCharacter * user = PlayerManager.getChar( _Taker );
	if ( user )
	{
		CBankAccessor_PLR::TMISSIONS::TArray &missionItem = CBankAccessor_PLR::getMISSIONS().getArray(_ClientIndex);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:TYPE",_ClientIndex), 0);
		missionItem.setTYPE(user->_PropertyDatabase, 0);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:ICON",_ClientIndex), 0);
		missionItem.setICON(user->_PropertyDatabase, CSheetId::Unknown);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:TITLE",_ClientIndex), 0);
		missionItem.setTITLE(user->_PropertyDatabase, 0);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:DETAIL_TEXT",_ClientIndex), 0);
		missionItem.setDETAIL_TEXT(user->_PropertyDatabase, 0);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:END_DATE",_ClientIndex), 0 );
		missionItem.setEND_DATE(user->_PropertyDatabase, 0);
//		user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:BEGIN_DATE",_ClientIndex), 0 );
		missionItem.setBEGIN_DATE(user->_PropertyDatabase, 0);
		for (uint i = 0; i < NB_JOURNAL_COORDS; i++)
		{
			CBankAccessor_PLR::TMISSIONS::TArray::TTARGET &targetItem = CBankAccessor_PLR::getMISSIONS().getArray(_ClientIndex).getTARGET(i);

//			user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:TARGET%u:TITLE",_ClientIndex,i), 0);
			targetItem.setTITLE(user->_PropertyDatabase, 0);
//			user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:TARGET%u:X",_ClientIndex,i), 0);
			targetItem.setX(user->_PropertyDatabase, 0);
//			user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:TARGET%u:Y",_ClientIndex,i), 0);
			targetItem.setY(user->_PropertyDatabase, 0);
		}
		for (uint i = 0; i < NB_STEP_PER_MISSION; i++)
		{
//			user->_PropertyDatabase.setProp( NLMISC::toString( "MISSIONS:%u:GOALS:%u:TEXT",_ClientIndex,i), 0);
			missionItem.getGOALS().getArray(i).setTEXT(user->_PropertyDatabase, 0);
		}	
	}
}


//----------------------------------------------------------------------------
void CMissionSolo::setupEscort(const std::vector<TAIAlias> & aliases)
{
	TDataSetRow row = TheDataset.getDataSetRow( CAIAliasTranslator::getInstance()->getEntityId( _Giver ) );
	if ( !TheDataset.isAccessible( row ) )
	{
		nlwarning("<MISSIONS> Cant setup escort : giver is invalid %s", CPrimitivesParser::aliasToString(_Giver).c_str() );
		return;
	}
	CCharacter* user = PlayerManager.getChar( _Taker );
	if ( !user )
	{
		nlwarning("<MISSIONS> Invalid user %u",_Taker.getIndex() );
		return;
	}

	CSetEscortTeamId msg;
	msg.Groups = aliases;
	CTeam * team = TeamManager.getRealTeam(	user->getTeamId() );
	if ( !team )
		TeamManager.addFakeTeam( user );
	msg.TeamId = user->getTeamId();
	
	CMirrorPropValueRO<uint32>	in(TheDataset, row, DSPropertyAI_INSTANCE);
	msg.InstanceNumber = in;
	CWorldInstances::instance().msgToAIInstance(in, msg);
}

//----------------------------------------------------------------------------
void CMissionSolo::getEntities(std::vector<TDataSetRow>& entities)
{
	entities.resize(1);
	entities[0] = _Taker;
}

//----------------------------------------------------------------------------
void CMissionSolo::stopChildren()
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
				CMissionSolo * mission = dynamic_cast<CMissionSolo*>( child->Instances[j] );
				if ( mission && mission->_Taker == _Taker )
					mission->onFailure( true,false );
			}
		}
		else
			nlwarning("<MISSIONS> : invalid child template %u",templ->ChildrenMissions[i] );
	}
}

//----------------------------------------------------------------------------
void CMissionSolo::onFailure(bool ignoreJumps, bool sendMessage)
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	CMission::onFailure(ignoreJumps);

	if ( getProcessingState() == CMissionBaseBehaviour::Normal  )
	{
		CCharacter * user = PlayerManager.getChar( _Taker );
		if (user != NULL)
		{
			if (sendMessage && !templ->Tags.NoList && !templ->Tags.AutoRemove)
				CCharacter::sendDynamicSystemMessage(_Taker, "MISSION_FAILED");

			if ( templ->Tags.NoList || isChained() || templ->Tags.AutoRemove )
				user->removeMission(getTemplateId(), mr_fail);
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
void CMissionSolo::forceSuccess()
{
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _Mission->getTemplateId() );
	nlassert(templ);
	CCharacter * user = PlayerManager.getChar( _Taker );
	if ( user )
	{
		_ProcessingState = Normal;
		CMissionEventMissionDone  event(templ->Alias);
		user->addSuccessfulMissions( *templ );
		if ( !templ->Tags.NoList && !templ->Tags.AutoRemove )
			CCharacter::sendDynamicSystemMessage( user->getId(), isChained()?"EGS_MISSION_STEP_SUCCESS":"EGS_MISSION_SUCCESS");
		CMissionManager::getInstance()->missionDoneOnce(templ);
		stopChildren();
		user->processMissionEvent( event );
		
		// only remove no list missions, other must be manually removed by user
		if ( templ->Tags.NoList || isChained() || templ->Tags.AutoRemove )
		{
			updateEncyclopedia();
			user->removeMission(getTemplateId(), mr_success);
		}
		else
		{
			setSuccessFlag();
		}
	}
	else
		nlwarning("<MISSIONS> alias %s : invalid user", CPrimitivesParser::aliasToString(templ->Alias).c_str());
}

CCharacter* CMissionSolo::getMainEntity()
{
	return PlayerManager.getChar(_Taker);
}
