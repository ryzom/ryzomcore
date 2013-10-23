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

#include "team_manager/team_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "egs_mirror.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "player_manager/action_distance_checker.h"
#include "chat_groups_ids.h"
#include "mission_manager/mission_manager.h"

#include "game_share/chat_group.h"

#include "player_manager/cdb.h"
#include "player_manager/cdb_leaf.h"
#include "player_manager/cdb_branch.h"
#include "player_manager/cdb_synchronised.h"

#include "pvp_manager/pvp_manager_2.h"

#include "nel/net/service.h"

#include "game_share/pvp_clan.h"


using namespace std;
using namespace NLNET;
using namespace NLMISC;


extern CPlayerManager				PlayerManager;
extern uint8						TeamMembersStatusMaxValue;
extern CGenericXmlMsgHeaderManager	GenericMsgManager;

//---------------------------------------------------
// addAllTeamsToChatGroup :
//---------------------------------------------------
void CTeamManager::addAllTeamsToChatGroup()
{
	for( uint i = 0; i < _Teams.size(); i++ )
	{
		if ( _Teams[i].isValid() && !_Teams[i].isFake() )
		{
			// Creation du groupe pour le chat
			TGroupId idGroupe = CHAT_GROUPS_IDS::getTeamChatGroupId(_Teams[i].getTeamId());
			CMessage msgout("ADD_GROUP");
			msgout.serial( idGroupe );
			CChatGroup::TGroupType type = CChatGroup::team;
			msgout.serialEnum( type );
			sendMessageViaMirror( "IOS", msgout );
			_Teams[i].addAllMembersToChatGroup(idGroupe);
		}
	}
} // addAllTeamsToChatGroup //

//---------------------------------------------------
// leagueJoinProposal :
//---------------------------------------------------
void CTeamManager::joinLeagueProposal( CCharacter * leader, const CEntityId &targetId)
{
	//check already done
	nlassert(leader);

	const NLMISC::CEntityId &leaderId = leader->getId();
	if (targetId == leaderId )
	{
		CCharacter::sendDynamicSystemMessage( leader->getId(),"INVALID_LEAGUE_TARGET" );
		return;
	}

	// get targeted player
	CCharacter *invitedPlayer = PlayerManager.getOnlineChar( targetId );
	if ( invitedPlayer == NULL )
	{
		CCharacter::sendDynamicSystemMessage( leader->getId(),"INVALID_LEAGUE_TARGET" );
		return;
	}
	
	// god player are forbidden to team
	if (leader->godMode() || invitedPlayer->godMode())
	{
		nlwarning("<CTeamManager joinLeagueProposal> Player %s invited %s, but at least on of them is god, forbidden", 
			leaderId.toString().c_str(),
			targetId.toString().c_str());
		CCharacter::sendDynamicSystemMessage( leader->getId(),"TEAM_GOD_FORBIDDEN" );
		return;
	}

	TInviteRetCode code = isLeagueInvitableBy(invitedPlayer,leader);
	if ( code == AlreadyInvited )
	{
		CCharacter::sendDynamicSystemMessage( leader->getId(),"LEAGUE_ALREADY_INVITED" );
		return;
	}
	else if ( code == AlreadyInLeague )
	{
		CTeam * team = getRealTeam( invitedPlayer->getTeamId() );
		CCharacter::sendDynamicSystemMessage( leader->getId(),"LEAGUE_ALREADY_IN_LEAGUE" );
		return;
	}
	else if ( code == NotLeader )
	{
		CTeam * team = getRealTeam( invitedPlayer->getTeamId() );
		joinLeagueProposal(leader, team->getLeader());
		return;
	}
	else if ( code == CantInvite )
	{
		CCharacter::sendDynamicSystemMessage( leader->getId(),"LEAGUE_INVITOR_NOT_LEADER" );
		return;
	}

	/// the invitor must not be in the ignore list of the target
	if(invitedPlayer->hasInIgnoreList(leaderId))
	{
		SM_STATIC_PARAMS_1( params1, STRING_MANAGER::player );
		params1[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias( targetId) );
		// Use the standard "player declines your offer". Don't use specific message because
		// maybe not a good idea to inform a player that someone ignores him
		CCharacter::sendDynamicSystemMessage( leaderId, "TEAM_DECLINE", params1 );
		return;
	}

	//set the target's invitor
	invitedPlayer->setLeagueInvitor(leaderId);

	CEntityId msgTargetEId = targetId;
	
	//send the appropriate string to the client
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	params[0].setEIdAIAlias( leaderId, CAIAliasTranslator::getInstance()->getAIAlias(leaderId) );
	uint32 txt = STRING_MANAGER::sendStringToClient(TheDataset.getDataSetRow(targetId), "LEAGUE_PROPOSAL", params );
	
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( const_cast<CEntityId&>(msgTargetEId) );
	CBitMemStream bms;
	nlverify ( GenericMsgManager.pushNameToStream( "PVP_CHALLENGE:INVITATION", bms) );
	bms.serial( txt );
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	sendMessageViaMirror( NLNET::TServiceId(msgTargetEId.getDynamicId()), msgout );

	params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias( targetId ) );
	PHRASE_UTILITIES::sendDynamicSystemMessage(leader->getEntityRowId(), "LEAGUE_INVITE", params);
	
	leader->updateTarget();	
}

