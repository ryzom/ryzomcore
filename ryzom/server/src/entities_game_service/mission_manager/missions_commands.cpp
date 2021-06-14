// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "egs_sheets/egs_sheets.h"

#include "server_share/pet_interface_msg.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "phrase_manager/phrase_manager.h"
#include "phrase_manager/toxic_cloud.h"
#include "mission_manager/mission_manager.h"
#include "primitives_parser.h"
#include "team_manager/team.h"
#include "team_manager/team_manager.h"
#include "weather_everywhere.h"
#include "death_penalties.h"
#include "harvest_source.h"
#include "mission_manager/mission_team.h"
#include "mission_manager/mission_step_ai.h"
#include "mission_manager/mission_guild.h"
#include "shop_type/named_items.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_member_module.h"
#include "guild_manager/fame_manager.h"
#include "building_manager/building_manager.h"
#include "building_manager/building_physical.h"
#include "progression/progression_pvp.h"
#include "zone_manager.h"
#include "egs_sheets/egs_sheets.h"

#include "admin.h"
#include "creature_manager/creature_manager.h"
#include "world_instances.h"



using namespace NLMISC;
using namespace NLNET;
using namespace std;

extern CCharacterBotChatBeginEnd CharacterBotChatBeginEnd;

NLMISC_COMMAND(forceMissionProgress,"debug command used to trigger debug commands","<user>")
{
	if (args.empty() || args.size() > 3)
		return false;
	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * c = PlayerManager.getChar(id);
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
	if (args.size() != 1)
		return false;
	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * user = PlayerManager.getChar(id);
	if (!user)
	{
		log.displayNL("invalid char");
		return true;
	}
	for (map<TAIAlias,CMission*>::iterator it = user->getMissionsBegin(); it != user->getMissionsEnd(); ++it)
	{
		(*it).second->updateUsersJournalEntry();
	}

	CTeam * team = TeamManager.getTeam(user->getTeamId());
	if (team)
	{
		for (uint i  = 0; i < team->getMissions().size(); i++)
		{
			team->getMissions()[i]->updateUsersJournalEntry();
		}
	}
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId(user->getGuildId());
	if (guild)
	{
		for (uint i  = 0; i < guild->getMissions().size(); i++)
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
	if (args.empty())
		return false;
	if (!CMissionEvent::simMissionEvent(args,log))
		log.displayNL("simMissionEvent failed");
	return true;
} // simMissionEvent //

//-----------------------------------------------
// reload missions
//-----------------------------------------------
NLMISC_COMMAND(reloadMissions,"reload the mission primitives. Picked missions are erased","[bool telling if we have tio reset aliases (default : false)]")
{
	if (args.size() > 1)
		return true;
	CPlayerManager::TMapPlayers::const_iterator itPlayer = PlayerManager.getPlayers().begin();

	for (; itPlayer != PlayerManager.getPlayers().end(); ++itPlayer)
	{
		if ((*itPlayer).second.Player)
		{
			CCharacter * user = (*itPlayer).second.Player->getActiveCharacter();
			if (user)
			{
				while (user->getMissionsBegin() != user->getMissionsEnd() )
				{
					user->removeMission((*user->getMissionsBegin()).first, mr_forced);
				}
				CTeam * team = TeamManager.getRealTeam(user->getTeamId());
				if (team)
				{
					for (uint i = 0; i < team->getMissions().size(); i++)
					{
						team->removeMission(i, mr_forced);
					}
				}
				/// todo guild mission
			}
		}
	}

	bool reloadAliases = false;
	if (args.size() == 1 && (args[0] == "true" || args[0] == "1"))
		reloadAliases = true;
	CMissionManager::release();
	if (reloadAliases)
	{
		log.displayNL("please restart AI service");
		CAIAliasTranslator::release();
	}
	if (reloadAliases)
		CAIAliasTranslator::init();
	CMissionManager::init();
	log.displayNL("missions reloaded");
	return true;
} // reloadMissions



NLMISC_COMMAND(addSuccessfulMission,"add a successful mission to the player","<player > <mission alias>")
{
	if (args.size() != 2)
		return false;

	CEntityId id;
	id.fromString(args[0].c_str());
	CCharacter * user = PlayerManager.getChar(id);
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
			user->addSuccessfulMissions(*mt);
	}
	else
		log.displayNL("Invalid user");

	return true;
} // addSuccesfulMission

