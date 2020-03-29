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
#include "team_manager/reward_sharing.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "team_manager/team.h"

extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern NLMISC::CRandom RandomGenerator;

using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CRewardSharing);

void CRewardSharing::addReward( CRewardSharing * reward, CTeam * team )
{
	_Rewards.resize( _Rewards.size() + reward->_Rewards.size() );
	std::copy(reward->_Rewards.begin(),reward->_Rewards.end(),_Rewards.begin() +  reward->_Rewards.size() );
	resetCandidates(team);
}// CRewardSharing::addReward

void CRewardSharing::resetCandidates(CTeam * team)
{
	// clear all info on the users, and rebuild the user list from the team member list
	_Candidates.clear();
	_Candidates.resize(team->getTeamMembers().size());
	list<CEntityId>::const_iterator it = team->getTeamMembers().begin();
	for ( uint i  = 0; i < _Candidates.size(); i++)
	{
		_Candidates[i].UserRow = TheDataset.getDataSetRow(*it);
		CCharacter * user = PlayerManager.getChar(  _Candidates[i].UserRow );
		if ( user )
		{
			for ( uint j = 0; j < 16; j++ )
			{
//				string entry =  NLMISC::toString( "INVENTORY:SHARE:%u:INFO_VERSION",j  );
//				user->_PropertyDatabase.setProp( entry, user->_PropertyDatabase.getProp( entry ) +1 );
				CBankAccessor_PLR::getINVENTORY().getSHARE().getArray(j).setINFO_VERSION(user->_PropertyDatabase, CBankAccessor_PLR::getINVENTORY().getSHARE().getArray(j).getINFO_VERSION(user->_PropertyDatabase)+1);
			}
		}
		++it;
	}
	// clear all the candidates for a specific item
	for ( uint i = 0; i < _Rewards.size(); i++ )
	{
		_Rewards[i].Candidates.clear();
	}
	// increment session and fill databases
	_Session++;
	setUsersDb();
}// CRewardSharing::resetCandidates

void CRewardSharing::userItemSelect(const TDataSetRow & userRow,uint32 itemPos,uint8 state)
{
	if ( itemPos >= _Rewards.size() )
	{
		nlwarning("<CRewardSharing userItemSelect> user %u: item Pos %u is invalid (%u items)",userRow.getIndex(),itemPos,_Rewards.size());
		return;
	}
	if ( state != _Session )
		return;
	_Session++;

	// force all team members who validated to invalidate their interface
	// benifit of this loop to find the position of the user in the candidate vector
	uint currentChar = 0;
	for (uint i = 0; i < _Candidates.size();i++)
	{
		if ( _Candidates[i].UserRow == userRow )
			currentChar = i;

		if ( _Candidates[i].Validated )
		{
			CEntityId id = getEntityIdFromRow(_Candidates[i].UserRow);
			CMessage msgout ("IMPULSION_ID");
			msgout.serial (id);
			CBitMemStream bms;
			if (!GenericMsgManager.pushNameToStream ("TEAM:SHARE_INVALID", bms))
				nlstopex (("Missing TEAM:SHARE_INVALID in msg.xml"));
			msgout.serialBufferWithSize ((uint8*)bms.buffer(), bms.length());
			sendMessageViaMirror ( NLNET::TServiceId(id.getDynamicId()), msgout);
			 _Candidates[i].Validated = false;
		}
	}	

	// try to find the player in the candidate list of the specified item
	bool add = true;
	for ( uint i = 0; i < _Rewards[itemPos].Candidates.size(); i++)
	{
		if ( currentChar ==  _Rewards[itemPos].Candidates[i].first )
		{
			// we found the player, that means that he already selected the item, so it is a deslection -> remove and update him
			_Rewards[itemPos].Candidates[i] = _Rewards[itemPos].Candidates.back();
			_Rewards[itemPos].Candidates.pop_back();
			add = false;
			_Candidates[currentChar].NbSelected--;
			break;
		}
	}
	if ( add )
	{
		// let's add the player to the candidate list
		_Rewards[itemPos].Candidates.push_back( make_pair(currentChar,0.0f) );
		_Candidates[currentChar].NbSelected++;
	}

	// we now have to update the rewards
	for ( uint i = 0; i < _Rewards.size(); i++ )
	{
		// compute the users chances
		float total = 0.0f;
		for ( uint j  =0; j < _Rewards[i].Candidates.size(); j++ )
		{
			_Rewards[i].Candidates[j].second = 1.0f / ( _Candidates[ _Rewards[i].Candidates[j].first ].NbSelected );
			total+= _Rewards[i].Candidates[j].second;
		}
		// scale the value on 100
		for ( uint j  =0; j < _Rewards[i].Candidates.size(); j++ )
		{
			_Rewards[i].Candidates[j].second*= (100.0f/total);
		}
	}
	setUsersDb();
}// CRewardSharing::userItemSelect