//---------------------------------------------------
// joinLeagueDecline :
//---------------------------------------------------
void CTeamManager::joinLeagueDecline( const NLMISC::CEntityId &charId)
{
	CCharacter * invited = PlayerManager.getOnlineChar(charId);
	if ( invited == NULL )
	{
		nlwarning("<CTeamManager joinLeagueDecline>Invalid char %s",charId.toString().c_str());
		return;
	}
	invited->setAfkState(false);
	if ( invited->getLeagueInvitor() == CEntityId::Unknown )
	{
		nlwarning("<CTeamManager joinLeagueDecline>character %s has an Invalid invitor",charId.toString().c_str());
		return;
	}

	//inform both players
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);	
	params[0].setEIdAIAlias( charId, CAIAliasTranslator::getInstance()->getAIAlias( charId) );
	PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(invited->getTeamInvitor()), "LEAGUE_DECLINE", params);
	
	params[0].setEIdAIAlias( invited->getTeamInvitor(), CAIAliasTranslator::getInstance()->getAIAlias( invited->getTeamInvitor() ) );
	PHRASE_UTILITIES::sendDynamicSystemMessage(invited->getEntityRowId(), "LEAGUE_YOU_DECLINE", params);

	//cancel the proposal
	invited->setLeagueInvitor( CEntityId::Unknown );
} // joinLeagueDecline //

//---------------------------------------------------
// joinLeagueAccept :
//---------------------------------------------------
void CTeamManager::joinLeagueAccept( const NLMISC::CEntityId &charId)
{
	// get the invited char
	CCharacter * invited = PlayerManager.getOnlineChar(charId);
	if ( invited == NULL )
	{
		nlwarning("<CTeamManager joinLeagueAccept>Invalid char %s",charId.toString().c_str());
		return;
	}

	// get the invitor id
	const NLMISC::CEntityId & invitorId = invited->getLeagueInvitor();

	if ( invitorId == CEntityId::Unknown )
	{
		nlwarning("<CTeamManager joinLeagueAccept>character %s has an Invalid invitor",charId.toString().c_str());
		return;
	}
	//get the invitor char
	CCharacter * invitor = PlayerManager.getOnlineChar(invitorId);
	if ( invitor == NULL  )
	{
		nlwarning("<CTeamManager joinLeagueAccept>character %s, Invalid invitor id %s",charId.toString().c_str(),invitorId.toString().c_str());
		invited->setLeagueInvitor( CEntityId::Unknown );
		return;
	}
	invitor->setAfkState(false);
	
	//cancel the proposal
	invited->setLeagueInvitor( CEntityId::Unknown );

	CTeam *teamInvitor;
	CTeam *teamInvited;
	//if the invited player had a fake team, remove it
	teamInvited = getRealTeam(invited->getTeamId());
	teamInvitor = getRealTeam(invitor->getTeamId());
	
	if ( !teamInvitor )
	{
		nlwarning("<CTeamManager joinLeagueAccept>character %s, invitor id %s, the invited or invitor player is not in a valid team. ",charId.toString().c_str(),invitor->getId().toString().c_str() );
		return;
	}
	
	
	// check that the invitor team have league else create them
	if (teamInvitor->getLeagueId() == DYN_CHAT_INVALID_CHAN )
	{
		teamInvitor->setLeague("League");
	}
	
	if (teamInvited) {
		const string playerName = CEntityIdTranslator::getInstance()->getByEntity(invited->getId()).toString();
		CPVPManager2::getInstance()->broadcastMessage(teamInvitor->getLeagueId(), string("<TEAM>"), "<-- "+playerName);
		teamInvited->setLeagueId(teamInvitor->getLeagueId());
		teamInvited->updateLeague();
	} else {
		const string playerName = CEntityIdTranslator::getInstance()->getByEntity(invited->getId()).toString();
		CPVPManager2::getInstance()->broadcastMessage(teamInvitor->getLeagueId(), string("<PLAYER>"), "<-- "+playerName);
		invited->setLeagueId(teamInvitor->getLeagueId(), true);
	}
	
} // joinLeagueAccept //