NLMISC_COMMAND(clearMissionDone,"Clear the list of already done missions.","<character id(id:type:crea:dyn)>")
{
	if (args.size() != 1)
		return false;

	CEntityId id;
	id.fromString(args[0].c_str());

	CCharacter *c = PlayerManager.getChar(id);
	if (c == 0)
	{
		log.displayNL("<clearMissionDone> unknown character '%s'", id.toString().c_str());
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
	if (args.size() == 2)
	{
		TAIAlias alias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName(args[1]);
		if (alias != CAIAliasTranslator::Invalid)
		{
			const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate(alias);
			if (templ != NULL)
			{
				if (args[0] == "end_escort")
				{
					for (uint i = 0; i < templ->Instances.size() ; i++)
					{
						if (templ->Instances[i])
						{
							vector<TDataSetRow> entities;
							templ->Instances[i]->getEntities(entities);
							for (uint j = 0; j < entities.size() ; j++)
							{
								CCharacter * user = PlayerManager.getChar(entities[j]);
								if (user)
								{
									CMissionEventEscort event(alias);
									user->processMissionEvent(event, alias);
								}
								else
									nlwarning("<CCAisActionMsgImp callback> invalid user %u",entities[j].getIndex());
							}
						}
						else
							nlwarning("<CCAisActionMsgImp callback> %s mission %s  has a NULL instance ",args[0].c_str(),args[1].c_str());
					}
				}
				else if (args[0] == "fail")
				{
					bool exit = false;

					// get instance currently in escort step
					for (uint i = 0; (i < templ->Instances.size()) && !exit ; ++i)
					{
						if (templ->Instances[i] != NULL)
						{
							// check step
							for (map<uint32, EGSPD::CActiveStepPD>::const_iterator itStep = templ->Instances[i]->getStepsBegin(); itStep != templ->Instances[i]->getStepsEnd(); ++itStep)
							{
								nlassert(uint((*itStep).second.getIndexInTemplate() - 1) < templ->Steps.size());

								CMissionStepEscort *escortStep = dynamic_cast<CMissionStepEscort*> (templ->Steps[ (*itStep).second.getIndexInTemplate() - 1 ]);
								if (escortStep != NULL)
								{
									templ->Instances[i]->onFailure(false);

									exit = true;
									break;
								}
							}
						}
						else
							nlwarning("<CCAisActionMsgImp callback> *fail* mission %s  has a NULL instance ",args[1].c_str());
					}
				}
				else
				{
					for (uint i = 0; i < templ->Instances.size() ; ++i)
					{
						if (templ->Instances[i])
						{
							vector<TDataSetRow> entities;
							templ->Instances[i]->getEntities(entities);
							for (uint j = 0 ; j < entities.size() ; ++j)
							{
								CCharacter * user = PlayerManager.getChar(entities[j]);
								if (user)
								{
									CMissionEventAIMsg event(args[0]);
									user->processMissionEvent(event, alias);
								}
								else
									nlwarning("<CCAisActionMsgImp callback> invalid user %u",entities[j].getIndex());
							}
						}
						else
							nlwarning("<CCAisActionMsgImp callback> %s mission %s  has a NULL instance ",args[0].c_str(),args[1].c_str());
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
	for (map<TAIAlias, CMission*>::iterator it = c->getMissionsBegin(); it != c->getMissionsEnd(); ++it)
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
NLMISC_COMMAND(addMission,"Add mission to character", "<character_id> <Mission giver Alias> <mission alias>")
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
	c->processMissionEventList(eventList,true, CAIAliasTranslator::Invalid);

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
			case INVENTORIES::handling:
			case INVENTORIES::pet_animal1:
			case INVENTORIES::pet_animal2:
			case INVENTORIES::pet_animal3:
			case INVENTORIES::pet_animal4:
			case INVENTORIES::pet_animal5:
			case INVENTORIES::pet_animal6:
			case INVENTORIES::pet_animal7:
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

INVENTORIES::TInventory getTInventory(const string &inv)
{
	INVENTORIES::TInventory inventory = INVENTORIES::bag;
	INVENTORIES::TInventory strinv = INVENTORIES::toInventory(inv.c_str());
	switch (strinv)
	{
		case INVENTORIES::temporary:
		case INVENTORIES::bag:
		case INVENTORIES::pet_animal1:
		case INVENTORIES::pet_animal2:
		case INVENTORIES::pet_animal3:
		case INVENTORIES::pet_animal4:
		case INVENTORIES::pet_animal5:
		case INVENTORIES::pet_animal6:
		case INVENTORIES::pet_animal7:
		case INVENTORIES::guild:
		case INVENTORIES::player_room:
			inventory = strinv;
			break;

		default:
			inventory = INVENTORIES::bag;
	}
	return inventory;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getEid, "get entitiy id of entity", "<uid>")
{

	GET_ACTIVE_CHARACTER

	log.displayNL("%s", c->getId().toString().c_str());

	return true;
}

NLMISC_COMMAND(spawnItem, "Spawn a new Item", "<uid> <inv> <quantity(0=force)> <sheetid> <quality> <drop=0|1> [<phraseid>|<param>=<value>,*]")
{

	GET_ACTIVE_CHARACTER

	if (args.size() < 6)
		return false;

	string selected_inv = args[1];

	CInventoryPtr inventory = getInventory(c, selected_inv);
	if (inventory == NULL)
	{
		log.displayNL("ERR: invalid inventory");
		return true;
	}

	uint16 quantity;
	NLMISC::fromString(args[2], quantity);

	if (quantity == 0)
	{
		CSheetId sheet = CSheetId(args[3].c_str());
		uint16 quality = 10;

		std::vector< std::string > quality_params;
		NLMISC::splitString(args[4], ":", quality_params);

		if (quality_params.size() > 0)
			NLMISC::fromString(quality_params[0], quality);

		if (sheet == CSheetId::Unknown)
		{
			log.displayNL("ERR: sheetId is Unknown");
			return true;
		}

		CGameItemPtr item = GameItemManager.createItem(sheet, quality, args[5] == "1", args[5] == "1");
		if (item != NULL)
		{
			if (c->addItemToInventory(getTInventory(selected_inv), item))
			{
				if (quality_params.size() > 1)
				{
					uint16 recommended;
					NLMISC::fromString(quality_params[1], recommended);
					item->recommended(recommended);
				}

				log.displayNL("OK");
				return true;
			}
			item.deleteItem();
		}
	}
	else
	{
		CMissionItem item;
		string params;

		std::vector< std::string > quality_params;
		NLMISC::splitString(args[4], ":", quality_params);
		if (quality_params.size() > 0)
			params = args[3]+":"+quality_params[0]+":"+args[5];
		else
			params = args[3]+":10:"+args[5];

		if (args.size() == 7)
			params += ":"+args[6];

		std::vector< std::string > script;
		NLMISC::splitString(params, ":", script);

		item.buildFromScript(script);
		CGameItemPtr finalItem = item.createItem(quantity);
		if (finalItem != NULL)
		{
			if (c->addItemToInventory(getTInventory(selected_inv), finalItem))
			{

				if (quality_params.size() > 1)
				{
					uint16 recommended;
					NLMISC::fromString(quality_params[1], recommended);
					finalItem->recommended(recommended);
				}

				log.displayNL("OK");
				return true;
			}
			finalItem.deleteItem();
		}
	}

	log.displayNL("ERR: adding item");
	return true;
}


NLMISC_COMMAND(spawnNamedItem, "Spawn a named Item", "<uid> <inv> <quantity> <named_item>")
{
	GET_ACTIVE_CHARACTER

	if (args.size() < 4)
		return false;

	string selected_inv = args[1];

	CInventoryPtr inventory = getInventory(c, selected_inv);
	if (inventory == NULL)
	{
		log.displayNL("ERR: invalid inventory");
		return true;
	}

	uint16 quantity;
	NLMISC::fromString(args[2], quantity);

	CGameItemPtr item = CNamedItems::getInstance().createNamedItem(args[3], quantity);
	if (item != NULL)
	{
		if (c->addItemToInventory(getTInventory(selected_inv), item)) {
			log.displayNL("OK");
			return true;
		}

		item.deleteItem();
	}

	log.displayNL("ERR: adding item");
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(getItemList, "get list of items of character by filter", "<uid> [bag sheet quantity_min quantity_max quality_min quality_max extra_infos]")
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
		inventories.push_back(INVENTORIES::handling);
		inventories.push_back(INVENTORIES::equipment);
		inventories.push_back(INVENTORIES::bag);
		inventories.push_back(INVENTORIES::pet_animal1);
		inventories.push_back(INVENTORIES::pet_animal2);
		inventories.push_back(INVENTORIES::pet_animal3);
		inventories.push_back(INVENTORIES::pet_animal4);
		inventories.push_back(INVENTORIES::pet_animal5);
		inventories.push_back(INVENTORIES::pet_animal6);
		inventories.push_back(INVENTORIES::pet_animal7);
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
			log.displayNL("#%s", INVENTORIES::toString(inventories[i]).c_str());

			for (uint j = 0; j < childSrc->getSlotCount(); j++)
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
		inventories.push_back(INVENTORIES::handling);
		inventories.push_back(INVENTORIES::equipment);
		inventories.push_back(INVENTORIES::bag);
		inventories.push_back(INVENTORIES::pet_animal1);
		inventories.push_back(INVENTORIES::pet_animal2);
		inventories.push_back(INVENTORIES::pet_animal3);
		inventories.push_back(INVENTORIES::pet_animal4);
		inventories.push_back(INVENTORIES::pet_animal5);
		inventories.push_back(INVENTORIES::pet_animal6);
		inventories.push_back(INVENTORIES::pet_animal7);
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

			for (uint j = 0; j < childSrc->getSlotCount(); j++)
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
NLMISC_COMMAND(deleteInventoryItems, "Delete items from a characters inventory", "<uid> <inventory> <sheetnames> <quality> <quantity>")
{
	if (args.size () < 5)
	{
		log.displayNL("ERR: Invalid number of parameters. Parameters: <inventory> <sheetnames> <quality> <quantity>");
		return false;
	}

	GET_ACTIVE_CHARACTER

	std::map<string, uint32> need_items;

	string selected_inv = args[1];

	std::vector<string> sheet_names;
	NLMISC::splitString(args[2], ",", sheet_names);
	std::vector<string> qualities;
	NLMISC::splitString(args[3], ",", qualities);
	std::vector<string> quantities;
	NLMISC::splitString(args[4], ",", quantities);

	for (uint32 i=0; i < std::min(quantities.size(), std::min(qualities.size(), sheet_names.size())); i++)
	{
		uint32 quantity = 0;
		fromString(quantities[i], quantity);
		need_items.insert(make_pair(sheet_names[i]+":"+qualities[i], quantity));
	}

	std::map<uint32, uint32> slots;
	std::map<string, uint32>::iterator itNeedItems;

	// Save list of slots and quantities to delete
	CInventoryPtr inventory = getInventory(c, selected_inv);
	if (inventory != NULL)
	{
		for (uint32 j = 0; j < inventory->getSlotCount(); j++)
		{
			CGameItemPtr itemPtr = inventory->getItem(j);
			if (itemPtr != NULL)
			{
				string sheet = itemPtr->getSheetId().toString();
				uint32 item_quality = itemPtr->quality();
				itNeedItems = need_items.find(sheet+":"+NLMISC::toString("%d", item_quality));
				if (itNeedItems != need_items.end() && (*itNeedItems).second > 0)
				{
					uint32 quantity = std::min((*itNeedItems).second, itemPtr->getStackSize());
					slots.insert(make_pair(j, quantity));
					(*itNeedItems).second -= quantity;
				}
			}
		}

		// Check if all items has been found
		for (itNeedItems = need_items.begin(); itNeedItems != need_items.end(); ++itNeedItems)
		{
			if ((*itNeedItems).second != 0) {
				log.displayNL("ERR: Not enough items.");
				return false;
			}
		}

		//Delete them
		for (std::map<uint32, uint32>::iterator it = slots.begin(); it != slots.end(); ++it)
		{
			inventory->deleteStackItem((*it).first, (*it).second);
		}
	}

	log.displayNL("OK");
	return true;
}



string getJewelEnchantAttr(CSheetId sbrick)
{
	const CStaticBrick * brick = CSheets::getSBrickForm(sbrick);
	if (brick && (brick->Family == BRICK_FAMILIES::BSGMC || brick->Family == BRICK_FAMILIES::BSGMCB))
	{
		if (brick->Params.size() > 0)
		{
			const TBrickParam::IId* param = brick->Params[0];
			CSBrickParamJewelAttrs *sbrickParam = (CSBrickParamJewelAttrs*)param;
			if (param->id() == TBrickParam::JEWEL_ATTRS)
				return sbrickParam->Attribute;
		}
	}
	return "";
}


//enchantEquipedItem 530162 WristR jmod_focus_tryker_1.sbrick
//enchantEquipedItem 530162 Neck tag_fyros_3.sbrick
//enchantEquipedItem 530162 Neck jrez_fulllife_tryker.sbrick,jboost2.sbrick
//----------------------------------------------------------------------------
NLMISC_COMMAND(checkInventoryItems, "Check items from a characters inventory", "<uid> <inventory> <sheetnames> <quality> <quantity>")
{
	if (args.size () < 5)
	{
		log.displayNL("ERR: Invalid number of parameters. Parameters: <inventory> <sheetnames> <quality> <quantity>");
		return false;
	}

	GET_ACTIVE_CHARACTER

	std::map<string, uint32> need_items;

	string selected_inv = args[1];

	std::vector<string> sheet_names;
	NLMISC::splitString(args[2], ",", sheet_names);
	std::vector<string> qualities;
	NLMISC::splitString(args[3], ",", qualities);
	std::vector<string> quantities;
	NLMISC::splitString(args[4], ",", quantities);

	for (uint32 i=0; i < std::min(quantities.size(), std::min(qualities.size(), sheet_names.size())); i++)
	{
		uint32 quantity = 0;
		fromString(quantities[i], quantity);
		need_items.insert(make_pair(sheet_names[i]+":"+qualities[i], quantity));
	}

	std::map<uint32, uint32> slots;
	std::map<string, uint32>::iterator itNeedItems;

	// Save list of slots and quantities to delete
	CInventoryPtr inventory = getInventory(c, selected_inv);
	if (inventory != NULL)
	{
		for (uint32 j = 0; j < inventory->getSlotCount(); j++)
		{
			CGameItemPtr itemPtr = inventory->getItem(j);
			if (itemPtr != NULL)
			{
				string sheet = itemPtr->getSheetId().toString();
				uint32 item_quality = itemPtr->quality();
				itNeedItems = need_items.find(sheet+":"+NLMISC::toString("%d", item_quality));
				if (itNeedItems != need_items.end() && (*itNeedItems).second > 0)
				{
					uint32 quantity = std::min((*itNeedItems).second, itemPtr->getStackSize());
					slots.insert(make_pair(j, quantity));
					(*itNeedItems).second -= quantity;
				}
			}
		}

		// Check if all items has been found
		for (itNeedItems = need_items.begin(); itNeedItems != need_items.end(); ++itNeedItems)
		{
			if ((*itNeedItems).second != 0) {
				log.displayNL("ERR: Not enough items.");
				return false;
			}
		}
	}

	log.displayNL("OK");
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(enchantEquipedItem, "enchantEquipedItem", "<uid> <slotname> <sheet1>,[<sheet2> ...] [<maxSpaLoad>]")
{
	if (args.size () < 3)
	{
		log.displayNL("ERR: Invalid number of parameters. Parameters: <uid> <slotname> <sheet1>,[<sheet2> ...] [<maxSpaLoad>]");
		return false;
	}

	GET_ACTIVE_CHARACTER

	string selected_slot = args[1];

	bool isTag = false;

	std::vector<CSheetId> sheets;
	if (args[2] != "*")
	{
		std::vector<string> sheet_names;
		NLMISC::splitString(args[2], ",", sheet_names);
		for (uint32 i=0; i<sheet_names.size(); i++)
		{
			CSheetId sheet = CSheetId(sheet_names[i]);
			if (getJewelEnchantAttr(sheet) == "tag")
				isTag = true;
			sheets.push_back(sheet);
		}
	}

	CGameItemPtr itemPtr = c->getItem(INVENTORIES::equipment, SLOT_EQUIPMENT::stringToSlotEquipment(selected_slot));
	if (itemPtr != NULL)
	{
		if (isTag)
			itemPtr->getJewelNonTagsEnchantments(sheets);
		else
			itemPtr->getJewelTagsEnchantments(sheets);

		itemPtr->applyEnchantment(sheets);

		if (args.size() > 3)
		{
			float maxSapLoad;
			fromString(args[3], maxSapLoad);
			itemPtr->setMaxSapLoad(maxSapLoad);
		}

		log.displayNL("OK");
		return true;
	}
	log.displayNL("KO");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getEnchantmentInEquipedItem, "getEnchantmentInEquipedItem", "<uid> <slotname>")
{
	if (args.size () < 2)
	{
		log.displayNL("ERR: Invalid number of parameters. Parameters: <uid> <slotname>");
		return false;
	}

	GET_ACTIVE_CHARACTER

	string selected_slot = args[1];

	CGameItemPtr itemPtr = c->getItem(INVENTORIES::equipment, SLOT_EQUIPMENT::stringToSlotEquipment(selected_slot));
	if (itemPtr != NULL)
	{
		const std::vector<CSheetId> &sheets = itemPtr->getEnchantment();
		for (uint32 i=0; i<sheets.size(); i++)
			log.displayNL("%s", sheets[i].toString().c_str());
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(sapLoadInEquipedItem, "reloadSapLoadInEquipedItem", "<uid> <slotname> [<value>]")
{

	if (args.size () < 2)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER

	string selected_slot = args[1];

	CGameItemPtr itemPtr = c->getItem(INVENTORIES::equipment, SLOT_EQUIPMENT::stringToSlotEquipment(selected_slot));
	if (itemPtr != NULL)
	{
		if (args.size() >= 3)
		{
			string quant = args[2];
			uint32 quantity;
			if (quant[0] == '-')
			{
				if (quant.size() > 1)
				{
					fromString(quant.substr(1), quantity);
					itemPtr->consumeSapLoad(quantity);
				}
			}
			else
			{
				fromString(quant, quantity);
				itemPtr->reloadSapLoad(quantity);
			}
		}

		uint32 sapLoad = itemPtr->sapLoad();
		uint32 max = itemPtr->maxSapLoad();
		log.displayNL("%u / %u", sapLoad, max);
	}

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(getPosition, "get position of entity", "<uid>")
{

	GET_ACTIVE_CHARACTER

	log.displayNL("%s", c->getPositionInfos().c_str());

	return true;
}


//----------------------------------------------------------------------------
// DEPRECATED use getTarget who send also position
NLMISC_COMMAND(getTargetPosition, "get position of entity", "<uid>")
{

	GET_ACTIVE_CHARACTER

	CCreature * target = CreatureManager.getCreature(c->getTarget());
	if (target)
	{
		double x = target->getState().X / 1000.;
		double y = target->getState().Y / 1000.;
		double z = target->getState().Z / 1000.;
		double h = target->getState().Heading;

		TDataSetRow dsr = target->getEntityRowId();
		CMirrorPropValueRO<TYPE_CELL> srcCell(TheDataset, dsr, DSPropertyCELL);
		sint32 cell = srcCell;

		log.displayNL("%.2f|%.2f|%.2f|%.4f|%d", x, y, z, h, cell);
	} else {
		log.displayNL("0");
	}

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(getBotPosition,"get_bot_position","<uid> <bot_name>")
{
	if (args.size() < 2)
		return false;

	GET_ACTIVE_CHARACTER

	bool found = false;


	if (args[1].find(".creature") != string::npos)
	{
		CSheetId creatureSheetId(args[1]);
		if (creatureSheetId != CSheetId::Unknown)
		{
			double minDistance = -1.;
			CCreature * creature = NULL;

			TMapCreatures::const_iterator it;
			const TMapCreatures& creatures = CreatureManager.getCreature();
			for(it = creatures.begin(); it != creatures.end(); ++it)
			{
				CSheetId sheetId = (*it).second->getType();
				if (sheetId == creatureSheetId)
				{
					double distance = PHRASE_UTILITIES::getDistance(c->getEntityRowId(), (*it).second->getEntityRowId());
					if (!creature || distance < minDistance)
					{
						creature = (*it).second;
						minDistance = distance;
					}
				}
			}

			if (creature)
			{
				double x = creature->getState().X() / 1000.;
				double y = creature->getState().Y() / 1000.;
				double z = creature->getState().Z() / 1000.;
				double h = creature->getState().Heading();


				TDataSetRow dsr = creature->getEntityRowId();
				CMirrorPropValueRO<TYPE_CELL> mirrorCell(TheDataset, dsr, DSPropertyCELL);
				sint32 cell = mirrorCell;
				found = true;
				log.displayNL("%.2f|%.2f|%.2f|%.4f|%d", x, y, z, h, cell);
			}
		}
	}
	else
	{
		vector<TAIAlias> aliases;
		CAIAliasTranslator::getInstance()->getNPCAliasesFromName(args[1], aliases);
		if (!aliases.empty())
		{
			for (uint i = 0; i < aliases.size(); i++)
			{
				TAIAlias alias = aliases[i];

				const CEntityId & botId = CAIAliasTranslator::getInstance()->getEntityId (alias);
				if (botId != CEntityId::Unknown)
				{
					CEntityBase *entityBase = CreatureManager.getCreature (botId);
					if (entityBase != NULL)
					{
						double x = entityBase->getState().X / 1000.;
						double y = entityBase->getState().Y / 1000.;
						double z = entityBase->getState().Z / 1000.;
						double h = entityBase->getState().Heading;

						TDataSetRow dsr = entityBase->getEntityRowId();
						CMirrorPropValueRO<TYPE_CELL> mirrorCell(TheDataset, dsr, DSPropertyCELL);
						sint32 cell = mirrorCell;
						found = true;
						log.displayNL("%.2f|%.2f|%.2f|%.4f|%d", x, y, z, h, cell);
					}
				}
			}
		}
	}

	if (!found)
		log.displayNL("0");

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getFame, "get/set fame of player", "<uid> <faction> [<value>] [<enforce caps>?]")
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

	if (args.size() >= 3)
	{
		string quant = args[2];
		sint32 quantity;
		if (quant[0] == '+')
		{
			if (quant.size() > 1)
			{
				fromString(quant.substr(1), quantity);
				fame += quantity;
			}
		}
		else
		{
			fromString(quant, fame);
		}

		CFameManager::getInstance().setEntityFame(c->getId(), factionIndex, fame, false);
	}

	if (args.size() < 4 || args[3] == "1")
	{
		CFameManager::getInstance().enforceFameCaps(c->getId(), c->getOrganization(), c->getAllegiance());
		// set tribe fame threshold and clamp fame if necessary
		CFameManager::getInstance().setAndEnforceTribeFameCap(c->getId(), c->getOrganization(), c->getAllegiance());
		fame = CFameInterface::getInstance().getFameIndexed(c->getId(), factionIndex);
	}

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

	string msg = c->getTargetInfos();

	log.displayNL("%s", msg.c_str());

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getMoney, "get money of player (if quantity, give/take/set the money)", "<uid> [+-]<quantity>")
{

	if (args.size () < 1)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER

	uint64 money = c->getMoney();

	if (args.size() == 2)
	{
		string quant = args[1];
		uint64 quantity;
		if (quant[0] == '+')
		{
			if (quant.size() > 1)
			{
				fromString(quant.substr(1), quantity);
				money += quantity;
			}
		}
		else if (quant[0] == '-')
		{
			if (quant.size() > 1)
			{
				fromString(quant.substr(1), quantity);
				if (money >= quantity)
				{
					money -= quantity;
				}
				else
				{
					log.displayNL("-1"); // No enough money
					return true;
				}
			}
		}
		else
		{
			fromString(quant, money);
		}

		c->setMoney(money);
	}

	log.displayNL("%" NL_I64 "u", money);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getGuildMoney, "get money of guild (if quantity, give/take/set the money)", "<uid> [+-]<quantity>")
{

	GET_ACTIVE_CHARACTER

	CGuild * g = CGuildManager::getInstance()->getGuildFromId(c->getGuildId());
	if (g)
	{
		uint64 money = g->getMoney();

		if (args.size() == 2)
		{
			string quant = args[1];
			uint64 quantity;
			if (quant[0] == '+')
			{
				if (quant.size() > 1)
				{
					fromString(quant.substr(1), quantity);
					money += quantity;
				}
			}
			else if (quant[0] == '-')
			{
				if (quant.size() > 1)
				{
					fromString(quant.substr(1), quantity);
					if (money >= quantity)
					{
						money -= quantity;
					}
					else
					{
						log.displayNL("-1"); // No enough money
						return true;
					}
				}
			}
			else
			{
				fromString(quant, money);
			}

			g->setMoney(money);
		}

		log.displayNL("%" NL_I64 "u", money);
	} else {
		log.displayNL("ERR: no guild");
	}

	return true;
}



//----------------------------------------------------------------------------
NLMISC_COMMAND(getPvpPoints, "get pvp points of player (if quantity, give/take/set the points)", "<uid> [+-]<quantity>")
{
	GET_ACTIVE_CHARACTER

	uint32 points = c->getPvpPoint();

	if (args.size() == 2)
	{
		string quant = args[1];
		uint32 quantity;
		if (quant[0] == '+')
		{
			if (quant.size() > 1)
			{
				fromString(quant.substr(1), quantity);
				points += quantity;
			}
		}
		else if (quant[0] == '-')
		{
			if (quant.size() > 1)
			{
				fromString(quant.substr(1), quantity);
				if (points >= quantity)
				{
					points -= quantity;
				}
				else
				{
					log.displayNL("-1"); // No enough points
					return true;
				}
			}
		}
		else
		{
			fromString(quant, points);
		}

		c->setPvpPoint(points);
	}

	log.displayNL("%u", points);
}



//----------------------------------------------------------------------------
NLMISC_COMMAND(getFactionPoints, "get faction points of player (if quantity, give/take/set the points)", "<uid> <faction> [[+-]<quantity>]")
{

	if (args.size() < 2)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER

	PVP_CLAN::TPVPClan clan = PVP_CLAN::fromString(args[1]);
	if ((clan < PVP_CLAN::BeginClans) || (clan > PVP_CLAN::EndClans))
	{
		return false;
	}

	uint32 points = c->getFactionPoint(clan);

	if (args.size() >= 3)
	{
		string quant = args[2];
		uint32 quantity;
		if (quant[0] == '+')
		{
			if (quant.size() > 1)
			{
				fromString(quant.substr(1), quantity);
				points += quantity;
			}
		}
		else if (quant[0] == '-')
		{
			if (quant.size() > 1)
			{
				fromString(quant.substr(1), quantity);
				if (points >= quantity)
				{
					points -= quantity;
				}
				else
				{
					log.displayNL("-1"); // No enough points
					return true;
				}
			}
		}
		else
		{
			fromString(quant, points);
		}

		c->setFactionPoint(clan, points, true);
	}

	log.displayNL("%u", points);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getGender, "get gender of player", "<uid>")
{
	GET_ACTIVE_CHARACTER

	if (c->getGender() == GSGENDER::female)
		log.displayNL("f");
	else
		log.displayNL("m");

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getRace, "get race of player", "<uid>")
{
	GET_ACTIVE_CHARACTER

	switch (c->getRace())
	{
		case EGSPD::CPeople::Fyros:
			log.displayNL("f");
			break;
		case EGSPD::CPeople::Matis:
			log.displayNL("m");
			break;
		case EGSPD::CPeople::Tryker:
			log.displayNL("t");
			break;
		case EGSPD::CPeople::Zorai:
			log.displayNL("z");
			break;
		default:
			log.displayNL("0");
	}

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
NLMISC_COMMAND(setOrg, "set the organization of player", "<uid> <org>")
{
	GET_ACTIVE_CHARACTER

	if (args.size() != 2)
	{
		log.displayNL("ERR: invalid arg count");
		return true;
	}

	uint32 org;
	fromString(args[1], org);
	c->setOrganization(org);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setFaction, "set the faction of player", "<uid> <faction> [<civ>]")
{
	if (args.size() < 2)
		return false;

	GET_ACTIVE_CHARACTER

	PVP_CLAN::TPVPClan faction, nation;

	faction = nation = PVP_CLAN::Unknown;

	if (args.size() > 2)
	{
		if (args[2][0] != '*')
			nation = PVP_CLAN::fromString(args[2].c_str());
	}
	if (args[1][0] != '*')
		faction = PVP_CLAN::fromString(args[1].c_str());

	if (nation != PVP_CLAN::Unknown)
		c->setDeclaredCiv(nation);

	if (faction != PVP_CLAN::Unknown)
		c->setDeclaredCult(faction);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(accessPowo, "give access to the powo", "<uid> [playername] [instance] [exit_pos] [can_xp,cant_dead,can_teleport,can_speedup,can_dp,onetry] [access_room_inv,access_guild_room] [scope]")
{
	if (args.size() < 2)
		return false;

	GET_ACTIVE_CHARACTER

	IBuildingPhysical *building;
	if (args.size() > 2)
		building = CBuildingManager::getInstance()->getBuildingPhysicalsByName(args[2]);
	else
		building = CBuildingManager::getInstance()->getBuildingPhysicalsByName("building_instance_ZO_player_111");

	string powoFlags = "000000";
	if (args.size() > 4 && args[4].length() == 6)
		powoFlags = args[4];

	string invFlags = "00";
	if (args.size() > 5 && args[5].length() == 2)
		invFlags = args[5];

	if (building)
	{
		nlinfo("Bulding: %s", building->getName().c_str());
		if (building->getTemplate()->Type == BUILDING_TYPES::Player)
		{
			CBuildingPhysicalPlayer *buildingPlayer = dynamic_cast<CBuildingPhysicalPlayer *>(building);

			CEntityId playerEid = CEntityIdTranslator::getInstance()->getByEntity(ucstring(args[1]));
			nlinfo("playerEid = %s", playerEid.toString().c_str());
			if (buildingPlayer && playerEid != CEntityId::Unknown)
			{
				CBuildingManager::getInstance()->removePlayerFromRoom(c, false);
				uint16 ownerId = buildingPlayer->getOwnerIdx(playerEid);
				nlinfo("ownerId = %d", ownerId);
				sint32 cell;
				if (buildingPlayer->addUser(c, 0, ownerId, cell, true, false))
				{
					nlinfo("Powo Flags : %s", powoFlags.c_str());
					c->setPowoCell(cell);
					if (args.size() > 6)
						c->setPowoScope(args[6]);

					c->setPowoFlag("xp", powoFlags[0] == '1');
					c->setPowoFlag("nodead", powoFlags[1] == '1');
					c->setPowoFlag("teleport", powoFlags[2] == '1');
					c->setPowoFlag("speed", powoFlags[3] == '1');
					c->setPowoFlag("dp", powoFlags[4] == '1');
					c->setPowoFlag("retry", powoFlags[5] == '1');

					c->setPowoFlag("room_inv", invFlags[0] == '1');
					c->setPowoFlag("guild_inv", invFlags[1] == '1');

					if (args.size () > 3 && args[3] != "*") // Change the default exit by exit of instance building
					{
						std::vector< std::string > pos;
						NLMISC::splitString(args[3], ",", pos);
						if (pos.size() > 2)
						{
							sint32 exitx;
							sint32 exity;
							fromString(pos[0], exitx);
							exitx *= 1000;
							fromString(pos[1], exity);
							exity *= 1000;
							if (pos[2] != "*")
							{
								sint32 exitcell;
								fromString(pos[2], exitcell);
								c->setBuildingExitPos(exitx, exity, exitcell);
							}
							else
								c->setBuildingExitPos(exitx, exity, cell);
						}
						else if (pos.size() > 1)
						{
							sint32 exitx;
							sint32 exity;
							fromString(pos[0], exitx);
							exitx *= 1000;
							fromString(pos[1], exity);
							exity *= 1000;
							c->setBuildingExitPos(exitx, exity, 0);
						}
						else
						{
							building = CBuildingManager::getInstance()->getBuildingPhysicalsByName(args[3]);
							if (building)
								c->setBuildingExitZone(building->getDefaultExitSpawn());
						}
					}

					log.displayNL("%d", cell);
				} else {
					log.displayNL("ERR: invalid cell");
					return false;
				}
			}
		} else {
			log.displayNL("ERR: invalid template");
			return true;
		}
	} else {
		log.displayNL("ERR: invalid building");
		return true;
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
	sint32 cell = c->getPowoCell();
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
NLMISC_COMMAND(getPlayersInPowos, "get list of players in a powo", "")
{
	CPlayerManager::TMapPlayers::const_iterator itPlayer = PlayerManager.getPlayers().begin();

	for (; itPlayer != PlayerManager.getPlayers().end(); ++itPlayer)
	{
		if ((*itPlayer).second.Player)
		{
			CCharacter * player = (*itPlayer).second.Player->getActiveCharacter();
			if (player)
			{
				sint32 powo = player->getPowoCell();
				if (powo != 0)
					log.displayNL("%d: %s", powo, player->getName().toString().c_str());
			}
		}
	}

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(kickPlayersFromPowo, "kick players from powo", "<player1,player2,...> <powo>")
{

	if (args.size () < 2)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	sint32 powo;
	fromString(args[1], powo);

	if (args[0] == "*")
	{
		for (CPlayerManager::TMapPlayers::const_iterator it = PlayerManager.getPlayers().begin(); it != PlayerManager.getPlayers().end(); ++it)
		{
			if ((*it).second.Player != 0)
			{
				CCharacter * player = (*it).second.Player->getActiveCharacter();
				if (player && player->getPowoCell() == powo)
				{
					CVector exitPos = player->getBuildingExitPos();
					if (exitPos.x != 0)
					{
						player->tpWanted(exitPos.x, exitPos.y, exitPos.z);
					}
					else
					{
						const CTpSpawnZone* zone = CZoneManager::getInstance().getTpSpawnZone(player->getBuildingExitZone());
						if (zone)
						{
							sint32 x, y, z;
							float heading;
							zone->getRandomPoint(x, y, z, heading);
							player->tpWanted(x, y, z, true, heading);
						}
					}
				}
			}
		}
	}
	else
	{
		std::vector< std::string > players;
		NLMISC::splitString(args[0], ",", players);

		for (uint32 i=0; i < players.size(); i++)
		{
			CCharacter * player = PlayerManager.getCharacterByName(players[i]);
			if (player && player->getPowoCell() == powo)
			{
				CVector exitPos = player->getBuildingExitPos();
				if (exitPos.x != 0)
				{
					player->tpWanted(exitPos.x, exitPos.y, exitPos.z);
				}
				else
				{
					const CTpSpawnZone* zone = CZoneManager::getInstance().getTpSpawnZone(player->getBuildingExitZone());
					if (zone)
					{
						sint32 x, y, z;
						float heading;
						zone->getRandomPoint(x, y, z, heading);
						player->tpWanted(x, y, z, true, heading);
					}
				}
			}
		}
	}

	return true;
}




//----------------------------------------------------------------------------
NLMISC_COMMAND(teleportMe, "teleport", "<uid> [x,y,z,h|player name|bot name] teleportMektoub? checks sameCell checkPowoFlag")
{
	if (args.size () < 2)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER


	// Checks : PvP Flag, PvP Tag, Sitting, Water, Mount, Fear, Sleep, Invu, Stun
	if (args.size () > 3)
	{
		bool pvpFlagValid = (c->getPvPRecentActionFlag() == false || c->getPVPFlag() == false);
		if (args[3][0] == '1' && !pvpFlagValid) {
			CCharacter::sendDynamicSystemMessage(c->getEntityRowId(), "PVP_TP_FORBIDEN");
			log.displayNL("ERR: PVP_FLAG");
			return false;
		}

		bool pvpTagValid =  c->getPVPFlag() == false;
		if (args[3].length() > 1 && args[3][1] == '1' && !pvpTagValid)
		{
			CCharacter::sendDynamicSystemMessage(c->getEntityRowId(), "PVP_TP_FORBIDEN");
			log.displayNL("ERR: PVP_TAG");
			return false;
		}

		if (args[3].length() > 2)
		{
			CBypassCheckFlags bypassCheckFlags;
			bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::WhileSitting, args[3].length() > 2 && args[3][2] == '0');
			bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::InWater, args[3].length() > 3 && args[3][3] == '0');
			bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::OnMount, args[3].length() > 4 && args[3][4] == '0');
			bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Fear, args[3].length() > 5 && args[3][5] == '0');
			bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Sleep, args[3].length() > 6 && args[3][6] == '0');
			bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Invulnerability, args[3].length() > 7 && args[3][7] == '0');
			bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Stun, args[3].length() > 8 && args[3][8] == '0');

			if (!c->canEntityUseAction(bypassCheckFlags, true))
			{
				if (!c->isDead() || (args[3].length() > 9 && args[3][9] == '1')) // Forbid if not dead or dead but not wanted
				{
				log.displayNL("ERR: OTHER_FLAG");
				return false;
				}
			}
		}
	}

	string value = args[1];

	vector<string> res;
	sint32 x = 0, y = 0, z = 0;
	float h = 0;
	sint32 cell;
	if (value.find(',') != string::npos) // Position x,y,z,a
	{
		explode (value, string(","), res);
		if (res.size() >= 2)
		{
			fromString(res[0], x);
			x *= 1000;
			fromString(res[1], y);
			y *= 1000;
		}
		if (res.size() >= 3)
		{
			fromString(res[2], z);
			z *= 1000;
		}
		if (res.size() >= 4)
			fromString(res[3], h);
	}
	else
	{
		if (value.find(".creature") != string::npos)
		{
			CSheetId creatureSheetId(value);
			if (creatureSheetId != CSheetId::Unknown)
			{
				double minDistance = -1.;
				CCreature * creature = NULL;

				TMapCreatures::const_iterator it;
				const TMapCreatures& creatures = CreatureManager.getCreature();
				for(it = creatures.begin(); it != creatures.end(); ++it)
				{
					CSheetId sheetId = (*it).second->getType();
					if (sheetId == creatureSheetId)
					{
						double distance = PHRASE_UTILITIES::getDistance(c->getEntityRowId(), (*it).second->getEntityRowId());
						if (!creature || distance < minDistance)
						{
							creature = (*it).second;
							minDistance = distance;
						}
					}
				}
				if (creature)
				{
					x = creature->getState().X();
					y = creature->getState().Y();
					z = creature->getState().Z();
					h = creature->getState().Heading();
				}
			}
			else
			{
				log.displayNL("ERR: INVALID_CREATURE");
			}
		}
		else
		{

			CEntityBase *entityBase = PlayerManager.getCharacterByName (CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), value));
			if (entityBase == NULL)
			{
				// try to find the bot name
				vector<TAIAlias> aliases;
				CAIAliasTranslator::getInstance()->getNPCAliasesFromName(value, aliases);
				if (aliases.empty())
				{
					log.displayNL("ERR: INVALID_BOT");
					return false;
				}

				TAIAlias alias = aliases[0];

				const CEntityId & botId = CAIAliasTranslator::getInstance()->getEntityId (alias);
				if (botId != CEntityId::Unknown)
				{
					entityBase = CreatureManager.getCreature (botId);
				}
				else
				{
					log.displayNL("ERR: BOT_NOT_SPAWNED");
					return false;
				}

			}
			if (entityBase != NULL)
			{
				x = entityBase->getState().X + sint32 (cos (entityBase->getState ().Heading) * 2000);
				y = entityBase->getState().Y + sint32 (sin (entityBase->getState ().Heading) * 2000);
				z = entityBase->getState().Z;
				h = entityBase->getState().Heading;

				TDataSetRow dsr = entityBase->getEntityRowId();
				CMirrorPropValueRO<TYPE_CELL> mirrorCell(TheDataset, dsr, DSPropertyCELL);
				cell = mirrorCell;
			}
		}
	}

	if (x == 0 && y == 0 && z == 0)
	{
		log.displayNL("ERR: INVALID_POSITION");
		return true;
	}

	CContinent * cont = CZoneManager::getInstance().getContinent(x,y);

	bool allowPetTp = false;
	if (args.size () > 2 && args[2] == "1")
		allowPetTp = true;

	if (allowPetTp)
		c->allowNearPetTp();
	else
		c->forbidNearPetTp();

	// Respawn player if dead
	if (c->isDead())
	{
		PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->playerRespawn(c);
		// apply respawn effects because user is dead
		c->applyRespawnEffects();
	}

	// Use same Cell
	if (args.size () > 4 && args[4] == "1")
	{
		TDataSetRow dsr = c->getEntityRowId();
		CMirrorPropValueRO<TYPE_CELL> mirrorCell(TheDataset, dsr, DSPropertyCELL);
		cell = mirrorCell;
	}

	// Check if PowoFlag canTeleport are true
	if (args.size () > 5 && args[5] == "1" && !c->getPowoFlag("teleport")) {
		log.displayNL("ERR: NO_POWO_FLAG");
		return true;
	}

	CMirrorPropValue<TYPE_VISUAL_FX> visualFx(TheDataset, c->getEntityRowId(), DSPropertyVISUAL_FX);
	CVisualFX fx;
	fx.unpack(visualFx.getValue());
	fx.Aura = MAGICFX::NoAura;
	sint64 prop;
	fx.pack(prop);
	visualFx = (sint16)prop;


	c->teleportCharacter(x,y,z,allowPetTp,true,h,0xFF,cell);

	if (cont)
	{
		c->getRespawnPoints().addDefaultRespawnPoint(CONTINENT::TContinent(cont->getId()));
	}

	// cancel any previous static action
	c->cancelStaticActionInProgress();

	log.displayNL("OK");

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setAuraFx, "setAuraFx", "<uid> aura")
{
	if (args.size() != 2)
		return false;

	GET_ACTIVE_CHARACTER

	CMirrorPropValue<TYPE_VISUAL_FX> visualFx(TheDataset, c->getEntityRowId(), DSPropertyVISUAL_FX);
	CVisualFX fx;
	fx.unpack(visualFx.getValue());

	if (args[1] == "marauder")
		fx.Aura = MAGICFX::TeleportMarauder;

	sint64 prop;
	fx.pack(prop);
	visualFx = (sint16)prop;
	return true;
}

//-----------------------------------------------
// Check Action Flags
//-----------------------------------------------
NLMISC_COMMAND(checkActionFlags,"Check Action Flags","<uid> [pvp_flag, pvp_tag, sitting, water, mount, fear, sleep, invu, stun]")
{
	if (args.size () != 2)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER
	// Checks : PvP Flag, PvP Tag, Sitting, Water, Mount, Fear, Sleep, Invu, Stun
	bool pvpFlagValid = (c->getPvPRecentActionFlag() == false || c->getPVPFlag() == false);
	if (args[1][0] == '1' && !pvpFlagValid)
	{
		CCharacter::sendDynamicSystemMessage(c->getEntityRowId(), "NO_ACTION_WHILE_PVP");
		log.displayNL("ERR: PVP_FLAG");
		return false;
	}

	bool pvpTagValid =  c->getPVPFlag() == false;
	if (args[1].length() > 1 && args[1][1] == '1' && !pvpTagValid)
	{
		CCharacter::sendDynamicSystemMessage(c->getEntityRowId(), "NO_ACTION_WHILE_PVP");
		log.displayNL("ERR: PVP_TAG");
		return false;
	}

	if (args[1].length() > 2)
	{
		CBypassCheckFlags bypassCheckFlags;
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::WhileSitting, args[1].length() > 2 && args[1][2] == '0');
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::InWater, args[1].length() > 3 && args[1][3] == '0');
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::OnMount, args[1].length() > 4 && args[1][4] == '0');
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Fear, args[1].length() > 5 && args[1][5] == '0');
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Sleep, args[1].length() > 6 && args[1][6] == '0');
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Invulnerability, args[1].length() > 7 && args[1][7] == '0');
		bypassCheckFlags.setFlag(CHECK_FLAG_TYPE::Stun, args[1].length() > 8 && args[1][8] == '0');

		if (!c->canEntityUseAction(bypassCheckFlags, true))
		{
			if (!c->isDead() || (args[1].length() > 9 && args[1][9] == '1')) // Forbid if not dead or dead but not wanted
			{
			log.displayNL("ERR: OTHER_FLAG");
			return false;
			}
		}
	}
	log.displayNL("OK");
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(setRespawn, "set respawn point for the player", "<uid> x y cell")
{
	if (args.size () < 4)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER

	sint32 x;
	sint32 y;
	uint32 cell;

	fromString(args[1], x);
	x *= 1000;

	fromString(args[2], y);
	y *= 1000;

	fromString(args[3], cell);

	c->getRespawnPoints().setArkRespawnpoint(x, y, cell);

	return true;
}

//-----------------------------------------------
// Add re-spawn point
//-----------------------------------------------
NLMISC_COMMAND(addRespawnPoint,"Add re-spawn point","<uid> <Re-spawn point name>")
{
	if (args.size () < 2)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER


	CCharacterRespawnPoints::TRespawnPoint respawnPoint = CZoneManager::getInstance().getTpSpawnZoneIdByName(args[1]);
	if (respawnPoint == InvalidSpawnZoneId)
		return false;

	c->getRespawnPoints().addRespawnPoint(respawnPoint);
	return true;
}

//-----------------------------------------------
// Respawn the player
//-----------------------------------------------
NLMISC_COMMAND(respawnPlayer,"Respawn the player at position","<uid> <withDp?> <x> <y> [<z> <heading>]")
{
	if (args.size() < 1)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER

	bool withDp = false;

	if (args.size() > 1)
		withDp = args[1] == "true" || args[1] == "1";

	sint32 x = c->getState().X;
	sint32 y = c->getState().Y;
	sint32 z = c->getState().Z;
	float h = c->getState().Heading;

	if (args.size() > 2)
	{
		fromString(args[2], x);
		x *= 1000;
	}

	if (args.size() > 3)
	{
		fromString(args[3], y);
		y *= 1000;
	}

	if (args.size() > 4)
	{
		fromString(args[4], z);
		z *= 1000;
	}

	if (args.size() > 5)
		fromString(args[5], h);

	c->respawn(x, y, z, h, withDp);
	return true;
}



//-----------------------------------------------
// Kill the player
//-----------------------------------------------
NLMISC_COMMAND(killPlayer,"Kill a player","<uid>")
{
	if (args.size () < 1)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER

	c->killMe();
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(spawn, "spawn entity", "<uid> quantity sheet dispersion spawnbot orientation groupname x y z look cell")
{

	if (args.size () < 12)
	{
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	CCharacter *c = NULL;

	bool isChar = false;
	if (args[0] != "*") {
		GET_ACTIVE_CHARACTER2
		isChar = true;
	}

	uint32 instanceNumber = 0;
	sint32 x = 0;
	sint32 y = 0;
	sint32 z = 0;
	if (isChar)
		z = c->getZ();
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

	fromString(args[3], dispersionRadius);
	if (dispersionRadius < 0.) {
		log.displayNL("ERR: invalid dispersion");
		return false;
	}

	bool spawnBots = true;
	fromString(args[4], spawnBots);

	if (isChar && args[5] == "self")
	{
		orientation = (sint32)(c->getHeading() * 1000.0);
	}
	else if (args[5] != "random")
	{
		NLMISC::fromString(args[5], orientation);
		orientation = (sint32)((double)orientation / 360.0 * (NLMISC::Pi * 2.0) * 1000.0);
	}

	string botsName = args[6];

	float userX;
	NLMISC::fromString(args[7], userX);
	x = (sint32)(userX * 1000.0);

	float userY;
	NLMISC::fromString(args[8], userY);
	y = (sint32)(userY * 1000.0);

	float userZ;
	if (args[9] != "*")
	{
		NLMISC::fromString(args[9], userZ);
		z = (sint32)(userZ * 1000.0);
	}

	string look;
	if (args[10] != "*")
	{
		look = args[10];
		if (look.find(".creature") == string::npos)
			look += ".creature";
	}

	if (isChar && args[11] == "*")
	{
		TDataSetRow dsr = c->getEntityRowId();
		CMirrorPropValueRO<TYPE_CELL> srcCell(TheDataset, dsr, DSPropertyCELL);
		cell = srcCell;
	}
	else
		NLMISC::fromString(args[11], cell);

	CContinent * continent = CZoneManager::getInstance().getContinent(x, y);

	if (!continent) {
		log.displayNL("ERR: invalid continent");
		return false;
	}

	uint32 aiInstance = CUsedContinent::instance().getInstanceForContinent((CONTINENT::TContinent)continent->getId());

	if (aiInstance == ~0)
	{
		log.displayNL("ERR: invalid continent");
		return false;
	}
	instanceNumber = aiInstance;

	CEntityId playerId;
	if (isChar)
		playerId = c->getId();

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


//----------------------------------------------------------------------------
NLMISC_COMMAND(grpScript, "executes a script on an event npc group", "<uid> <groupname> <script>")
{
	if (args.size () < 3) return false;

	uint32 instanceNumber = std::numeric_limits<uint32>::max();
	string playerEid = "";

	CCharacter *c = NULL;

	bool isChar = false;
	if (args[0] != "*") {
		GET_ACTIVE_CHARACTER2
		isChar = true;
		instanceNumber = c->getInstanceNumber();
		playerEid = c->getId().toString();
	}

	uint32 nbString = (uint32)args.size();

	string botsName = args[1];
	if (!getAIInstanceFromGroupName(botsName, instanceNumber) && instanceNumber == std::numeric_limits<uint32>::max())
	{
		log.displayNL("ERR: invalid instance");
		return false;
	}

	CMessage msgout("EVENT_NPC_GROUP_SCRIPT");
	uint32 messageVersion = 1;
	msgout.serial(messageVersion);
	msgout.serial(nbString);

	msgout.serial(playerEid);
	msgout.serial(botsName);
	for (uint32 i=2; i<nbString; ++i)
	{
		string arg = args[i]+";";

		size_t pos = 0;
		while((pos = arg.find("&nbsp&", pos)) != string::npos)
		{
			arg.replace(pos, 6, " ");
			pos ++;
		}
		pos = 0;
		while((pos = arg.find("__OR__", pos)) != string::npos)
		{
			arg.replace(pos, 6, "|");
			pos ++;
		}
		msgout.serial(arg);
	}
	CWorldInstances::instance().msgToAIInstance2(instanceNumber, msgout);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setUrl, "changes the url of a bot", "<uid> <groupname> [<url>] [<name>]")
{
	if (args.size () < 2) return false;

	GET_ACTIVE_CHARACTER

	uint32 instanceNumber = c->getInstanceNumber();

	string groupname = args[1];
	if (! getAIInstanceFromGroupName(groupname, instanceNumber))
	{
		log.displayNL("ERR: INVALID_AI_INSTANCE");
		return false;
	}


	// try to find the bot name
	vector<TAIAlias> aliases;

	log.displayNL("NAME: %s", groupname.c_str());
	CAIAliasTranslator::getInstance()->getNPCAliasesFromName(groupname, aliases);
	if (aliases.empty())
	{
		log.displayNL("ERR: INVALID_BOT");
		return false;
	}

	TAIAlias alias = aliases[0];

	const CEntityId & botId = CAIAliasTranslator::getInstance()->getEntityId (alias);
	if (botId != CEntityId::Unknown)
	{

		CCreature* creature = CreatureManager.getCreature(botId);

		uint32 program = creature->getBotChatProgram();
		if (!(program & (1<<BOTCHATTYPE::WebPageFlag)))
		{
			program |= 1 << BOTCHATTYPE::WebPageFlag;
			creature->setBotChatProgram(program);
		}

		const string &wp = creature->getWebPage();
		if (args.size() < 3)
		{
			(string &)wp = "";
			program &= ~(1 << BOTCHATTYPE::WebPageFlag);
			creature->setBotChatProgram(program);
		}
		else
		{
			(string &)wp = args[2];
			if (args.size() > 3)
			{
				const string &wpn = creature->getWebPageName();
				(string &)wpn = args[3];
			}
		}
	}
	else
	{
		log.displayNL("ERR: BOT_NOT_SPAWNED");
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(temporaryRename, "rename a player for the event", "<uid> <new name>")
{
	if (args.size() != 2) {
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER

	ucstring newName(args[1]);

	c->registerName(newName);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setTitle, "set player title", "<uid> <title>")
{
	if (args.size() != 2) {
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER

	TDataSetRow row = c->getEntityRowId();
	c->setNewTitle(args[1]);
	string fullname = c->getName().toString()+"$"+args[1]+"#"+c->getTagPvPA()+"#"+c->getTagPvPB()+"#"+c->getTagA()+"#"+c->getTagB()+"$";
	ucstring name;
	name.fromUtf8(fullname);
	nlinfo("Set title : %s", name.toUtf8().c_str());
	NLNET::CMessage	msgout("CHARACTER_NAME");
	msgout.serial(row);
	msgout.serial(name);
	sendMessageViaMirror("IOS", msgout);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setTag, "set player title", "<uid> <tag> <value>")
{
	if (args.size() != 3) {
		log.displayNL("ERR: invalid arg count");
		return false;
	}

	GET_ACTIVE_CHARACTER

	TDataSetRow row = c->getEntityRowId();
	if (args[1] == "pvpA") c->setTagPvPA(args[2]);
	if (args[1] == "pvpB") c->setTagPvPB(args[2]);
	if (args[1] == "A") c->setTagA(args[2]);
	if (args[1] == "B") c->setTagB(args[2]);
	string fullname = c->getName().toString()+"$"+c->getNewTitle()+"#"+c->getTagPvPA()+"#"+c->getTagPvPB()+"#"+c->getTagA()+"#"+c->getTagB()+"$";
	ucstring name;
	name.fromUtf8(fullname);
	NLNET::CMessage	msgout("CHARACTER_NAME");
	msgout.serial(row);
	msgout.serial(name);
	sendMessageViaMirror("IOS", msgout);
	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(getArkMissions,"dump character ark missions","<uid>")
{
	if (args.size() != 1)
		return false;

	GET_ACTIVE_CHARACTER

	string text;
	uint i = 0;
	for (map<TAIAlias, CMission*>::iterator it = c->getMissionsBegin(); it != c->getMissionsEnd(); ++it)
	{
		const string& name = CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId((*it).first);
		if (name.substr(0, 4) == "ark_")
			log.displayNL("%s", name.c_str());
	}

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(getPlayerStats,"get player stats","<uid> <stat1,stat2,stat3..>")
{

	if (args.size() <= 1)
		return false;

	GET_ACTIVE_CHARACTER

	std::vector< std::string > stats;
	NLMISC::splitString(args[1],",",stats);
	uint32 i=0;

	const CInventoryPtr & userBag = c->getInventory(INVENTORIES::bag);

	if (i < stats.size() && stats[i] == "wmal") // wear malus
	{
		log.displayNL("%f", c->wearMalus());
		i++;
	}

	if (i < stats.size() && stats[i] == "ibulk") // inventory bulk
	{
		log.displayNL("%d", userBag->getInventoryBulk());
		i++;
	}

	if (i < stats.size() && stats[i] == "mbulk") // inventory max bulk
	{
		log.displayNL("%d", userBag->getMaxBulk());
		i++;
	}

	if (i < stats.size() && stats[i] == "iwegt") // inventory weight
	{
		log.displayNL("%d", userBag->getInventoryWeight());
		i++;
	}

	if (i < stats.size() && stats[i] == "mwegt") // inventory max weight
	{
		log.displayNL("%d", userBag->getMaxWeight());
		i++;
	}

	if (i < stats.size() && stats[i] == "slots") // inventory nb slots
	{
		log.displayNL("%d", userBag->getUsedSlotCount());
		i++;
	}

	if (i < stats.size() && stats[i] == "powo") // powo cell
	{
		log.displayNL("%d", c->getPowoCell());
		i++;
	}

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(getServerStats,"get server stats","<uid> <stat1,stat2,stat3..> [<arg1>] [<arg2>]")
{

	if (args.size() <= 1)
		return false;

	CCharacter *c = NULL;

	if (args[0] != "*") {
		GET_ACTIVE_CHARACTER2
	}

	std::vector< std::string > stats;
	NLMISC::splitString(args[1],",",stats);
	uint32 i=0;

	for (i = 0; i < stats.size(); i++)
	{
		if (stats[i] == "time") // Atys time
			log.displayNL("%f", CTimeDateSeasonManager::getRyzomTimeReference().getRyzomTime ());
		else if (stats[i] == "date") // Atys date
			log.displayNL("%d", CTimeDateSeasonManager::getRyzomTimeReference().getRyzomDay ());
		else if (stats[i] == "season") // Atys season
			log.displayNL("%s", EGSPD::CSeason::toString(CTimeDateSeasonManager::getRyzomTimeReference().getRyzomSeason()).c_str());
		else if (stats[i] == "weather") // Atys weather
		{
			CVector pos;
			if (args.size() <= 2)
			{
				pos.x = c->getState().X / 1000.;
				pos.y = c->getState().Y / 1000.;
			}
			else
			{
				fromString(args[2], pos.x);
				fromString(args[3], pos.y);
			}
			pos.z = 0;
			CRyzomTime::EWeather weather = WeatherEverywhere.getWeather(pos, CTimeDateSeasonManager::getRyzomTimeReference());
			log.displayNL("%u", (uint)weather);
		}
	}

	return true;
}

//addCheckPos 530162 26140 -2436 5 test Prout

//-----------------------------------------------
NLMISC_COMMAND(addCheckPos,"add check pos","<uid> <x> <y> <radius> <mission_name> <use_compass>")
{
	if (args.size() != 6)
		return false;

	GET_ACTIVE_CHARACTER;

	sint32 x;
	sint32 y;
	uint32 r;
	fromString(args[1], x);
	fromString(args[2], y);
	fromString(args[3], r);

	c->addPositionCheck(x, y, r, args[4], args[5] == "1");

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(spawnArkMission,"spawn Mission","<uid> <bot_name> <mission_name>")
{
	if (args.size() < 3)
		return false;

	GET_ACTIVE_CHARACTER;

	vector<TAIAlias> aliases;
	CAIAliasTranslator::getInstance()->getNPCAliasesFromName(args[1], aliases);
	if (aliases.empty())
	{
		nldebug ("<spawn_mission> No NPC found matching name '%s'", args[1].c_str());
		return false;
	}

	TAIAlias giverAlias = aliases[0];
	TAIAlias missionAlias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName(args[2]);

	if (missionAlias == CAIAliasTranslator::Invalid)
	{
		nldebug ("<addMissionByName> No Mission found matching name '%s'", args[2].c_str());
		return false;
	}

	c->removeMission(missionAlias, 0, true);
	c->removeMissionFromHistories(missionAlias);

	c->endBotChat();

	std::list< CMissionEvent* > eventList;
	uint8 result = CMissionManager::getInstance()->instanciateMission(c, missionAlias, giverAlias, eventList);
	if (!result)
	{
	c->processMissionEventList(eventList,true, CAIAliasTranslator::Invalid);
		log.displayNL("OK");
	}
	else
		log.displayNL("ERR: %d", result);

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(removeArkMission,"remove Mission","<uid> <mission_name>")
{
	if (args.size() != 2)
		return false;

	GET_ACTIVE_CHARACTER;

	TAIAlias missionAlias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName(args[1]);
	c->removeMission(missionAlias, 0);
	c->removeMissionFromHistories(missionAlias);

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(finishArkMission,"finish Mission","<uid> <mission_name>")
{
	if (args.size() != 2)
		return false;

	GET_ACTIVE_CHARACTER;

	TAIAlias missionAlias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName(args[1]);
	c->removeMission(missionAlias, 0, true);
	c->removeMissionFromHistories(missionAlias);

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(resetArkMission,"reset Mission","<uid> <mission_name>")
{
	if (args.size() != 2)
		return false;

	GET_ACTIVE_CHARACTER;

	TAIAlias missionAlias = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName(args[1]);
	c->resetMissionSuccessfull(missionAlias);

	return true;
}


//-----------------------------------------------
NLMISC_COMMAND(setArkMissionText,"set Mission Text","<uid> <mission_name> <line1> <line2> <line3>..")
{
	if (args.size() < 3)
		return false;

	GET_ACTIVE_CHARACTER;

	uint32 nbString = (uint32)args.size();
	string text = getStringFromHash(args[2]);

	for (uint32 i=3; i<nbString; ++i)
		text +=  "\n"+getStringFromHash(args[i]);
	c->setCustomMissionParams(args[1], text);

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(delArkMissionParams,"del Mission Params","<uid> <mission_name>")
{
	if (args.size() != 2)
		return false;

	GET_ACTIVE_CHARACTER;

	c->setCustomMissionParams(args[1], "");

	return true;
}


//-----------------------------------------------
NLMISC_COMMAND(setArkMissionParams,"set Mission Params","<uid> <mission_name> <params> <app_callback> <callback_params>")
{
	if (args.size() != 5)
		return false;

	GET_ACTIVE_CHARACTER;

	c->setCustomMissionParams(args[1], args[3]+" "+args[4]+","+args[2]);

	return true;
}


//-----------------------------------------------
NLMISC_COMMAND(addArkMissionParams,"add Mission Params","<uid> <mission_name> <params>")
{
	if (args.size() != 3)
		return false;

	GET_ACTIVE_CHARACTER;

	c->addCustomMissionParam(args[1], args[2]);

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(getLastTpTick,"get tick of last teleport","<uid>")
{
	if (args.size() != 1)
		return false;

	GET_ACTIVE_CHARACTER;

	log.displayNL("%d", c->getLastTpTick());

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(getLastOverSpeedTick,"get tick of last over speed","<uid>")
{
	if (args.size() != 1)
		return false;

	GET_ACTIVE_CHARACTER;

	log.displayNL("%d", c->getLastOverSpeedTick());

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(getLastMountTick,"get tick of last mount","<uid>")
{
	if (args.size() != 1)
		return false;

	GET_ACTIVE_CHARACTER;

	log.displayNL("%d", c->getLastMountTick());

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(getLastUnMountTick,"get tick of last umount","<uid>")
{
	if (args.size() != 1)
		return false;

	GET_ACTIVE_CHARACTER;

	log.displayNL("%d", c->getLastUnMountTick());

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(getLastFreeMount,"get tick of last free mount","<uid>")
{
	if (args.size() != 1)
		return false;

	GET_ACTIVE_CHARACTER;

	log.displayNL("%d", c->getLastFreeMount());

	return true;
}

//-----------------------------------------------
NLMISC_COMMAND(getLastExchangeMount,"get tick of last exchange mount","<uid>")
{
	if (args.size() != 1)
		return false;

	GET_ACTIVE_CHARACTER;

	log.displayNL("%d", c->getLastExchangeMount());

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getPlayerVar, "get the value of a variable of player","<uid> <var>")
{
	if (args.size() != 2)
		return false;

	GET_ACTIVE_CHARACTER;

	string value = "";

	if (c->getValue("Base"+args[1], value))
		log.displayNL("%s", value.c_str());
	else
		log.displayNL("ERR: Variable not found");

	if (c->getValue("Max"+args[1], value))
		log.displayNL("%s", value.c_str());

	if (c->getValue("Current"+args[1], value))
		log.displayNL("%s", value.c_str());

	if (c->getValue("Modifier"+args[1], value))
		log.displayNL("%s", value.c_str());

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setPlayerVar, "set the value of a variable of player","<uid> <var> <value>")
{
	if (args.size() != 3)
		return false;

	GET_ACTIVE_CHARACTER;

	if (c->setValue(args[1], args[2]))
		log.displayNL("OK");
	else
		log.displayNL("ERR: Variable not found");

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(addPlayerVar, "add to the value of a variable of player","<uid> <var> <value>")
{
	if (args.size() != 3)
		return false;

	GET_ACTIVE_CHARACTER;

	if (c->modifyValue(args[1], args[2]))
		log.displayNL("OK");
	else
		log.displayNL("ERR: Variable not found");

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getTeam, "get the team of a player","<uid>")
{
	if (args.size() != 1)
		return false;

	GET_ACTIVE_CHARACTER;

	CTeam* pTeam = TeamManager.getRealTeam(c->getTeamId());
	if (pTeam != NULL)
	{
		log.displayNL("%d", c->getTeamId());
		for (list<CEntityId>::const_iterator it = pTeam->getTeamMembers().begin(); it != pTeam->getTeamMembers().end(); ++it)
		{
			ucstring name = CEntityIdTranslator::getInstance()->getByEntity((*it));
			CEntityIdTranslator::removeShardFromName(name);
			log.displayNL("%" NL_I64 "u|%s", (*it).asUint64(), name.toUtf8().c_str());
		}
	} else
		log.displayNL("-1");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setTrigger, "set a custom trigger", "<trigger> [<web_app>] [<args>]")
{
	if (args.size() < 1)
		return false;

	sint32 triggerId;
	fromString(args[0], triggerId);

	if (args.size() == 3)
		CBuildingManager::getInstance()->setCustomTrigger(triggerId, args[1]+" "+args[2]);
	else
		CBuildingManager::getInstance()->setCustomTrigger(triggerId, "");
	log.displayNL("OK");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(muteUser, "mute a user", "<player name> <duration> [<universe>?]")
{
	if (args.size() < 2)
		return false;

	CCharacter * target = PlayerManager.getCharacterByName(args[0]);
	if (!target || !TheDataset.isAccessible(target->getEntityRowId()))
	{
		log.displayNL("ERR: user not found");
		return true;
	}

	uint32 duration;
	fromString(args[1], duration);
	TGameCycle cycle = (NLMISC::TGameCycle)(duration / CTickEventHandler::getGameTimeStep() + CTickEventHandler::getGameCycle());
	if (args.size() == 3)
		PlayerManager.muteUniverse(CEntityId::Unknown, cycle, target->getId());
	else
		PlayerManager.addGMMute(CEntityId::Unknown, target->getId(), cycle);
	log.displayNL("OK");
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(sendMessageToUser, "send a message to a user", "<player name> <message>")
{
	if (args.size() != 2)
		return false;

	CCharacter * target = PlayerManager.getCharacterByName(args[0]);
	if (!target || !TheDataset.isAccessible(target->getEntityRowId()))
	{
		log.displayNL("ERR: user not found");
		return true;
	}
	SM_STATIC_PARAMS_1(params,STRING_MANAGER::literal);
	params[0].Literal = args[1];

	CCharacter::sendDynamicSystemMessage(target->getId(), "LITERAL", params);
	log.displayNL("OK");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(sendUrlToUser, "send an url to a user", "<player name> <app> <params>")
{
	if (args.size() != 3)
		return false;

	CCharacter * target = PlayerManager.getCharacterByName(args[0]);
	if (!target || !TheDataset.isAccessible(target->getEntityRowId()))
	{
		log.displayNL("ERR: user not found");
		return true;
	}

	target->sendUrl(args[1]+" "+args[2]);
	log.displayNL("OK");
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(setGuildPoints, "get/set the guild points", "<uid> <value>")
{
	GET_ACTIVE_CHARACTER

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId(c->getGuildId());
	if (guild)
	{
		uint32 points = guild->getXP();

		if (args.size() == 2)
		{
			string quant = args[1];
			uint32 quantity;
			if (quant[0] == '+')
			{
				if (quant.size() > 1)
				{
					fromString(quant.substr(1), quantity);
					points += quantity;
				}
			}
			else if (quant[0] == '-')
			{
				if (quant.size() > 1)
				{
					fromString(quant.substr(1), quantity);
					if (points >= quantity)
					{
						points -= quantity;
					}
					else
					{
						log.displayNL("ERR: not enough"); // No enough points
						return true;
					}
				}
			}
			else
			{
				fromString(quant, points);
			}

			guild->setPoints(points);
		}

		log.displayNL("%u", points);
	} else {
		log.displayNL("ERR: no guild");
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(resetTodayGuildPoints, "reset the today guild points", "<uid>")
{
	GET_ACTIVE_CHARACTER
	c->resetTodayGuildPoints();

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(addPlayerPet, "add a pet to player", "<uid> <sheetid> [size] [name]")
{
	if (args.size() < 2)
		return false;

	GET_ACTIVE_CHARACTER

	CSheetId ticket = CSheetId(args[1]);

	uint8 size = 100;
	if (args.size() >= 3)
		fromString(args[2], size);

	ucstring customName;
	if (args.size() >= 4)
		customName.fromUtf8(args[3]);

	if (ticket != CSheetId::Unknown)
	{
		CGameItemPtr item = c->createItemInInventoryFreeSlot(INVENTORIES::bag, 1, 1, ticket);
		if (item != 0)
		{
			if (! c->addCharacterAnimal(ticket, 0, item, size, customName))
			{
				item.deleteItem();
				log.displayNL("ERR: CAN'T ADD ANIMAL");
				return true;
			}
			log.displayNL("OK");
			return true;
		}

		log.displayNL("ERR: CAN'T CREATE TICKET");
		return true;
	}

	log.displayNL("ERR: CAN'T FOUND VALID TICKET");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setPlayerPetSheetid, "change the sheetid of a player pet", "<uid> <index> <sheetid> [<posx>] [<posy>]")
{
	if (args.size() < 3)
		return false;

	GET_ACTIVE_CHARACTER

	uint8 index;
	fromString(args[1], index);
	CSheetId sheet = CSheetId(args[2].c_str());
	if (sheet != CSheetId::Unknown) {
		c->removeAnimalIndex(index, CPetCommandMsg::DESPAWN);
		c->setAnimalSheetId(index, sheet);

		if (args.size() == 5)
		{
			sint32 x;
			sint32 y;
			fromString(args[3], x);
			fromString(args[4], y);
			c->setAnimalPosition(index, x, y);
		}

		if (!c->spawnCharacterAnimal(index))
		{
			log.displayNL("ERR: invalid spawn");
			return true;
		}
	}
	else
	{
		log.displayNL("ERR: invalid sheet");
		return true;
	}

	log.displayNL("OK");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getPlayerPets, "get player pets", "<uid>")
{
	GET_ACTIVE_CHARACTER

	string pets = c->getPets();

	log.displayNL("%s", pets.c_str());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getPlayerPetsInfos, "get player pets infos", "<uid>")
{
	GET_ACTIVE_CHARACTER

	string pets = c->getPetsInfos();

	log.displayNL("%s", pets.c_str());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(spawnPlayerPet, "spawn player pet", "<uid> <slot>")
{
	if (args.size() < 2)
		return false;

	GET_ACTIVE_CHARACTER

	uint32 index;
	fromString(args[1], index);

	c->setPetStatus(index, CPetAnimal::waiting_spawn);
	c->updateOnePetDatabase(index, false);
	c->removeAnimalIndex(index, CPetCommandMsg::DESPAWN);
	c->setAnimalPosition(index, c->getState().X, c->getState().Y);
	if (!c->spawnCharacterAnimal(index))
		log.displayNL("ERR: invalid spawn");
	else
		log.displayNL("OK");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(removePlayerPet, "put player pet", "<uid> <slot> [<keepInventory=0|1>]")
{
	if (args.size() < 2)
		return false;

	GET_ACTIVE_CHARACTER

	uint32 index;
	fromString(args[1], index);

	bool keepInventory =  args.size() > 2 && args[2] == "1";

	c->removeAnimalIndex(index, CPetCommandMsg::LIBERATE, keepInventory);
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(setPlayerPetName, "change the name of a player pet", "<uid> <index> <name>")
{
	if (args.size() != 3)
		return false;

	GET_ACTIVE_CHARACTER
	uint8 index;
	fromString(args[1], index);
	ucstring customName;
	if (args[2] != "-")
		customName.fromUtf8(args[2]);
	else
		customName = "";
	c->setAnimalName(index, customName);
	log.displayNL("OK");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setPlayerPetTitle, "change the name of a player pet", "<uid> <index> <title>")
{
	if (args.size() != 3)
		return false;

	GET_ACTIVE_CHARACTER
	uint8 index;
	fromString(args[1], index);
	string title;
	if (args[2] != "-")
		title = args[2];
	else
		title = "";

	c->setAnimalTitle(index, title);
	log.displayNL("OK");
	return true;
}

//setPlayerVisual 530162 haircut fy_hof_hair_basic02.sitem
//----------------------------------------------------------------------------
NLMISC_COMMAND(setPlayerVisual, "get visual of a player", "<uid> <visual_prop1>[,<visual_prop1>,...] <arg1>[,<arg2>,...]")
{
	if (args.size() < 2)
		return false;

	GET_ACTIVE_CHARACTER;

	std::vector< std::string > props;
	NLMISC::splitString(args[1], ",", props);

	std::vector< std::string > prop_args;
	if (args.size() == 3)
		NLMISC::splitString(args[2], ",", prop_args);


	uint32 i=0;

	for (i = 0; i < props.size(); i++)
	{
		if (props[i] == "haircut" || props[i] == "wig")
		{
			if (args.size() == 3)
			{
				CSheetId sheetId(prop_args[i]);
				if (sheetId == CSheetId::Unknown)
				{
					log.displayNL("ERR: sheet unknown '%s'", sheetId.toString().c_str());
					return true;
				}

				uint32 hairValue = CVisualSlotManager::getInstance()->sheet2Index(sheetId, SLOTTYPE::HEAD_SLOT);
				if (!c->setHair(hairValue, props[i] == "wig", false))
				{
					log.displayNL("ERR: same haircut");
					continue;
				}
				c->resetHairCutDiscount();
			}
			else
			{
				uint8 haircut = c->getHair();
				CSheetId *sheet = CVisualSlotManager::getInstance()->index2Sheet(haircut, SLOTTYPE::HEAD_SLOT);
				bool isWig = c->getUseWig();
				if (sheet)
				{
					if (isWig)
						log.displayNL("W %s", sheet->toString().c_str());
					else
						log.displayNL("H %s", sheet->toString().c_str());
				}
				else
					log.displayNL("ERR: no haircut");
			}
		}
		else if (props[i] == "haircolor" || props[i] == "force_haircolor")
		{
			if (args.size() == 3)
			{
				uint32 color;
				fromString(prop_args[i], color);

				bool isWig = c->getUseWig();
				if (props[i] == "force_haircolor") // If force_haircolor the color will be applyed even player use a wig. To do that, remove useWig state and reapply it after
					c->setUseWig(false);

				if (!c->setHairColor(color, false))
					log.displayNL("ERR: same color");

				if (props[i] == "wigcolor")
					c->setUseWig(isWig);
			}
			else
			{
				uint32 haircolor = c->getHairColor();
				log.displayNL("%u", haircolor);
			}
		}
	}

	log.displayNL("OK");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(scaleEntity, "change the size of an entity", "<uid> <eid> <scale>")
{
	if (args.size() != 3)
		return false;

	GET_ACTIVE_CHARACTER;

	CEntityId entityId(args[1]);

	if (entityId == CEntityId::Unknown)
	{
		log.displayNL("ERR: invalid eid");
		return true;
	}

	TDataSetRow row = TheDataset.getDataSetRow(entityId);

	uint32 scale;
	fromString(args[2], scale);

	if (scale>255)
		scale = 0;

	CMirrorPropValue< SAltLookProp2, CPropLocationPacked<2> > visualPropertyB(TheDataset, row, DSPropertyVPB);
	SET_STRUCT_MEMBER(visualPropertyB, PropertySubData.Scale, scale);

	log.displayNL("OK");
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setPlayerPetSize, "change the size of a player pet", "<uid> <index> <size>")
{
	if (args.size() != 3)
		return false;

	GET_ACTIVE_CHARACTER
	uint8 index;
	fromString(args[1], index);
	uint8 size;
	fromString(args[2], size);
	c->removeAnimalIndex(index, CPetCommandMsg::DESPAWN);
	c->setAnimalSize(index, size);
	c->spawnCharacterAnimal(index);
	log.displayNL("OK");
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(setVpx, "change/get the vpx of a player", "<uid> <[vpx1,vpx2,vpx3,...]> <[value1,value2,vlaue3,...]>")
{
	if (args.size() < 2)
		return false;

	GET_ACTIVE_CHARACTER

	std::vector< std::string > vpx;
	NLMISC::splitString(args[1], ",", vpx);

	string ret = "";
	if (args.size() == 2)
	{ // get the values
		for (uint32 i=0; i<vpx.size(); i++)
		{
			string name = vpx[i];
			uint32 value = 0;
			if (name == "Sex")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.Sex;
			else if (name == "HatModel")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.HatModel;
			else if (name == "HatColor")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.HatColor;
			else if (name == "JacketModel")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.JacketModel;
			else if (name == "JacketColor")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.JacketColor;
			else if (name == "TrouserModel")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.TrouserModel;
			else if (name == "TrouserColor")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.TrouserColor;
			else if (name == "WeaponRightHand")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.WeaponRightHand;
			else if (name == "WeaponLeftHand")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.WeaponLeftHand;
			else if (name == "ArmModel")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.ArmModel;
			else if (name == "ArmColor")
				value = c->getVisualPropertyA().directAccessForStructMembers().PropertySubData.ArmColor;
			else if (name == "HandsModel")
				value = c->getVisualPropertyB().directAccessForStructMembers().PropertySubData.HandsModel;
			else if (name == "HandsColor")
				value = c->getVisualPropertyB().directAccessForStructMembers().PropertySubData.HandsColor;
			else if (name == "FeetModel")
				value = c->getVisualPropertyB().directAccessForStructMembers().PropertySubData.FeetModel;
			else if (name == "FeetColor")
				value = c->getVisualPropertyB().directAccessForStructMembers().PropertySubData.FeetColor;
			else if (name == "MorphTarget1")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.MorphTarget1;
			else if (name == "MorphTarget2")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.MorphTarget2;
			else if (name == "MorphTarget3")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.MorphTarget3;
			else if (name == "MorphTarget4")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.MorphTarget4;
			else if (name == "MorphTarget5")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.MorphTarget5;
			else if (name == "MorphTarget6")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.MorphTarget6;
			else if (name == "MorphTarget7")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.MorphTarget7;
			else if (name == "MorphTarget8")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.MorphTarget8;
			else if (name == "EyesColor")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.EyesColor;
			else if (name == "Tattoo")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.Tattoo;
			else if (name == "CharacterHeight")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.CharacterHeight;
			else if (name == "TorsoWidth")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.TorsoWidth;
			else if (name == "ArmsWidth")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.ArmsWidth;
			else if (name == "LegsWidth")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.LegsWidth;
			else if (name == "BreastSize")
				value = c->getVisualPropertyC().directAccessForStructMembers().PropertySubData.BreastSize;
			ret += toString("%d,", value);
		}
	}
	else // set the values
	{
		std::vector< std::string > values;
		NLMISC::splitString(args[2], ",", values);

		if (values.size() != vpx.size())
			return false;

		for (uint32 i=0; i<vpx.size(); i++)
		{
			string name = vpx[i];
			uint32 value;
			fromString(values[i], value);

			if (name == "Sex")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.Sex, value);
			}
			else if (name == "HatModel")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.HatModel, value);
			}
			else if (name == "HatColor")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.HatColor, value);
			}
			else if (name == "JacketModel")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.JacketModel, value);
			}
			else if (name == "JacketColor")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.JacketColor, value);
			}
			else if (name == "TrouserModel")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.TrouserModel, value);
			}
			else if (name == "TrouserColor")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.TrouserColor, value);
			}
			else if (name == "WeaponRightHand")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.WeaponRightHand, value);
			}
			else if (name == "WeaponLeftHand")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.WeaponLeftHand, value);
			}
			else if (name == "ArmModel")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.ArmModel, value);
			}
			else if (name == "ArmColor")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyA(), PropertySubData.ArmColor, value);
			}
			else if (name == "HandsModel")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyB(), PropertySubData.HandsModel, value);
			}
			else if (name == "HandsColor")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyB(), PropertySubData.HandsColor, value);
			}
			else if (name == "FeetModel")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyB(), PropertySubData.FeetModel, value);
			}
			else if (name == "FeetColor")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyB(), PropertySubData.FeetColor, value);
			}
			else if (name == "MorphTarget1")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.MorphTarget1, value);
			}
			else if (name == "MorphTarget2")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.MorphTarget2, value);
			}
			else if (name == "MorphTarget3")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.MorphTarget3, value);
			}
			else if (name == "MorphTarget4")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.MorphTarget4, value);
			}
			else if (name == "MorphTarget5")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.MorphTarget5, value);
			}
			else if (name == "MorphTarget6")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.MorphTarget6, value);
			}
			else if (name == "MorphTarget7")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.MorphTarget7, value);
			}
			else if (name == "MorphTarget8")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.MorphTarget8, value);
			}
			else if (name == "EyesColor")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.EyesColor, value);
			}
			else if (name == "Tattoo")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.Tattoo, value);
			}
			else if (name == "CharacterHeight")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.CharacterHeight, value);
			}
			else if (name == "TorsoWidth")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.TorsoWidth, value);
			}
			else if (name == "ArmsWidth")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.ArmsWidth, value);
			}
			else if (name == "LegsWidth")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.LegsWidth, value);
			}
			else if (name == "BreastSize")
			{
				SET_STRUCT_MEMBER(c->getVisualPropertyC(), PropertySubData.BreastSize, value);
			}
		}
	}

	if (!ret.empty())
		log.displayNL("%s", ret.c_str());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getPlayerGuild, "get player guild informations", "<uid>")
{
	GET_ACTIVE_CHARACTER

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId(c->getGuildId());
	if (guild)
	{
		CGuildMember* member = guild->getMemberFromEId(c->getId());

		if (member)
		{
			if (member->getGrade() == EGSPD::CGuildGrade::Leader)
				log.displayNL("Leader");
			else if (member->getGrade() == EGSPD::CGuildGrade::HighOfficer)
				log.displayNL("HighOfficer");
			else if (member->getGrade() == EGSPD::CGuildGrade::Officer)
				log.displayNL("Officer");
			else
				log.displayNL("Member");

			log.displayNL("%d", c->getGuildId());
			log.displayNL("%s", guild->getName().toString().c_str());
			CGuild::TAllegiances allegiance = guild->getAllegiance();
			log.displayNL("%s", PVP_CLAN::toString(allegiance.first).c_str());
			log.displayNL("%s", PVP_CLAN::toString(allegiance.second).c_str());
			return true;
		}
	}

	log.displayNL("NoGuild");
	return true;
}

