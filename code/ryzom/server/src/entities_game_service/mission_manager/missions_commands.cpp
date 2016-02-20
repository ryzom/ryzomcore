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
#include "mission_manager/ai_alias_translator.h"
#include "nel/net/service.h"
#include "nel/misc/command.h"
#include "nel/misc/algo.h"

#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "mission_manager/mission_manager.h"
#include "primitives_parser.h"
#include "team_manager/team.h"
#include "team_manager/team_manager.h"
#include "mission_manager/mission_team.h"
#include "mission_manager/mission_step_ai.h"
#include "mission_manager/mission_guild.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "building_manager/building_manager.h"
#include "building_manager/building_physical.h"

#include "admin.h"
#include "creature_manager/creature_manager.h"
#include "world_instances.h"

#include "server_share/used_continent.h"
#include "game_share/shard_names.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

NLMISC_COMMAND( forceMissionProgress,"debug command used to trigger debug commands","<user>" )
{
	if (args.empty() || args.size() > 3)
		return false;
	CEntityId id;
	id.fromString( args[0].c_str() );
	CCharacter * c = PlayerManager.getChar( id );
	if (!c)
		return true;
	CMissionEventDebug event;
	c->processMissionEvent(event);
	return true;
}

//-----------------------------------------------
// forceJournalUpdate
//-----------------------------------------------
NLMISC_COMMAND(forceJournalUpdate,"force mission journal update","<player id(id:type:crea:dyn)>")
{
	if ( args.size() != 1 )
		return false;
	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * user = PlayerManager.getChar( id );
	if( !user )
	{
		log.displayNL("invalid char");
		return true;
	}
	for ( map<TAIAlias,CMission*>::iterator it = user->getMissionsBegin(); it != user->getMissionsEnd(); ++it )
	{
		(*it).second->updateUsersJournalEntry();
	}

	CTeam * team = TeamManager.getTeam( user->getTeamId() );
	if ( team )
	{
		for ( uint i  = 0; i < team->getMissions().size(); i++ )
		{
			team->getMissions()[i]->updateUsersJournalEntry();
		}
	}
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( user->getGuildId() );
	if (guild)
	{
		for ( uint i  = 0; i < guild->getMissions().size(); i++ )
		{
			guild->getMissions()[i]->updateUsersJournalEntry();
		}
	}
	/*for (uint i = 0; i < MaxGuildMissionCount; i++)
	{
		/// todo guild mission
	}*/
	return true;
} // missionProgress //

//-----------------------------------------------
// simMissionEvent
//-----------------------------------------------
NLMISC_COMMAND(simMissionEvent,"simulate a mission event","<player> <event type> *[<event param>]")
{
	if ( args.empty() )
		return false;
	if ( !CMissionEvent::simMissionEvent( args,log ) )
		log.displayNL("simMissionEvent failed");
	return true;
} // simMissionEvent //

//-----------------------------------------------
// reload missions
//-----------------------------------------------
NLMISC_COMMAND(reloadMissions,"reload the mission primitives. Picked missions are erased","[bool telling if we have tio reset aliases (default : false)]")
{
	if ( args.size() > 1 )
		return true;
	CPlayerManager::TMapPlayers::const_iterator itPlayer = PlayerManager.getPlayers().begin();

	for (; itPlayer != PlayerManager.getPlayers().end(); ++itPlayer )
	{
		if ( (*itPlayer).second.Player )
		{
			CCharacter * user = (*itPlayer).second.Player->getActiveCharacter();
			if ( user )
			{
				while ( user->getMissionsBegin() != user->getMissionsEnd()  )
				{
					user->removeMission( ( *user->getMissionsBegin() ).first, mr_forced);
				}
				CTeam * team = TeamManager.getRealTeam( user->getTeamId() );
				if ( team )
				{
					for ( uint i = 0; i < team->getMissions().size(); i++ )
					{
						team->removeMission( i, mr_forced);
					}
				}
				/// todo guild mission
			}
		}
	}

	bool reloadAliases = false;
	if ( args.size() == 1 && ( args[0] == "true" || args[0] == "1" ) )
		reloadAliases = true;
	CMissionManager::release();
	if ( reloadAliases )
	{
		log.displayNL( "please restart AI service" );
		CAIAliasTranslator::release();
	}
	if ( reloadAliases )
		CAIAliasTranslator::init();
	CMissionManager::init();
	log.displayNL( "missions reloaded" );
	return true;
} // reloadMissions