//---------------------------------------------------
// joinProposal :
//---------------------------------------------------
void CTeamManager::joinProposal( CCharacter * leader, const CEntityId &targetId)
{
	//check already done
	nlassert(leader);

	const NLMISC::CEntityId &leaderId = leader->getId();
	if (targetId == leaderId )
	{
		nlwarning("<CTeamManager joinProposal> Player %s invited himself in his team, cancel", leaderId.toString().c_str() );
		return;
	}

	// get targeted player
	CCharacter *invitedPlayer = PlayerManager.getOnlineChar( targetId );
	if ( invitedPlayer == NULL )
	{
		CCharacter::sendDynamicSystemMessage( leader->getId(),"INVALID_LEAGUE_TARGET" );
		return;
	}
	
	// god player are forbidden to team
	if (leader->godMode() || invitedPlayer->godMode())
	{
		nlwarning("<CTeamManager joinProposal> Player %s invited %s, but at least on of them is god, forbidden", 
			leaderId.toString().c_str(),
			targetId.toString().c_str());
		CCharacter::sendDynamicSystemMessage( leader->getId(),"TEAM_GOD_FORBIDDEN" );
		return;
	}

	TInviteRetCode code = isInvitableBy(invitedPlayer,leader);
	if ( code == AlreadyInvited )
	{
		CCharacter::sendDynamicSystemMessage( leader->getId(),"TEAM_ALREADY_INVITED" );
		return;
	}
	else if ( code == AlreadyInTeam )
	{
		CCharacter::sendDynamicSystemMessage( leader->getId(),"TEAM_TARGET_ALREADY_IN_TEAM" );
		return;
	}
	else if ( code == CantInviteEnemy )
	{
		CCharacter::sendDynamicSystemMessage( leader->getId(),"TEAM_CANT_INVITE_ENEMY" );
		return;
	}
	else if ( code == CantInvite )
	{
		CCharacter::sendDynamicSystemMessage( leader->getId(),"TEAM_CANT_INVITE" );
		return;
	}

	/// the invitor must not be in the ignore list of the target
	if(invitedPlayer->hasInIgnoreList(leaderId))
	{
		SM_STATIC_PARAMS_1( params1, STRING_MANAGER::player );
		params1[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias( targetId) );
		// Use the standard "player declines your offer". Don't use specific message because
		// maybe not a good idea to inform a player that someone ignores him
		CCharacter::sendDynamicSystemMessage( leaderId, "TEAM_DECLINE", params1 );
		return;
	}

	//set the target's invitor
	invitedPlayer->setTeamInvitor(leaderId);


	//send the appropriate string to the client
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	params[0].setEIdAIAlias( leaderId, CAIAliasTranslator::getInstance()->getAIAlias( leaderId ) );
	uint32 strId = STRING_MANAGER::sendStringToClient(TheDataset.getDataSetRow(targetId),"TEAM_PROPOSAL", params);

	//send the invitation msg to the target, with the id of the built string
	CMessage msgout( "IMPULSION_ID" );
	CBitMemStream bms;
	CEntityId targetIdToSerial = targetId;
	msgout.serial( targetIdToSerial );
	if ( ! GenericMsgManager.pushNameToStream( "TEAM:INVITATION", bms) )
	{
		nlwarning("<CTeamManager joinProposal> Msg name TEAM:INVITATION not found");
		return;
	}

	bms.serial(strId);
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	sendMessageViaMirror( NLNET::TServiceId(targetId.getDynamicId()), msgout );

//	TVectorParamCheck params;
//	params.resize(1);

	// inform the team leader	
//	params[0].Type = STRING_MANAGER::player;	
	params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias( targetId ) );
	PHRASE_UTILITIES::sendDynamicSystemMessage(leader->getEntityRowId(), "TEAM_INVITE", params);
	
	leader->updateTarget();
} // joinProposal //