NLMISC_COMMAND(addXp, "Gain experience in a given skills", "<uid> <xp> <skill> [<count>]")
{
	if (args.size() < 3) return false;

	GET_ACTIVE_CHARACTER

	uint32 xp;
	NLMISC::fromString(args[1], xp);

	string skill = args[2];

	uint count;
	if (args.size()==3)
		count = 1;
	else
		NLMISC::fromString(args[3], count);

	count = min(count, (uint)100);

	uint i;
	for (i=0; i<count; ++i)
		c->addXpToSkill((double)xp, skill, true);

	return true;
}

NLMISC_COMMAND(removeDp, "Update the DP", "<uid> <dp>")
{
	if (args.size() < 2) return false;

	GET_ACTIVE_CHARACTER


	double dpToGain = c->getDeathPenalties().getDeathXPToGain();
	log.displayNL("%d", dpToGain);

	uint32 remove;
	NLMISC::fromString(args[1], remove);

	if (remove <= 100 && remove >0)
	{
		dpToGain = remove * (dpToGain / 100);
		c->getDeathPenalties().addDeathXP(c, dpToGain);
	}
	log.displayNL("%d", dpToGain);

	return true;
}


NLMISC_COMMAND(addBricks, "Specified player learns given brick", "<uid> <brick1,brick2>")
{
	if (args.size() != 2) return false;
	GET_ACTIVE_CHARACTER

	std::vector< std::string > bricks;
	NLMISC::splitString(args[1], ",", bricks);
	for (uint32 i=0; i<bricks.size(); i++)
	{
		CSheetId brickId(bricks[i]);
		c->addKnownBrick(brickId);
	}
	return true;
}


