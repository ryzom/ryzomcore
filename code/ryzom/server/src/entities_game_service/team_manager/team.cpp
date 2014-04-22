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
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/entity_types.h"
#include "game_share/utils.h"
#include "team_manager/team.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "mission_manager/mission_manager.h"
#include "team_manager/team_manager.h"
#include "progression/progression_pve.h"
#include "progression/progression_pvp.h"
#include "chat_groups_ids.h"
#include "mission_manager/mission_team.h"
#include "entities_game_service.h"
#include "pvp_manager/pvp.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern uint8	TeamMembersStatusMaxValue;
extern CGenericXmlMsgHeaderManager	GenericMsgManager;


//---------------------------------------------------
// CTeam init
//---------------------------------------------------
void CTeam::init( CCharacter*  leader, uint16 teamId )
{
#ifdef NL_DEBUG
	nlassert(leader);
#endif

	// check already done
	_ValidityFlags.Valid = true;
	_ValidityFlags.Fake = false;
	_NbMembers = 1;
	_TeamId  = teamId;
	_LeagueId = leader->getLeagueId();
	_LeaderId = leader->getId();
	_TeamMembers.push_back(_LeaderId);
	// init the team chat group
	TGroupId idGroupe = CHAT_GROUPS_IDS::getTeamChatGroupId(teamId);
	CChatGroup::TGroupType typeGroupe = CChatGroup::team;
	CMessage msgout("ADD_GROUP");
	msgout.serial( idGroupe );
	msgout.serialEnum( typeGroupe );
	sendMessageViaMirror( "IOS", msgout );
	
	// Add leader to chat group
	CMessage msgAdd("ADD_TO_GROUP");
	msgAdd.serial( idGroupe );
	msgAdd.serial( const_cast<CEntityId&> (leader->getId()) );
	sendMessageViaMirror( "IOS", msgAdd );
	
	// inform the new team leader
	PHRASE_UTILITIES::sendDynamicSystemMessage( leader->getEntityRowId(), "TEAM_CREATE");
	//CCharacter::sendMessageToClient(_LeaderId,"OPS_CREATE_TEAM");
	
	// update leader DB
//	leader->_PropertyDatabase.setProp( "USER:TEAM_MEMBER", 1 );
	CBankAccessor_PLR::getUSER().setTEAM_MEMBER(leader->_PropertyDatabase, true);
//	leader->_PropertyDatabase.setProp( "USER:TEAM_LEADER", 1 );
	CBankAccessor_PLR::getUSER().setTEAM_LEADER(leader->_PropertyDatabase, true);
//	leader->_PropertyDatabase.setProp( "GROUP:LEADER_INDEX", -1 );
	CBankAccessor_PLR::getGROUP().setLEADER_INDEX(leader->_PropertyDatabase, checkedCast<uint8>(0xf));
//	leader->_PropertyDatabase.setProp( "GROUP:SUCCESSOR_INDEX", 0 );
	CBankAccessor_PLR::getGROUP().setSUCCESSOR_INDEX(leader->_PropertyDatabase, 0);

	// tell progression system this player has joigned this team
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->playerJoinsTeam(leader->getId(), _TeamId);
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->playerJoinsTeam(leader, _TeamId);

}// CTeam init

//---------------------------------------------------
// CTeam release
//---------------------------------------------------
void CTeam::release()
{
	if ( _RewardSharing )
	{
		if (! _TeamMembers.empty() )
		{
			_RewardSharing->giveAllItems( TheDataset.getDataSetRow( *_TeamMembers.begin()));
		}
		delete _RewardSharing;
		_RewardSharing = NULL;		
	}
	const uint size = (uint)_Missions.size();
	for ( uint i = 0; i < size; i++ )
	{
		_Missions[i]->clearUsersJournalEntry();
		CMissionManager::getInstance()->deInstanciateMission(_Missions[i]);
		delete _Missions[i];
	}
	_Missions.clear();

	// indicate to progression system all members leave teams and team is disolved
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->disbandTeam(_TeamId, _TeamMembers);
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->disbandTeam(_TeamId, _TeamMembers);

	if (_LeagueId != DYN_CHAT_INVALID_CHAN)
	{
		DynChatEGS.removeChan(_LeagueId);
	}

	_LeagueId = DYN_CHAT_INVALID_CHAN;
	_TeamId = CTEAM::InvalidTeamId;
	_TeamMembers.clear();
	_NbMembers = 0;
	_LeaderId = CEntityId::Unknown;
	_ValidityFlags.Fake = false;
	_ValidityFlags.Valid = false;
}

//---------------------------------------------------
// CTeam addAllMembersToChatGroup
//---------------------------------------------------
void CTeam::addAllMembersToChatGroup( TGroupId idGroup )
{
	for ( std::list<NLMISC::CEntityId>::iterator it =  _TeamMembers.begin(); it != _TeamMembers.end(); ++it )
	{
		CMessage msgout("ADD_TO_GROUP");
		msgout.serial( idGroup );
		msgout.serial( *it );
		sendMessageViaMirror( "IOS", msgout );
	}
}// CTeam addAllMembersToChatGroup