//---------------------------------------------------
// joinDecline :
//---------------------------------------------------
void CTeamManager::joinDecline( const NLMISC::CEntityId &charId)
{
	CCharacter * invited = PlayerManager.getOnlineChar(charId);
	if ( invited == NULL )
	{
		nlwarning("<CTeamManager joinDecline>Invalid char %s",charId.toString().c_str());
		return;
	}
	invited->setAfkState(false);
	if ( invited->getTeamInvitor() == CEntityId::Unknown )
	{
		nlwarning("<CTeamManager joinDecline>character %s has an Invalid invitor",charId.toString().c_str());
		return;
	}

	//inform both players
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);	
	params[0].setEIdAIAlias( charId, CAIAliasTranslator::getInstance()->getAIAlias( charId) );
	PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(invited->getTeamInvitor()), "TEAM_DECLINE", params);
	
	params[0].setEIdAIAlias( invited->getTeamInvitor(), CAIAliasTranslator::getInstance()->getAIAlias( invited->getTeamInvitor() ) );
	PHRASE_UTILITIES::sendDynamicSystemMessage(invited->getEntityRowId(), "TEAM_YOU_DECLINE", params);

	//cancel the proposal
	invited->setTeamInvitor( CEntityId::Unknown );
} // joinDecline //

//---------------------------------------------------
// joinAccept :
//---------------------------------------------------
void CTeamManager::joinAccept( const NLMISC::CEntityId &charId)
{
	// get the invited char
	CCharacter * invited = PlayerManager.getOnlineChar(charId);
	if ( invited == NULL )
	{
		nlwarning("<CTeamManager joinAccept>Invalid char %s",charId.toString().c_str());
		return;
	}

	// get the invitor id
	const NLMISC::CEntityId & invitorId = invited->getTeamInvitor();

	if ( invitorId == CEntityId::Unknown )
	{
		nlwarning("<CTeamManager joinAccept>character %s has an Invalid invitor",charId.toString().c_str());
		return;
	}
	//get the invitor char
	CCharacter * invitor = PlayerManager.getOnlineChar(invitorId);
	if ( invitor == NULL  )
	{
		nlwarning("<CTeamManager joinAccept>character %s, Invalid invitor id %s",charId.toString().c_str(),invitorId.toString().c_str());
		invited->setTeamInvitor( CEntityId::Unknown );
		return;
	}
	invitor->setAfkState(false);
	//cancel the proposal
	invited->setTeamInvitor( CEntityId::Unknown );

	CTeam *team;
	//if the invited player had a fake team, remove it
	team = getTeam( invited->getTeamId() );
	
	if ( team )
	{
		// if the team is not fake, there is a problem...
		if ( !team->isFake() )
		{
			nlwarning("<CTeamManager joinAccept>character %s, invitor id %s, the invited player is in a valid team. ",charId.toString().c_str(),invitor->getId().toString().c_str() );
			return ;
		}
		else
		{
			team->release();
			removeTeam( invited->getTeamId() );
		}
	}

	team = getTeam(invitor->getTeamId());
	//  create the team if it does not exist
	if ( !team )
	{
		//check for reallocation
		if ( _FirstFreeTeamId >= (uint16)_Teams.size() )
		{
			_Teams.resize( _TeamAllocStep + _FirstFreeTeamId );
			for (uint i = _FirstFreeTeamId; i < _Teams.size(); i++)
			{
				_Teams[i].setNextFreeId( i + 1 );
			}
		}
		//get a pointer on the new team
		team = &_Teams[_FirstFreeTeamId];
		// set the invitor team id
		invitor->setTeamId(_FirstFreeTeamId);
		//init the team
		team->init( invitor,_FirstFreeTeamId );
		// update alloc data
		_FirstFreeTeamId = team->getNextFreeId();
	}
	// If the team is fake transform it in an unfake team
	else if ( team->isFake() )
	{
		// init the team
		team->init( invitor,invitor->getTeamId() );
	}

	// check the team size
	if ( team->getTeamSize() < CTEAM::TeamMaxNbMembers )
	{
		//add the new character to the team
		team->addCharacter(invited);
	}
	else
	{
		CCharacter::sendDynamicSystemMessage(charId,"OPS_TEAM_MAX_SIZE_REACHED");
//		CCharacter::sendMessageToClient(charId,"OPS_TEAM_MAX_SIZE_REACHED");
	}

	CMissionManager::getInstance()->updateEscortTeam( charId );
} // joinAccept //