NLMISC_COMMAND(delBrick, "Specified player unlearns given brick", "<uid> <brick1>")
{
	if (args.size() != 2) return false;
	GET_ACTIVE_CHARACTER

	CSheetId brickId(args[1]);
	c->removeKnownBrick(brickId);

	return true;
}


NLMISC_COMMAND(execAiAction, "Exec Ai Action", "<uid> <brick1> <target?>")
{
	if (args.size() < 2) return false;

	GET_ACTIVE_CHARACTER

	CSheetId ActionId(args[1]);
	TDataSetRow TargetRowId;

	if (ActionId == CSheetId::Unknown)
	{
		log.displayNL("ERR: sheetId is Unknown");
		return true;
	}

	if (args.size() > 2)
	{
		const CEntityId &target = c->getTarget();

		string error;
		if (target == CEntityId::Unknown)
			error = "unknown";
		else if (target.getType() == RYZOMID::creature && args[2] != "creature")
			error = "not a creature";
		else if (target.getType() == RYZOMID::npc && args[2] != "npc")
			error = "not a npc";
		else if (target.getType() == RYZOMID::player && args[2] != "player")
			error = "not a player";

		if (!error.empty())
		{
			log.displayNL("ERR: target %s", error.c_str());
			return true;
		}

		TargetRowId = TheDataset.getDataSetRow(target);
	}
	else
	{
		TargetRowId = c->getEntityRowId();
	}
		CPhraseManager::getInstance().executeAiAction(c->getEntityRowId(), TargetRowId, ActionId);

	return true;
}