//---------------------------------------------------
// CTeam addCharacter
//---------------------------------------------------
void CTeam::addCharacter(CCharacter *newCharacter)
{
	// check already done
	nlassert(newCharacter);

	// init new character DB
//	newCharacter->_PropertyDatabase.setProp( "USER:TEAM_MEMBER", 1 );
	CBankAccessor_PLR::getUSER().setTEAM_MEMBER(newCharacter->_PropertyDatabase, true);
//	newCharacter->_PropertyDatabase.setProp( "USER:TEAM_LEADER", 0 );
	CBankAccessor_PLR::getUSER().setTEAM_LEADER(newCharacter->_PropertyDatabase, false);
//	newCharacter->_PropertyDatabase.setProp( "GROUP:LEADER_INDEX", 0 );
	CBankAccessor_PLR::getGROUP().setLEADER_INDEX(newCharacter->_PropertyDatabase, 0);

	if ( _TeamMembers.size() == 1 )
	{
		_SuccessorId = newCharacter->getId();
//		newCharacter->_PropertyDatabase.setProp( "GROUP:SUCCESSOR_INDEX", -1  );
		CBankAccessor_PLR::getGROUP().setSUCCESSOR_INDEX(newCharacter->_PropertyDatabase, 0xf);
	}
	else
	{
//		newCharacter->_PropertyDatabase.setProp( "GROUP:SUCCESSOR_INDEX", 1  );
		uint8 index = getSuccessorIndex();
		CBankAccessor_PLR::getGROUP().setSUCCESSOR_INDEX(newCharacter->_PropertyDatabase, index);
	}
	
	// update all member's DB
//	char buffer[256];
	uint position = (uint)_TeamMembers.size()-1;
	uint i =0;
	for (std::list<CEntityId>::const_iterator it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
	{
		uint8 hp, sap, stamina;
		uint32 nameId;
		CCharacter* character = PlayerManager.getOnlineChar( (*it) );
		if (character != NULL)
		{
			// update the current character team slot
///\todo: can be done outside the loop
			if ( newCharacter->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max != 0)
				hp = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( newCharacter->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Current ) ) / ( newCharacter->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max ) );
			else
				hp = 0;

			if ( newCharacter->getPhysScores()._PhysicalScores[ SCORES::sap ].Max != 0)
				sap = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( newCharacter->getPhysScores()._PhysicalScores[ SCORES::sap ].Current ) ) / ( newCharacter->getPhysScores()._PhysicalScores[ SCORES::sap ].Max ) );
			else
				sap = 0;

			if ( newCharacter->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max != 0)
				stamina = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( newCharacter->getPhysScores()._PhysicalScores[ SCORES::stamina ].Current ) ) / ( newCharacter->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max ) );
			else
				stamina = 0;

			CMirrorPropValueRO<uint32> nameIndexValue1( TheDataset, newCharacter->getId(), "NameIndex" );
			nameId = nameIndexValue1();

			CBankAccessor_PLR::TGROUP::TArray &arrayItem = CBankAccessor_PLR::getGROUP().getArray(position);
//			sprintf(buffer, "GROUP:%d:HP",position );
//			character->_PropertyDatabase.setProp( buffer, hp );
			arrayItem.setHP(character->_PropertyDatabase, hp);

//			sprintf(buffer, "GROUP:%d:SAP",position );
//			character->_PropertyDatabase.setProp( buffer, sap );
			arrayItem.setSAP(character->_PropertyDatabase, sap);

//			sprintf(buffer, "GROUP:%d:STA",position );
//			character->_PropertyDatabase.setProp( buffer, stamina );
			arrayItem.setSTA(character->_PropertyDatabase, stamina);

//			sprintf(buffer, "GROUP:%d:NAME",position );
//			character->_PropertyDatabase.setProp( buffer, nameId );
			arrayItem.setNAME(character->_PropertyDatabase, nameId);

//			sprintf(buffer, "GROUP:%d:UID",position );
//			character->_PropertyDatabase.setProp( buffer, newCharacter->getEntityRowId().getCompressedIndex() );
			arrayItem.setUID(character->_PropertyDatabase, newCharacter->getEntityRowId().getCompressedIndex());

//			sprintf(buffer, "GROUP:%d:PRESENT",position );
//			character->_PropertyDatabase.setProp( buffer, (uint8)1 );
			arrayItem.setPRESENT(character->_PropertyDatabase, true);

			// update the new character team slot corresponding to character
			if ( character->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max != 0)
				hp = (uint8)  ( ( float(TeamMembersStatusMaxValue) * ( character->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Current ) ) / ( character->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max ) );
			else
				hp = 0;
				
			if ( character->getPhysScores()._PhysicalScores[ SCORES::sap ].Max != 0)
				sap = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( character->getPhysScores()._PhysicalScores[ SCORES::sap ].Current ) ) / ( character->getPhysScores()._PhysicalScores[ SCORES::sap ].Max ) );
			else
				sap = 0;

			if ( character->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max != 0)
				stamina = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( character->getPhysScores()._PhysicalScores[ SCORES::stamina ].Current ) ) / ( character->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max ) );
			else
				stamina = 0;

			CMirrorPropValueRO<uint32> nameIndexValue2( TheDataset, character->getId(), "NameIndex" );
			nameId = nameIndexValue2();

			CBankAccessor_PLR::TGROUP::TArray &newArrayItem = CBankAccessor_PLR::getGROUP().getArray(i);
//			sprintf(buffer, "GROUP:%d:HP",i );
//			newCharacter->_PropertyDatabase.setProp( buffer, hp );
			newArrayItem.setHP(newCharacter->_PropertyDatabase, hp);
				
//			sprintf(buffer, "GROUP:%d:SAP",i );
//			newCharacter->_PropertyDatabase.setProp( buffer, sap );
			newArrayItem.setSAP(newCharacter->_PropertyDatabase, sap);

//			sprintf(buffer, "GROUP:%d:STA",i );
//			newCharacter->_PropertyDatabase.setProp( buffer, stamina );
			newArrayItem.setSTA(newCharacter->_PropertyDatabase, stamina);

//			sprintf(buffer, "GROUP:%d:NAME",i );
//			newCharacter->_PropertyDatabase.setProp( buffer, nameId );
			newArrayItem.setNAME(newCharacter->_PropertyDatabase, nameId);

//			sprintf(buffer, "GROUP:%d:UID",i );
//			newCharacter->_PropertyDatabase.setProp( buffer, character->getEntityRowId().getCompressedIndex() );
			newArrayItem.setUID(newCharacter->_PropertyDatabase, character->getEntityRowId().getCompressedIndex());