//---------------------------------------------------
// quitTeam :
//---------------------------------------------------
void CTeamManager::quitTeam( const NLMISC::CEntityId &charId )
{
	removeCharacter( charId );
	CMissionManager::getInstance()->updateEscortTeam( charId );
}

//---------------------------------------------------
// kickCharacter :
//---------------------------------------------------
void CTeamManager::kickCharacter( CCharacter * leader, uint8 memberIndex )
{
	// check must be done before
	nlassert(leader);
	const NLMISC::CEntityId & leaderId = leader->getId();
	
	// get the leader's team
	CTeam * team = getRealTeam( leader->getTeamId() );	
	if ( team == NULL )
	{
		nlwarning("<CTeamManager kickCharacter> Player %s has an invalid team id %d", leaderId.toString().c_str(), leader->getTeamId() );
		return;
	}
	// only leaders can kick
	if ( leaderId != team->getLeader() )
	{
		nlwarning("<CTeamManager kickCharacter> Player %s tries kicking but is not the leader", leaderId.toString().c_str() );
		return;
	}

	// increment the target index as the leader is not in its team list
	memberIndex++;
	const std::list< NLMISC::CEntityId >  & members = team->getTeamMembers();
	std::list< NLMISC::CEntityId >::const_iterator it = members.begin();
	uint i = 0;
	for (; i < memberIndex; i++ )
	{
		if ( it == members.end() )
		{
			nlwarning("<CTeamManager kickCharacter> Player %s kicks teammate %d but there are %d members", leaderId.toString().c_str(), memberIndex,i+1 );
			return;
		}
		++it;
	}
	if ( it == members.end() )
	{
		nlwarning("<CTeamManager kickCharacter> Player %s kicks teammate %d but there are %d members", leaderId.toString().c_str(), memberIndex,i+1 );
		return;
	}

	CCharacter * kickedChar = PlayerManager.getOnlineChar( (*it) );
	if ( kickedChar == NULL )
	{
		nlwarning("<CTeamManager kickCharacter> kicked Player %d is invalid", memberIndex );
		return;
	}


	// inform the kicked player
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);	
	params[0].setEIdAIAlias( leaderId, CAIAliasTranslator::getInstance()->getAIAlias( leaderId ) );
	PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(*it), "TEAM_KICKED_YOU", params);
	
	// inform other team members
	params[0].setEIdAIAlias( *it, CAIAliasTranslator::getInstance()->getAIAlias( *it ) );
	set<CEntityId> exclude;
	exclude.insert(*it);
	team->sendDynamicMessageToMembers("TEAM_KICKED", params, exclude);

	//  remove the kicked player from team	
	team->removeCharacter( kickedChar );

	leader->updateTarget();
	/*
	TGroupId groupId = CEntityId(RYZOMID::chatGroup, leader->getTeamId() + 1);
	string msgName = "OPS_KICKED_TEAM_E";
	CMessage msggroup("STATIC_STRING");
	msggroup.serial( groupId );
	std::set<CEntityId> exclude;
	exclude.insert(*it);
	msggroup.serialCont( exclude);
	msggroup.serial( msgName );
	msggroup.serial( (CEntityId&)(*it) );
	sendMessageViaMirror( "IOS", msggroup );
	*/

	CMissionManager::getInstance()->updateEscortTeam( kickedChar->getId() );
	
}// kickCharacter