//spawnToxic 530162 18905 -24318 water_bomb.fx 2 -100 focus 4 4
NLMISC_COMMAND(spawnToxic, "Spawn a toxic cloud", "<uid> <posX> <posY> <fx> <Radius=1> <dmgPerHit=0> <affectedScore=hit_points> <updateFrequency=ToxicCloudUpdateFrequency> <lifetimeInTicks=ToxicCloudDefaultLifetime>")
{
	if ( args.size() < 1 )
		return false;

	GET_ACTIVE_CHARACTER

	float x = (float)c->getX() / 1000.f;
	float y = (float)c->getY() / 1000.f;

	if (args.size() > 1)
		NLMISC::fromString(args[1], x);

	if (args.size() > 2)
		NLMISC::fromString(args[2], y);

	string fx = "toxic_cloud_1.fx";
	if (args.size() > 3)
		fx = args[3];

	CVector cloudPos( x, y, 0.0f );
	float radius = 1.f;
	sint32 dmgPerHit = 100;
	TGameCycle updateFrequency = ToxicCloudUpdateFrequency;
	TGameCycle lifetime = CToxicCloud::ToxicCloudDefaultLifetime;

	SCORES::TScores affectedScore = SCORES::hit_points;

	if (args.size() > 4)
	{
		NLMISC::fromString(args[4], radius);
		if (args.size() > 5)
		{
			NLMISC::fromString(args[5], dmgPerHit);
			if (args.size() > 6)
			{
				affectedScore = SCORES::toScore(args[6]);

				if (args.size() > 7)
				{
					NLMISC::fromString(args[7], updateFrequency);
					if (args.size() > 8)
					{
						NLMISC::fromString(args[8], lifetime);
					}
				}
			}
		}
	}

	CToxicCloud *tc = new CToxicCloud();
	tc->init(cloudPos, radius, dmgPerHit, updateFrequency, lifetime, affectedScore);

	CSheetId sheet(fx);

	if (tc->spawn(sheet))
	{
		CEnvironmentalEffectManager::getInstance()->addEntity(tc);
		log.displayNL("OK");
	}
	else
	{
		log.displayNL("ERR");
	}
	return true;
}