//			sprintf(buffer, "GROUP:%d:PRESENT",i );
//			newCharacter->_PropertyDatabase.setProp( buffer, (uint8)1 );
			newArrayItem.setPRESENT(newCharacter->_PropertyDatabase, true);
		}
		else
			nlwarning("<CTeam::addCharacter> Unknown character %s", (*it).toString().c_str() );
		++i;
	}


	// insert new member
	_TeamMembers.push_back( newCharacter->getId() );
	++_NbMembers;
	
	// set the character team
	newCharacter->setTeamId(_TeamId);

	// set the character alliance
	newCharacter->setLeagueId(_LeagueId);

	// Add character to chat group
	TGroupId idGroupe = CHAT_GROUPS_IDS::getTeamChatGroupId(_TeamId);
	CMessage msgout("ADD_TO_GROUP");
	msgout.serial( idGroupe );
	msgout.serial( const_cast<CEntityId&> (newCharacter->getId()) );
	sendMessageViaMirror( "IOS", msgout );
	
	// send messages to members
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);

	// to new member
	//CCharacter::sendMessageToClient(newCharacter->getId(),"OPS_JOIN_TEAM_E",_LeaderId);
	params[0].setEId(  _LeaderId );
	PHRASE_UTILITIES::sendDynamicSystemMessage(newCharacter->getEntityRowId(), "TEAM_YOU_JOIN", params);
	
	params[0].setEIdAIAlias( newCharacter->getId(), CAIAliasTranslator::getInstance()->getAIAlias( newCharacter->getId()) );

	set<CEntityId> exclude;
	exclude.insert( newCharacter->getId() );
	this->sendDynamicMessageToMembers("TEAM_ACCEPT",params,exclude);

	// update the reward share if needed
	if ( _RewardSharing )
	{
		CMessage msgout ("IMPULSION_ID");
		msgout.serial ((CEntityId&)(newCharacter->getId()));
		CBitMemStream bms;
		if (!GenericMsgManager.pushNameToStream ("TEAM:SHARE_OPEN", bms))
			nlstopex (("Missing TEAM:SHARE_OPEN in msg.xml"));
		msgout.serialBufferWithSize ((uint8*)bms.buffer(), bms.length());
		sendMessageViaMirror ( NLNET::TServiceId(newCharacter->getId().getDynamicId()), msgout);
		_RewardSharing->resetCandidates(this);
	}

	// update positions
	updateMembersPositions(true);

	/*TGroupId groupId = TGroupId idGroupe = CHAT_GROUPS_IDS::getTeamChatGroupId(_TeamId);
	string msgName = "OPS_JOIN_TEAM_ACCEPT_E";
	CMessage msggroup("STATIC_STRING");
	msggroup.serial( groupId );
	
	msggroup.serialCont( exclude);
	msggroup.serial( msgName );
	msggroup.serial( const_cast<CEntityId&>(newCharacter->getId()) );
	sendMessageViaMirror( "IOS", msggroup );
*/

	// tell progression system this player has joigned this team
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->playerJoinsTeam(newCharacter->getId(), _TeamId);
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->playerJoinsTeam(newCharacter, _TeamId);

	newCharacter->updateTargetingChars();
}// CTeam addCharacter