NLMISC_COMMAND(addSuccessfulMission,"add a successful mission to the player","<player > <mission alias>")
{
	if ( args.size() != 2 )
		return false;
	
	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * user = PlayerManager.getChar( id );
	if (user)
	{
		TAIAlias alias;
		NLMISC::fromString(args[1], alias);
		const CHashMap< uint,CMissionTemplate* > &mts = CMissionManager::getInstance()->getMissionTemplates();
		if (mts.find(alias) == mts.end())
		{
			log.displayNL("Invalid mission");
			return true;
		}
		CMissionTemplate *mt = mts.find(alias)->second;
		if (mt)
			user->addSuccessfulMissions( *mt );
	}
	else
		log.displayNL("Invalid user");

	return true;
} // addSuccesfulMission



NLMISC_COMMAND(createMissionItem,"","")
{
	if ( args.size() != 3 )
		return false;
	string varName;

	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * user = PlayerManager.getChar(id);

	if ( !user )
	{
		log.displayNL( "invalid char" );
		return true;
	}

	uint16 quantity;
	NLMISC::fromString(args[1], quantity);

	std::vector< std::string > script;
	vector< pair<string, STRING_MANAGER::TParamType > > chatParams;
	NLMISC::splitString( args[2],":",script );

	CMissionItem item;
	item.buildFromScript( script,chatParams, varName );

	item.createItemInTempInv( user,quantity );
		
	return true;
} // createMissionItem

NLMISC_COMMAND(clearMissionDone,"Clear the list of already done missions.","<character id(id:type:crea:dyn)>")
{
	if (args.size() != 1)
		return false;
	
	CEntityId id;
	id.fromString( args[0].c_str() );
	
	CCharacter *c = PlayerManager.getChar(id);
	if( c == 0 )
	{
		log.displayNL("<clearMissionDone> unknown character '%s'", id.toString().c_str() );
		return false;
	}
	
	c->clearSuccessfullMission();
	return true;
}

NLMISC_COMMAND(clearMissionGlobalReplayTimer, "Clear the global replay timer of all mission", "")
{
	CMissionManager *mm = CMissionManager::getInstance();

	const CHashMap< uint,CMissionTemplate* >&mts = mm->getMissionTemplates();
	CHashMap< uint,CMissionTemplate* >::const_iterator first(mts.begin()), last(mts.end());
	for (; first != last; ++first)
	{
		const CMissionTemplate *mt = first->second;

		mt->resetGlobalReplayTimer();
	}

	log.displayNL("cleared %u mission global replay timer.", mts.size());
	return true;
}

NLMISC_COMMAND(displayMissionsPlayerStats, "display the missions statistic data for a player", "<character_id>")
{

	if (args.size() != 1)
	{
		log.displayNL("Syntax error: missing entity id");
		return false;
	}

	CEntityId eid(args[0]);

	if (eid == CEntityId::Unknown)
	{
		log.displayNL("Invalid entity id '%s'", args[0].c_str());
		return false;
	}

//	CCharacterManager *cm = CCharacterManager::getInstance();
	CCharacter *c = PlayerManager.getChar(eid);
	if (c == NULL)
	{
		log.displayNL("No charactere with entity id '%s'", args[0].c_str());
		return false;
	}

//	const std::vector< CMissionInstanceSolo* > & missions = c->getMissions();


	const std::map< TAIAlias, TMissionHistory > &mhs = c->getMissionHistories();
	log.displayNL("Listing %u missions history for player '%s'", mhs.size(), args[0].c_str());
	std::map< TAIAlias, TMissionHistory >::const_iterator firsth(mhs.begin()), lasth(mhs.end());
	for (; firsth != lasth; ++firsth)
	{
		const TMissionHistory &mh = firsth->second;
		bool	running = false;

		map<TAIAlias, CMission*>::iterator first(c->getMissionsBegin()), last(c->getMissionsEnd());
		for (; first != last; ++first)
		{
			CMission *mission = first->second;
			if (mission->getTemplateId() == firsth->first)
			{
				running = true;
				break;
			}
		}

		log.displayNL("Mission '%s' \t(alias %10u) : Success:%s, Status:%s, last success : %s",
			CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId(firsth->first).c_str(),
			firsth->first,
			mh.Successfull ? "YES":"NO",
			running ? "RUNNING":"NOT RUNNING",
			mh.LastSuccessDate != 0 ? toString("%10u ticks ago", CTickEventHandler::getGameCycle() - mh.LastSuccessDate).c_str() : "never");
	}

	return true;
}