NLMISC_COMMAND(searchEntity, "Search an Entity (Player, Creature or Npc)", "<uid> <type=creature|bot|race|player> <name> [<all_levels?>]")
{

	if ( args.size() < 3 )
		return false;

	GET_ACTIVE_CHARACTER

	float x = (float)c->getX() / 1000.f;
	float y = (float)c->getY() / 1000.f;

	if ( args[1] == "creature" )
	{
		CSheetId creatureSheetId1;
		CSheetId creatureSheetId2;
		CSheetId creatureSheetId3;
		CSheetId creatureSheetId4;
		if ( args.size() > 3 && args[3] == "1")
		{
			creatureSheetId1 = CSheetId(args[2]+"1.creature");
			creatureSheetId2 = CSheetId(args[2]+"2.creature");
			creatureSheetId3 = CSheetId(args[2]+"3.creature");
			creatureSheetId4 = CSheetId(args[2]+"4.creature");
		}
		else
		{
			creatureSheetId1 = CSheetId(args[2]);
		}

		if( creatureSheetId1 != CSheetId::Unknown )
		{
			double minDistance = -1.;
			CCreature * creature = NULL;

			TMapCreatures::const_iterator it;
			const TMapCreatures& creatures = CreatureManager.getCreature();
			nlinfo("creature size : %d", creatures.size());
			if( creatureSheetId2 != CSheetId::Unknown )
			{
				for( it = creatures.begin(); it != creatures.end(); ++it )
				{
					CSheetId sheetId = (*it).second->getType();

					if( sheetId == creatureSheetId1 || creatureSheetId2 == creatureSheetId1 || creatureSheetId3 == creatureSheetId1 || creatureSheetId4 == creatureSheetId1 )
					{
						double distance = PHRASE_UTILITIES::getDistance( c->getEntityRowId(), (*it).second->getEntityRowId() );
						if( !creature || (creature && distance < minDistance) )
						{
							creature = (*it).second;
							minDistance = distance;
						}
					}
				}
			}
			else
			{
				for( it = creatures.begin(); it != creatures.end(); ++it )
				{
					CSheetId sheetId = (*it).second->getType();

					if( sheetId == creatureSheetId1 )
					{
						double distance = PHRASE_UTILITIES::getDistance( c->getEntityRowId(), (*it).second->getEntityRowId() );
						if( !creature || (creature && distance < minDistance) )
						{
							creature = (*it).second;
							minDistance = distance;
						}
					}
				}
			}

			if( creature )
			{
				float fx = 0, fy = 0, fz = 0;
				fx = creature->getState().X() / 1000.0f;
				fy = creature->getState().Y() / 1000.0f;
				fz = creature->getState().Z() / 1000.0f;
				log.displayNL("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", fx, fy, fz, minDistance, x-fx, y-fy);
				return true;
			}
		}
		else
		{
			log.displayNL("ERR: sheet not found");
		}
	}
	else if ( args[1] == "race" )
	{
		EGSPD::CPeople::TPeople race;
		race = EGSPD::CPeople::fromString(args[2]);
		if ( race != EGSPD::CPeople::EndPeople )
		{
			double minDistance = -1.;
			CCreature * creature = NULL;

			TMapCreatures::const_iterator it;
			const TMapCreatures& creatures = CreatureManager.getCreature();
			for( it = creatures.begin(); it != creatures.end(); ++it )
			{
				if( race == (*it).second->getRace() )
				{
					double distance = PHRASE_UTILITIES::getDistance( c->getEntityRowId(), (*it).second->getEntityRowId() );
					if( !creature || (creature && distance < minDistance) )
					{
						creature = (*it).second;
						minDistance = distance;
					}
				}
			}

			if( creature )
			{
				float fx = 0, fy = 0, fz = 0;
				fx = creature->getState().X() / 1000.0f;
				fy = creature->getState().Y() / 1000.0f;
				fz = creature->getState().Z() / 1000.0f;
				log.displayNL("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", fx, fy, fz, minDistance, x-fx, y-fy);
				return true;
			}
		}
		else
		{
			log.displayNL("ERR: race not found");
		}
	}
	else if ( args[1] == "player" )
	{
		CEntityBase *entityBase = PlayerManager.getCharacterByName(CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), args[2]));
		if (entityBase != NULL)
		{
			double minDistance = PHRASE_UTILITIES::getDistance( c->getEntityRowId(), entityBase->getEntityRowId() );
			float fx = 0, fy = 0, fz = 0;
			fx = entityBase->getState().X / 1000.0f;
			fy = entityBase->getState().Y / 1000.0f;
			fz = entityBase->getState().Z / 1000.0f;
			log.displayNL("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", fx, fy, fz, minDistance, x-fx, y-fy);
			return true;
		}
		else
		{
			log.displayNL("ERR: player not found");
		}
	} else {
		// try to find the bot name
		vector<TAIAlias> aliases;
		CAIAliasTranslator::getInstance()->getNPCAliasesFromName( args[2], aliases );
		if ( !aliases.empty() )
		{
			TAIAlias alias = aliases[0];
			const CEntityId & botId = CAIAliasTranslator::getInstance()->getEntityId(alias);
			if ( botId != CEntityId::Unknown )
			{
				CEntityBase *entityBase = CreatureManager.getCreature(botId);
				if (entityBase != NULL)
				{
					double minDistance = PHRASE_UTILITIES::getDistance( c->getEntityRowId(), entityBase->getEntityRowId() );
					float fx = 0, fy = 0, fz = 0;
					fx = entityBase->getState().X / 1000.0f;
					fy = entityBase->getState().Y / 1000.0f;
					fz = entityBase->getState().Z / 1000.0f;
					log.displayNL("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", fx, fy, fz, minDistance, x-fx, y-fy);
					return true;
				}
			}
		}
		else
		{
			log.displayNL("ERR: bot not found");
		}
	}
	log.displayNL("0,0,0,0,0,0");
	return true;
}