//---------------------------------------------------
// removeCharacter :
//---------------------------------------------------
void CTeamManager::removeCharacter( const CEntityId &charId )
{
	CCharacter * player = PlayerManager.getOnlineChar( charId );
	if ( player == NULL )
	{
		nlwarning("<CTeamManager removeCharacter> Player %s is invalid", charId.toString().c_str() );
		return;
	}
	player->setAfkState(false);
	CTeam * team = getRealTeam( player->getTeamId() );	
	if ( team == NULL )
	{
		nlwarning("<CTeamManager removeCharacter> Player %s has an invalid team id %d", charId.toString().c_str(), player->getTeamId() );
		return;
	}

	uint16 nbMembers = team->getTeamSize();
	// send message to former member
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	if (nbMembers > 2)
	{
		if (charId != team->getLeader())
		{
			params[0].setEIdAIAlias( team->getLeader(), CAIAliasTranslator::getInstance()->getAIAlias(team->getLeader()) );
			PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(charId), "TEAM_YOU_LEAVE", params);
		}
		else
		{
			const list<CEntityId> &members = team->getTeamMembers();
			
			CEntityId eId(team->getSuccessor());
			params[0].setEIdAIAlias( eId, CAIAliasTranslator::getInstance()->getAIAlias(eId) );
			PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(charId), "TEAM_YOU_LEAVE_LEADER", params);
		}
	}
	else if(nbMembers==2)
	{
		params[0].setEIdAIAlias( team->getLeader(), CAIAliasTranslator::getInstance()->getAIAlias(team->getLeader()) );
		PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(charId), "TEAM_YOU_LEAVE_DISOLVE", params);
	}

	// send message to all other team members
	params[0].setEIdAIAlias( charId, CAIAliasTranslator::getInstance()->getAIAlias(charId) );
	set<CEntityId> exclude;
	exclude.insert( charId );
	team->sendDynamicMessageToMembers("TEAM_LEAVE", params, exclude);

	// remove the player from his team
	team->removeCharacter( player );

	/*
	TGroupId groupId =CHAT_GROUPS_IDS::getTeamChatGroupId(player->getTeamId());
	string msgName = "OPS_LEAVE_TEAM_E";
	CMessage msggroup("STATIC_STRING");
	msggroup.serial( groupId );
	std::set<CEntityId> exclude;
	exclude.insert( charId );
	msggroup.serialCont( exclude);
	msggroup.serial( msgName );
	msggroup.serial( (CEntityId&) charId );
	sendMessageViaMirror( "IOS", msggroup );
	*/
} // removeCharacter //

//---------------------------------------------------
// addFakeTeam
//---------------------------------------------------
void CTeamManager::addFakeTeam(CCharacter * player)
{
	nlassert( player );
	if ( player->getTeamId() == CTEAM::InvalidTeamId )
	{
		//check for reallocation
		if ( _FirstFreeTeamId >= (uint16)_Teams.size() )
		{
			_Teams.resize( _TeamAllocStep + _FirstFreeTeamId );
			for (uint i = _FirstFreeTeamId; i < _Teams.size(); i++)
			{
				_Teams[i].setNextFreeId( i + 1 );
			}
		}	
		_Teams[_FirstFreeTeamId].setAsFake();
		// update the new leader
		player->setTeamId(_FirstFreeTeamId);
		// update alloc data
		_FirstFreeTeamId = _Teams[_FirstFreeTeamId].getNextFreeId();
	}
	else
		nlwarning("<CTeamManager getTeam>char %s has already a team : %d",player->getId().toString().c_str(),player->getTeamId());
}// addFakeTeam