bool CRewardSharing::userValidSelect(const TDataSetRow & userRow, uint8 state)
{
	if ( state != _Session )
		return false;

	// get the position of the candidate
	uint pos = 0;
	for ( uint i = 0; i < _Candidates.size(); ++i )
	{
		if ( _Candidates[i].UserRow == userRow )
			break;
		pos++;
	}
	if ( pos > _Candidates.size())
	{
		nlwarning("<CRewardSharing userValidSelect> Invalid validation position %u. size = %u",pos,_Candidates.size());
		return false;
	}
	
	if ( _Candidates[pos].Validated )
		return false;
	
	_Session++;
	_Candidates[pos].Validated = true;

	
	// check if everybody validated
	bool validated = true;
	for ( uint i = 0; i < _Candidates.size(); i++ )
	{
		if ( _Candidates[i].Validated == 0)
		{
			validated = false;
			break;
		}
	}

	if ( validated )
	{
		// give the rewards
		for ( uint i = 0; i < _Rewards.size();i++ )
		{
			if ( !_Rewards[i].Candidates.empty() )
			{
				bool given = false;
				while(!given)
				{
					float rand = RandomGenerator.frand(100.0f);
					float currentRange = 0;
					for ( uint j = 0; j < _Rewards[i].Candidates.size();j++ )
					{
						currentRange += _Rewards[i].Candidates[j].second;
						if ( currentRange >= rand )
						{
							CCharacter * user = PlayerManager.getChar( _Candidates[ _Rewards[i].Candidates[j].first ].UserRow );
							if ( !user )
							{
								nlwarning("<CRewardSharing userValidSelect> Invalid validation char '%u'.",_Candidates[ _Rewards[i].Candidates[j].first ].UserRow.getIndex());
							}
							else
							{
								if ( _Rewards[i].Item != NULL )
								{
									if ( !user->addItemToInventory(INVENTORIES::bag, _Rewards[i].Item) && !user->addItemToInventory(INVENTORIES::temporary, _Rewards[i].Item) )
									{
										_Rewards[i].Item.deleteItem();
										nlwarning("<CRewardSharing::userValidSelect> Both Temp and bag are full for user %s. item index :'%u'. '%u'",user->getId().toString().c_str(),i,_Rewards.size() );
									}
								}
								else
								{
									uint16 slot = user->getFirstFreePhraseSlot();
									// we replace previous phrase to force the learning of new bricks
									if ( slot == 0xFFFF )
										slot = 0;
									user->learnPrebuiltPhrase( _Rewards[i].SheetId,slot,true );
									SM_STATIC_PARAMS_1(params, STRING_MANAGER::sphrase);
									params[0].SheetId = _Rewards[i].SheetId;
									PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_RECV_ACTION",params);
								}
								given = true;
								break;
							}
						}
					}
				}
			}
		}
		// close and reset the interfaces
		for ( uint i = 0; i < _Candidates.size(); i++ )
		{
			CCharacter * user = PlayerManager.getChar(_Candidates[i].UserRow);
			if ( user )
			{
				CEntityId id = user->getId();
				CMessage msgout ("IMPULSION_ID");
				msgout.serial (id);
				CBitMemStream bms;
				if (!GenericMsgManager.pushNameToStream ("TEAM:SHARE_CLOSE", bms))
					nlstopex (("Missing TEAM:SHARE_OPEN in msg.xml"));
				msgout.serialBufferWithSize ((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror ( NLNET::TServiceId(id.getDynamicId()), msgout);
				// clear unused entries
				for (uint j = 0;j < 16; j++)
				{
					CBankAccessor_PLR::TINVENTORY::TSHARE::TArray &shareItem = CBankAccessor_PLR::getINVENTORY().getSHARE().getArray(j);
					
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:SHEET",j),0 );
					shareItem.setSHEET(user->_PropertyDatabase, CSheetId::Unknown);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:QUALITY",j),0 );
					shareItem.setQUALITY(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:QUANTITY",j), 0 );
					shareItem.setQUANTITY(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:USER_COLOR",j), 0 );
					shareItem.setUSER_COLOR(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:WEIGHT",j), 0 );
					shareItem.setWEIGHT(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:NAMEID",j), 0 );
					shareItem.setNAMEID(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:INFO_VERSION",j),0);
					shareItem.setINFO_VERSION(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:NB_MEMBER",j),0);
					shareItem.setNB_MEMBER(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:WANTED",j), 0 );
					shareItem.setWANTED(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:CHANCE",j), 0 );					
					shareItem.setCHANCE(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:RM_CLASS_TYPE",j) , 0 );
					shareItem.setRM_CLASS_TYPE(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:RM_FABER_STAT_TYPE",j) , 0 );
					shareItem.setRM_FABER_STAT_TYPE(user->_PropertyDatabase, 0);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:PREREQUISIT_VALID",j) , 0 );
					shareItem.setPREREQUISIT_VALID(user->_PropertyDatabase, 0);
				}
				for ( uint j = 0; j < 8; j++)
				{
					CBankAccessor_PLR::TINVENTORY::TSHARE::TTM_ &tmItem = CBankAccessor_PLR::getINVENTORY().getSHARE().getTM_(j);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:TM_%u:VALID",j),0);
					tmItem.setVALID(user->_PropertyDatabase, false);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:TM_%u:NAME",j),0);
					tmItem.setNAME(user->_PropertyDatabase, ucstring());
				}
			}
		}
		return true;
	}
	setUsersDb();
	return false;
}

void CRewardSharing::setUsersDb()
{
	for (uint k  = 0; k < _Candidates.size(); ++k )
	{
		CCharacter * user = PlayerManager.getChar( _Candidates[k].UserRow );
		if ( user ) try
		{
			// set version
//			user->_PropertyDatabase.setProp("INVENTORY:SHARE:SESSION",_Session);
			CBankAccessor_PLR::getINVENTORY().getSHARE().setSESSION(user->_PropertyDatabase, _Session);
			uint i = 0;
			for ( ; i < _Candidates.size(); i++ )
			{
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:TM_%u:VALID",i),_Candidates[i].Validated?1:0);
				CBankAccessor_PLR::getINVENTORY().getSHARE().getTM_(i).setVALID(user->_PropertyDatabase, _Candidates[i].Validated);
				CMirrorPropValueRO<TYPE_NAME_STRING_ID> nameId(TheDataset, _Candidates[i].UserRow, DSPropertyNAME_STRING_ID );	
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:TM_%u:NAME",i),nameId());
				CBankAccessor_PLR::getINVENTORY().getSHARE().getTM_(i).setNAME(user->_PropertyDatabase, nameId());
			}
			for ( uint j = i; j < 8; j++)
			{
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:TM_%u:VALID",j),0);
				CBankAccessor_PLR::getINVENTORY().getSHARE().getTM_(j).setNAME(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:TM_%u:NAME",j),0);
				CBankAccessor_PLR::getINVENTORY().getSHARE().getTM_(j).setNAME(user->_PropertyDatabase, 0);
			}
			
			

			uint j = 0;
			for (; j < _Rewards.size(); j++)
			{
				CBankAccessor_PLR::TINVENTORY::TSHARE::TArray &shareItem = CBankAccessor_PLR::getINVENTORY().getSHARE().getArray(j);
				// set phrase property
				if ( _Rewards[j].Item == NULL )
				{
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:SHEET",j),_Rewards[j].SheetId.asInt() );
					shareItem.setSHEET(user->_PropertyDatabase, _Rewards[j].SheetId);
				}
				// set item property
				else
				{
					CGameItemPtr item = _Rewards[j].Item;
					static CSheetId stackId ("stack.sitem");
//					if ( item->getSheetId() == stackId )
//					{
//						if ( item->getChildren().empty() || item->getChildren()[0] == NULL )
//						{
//							nlwarning("<CRewardSharing setUsersDb>EMPTY STACK...");
//							return;
//						}
//						item = item->getChildren()[0];
//					}

//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:SHEET",j),item->getSheetId().asInt() );
					shareItem.setSHEET(user->_PropertyDatabase, item->getSheetId());
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:QUALITY",j),item->quality() );
					shareItem.setQUALITY(user->_PropertyDatabase, item->quality());
					uint32 quantity = item->getStackSize();
//					uint16 quantity = item->getChildren().size();
//					if ( quantity == 0)
//						quantity = 1;
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:QUANTITY",j), quantity );
					shareItem.setQUANTITY(user->_PropertyDatabase, checkedCast<uint16>(quantity));
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:USER_COLOR",j), item->color() );
					shareItem.setUSER_COLOR(user->_PropertyDatabase, item->color());
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:WEIGHT",j), (uint16)(item->weight() / 10 ) );
					shareItem.setWEIGHT(user->_PropertyDatabase, checkedCast<uint16>(item->weight() / 10 ));
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:NAMEID",j), item->sendNameId(user) );
					shareItem.setNAMEID(user->_PropertyDatabase, item->sendNameId(user));
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:RM_CLASS_TYPE",j) , item->getItemClass() );
					shareItem.setRM_CLASS_TYPE(user->_PropertyDatabase, item->getItemClass());
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:RM_FABER_STAT_TYPE",j) , item->getCraftParameters() == 0 ? RM_FABER_STAT_TYPE::Unknown : item->getCraftParameters()->getBestItemStat() );
					shareItem.setRM_FABER_STAT_TYPE(user->_PropertyDatabase, item->getCraftParameters() == 0 ? RM_FABER_STAT_TYPE::Unknown : item->getCraftParameters()->getBestItemStat() );
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:PREREQUISIT_VALID",j) , user->checkPreRequired(item) );
					shareItem.setPREREQUISIT_VALID(user->_PropertyDatabase, user->checkPreRequired(item) );
					
					string infoEntry = NLMISC::toString("INVENTORY:SHARE:%u:INFO_VERSION",0);
//					user->_PropertyDatabase.setProp( infoEntry,0 /*user->_PropertyDatabase.getProp(infoEntry) + 1*/ );
					shareItem.setINFO_VERSION(user->_PropertyDatabase, 0);
				}
				// set generic property
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:NB_MEMBER",j), _Rewards[j].Candidates.size() );
				shareItem.setNB_MEMBER(user->_PropertyDatabase, (uint8)_Rewards[j].Candidates.size());
				
				uint p = 0;
				for (; p < _Rewards[j].Candidates.size();p++)
				{
					if ( _Candidates [ _Rewards[j].Candidates[p].first ].UserRow == user->getEntityRowId() )
					{
//						user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:WANTED",j), 1 );
						shareItem.setWANTED(user->_PropertyDatabase, true);
//						user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:CHANCE",j), (sint64)_Rewards[j].Candidates[p].second );
						shareItem.setCHANCE(user->_PropertyDatabase, checkedCast<uint8>(_Rewards[j].Candidates[p].second));
						break;
					}
				}
				if ( p == _Rewards[j].Candidates.size() )
				{
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:WANTED",j), 0 );
					shareItem.setWANTED(user->_PropertyDatabase, false);
//					user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:CHANCE",j), 0 );
					shareItem.setCHANCE(user->_PropertyDatabase, 0);
				}
			}
			// clear unused entries
			for (;j < 16; j++)
			{
				CBankAccessor_PLR::TINVENTORY::TSHARE::TArray &shareItem = CBankAccessor_PLR::getINVENTORY().getSHARE().getArray(j);
				
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:SHEET",j),0 );
				shareItem.setSHEET(user->_PropertyDatabase, CSheetId::Unknown);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:QUALITY",j),0 );
				shareItem.setQUALITY(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:QUANTITY",j), 0 );
				shareItem.setQUANTITY(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:USER_COLOR",j), 0 );
				shareItem.setUSER_COLOR(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:WEIGHT",j), 0 );
				shareItem.setWEIGHT(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:NAMEID",j), 0 );
				shareItem.setNAMEID(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:INFO_VERSION",j),0);
				shareItem.setINFO_VERSION(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:NB_MEMBER",j),0);
				shareItem.setNB_MEMBER(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:WANTED",j), 0 );
				shareItem.setWANTED(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:CHANCE",j), 0 );					
				shareItem.setCHANCE(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:RM_CLASS_TYPE",j) , 0 );
				shareItem.setRM_CLASS_TYPE(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:RM_FABER_STAT_TYPE",j) , 0 );
				shareItem.setRM_FABER_STAT_TYPE(user->_PropertyDatabase, 0);
//				user->_PropertyDatabase.setProp( NLMISC::toString("INVENTORY:SHARE:%u:PREREQUISIT_VALID",j) , 0 );
				shareItem.setPREREQUISIT_VALID(user->_PropertyDatabase, 0);
			}
		}
		catch(const NLMISC::Exception &e)
		{
			nlwarning("exception in <CRewardSharing setUsersDb> :'%s'",e.what());
		}
	}
}//CRewardSharing::setUsersDb

/// give all items to the specified player
void CRewardSharing::giveAllItems( const TDataSetRow & row)
{
	CCharacter * user = PlayerManager.getChar(row);
	if ( user )
	{
		// give the rewards
		for ( uint i = 0; i < _Rewards.size();i++ )
		{
			if ( _Rewards[i].Item != NULL )
			{
				if ( !user->addItemToInventory(INVENTORIES::bag, _Rewards[i].Item) && !user->addItemToInventory(INVENTORIES::temporary, _Rewards[i].Item) )
				{
					_Rewards[i].Item.deleteItem();
					nlwarning("<CRewardSharing::userValidSelect> Both Temp and bag are full for user %s. item index :'%u'. '%u'",user->getId().toString().c_str(),i,_Rewards.size() );
				}
			}
			else
			{
				uint16 slot = user->getFirstFreePhraseSlot();
				// we replace previous phrase to force the learning of new bricks
				if ( slot == 0xFFFF )
					slot = 0;
				user->learnPrebuiltPhrase( _Rewards[i].SheetId,slot,true );
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::sphrase);
				params[0].SheetId = _Rewards[i].SheetId;
				PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_RECV_ACTION",params);
			}
		}
		CMessage msgout ("IMPULSION_ID");
		msgout.serial ((CEntityId&)(user->getId()));
		CBitMemStream bms;
		if (!GenericMsgManager.pushNameToStream ("TEAM:SHARE_CLOSE", bms))
			nlstopex (("Missing TEAM:SHARE_OPEN in msg.xml"));
		msgout.serialBufferWithSize ((uint8*)bms.buffer(), bms.length());
		sendMessageViaMirror ( NLNET::TServiceId(user->getId().getDynamicId()), msgout);
	}
}// CRewardSharing