//---------------------------------------------------
// CTeam removeCharacter
//---------------------------------------------------
void CTeam::removeCharacter( CCharacter * player )
{
	// check already done
	nlassert(player);

	const CEntityId & charId = player->getId();
	uint16 teamId = player->getTeamId();

	// check if the character is in the team and remove him
	uint formerPos = 0;
	bool found = false;
	for ( std::list<NLMISC::CEntityId>::iterator it = _TeamMembers.begin(); it != _TeamMembers.end(); ++it )
	{
		if ( charId  == *it )
		{
			clearPlayerTeamDB(charId);
			_TeamMembers.erase(it);
			--_NbMembers;
			found = true;
			break;
		}
		formerPos++;
	}
	if ( !found )
	{
		nlwarning("<CTeam removeCharacter> charId %s not found in the team", charId.toString().c_str() );
		return;
	}
	if ( _RewardSharing )
	{
		CMessage msgout ("IMPULSION_ID");
		msgout.serial ((CEntityId&)(player->getId()));
		CBitMemStream bms;
		if (!GenericMsgManager.pushNameToStream ("TEAM:SHARE_CLOSE", bms))
			nlstopex (("Missing TEAM:SHARE_CLOSE in msg.xml"));
		msgout.serialBufferWithSize ((uint8*)bms.buffer(), bms.length());
		sendMessageViaMirror (NLNET::TServiceId(player->getId().getDynamicId()), msgout);
		_RewardSharing->resetCandidates(this);
	}

	///\todo give him a player team Id
	player->setTeamId( CTEAM::InvalidTeamId );
	player->setLeagueId(DYN_CHAT_INVALID_CHAN);
	player->updateTargetingChars();

	// tell progression system this player has been removed from his team
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->playerLeavesTeam(charId, _TeamId);
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->playerLeavesTeam(player, _TeamId);

	if ( player->getPVPInterface().isValid() )
		player->getPVPInterface().leavePVP( IPVP::QuitTeam );

	// check if the team must be removed
	if ( _NbMembers == 1 )
	{
		const uint size = (uint)_Missions.size();
		uint count = 0;
		while ( !_Missions.empty() && count < size )
		{
			_Missions[0]->onFailure(true);
			count++;
		}
		
		// send a message to the last team member
		PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(_TeamMembers.front()), "TEAM_DISOLVED");
		
		// clear the player DB
		clearPlayerTeamDB( *_TeamMembers.begin() );
		CCharacter * lastPlayer = PlayerManager.getOnlineChar( *_TeamMembers.begin() );
		if ( lastPlayer )
		{
			lastPlayer->setTeamId( CTEAM::InvalidTeamId );
			lastPlayer->setLeagueId(DYN_CHAT_INVALID_CHAN);
		}
		else
		{
			nlwarning("<CTeam removeCharacter> charId %s not found in the team", (*_TeamMembers.begin()).toString().c_str() );
			return;
		}

		// remove league
		setLeague("");

		// remove the team chat group
		TGroupId idGroupe = CHAT_GROUPS_IDS::getTeamChatGroupId(_TeamId);
		CMessage msgRemoveGroup("REMOVE_GROUP");
		msgRemoveGroup.serial( idGroupe );
		sendMessageViaMirror( "IOS", msgRemoveGroup );
	
		// release the team
		release();
		//remove the team from the manager
		TeamManager.removeTeam(teamId);

		// let the last player continue his escort mission (with a fake team) if he has one
		CMissionManager::getInstance()->updateEscortTeam( lastPlayer->getId() );

		return;
	}

	// if that was the leader, get another one
	else if ( _LeaderId == charId )
	{
		setLeader(_SuccessorId);
	}
	else if ( _SuccessorId == charId )
	{
		// The current successor dropped from team; set it to the next in line after leader
		setSuccessor(1);
	}
	else
	{
		// A random member dropped from team; make sure the current successor is shown currectly
		// in all team lists.
		uint8 index = 0;
		for (std::list<NLMISC::CEntityId>::iterator it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
		{
			if (_SuccessorId == (*it))
				break;
			++index;
		}
		// Don't show a message because the successor hasn't actually changed.
		setSuccessor(index, false);
	}

	for (std::list<NLMISC::CEntityId>::iterator it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
	{
		uint8 hp, sap, stamina;
		uint32 nameId;
		uint pos = 0;
//		char buffer[256];

		CCharacter * ch1 = PlayerManager.getOnlineChar( (*it) );
		///\todo log if nothing
		if (ch1)
		{
			for (std::list<NLMISC::CEntityId>::iterator it2 = _TeamMembers.begin() ; it2 != _TeamMembers.end() ; ++it2)
			{
				if ( (*it) == (*it2) )
					continue;	

				CBankAccessor_PLR::TGROUP::TArray &groupItem = CBankAccessor_PLR::getGROUP().getArray(pos);

				CCharacter * ch2 = PlayerManager.getOnlineChar( (*it2) );	
				if (ch2 != NULL)
				{
					// update new char for old char
					if ( ch2->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max != 0)
						hp = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( ch2->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Current ) ) / ( ch2->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max ) );
					else
						hp = 0;
					if ( ch2->getPhysScores()._PhysicalScores[ SCORES::sap ].Max != 0)
						sap = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( ch2->getPhysScores()._PhysicalScores[ SCORES::sap ].Current ) ) / ( ch2->getPhysScores()._PhysicalScores[ SCORES::sap ].Max ) );
					else
						sap = 0;
					if ( ch2->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max != 0)
						stamina = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( ch2->getPhysScores()._PhysicalScores[ SCORES::stamina ].Current ) ) / ( ch2->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max ) );
					else
						stamina = 0;
					
					CMirrorPropValueRO<uint32> nameIndexValue( TheDataset, ch2->getId(), "NameIndex" );
					nameId = nameIndexValue();
									
//					sprintf(buffer, "GROUP:%d:HP",pos );
//					ch1->_PropertyDatabase.setProp( buffer, hp );
					groupItem.setHP(ch1->_PropertyDatabase, hp);
					
//					sprintf(buffer, "GROUP:%d:SAP",pos );
//					ch1->_PropertyDatabase.setProp( buffer, sap );
					groupItem.setSAP(ch1->_PropertyDatabase, sap);
					
//					sprintf(buffer, "GROUP:%d:STA",pos );
//					ch1->_PropertyDatabase.setProp( buffer, stamina );
					groupItem.setSTA(ch1->_PropertyDatabase, stamina);
					
//					sprintf(buffer, "GROUP:%d:NAME",pos );
//					ch1->_PropertyDatabase.setProp( buffer, nameId );
					groupItem.setNAME(ch1->_PropertyDatabase, nameId);

//					sprintf(buffer, "GROUP:%d:UID",pos );
//					ch1->_PropertyDatabase.setProp( buffer, ch2->getEntityRowId().getCompressedIndex() );
					groupItem.setUID(ch1->_PropertyDatabase, ch2->getEntityRowId().getCompressedIndex());
					
//					sprintf(buffer, "GROUP:%d:PRESENT",pos );
//					ch1->_PropertyDatabase.setProp( buffer, (uint8)1 );
					groupItem.setPRESENT(ch1->_PropertyDatabase, true);
				}
				pos++;
			}

			CBankAccessor_PLR::TGROUP::TArray &groupItem = CBankAccessor_PLR::getGROUP().getArray(pos);

//			sprintf(buffer, "GROUP:%d:PRESENT",pos );
//			ch1->_PropertyDatabase.setProp( buffer, (uint8)0 );
			groupItem.setHP(ch1->_PropertyDatabase, 0);
//			sprintf(buffer, "GROUP:%d:NAME",pos );
//			ch1->_PropertyDatabase.setProp( buffer, (uint32)0 );
			groupItem.setNAME(ch1->_PropertyDatabase, 0);
//			sprintf(buffer, "GROUP:%d:UID",pos );
//			ch1->_PropertyDatabase.setProp( buffer, (uint32)CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
			groupItem.setUID(ch1->_PropertyDatabase, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX);
		}
	}

	if ( _RewardSharing )
		_RewardSharing->resetCandidates(this);

	// update positions
	updateMembersPositions();

	// remove character from chat group
	TGroupId idGroupe = CHAT_GROUPS_IDS::getTeamChatGroupId(_TeamId);
	CMessage msgRemoveGroup("REMOVE_FROM_GROUP");
	msgRemoveGroup.serial( idGroupe );
	msgRemoveGroup.serial( const_cast<CEntityId&> ( charId ) );
	sendMessageViaMirror( "IOS", msgRemoveGroup );

}// CTeam removeCharacter