//---------------------------------------------------
// removeFakeTeam
//---------------------------------------------------
void CTeamManager::removeFakeTeam(CCharacter * player)
{
	nlassert( player );
	if ( player->getTeamId() < (sint16)_Teams.size() )
	{
		if ( _Teams[player->getTeamId()].isFake() )
		{
			_Teams[_FirstFreeTeamId].release();
			removeTeam( player->getTeamId() );
			player->setTeamId( CTEAM::InvalidTeamId );
		}
		else
			nlwarning("<CTeamManager getTeam>char %s the team id %d is not fake",player->getId().toString().c_str(),player->getTeamId());
	}
	else
		nlwarning("<CTeamManager removeFakeTeam> char %s the team id %d is out of bounds (nb teams = %d)",player->getId().toString().c_str(),player->getTeamId(),_Teams.size());
}// removeFakeTeam

//---------------------------------------------------
// isLeagueInvitableBy :
//---------------------------------------------------
CTeamManager::TInviteRetCode CTeamManager::isLeagueInvitableBy(CCharacter * invited, CCharacter * invitor )
{
	// check must be done before
	nlassert( invited );
	nlassert( invitor );

	if ( !TheDataset.isAccessible(invited->getEntityRowId()) || !TheDataset.isAccessible(invitor->getEntityRowId()))
		return CantInvite;
	
	// check that the invitor is in team
	CTeam * team = getRealTeam( invitor->getTeamId() );
	if (!team)
		return CantInvite;
	
	// check that the invitor is the leader
	if (team->getLeader() != invitor->getId() )
		return CantInvite;
		
	// check that the invited don't have league
	if (invited->getLeagueId() !=  DYN_CHAT_INVALID_CHAN)
		return AlreadyInLeague;
	  
	// check if target is not already invited
	if( invited->getLeagueInvitor() != CEntityId::Unknown )
		return AlreadyInvited;	

	// get the target team
	team = getRealTeam( invited->getTeamId() );
	if (!team)
		return NotInTeam;

	// check that the invited is the leader
	if (team->getLeader() != invited->getId() )
		return NotLeader;

	return Ok;
}


//---------------------------------------------------
// processMissionTeamEvent :
//---------------------------------------------------
CTeamManager::TInviteRetCode CTeamManager::isInvitableBy(CCharacter * invited, CCharacter * invitor )
{
	// check must be done before
	nlassert( invited );
	nlassert( invitor );

	// check if target is not already invited
	if( invited->getTeamInvitor() != CEntityId::Unknown )
	{
		return AlreadyInvited;
	}

	// get the target team, which must be fake
	CTeam * team = getRealTeam( invited->getTeamId() );
	if( team )
	{
		return AlreadyInTeam;
	}
	
	// check that the invitor is alone or a group leader
	team = getRealTeam( invitor->getTeamId() );
	
	if (team && team->getLeader()!= invitor->getId())
	{
		return CantInvite;
	}

	// check faction of invitor  and invited, player can't invite an enemy in team.
	if ( !TheDataset.isAccessible(invited->getEntityRowId()) || !TheDataset.isAccessible(invitor->getEntityRowId()))
		return CantInvite;

	// cannot invite enemy in faction PvP zones
/*	if( CPVPManager2::getInstance()->isOffensiveActionValid( invited, invitor, true ) )
		if( invited->getPvPRecentActionFlag() || invitor->getPvPRecentActionFlag() )
			return CantInviteEnemy;*/
	return Ok;
}