NLMISC_COMMAND(setBehaviour," change entity behaviour","<uid|*> <behaviour> [<target|eid>]")
{
	if ( args.size() < 2 )
		return false;

	CEntityId id;
	CEntityBase *e = NULL;

	bool isChar = false;
	if (args[0] != "*")
	{
		CCharacter *c = NULL;
		GET_ACTIVE_CHARACTER2

		if ( args.size() > 2 && args[2] == "target")
		{
			id = c->getTarget();
			if( id.getType() == 0 )
			{
				CCharacter *c = PlayerManager.getChar(id);
				if (c && c->getEnterFlag())
					e = c;
			}
			else
			{
				e = CreatureManager.getCreature(id);
			}
		}
		else
			e = c;
	}
	else
	{
		if ( args.size() < 3 )
			return false;

		id.fromString( args[2].c_str() );
		e = CreatureManager.getCreature(id);
	}

	if (e)
	{
		sint behav;
		NLMISC::fromString(args[1], behav);
		MBEHAV::EBehaviour behaviour = MBEHAV::EBehaviour(behav);
		e->setBehaviour( behaviour );
		log.displayNL("%s", toString( e->getBehaviour() ).c_str() );
	}
	else
	{
		log.displayNL("ERR: entity not found");
	}

	return true;
}