NLMISC_COMMAND(simAISMAction, "simulate an AI action.", "<action name> *[params]")
{
	if ( args.size() == 2 )
	{
		TAIAlias alias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( args[1] );
		if ( alias != CAIAliasTranslator::Invalid )
		{
			const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate(alias);
			if ( templ != NULL )
			{
				if ( args[0] == "end_escort" )
				{
					for ( uint i = 0; i < templ->Instances.size() ; i++)
					{
						if ( templ->Instances[i] )
						{
							vector<TDataSetRow> entities;
							templ->Instances[i]->getEntities( entities );
							for ( uint j = 0; j < entities.size() ; j++ )
							{
								CCharacter * user = PlayerManager.getChar( entities[j] );
								if( user)
								{
									CMissionEventEscort event( alias );
									user->processMissionEvent( event, alias );
								}
								else
									nlwarning( "<CCAisActionMsgImp callback> invalid user %u",entities[j].getIndex() );
							}
						}
						else
							nlwarning( "<CCAisActionMsgImp callback> %s mission %s  has a NULL instance ",args[0].c_str(),args[1].c_str() );
					}
				}
				else if ( args[0] == "fail" )
				{
					bool exit = false;
					
					// get instance currently in escort step
					for ( uint i = 0; (i < templ->Instances.size()) && !exit ; ++i)
					{
						if ( templ->Instances[i] != NULL )
						{
							// check step
							for ( map<uint32, EGSPD::CActiveStepPD>::const_iterator itStep = templ->Instances[i]->getStepsBegin(); itStep != templ->Instances[i]->getStepsEnd(); ++itStep )
							{
								nlassert( uint( (*itStep).second.getIndexInTemplate() - 1 ) < templ->Steps.size() );
								
								CMissionStepEscort *escortStep = dynamic_cast<CMissionStepEscort*> (templ->Steps[ (*itStep).second.getIndexInTemplate() - 1 ]);
								if ( escortStep != NULL )
								{
									templ->Instances[i]->onFailure(false);
									
									exit = true;
									break;
								}
							}							
						}
						else
							nlwarning( "<CCAisActionMsgImp callback> *fail* mission %s  has a NULL instance ",args[1].c_str() );
					}
				}
				else
				{
					for ( uint i = 0; i < templ->Instances.size() ; ++i)
					{
						if ( templ->Instances[i] )
						{
							vector<TDataSetRow> entities;
							templ->Instances[i]->getEntities( entities );
							for ( uint j = 0 ; j < entities.size() ; ++j )
							{
								CCharacter * user = PlayerManager.getChar( entities[j] );
								if( user)
								{
									CMissionEventAIMsg event(args[0]);
									user->processMissionEvent( event, alias );
								}
								else
									nlwarning( "<CCAisActionMsgImp callback> invalid user %u",entities[j].getIndex() );
							}
						}
						else
							nlwarning( "<CCAisActionMsgImp callback> %s mission %s  has a NULL instance ",args[0].c_str(),args[1].c_str() );
					}
				}
			}
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// failMissionCategory
//-----------------------------------------------
NLMISC_COMMAND(failMissionCategory,"fail all missions of a specified category for all players","<mission_category>")
{
	if (args.size() != 1)
		return false;
	
	string sMissCat = strlwr(args[0]);

	CMissionManager *pMM = CMissionManager::getInstance();

	// For all characters connected
	const CPlayerManager::TMapPlayers &allPlayers = PlayerManager.getPlayers();
	CPlayerManager::TMapPlayers::const_iterator it = allPlayers.begin();
	while (it != allPlayers.end())
	{
		CPlayer *pPlayer = it->second.Player;
		if (pPlayer != NULL)
		{
			CCharacter *pChar = pPlayer->getActiveCharacter();
			if (pChar != NULL)
			{
				// Parse all missions to see if there is a mission that belongs to the mission_category
				std::map<TAIAlias, CMission*>::iterator itMiss = pChar->getMissionsBegin();
				while (itMiss != pChar->getMissionsEnd())
				{
					map<TAIAlias, CMission*>::iterator itNext = itMiss;
					++itNext;

					CMission *pMiss = itMiss->second;
					if (pMiss != NULL)
					{						
						CMissionTemplate *pMissTemplate;
						bool bFailed = false;
						// Check the mission template category
						pMissTemplate = pMM->getTemplate(pMiss->getTemplateId());
						if (pMissTemplate != NULL)
							if (strlwr(pMissTemplate->MissionCategory) == sMissCat)
							{
								pMiss->onFailure(true, false);
								bFailed = true;
							}

						// and the main mission template category
						if (!bFailed) // do not fail a mission twice 
						{
							pMissTemplate = pMM->getTemplate(pMiss->getMainMissionTemplateId());
							if (pMissTemplate != NULL)
								if (strlwr(pMissTemplate->MissionCategory) == sMissCat)
									pMiss->onFailure(true, false);
						}
					}
					// next mission
					itMiss = itNext;
				}
			}
		}
		// next player
		++it;
	}

	return true;
} // failMissionCategory

//-----------------------------------------------
// characterMissionDump
//-----------------------------------------------
NLMISC_COMMAND(characterMissionDump,"dump character missions","<character_id>")
{
	if (args.size() != 1)
		return false;

	GET_CHARACTER

	std::string text;
	uint i = 0;
	for ( map<TAIAlias, CMission*>::iterator it = c->getMissionsBegin(); it != c->getMissionsEnd(); ++it )
	{
		log.displayNL("- %2d: Mission '%s' (alias %10u) Bot Giver Alias: %10u", i++,
			CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId((*it).first).c_str(),
			(*it).first,
			(*it).second->getGiver());
	}

	return true;
}

//-----------------------------------------------
// removeMission
//-----------------------------------------------
NLMISC_COMMAND(removeMission,"Remove mission of character","<character_id> <mission alias>")
{
	if (args.size() != 2)
		return false;

	GET_CHARACTER

	TAIAlias missionAlias;
	NLMISC::fromString(args[1], missionAlias);
	c->removeMission(missionAlias, 0);
	c->removeMissionFromHistories(missionAlias);

	log.displayNL("Mission '%s' \t(alias %10u)  removed from character %s",
	CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId(missionAlias).c_str(),
	missionAlias, 
	args[0].c_str());

	return true;
}

//-----------------------------------------------
// removeMission
//-----------------------------------------------
NLMISC_COMMAND(addMission,"Add mission to character","<character_id> <Mission giver Alias> <mission alias>")
{
	if (args.size() != 3)
		return false;

	GET_CHARACTER
	
	TAIAlias giverAlias;
	NLMISC::fromString(args[1], giverAlias);	

	TAIAlias missionAlias;
	NLMISC::fromString(args[2], missionAlias);	

	c->endBotChat();
	c->setAfkState(false);
	
	std::list< CMissionEvent* > eventList;
	CMissionManager::getInstance()->instanciateMission(c, missionAlias,	giverAlias, eventList);
	c->processMissionEventList( eventList,true, CAIAliasTranslator::Invalid );

	log.displayNL("Mission '%s' \t(alias %10u)  added to character %s",
		CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId(missionAlias).c_str(),
		missionAlias, 
		args[0].c_str());

	return true;
}



/// Commands used by ARK


CInventoryPtr getInventory(CCharacter *c, const string &inv)
{
	CInventoryPtr inventoryPtr = NULL;
	if (!inv.empty())
	{
		INVENTORIES::TInventory selectedInv = INVENTORIES::toInventory(inv);
		switch (selectedInv)
		{
			case INVENTORIES::temporary:
			case INVENTORIES::bag:
			case INVENTORIES::equipment:
			case INVENTORIES::pet_animal1:
			case INVENTORIES::pet_animal2:
			case INVENTORIES::pet_animal3:
			case INVENTORIES::pet_animal4:
			case INVENTORIES::guild:
			case INVENTORIES::player_room:
				inventoryPtr = c->getInventory(selectedInv);
				break;

			default:
				// No-op
				break;
		}
	}
	return inventoryPtr;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getEid, "get entitiy id of entity", "<uid>")
{

	GET_ACTIVE_CHARACTER

	log.displayNL("%s", c->getId().toString().c_str());

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getItemList, "get list of named items of character by filter", "<uid> [bag sheet quantity_min quantity_max quality_min quality_max extra_infos]")
{

	GET_ACTIVE_CHARACTER

	std::vector<INVENTORIES::TInventory> inventories;

	string selected_inv = "*";
	string filter = "*";
	uint32 quantity_min = 0;
	uint32 quantity_max = 999;
	uint32 quality_min = 0;
	uint32 quality_max = 999;

	string extra;

	if (args.size() > 1)
		selected_inv = args[1];

	if (args.size() > 2)
		filter = args[2];

	if (args.size() > 3)
		fromString(args[3], quantity_min);

	if (args.size() > 4)
		fromString(args[4], quantity_max);

	if (args.size() > 5)
		fromString(args[5], quality_min);

	if (args.size() > 6)
		fromString(args[6], quality_max);

	if (args.size() > 7)
		extra = args[7];

	string msg;

	if (selected_inv != "*")
	{
		std::vector<string> invs;
		NLMISC::splitString(selected_inv, ",", invs);
		for (uint32 i=0; i<invs.size(); i++)
		{
			INVENTORIES::TInventory selectedInv = INVENTORIES::toInventory(invs[i]);
			if (selectedInv != INVENTORIES::UNDEFINED)
				inventories.push_back(selectedInv);
		}
	} else {
		inventories.push_back(INVENTORIES::equipment);
		inventories.push_back(INVENTORIES::bag);
		inventories.push_back(INVENTORIES::pet_animal1);
		inventories.push_back(INVENTORIES::pet_animal2);
		inventories.push_back(INVENTORIES::pet_animal3);
		inventories.push_back(INVENTORIES::pet_animal4);
		inventories.push_back(INVENTORIES::guild);
		inventories.push_back(INVENTORIES::player_room);
	}

	if (inventories.empty()) {
		log.displayNL("ERR: invalid inventories");
		return false;
	}

	for (uint32 i=0; i<inventories.size(); i++)
	{
		CInventoryPtr childSrc = c->getInventory(inventories[i]);
		if (childSrc != NULL)
		{
			uint32 k = 0;
			log.displayNL("#%s", INVENTORIES::toString(inventories[i]).c_str());

			for ( uint j = 0; j < childSrc->getSlotCount(); j++ )
			{
				CGameItemPtr itemPtr = childSrc->getItem(j);
				if (itemPtr != NULL)
				{
					string sheet = itemPtr->getSheetId().toString();
					if (testWildCard(sheet, filter))
					{
						uint32 item_stack = itemPtr->getStackSize();
						uint32 item_quality = itemPtr->quality();
						if (item_stack >= quantity_min && item_stack <= quantity_max
							&& item_quality >= quality_min && item_quality <= quality_max)
						{
							string item_stats = toString("%3d|%s|", j, sheet.c_str());
							if (!extra.empty())
								itemPtr->getStats(extra, item_stats);
							log.displayNL(item_stats.c_str());
						}
					}
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getNamedItemList, "get list of named items of character by filter", "<uid> [bag named quantity_min quantity_max quality_min quality_max extra_infos]")
{

	GET_ACTIVE_CHARACTER

	std::vector<INVENTORIES::TInventory> inventories;

	string selected_inv = "*";
	string filter = "*";
	uint32 quantity_min = 0;
	uint32 quantity_max = 999;
	uint32 quality_min = 0;
	uint32 quality_max = 999;

	string extra;

	if (args.size() > 1)
		selected_inv = args[1];

	if (args.size() > 2)
		filter = args[2];

	if (args.size() > 3)
		fromString(args[3], quantity_min);

	if (args.size() > 4)
		fromString(args[4], quantity_max);

	if (args.size() > 5)
		fromString(args[5], quality_min);

	if (args.size() > 6)
		fromString(args[6], quality_max);

	if (args.size() > 7)
		extra = args[7];

	string msg;

	if (selected_inv != "*")
	{
		std::vector<string> invs;
		NLMISC::splitString(selected_inv, ",", invs);
		for (uint32 i=0; i<invs.size(); i++)
		{
			INVENTORIES::TInventory selectedInv = INVENTORIES::toInventory(invs[i]);
			if (selectedInv != INVENTORIES::UNDEFINED)
				inventories.push_back(selectedInv);
		}
	} else {
		inventories.push_back(INVENTORIES::equipment);
		inventories.push_back(INVENTORIES::bag);
		inventories.push_back(INVENTORIES::pet_animal1);
		inventories.push_back(INVENTORIES::pet_animal2);
		inventories.push_back(INVENTORIES::pet_animal3);
		inventories.push_back(INVENTORIES::pet_animal4);
		inventories.push_back(INVENTORIES::guild);
		inventories.push_back(INVENTORIES::player_room);
	}

	for (uint32 i=0; i<inventories.size(); i++)
	{
		CInventoryPtr childSrc = c->getInventory(inventories[i]);
		if (childSrc != NULL)
		{
			uint32 k = 0;
			log.displayNL("#%s", INVENTORIES::toString(inventories[i]).c_str());

			for ( uint j = 0; j < childSrc->getSlotCount(); j++ )
			{
				CGameItemPtr itemPtr = childSrc->getItem(j);
				if (itemPtr != NULL)
				{
					string phraseId = itemPtr->getPhraseId();
					if (!phraseId.empty() && testWildCard(phraseId, filter))
					{
						uint32 item_stack = itemPtr->getStackSize();
						uint32 item_quality = itemPtr->quality();
						if (item_stack >= quantity_min && item_stack <= quantity_max
							&& item_quality >= quality_min && item_quality <= quality_max)
						{
							string item_stats = toString("%3d|%s|", j, phraseId.c_str());
							if (!extra.empty())
								itemPtr->getStats(extra, item_stats);
							log.displayNL(item_stats.c_str());
						}
					}
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(deleteInventoryItems, "Delete items from a characters inventory", "<uid> <sheetnames> <quality> <quantity>")
{
	if (args.size () < 5)
	{
		log.displayNL("ERR: Invalid number of parameters. Parameters: <inventory> <sheetnames> <quality> <quantity>");
		return false;
	}

	GET_ACTIVE_CHARACTER

	std::map<string, uint32> need_items;

	std::vector<string> sheet_names;
	NLMISC::splitString(args[1], ",", sheet_names);
	std::vector<string> qualities;
	NLMISC::splitString(args[2], ",", qualities);
	std::vector<string> quantities;
	NLMISC::splitString(args[3], ",", quantities);

	for (uint32 i=0; i < std::min(quantities.size(), std::min(qualities.size(), sheet_names.size())); i++)
	{
		uint32 quantity = 0;
		fromString(quantities[i], quantity);
		need_items.insert(make_pair(sheet_names[i]+":"+qualities[i], quantity));
	}

	std::map<uint32, uint32> slots;
	std::map<string, uint32>::iterator itNeedItems;

	// Save list of slots and quantities to delete
	CInventoryPtr inventory = c->getInventory(INVENTORIES::bag);
	if (inventory != NULL)
	{
		for ( uint32 j = 0; j < inventory->getSlotCount(); j++ )
		{
			CGameItemPtr itemPtr = inventory->getItem(j);
			if (itemPtr != NULL)
			{
				string sheet = itemPtr->getSheetId().toString();
				uint32 item_quality = itemPtr->quality();
				itNeedItems = need_items.find(sheet+":"+NLMISC::toString("%d", item_quality));
				if (itNeedItems != need_items.end() && (*itNeedItems).second > 0)
				{
					nlinfo("Found : %s %d", sheet.c_str(), item_quality);
					uint32 quantity = std::min((*itNeedItems).second, itemPtr->getStackSize());
					slots.insert(make_pair(j, quantity));
					(*itNeedItems).second -= quantity;
				}
			}
		}

		// Check if all items has been found
		for ( itNeedItems = need_items.begin(); itNeedItems != need_items.end(); ++itNeedItems )
		{
			if ((*itNeedItems).second != 0) {
				nlinfo("Missing : %s", (*itNeedItems).first.c_str());
				log.displayNL("ERR: Not enough items.");
				return false;
			}
		}

		//Delete them
		for ( std::map<uint32, uint32>::iterator it = slots.begin(); it != slots.end(); ++it )
		{
			nlinfo("Deleting... %d, %d", (*it).first, (*it).second);
			inventory->deleteStackItem((*it).first, (*it).second);
		}
	}

	return true;
}



//----------------------------------------------------------------------------
NLMISC_COMMAND(getPosition, "get position of entity", "<uid>")
{

	GET_ACTIVE_CHARACTER

	double x = 0, y = 0, z = 0, h = 0;
	sint32 cell = 0;

	x = c->getState().X / 1000.;
	y = c->getState().Y / 1000.;
	z = c->getState().Z / 1000.;
	h = c->getState().Heading;

	TDataSetRow dsr = c->getEntityRowId();
	CMirrorPropValueRO<TYPE_CELL> srcCell( TheDataset, dsr, DSPropertyCELL );
	cell = srcCell;

	log.displayNL("%.2f|%.2f|%.2f|%.4f|%d", x, y, z, h, cell);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getFame, "get fame of player", "<uid> faction")
{

	if (args.size () < 2)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}
	
	GET_ACTIVE_CHARACTER

	uint32 factionIndex	= CStaticFames::getInstance().getFactionIndex(args[1]);
	if (factionIndex == CStaticFames::INVALID_FACTION_INDEX)
	{
		log.displayNL("ERR: invalid fame");
		return false;
	}
	
	sint32 fame = CFameInterface::getInstance().getFameIndexed(c->getId(), factionIndex);
	log.displayNL("%d", fame);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getFames, "get fames of player", "<uid> faction1,faction2,faction3,...")
{

	if (args.size () < 2)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}
	
	GET_ACTIVE_CHARACTER

	string sfames;

	std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance = c->getAllegiance();
	log.displayNL("%s", PVP_CLAN::toString(allegiance.first).c_str());
	log.displayNL("%s", PVP_CLAN::toString(allegiance.second).c_str());
	log.displayNL("%d", c->getOrganization());

	std::vector<string> fames;
	NLMISC::splitString(args[1], ",", fames);
	for (uint32 i=0; i<fames.size(); i++)
	{
			
		uint32 factionIndex	= CStaticFames::getInstance().getFactionIndex(fames[i]);
		if (factionIndex == CStaticFames::INVALID_FACTION_INDEX)
			log.displayNL("ERR: invalid fame");
		else
			log.displayNL("%d", CFameInterface::getInstance().getFameIndexed(c->getId(), factionIndex));
	}

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(getTarget, "get target of player", "<uid>")
{

	GET_ACTIVE_CHARACTER

	const CEntityId &target = c->getTarget();
	string msg = target.toString()+"|";

	if (target == CEntityId::Unknown)
	{
		log.displayNL("0");
		return true;
	}

	if (target.getType() == RYZOMID::creature)
		msg += "c|";
	else if (target.getType() == RYZOMID::npc)
		msg += "n|";
	else if (target.getType() == RYZOMID::player)
		msg += "p|";
	else
		msg += "0";

	if (target.getType() == RYZOMID::player)
	{
		CCharacter * cTarget = dynamic_cast<CCharacter*>(CEntityBaseManager::getEntityBasePtr(target));
		if (cTarget) {
			msg += cTarget->getName().toString()+"|";

			if (c->getGuildId() != 0 && c->getGuildId() == cTarget->getGuildId())
				msg += "g|";
			else
				msg += "0|";

			if (c->getTeamId() != CTEAM::InvalidTeamId && c->getTeamId() == cTarget->getTeamId())
				msg += "t";
			else
				msg += "0";
		}
	}
	
	log.displayNL(msg.c_str());
	
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getMoney, "get money of player", "<uid>")
{

	GET_ACTIVE_CHARACTER

	string value = toString("%"NL_I64"u", c->getMoney());

	log.displayNL(value.c_str());

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(getPvpPoints, "get pvp points of player", "<uid>")
{

	GET_ACTIVE_CHARACTER

	string value = toString("%u", c->getPvpPoint());

	log.displayNL(value.c_str());

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getCivCultOrg, "get civ cult and organization of player", "<uid>")
{

	GET_ACTIVE_CHARACTER

	std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance = c->getAllegiance();


	log.displayNL("%s|%s|%u", PVP_CLAN::toString(allegiance.first).c_str(), PVP_CLAN::toString(allegiance.second).c_str(), c->getOrganization());

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(accessPowo, "give access to the powo", "<uid> player_name number")
{
	GET_ACTIVE_CHARACTER

	IBuildingPhysical * building;	
	if (args.size () >= 3)
		building = CBuildingManager::getInstance()->getBuildingPhysicalsByName("building_instance_ZO_player_11"+args[2]);
	else
		building = CBuildingManager::getInstance()->getBuildingPhysicalsByName("building_instance_ZO_player_111");


	if ( building )
	{

		if (building->getTemplate()->Type == BUILDING_TYPES::Player)
		{

			CBuildingPhysicalPlayer * buildingPlayer = dynamic_cast<CBuildingPhysicalPlayer *>( building );

			CEntityBase *entityBase = PlayerManager.getCharacterByName(CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), args[2]));
			if (buildingPlayer && entityBase)
			{
				CBuildingManager::getInstance()->removePlayerFromRoom( c );
				uint16 ownerId = buildingPlayer->getOwnerIdx( entityBase->getId() );
				sint32 cell;
				buildingPlayer->addUser(c, 0, ownerId, cell);
//				c->setPowoCell(cell);
//				CBuildingManager::getInstance()->setRoomLifeTime(cell, TGameCycle(NLMISC::TGameTime(4*60*60) / CTickEventHandler::getGameTimeStep()));
				log.displayNL("%d", cell);
			}
		} else {
			log.displayNL("ERR: invalid number");
			return false;
		}
	} else {
		log.displayNL("ERR: invalid number");
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(slide, "slide to the powo", "<uid> x y cell [z] [h]")
{

	if (args.size () < 4)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER
	
	string value = args[1];
	
	sint32 x;
	sint32 y;
	sint32 cell = 0; // c->getPowoCell();
	sint32 z = 0;
	float h = 0;

	fromString(args[1], x);
	x *= 1000;
	fromString(args[2], y);
	y *= 1000;
	if (args[3] != "*")
		fromString(args[3], cell);

	if (args.size() >= 5)
	{
		fromString(args[4], z);
		z *= 1000;
	}

	if (args.size() >= 6)
		fromString(args[5], h);

	c->teleportCharacter(x,y,z,false,true,h,0xFF,cell);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(spawn, "spawn entity", "<uid> quantity sheet dispersion orientation groupname x y look cell")
{

	if (args.size () < 10)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER


	uint32 instanceNumber = 0;
	sint32 x = 0;
	sint32 y = 0;
	sint32 z = c->getZ();
	sint32 cell = 0;
	sint32 orientation = 6666; // used to specify a random orientation

	uint32 nbBots;
	fromString(args[1], nbBots);
	if (nbBots<=0)
	{
		log.displayNL("ERR: invalid bot count");
		return false;
	}

	NLMISC::CSheetId sheetId(args[2]);
	if (sheetId == NLMISC::CSheetId::Unknown)
		sheetId = args[2] + ".creature";
	if (sheetId == NLMISC::CSheetId::Unknown)
		return true;

	double dispersionRadius = 10.;
	if (args.size()>3)
	{
		fromString(args[3], dispersionRadius);
		if (dispersionRadius < 0.) {
			log.displayNL("ERR: invalid dispersion");
			return false;
		}
	}

	bool spawnBots = true;

	if (args.size()>4)
	{
		if (args[4] == "self")
		{
			orientation = (sint32)(c->getHeading() * 1000.0);
		}
		else if (args[4] != "random")
		{
			NLMISC::fromString(args[4], orientation);
			orientation = (sint32)((double)orientation / 360.0 * (NLMISC::Pi * 2.0) * 1000.0);
		}
	}

	string botsName = args[5];
		
	float userX;
	NLMISC::fromString(args[6], userX);
	x = (sint32)(userX * 1000.0);

	float userY;
	NLMISC::fromString(args[7], userY);
	y = (sint32)(userY * 1000.0);

	string look = args[8];
	NLMISC::fromString(args[9], cell);

	// See if another AI instance has been specified
	if (botsName.find("@") != string::npos)
	{
		string continent = botsName.substr(0, botsName.find('@'));
		uint32 nr = CUsedContinent::instance().getInstanceForContinent(continent);
		if (nr == ~0)
		{
			log.displayNL("ERR: invalid continent");
			return false;
		}
		instanceNumber = nr;
		botsName = botsName.substr(botsName.find('@') + 1, botsName.size());
	}

	CEntityId playerId = c->getId();

	CMessage msgout("EVENT_CREATE_NPC_GROUP");
	uint32 messageVersion = 1;
	msgout.serial(messageVersion);
	msgout.serial(instanceNumber);
	msgout.serial(playerId);
	msgout.serial(x);
	msgout.serial(y);
	msgout.serial(z);
	msgout.serial(orientation);
	msgout.serial(nbBots);
	msgout.serial(sheetId);
	msgout.serial(dispersionRadius);
	msgout.serial(spawnBots);
	msgout.serial(botsName);
	msgout.serial(look);
	msgout.serial(cell);
	CWorldInstances::instance().msgToAIInstance2(instanceNumber, msgout);

	return true;
}