//---------------------------------------------------
// PvP attack occurs in a team
//---------------------------------------------------
void CTeamManager::pvpAttackOccursInTeam( CCharacter * actor, CCharacter * target )
{
	return;
	nlassert( actor );
	nlassert( target);
	CTeam * team = getRealTeam( actor->getTeamId() );
	if ( team != NULL )
	{
		//check if actor and target is in same team
		if( actor->getTeamId() == target->getTeamId() )
		{
			// check if attack is in PvP faction context
			if( actor->getPVPFlag() && target->getPVPFlag() )
			{
				if( CPVPManager2::getInstance()->factionWarOccurs( actor->getAllegiance(), target->getAllegiance() ) )
				{
					// attack is made in PvP faction context, kick aggressor
			
					// inform the kicked player
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);	
					params[0].setEIdAIAlias( actor->getId(), CAIAliasTranslator::getInstance()->getAIAlias( actor->getId() ) );
					PHRASE_UTILITIES::sendDynamicSystemMessage(actor->getEntityRowId(), "TEAM_PVP_ATTACKER_KICKED_YOU", params);
					
					// inform other team members
					SM_STATIC_PARAMS_2(params2, STRING_MANAGER::player, STRING_MANAGER::player);	
					params2[0].setEIdAIAlias( actor->getId(), CAIAliasTranslator::getInstance()->getAIAlias( actor->getId() ) );
					params2[1].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias( target->getId() ) );
					set<CEntityId> exclude;
					exclude.insert( actor->getId() );
					team->sendDynamicMessageToMembers("TEAM_PVP_ATTACKER_KICKED", params2, exclude);

					//  remove the kicked player from team	
					removeCharacter( actor->getId() );
				}
			}
		}
	}
}

//---------------------------------------------------
// PvP help occurs in a team
//---------------------------------------------------
void CTeamManager::pvpHelpOccursInTeam( CCharacter * actor, CCharacter * target )
{
	return;
	nlassert( actor );
	nlassert( target);
	CTeam * team = getRealTeam( actor->getTeamId() );
	if ( team != NULL )
	{
		//check if actor and target is not in same team
		if( actor->getTeamId() != target->getTeamId() )
		{
			// check if attack is in PvP faction context
			if( actor->getPVPFlag() && target->getPVPFlag() )
			{
				if( CPVPManager2::getInstance()->isFactionInWar( actor->getAllegiance().first ) || CPVPManager2::getInstance()->isFactionInWar( actor->getAllegiance().second ) )
				{
					// help is made in PvP faction context, on an external character of team
			
					// inform the kicked player
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);	
					params[0].setEIdAIAlias( actor->getId(), CAIAliasTranslator::getInstance()->getAIAlias( actor->getId() ) );
					PHRASE_UTILITIES::sendDynamicSystemMessage(actor->getEntityRowId(), "TEAM_PVP_HELPER_KICKED_YOU", params);
					
					// inform other team members
					SM_STATIC_PARAMS_2(params2, STRING_MANAGER::player, STRING_MANAGER::player);	
					params2[0].setEIdAIAlias( actor->getId(), CAIAliasTranslator::getInstance()->getAIAlias( actor->getId() ) );
					params2[1].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias( target->getId() ) );
					set<CEntityId> exclude;
					exclude.insert( actor->getId() );
					team->sendDynamicMessageToMembers("TEAM_PVP_HELPER_KICKED", params2, exclude);

					//  remove the kicked player from team	
					removeCharacter( actor->getId() );
				}
			}
		}
	}
}

//---------------------------------------------------
// update :
//---------------------------------------------------
void CTeamManager::update()
{
	const TGameCycle time = CTickEventHandler::getGameCycle();

	// update team members position every 5s
	const uint size = (uint)_Teams.size();
	for ( uint i = 0 ; i < size ; ++i)
	{
		if ( (time + _Teams[i].getTeamId()) % 30 == 0)
			_Teams[i].updateMembersPositions();
	}
} // update //

//---------------------------------------------------
// removeTeam
//---------------------------------------------------
void CTeamManager::removeTeam( uint16 teamId )
{
	BOMB_IF(teamId==CTEAM::InvalidTeamId,"removeTeam(): Invalid team Id",return);
	BOMB_IF(teamId>=_Teams.size(),"removeTeam(): Invalid team Id",return);
	
	_Teams[teamId].setNextFreeId( _FirstFreeTeamId );
	_FirstFreeTeamId = teamId;
}// removeTeam

//---------------------------------------------------
// getTeam
//---------------------------------------------------
CTeam* CTeamManager::getTeam(uint16 id)
{
	if ( id < (sint16)_Teams.size() && _Teams[id].isValid() )
		return &_Teams[id];
	return NULL;
}

//---------------------------------------------------
// getRealTeam
//---------------------------------------------------
CTeam* CTeamManager::getRealTeam(uint16 id)
{
	CTeam * team = getTeam(id);
	if ( !team || team->isFake())
		return NULL;
	return team;
}// getRealTeam