void CTeam::setLeague(const string &leagueName)
{
	TChanID chanId = _LeagueId;
	if (_LeagueId != DYN_CHAT_INVALID_CHAN)
	{
		// Remove players from previous channel
		_LeagueId = DYN_CHAT_INVALID_CHAN;
		updateLeague();
		// Remove channel if empty
		vector<CEntityId> players;
		if (!DynChatEGS.getPlayersInChan(chanId, players))
			DynChatEGS.removeChan(chanId);
	}

	if (!leagueName.empty())
	{
		_LeagueId = DynChatEGS.addChan("league_"+toString(DynChatEGS.getNextChanID()), leagueName);
		if (_LeagueId == DYN_CHAT_INVALID_CHAN)
		{
			nlinfo("Error channel creation !!!");
			return;
		}
		// set historic size of the newly created channel
		DynChatEGS.setHistoricSize(_LeagueId, 100);
	}
	
	updateLeague();
}

void CTeam::updateLeague()
{
	for (list<CEntityId>::iterator it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
	{
		CCharacter * ch = PlayerManager.getOnlineChar((*it));
		if (ch != NULL && ch->getLeagueId() != _LeagueId)
		{
			ch->setLeagueId(_LeagueId);
		}
	}
}

uint8 CTeam::getSuccessorIndex()
{
	list<CEntityId>::const_iterator it = _TeamMembers.begin();
	uint8 i = 0;
	for (; it != _TeamMembers.end(); ++it)
	{
		if ( (*it) == _SuccessorId )
			break;
		++i;
	}
	return i;
}

void CTeam::setLeader(CEntityId id, bool bMessage)
{
	_LeaderId = id;

	// Move new leader to top of list
	_TeamMembers.remove(_LeaderId);
	_TeamMembers.insert(_TeamMembers.begin(), _LeaderId);

	// inform the new leader
	SM_STATIC_PARAMS_1(params1, STRING_MANAGER::player);		
	params1[0].setEId( id );
	PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(_LeaderId), "TEAM_YOU_NEW_LEADER", params1);

	// inform the group 
	SM_STATIC_PARAMS_2(params, STRING_MANAGER::player, STRING_MANAGER::player);		
	params[0].setEId( id );
	params[1].setEId( _LeaderId );

	set<CEntityId> exclude;
	exclude.insert( _LeaderId );
	exclude.insert( id );
	sendDynamicMessageToMembers("TEAM_NEW_LEADER", params, exclude);

	// New leader was successor, choose a new successor
	if (id == _SuccessorId)
	{
		_SuccessorId = CEntityId::Unknown;
		// Set the new successor to next in line
		setSuccessor(1);
	}

	// update leader DB
	CCharacter *leader = PlayerManager.getOnlineChar( _LeaderId );
	if (leader)
	{
		// switch dyn chat speaker
		CMissionManager::getInstance()->switchDynChatSpeaker(leader, _LeaderId);

		CBankAccessor_PLR::getGROUP().setLEADER_INDEX(leader->_PropertyDatabase, 0xf);
	}
	else
		nlwarning("<CTeam setLeader> invalid new leader %s", _LeaderId.toString().c_str() );
}

void CTeam::setLeader(uint8 memberIdx, bool bMessage)
{
	list<CEntityId>::const_iterator it = _TeamMembers.begin();
	uint8 i = 0;
	for (; it != _TeamMembers.end(); ++it)
	{
		if ( i == memberIdx )
			break;
		++i;
	}
	if ( it == _TeamMembers.end() )
	{
		nlwarning("invalid team member %u : count is %u", memberIdx,_TeamMembers.size() );
		return;
	}
	CEntityId newLeaderId = (*it);
	setLeader(newLeaderId, bMessage);
}