NLMISC_COMMAND(getBehaviour," get entity behaviour","<uid|*> [<target|eid>]")
{
	if ( args.size() < 1 )
		return false;

	CEntityId id;
	CEntityBase *e = NULL;

	bool isChar = false;
	if (args[0] != "*")
	{
		CCharacter *c = NULL;
		GET_ACTIVE_CHARACTER2

		if ( args.size() > 2 && args[2] == "target")
		{
			id = c->getTarget();
			if( id.getType() == 0 )
			{
				CCharacter *c = PlayerManager.getChar(id);
				if (c && c->getEnterFlag())
					e = c;
			}
			else
			{
				e = CreatureManager.getCreature(id);
			}
		}
		else
			e = c;
	}
	else
	{
		if ( args.size() < 2 )
			return false;

		id.fromString( args[2].c_str() );
		e = CreatureManager.getCreature(id);
	}

	if (e)
	{
		log.displayNL("%s", toString( e->getBehaviour() ).c_str() );
	}
	else
	{
		log.displayNL("ERR: entity not found");
	}

	return true;
}


NLMISC_COMMAND(stopMoveBot,"stop move of a bot","<uid|*> [<target|eid>]")
{
	if ( args.size() < 1 )
		return false;

	TDataSetRow TargetRowId;
	CEntityBase *e = NULL;

	bool isChar = false;
	if (args[0] != "*")
	{
		CCharacter *c = NULL;
		GET_ACTIVE_CHARACTER2

		if (c)
		{
			const CEntityId &target = c->getTarget();
			if (target == CEntityId::Unknown)
			{
				log.displayNL("ERR: target");
				return true;
			}

			TargetRowId = TheDataset.getDataSetRow(target);
			TDataSetRow stoppedNpc = c->getStoppedNpc();
			if (stoppedNpc == TargetRowId)
				return true;

			if (TheDataset.isAccessible(stoppedNpc))
			{
				CharacterBotChatBeginEnd.BotChatEnd.push_back(c->getEntityRowId());
				CharacterBotChatBeginEnd.BotChatEnd.push_back(stoppedNpc);
			}
			CharacterBotChatBeginEnd.BotChatStart.push_back(c->getEntityRowId());
			c->setStoppedNpc(TargetRowId);
			c->setStoppedNpcTick();
		}
		else
		{
			log.displayNL("ERR: user");
			return true;
		}
	}
	else
	{
		if ( args.size() < 2 )
			return false;

		CEntityId target;
		target.fromString( args[2].c_str() );
		if (target == CEntityId::Unknown)
		{
			log.displayNL("ERR: target");
			return true;
		}

		TargetRowId = TheDataset.getDataSetRow(target);
	}

	CharacterBotChatBeginEnd.BotChatStart.push_back(TargetRowId);
}


NLMISC_COMMAND(startMoveBot,"start move bot or previous stopped bot","<uid|*> [<target|eid>]")
{
	if ( args.size() < 1 )
		return false;

	TDataSetRow TargetRowId;
	CEntityBase *e = NULL;

	bool isChar = false;
	if (args[0] != "*")
	{
		CCharacter *c = NULL;
		GET_ACTIVE_CHARACTER2

		if (c)
		{
			CharacterBotChatBeginEnd.BotChatEnd.push_back(c->getEntityRowId());
			TargetRowId = c->getStoppedNpc();
			c->setStoppedNpc(TDataSetRow());
		}
		else
		{
			log.displayNL("ERR: user");
			return true;
		}
	}
	else
	{
		if ( args.size() < 2 )
			return false;

		CEntityId target;
		target.fromString( args[2].c_str() );
		if (target == CEntityId::Unknown)
		{
			log.displayNL("ERR: target");
			return true;
		}

		TargetRowId = TheDataset.getDataSetRow(target);
	}

	CharacterBotChatBeginEnd.BotChatEnd.push_back(TargetRowId);
	log.displayNL("OK");
	return true;
}

NLMISC_COMMAND(closeDynChat, "close DynChat", "<uid> <process missions?>")
{
	if (args.size() < 1) return false;

	GET_ACTIVE_CHARACTER

	bool processMissions = true;
	if (args.size() >= 2 && (args[1] == "false" || args[1] == "0"))
		processMissions = false;

	c->endBotChat(false, false, processMissions);

	return true;
}

NLMISC_COMMAND(manageBuilding, "Manage a building", "<uid> <action> <value>")
{
	if (args.size() < 3) return false;

	GET_ACTIVE_CHARACTER

	string action = args[1]; // trigger_in, trigger_out, add_guild_room, add_player_room

	if (action == "trigger_in")
	{
		uint32 liftId;
		NLMISC::fromString(args[2], liftId);
		CBuildingManager::getInstance()->addTriggerRequest(c->getEntityRowId(), liftId);
	}
	else if (action == "trigger_out")
	{
		CBuildingManager::getInstance()->removeTriggerRequest(c->getEntityRowId());

	}
	else if (action == "add_guild_room")
	{
		CBuildingPhysicalGuild * building = dynamic_cast<CBuildingPhysicalGuild *>(CBuildingManager::getInstance()->getBuildingPhysicalsByName(args[2]));
		if (building)
			building->addGuild(c->getGuildId());
		else
		{
			log.displayNL("KO: no building");
			return true;
		}
	}
	else if (action == "add_player_room")
	{
		CBuildingPhysicalPlayer * building = dynamic_cast<CBuildingPhysicalPlayer *>(CBuildingManager::getInstance()->getBuildingPhysicalsByName(args[2]));
		if (building)
			building->addPlayer(c->getId());
		else
		{
			log.displayNL("KO: no building");
			return true;
		}
	}
	else if (action == "buy_guild_room")
	{
		CBuildingPhysicalGuild * building = dynamic_cast<CBuildingPhysicalGuild *>(CBuildingManager::getInstance()->getBuildingPhysicalsByName(args[2]));
		if (building)
		{
			CGuild * guild = CGuildManager::getInstance()->getGuildFromId(c->getGuildId());
			if (guild != NULL)
				guild->setBuilding(building->getAlias());
			else
			{
				log.displayNL("KO: no guild");
				return true;
			}
		}
	}
	else if (action == "buy_player_room")
	{
		CBuildingPhysicalPlayer * building = dynamic_cast<CBuildingPhysicalPlayer *>(CBuildingManager::getInstance()->getBuildingPhysicalsByName(args[2]));
		if ( building )
			CBuildingManager::getInstance()->buyBuilding(c->getId(), building->getAlias());
		else
		{
			log.displayNL("KO: no building");
			return true;
		}
	}
	else if (action == "set_player_room")
	{
	/*	CBuildingPhysicalPlayer * building = dynamic_cast<CBuildingPhysicalPlayer *>(CBuildingManager::getInstance()->getBuildingPhysicalsByName(args[2]));
		if ( building )
		{
			c->getRoomInterface().setBuilding(building);
			building->addPlayer(c->getId());
		}
		else
		{
			log.displayNL("KO: no building");
			return true;
		}*/
	}
	else if (action == "get_access_room")
	{

		CCharacter *owner = PlayerManager.getCharacterByName(CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), args[2]));
		if (owner)
			owner->addRoomAccessToPlayer(c->getId());
		else
		{
			log.displayNL("KO: no owner");
			return true;
		}
	}

	log.displayNL("OK");
	return true;
}


NLMISC_COMMAND(despawnTargetSource, "Despawn the target source", "<uid>")
{
	if (args.size() < 1) return false;

	GET_ACTIVE_CHARACTER
	const CEntityId &target = c->getTarget();
	if (target.getType() == RYZOMID::forageSource)
	{
		TDataSetRow sourceRowId = c->getTargetDataSetRow();
		CHarvestSource	*source = CHarvestSourceManager::getInstance()->getEntity( sourceRowId );
		if (source && !source->wasProspected())
		{
			source->spawnEnd(false);
			log.displayNL("OK");
			return true;
		}
	}

	log.displayNL("ERR");
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(setServerPhrase, "Set an IOS phrase", "<phrase> [<language code>]")
{
	if (args.size() < 2)
		return false;

	string phraseName = args[0];
	ucstring content;
	content.fromUtf8(args[1]);
	ucstring phraseContent = phraseName;
	phraseContent += "(){[";
	phraseContent += content;
	phraseContent += "]}";

	string msgname = "SET_PHRASE";
	bool withLang = false;
	string lang = "";
	if (args.size() == 3)
	{
		lang = args[2];
		if (lang != "all")
		{
			withLang = true;
			msgname = "SET_PHRASE_LANG";
		}
	}

	NLNET::CMessage	msgout(msgname);
	msgout.serial(phraseName);
	msgout.serial(phraseContent);
	if (withLang)
		msgout.serial(lang);
	sendMessageViaMirror("IOS", msgout);
	return true;
}