void CTeam::setSuccessor( uint8 memberIdx, bool bMessage)
{
	list<CEntityId>::const_iterator it = _TeamMembers.begin();
	uint8 i = 0;
	for (; it != _TeamMembers.end(); ++it)
	{
		if ( i == memberIdx )
			break;
		++i;
	}
	if ( it == _TeamMembers.end() )
	{
		nlwarning("invalid team member %u : count is %u", memberIdx,_TeamMembers.size() );
		return;
	}

	_SuccessorId = (*it);
	i = 0;
	for (it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
	{
		CCharacter * ch = PlayerManager.getOnlineChar( (*it) );
		if (ch)
		{
			// If this char is the successor, don't show
			if (i == memberIdx)
			{
				CBankAccessor_PLR::getGROUP().setSUCCESSOR_INDEX(ch->_PropertyDatabase, 0xf);
			}
			else if (i < memberIdx)
			{
				// If the successor comes after this player in the list,
				// bump it up one.
				CBankAccessor_PLR::getGROUP().setSUCCESSOR_INDEX(ch->_PropertyDatabase, memberIdx - 1);
			}
			else
			{
				CBankAccessor_PLR::getGROUP().setSUCCESSOR_INDEX(ch->_PropertyDatabase, memberIdx);
			}
		}
		++i;
	}

	if (bMessage)
	{
		SM_STATIC_PARAMS_1( params,STRING_MANAGER::player );
		params[0].setEIdAIAlias(_SuccessorId, CAIAliasTranslator::getInstance()->getAIAlias(_SuccessorId));
		sendDynamicMessageToMembers("TEAM_NEW_SUCCESSOR", params);
	}
}

//---------------------------------------------------
// CTeam findCharacterPosition
//---------------------------------------------------
sint16 CTeam::findCharacterPosition( const NLMISC::CEntityId &charId ) const
{
	
	list<CEntityId>::const_iterator it = _TeamMembers.begin();
	sint16 i = 0;
	for (; it != _TeamMembers.end(); ++it)
	{
		if ( (*it) == charId )
		{
			return i;
		}
		++i;
	}
	// not in team
	return -1;
} // findCharacterPosition //

//---------------------------------------------------
// CTeam updateCharacterScore
//---------------------------------------------------
//void CTeam::updateCharacterScore(const CCharacter *player, const string &scoreStr, uint8 value) const
void CTeam::updateCharacterScore(const CCharacter *player, SCORES::TScores score, uint8 value) const
{
	if (!player) return;

	sint16 position = findCharacterPosition(player->getId());
	if (position != -1)
	{
		CCharacter *character = NULL;
//		char buffer[256];
		const sint16 charPosition = position;
		sint16 i = 0;
		for (list<CEntityId>::const_iterator it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
		{
			if ( (*it) == player->getId())
				continue;
			
			if ( charPosition > i )
				position = charPosition - 1 ;
			else
				position = charPosition;
							
			character = PlayerManager.getOnlineChar( (*it) );
			if (character != NULL)
			{
//				sprintf(buffer, "GROUP:%d:%s",position ,scoreStr.c_str() );
//				character->_PropertyDatabase.setProp( buffer, value );
				switch(score)
				{
				case SCORES::hit_points:
					CBankAccessor_PLR::getGROUP().getArray(position).setHP(character->_PropertyDatabase, value);
					break;
				case SCORES::stamina :
					CBankAccessor_PLR::getGROUP().getArray(position).setSTA(character->_PropertyDatabase, value);
					break;
				case SCORES::sap :
					CBankAccessor_PLR::getGROUP().getArray(position).setSAP(character->_PropertyDatabase, value);
					break;
				default:
					STOP("Invalid score "<<SCORES::toString(score));
				}
			}
			else
			{
				nlwarning("<CTeam::updateCharacterScore> Unknown character %s", player->getId().toString().c_str() );
			}
			
			++i;
		}		
	}
} // updateCharacterScore //


//---------------------------------------------------
// CTeam clearPlayerTeamVar
//---------------------------------------------------
void CTeam::clearPlayerTeamDB( const NLMISC::CEntityId &charId )
{
	// clear the team interface of the removed player
	CCharacter	*character = PlayerManager.getOnlineChar( charId );
	if (character)
	{
//		char buffer[256];

//		character->_PropertyDatabase.setProp( "USER:TEAM_MEMBER", 0 );
		CBankAccessor_PLR::getUSER().setTEAM_MEMBER(character->_PropertyDatabase, 0);
//		character->_PropertyDatabase.setProp( "USER:TEAM_LEADER", 0 );
		CBankAccessor_PLR::getUSER().setTEAM_LEADER(character->_PropertyDatabase, 0);
		
		for (uint8 i = 0; i < CTEAM::TeamMaxNbMembers - (uint8)1 ; ++i)
		{			
			CBankAccessor_PLR::TGROUP::TArray &groupItem = CBankAccessor_PLR::getGROUP().getArray(i);
//			sprintf(buffer, "GROUP:%d:PRESENT",i );
//			character->_PropertyDatabase.setProp( buffer, (uint8)0 );
			groupItem.setPRESENT(character->_PropertyDatabase, false);
//			sprintf(buffer, "GROUP:%d:NAME",i );
//			character->_PropertyDatabase.setProp( buffer, (uint32)0 );
			groupItem.setNAME(character->_PropertyDatabase, 0);
//			sprintf(buffer, "GROUP:%d:UID",i );
//			character->_PropertyDatabase.setProp( buffer, (uint32)CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
			groupItem.setUID(character->_PropertyDatabase, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX);
		}
	}
}// CTeam clearPlayerTeamVar

//---------------------------------------------------
// CTeam sendDynamicMessageToMembers
//---------------------------------------------------
void CTeam::sendDynamicMessageToMembers(const string &msgName, const TVectorParamCheck &params, const set<CEntityId> &excluded) const
{
	std::list<NLMISC::CEntityId>::const_iterator itTeam = _TeamMembers.begin();
	for ( ; itTeam != _TeamMembers.end() ; ++itTeam )
	{
		if ( excluded.find(*itTeam) == excluded.end())
		{
			const uint32 stringId = STRING_MANAGER::sendStringToClient(TheDataset.getDataSetRow(*itTeam), msgName, params );
			PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(*itTeam), stringId);
		}
	}
} // sendDynamicMessageToMembers //

void CTeam::addMission( CMissionTeam * mission )
{
	_Missions.push_back( mission );
	mission->updateUsersJournalEntry();
}// CTeam::addMission


/// remove a mission to team
void CTeam::removeMission( uint idx, TMissionResult result)
{
	if ( idx >= _Missions.size() )
		return;

	/// if the mission was finished, the result is success
	if ( _Missions[idx]->getFinished() )
	{
		if ( _Missions[idx]->getMissionSuccess() )
			result = mr_success;
		else
			result = mr_fail;
	}

	CMissionTemplate *tpl = CMissionManager::getInstance()->getTemplate(_Missions[idx]->getTemplateId());

	if ( tpl && !tpl->Tags.NoList )
	{
		_Missions[idx]->clearUsersJournalEntry();
	}

	/*Bsi.append( StatPath, NLMISC::toString("[MIT%s] %u %s",
		MissionResultStatLogTag[result],
		this->_TeamId,
		tpl->getMissionName().c_str()) );*/

	/*
	EgsStat.displayNL("[MIT%s] %u %s",
		MissionResultStatLogTag[result],
		this->_TeamId,
		tpl->getMissionName().c_str());
	*/
//	EGSPD::missionTeamLog(MissionResultStatLogTag[result], this->_TeamId, tpl->getMissionName());
	CMissionManager::getInstance()->deInstanciateMission(_Missions[idx]);
	delete _Missions[idx];
	_Missions.erase(_Missions.begin() + idx) ;
}

bool CTeam::processTeamMissionEvent(std::list< CMissionEvent * > & eventList, TAIAlias missionAlias )
{
	for (uint i = 0; i < _Missions.size(); i++ )
	{
		nlassert( _Missions[i] );
		if ( missionAlias == CAIAliasTranslator::Invalid	|| _Missions[i]->getTemplateId() == missionAlias )
		{
			if ( processTeamMissionStepEvent( eventList, _Missions[i]->getTemplateId() ,0xFFFFFFFF) )
				return true;
		}
	}
	return false;
}// CTeam::processTeamMissionEvent

bool CTeam::processTeamMissionStepEvent(std::list< CMissionEvent* > & eventList, TAIAlias missionAlias, uint32 stepIndex )
{
	CMissionTeam * mission = getMissionByAlias( missionAlias );
	if (!mission )
	{
		nlwarning("invalid missionAlias");
		return false;
	}
	CMissionEvent::TResult result = mission->processEvent( TheDataset.getDataSetRow( _LeaderId) ,eventList,stepIndex );
	if ( result == CMissionEvent::Nothing )
		return false;
	else if ( result == CMissionEvent::MissionFailed )
		return true;
	
	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
	nlassert( templ );
	if ( result == CMissionEvent::MissionEnds )
	{
		CMissionEventMissionDone * event = new CMissionEventMissionDone(templ->Alias);
		eventList.push_back(event);
		for (std::list<CEntityId>::const_iterator it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
		{
			CCharacter* user = PlayerManager.getOnlineChar( *it );
			if (user != NULL)
			{
				user->addSuccessfulMissions( *templ );
				if ( templ->Tags.NoList == false )
					CCharacter::sendDynamicSystemMessage( user->getEntityRowId(),"EGS_MISSION_SUCCESS");
			}
		}

		CMissionManager::getInstance()->missionDoneOnce(templ);
		mission->stopChildren();
		
		// only remove no list missions, other must be manually removed by user
		if ( templ->Tags.NoList || mission->isChained() || templ->Tags.AutoRemove )
		{
			mission->updateEncyclopedia();
			removeMission(mission, mr_success);
		}
		else
		{
			mission->setSuccessFlag();
			mission->updateUsersJournalEntry();
		}
		return true;
	}
	else if ( result == CMissionEvent::StepEnds )
	{
		if ( templ->Tags.NoList == false )
		{
			for (std::list<CEntityId>::const_iterator it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
			{
				CCharacter::sendDynamicSystemMessage( *it,"EGS_MISSION_STEP_SUCCESS");
			}
		}
	}
	mission->updateUsersJournalEntry();
	return true;
}// CTeam::processTeamMissionStepEvent


void CTeam::rewardSharing(CRewardSharing * reward)
{
	// if there is a reward add the new it
	if ( _RewardSharing )
	{
		_RewardSharing->addReward(reward,this);
	}
	// otherwise, create a new one
	else
	{
		_RewardSharing = reward;
		for (list<CEntityId>::const_iterator it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
		{
			CCharacter * user = PlayerManager.getOnlineChar( (*it) );
			if (user )
			{
				CMessage msgout ("IMPULSION_ID");
				msgout.serial ((CEntityId&)(*it));
				CBitMemStream bms;
				
				if (!GenericMsgManager.pushNameToStream ("TEAM:SHARE_OPEN", bms))
					nlstopex (("Missing TEAM:SHARE_OPEN in msg.xml"));
				msgout.serialBufferWithSize ((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror ( NLNET::TServiceId((*it).getDynamicId()), msgout);
			}
		}
		_RewardSharing->resetCandidates(this);
	}
}// CTeam::rewardSharing


struct TUpdatedMemberPosition
{
	sint64 Pos;
	
	TUpdatedMemberPosition(sint32 x, sint32 y) { Pos = (sint64(x)<<32) + y; }
};

//---------------------------------------------------
// CTeam updateMembersPositions
//---------------------------------------------------
void CTeam::updateMembersPositions(bool forceUpdate)
{
	CCharacter *character = NULL;
//	char buffer[256];

	const list<CEntityId>::const_iterator itEnd = _TeamMembers.end();
	sint16 i=0;

	typedef map< CEntityId, TUpdatedMemberPosition > TUpdatedMemberPositionMap;
	TUpdatedMemberPositionMap UpdatedMemberPos;

	for (list<CEntityId>::const_iterator it = _TeamMembers.begin() ; it != itEnd ; ++it)
	{
		character = PlayerManager.getOnlineChar( (*it) );
		if (character == NULL)
		{
			nlwarning("<CTeam::updateCharacterScore> Unknown character %s", (*it).toString().c_str() );
			continue;
		}
		
		const sint32 lastX = (sint32)(character->getLastPosXInDB() * 0.001f);
		const sint32 lastY = (sint32)(character->getLastPosYInDB() * 0.001f);
		
		const sint32 posX = (sint32)(character->getX() * 0.001f);
		const sint32 posY = (sint32)(character->getY() * 0.001f);
		
		const sint32 member1Delta = (sint32)CVector2d(posX - lastX, posY - lastY).sqrnorm();
		
		sint16 j = i;
		list<CEntityId>::const_iterator it2 = it;
		++it2;
		for ( ; it2 != itEnd ; ++it2)
		{
			CCharacter *member2 = PlayerManager.getOnlineChar( (*it2) );
			if (member2 == NULL)
			{
				nlwarning("<CTeam::updateCharacterScore> Unknown character %s", (*it2).toString().c_str() );
				continue;
			}
			
			const sint32 lastXMember2 = (sint32)(member2->getLastPosXInDB() * 0.001f);
			const sint32 lastYMember2 = (sint32)(member2->getLastPosYInDB() * 0.001f);
			
			const sint32 posXMember2 = (sint32)(member2->getX() * 0.001f);
			const sint32 posYMember2 = (sint32)(member2->getY() * 0.001f);
			
			// get distance between the two players
			const sint32 dist = (sint32)CVector2d(posXMember2 - posX, posYMember2 - posY).sqrnorm();
			// distance between new and old pos for player 2
			const sint32 member2Delta = (sint32)CVector2d(posXMember2 - lastXMember2, posYMember2 - lastYMember2).sqrnorm();
			
			// update first player DB if member2Delta is important enough
			if ( ( ( member2Delta<<5 > dist || member2Delta > (80*80) ) && dist > 1 ) || forceUpdate) 
			{
				UpdatedMemberPos.insert( make_pair( member2->getId(), TUpdatedMemberPosition(member2->getX(), member2->getY())));
				member2->setLastPosXInDB(member2->getX());
				member2->setLastPosYInDB(member2->getY());
			}
			
			// update member2  DB is member1Delta is important enough
			if ( ( ( member1Delta<<5 > dist || member1Delta > (80*80) ) && dist > 1 ) || forceUpdate )
			{
				UpdatedMemberPos.insert( make_pair( character->getId(), TUpdatedMemberPosition(character->getX(), character->getY())));
				character->setLastPosXInDB(character->getX());
				character->setLastPosYInDB(character->getY());
			}
			++j;
		}
		++i;
	}		

	i=0;
	for (list<CEntityId>::const_iterator it = _TeamMembers.begin() ; it != itEnd ; ++it)
	{
		character = PlayerManager.getOnlineChar( (*it) );
		if (character == NULL)
		{
			nlwarning("<CTeam::updateCharacterScore> Unknown character %s", (*it).toString().c_str() );
			continue;
		}

		sint16 j = i;
		list<CEntityId>::const_iterator it2 = it;
		++it2;
		for ( ; it2 != itEnd ; ++it2)
		{
			CCharacter *member2 = PlayerManager.getOnlineChar( (*it2) );
			if (member2 == NULL)
			{
				nlwarning("<CTeam::updateCharacterScore> Unknown character %s", (*it2).toString().c_str() );
				continue;
			}

			TUpdatedMemberPositionMap::iterator it;
			it = UpdatedMemberPos.find( member2->getId() );
			if( it != UpdatedMemberPos.end() )
			{
//				sprintf(buffer, "GROUP:%d:POS", j );
//				character->_PropertyDatabase.setProp( buffer, (*it).second.Pos );
				CBankAccessor_PLR::getGROUP().getArray(j).setPOS(character->_PropertyDatabase, (*it).second.Pos);
			}

			it = UpdatedMemberPos.find( character->getId() );
			if( it != UpdatedMemberPos.end() )
			{
//				sprintf(buffer, "GROUP:%d:POS", i );
//				member2->_PropertyDatabase.setProp( buffer, (*it).second.Pos );
				CBankAccessor_PLR::getGROUP().getArray(i).setPOS(member2->_PropertyDatabase, (*it).second.Pos);
			}
			++j;
		}
		++i;
	}		
} // updateCharacterScore //

CMissionTeam* CTeam::getMissionByAlias( TAIAlias missionAlias )
{
	const uint size = (uint)_Missions.size();
	for ( uint i = 0; i < size; i++ )
	{
		if ( _Missions[i] && _Missions[i]->getTemplateId() == missionAlias )
			return _Missions[i];
	}
	return NULL;
}

void CTeam::updateMembersDb()
{
	for (std::list<NLMISC::CEntityId>::iterator it = _TeamMembers.begin() ; it != _TeamMembers.end() ; ++it)
	{
		uint8 hp, sap, stamina;
		uint32 nameId;
		uint pos = 0;

		CCharacter * ch1 = PlayerManager.getOnlineChar( (*it) );

		if (ch1->getId() == _LeaderId)
		{
			CBankAccessor_PLR::getGROUP().setLEADER_INDEX(ch1->_PropertyDatabase, 0xf);
		}
		else 
		{
			CBankAccessor_PLR::getGROUP().setLEADER_INDEX(ch1->_PropertyDatabase, 0);
		}
		///\todo log if nothing
		if (ch1)
		{
			for (std::list<NLMISC::CEntityId>::iterator it2 = _TeamMembers.begin() ; it2 != _TeamMembers.end() ; ++it2)
			{
				if ( (*it) == (*it2) )
					continue;	

				CBankAccessor_PLR::TGROUP::TArray &groupItem = CBankAccessor_PLR::getGROUP().getArray(pos);

				CCharacter * ch2 = PlayerManager.getOnlineChar( (*it2) );	
				if (ch2 != NULL)
				{
					// update new char for old char
					if ( ch2->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max != 0)
						hp = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( ch2->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Current ) ) / ( ch2->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max ) );
					else
						hp = 0;
					if ( ch2->getPhysScores()._PhysicalScores[ SCORES::sap ].Max != 0)
						sap = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( ch2->getPhysScores()._PhysicalScores[ SCORES::sap ].Current ) ) / ( ch2->getPhysScores()._PhysicalScores[ SCORES::sap ].Max ) );
					else
						sap = 0;
					if ( ch2->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max != 0)
						stamina = (uint8) ( ( float(TeamMembersStatusMaxValue) * ( ch2->getPhysScores()._PhysicalScores[ SCORES::stamina ].Current ) ) / ( ch2->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max ) );
					else
						stamina = 0;
					
					CMirrorPropValueRO<uint32> nameIndexValue( TheDataset, ch2->getId(), "NameIndex" );
					nameId = nameIndexValue();
									
					groupItem.setHP(ch1->_PropertyDatabase, hp);
					groupItem.setSAP(ch1->_PropertyDatabase, sap);
					groupItem.setSTA(ch1->_PropertyDatabase, stamina);
					groupItem.setNAME(ch1->_PropertyDatabase, nameId);
					groupItem.setUID(ch1->_PropertyDatabase, ch2->getEntityRowId().getCompressedIndex());
					groupItem.setPRESENT(ch1->_PropertyDatabase, true);
				}
				pos++;
			}
		}
	}
}


