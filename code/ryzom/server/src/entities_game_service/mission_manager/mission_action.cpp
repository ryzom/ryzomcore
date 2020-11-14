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

#include "nel/misc/algo.h"

#include "game_share/send_chat.h"
#include "game_share/fame.h"
#include "game_share/people.h"

#include "egs_sheets/egs_sheets.h"

#include "mission_action.h"
#include "mission_log.h"
#include "mission_manager/mission_template.h"
#include "mission_manager/ai_alias_translator.h"
#include "mission_manager/mission_parser.h"
#include "mission_manager/mission_manager.h"
#include "phrase_manager/phrase_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "creature_manager/creature_manager.h"
#include "zone_manager.h"
#include "team_manager/reward_sharing.h"
#include "guild_manager/guild_charge.h"
#include "team_manager/team_manager.h"
#include "world_instances.h"
#include "mission_manager/mission_solo.h"
#include "mission_manager/mission_team.h"
#include "mission_manager/mission_guild.h"
#include "primitives_parser.h"
#include "player_manager/character_encyclopedia.h"
#include "player_manager/character_game_event.h"
#include "player_manager/character_respawn_points.h"
#include "stat_db.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "shop_type/named_items.h"
#include "server_share/log_item_gen.h"
#include "server_share/log_character_gen.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(IMissionAction);


/***************************************************************************************************
Implementation of all mission instructions
	-failure			: set the current failure instructions
	-set_desc			: set the mission description
	-system_msg			: send a sys message to the player
	-popup_msg			: send a popup message to the player
	-bot_chat			: a bot say something ( in any valid chat mode )
	-recv_item			: give an item to user
	-recv_named_item	: give a named item to user
	-destroy_item		: destroy 1+ item in the bag/pack animal of the user
	-if_item_in_inv		: jump ot an instruction if there is enough item of sheet/quality in the bag/pack animal of the user
	-learn_action		: learn an action
	-recv_money			: give money
	-recv_fame			: give fame
	-recv_xp			: give xp
	-recv_faction_point	: give faction points
	-bot_emot			: bot executes an emot
	-jump				: jump to an instruction
	-if_mission_done	: jump to an instruction if the mission is done
	-end				: force mission end (success)
	-fail				: force failure
	-fail_if_sdb		: force failure if the given SDB expression is true
	-fail_mission_cat	: force failure of all current player missions belonging to the given category
	-add_compass_npc	: add a NPC in the compass
	-add_compass_place	: add a place in the compass
	-ai_event			: triggers an AI event
	-timer				: set the mission timer
	-season				: set the mission constraint
	-day_period			: set the time bounds of the mission
	-reward				: trigger group reward bloc
	-set_teleport		: add a teleport option to NPCS
	-teleport			: teleport people in a tp spawn zone as soon as the action is triggered
	-recv_charge_point	: add charge points to a guild
	-give_control		: give control of an outpost
	-building			: add a building to an outpost
	-declare_war		: declare a war to the guild
	-recv_guild_xp		: give xp to a guild
	-inside				: add an inside constraint
	-outside			: add an outside constraint
	-cancel_inside		: cancel an inside constraint
	-cancel_outside		: cancel an inside constraint

	-spawn_mission		: spawn a new mission
	-chain_mission		: ends the current and spawn a new mission
	-encyclo_unlock		: unlock a thema (player can see all tasks and rite)
	-game_event_subscribe : the player wants to participate to the global game event
	-game_event_reset	: the player finished or abandon the global game event
	-set_respawn_points	: set temporary respawn points for a continent (they hide other respawn points of the continent)
	-sdb_set			: set a value leaf in SDB
	-sdb_add			: add a delta to a value leaf in SDB
	-sdb_player_add		: add a delta to a table leaf SDB (both player and guild score are changed)
	-if_sdb				: jump to an instruction if the SDB expression is true
	-sdb_set_pvp_path	: set the path where the player wins HoF points in PvP
	-sdb_clear_pvp_path	: clear the PvP path (player will not win HoF points in PvP anymore)
	-if_faction_point	: jump to an instruction if the Faction Point expression is true
	-if_race			: jump to an instruction if all characters made the mission are of race asked in if_race test
	-if_cult			: jump to an instruction if all characters made the mission are of cult asked in if_cult test
	-if_civ				: jump to an instruction if all characters made the mission are of civ asked in if_civ test
	-if_room_in_inventory : jump to an instruction if there is enough room in inventory for at least one entity
	-set_cult			: set cult of characters made the mission
	-set_civ			: set civilization of character made the mission
	-if_guild_cult		: jump to an instruction if character made the mission are in guild of cult asked in test
	-if_guild_civ		: jump to an instruction if character made the mission are in guild of civ asked in test
	-if_guild_fame		: jump to an instruction if character made the mission are in guild with at least fame point asked in test
	-set_guild_civ		: set civilization of guild of character made the mission
	-set_guild_cult		: set cult of guild of characters made the mission
	-if_no_trial		: jump to an instruction if character made the mission are not a trial account



***************************************************************************************************/

// HELPERS
#define MISSION_ACTION_GETNEWPTR(_class_)\
IMissionAction* getNewPtr()\
{\
	_class_ * ptr = new _class_;\
	*ptr = *this;\
	return ptr;\
}

void logMissionActionLaunch(uint32 line, CMission *m, const string &ActionNameAndParams)
{
	if (!VerboseMissions) return;
	string sTmp = "sline:" + toString(line) + " ";
	
	std::vector<TDataSetRow> entities;
	m->getEntities( entities );
	for ( uint i  = 0; i < entities.size(); i++)
	{
		CCharacter *pChar = PlayerManager.getChar(entities[i]);
		sTmp += "user:" + pChar->getId().toString() + " ";
	}
	
	TAIAlias alias = m->getTemplateId();
	sTmp += "miss:" + CPrimitivesParser::aliasToString(alias);
	CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate( alias );
	if (templ != NULL)
		sTmp += ",'" + templ->getMissionName() + "' ";
	
	sTmp += "EXEC " + ActionNameAndParams;
	
	MISDBG("%s", sTmp.c_str());
}
#define LOGMISSIONACTION(xxxx) logMissionActionLaunch(_SourceLine, instance, xxxx)

// buildAction default

std::vector< std::pair< std::string, IMissionActionFactory* > > * IMissionActionFactory::Entries;

IMissionAction* IMissionActionFactory::buildAction( uint32 line, const std::vector<std::string> & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData )
{
	for ( uint i = 0; i < Entries->size(); i++ )
	{
		if ( script[0] == (*Entries)[i].first )
		{
			IMissionAction * ret = (*Entries)[i].second->instanciate();
			if ( !ret )
			{
				nlerror("BUG IN ACTION FACTORY : BAD INIT CODE");
				return NULL;
			}
			if ( !ret->buildAction(line, script,globalData,missionData) )
			{
				MISLOGERROR("building action failed");
				delete ret;
				return NULL;
			}
			return ret;
		}
	}
	return NULL;
}// IMissionActionFactory buildAction

void IMissionActionFactory::init()
{
	if ( ! Entries )
		Entries = new std::vector< std::pair< std::string, IMissionActionFactory* > >;
}// IMissionActionFactory init

MISSION_REGISTER_ACTION(CMissionActionSetFailureIndex,"failure");
void CMissionActionSetFailureIndex::launch(CMission* instance, std::list< CMissionEvent * > & eventList)
{
	LOGMISSIONACTION("failure");
	instance->setFailureIndex(_Index);
}

MISSION_REGISTER_ACTION(CMissionActionSetPlayerReconnectHandlerIndex,"player_reconnect");
void CMissionActionSetPlayerReconnectHandlerIndex::launch(CMission* instance, std::list< CMissionEvent * > & eventList)
{
	LOGMISSIONACTION("player_reconnect ("+toString(_Index)+")");
	// add the mission in the manager, if it is its first 'player_reconnect' handler
	if ( instance->getPlayerReconnectHandlerIndex() == 0xFFFFFFFF )
		CMissionManager::getInstance()->addPlayerReconnectHandlingMissions(*instance);
	instance->setPlayerReconnectHandlerIndex(_Index);
}

MISSION_REGISTER_ACTION(CMissionActionSetCrashHandlerIndex,"crash");
void CMissionActionSetCrashHandlerIndex::launch(CMission* instance, std::list< CMissionEvent * > & eventList)
{
	LOGMISSIONACTION("crash");
		// add the mission in the manager, if it is its first crash handler
	if ( instance->getCrashHandlerIndex() == 0xFFFFFFFF )
		CMissionManager::getInstance()->addCrashHandlingMissions(*instance);
	instance->setCrashHandlerIndex(_Index);
}

// HELPERS

static bool evaluateSDBExpr(const std::string &sdbExpr, double &res, bool ignoreUnknownVar)
{
	string numExpr;
	uint i = 0;
	while (i < sdbExpr.size())
	{
		if (sdbExpr[i] == '{')
		{
			string varName;
			sint32 varValue;
			i++;
			while (i < sdbExpr.size() && sdbExpr[i] != '}')
			{
				if (!(isalnum(sdbExpr[i]) || sdbExpr[i] == '_' || sdbExpr[i] == '.'))
					return false;
				varName += sdbExpr[i];
				i++;
			}
			if (i == sdbExpr.size() || varName.empty())
				return false;
			
			if (!CStatDB::getInstance()->valueGet(varName, varValue))
			{
				if (!ignoreUnknownVar)
					return false;

				varValue = 0;
			}

			numExpr += NLMISC::toString(varValue);
		}
		else
		{
			numExpr += sdbExpr[i];
		}

		i++;
	}

	if (CEvalNumExpr().evalExpression(numExpr.c_str(), res, NULL) != CEvalNumExpr::NoError)
		return false;
	return true;
}


/// basic class used to parse and managed texts params
// ----------------------------------------------------------------------------
class CMissionActionText : public IMissionAction
{
protected:
	bool buildAction( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<phrase_id>*[;<param>]");
			return false;
		}
		// store all texts params temporary in the _Params vector, we'll solve them at the end of the parsing
		// keep first space for player name
		return CMissionParser::parseParamText(line, script[1], _Text, _Params );
	}
	bool solveTextsParams( CMissionSpecificParsingData & missionData )
	{
		return CMissionParser::solveTextsParams( _SourceLine, _Params, missionData );
	}
	
	string	_Text;
	TVectorParamCheck _Params;
};


///send a system message
// ----------------------------------------------------------------------------
class CMissionActionSysMsg : public CMissionActionText
{

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("system_msg");
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		TVectorParamCheck params = _Params;
		CMissionParser::solveBotNames(params,CAIAliasTranslator::getInstance()->getEntityId(instance->getGiver()));
		for ( uint i  = 0; i < entities.size(); i++)
		{
			CMissionParser::solvePlayerName(params, entities[i]);
			PHRASE_UTILITIES::sendDynamicSystemMessage(entities[i], _Text, params);
		}
	};
	MISSION_ACTION_GETNEWPTR(CMissionActionSysMsg)
};
MISSION_REGISTER_ACTION(CMissionActionSysMsg, "system_msg");


///an NPC chats something
// ----------------------------------------------------------------------------
class CMissionActionBotChat :public CMissionActionText
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		bool ret = true;
		if ( script.size() != 4)
		{
			MISLOGSYNTAXERROR("<chat_type>:<bot_name>:<phrase_id>[*;<param>]");
			return false;
		}
		// get chat mode
		string sChatType = CMissionParser::getNoBlankString(script[1]);
		_ChatMode = CChatGroup::stringToGroupType(sChatType);
		if ( _ChatMode == CChatGroup::nbChatMode )
		{
			MISLOGERROR1("unknown chat mode '%s'", sChatType.c_str());
			ret = false;
		}

		// get speaking bot
		if ( !CMissionParser::parseBotName(script[2],_Bot,missionData) )
			return false;
		
		return CMissionParser::parseParamText(line, script[3], _Text, _Params );
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("bot_chat");
		TAIAlias alias;
		if ( _Bot == CAIAliasTranslator::Invalid )
			alias = instance->getGiver();
		else
			alias = _Bot;
		const CEntityId & botId = CAIAliasTranslator::getInstance()->getEntityId( alias );
		if ( botId == CEntityId::Unknown )
		{
			LOGMISSIONACTION("bot_chat : Bot alias " + CPrimitivesParser::aliasToString(alias) + " not in mirror" );
			return;
		}
		const TDataSetRow & botRow = TheDataset.getDataSetRow( botId );

		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		TVectorParamCheck params = _Params;
		
		CMissionParser::solveBotNames(params,CAIAliasTranslator::getInstance()->getEntityId(instance->getGiver()));

		// 2 different loop : avoid multi test on chat type
		if( _ChatMode == CChatGroup::tell  )
		{
			for ( uint i  =0; i < entities.size(); i++)
			{
				CMissionParser::solvePlayerName(params, entities[i]);
				uint32 txt = STRING_MANAGER::sendStringToClient( entities[i],_Text,params );
				npcTellToPlayerEx( botRow,entities[i],txt );
			}
		}
		else
		{
			for ( uint i  =0; i < entities.size(); i++)
			{
				CMissionParser::solvePlayerName(params, entities[i]);
				uint32 txt = STRING_MANAGER::sendStringToClient( entities[i],_Text,params );
				npcChatToChannelEx(botRow, _ChatMode, txt);
			}
		}
	};
	
	TAIAlias				_Bot;
	CChatGroup::TGroupType	_ChatMode;

	MISSION_ACTION_GETNEWPTR(CMissionActionBotChat)
};
MISSION_REGISTER_ACTION(CMissionActionBotChat,"bot_chat");


///send a popup message
// ----------------------------------------------------------------------------
class CMissionActionPopupMsg : public CMissionActionText
{
protected:
	bool buildAction( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3 )
		{
			MISLOGSYNTAXERROR("<title_id>*[;<param>] : <phrase_id>*[;<param>]");
			return false;
		}
		// store all texts params temporary in the _Params vector, we'll solve them at the end of the parsing
		// keep first space for player name
		return CMissionParser::parseParamText(line, script[1], _Title, _TitleParams )
			&& CMissionParser::parseParamText(line, script[2], _Text, _TextParams );
	}

	bool solveTextsParams( CMissionSpecificParsingData & missionData )
	{
		return CMissionParser::solveTextsParams( _SourceLine, _TitleParams, missionData )
			&& CMissionParser::solveTextsParams( _SourceLine, _TextParams, missionData );
	}
	
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("popup_msg");
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		// solve bot names for title and text
		TVectorParamCheck titleParams = _TitleParams;
		TVectorParamCheck textParams = _TextParams;
		CMissionParser::solveBotNames(titleParams,CAIAliasTranslator::getInstance()->getEntityId(instance->getGiver()));
		CMissionParser::solveBotNames(textParams,CAIAliasTranslator::getInstance()->getEntityId(instance->getGiver()));
		// For all entities that do this mission
		for ( uint i  = 0; i < entities.size(); i++)
		{
			// solve player name
			CMissionParser::solvePlayerName(titleParams, entities[i]);
			CMissionParser::solvePlayerName(textParams, entities[i]);

			// send the popup message
			uint32 titleId = STRING_MANAGER::sendStringToClient(entities[i], _Title, titleParams);
			uint32 textId = STRING_MANAGER::sendStringToClient(entities[i], _Text, textParams);
			CEntityId eid = TheDataset.getEntityId(entities[i]);
			PlayerManager.sendImpulseToClient(eid, "USER:POPUP", titleId, textId);
			
		}
	};
	MISSION_ACTION_GETNEWPTR(CMissionActionPopupMsg)

private:
	string				_Title;
	TVectorParamCheck	_TitleParams;
	string				_Text;
	TVectorParamCheck	_TextParams;
};
MISSION_REGISTER_ACTION(CMissionActionPopupMsg, "popup_msg");


/// set the mission description
// ----------------------------------------------------------------------------
class CMissionActionSetDesc : public IMissionAction
{
	bool buildAction( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<phrase_id>*[;<param>]");
			return false;
		}
		// store all texts params temporary in the _Params vector, we'll solve them at the end of the parsing
		// keep first space for player name
		missionData.Template->OverloadedDescs.push_back( CMissionTemplate::COverloadedDesc() );
		_DescIndex = (uint32)missionData.Template->OverloadedDescs.size() - 1;
		if ( !CMissionParser::parseParamText(line, script[1], missionData.Template->OverloadedDescs.back().Text,missionData.Template->OverloadedDescs.back().Params ) )
			return false;
		return true;
	}
	
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("set_desc");
		instance->overrideDesc( _DescIndex );
	};
	
	// index of this description in the template
	uint32	_DescIndex;
	
	MISSION_ACTION_GETNEWPTR(CMissionActionSetDesc)
};
MISSION_REGISTER_ACTION(CMissionActionSetDesc,"set_desc");


/// user receive an item
// ----------------------------------------------------------------------------
class CMissionActionRecvItem : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		bool ret = true;
		if ( script.size() != 2 && script.size() != 3 && script.size() != 4)
		{
			MISLOGSYNTAXERROR("<item> [<quantity>] [<quality>][:npc_name][:group][:guild]");
			return false;
		}
		vector<string> args;
		CMissionParser::tokenizeString( script[1]," \t",args );

		_Quantity = 1;
		if ( args.size() >= 2)
			NLMISC::fromString(args[1], _Quantity);

		uint i = 0;
		for (; i < missionData.Items.size(); i++ )
		{
			if ( !nlstricmp( missionData.Items[i].first , args[0] ) )
			{
				if ( args.size() > 2 )
				{
					MISLOGSYNTAXERROR("<special item> [<quantity>]");
					return false;
				}
				_Item = missionData.Items[i].second;
				break;
			}
		}

		if ( i == missionData.Items.size() )
		{
			string sSheetName = CMissionParser::getNoBlankString(args[0]) + ".sitem";
			_SheetId = CSheetId( sSheetName );
			if ( _SheetId == CSheetId::Unknown )
			{
				MISLOGERROR1("sheetId '%s' is unknon", sSheetName.c_str());
				ret = false;
			}
			missionData.ChatParams.push_back( make_pair( args[0], STRING_MANAGER::item ) );
			
			_Quality = 1;
			if ( args.size() == 3 )
			{
				NLMISC::fromString(args[2], _Quality);
				if ( _Quality == 0 )
				{
					MISLOGERROR("quality = 0");
					ret = false;
				}
			}
		}

		_Group = false;
		if ( script.size() == 3 )
		{
			string option = CMissionParser::getNoBlankString(script[2]);
			if ( !nlstricmp(option,"group") )
				_Group = true;
		}

		// We check for the guild option
		_Guild = false;
		for (std::vector< std::string >::const_iterator it = script.begin(); it != script.end(); ++it)
		{
			if (CMissionParser::getNoBlankString(*it) == "guild")
			{
				_Guild = true;
				break;
			}
		}

		
		if ( _Quantity == 0 )
		{
			MISLOGERROR("quantity = 0");
			return false;
		}
		return ret;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("recv_item");
		TLogContext_Item_Mission	logContext(instance->getMainEntity()->getId());

		static const CSheetId stackId("stack.sitem");

		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);
		if ( entities.empty() )
			return;
		if ( dynamic_cast<CMissionSolo*>(instance) )
		{
			if ( _Group )
			{
				CCharacter * user = PlayerManager.getChar( entities[0] );
				CTeam * team = TeamManager.getRealTeam(user->getTeamId());
				if ( team )
				{
					entities.resize( team->getTeamMembers().size() );
					list<CEntityId>::const_iterator it = team->getTeamMembers().begin();
					for (uint i = 0; i < team->getTeamMembers().size(); i++)
					{
						entities[i] = TheDataset.getDataSetRow( *it );
						++it;
					}
				}
			}
		}
		else if ( !_Group && !_Guild)
		{
			CCharacter * user = PlayerManager.getChar( entities[0] );
			CTeam * team = TeamManager.getRealTeam(user->getTeamId());
			if ( team )
			{
				entities.resize(1);
				entities[0] = TheDataset.getDataSetRow( team->getLeader() );
			}
		}

		// If the case we want to give the item to the guild
		if (_Guild)
		{
			if (entities.size() == 0)
				return;

			CCharacter * user = PlayerManager.getChar( entities[0] );
			if (!user)
			{
				LOGMISSIONACTION("recv_fame : Invalid user");
				return;
			}

			CGuild * guild = CGuildManager::getInstance()->getGuildFromId(user->getGuildId());
			if (!guild)
			{
				LOGMISSIONACTION("recv_fame : Invalid guild id '" + NLMISC::toString(user->getGuildId()) + "'");
				return;
			}

			SM_STATIC_PARAMS_3(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer);
			if ( _SheetId != CSheetId::Unknown )
			{
				const CStaticItem * form = CSheets::getForm( _SheetId );
				if ( !form )
				{
					LOGMISSIONACTION("sheetId '" + _SheetId.toString() + "' is unknown");
					return;
				}
				if (form->Family != ITEMFAMILY::MISSION_ITEM)
					return;

				uint quantity = _Quantity;
				while (quantity > 0)
				{
					CGameItemPtr item = user->createItem(_Quality, quantity, _SheetId);
					if (item == NULL)
						break;
					const uint32 stackSize = item->getStackSize();

					if (!guild->putItem(item))
					{
						CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( instance->getTemplateId() );
						if ( templ )
						{
							if ( templ->Tags.FailIfInventoryIsFull )
							{
								instance->setProcessingState(CMission::ActionFailed);
								return;
							}
						}
					}
					// from here item maybe NULL (because of autostack)

					quantity -= stackSize;
				}
				params[2].Int = _Quality;
			}
			else
			{
				const CStaticItem * form = CSheets::getForm( _Item.getSheetId() );
				if ( !form )
				{
					LOGMISSIONACTION("sheetId '" + _Item.getSheetId().toString() + "' is unknown");
					return;
				}
				uint quantity = _Quantity;
				while (quantity > 0)
				{
					CGameItemPtr item = _Item.createItem(quantity);
					if (item == NULL)
						break;

					const uint32 stackSize = item->getStackSize();
					if (!guild->putItem(item))
					{
						CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( instance->getTemplateId() );
						if ( templ )
						{
							if ( templ->Tags.FailIfInventoryIsFull )
							{
								instance->setProcessingState(CMission::ActionFailed);
								return;
							}
						}
					}
					// from here item maybe NULL (because of autostack)

					quantity -= stackSize;
				}
				params[2].Int = _Item.getQuality();
			}

			params[0].SheetId = _SheetId;
			params[1].Int = _Quantity;

			for ( uint i = 0; i < entities.size(); i++ )
			{
				CCharacter * user = PlayerManager.getChar( entities[i] );
				if ( user )
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_GUILD_RECV_ITEM", params);
			}
		}
		else
		{
			// check free room space in inventory
			// NB : in case of group, fail happens only if none in the group have enough free space
			sint16 neededSlotCount = 0;
			uint32 neededBulk = 0;
			CSheetId sheet = ( _SheetId != CSheetId::Unknown )?_SheetId:_Item.getSheetId();
			CGameItemPtr itemTmp = GameItemManager.createItem(sheet, _Quality, true, true);
			if (itemTmp != NULL)
			{
				neededSlotCount = (sint16) ceil( (float)_Quantity / itemTmp->getMaxStackSize() );
				neededBulk = _Quantity * itemTmp->getStackBulk();
				itemTmp.deleteItem();
			}
			else
			{
				LOGMISSIONACTION("can't get static item from sheet " + sheet.toString());
				return;
			}

			bool fail = true;
			for ( uint i = 0; i < entities.size(); i++ )
			{
				CCharacter * user = PlayerManager.getChar( entities[i] );
				if ( user )
				{
					CInventoryPtr invBag = user->getInventory( INVENTORIES::bag );
					sint16 freeSlotcount = invBag->getFreeSlotCount();
					uint32 maxBulk = invBag->getMaxBulk();

					CInventoryPtr invTemp = user->getInventory( INVENTORIES::temporary );
					if( invTemp )
					{
						freeSlotcount -= invTemp->getUsedSlotCount();
						maxBulk -= invTemp->getInventoryBulk();
					}

					if( (neededSlotCount <= freeSlotcount) && ( neededBulk + invBag->getInventoryBulk() <= maxBulk) )
					{
						fail = false;
						break;
					}
				}
			}
			if( fail )
			{
				CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( instance->getTemplateId() );
				if ( templ )
				{
					if ( templ->Tags.FailIfInventoryIsFull )
					{
						instance->setProcessingState(CMission::ActionFailed);
						return;
					}
				}
			}

			for ( uint i = 0; i < entities.size(); i++ )
			{
				CCharacter * user = PlayerManager.getChar( entities[i] );
				if ( user )
				{
					SM_STATIC_PARAMS_3(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer);
					if ( _SheetId != CSheetId::Unknown )
					{
						const CStaticItem * form = CSheets::getForm( _SheetId );
						if ( !form )
						{
							LOGMISSIONACTION("sheetId '" + _SheetId.toString() + "' is unknown");
							return;
						}
						if (form->Family != ITEMFAMILY::MISSION_ITEM && !user->enterTempInventoryMode(TEMP_INV_MODE::MissionReward))
							continue;

						uint quantity = _Quantity;
						while (quantity > 0)
						{
							CGameItemPtr item = user->createItem(_Quality, quantity, _SheetId);
							if (item == NULL)
								break;
							const uint32 stackSize = item->getStackSize();

							if( form->Family != ITEMFAMILY::MISSION_ITEM )
							{
								if (!user->addItemToInventory(INVENTORIES::temporary, item))
								{
									item.deleteItem();
									break;
								}
							}
							else
							{
								if (!user->addItemToInventory(INVENTORIES::bag, item))
								{
									item.deleteItem();
									break;
								}
							}
							// from here item maybe NULL (because of autostack)

							quantity -= stackSize;
						}
						params[2].Int = _Quality;
					}
					else
					{
						const CStaticItem * form = CSheets::getForm( _Item.getSheetId() );
						if ( !form )
						{
							LOGMISSIONACTION("sheetId '" + _Item.getSheetId().toString() + "' is unknown");
							return;
						}
						uint quantity = _Quantity;
						while (quantity > 0)
						{
							CGameItemPtr item = _Item.createItem(quantity);
							if (item == NULL)
								break;

							const uint32 stackSize = item->getStackSize();
							if (!user->addItemToInventory(INVENTORIES::temporary, item))
							{
								item.deleteItem();
								break;
							}
							// from here item maybe NULL (because of autostack)

							quantity -= stackSize;
						}
						params[2].Int = _Item.getQuality();
					}

					params[0].SheetId = _SheetId;
					params[1].Int = _Quantity;
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_RECV_ITEM", params);
				}
			}
		}
	};
	CMissionItem _Item;
	uint16		 _Quality;
	uint16		 _Quantity;
	CSheetId	 _SheetId;
	bool		 _Group;
	bool		_Guild;

	MISSION_ACTION_GETNEWPTR(CMissionActionRecvItem)
};
MISSION_REGISTER_ACTION(CMissionActionRecvItem,"recv_item");

/// user receive an named item
// ----------------------------------------------------------------------------
class CMissionActionRecvNamedItem : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 && script.size() != 3 && script.size() != 4)
		{
			MISLOGSYNTAXERROR("<named_item> [<quantity>] [:group] [:guild]");
			return false;
		}
		vector<string> args;
		CMissionParser::tokenizeString( script[1]," \t",args );

		// read named item
		string	namedItem;
		if ( args.size() >= 1)
		{
			_NamedItem = CMissionParser::getNoBlankString(args[0]);
			if(CNamedItems::getInstance().getNamedItemRef(_NamedItem)==NULL)
			{
				MISLOGERROR1("namedItem '%s' is unknon", _NamedItem.c_str());
				return false;
			}
			// yoyo: I don't know for what it is used
			//missionData.ChatParams.push_back( make_pair( args[0], STRING_MANAGER::item ) );
		}
		else
		{
			MISLOGERROR("bad named item format");
			return false;
		}
			
		// read quantity
		_Quantity = 1;
		if ( args.size() >= 2)
			NLMISC::fromString(args[1], _Quantity);
		
		// read group
		_Group = false;
		if ( script.size() == 3 )
		{
			string option = CMissionParser::getNoBlankString(script[2]);
			if ( !nlstricmp(option,"group") )
				_Group = true;
		}

		// We check for the guild option
		_Guild = false;
		for (std::vector< std::string >::const_iterator it = script.begin(); it != script.end(); ++it)
		{
			if (CMissionParser::getNoBlankString(*it) == "guild")
			{
				_Guild = true;
				break;
			}
		}

		if ( _Quantity == 0 )
		{
			MISLOGERROR("quantity = 0");
			return false;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("recv_named_item");
		TLogContext_Item_Mission	logContext(instance->getMainEntity()->getId());

		// get the entities to apply the action
		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);
		if ( entities.empty() )
			return;
		if ( dynamic_cast<CMissionSolo*>(instance) )
		{
			if ( _Group )
			{
				CCharacter * user = PlayerManager.getChar( entities[0] );
				CTeam * team = TeamManager.getRealTeam(user->getTeamId());
				if ( team )
				{
					entities.resize( team->getTeamMembers().size() );
					list<CEntityId>::const_iterator it = team->getTeamMembers().begin();
					for (uint i = 0; i < team->getTeamMembers().size(); i++)
					{
						entities[i] = TheDataset.getDataSetRow( *it );
						++it;
					}
				}
			}
		}
		else if ( !_Group && !_Guild)
		{
			CCharacter * user = PlayerManager.getChar( entities[0] );
			CTeam * team = TeamManager.getRealTeam(user->getTeamId());
			if ( team )
			{
				entities.resize(1);
				entities[0] = TheDataset.getDataSetRow( team->getLeader() );
			}
		}

		// If the case we want to give the item to the guild
		if (_Guild)
		{
			if (entities.size() == 0)
				return;

			CCharacter * user = PlayerManager.getChar( entities[0] );
			if (!user)
			{
				LOGMISSIONACTION("recv_fame : Invalid user");
				return;
			}

			CGuild * guild = CGuildManager::getInstance()->getGuildFromId(user->getGuildId());
			if (!guild)
			{
				LOGMISSIONACTION("recv_fame : Invalid guild id '" + NLMISC::toString(user->getGuildId()) + "'");
				return;
			}

			// add the item to inventory
			CGameItemPtr item = CNamedItems::getInstance().createNamedItem(_NamedItem, _Quantity);
			if (item == NULL)
			{
				LOGMISSIONACTION("named item '" + _NamedItem + "' is unknown");
				return;
			}
			if (!guild->putItem(item))
			{
				CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( instance->getTemplateId() );
				if ( templ )
				{
					if ( templ->Tags.FailIfInventoryIsFull )
					{
						instance->setProcessingState(CMission::ActionFailed);
						return;
					}
				}
			}
			else
			{
				for ( uint i = 0; i < entities.size(); i++ )
				{
					CCharacter * user = PlayerManager.getChar( entities[i] );
					if ( user )
					{
						SM_STATIC_PARAMS_2(params, STRING_MANAGER::dyn_string_id, STRING_MANAGER::integer);
						params[0].StringId = item->sendNameId(user);
						params[1].Int = _Quantity;
						PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_GUILD_RECV_NAMED_ITEM", params);
					}
				}
			}
		}
		else
		{
			// check free room space in inventory
			// NB : in case of group, fail happens only if noone in the group have enough free space
			CGameItemPtr itemTmp = CNamedItems::getInstance().createNamedItem(_NamedItem, _Quantity);
			if( itemTmp != NULL )
			{
				sint16 neededSlotCount = (sint16) ceil( (float)_Quantity / itemTmp->getMaxStackSize() );
				uint32 neededBulk = _Quantity * itemTmp->getStackBulk();
				itemTmp.deleteItem();

				bool fail = true;
				for ( uint i = 0; i < entities.size(); i++ )
				{
					CCharacter * user = PlayerManager.getChar( entities[i] );
					if ( user )
					{
						CInventoryPtr invBag = user->getInventory( INVENTORIES::bag );
						sint16 freeSlotcount = invBag->getFreeSlotCount();
						uint32 maxBulk = invBag->getMaxBulk();

						CInventoryPtr invTemp = user->getInventory( INVENTORIES::temporary );
						if( invTemp )
						{
							freeSlotcount -= invTemp->getUsedSlotCount();
							maxBulk -= invTemp->getInventoryBulk();
						}

						if( (neededSlotCount <= freeSlotcount) && ( neededBulk + invBag->getInventoryBulk() <= maxBulk) )
						{
							fail = false;
							break;
						}
					}
				}

				if( fail )
				{
					CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( instance->getTemplateId() );
					if ( templ )
					{
						if ( templ->Tags.FailIfInventoryIsFull )
						{
							instance->setProcessingState(CMission::ActionFailed);
							return;
						}
					}
				}
			}
			else
			{
				LOGMISSIONACTION("named item '" + _NamedItem + "' is unknown");
				return;
			}

			// apply the action to all entities
			for ( uint i = 0; i < entities.size(); i++ )
			{
				CCharacter * user = PlayerManager.getChar( entities[i] );
				if ( user )
				{
					if (!user->enterTempInventoryMode(TEMP_INV_MODE::MissionReward))
						continue;

					// add the item to inventory
					CGameItemPtr item = CNamedItems::getInstance().createNamedItem(_NamedItem, _Quantity);
					if (item == NULL)
					{
						LOGMISSIONACTION("named item '" + _NamedItem + "' is unknown");
						return;
					}
					if(!user->addItemToInventory(INVENTORIES::temporary, item))
					{
						item.deleteItem();
					}
					else
					{
						SM_STATIC_PARAMS_2(params, STRING_MANAGER::dyn_string_id, STRING_MANAGER::integer);
						params[0].StringId = item->sendNameId(user);
						params[1].Int = _Quantity;
						PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_RECV_NAMED_ITEM", params);
					}
				}
			}
		}
	};
	std::string	 _NamedItem;
	uint16		 _Quantity;
	bool		 _Group;
	bool		_Guild;

	MISSION_ACTION_GETNEWPTR(CMissionActionRecvNamedItem)
};
MISSION_REGISTER_ACTION(CMissionActionRecvNamedItem,"recv_named_item");


/// Base Class use to build the selection of items of certain sheet/quality
// -----------------------------------------------------------------------------
class CMissionBaseItemSelector
{
public:
	// called at parse to fill filter infos
	bool	buildItemFilter(uint32 line, const std::vector< std::string > & script, CMissionSpecificParsingData &missionData)
	{
		bool ret = true;
		if(script.size()<2)
		{
			MISLOGSYNTAXERROR("Need the command + filters");
			return false;
		}

		vector<string> args;
		CMissionParser::tokenizeString( script[1]," \t",args );
		
		// read the quantity, or 1 by default
		_Quantity = 1;
		if ( args.size() >= 2)
			NLMISC::fromString(args[1], _Quantity);
		
		// If the name of the item macthes one of the special defined mission items for this mission
		uint i = 0;
		for (; i < missionData.Items.size(); i++ )
		{
			if ( !nlstricmp( missionData.Items[i].first , args[0] ) )
			{
				if ( args.size() > 2 )
				{
					MISLOGSYNTAXERROR("<special item> [<quantity>]");
					return false;
				}
				_SheetId = missionData.Items[i].second.getSheetId();
				_Quality = missionData.Items[i].second.getQuality();
				break;
			}
		}
		
		// If no special mission item found, get a sheetid/quality
		if ( i == missionData.Items.size() )
		{
			// read the sheetid of the item to destroy
			string sSheetName = CMissionParser::getNoBlankString(args[0]) + ".sitem";
			_SheetId = CSheetId( sSheetName );
			if ( _SheetId == CSheetId::Unknown )
			{
				MISLOGERROR1("sheetId '%s' is unknon", sSheetName.c_str());
				ret = false;
			}
			missionData.ChatParams.push_back( make_pair( args[0], STRING_MANAGER::item ) );
			
			// read the minimum quality to destroy (1 if not specified)
			_Quality = 1;
			if ( args.size() == 3 )
			{
				NLMISC::fromString(args[2], _Quality);
				if ( _Quality == 0 )
				{
					MISLOGERROR("quality = 0");
					ret = false;
				}
			}
		}
		
		// Bad quantity?
		if ( _Quantity == 0 )
		{
			MISLOGERROR("quantity = 0");
			return false;
		}

		return ret;
	}

	// Selection Filter
	CSheetId	_SheetId;
	uint16		_Quality;
	uint16		_Quantity;
};
	
/// we destroy the item of the user in its inventory
// -----------------------------------------------------------------------------
class CMissionActionDestroyItem : 
 public IMissionAction,
 private CMissionBaseItemSelector
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		// Parse the line
		_SourceLine = line;
		if ( script.size() != 2 && script.size() != 3 && script.size() != 4)
		{
			MISLOGSYNTAXERROR("<item> [<quantity>] [<quality>]:[npc_name] [:guild]");
			return false;
		}

		// Build the item filter
		bool	ret= CMissionBaseItemSelector::buildItemFilter(line, script, missionData);

		// parse the optional bot name
		_Npc = CAIAliasTranslator::Invalid;
		if(script.size()==3)
		{
			if (!CMissionParser::parseBotName(script[2], _Npc, missionData))
				ret= false;
		}

		// We check for the guild option
		_Guild = false;
		for (std::vector< std::string >::const_iterator it = script.begin(); it != script.end(); ++it)
		{
			if (CMissionParser::getNoBlankString(*it) == "guild")
			{
				_Guild = true;
				break;
			}
		}

		return ret;
	}

	// Execute the action
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("destroy_item");
		TLogContext_Item_Mission	logContext(instance->getMainEntity()->getId());

		// For all entities affected by the mission
		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);
		if ( entities.empty() )
			return;

		// If the "guild" parameter is not set, we destroy the items for the users
		if (!_Guild)
		{
			for ( uint i = 0; i < entities.size(); i++ )
			{
				CCharacter * user = PlayerManager.getChar( entities[i] );
				if ( user )
				{
					// Select the items in Bag AND mektoub that match the request
					vector<CCharacter::CItemSlotId>		itemList;
					user->selectItems(INVENTORIES::bag, _SheetId, _Quality, &itemList);
					for(uint pa=0;pa<INVENTORIES::max_pet_animal;pa++)
						user->selectItems(INVENTORIES::TInventory(INVENTORIES::pet_animal + pa), _SheetId, _Quality, &itemList);

					// Destroy them, up to quantity wanted
					// NB: don't care if destroying an item owned by a mektoub is strange because mektoub not near!
					uint	quantityReallyDestroyed;
					quantityReallyDestroyed= user->destroyItems(itemList, _Quantity);

					// Send message
					SM_STATIC_PARAMS_4(params, STRING_MANAGER::bot, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer);
					TAIAlias	botAlias= _Npc;
					if(botAlias==CAIAliasTranslator::Invalid)
						botAlias= instance->getGiver();
					params[0].setEIdAIAlias(CAIAliasTranslator::getInstance()->getEntityId( botAlias ), botAlias);
					params[1].SheetId = _SheetId;
					params[2].Int = quantityReallyDestroyed;
					params[3].Int = _Quality;
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_DESTROY_ITEM", params);
				}
			}
		}
		// We destroy the item in the guild
		else
		{
			CCharacter * user = PlayerManager.getChar( entities[0] );
			if (!user)
			{
				LOGMISSIONACTION("recv_fame : Invalid user");
				return;
			}

			CGuild * guild = CGuildManager::getInstance()->getGuildFromId(user->getGuildId());
			if (!guild)
			{
				LOGMISSIONACTION("recv_fame : Invalid guild id '" + NLMISC::toString(user->getGuildId()) + "'");
				return;
			}

			vector<CGuild::CItemSlotId>		itemList;
			guild->selectItems(_SheetId, _Quality, &itemList);

			// Destroy them, up to quantity wanted
			uint	quantityReallyDestroyed;
			quantityReallyDestroyed = guild->destroyItems(itemList, _Quantity);

			// Send message
			for ( uint i = 0; i < entities.size(); i++ )
			{
				CCharacter * user = PlayerManager.getChar( entities[i] );
				if ( user )
				{
					SM_STATIC_PARAMS_4(params, STRING_MANAGER::bot, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer);
					TAIAlias	botAlias= _Npc;
					if(botAlias==CAIAliasTranslator::Invalid)
						botAlias= instance->getGiver();
					params[0].setEIdAIAlias(CAIAliasTranslator::getInstance()->getEntityId( botAlias ), botAlias);
					params[1].SheetId = _SheetId;
					params[2].Int = quantityReallyDestroyed;
					params[3].Int = _Quality;
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_DESTROY_ITEM", params);
				}
			}
		}
	};
	TAIAlias	_Npc;
	bool		_Guild;

	MISSION_ACTION_GETNEWPTR(CMissionActionDestroyItem)
};
MISSION_REGISTER_ACTION(CMissionActionDestroyItem,"destroy_item");


/// We test if the user has the requested item in its inventory
// ----------------------------------------------------------------------------
class CMissionActionCondJumpIfItemInInv : 
 public CMissionActionJump,
 private CMissionBaseItemSelector
{
public:
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<item> [<quantity>] [<quality>]:<label>");
			return false;
		}
		
		// Build the item filter
		bool	ret= CMissionBaseItemSelector::buildItemFilter(line, script, missionData);
		
		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return ret;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_item_in_inv");
		
		// For all entities affected by the mission
		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);
		if ( entities.empty() )
			return;

		// if a player has not enough requested items, do not jump
		for ( uint i = 0; i < entities.size(); i++ )
		{
			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( user )
			{
				// Select the items in Bag AND mektoub that match the request
				uint	quantityDetected= 0;
				quantityDetected+= user->selectItems(INVENTORIES::bag, _SheetId, _Quality);
				for(uint pa=0;pa<INVENTORIES::max_pet_animal;pa++)
					quantityDetected+= user->selectItems(INVENTORIES::TInventory(INVENTORIES::pet_animal + pa), _SheetId, _Quality);
				
				// if not enough requested items, fail!
				if(quantityDetected<_Quantity)
					return;
			}
		}

		// ok all palyer have the request in their bag!
		CMissionActionJump::launch( instance, eventList );
	}
	
	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpIfItemInInv)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpIfItemInInv,"if_item_in_inv");

/// user receive one or more actions
// ----------------------------------------------------------------------------
class CMissionActionLearnAction : public IMissionAction
{
	bool buildAction( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() < 2 && script.size() > 4)
		{
			MISLOGSYNTAXERROR("<action> *[;<action] [: npc_name] [: group]");
			return false;
		}
		vector<string>	args;
		explode(script[1], string(";"), args, true);
		if (args.size() == 0)
		{
			MISLOGSYNTAXERROR("<action> *[;<action] [: npc_name] [: group]");
			return false;
		}
		for (uint i=0; i<args.size(); ++i)
		{
			string s = CMissionParser::getNoBlankString(args[i]) + ".sphrase";
			CSheetId sheetId = CSheetId( s );
			if ( sheetId == CSheetId::Unknown )
			{
				MISLOGERROR2("%uth sheetId '%s' is unknon", i, s.c_str());
				return false;
			}
			_ActionSheets.push_back(sheetId);
		}
		uint idx = 2;
		_Group = false;
		_Npc = CAIAliasTranslator::Invalid;

		while (idx < script.size())
		{
			if (CMissionParser::getNoBlankString(script[idx]) == "group")
			{
				// we found the group flag, must be last param
				if (idx != script.size()-1)
				{
					MISLOGSYNTAXERROR("'group' must be last parameter if present");
					return false;
				}

				_Group = true;
			}
			else
			{
				if (_Npc != CAIAliasTranslator::Invalid)
				{
					MISLOGERROR1("npc '%s' already present", script[idx].c_str());
					return false;
				}
				// this must be a npc identifier
				if (!CMissionParser::parseBotName(script[idx], _Npc, missionData))
					return false;
			}
			idx++;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("learn_action");
		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);
		if ( entities.empty() )
			return;
		if ( dynamic_cast<CMissionSolo*>(instance) )
		{
			if ( _Group )
			{
				CCharacter * user = PlayerManager.getChar( entities[0] );
				CTeam * team = TeamManager.getRealTeam(user->getTeamId());
				if ( team )
				{
					entities.resize( team->getTeamMembers().size() );
					list<CEntityId>::const_iterator it = team->getTeamMembers().begin();
					for (uint i = 0; i < team->getTeamMembers().size(); i++)
					{
						entities[i] = TheDataset.getDataSetRow( *it );
						++it;
					}
				}
			}
		}
		else if ( !_Group )
		{
			CCharacter * user = PlayerManager.getChar( entities[0] );
			CTeam * team = TeamManager.getRealTeam(user->getTeamId());
			if ( team )
			{
				entities.resize(1);
				entities[0] = TheDataset.getDataSetRow( team->getLeader() );
			}
		}
		for ( uint i = 0; i < entities.size(); i++ )
		{

			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( user )
			{
				TVectorParamCheck params(_ActionSheets.size());
				for (uint j=0; j<_ActionSheets.size(); ++j)
				{
					uint16 slot = user->getFirstFreePhraseSlot();
					// we replace previous phrase to force the learning of new bricks
					if ( slot == 0xFFFF )
						slot = 0;
					user->learnPrebuiltPhrase( _ActionSheets[j], slot, true );

					params[j].Type = STRING_MANAGER::sphrase;
					params[j].SheetId = _ActionSheets[j];
				}

				if (_Npc == CAIAliasTranslator::Invalid)
				{
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), 
						toString("MIS_RECV_ACTION_%u", _ActionSheets.size()), 
						params);
				}
				else
				{
					STRING_MANAGER::TParam p(STRING_MANAGER::bot, _Npc);
					params.push_back(p);
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), 
						toString("MIS_RECV_ACTION_NPC_%u", _ActionSheets.size()), 
						params);
				}
			}
		}
	};
	vector<CSheetId> _ActionSheets; //_SheetId;
	bool		_Group;
	TAIAlias	_Npc;

	MISSION_ACTION_GETNEWPTR(CMissionActionLearnAction)
};
MISSION_REGISTER_ACTION(CMissionActionLearnAction,"learn_action");

/// user receive a brick
// ----------------------------------------------------------------------------
class CMissionActionLearnBrick : public IMissionAction
{
	bool buildAction( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() < 2 && script.size() > 4)
		{
			MISLOGSYNTAXERROR("<brick> *[;<brick>] [: npc_name] [: group]");
			return false;
		}
		vector<string>	args;
		explode(script[1], string(";"), args, true);
		if (args.size() == 0)
		{
			MISLOGSYNTAXERROR("<brick> *[;<brick>] [: npc_name] [: group]");
			return false;
		}
		for (uint i=0; i<args.size(); ++i)
		{
			string s = CMissionParser::getNoBlankString(args[i]) + ".sbrick";
			CSheetId sheetId = CSheetId( s );
			if ( sheetId == CSheetId::Unknown )
			{
				MISLOGERROR2("%uth sheetId '%s' is unknown", i, s.c_str());
				return false;
			}
			_BrickSheets.push_back(sheetId);
		}
		uint idx = 2;
		_Group = false;
		_Npc = CAIAliasTranslator::Invalid;

		while (idx < script.size())
		{
			if (CMissionParser::getNoBlankString(script[idx]) == "group")
			{
				// we found the group flag, must be last param
				if (idx != script.size()-1)
				{
					MISLOGSYNTAXERROR("'group' must be last parameter if present");
					return false;
				}

				_Group = true;
			}
			else
			{
				if (_Npc != CAIAliasTranslator::Invalid)
				{
					MISLOGERROR1("npc '%s' already present", script[idx].c_str());
					return false;
				}
				// this must be a npc identifier
				if (!CMissionParser::parseBotName(script[idx], _Npc, missionData))
					return false;
			}
			idx++;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("learn_brick");
		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);
		if ( entities.empty() )
			return;
		if ( dynamic_cast<CMissionSolo*>(instance) )
		{
			if ( _Group )
			{
				CCharacter * user = PlayerManager.getChar( entities[0] );
				CTeam * team = TeamManager.getRealTeam(user->getTeamId());
				if ( team )
				{
					entities.resize( team->getTeamMembers().size() );
					list<CEntityId>::const_iterator it = team->getTeamMembers().begin();
					for (uint i = 0; i < team->getTeamMembers().size(); i++)
					{
						entities[i] = TheDataset.getDataSetRow( *it );
						++it;
					}
				}
			}
		}
		else if ( !_Group )
		{
			CCharacter * user = PlayerManager.getChar( entities[0] );
			CTeam * team = TeamManager.getRealTeam(user->getTeamId());
			if ( team )
			{
				entities.resize(1);
				entities[0] = TheDataset.getDataSetRow( team->getLeader() );
			}
		}
		for ( uint i = 0; i < entities.size(); i++ )
		{

			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( user )
			{
				TVectorParamCheck params(_BrickSheets.size());
				for (uint j=0; j<_BrickSheets.size(); ++j)
				{
					if(_BrickSheets[j] != CSheetId::Unknown)
					{
						user->addKnownBrick(_BrickSheets[j]);

						params[j].Type = STRING_MANAGER::sbrick;
						params[j].SheetId = _BrickSheets[j];
					}
					else
					{
						nlwarning("<CMissionActionLearnBrick> Can't learn brick, brick is Unknown, mission %d", instance->getTemplateId());
					}
				}

				if (_Npc == CAIAliasTranslator::Invalid)
				{
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), 
						toString("MIS_RECV_BRICK_%u", _BrickSheets.size()), 
						params);
				}
				else
				{
					STRING_MANAGER::TParam p(STRING_MANAGER::bot, _Npc);
					params.push_back(p);
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), 
						toString("MIS_RECV_BRICK_NPC_%u", _BrickSheets.size()), 
						params);
				}
			}
		}
	}
	vector<CSheetId> _BrickSheets; //_SheetId;
	bool		_Group;
	TAIAlias	_Npc;

	MISSION_ACTION_GETNEWPTR(CMissionActionLearnBrick)
};
MISSION_REGISTER_ACTION(CMissionActionLearnBrick,"learn_brick");

/// user forget a brick
// ----------------------------------------------------------------------------
class CMissionActionUnlearnBrick : public IMissionAction
{
	bool buildAction( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() < 2 && script.size() > 4)
		{
			MISLOGSYNTAXERROR("<brick> *[;<brick>] [: npc_name] [: group]");
			return false;
		}
		vector<string>	args;
		explode(script[1], string(";"), args, true);
		if (args.size() == 0)
		{
			MISLOGSYNTAXERROR("<brick> *[;<brick>] [: npc_name] [: group]");
			return false;
		}
		for (uint i=0; i<args.size(); ++i)
		{
			string s = CMissionParser::getNoBlankString(args[i]) + ".sbrick";
			CSheetId sheetId = CSheetId( s );
			if ( sheetId == CSheetId::Unknown )
			{
				MISLOGERROR2("%uth sheetId '%s' is unknown", i, s.c_str());
				return false;
			}
			_BrickSheets.push_back(sheetId);
		}
		uint idx = 2;
		_Group = false;
		_Npc = CAIAliasTranslator::Invalid;

		while (idx < script.size())
		{
			if (CMissionParser::getNoBlankString(script[idx]) == "group")
			{
				// we found the group flag, must be last param
				if (idx != script.size()-1)
				{
					MISLOGSYNTAXERROR("'group' must be last parameter if present");
					return false;
				}

				_Group = true;
			}
			else
			{
				if (_Npc != CAIAliasTranslator::Invalid)
				{
					MISLOGERROR1("npc '%s' already present", script[idx].c_str());
					return false;
				}
				// this must be a npc identifier
				if (!CMissionParser::parseBotName(script[idx], _Npc, missionData))
					return false;
			}
			idx++;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("unlearn_brick");
		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);
		if ( entities.empty() )
			return;
		if ( dynamic_cast<CMissionSolo*>(instance) )
		{
			if ( _Group )
			{
				CCharacter * user = PlayerManager.getChar( entities[0] );
				CTeam * team = TeamManager.getRealTeam(user->getTeamId());
				if ( team )
				{
					entities.resize( team->getTeamMembers().size() );
					list<CEntityId>::const_iterator it = team->getTeamMembers().begin();
					for (uint i = 0; i < team->getTeamMembers().size(); i++)
					{
						entities[i] = TheDataset.getDataSetRow( *it );
						++it;
					}
				}
			}
		}
		else if ( !_Group )
		{
			CCharacter * user = PlayerManager.getChar( entities[0] );
			CTeam * team = TeamManager.getRealTeam(user->getTeamId());
			if ( team )
			{
				entities.resize(1);
				entities[0] = TheDataset.getDataSetRow( team->getLeader() );
			}
		}
		for ( uint i = 0; i < entities.size(); i++ )
		{

			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( user )
			{
				TVectorParamCheck params(_BrickSheets.size());
				for (uint j=0; j<_BrickSheets.size(); ++j)
				{
					if(_BrickSheets[j] != CSheetId::Unknown)
					{
						user->removeKnownBrick(_BrickSheets[j]);

						params[j].Type = STRING_MANAGER::sbrick;
						params[j].SheetId = _BrickSheets[j];
					}
					else
					{
						nlwarning("<CMissionActionUnlearnBrick> Can't unlearn brick, brick is Unknown, mission %d", instance->getTemplateId());
					}
				}

				if (_Npc == CAIAliasTranslator::Invalid)
				{
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), 
						toString("MIS_REMV_BRICK_%u", _BrickSheets.size()), 
						params);
				}
				else
				{
					STRING_MANAGER::TParam p(STRING_MANAGER::bot, _Npc);
					params.push_back(p);
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(), 
						toString("MIS_REMV_BRICK_NPC_%u", _BrickSheets.size()), 
						params);
				}
			}
		}
	}
	vector<CSheetId> _BrickSheets; //_SheetId;
	bool		_Group;
	TAIAlias	_Npc;

	MISSION_ACTION_GETNEWPTR(CMissionActionUnlearnBrick)
};
MISSION_REGISTER_ACTION(CMissionActionUnlearnBrick,"unlearn_brick");

/// user receive money
// ----------------------------------------------------------------------------
class CMissionActionRecvMoney : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		bool ret = true;
		_SourceLine = line;
		if ( script.size() != 2 && script.size() != 3)
		{
			MISLOGSYNTAXERROR("<money> [: guild] OR <item><quality><factor> *[;<item><quality><factor>]");
			return false;
		}
		
		_Amount = 0;
		vector<string> multiArgs;
		CMissionParser::tokenizeString( script[1],";",multiArgs );
		if ( multiArgs.size() == 1 )
		{
			vector<string> args;
			CMissionParser::tokenizeString( multiArgs[0]," \t",args );
			if ( args.size() == 1 )
			{
				NLMISC::fromString(script[1], _Amount);
			}
			else if ( !CMissionParser::addItemPrice( _SourceLine, args,_Amount ) )
				ret = false;
		}
		else
		{
			for ( uint i = 0; i < multiArgs.size(); i++ )
			{
				vector<string> args;
				CMissionParser::tokenizeString( multiArgs[i]," \t",args );
				if ( !CMissionParser::addItemPrice(line, args,_Amount ) )
					ret = false;
			}
		}

		// We check for the guild option
		_Guild = false;
		for (std::vector< std::string >::const_iterator it = script.begin(); it != script.end(); ++it)
		{
			if (CMissionParser::getNoBlankString(*it) == "guild")
			{
				_Guild = true;
				break;
			}
		}

		return ret;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("recv_money");
		TLogContext_Item_Mission	logContext(instance->getMainEntity()->getId());

		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);

		// If the guild parameter is not set we just divide the money and give it to each entity
		if (!_Guild)
		{
			uint amount = _Amount / (uint)entities.size();
			if ( amount == 0 || _Amount % entities.size() )
				amount++;
			for ( uint i = 0; i < entities.size(); i++ )
			{
				CCharacter * user = PlayerManager.getChar( entities[i] );
				if ( user )
				{
					user->giveMoney( _Amount );
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
					params[0].Int = _Amount;
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_RECV_MONEY",params);
				}
			}
		}
		// Else we give the money to the guild
		else
		{
			if (entities.size() == 0)
				return;

			CCharacter * user = PlayerManager.getChar( entities[0] );
			if (!user)
			{
				LOGMISSIONACTION("recv_money : Invalid user");
				return;
			}

			CGuild * guild = CGuildManager::getInstance()->getGuildFromId(user->getGuildId());
			if (guild)
			{
				guild->addMoney(_Amount);
			}
			else
			{
				LOGMISSIONACTION("recv_money : Invalid guild id '" + NLMISC::toString(user->getGuildId()) + "'");
				return;
			}

			// tell everyone some money has been given to the guild
			for ( uint i = 0; i < entities.size(); i++ )
			{
				CCharacter * user = PlayerManager.getChar( entities[i] );
				if ( user )
				{
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
					params[0].Int = _Amount;
					PHRASE_UTILITIES::sendDynamicSystemMessage(user->getEntityRowId(),"MIS_GUILD_RECV_MONEY",params);
				}
			}
		}
	};
	uint _Amount;
	bool _Guild;

	MISSION_ACTION_GETNEWPTR(CMissionActionRecvMoney)
};
MISSION_REGISTER_ACTION(CMissionActionRecvMoney,"recv_money");

/// user receive fame
// ----------------------------------------------------------------------------
class CMissionActionRecvFame : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 && script.size() != 3)
		{
			MISLOGSYNTAXERROR("<faction> <value> [:guild]");
			return false;
		}
		vector<string> args;
		CMissionParser::tokenizeString( script[1]," \t",args );
		if ( args.size() != 2 )
		{
			MISLOGSYNTAXERROR("<faction> <value>");
			return false;
		}

		_Faction = CStaticFames::getInstance().getFactionIndex( args[0] );
		if (_Faction == CStaticFames::INVALID_FACTION_INDEX )
		{
			MISLOGERROR1("invalid faction '%s'", args[0].c_str());
			return false;
		}
		NLMISC::fromString(args[1], _Value);
		if ( _Value == 0 )
		{
			MISLOGERROR("fame = 0");
			return false;
		}

		// We check for the guild option
		_Guild = false;
		for (std::vector< std::string >::const_iterator it = script.begin(); it != script.end(); ++it)
		{
			if (CMissionParser::getNoBlankString(*it) == "guild")
			{
				_Guild = true;
				break;
			}
		}

		return true;
		
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("recv_fame");
		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);

		// If there is no "guild" parameter we give the fame to every user
		if (!_Guild)
		{
			for ( uint i = 0; i < entities.size(); i++ )
			{
				CEntityId eid = TheDataset.getEntityId(entities[i]);
				CFameInterface::getInstance().addFameIndexed(eid, _Faction, _Value, true);

				// Make the client refresh the icons on mission giver NPCs, at once
				CCharacter *character = PlayerManager.getChar(entities[i]);
				if (character)
					character->sendEventForMissionAvailabilityCheck();
			}
		}
		// Else we just give it to the guild
		else
		{
			if (entities.size() == 0)
				return;

			CCharacter * user = PlayerManager.getChar( entities[0] );
			if (!user)
			{
				LOGMISSIONACTION("recv_fame : Invalid user");
				return;
			}

			CGuild * guild = CGuildManager::getInstance()->getGuildFromId(user->getGuildId());
			if (guild)
			{
				CFameInterface::getInstance().addFameIndexed(guild->getEId(), _Faction, _Value, true);
			}
			else
			{
				LOGMISSIONACTION("recv_fame : Invalid guild id '" + NLMISC::toString(user->getGuildId()) + "'");
				return;
			}

			// tell everyone some money has been given to the guild
			for ( uint i = 0; i < entities.size(); i++ )
			{
				// Make the client refresh the icons on mission giver NPCs, at once
				CCharacter *character = PlayerManager.getChar(entities[i]);
				if (character)
					character->sendEventForMissionAvailabilityCheck();
			}
		}

	};
	uint32		_Faction;
	sint32		_Value;
	bool		_Guild;

	MISSION_ACTION_GETNEWPTR(CMissionActionRecvFame)
};
MISSION_REGISTER_ACTION(CMissionActionRecvFame,"recv_fame");

/// user receive XP
// ----------------------------------------------------------------------------
class CMissionActionRecvXp : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<skillForAdd> <value>");
			return false;
		}

		vector<string> args;
		CMissionParser::tokenizeString( script[1]," \t",args );
		if ( args.size() != 2 )
		{
			MISLOGSYNTAXERROR("improper format when tokenizing string for recv_xp, should be:<skillForAdd> <value>");
			return false;
		}

		// parse skill
		_Skill= SKILLS::toSkill(args[0]);
		if(_Skill==SKILLS::unknown)
		{
			MISLOGERROR1("skill unknown '%s'", args[0].c_str());
			return false;
		}

		// parse gain
		NLMISC::fromString(args[1], _Value);
		if (_Value <= 0)
		{
			MISLOGERROR("XP amount <= 0");
			return false;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("recv_xp");

		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);
		
		// distribute XP among all entities
		for (uint32 i = 0; i < entities.size(); i++)
		{
			CCharacter * user = PlayerManager.getChar( entities[i] );
			if (user != NULL)
			{
				TLogContext_Character_MissionRecvXp logContext(user->getId());
				sint32 xpGain;
				if (entities.size() == 1)
				{
					xpGain = _Value;
				}
				else
				{
					xpGain = sint32(double(_Value) / double(entities.size()) + 0.5);
				}

				user->addXpToSkillBranch( (double) xpGain, SKILLS::toString(_Skill) );
			}
		}
	};

	SKILLS::ESkills		_Skill;
	sint				_Value;

	MISSION_ACTION_GETNEWPTR(CMissionActionRecvXp)
};
MISSION_REGISTER_ACTION(CMissionActionRecvXp,"recv_xp");

/// user receive faction point
// ----------------------------------------------------------------------------
class CMissionActionRecvFactionPoint : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<faction_type> <faction_points>");
			return false;
		}
		vector<string> args;
		CMissionParser::tokenizeString( script[1]," \t",args );
		if ( args.size() != 2 )
		{
			MISLOGSYNTAXERROR("<faction_type> <faction_points>");
			return false;
		}

		_Clan = PVP_CLAN::fromString( args[0] );
		if ((_Clan < PVP_CLAN::BeginClans) || (_Clan > PVP_CLAN::EndClans))
		{
			MISLOGERROR1("invalid faction '%s'", args[0].c_str());
			return false;
		}
		NLMISC::fromString(args[1], _Value);
		if (_Value == 0)
		{
			MISLOGERROR("faction points = 0");
			return false;
		}
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("recv_faction_point");
		std::vector<TDataSetRow> entities;
		instance->getEntities(entities);

		for (uint32 i = 0; i < entities.size(); i++)
		{
			CCharacter * user = PlayerManager.getChar( entities[i] );
			if (user != NULL)
			{
				sint32 fpGain;
				if (entities.size() == 1)
				{
					fpGain = _Value;
				}
				else
				{
					fpGain = sint32(double(_Value) / double(entities.size()) + (_Value > 0 ? 0.5 : -0.5));
				}
				uint32 fp = (uint32) max( (sint32(user->getFactionPoint(_Clan)) + fpGain), sint32(0) );
				user->setFactionPoint(_Clan, fp, true);

				if (fpGain > 0)
					user->sendFactionPointGainMessage(_Clan, uint32(fpGain));
				else
					user->sendFactionPointLoseMessage(_Clan, uint32(-fpGain));
			}
		}
	};
	PVP_CLAN::TPVPClan	_Clan;
	sint32				_Value;

	MISSION_ACTION_GETNEWPTR(CMissionActionRecvFactionPoint)
};
MISSION_REGISTER_ACTION(CMissionActionRecvFactionPoint,"recv_faction_point");

/// bot does an action
// ----------------------------------------------------------------------------
class CMissionActionEmote : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		bool ret = true;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<bot><emote>");
			return false;
		}
		vector<string> vars;
		CMissionParser::tokenizeString(script[1]," \t",vars);
		if ( vars.size() != 2)
		{
			MISLOGSYNTAXERROR("<bot><emote>");
			return false;
		}

		// get speaking bot
		if ( !CMissionParser::parseBotName(vars[0],_Bot,missionData) )
			ret = false;
		
		uint32 emoteId = 0;
		string sEmote = CMissionParser::getNoBlankString(vars[1]);
		if ( !CMissionManager::getInstance()->getEmoteId(sEmote, emoteId) )
		{
			MISLOGERROR1("invalid emote %s", sEmote.c_str());
			ret = false;
		}
		_Emote = (MBEHAV::EBehaviour)(emoteId + MBEHAV::EMOTE_BEGIN);
		return ret;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("bot_emot");
		// get the bot
		TAIAlias alias;
		if ( _Bot == CAIAliasTranslator::Invalid )
			alias = instance->getGiver();
		else
			alias = _Bot;

		const CEntityId & botId = CAIAliasTranslator::getInstance()->getEntityId( alias);
		if ( botId == CEntityId::Unknown )
		{
			LOGMISSIONACTION("bot_emot : Invalid npc alias '" + CPrimitivesParser::aliasToString(alias) + "'");
			return;
		}
		CCreature * bot = CreatureManager.getCreature( botId );
		if ( !bot )
		{
			LOGMISSIONACTION("bot_emot : Invalid npc alias '" + botId.toString() + "'");
			return;
		}
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		CCharacter * user = PlayerManager.getChar( entities[0] );
		if ( !user )
		{
			LOGMISSIONACTION("bot_emot : Invalid user '" + toString(entities[0].getIndex()) + "'");
			return;
		}

		// set the bot heading : he must face the user
		const CEntityState & botPos = bot->getState();
		const CEntityState & userPos = user->getState();

		double dx = (double) ( user->getState().X - bot->getState().X );
		double dy = (double) ( user->getState().Y - bot->getState().Y );
		
		// send the heading to AI
		CSetBotHeadingMsg msg;
		msg.BotRowId = bot->getEntityRowId();
		msg.Heading = (float)atan2( dy,dx );
		CWorldInstances::instance().msgToAIInstance(bot->getInstanceNumber(), msg);
//		msg.send( "AIS" );

		MBEHAV::CBehaviour bh(_Emote);
		bh.Data = (uint16)CTickEventHandler::getGameCycle();
		bot->setBehaviour( bh );
	}
	MBEHAV::EBehaviour		_Emote;
	TAIAlias				_Bot;

	MISSION_ACTION_GETNEWPTR(CMissionActionEmote)
};
MISSION_REGISTER_ACTION(CMissionActionEmote,"bot_emot");

// ----------------------------------------------------------------------------
bool CMissionActionJump::buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
{
	_SourceLine = line;
	if ( script.size() != 2)
	{
		MISLOGSYNTAXERROR("<label>");
		return false;
	}
	Label = CMissionParser::getNoBlankString(script[1]);
	missionData.Jumps.push_back( Label );
	return true;
}

void CMissionActionJump::launch(CMission* instance, std::list< CMissionEvent * > & eventList)
{
	LOGMISSIONACTION("jump " + Label);
	// get the label description
	const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( instance->getTemplateId() );
	nlassert(templ);
	for ( uint i = 0; i < templ->JumpPoints.size(); i++ )
	{
		if ( templ->JumpPoints[i].Name == Label )
		{
			instance->jump( templ->JumpPoints[i].Step,templ->JumpPoints[i].Action, eventList );
			return;
		}
	}
	LOGMISSIONACTION("jump : Invalid jump point '" + Label + "'");
}

IMissionAction* CMissionActionJump::getNewPtr()
{
	CMissionActionJump * ptr = new CMissionActionJump;
	*ptr = *this;
	return ptr; 
}
MISSION_REGISTER_ACTION(CMissionActionJump,"jump");

///conditionnal jump
// ----------------------------------------------------------------------------
class CMissionActionCondJump : public CMissionActionJump
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<mission>:<label>");
			return false;
		}
		string sMiss = CMissionParser::getNoBlankString(script[1]);
		Mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( sMiss );
		if ( Mission == CAIAliasTranslator::Invalid )
		{
			MISLOGERROR1("invalid mission %s", sMiss.c_str());
			return false;
		}
		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_mission_done");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		for ( uint i = 0; i < entities.size(); i++ )
		{
			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( !user )
			{
				LOGMISSIONACTION("if_mission_done : invalid player '" + toString(entities[i].getIndex()) + "'");
				continue;
			}
			const std::map<TAIAlias, TMissionHistory> &mhs = user->getMissionHistories();
			std::map<TAIAlias, TMissionHistory>::const_iterator first(mhs.begin()), last(mhs.end());
			for (; first != last; ++first)
			{
				if (first->first == Mission && first->second.Successfull)
				{
					CMissionActionJump::launch( instance, eventList );
					return;
				}
			}
/*			for ( uint i = 0; i < user->getSuccessfulMissions().size(); i++ )
			{
				if ( user->getSuccessfulMissions()[i] == Mission )
				{
					CMissionActionJump::launch( instance );
					return;
				}
			}
*/
		}
	}
	TAIAlias	Mission;

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJump)
};
MISSION_REGISTER_ACTION(CMissionActionCondJump,"if_mission_done");

///conditionnal jump
// ----------------------------------------------------------------------------
class CMissionActionCondJumpBrick : public CMissionActionJump
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<sbrick> [;<sbrick]* : <label>");
			return false;
		}

		vector<string> bn;
		explode(script[1], string(";"), bn, true);

		if (bn.empty())
		{
			MISLOGERROR1("invalid brick list '%s'", script[1].c_str());
			return false;
		}
		for (uint i=0; i<bn.size(); ++i)
		{
			string brickName = CMissionParser::getNoBlankString(bn[i]);
			CSheetId brick(brickName + ".sbrick");
			if (brick == CSheetId::Unknown)
			{
				MISLOGERROR2("invalid brick (%uth brick) '%s'", i+1, brickName.c_str());
				return false;
			}
			bricks.push_back(brick);
			missionData.ChatParams.push_back(  make_pair(brickName, STRING_MANAGER::sbrick) );
		}

		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_bricks");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		for ( uint i = 0; i < entities.size(); i++ )
		{
			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( !user )
			{
				LOGMISSIONACTION("if_bricks : invalid player '" + toString(entities[i].getIndex()) + "'");
				continue;
			}

			const std::set<NLMISC::CSheetId> &knownBricks = user->getKnownBricks();
			uint j;
			for (j=0; j<bricks.size(); ++j)
			{
				if (knownBricks.find(bricks[j]) == knownBricks.end())
					break;
			}

			if (j == bricks.size())
			{
				// ok, this player has all the required bricks, jump !
				CMissionActionJump::launch( instance, eventList );
				return;
			}
		}
	}

	// the list of required brick to validate the test
	vector<CSheetId>	bricks;

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpBrick)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpBrick,"if_bricks");

///conditionnal jump
// ----------------------------------------------------------------------------
class CMissionActionCondJumpSkill : public CMissionActionJump
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<skill> <level> [;<skill> <level>]* : <label>");
			return false;
		}

		vector<string> sn;
		explode(script[1], string(";"), sn, true);

		if (sn.empty())
		{
			MISLOGERROR1("invalid skill list '%s'", script[1].c_str());
			return false;
		}
		for (uint i=0; i<sn.size(); ++i)
		{
			SKILLS::ESkills skill;
			sint32			level;

			vector<string> parts;
			explode(sn[i], string(" "), parts, true);

			if (parts.size() != 2)
			{
				MISLOGERROR2("invalid skill (%uth skill) '%s'", i, sn[i].c_str());
				return false;
			}

			skill = SKILLS::toSkill(parts[0]);
			if (skill == SKILLS::unknown)
			{
				MISLOGERROR2("invalid skill name (%uth skill) '%s'", i, parts[0].c_str());
				return false;
			}
			NLMISC::fromString(parts[1], level);
			if (level == 0)
			{
				MISLOGERROR2("invalid skill value (%uth skill) '%s'", i, sn[i].c_str());
				return false;
			}

			skills.push_back(make_pair(skill, level));
			missionData.ChatParams.push_back(  make_pair(parts[0], STRING_MANAGER::skill) );
		}

		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_skills");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		for ( uint i = 0; i < entities.size(); i++ )
		{
			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( !user )
			{
				LOGMISSIONACTION("if_skills : invalid player '" + toString(entities[i].getIndex()) + "'");
				continue;
			}

			uint j;
			for (j=0; j<skills.size(); ++j)
			{
				try
				{
					if (user->lookupStat(skills[j].first, SSkill::maxLvlReached) < skills[j].second)
						break;
				}
				catch(...)
				{}
			}

			if (j == skills.size())
			{
				// ok, this player has all the required skill level, jump !
				CMissionActionJump::launch( instance, eventList );
				return;
			}
		}
	}

	// the list of required brick to validate the test
	vector<pair<SKILLS::ESkills, sint32> >	skills;

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpSkill)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpSkill,"if_skills");

/// end
// ----------------------------------------------------------------------------
class CMissionActionEnd : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 1)
		{
			MISLOGSYNTAXERROR("no param expected");
			return false;
		}
		
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("end");
		instance->setProcessingState(CMission::Complete);
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionEnd)
};
MISSION_REGISTER_ACTION(CMissionActionEnd,"end");

/// failure
// ----------------------------------------------------------------------------
class CMissionActionFail : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 1)
		{
			MISLOGSYNTAXERROR("no param expected");
			return false;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("fail");
		instance->setProcessingState(CMission::Failed);
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionFail)
};
MISSION_REGISTER_ACTION(CMissionActionFail,"fail");

/// failure
// ----------------------------------------------------------------------------
class CMissionActionFailIfSDB : public IMissionAction
{
	string _SDBExpr;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<sdb_expr>");
			return false;
		}

		_SDBExpr = script[1];

		double res;
		if (!evaluateSDBExpr(_SDBExpr, res, true))
		{
			MISLOGERROR("SDB expression is invalid!");
			return false;
		}

		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("fail_if_sdb");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		double res;
		if (evaluateSDBExpr(_SDBExpr, res, false))
		{
			// if the expression is true, fail !!!
			if (res != 0.0)
			{
				instance->setProcessingState(CMission::Failed);
			}
		}
		else
		{
			LOGMISSIONACTION("fail_if_sdb : invalid expression (" + _SDBExpr + ")");
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionFailIfSDB)
};
MISSION_REGISTER_ACTION(CMissionActionFailIfSDB,"fail_if_sdb");

/// failure
// ----------------------------------------------------------------------------
class CMissionActionFailMissionCat : public IMissionAction
{
	string _MissionCategory;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if (script.size() != 2)
		{
			MISLOGSYNTAXERROR("<mission_category>");
			return false;
		}
		_MissionCategory = NLMISC::toLower(CMissionParser::getNoBlankString(script[1]));
		if (NLMISC::toLower(missionData.Template->MissionCategory) == _MissionCategory)
		{
			MISLOGERROR1("a mission cannot make fail its own category '%s'", _MissionCategory.c_str());
			return false;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("fail_mission_cat");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;
		CCharacter * pChar = PlayerManager.getChar(entities[0]);
		if (pChar == NULL)
			return;

		CMissionManager *pMM = CMissionManager::getInstance();
		std::map<TAIAlias, CMission*>::iterator itMiss = pChar->getMissionsBegin();
		while (itMiss != pChar->getMissionsEnd())
		{
			map<TAIAlias, CMission*>::iterator itNext = itMiss;
			++itNext;

			CMission *pMiss = itMiss->second;
			if (pMiss != NULL && pMiss != instance)	// Do not fail current mission even if its category matches
			{
				CMissionTemplate *pMissTemplate;
				bool bFailed = false;
				// Check the mission template category
				pMissTemplate = pMM->getTemplate(pMiss->getTemplateId());
				if (pMissTemplate != NULL)
				{
					if (NLMISC::toLower(pMissTemplate->MissionCategory) == _MissionCategory)
					{
						pMiss->onFailure(true, false);
						bFailed = true;
					}
				}

				// and the main mission template category
				if (!bFailed) // do not fail a mission twice 
				{
					pMissTemplate = pMM->getTemplate(pMiss->getMainMissionTemplateId());
					if (pMissTemplate != NULL)
					{
						if (NLMISC::toLower(pMissTemplate->MissionCategory) == _MissionCategory)
							pMiss->onFailure(true, false);
					}
				}
			}
			// next mission
			itMiss = itNext;
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionFailMissionCat)
};
MISSION_REGISTER_ACTION(CMissionActionFailMissionCat,"fail_mission_cat");

/// compass npc
// ----------------------------------------------------------------------------
class CMissionActionCompassNpc : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<bot>");
			return false;
		}
		if ( !CMissionParser::parseBotName(script[1],Alias,missionData) )
			return false;
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("add_compass_npc");
		TAIAlias alias;
		if ( Alias == CAIAliasTranslator::Invalid )
			alias = instance->getGiver();
		else
			alias = Alias;

		instance->addCompassTarget(alias,true);
	}
	TAIAlias Alias;

	MISSION_ACTION_GETNEWPTR(CMissionActionCompassNpc)
};
MISSION_REGISTER_ACTION(CMissionActionCompassNpc,"add_compass_npc");

/// compass npc
// ----------------------------------------------------------------------------
class CMissionActionCompassPlace : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<place>");
			return false;
		}
		string name = CMissionParser::getNoBlankString( script[1] );
		CPlace * place = CZoneManager::getInstance().getPlaceFromName( name );
		if ( ! place)
		{
			MISLOGERROR1("invalid place '%s'", name.c_str());
			return false;
		}
		PlaceId = place->getId();
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("add_compass_place");
		instance->addCompassTarget(PlaceId,false);
	}
	uint16 PlaceId;

	MISSION_ACTION_GETNEWPTR(CMissionActionCompassPlace)
};
MISSION_REGISTER_ACTION(CMissionActionCompassPlace,"add_compass_place");

/// remove compass npc
// ----------------------------------------------------------------------------
class CMissionActionRemoveCompassNpc : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<bot>");
			return false;
		}
		if ( !CMissionParser::parseBotName(script[1],Alias,missionData) )
			return false;
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("remove_compass_npc");
		TAIAlias alias;
		if ( Alias == CAIAliasTranslator::Invalid )
			alias = instance->getGiver();
		else
			alias = Alias;
		
		instance->removeCompassBot(alias);
	}
	TAIAlias Alias;

	MISSION_ACTION_GETNEWPTR(CMissionActionRemoveCompassNpc)
};
MISSION_REGISTER_ACTION(CMissionActionRemoveCompassNpc,"remove_compass_npc");

/// remove compass place
// ----------------------------------------------------------------------------
class CMissionActionRemoveCompassPlace : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<place>");
			return false;
		}
		string name = CMissionParser::getNoBlankString( script[1] );
		CPlace * place = CZoneManager::getInstance().getPlaceFromName( name );
		if ( ! place)
		{
			MISLOGERROR1("invalid place '%s'", name.c_str());
			return false;
		}
		PlaceId = place->getId();
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("remove_compass_place");
		instance->removeCompassPlace(PlaceId);
	}
	uint16 PlaceId;

	MISSION_ACTION_GETNEWPTR(CMissionActionRemoveCompassPlace)
};
MISSION_REGISTER_ACTION(CMissionActionRemoveCompassPlace,"remove_compass_place");


/// AI event
// ----------------------------------------------------------------------------
class CMissionActionAIEvent : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<group>;<event number>*[;<param>]");
			return false;
		}
		bool ret = true;

		vector<string> args;
		string separators(";");
		CMissionParser::tokenizeString(script[1], separators ,args);
		if (args.size() < 2)
		{
			MISLOGSYNTAXERROR("<group>;<event number>*[;<param>]");
			return false;
		}
		
		string sGroup = CMissionParser::getNoBlankString(args[0]);
		CAIAliasTranslator::getInstance()->getGroupAliasesFromName(sGroup, Groups);
		if ( Groups.empty() )
		{
			MISLOGERROR1("invalid group '%s'", sGroup.c_str() );
			ret = false;
		}
		NLMISC::fromString(args[1], EventId);
		if( EventId > 9)
		{
			MISLOGERROR1("invalid event '%s' ( [0-9] )", args[1].c_str() );
			ret = false;
		}

		for (uint32 i = 2; i < args.size(); ++i)
			Params.push_back(args[i]);

		return ret;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("ai_event");
		CUserEventMsg msg;
		msg.EventId = EventId;
		for ( uint i = 0; i < Groups.size(); i++ )
		{
			TDataSetRow giverRow = TheDataset.getDataSetRow( CAIAliasTranslator::getInstance()->getEntityId( instance->getGiver() ) );
			if (TheDataset.isAccessible(giverRow) )
			{
				CMirrorPropValueRO<uint32>	in(TheDataset, giverRow, DSPropertyAI_INSTANCE);
				msg.InstanceNumber = in;
				msg.GrpAlias = Groups[i];
				msg.Params = Params;
				CWorldInstances::instance().msgToAIInstance(in, msg);
			}
			else
			{
				const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( instance->getTemplateId() );
				nlassert(templ);
				LOGMISSIONACTION("ai_event : can't send event '" + toString(EventId) + "' from mission '" + 
							templ->getMissionName() + "' : mission giver invalid, can't retrieve AI instance");
			}
		}
	}
	uint8				EventId;
	vector<TAIAlias>	Groups;
	vector<string>		Params;

	MISSION_ACTION_GETNEWPTR(CMissionActionAIEvent)
};
MISSION_REGISTER_ACTION(CMissionActionAIEvent,"ai_event");

/// timer event
// ----------------------------------------------------------------------------
class CMissionActionTimer : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<delay>");
			return false;
		}
		NLMISC::fromString(script[1], Delay);
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("timer");
		instance->setTimer(Delay);
		instance->updateUsersJournalEntry();
	}
	NLMISC::TGameCycle	Delay;

	MISSION_ACTION_GETNEWPTR(CMissionActionTimer)
};
MISSION_REGISTER_ACTION(CMissionActionTimer,"timer");

/// set day period
// ----------------------------------------------------------------------------
class CMissionActionDayPeriod : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<hour> <min>; <hour> <min>");
			return false;
		}
		
		vector<string> args;
		splitString(script[1],";",args);
		if ( args.size() != 2)
		{
			MISLOGSYNTAXERROR("<hour> <min>; <hour> <min>");
			return false;
		}
		
		vector<string> params;
		CMissionParser::tokenizeString(args[0]," \t",params);
		if ( params.size() != 2 )
		{
			MISLOGSYNTAXERROR("<hour> <min>; <hour> <min>");
			return false;
		}
		uint hour;
		NLMISC::fromString(params[0], hour);
		uint min;
		NLMISC::fromString(params[1], min);
		if ( hour < 0 || hour >23 )
		{
			MISLOGERROR("hour must be between 0 and 23");
			return false;
		}
		if ( min < 0 || min >59 )
		{
			MISLOGERROR("minutes must be between 0 and 59");
			return false;
		}
		LowerBound = hour + float(min) / 60.0f;


		params.clear();
		CMissionParser::tokenizeString(args[1]," \t",params);
		if ( params.size() != 2 )
		{
			MISLOGSYNTAXERROR("<hour> <min>; <hour> <min>");
			return false;
		}
		NLMISC::fromString(params[0], hour);
		NLMISC::fromString(params[1], min);
		if ( hour < 0 || hour >23 )
		{
			MISLOGERROR("hour must be between 0 and 23");
			return false;
		}
		if ( min < 0 || min >59 )
		{
			MISLOGERROR("minutes must be between 0 and 59");
			return false;
		}
		UpperBound = hour +float(min)/60.0f;

		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("day_period");
		instance->setHourLowerBound(LowerBound);
		instance->setHourUpperBound(UpperBound);
	}

	float UpperBound;
	float LowerBound;

	MISSION_ACTION_GETNEWPTR(CMissionActionDayPeriod)
};
MISSION_REGISTER_ACTION(CMissionActionDayPeriod,"day_period");


/// season event
// ----------------------------------------------------------------------------
class CMissionActionSeason : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<season>");
			return false;
		}
		string str = CMissionParser::getNoBlankString( script[1] );
		if ( str == "none" )
			Season = EGSPD::CSeason::Invalid;
		else
		{
			Season = EGSPD::CSeason::fromString( str );
			if ( Season == EGSPD::CSeason::EndSeason || Season == EGSPD::CSeason::Invalid )
			{
				MISLOGSYNTAXERROR("<season>");
				return false;
			}
		}
		return true;
	}
	
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("season");
		instance->setSeason(Season);
	}
	
	EGSPD::CSeason::TSeason Season;

	MISSION_ACTION_GETNEWPTR(CMissionActionSeason)
};
MISSION_REGISTER_ACTION(CMissionActionSeason,"season");

// ----------------------------------------------------------------------------
// TODO : the log when this action will be used ...
// ---------------------------------------------
class CMissionActionRewardGroup : public IMissionAction
{
public:
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 1)
		{
			MISLOGSYNTAXERROR("no param expected");
			return false;
		}
		return true;
	}
	
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("reward");
		vector<TDataSetRow> entities;
		instance->getEntities(entities);
		if ( entities.empty() )
			return;
		CCharacter * user = PlayerManager.getChar(entities[0]);
		if ( !user )
			return;
		CTeam * team = TeamManager.getRealTeam(user->getTeamId());
		if ( !team )
			return;
		CRewardSharing * reward = new CRewardSharing();
		for  (uint i =  0; i< Phrases.size(); i++ )
		{
			reward->addPhrase(Phrases[i]);
		}
		for  (uint i =  0; i< MissionItems.size(); i++ )
		{
			CGameItemPtr item =	MissionItems[i].first.createItem(MissionItems[i].second);
			if ( item != NULL )
				reward->addItem(item);
		}
		for  (uint i =  0; i< StdItems.size(); i++ )
		{
			CGameItemPtr item = GameItemManager.createInGameItem(StdItems[i].Quality,StdItems[i].Quantity,StdItems[i].Sheet,CEntityId::Unknown,NULL);
			if ( item != NULL )
				reward->addItem(item);
		}
		team->rewardSharing(reward);
	}

	virtual bool addReward(const std::vector<std::string>&  script,const std::vector< std::pair<std::string,CMissionItem> >& items,uint32 line)
	{
		if ( script.size() != 2 )
		{
			MISLOG("<addReward> line %u: syntax error <item>|<action>:<parameters>",line);
			return false;
		}
		if ( !nlstricmp(script[0],"item"))
		{
			bool ret = true;
			vector<string> args;
			CMissionParser::tokenizeString( script[1]," \t",args );
			
			uint16 quantity = 1;
			if ( args.size() >= 2)
				NLMISC::fromString(args[1], quantity);
			
			for (uint i = 0; i < items.size(); i++ )
			{
				if ( !nlstricmp( items[i].first , args[0] ) )
				{
					if ( args.size() > 2 )
					{
						MISLOG("sline:%u ERROR : %s:'item:<item> [<quantity>]'", line);
						return false;
					}
					MissionItems.push_back( make_pair(items[i].second,quantity) );
					return true;
				}
			}
			
			CSheetId sheet = CSheetId( CMissionParser::getNoBlankString(args[0]) + ".sitem" );
			if ( sheet == CSheetId::Unknown )
			{
				MISLOG("sline:%u  syntax error sheetId '%s' is unknon", line, (CMissionParser::getNoBlankString(args[0]) + ".sitem").c_str() ) ;
				ret = false;
			}
			
			uint16 quality = 1;
			if ( args.size() == 3 )
			{
				NLMISC::fromString(args[2], quality);
				if ( quality == 0 )
				{
					MISLOG("sline:%u  syntax error quality = 0", line);
					ret = false;
				}
			}
			StdItems.push_back(CItemDesc());
			StdItems.back().Sheet = sheet;
			StdItems.back().Quality = quality;
			StdItems.back().Quantity = quantity;
			return ret;
		}
		else if ( !nlstricmp(script[0],"action"))
		{
			CSheetId sheet = CSheetId(CMissionParser::getNoBlankString(script[1]) + ".sphrase");
			if ( sheet == CSheetId::Unknown )
			{
				MISLOG("<addReward> line %u: invalid sheet %",line);
				return false;
			}
			Phrases.push_back(sheet);
			return true;
		}
		else
		{
			MISLOG("<addReward> line %u: syntax error invalid reward '%s'",line,script[0].c_str());
			return false;
		}
	}

	struct CItemDesc
	{
		CSheetId Sheet;
		uint16 Quality;
		uint16 Quantity;
	};

	vector<CSheetId>					Phrases;
	vector< pair<CMissionItem,uint16> >	MissionItems;
	vector<CItemDesc>					StdItems;
		

	MISSION_ACTION_GETNEWPTR(CMissionActionRewardGroup)
};
MISSION_REGISTER_ACTION(CMissionActionRewardGroup,"reward");

// ----------------------------------------------------------------------------
bool CMissionActionSetTeleport::buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
{
	SourceLine = line;
	if ( script.size() < 3 || script.size() > 5 )
	{
		MISLOGSYNTAXERROR("<bot>:<teleport>:[<once>]:[<phrase_id>*[;<param>]]");
		return false;
	}

	bool ret = true;
	if ( !CMissionParser::parseBotName(script[1],Bot,missionData) )
		ret = false;

	if ( script.size() == 2 )
	{
		DestinationIdx = InvalidSpawnZoneId;
		return true;
	}

	string name = CMissionParser::getNoBlankString( script[2] );
	DestinationIdx = CZoneManager::getInstance().getTpSpawnZoneIdByName( name );
	if ( DestinationIdx == InvalidSpawnZoneId )
	{
		MISLOGERROR1("invalid tp spawn zone '%s'", name.c_str());
		ret = false;
	}
	Once = false;
	bool noText = true;
	Params.resize(1);
	Params[0].Type = STRING_MANAGER::place;
	Params[0].Identifier = name;
	missionData.ChatParams.push_back( make_pair( name,STRING_MANAGER::place ) );
	for ( uint i = 3; i < script.size();i++ )
	{
		if ( CMissionParser::getNoBlankString( script[3] ) == "once" )
		{
			Once = true;
		}
		else
		{
			TVectorParamCheck params;
			ret = ret && CMissionParser::parseParamText(line, script[i], PhraseId, params );
			Params.resize( 1 + params.size() );
			std::copy( params.begin(), params.end(), Params.begin() + 1);
			noText = false;
		}
	}
	if ( noText )
	{
		PhraseId = "MIS_TELEPORT";
	}
	nlassert( missionData.Template );
	missionData.Template->Teleports.push_back(this);
	Index = (uint32)missionData.Template->Teleports.size();
	return ret;
}

void CMissionActionSetTeleport::launch(CMission* instance, std::list< CMissionEvent * > & eventList)
{
	LOGMISSIONACTION("set_teleport");
	if ( DestinationIdx != InvalidSpawnZoneId )
		instance->addTeleport(Index);
	else
		instance->removeTeleport( Index );
}

bool CMissionActionSetTeleport::solveTextsParams( CMissionSpecificParsingData & missionData )
{
	return CMissionParser::solveTextsParams( _SourceLine, Params,missionData );
}
MISSION_REGISTER_ACTION(CMissionActionSetTeleport,"set_teleport");

// ----------------------------------------------------------------------------
class CMissionActionTeleport : public IMissionAction
{
protected:
	uint16 DestinationIdx;
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<teleport>");
			return false;
		}
		string name = CMissionParser::getNoBlankString( script[1] );
		DestinationIdx = CZoneManager::getInstance().getTpSpawnZoneIdByName( name );
		if ( DestinationIdx == InvalidSpawnZoneId )
		{
			MISLOGERROR1("invalid tp spawn zone '%s'", name.c_str());
			return false;
		}
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("teleport");
		vector<TDataSetRow> entities;
		instance->getEntities( entities );
		const CTpSpawnZone* tp = CZoneManager::getInstance().getTpSpawnZone( DestinationIdx );
		if ( ! tp )
		{
			LOGMISSIONACTION("teleport : invalid tp index " + toString(DestinationIdx) + "TP is NULL");
			return;
		}
		
		sint32 x,y,z;
		float heading;
		for ( uint i = 0; i < entities.size(); i++ )
		{
			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( user )
			{
				tp->getRandomPoint(x,y,z,heading);
				user->forbidNearPetTp();
				user->tpWanted(x,y,z,true,heading);
			}
		}
	};

	MISSION_ACTION_GETNEWPTR(CMissionActionTeleport)
};
MISSION_REGISTER_ACTION(CMissionActionTeleport,"teleport");

// ----------------------------------------------------------------------------
class CMissionActionTeleportToPos : public IMissionAction
{
protected:
	sint32		X, Y, Z;
	float		Heading;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() < 3 )
		{
			MISLOGSYNTAXERROR("<teleport_x_y>");
			return false;
		}
		X = (sint32)(atof( CMissionParser::getNoBlankString( script[1] ).c_str() ) * 1000.0f);
		Y = (sint32)(atof( CMissionParser::getNoBlankString( script[2] ).c_str() ) * 1000.0f);
		Z = 0;
		Heading = -1.0f;
		if ( script.size() > 3 )
		{
			Z = (sint32)(atof( CMissionParser::getNoBlankString( script[3] ).c_str() ) * 1000.0f);
			if ( script.size() > 4 )
			{
				Heading = (float)atof( CMissionParser::getNoBlankString( script[4] ).c_str() );
			}
		}
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("teleport_x_y");
		vector<TDataSetRow> entities;
		instance->getEntities( entities );
		
		for ( uint i = 0; i < entities.size(); i++ )
		{
			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( user )
			{
				user->forbidNearPetTp();
				user->tpWanted( X, Y, Z, (Heading != -1.0f), Heading );
			}
		}
	};

	MISSION_ACTION_GETNEWPTR(CMissionActionTeleportToPos)
};
MISSION_REGISTER_ACTION(CMissionActionTeleportToPos,"teleport_x_y");

// ----------------------------------------------------------------------------
class CMissionActionSetCult : public IMissionAction
{
public:
	PVP_CLAN::TPVPClan Cult;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<Cult>");
			return false;
		}
		Cult = PVP_CLAN::fromString(CMissionParser::getNoBlankString(script[1]).c_str());
		if ( ! ((Cult >= PVP_CLAN::BeginCults) && (Cult <= PVP_CLAN::EndCults) || Cult == PVP_CLAN::Neutral))
		{
			MISLOGERROR1("'%s' is not a cult !", script[1].c_str());
			return false;
		}
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("set_cult");

		vector<TDataSetRow> entities;
		instance->getEntities( entities );

		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("set_cult : invalid player row id " + entities[i].toString());
				continue;
			}
			pChar->setDeclaredCult(Cult);
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSetCult)
};
MISSION_REGISTER_ACTION(CMissionActionSetCult,"set_cult");

// ----------------------------------------------------------------------------
class CMissionActionSetCiv : public IMissionAction
{
public:
	PVP_CLAN::TPVPClan Civ;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<Civ>");
			return false;
		}
		Civ = PVP_CLAN::fromString(CMissionParser::getNoBlankString(script[1]).c_str());
		if ( ! ((Civ >= PVP_CLAN::BeginCivs) && (Civ <= PVP_CLAN::EndCivs) || Civ == PVP_CLAN::Neutral))
		{
			MISLOGERROR1("'%s' is not a civ !", script[1].c_str());
			return false;
		}
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("set_civ");

		vector<TDataSetRow> entities;
		instance->getEntities( entities );

		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("set_civ : invalid player row id " + entities[i].toString());
				continue;
			}
			pChar->setDeclaredCiv(Civ);
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSetCiv)
};
MISSION_REGISTER_ACTION(CMissionActionSetCiv,"set_civ");


// ----------------------------------------------------------------------------
class CMissionActionSetGuildCult : public CMissionActionSetCult
{
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("set_guild_cult");

		vector<TDataSetRow> entities;
		instance->getEntities( entities );

		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (pChar)
			{
				CGuild * pGuild = CGuildManager::getInstance()->getGuildFromId( pChar->getGuildId() );
				if ( pGuild )
				{
					pGuild->setDeclaredCult(Cult);
				}
				else
				{
					LOGMISSIONACTION("set_guild_cult : player row id " + entities[i].toString() + " have no guild");
				}
			}
			else
			{
				LOGMISSIONACTION("set_guild_cult : invalid player row id " + entities[i].toString());
			}
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSetGuildCult)
};
MISSION_REGISTER_ACTION(CMissionActionSetGuildCult,"set_guild_cult");


// ----------------------------------------------------------------------------
class CMissionActionSetGuildCiv : public CMissionActionSetCiv
{
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("set_guild_civ");

		vector<TDataSetRow> entities;
		instance->getEntities( entities );

		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (pChar)
			{
				CGuild * pGuild = CGuildManager::getInstance()->getGuildFromId( pChar->getGuildId() );
				if ( pGuild )
				{
					pGuild->setDeclaredCiv(Civ);
				}
				else
				{
					LOGMISSIONACTION("set_guild_civ : player row id " + entities[i].toString() + " have no guild");
				}
			}
			else
			{
				LOGMISSIONACTION("set_guild_civ : invalid player row id " + entities[i].toString());
			}
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSetGuildCiv)
};
MISSION_REGISTER_ACTION(CMissionActionSetGuildCiv,"set_guild_civ");


// ----------------------------------------------------------------------------
class CMissionActionRecvChargePoint : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<_ChargesPoints>");
			return false;
		}
		NLMISC::fromString(script[1], _ChargesPoints);
		if (_ChargesPoints == 0)
		{
			MISLOGERROR("_ChargesPoints = 0");
			return false;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("recv_charge_point");
		//		CMissionInstanceGuild * mission = dynamic_cast<CMissionInstanceGuild*>(instance);
		vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if (entities.size() != 1)
		{
			LOGMISSIONACTION("recv_charge_point : mission " + toString(instance->getTemplateId()) +" need 1 mission owner, found " + toString(entities.size()));
		}
		else
		{
			CCharacter *c = PlayerManager.getChar(entities.front());
			if (c == NULL)
			{
				LOGMISSIONACTION("recv_charge_point : mission " + toString(instance->getTemplateId()) +" Can't find character info");
			}
			else
			{
				/// todo guild
				/*
				CGuild *guild = c->getGuild();
				if (guild == NULL)
				{
					nlwarning("<MISSIONS> The character %u don't have a guild (in revc_charge_point in mission %u)", c->getId().toString().c_str(), instance->getTemplate()->Alias);
				}
				else
				{
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
					params[0].Int = _ChargesPoints;
					/// todo charge
					//guild->addChargePoints( _ChargesPoints );
					//guild->sendMessageToMembers( "GUILD_GAIN_CHARGE_POINT", params);
					CCharacter::sendDynamicSystemMessage( c->getId(), "MIS_RECV_CHARGE_POINT", params );

				}
				*/
			}
		}
	};
	uint32 _ChargesPoints;

	MISSION_ACTION_GETNEWPTR(CMissionActionRecvChargePoint)
};
MISSION_REGISTER_ACTION(CMissionActionRecvChargePoint,"recv_charge_point");

// ----------------------------------------------------------------------------
class CMissionActionGiveOutpostControl : public IMissionAction
{
	string		_OutpostName;
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<outpost_name>");
			return false;
		}
		_OutpostName = CMissionParser::getNoBlankString( script[1] );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("give_control");
		/// todo charge
		/*
		CMissionInstanceGuild * mission = dynamic_cast<CMissionInstanceGuild*>(instance);
		if (mission)
		{
			CGuild * guild = mission->getGuild();
			if ( guild && guild->getOwnedCharge() && guild->getOwnedCharge()->getOutpost() )
			{
				COutpost *op = COutpostManager::getInstance().getOutpostByName(_OutpostName);
				if (op == NULL)
				{
					nlwarning("GiveOutpostControl : unknown outpost '%s'", _OutpostName.c_str());
				}
				else
				{
					if (guild->getOwnedCharge()->getOutpost() != op)
					{
						nlwarning("GiveOutpostControl : the guild '%s' is associated to outpost '%s', NOT to '%s'", 
							guild->getGuildName().toString().c_str(),
							guild->getOwnedCharge()->getOutpost()->getName().c_str(),
							_OutpostName.c_str());
					}
					else
					{
						guild->getOwnedCharge()->getOutpost()->setState(ZCSTATE::GuildInPeace);
						guild->clearChargePoints();
						CMissionEventOutpostGain * gain = new CMissionEventOutpostGain;
						eventList.push_back( gain );
						guild->sendMessageToMembers( "GUILD_OUTPOST_CONTROL" );
					}
				}
			}
			else
				nlwarning( "<MISSIONS> : give outpost control non guild mission %u, no guild in mission", instance->getTemplate()->Alias );
		}
		else
			nlwarning( "<MISSIONS> : give outpost control in non guild mission %u", instance->getTemplate()->Alias );
			*/
	};

	MISSION_ACTION_GETNEWPTR(CMissionActionGiveOutpostControl)
};
MISSION_REGISTER_ACTION(CMissionActionGiveOutpostControl,"give_control");

// ----------------------------------------------------------------------------
class CMissionActionOutpostBuilding : public IMissionAction
{
	CSheetId SheetId;
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<building desc>");
			return false;
		}
		string sheet = CMissionParser::getNoBlankString(script[1]) + ".building";
		SheetId = CSheetId( sheet );
		if ( SheetId == CSheetId::Unknown )
		{
			MISLOGERROR1("invalid sheet %s", sheet.c_str());
			return false;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("building");
		/// todo charge
		/*
		CMissionInstanceGuild * mission = dynamic_cast<CMissionInstanceGuild*>(instance);
		if (mission)
		{
			CGuild * guild = mission->getGuild();
			if ( guild && guild->getOwnedCharge() && guild->getOwnedCharge()->getOutpost() )
			{
				guild->getOwnedCharge()->getOutpost()->addPower( SheetId );
				guild->sendMessageToMembers( "GUILD_OUTPOST_BUILDING" );
			}
			else
				nlwarning( "<MISSIONS> : give outpost control non guild mission %u, no guild in mission", instance->getTemplate()->Alias );
		}
		else
			nlwarning( "<MISSIONS> : give outpost control in non guild mission %u", instance->getTemplate()->Alias );
			*/
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionOutpostBuilding)
};
MISSION_REGISTER_ACTION(CMissionActionOutpostBuilding,"building");

// ----------------------------------------------------------------------------
class CMissionActionDeclareWar : public IMissionAction
{
	CSheetId SheetId;
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if( script.size() != 1 )
		{
			MISLOGSYNTAXERROR("no param expected");
			return false;
		}
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("declare_war");
		/// todo charge
		/*
		CMissionInstanceGuild * mission = dynamic_cast<CMissionInstanceGuild*>(instance);
		if (mission)
		{
			CGuild * guild = mission->getGuild();
			if ( guild && guild->getOwnedCharge() && guild->getOwnedCharge()->getOutpost() )
			{
				guild->getOwnedCharge()->getOutpost()->setState( ZCSTATE::TribeInWar );
				guild->sendMessageToMembers( "GUILD_DECLARE_WAR" );
			}
			else
				nlwarning( "<MISSIONS> : CMissionActionDeclareWar mission %u, no guild in mission", instance->getTemplate()->Alias );
		}
		else
			nlwarning( "<MISSIONS> : CMissionActionDeclareWar in non guild mission %u", instance->getTemplate()->Alias );
			*/
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionDeclareWar)
};
MISSION_REGISTER_ACTION(CMissionActionDeclareWar,"declare_war");

// ----------------------------------------------------------------------------
class CMissionActionRecvGuildXp : public IMissionAction
{
	uint16 Amount;
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<amount>");
			return false;
		}
		NLMISC::fromString(script[1], Amount);
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("recv_guild_xp");
		/// todo charge
		/*
		CMissionInstanceGuild * mission = dynamic_cast<CMissionInstanceGuild*>(instance);
		if (mission)
		{
			CGuild * guild = mission->getGuild();
			if ( guild )
			{
				guild->addXp(Amount);
				guild->sendMessageToMembers( "GUILD_XP_GAIN" );
			}
			else
				nlwarning( "<MISSIONS> : CMissionActionRecvGuildXp mission %u, no guild in mission", instance->getTemplate()->Alias );
		}
		else
			nlwarning( "<MISSIONS> : CMissionActionRecvGuildXp non guild mission %u", instance->getTemplate()->Alias );
			*/
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionRecvGuildXp)
};
MISSION_REGISTER_ACTION(CMissionActionRecvGuildXp,"recv_guild_xp");


// ----------------------------------------------------------------------------
class CMissionActionInside : public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if( script.size() != 2 && script.size() != 3 )
		{
			MISLOGSYNTAXERROR("<place> [:delay]");
			return false;
		}
		string placeStr = CMissionParser::getNoBlankString( script[1] );
		CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr ) ;
		if ( !place )
		{
			MISLOGERROR1("invalid place '%s'", placeStr.c_str());
			return false;
		}
		Place = place->getId();
		if ( script.size() == 3 )
			NLMISC::fromString(script[2], Delay);
		else
			Delay = 300;
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("inside");
		CPlace * place = CZoneManager::getInstance().getPlaceFromId( Place ) ;
		nlassert(place);
		EGSPD::CMissionInsidePlacePD* placePD = instance->addToInsidePlaces( place->getAlias() );
		nlassert(placePD);
		placePD->setDelay( Delay );
		CMissionManager::getInstance()->checkPlaceConstraints( instance );
	}
	uint16 Place;
	NLMISC::TGameCycle Delay;

	MISSION_ACTION_GETNEWPTR(CMissionActionInside)
};
MISSION_REGISTER_ACTION(CMissionActionInside,"inside");

// ----------------------------------------------------------------------------
class CMissionActionOutside: public IMissionAction
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if( script.size() != 2 && script.size() != 3 )
		{
			MISLOGSYNTAXERROR("<place> [:delay]");
			return false;
		}
		string placeStr = CMissionParser::getNoBlankString( script[1] );
		CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
		if ( !place )
		{
			MISLOGERROR1("invalid place '%s'", placeStr.c_str());
			return false;
		}
		Place = place->getId();
		if ( script.size() == 3 )
			NLMISC::fromString(script[2], Delay);
		else
			Delay = 300;
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("outside");
		CPlace * place = CZoneManager::getInstance().getPlaceFromId( Place ) ;
		nlassert(place);
		EGSPD::CMissionOutsidePlacePD* placePD = instance->addToOutsidePlaces( place->getAlias() );
		nlassert(placePD);
		placePD->setDelay( Delay );
		CMissionManager::getInstance()->checkPlaceConstraints( instance );
	}
	uint16 Place;
	NLMISC::TGameCycle Delay;

	MISSION_ACTION_GETNEWPTR(CMissionActionOutside)
};
MISSION_REGISTER_ACTION(CMissionActionOutside,"outside");

// ----------------------------------------------------------------------------
class CMissionActionCancelInside : public IMissionAction
{
	uint16 Amount;
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<place>");
			return false;
		}
		string placeStr = CMissionParser::getNoBlankString( script[1] );
		CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
		if ( !place )
		{
			MISLOGERROR1("invalid place '%s'", placeStr.c_str());
			return false;
		}
		Place = place->getId();
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("cancel_inside");
		CPlace * place = CZoneManager::getInstance().getPlaceFromId( Place ) ;
		nlassert(place);
		instance->deleteFromInsidePlaces( place->getAlias() );
		CMissionManager::getInstance()->cleanPlaceConstraint( instance, place->getAlias() );
	}
	uint16 Place;

	MISSION_ACTION_GETNEWPTR(CMissionActionCancelInside)
};
MISSION_REGISTER_ACTION(CMissionActionCancelInside,"cancel_inside");

// ----------------------------------------------------------------------------
class CMissionActionCancelOutside: public IMissionAction
{
	uint16 Amount;
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<place>");
			return false;
		}
		string placeStr = CMissionParser::getNoBlankString( script[1] );
		CPlace * place = CZoneManager::getInstance().getPlaceFromName( placeStr );
		if ( !place )
		{
			MISLOGERROR1("invalid place '%s'", placeStr.c_str());
			return false;
		}
		Place = place->getId();
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("cancel_outside");
		CPlace * place = CZoneManager::getInstance().getPlaceFromId( Place ) ;
		nlassert(place);
		instance->deleteFromOutsidePlaces( place->getAlias() );
		CMissionManager::getInstance()->cleanPlaceConstraint( instance, Place );
	}
	uint16 Place;
	
	MISSION_ACTION_GETNEWPTR(CMissionActionCancelOutside)
};
MISSION_REGISTER_ACTION(CMissionActionCancelOutside,"cancel_outside");

// ----------------------------------------------------------------------------
class CMissionActionSpawnMission: public IMissionAction
{
protected:

	TAIAlias	Mission;	
	TAIAlias	NPCOwner;	// NPC giver the mission have to be attached at spawn time
	bool		Guild;

protected:

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3 && script.size() != 4)
		{
			MISLOGSYNTAXERROR("<mission_name> : <giver_name> [: guild]");
			return false;
		}
		string name = CMissionParser::getNoBlankString( script[1] );
		Mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( name );
		if ( Mission == CAIAliasTranslator::Invalid )
		{
			MISLOGERROR1("invalid mission '%s'", name.c_str());
			return false;
		}
		
		// Get the owner of the mission that will be spawn

		name = CMissionParser::getNoBlankString( script[2] );
		vector<TAIAlias> vRet;

		if (name != "giver")
		{
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName(name, vRet);
			if (vRet.size() == 0)
			{
				MISLOG("sline:%u WARNING %s : name '%s' give no alias", line, script[0].c_str(), name.c_str());
			}
			else if (vRet.size() != 1)
			{
				MISLOG("sline:%u WARNING %s : name '%s' give multiple aliases", line, script[0].c_str(), name.c_str());
			}
		}
		
		NPCOwner = CAIAliasTranslator::Invalid;
		if (vRet.size() > 0)
			NPCOwner = vRet[0];

		// We check for the guild option
		Guild = false;
		for (std::vector< std::string >::const_iterator it = script.begin(); it != script.end(); ++it)
		{
			if (CMissionParser::getNoBlankString(*it) == "guild")
			{
				Guild = true;
				break;
			}
		}

		return true;
	}

	void launchExt(CMission* instance, std::list< CMissionEvent * > & eventList, TAIAlias mainMission)
	{
		string sDebugBotName;
		if (NPCOwner == CAIAliasTranslator::Invalid)
		{
			CAIAliasTranslator::getInstance()->getNPCNameFromAlias(instance->getGiver(), sDebugBotName);
			nlassert(instance);
			CMissionEventAddMission * event = new CMissionEventAddMission( instance->getGiver(), Mission, mainMission, Guild );
			eventList.push_back( event );
		}
		else
		{
			CAIAliasTranslator::getInstance()->getNPCNameFromAlias(NPCOwner, sDebugBotName);
			CMissionEventAddMission * event = new CMissionEventAddMission( NPCOwner, Mission, mainMission, Guild );
			eventList.push_back( event );
		}
		LOGMISSIONACTION("spawn_mission bot:" + sDebugBotName + " newmiss:" + CPrimitivesParser::aliasToString(Mission)
			+ " mainmiss:" + CPrimitivesParser::aliasToString(mainMission));
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		launchExt(instance, eventList, CAIAliasTranslator::Invalid);
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSpawnMission)
};
MISSION_REGISTER_ACTION(CMissionActionSpawnMission,"spawn_mission");

// ----------------------------------------------------------------------------
class CMissionActionChainMission : public CMissionActionSpawnMission
{
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		if ( !CMissionActionSpawnMission::buildAction( line, script, globalData, missionData ) )
			return false;
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("chain_mission");

		CMissionGuild *pMG = dynamic_cast<CMissionGuild*>(instance);
		if (pMG != NULL)
			pMG->setChained();

		CMissionTeam *pMT = dynamic_cast<CMissionTeam*>(instance);
		if (pMT != NULL)
			pMT->setChained();

		CMissionSolo *pMS = dynamic_cast<CMissionSolo*>(instance);
		if (pMS != NULL)
			pMS->setChained();

		// if no main mission this is the first chain so give the current mission id as main mission for chained one
		if (instance->getMainMissionTemplateId() == CAIAliasTranslator::Invalid)
			launchExt(instance, eventList, instance->getTemplateId());
		else // else transfert main mission template id
			launchExt(instance, eventList, instance->getMainMissionTemplateId());
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionChainMission)
};
MISSION_REGISTER_ACTION(CMissionActionChainMission,"chain_mission");

// ----------------------------------------------------------------------------
class CMissionActionEncycloUnlock : public IMissionAction
{
	uint32 AlbumNb;
	uint32 ThemaNb;
	
	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
						CMissionGlobalParsingData & globalData, 
						CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		if( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<album> <thema>");
			return false;
		}

		vector<string> vars;
		CMissionParser::tokenizeString( script[1]," \t",vars );
		if ( vars.size() != 2 )
		{
			MISLOGSYNTAXERROR("<album> <thema>");
			return false;
		}
		
		NLMISC::fromString(vars[0], AlbumNb);
		NLMISC::fromString(vars[1], ThemaNb);

		return true;
	}


	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("encyclo_unlock");
		vector<TDataSetRow> entities;
		instance->getEntities(entities);

		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("encyclo_unlock : invalid player row id " + entities[i].toString());
				continue;
			}
			pChar->getEncyclopedia().unlockThema(AlbumNb, ThemaNb);
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionEncycloUnlock)
};
MISSION_REGISTER_ACTION(CMissionActionEncycloUnlock,"encyclo_unlock");

// ----------------------------------------------------------------------------
class CMissionActionGameEventSubscribe : public IMissionAction
{

	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
		CMissionGlobalParsingData & globalData, 
		CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		// For the moment there are no parameters
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("game_event_subscribe");
		vector<TDataSetRow> entities;
		instance->getEntities(entities);
		
		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("game_event_subscribe : invalid player row id " + entities[i].toString());
				continue;
			}
			pChar->getGameEvent().subscribe();
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionGameEventSubscribe)
};
MISSION_REGISTER_ACTION(CMissionActionGameEventSubscribe,"game_event_subscribe");

// ----------------------------------------------------------------------------
class CMissionActionGameEventReset : public IMissionAction
{

	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
		CMissionGlobalParsingData & globalData, 
		CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		// For the moment there are no parameters
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("game_event_reset");
		vector<TDataSetRow> entities;
		instance->getEntities(entities);
		
		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("game_event_reset : invalid player row id " + entities[i].toString());
				continue;
			}
			pChar->getGameEvent().reset();
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionGameEventReset)
};
MISSION_REGISTER_ACTION(CMissionActionGameEventReset, "game_event_reset");

// ----------------------------------------------------------------------------
class CMissionActionSetEventFaction : public IMissionAction
{
	string EventFaction;

	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
						CMissionGlobalParsingData & globalData, 
						CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		if( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<event_faction>");
			return false;
		}
		EventFaction = CMissionParser::getNoBlankString( script[1] );
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("set_event_faction");
		vector<TDataSetRow> entities;
		instance->getEntities(entities);

		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("set_event_faction : invalid player row id " + entities[i].toString());
				continue;
			}
			pChar->getGameEvent().setEventFaction(EventFaction);
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSetEventFaction)
};
MISSION_REGISTER_ACTION(CMissionActionSetEventFaction,"set_event_faction");

// ----------------------------------------------------------------------------
class CMissionActionSetRespawnPoints : public IMissionAction
{
	CONTINENT::TContinent	_Continent;
	vector<uint16>			_RespawnPoints;
	bool					_HideOthers;

	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
		CMissionGlobalParsingData & globalData, 
		CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		if (script.size() < 3 || script.size() > 4)
		{
			MISLOGSYNTAXERROR("<continent> : *;<spawn_zone_name> [: <hide_others=true|false>]");
			return false;
		}

		_Continent = CONTINENT::toContinent(CMissionParser::getNoBlankString(script[1]));
		nlassert(_Continent != CONTINENT::UNKNOWN);

		vector<string> args;
		CMissionParser::tokenizeString(script[2], ";", args);

		// check that the given respawn points exist and are all in the same continent
		CONTINENT::TContinent lastContinent;
		for (uint i = 0; i < args.size(); i++)
		{
			string respawnPointName = CMissionParser::getNoBlankString(args[i]);
			uint16 respawnPoint = CZoneManager::getInstance().getTpSpawnZoneIdByName(respawnPointName);
			if (respawnPoint == InvalidSpawnZoneId)
			{
				MISLOGERROR1("unknown tp spawn zone: '%s'", respawnPointName.c_str());
				return false;
			}

			const CTpSpawnZone * spawnZone = CZoneManager::getInstance().getTpSpawnZone(respawnPoint);
			if (spawnZone == NULL)
			{
				MISLOGERROR1("NULL tp spawn zone: '%s'", respawnPointName.c_str());
				return false;
			}

			if (i > 0)
			{
				if (spawnZone->getContinent() != lastContinent)
				{
					MISLOGERROR("all spawn zones must be in the same continent!");
					return false;
				}
			}

			_RespawnPoints.push_back(respawnPoint);
			lastContinent = spawnZone->getContinent();
		}

		// by default hide other respawn points
		_HideOthers = true;
		if (script.size() >= 4)
		{
			if (CMissionParser::getNoBlankString(script[3]) == "false")
			{
				if (lastContinent == _Continent)
				{
					_HideOthers = false;
				}
				else
				{
					MISLOGERROR1("spawn zones are not in the continent '%s', hide_others MUST be true", CONTINENT::toString(_Continent).c_str());
					return false;
				}
			}
		}

		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("set_respawn_points");
		vector<TDataSetRow> entities;
		instance->getEntities(entities);
		
		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("set_respawn_points : invalid player row id " + entities[i].toString());
				continue;
			}

			pChar->getRespawnPoints().setMissionRespawnPoints(_Continent, _RespawnPoints, _HideOthers);
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSetRespawnPoints)
};
MISSION_REGISTER_ACTION(CMissionActionSetRespawnPoints,"set_respawn_points");

// ----------------------------------------------------------------------------
class CMissionActionSDBSet : public IMissionAction
{
	string _SDBPath;
	sint32 _SDBValue;

	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
		CMissionGlobalParsingData & globalData, 
		CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		if (script.size() != 3)
		{
			MISLOGSYNTAXERROR("<sdb_path> : <value>");
			return false;
		}

		_SDBPath = CMissionParser::getNoBlankString(script[1]);
		nlassert(!_SDBPath.empty());

		NLMISC::fromString(CMissionParser::getNoBlankString(script[2]), _SDBValue);

		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("sdb_set");
		if (!CStatDB::getInstance()->valueSet(_SDBPath, _SDBValue))
		{
			LOGMISSIONACTION(toString("sdb_set : invalid path : '%s'", _SDBPath.c_str()));
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSDBSet)
};
MISSION_REGISTER_ACTION(CMissionActionSDBSet,"sdb_set");

// ----------------------------------------------------------------------------
class CMissionActionSDBAdd : public IMissionAction
{
	string _SDBPath;
	sint32 _SDBDelta;

	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
		CMissionGlobalParsingData & globalData, 
		CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		if (script.size() != 3)
		{
			MISLOGSYNTAXERROR("<sdb_path> : <value>");
			return false;
		}

		_SDBPath = CMissionParser::getNoBlankString(script[1]);
		nlassert(!_SDBPath.empty());

		NLMISC::fromString(CMissionParser::getNoBlankString(script[2]), _SDBDelta);

		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("sdb_add");
		if (!CStatDB::getInstance()->valueAdd(_SDBPath, _SDBDelta))
		{
			LOGMISSIONACTION(toString("sdb_add : invalid path : '%s'", _SDBPath.c_str()));
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSDBAdd)
};
MISSION_REGISTER_ACTION(CMissionActionSDBAdd,"sdb_add");

// ----------------------------------------------------------------------------
class CMissionActionSDBPlayerAdd : public IMissionAction
{
	string _SDBPath;
	sint32 _SDBDelta;

	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
		CMissionGlobalParsingData & globalData, 
		CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		if (script.size() != 3)
		{
			MISLOGSYNTAXERROR("<sdb_path> : <delta>");
			return false;
		}

		_SDBPath = CMissionParser::getNoBlankString(script[1]);
		nlassert(!_SDBPath.empty());

		NLMISC::fromString(CMissionParser::getNoBlankString(script[2]), _SDBDelta);

		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("sdb_player_add");
		vector<TDataSetRow> entities;
		instance->getEntities(entities);
		
		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("sdb_player_add : invalid player row id " + entities[i].toString());
				continue;
			}

			bool res = CStatDB::getInstance()->tablePlayerAdd(_SDBPath, pChar->getId(), _SDBDelta);
			if (pChar->getGuildId() != 0)
			{
				res &= CStatDB::getInstance()->tableGuildAdd(_SDBPath, pChar->getGuildId(), _SDBDelta);
			}

			if (!res)
			{
				LOGMISSIONACTION(toString("sdb_player_add : invalid path : '%s'", _SDBPath.c_str()));
			}
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSDBPlayerAdd)
};
MISSION_REGISTER_ACTION(CMissionActionSDBPlayerAdd,"sdb_player_add");

// ----------------------------------------------------------------------------
class CMissionActionCondJumpSDB : public CMissionActionJump
{
	string _SDBExpr;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<sdb_expr> : <label>");
			return false;
		}

		_SDBExpr = script[1];

		double res;
		if (!evaluateSDBExpr(_SDBExpr, res, true))
		{
			MISLOGERROR("SDB expression is invalid!");
			return false;
		}

		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_sdb");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		double res;
		if (evaluateSDBExpr(_SDBExpr, res, false))
		{
			// if the expression is true, jump
			if (res != 0.0)
				CMissionActionJump::launch( instance, eventList );

		}
		else
		{
			LOGMISSIONACTION("if_sdb : invalid expression (" + _SDBExpr + ")");
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpSDB)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpSDB,"if_sdb");

// ----------------------------------------------------------------------------
class CMissionActionCondJumpRace : public CMissionActionJump
{
	EGSPD::CPeople::TPeople _Race;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<race> : <label>");
			return false;
		}

		_Race = EGSPD::CPeople::fromString(CMissionParser::getNoBlankString(script[1]));
		if (_Race == EGSPD::CPeople::Unknown)
		{
			MISLOGERROR("race unknown");
			return false;
		}

		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_race");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		// If a player from the mission is not of the race selected do not jump
		for (uint i = 0 ; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (pChar != NULL)
			{
				if (pChar->getRace() != _Race) return;
			}
			else
			{
				LOGMISSIONACTION("if_race : invalid player row id " + entities[i].toString());
			}
		}

		CMissionActionJump::launch( instance, eventList );
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpRace)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpRace,"if_race");

// ----------------------------------------------------------------------------
class CMissionActionCondJumpCult : public CMissionActionJump
{
public:
	PVP_CLAN::TPVPClan _Cult;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<cult> : <label>");
			return false;
		}

		_Cult = PVP_CLAN::fromString(CMissionParser::getNoBlankString(script[1]));
		if (!(_Cult >= PVP_CLAN::BeginCults && _Cult <= PVP_CLAN::EndCults || _Cult == PVP_CLAN::Neutral || _Cult == PVP_CLAN::None))
		{
			MISLOGERROR("invalid cult");
			return false;
		}

		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_cult");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		// If a player from the mission is not of the race selected do not jump
		for (uint i = 0 ; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (pChar != NULL)
			{
				if (pChar->getAllegiance().first != _Cult) return;
			}
			else
			{
				LOGMISSIONACTION("if_cult : invalid player row id " + entities[i].toString());
			}
		}

		CMissionActionJump::launch( instance, eventList );
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpCult)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpCult,"if_cult");

// ----------------------------------------------------------------------------
class CMissionActionCondJumpCiv : public CMissionActionJump
{
public:
	PVP_CLAN::TPVPClan _Civ;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<civ> : <label>");
			return false;
		}

		_Civ = PVP_CLAN::fromString(CMissionParser::getNoBlankString(script[1]));
		if (!(_Civ >= PVP_CLAN::BeginCivs && _Civ <= PVP_CLAN::EndCivs || _Civ == PVP_CLAN::Neutral || _Civ == PVP_CLAN::None))
		{
			MISLOGERROR("civ unknown");
			return false;
		}

		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_civ");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		// If a player from the mission is not of the race selected do not jump
		for (uint i = 0 ; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (pChar != NULL)
			{
				if (pChar->getAllegiance().second != _Civ) return;
			}
			else
			{
				LOGMISSIONACTION("if_civ : invalid player row id " + entities[i].toString());
			}
		}

		CMissionActionJump::launch( instance, eventList );
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpCiv)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpCiv,"if_civ");

// ----------------------------------------------------------------------------
class CMissionActionCondJumpGuildCult : public CMissionActionJump
{
	PVP_CLAN::TPVPClan _Cult;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<cult> : <label>");
			return false;
		}

		_Cult = PVP_CLAN::fromString(CMissionParser::getNoBlankString(script[1]));
		if (!(_Cult >= PVP_CLAN::BeginCults && _Cult <= PVP_CLAN::EndCults || _Cult == PVP_CLAN::Neutral || _Cult == PVP_CLAN::None))
		{
			MISLOGERROR("invalid cult");
			return false;
		}

		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_guild_cult");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		// If guild player from the mission is not of the race selected do not jump
		for (uint i = 0 ; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (pChar != NULL)
			{
				CGuild * pGuild = CGuildManager::getInstance()->getGuildFromId( pChar->getGuildId() );
				if ( pGuild )
				{
					if( pGuild->getAllegiance().first != _Cult ) return;
				}
				else
				{
					LOGMISSIONACTION("if_guild_cult : player row id " + entities[i].toString() + " have no guild");
				}
			}
			else
			{
				LOGMISSIONACTION("if_guild_cult : invalid player row id " + entities[i].toString());
			}
		}

		CMissionActionJump::launch( instance, eventList );
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpGuildCult)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpGuildCult,"if_guild_cult");

// ----------------------------------------------------------------------------
class CMissionActionCondJumpGuildCiv : public CMissionActionJump
{
	PVP_CLAN::TPVPClan _Civ;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<civ> : <label>");
			return false;
		}

		_Civ = PVP_CLAN::fromString(CMissionParser::getNoBlankString(script[1]));
		if (!(_Civ >= PVP_CLAN::BeginCivs && _Civ <= PVP_CLAN::EndCivs || _Civ == PVP_CLAN::Neutral || _Civ == PVP_CLAN::None))
		{
			MISLOGERROR("civ unknown");
			return false;
		}

		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_guild_civ");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		// If guild player from the mission is not of the race selected do not jump
		for (uint i = 0 ; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (pChar != NULL)
			{
				CGuild * pGuild = CGuildManager::getInstance()->getGuildFromId( pChar->getGuildId() );
				if ( pGuild )
				{
					if( pGuild->getAllegiance().second != _Civ ) return;
				}
				else
				{
					LOGMISSIONACTION("if_guild_civ : player row id " + entities[i].toString() + " have no guild");
				}
			}
			else
			{
				LOGMISSIONACTION("if_guild_civ : invalid player row id " + entities[i].toString());
			}
		}

		CMissionActionJump::launch( instance, eventList );
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpGuildCiv)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpGuildCiv,"if_guild_civ");

							  
// ----------------------------------------------------------------------------
class CMissionActionCondJumpGuildFame : public CMissionActionJump
{
public:
	uint32 factionIndex;
	sint32 fameValue;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 4)
		{
			MISLOGSYNTAXERROR("<fame> : <value> : <label>");
			return false;
		}

		factionIndex = CStaticFames::getInstance().getFactionIndex( CMissionParser::getNoBlankString(script[1]));
		NLMISC::fromString( CMissionParser::getNoBlankString(script[2]), fameValue);

		if( factionIndex == CStaticFames::INVALID_FACTION_INDEX )
		{
			MISLOGERROR("fame unknown");
			return false;
		}

		Label = CMissionParser::getNoBlankString(script[3]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_guild_fame");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		// If guild player from the mission have not enought fame do not jump
		for (uint i = 0 ; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (pChar != NULL)
			{
				CGuild * pGuild = CGuildManager::getInstance()->getGuildFromId( pChar->getGuildId() );
				if ( pGuild )
				{
					if( CFameInterface::getInstance().getFameIndexed( pGuild->getEId(), factionIndex ) < fameValue )
						return;
				}
				else
				{
					LOGMISSIONACTION("if_guild_fame : player row id " + entities[i].toString() + " have no guild");
				}
			}
			else
			{
				LOGMISSIONACTION("if_guild_fame : invalid player row id " + entities[i].toString());
			}
		}
		CMissionActionJump::launch( instance, eventList );
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpGuildFame)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpGuildFame,"if_guild_fame");

// ----------------------------------------------------------------------------
class CMissionActionCondJumpNoTrial : public CMissionActionJump
{
public:
	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 2)
		{
			MISLOGSYNTAXERROR("<label>");
			return false;
		}
		
		Label = CMissionParser::getNoBlankString(script[1]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_no_trial");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;
		
		// If player have a trial account, do not jump
		for (uint i = 0 ; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			CPlayer *pPlayer = 0;
			if( pChar )
				pPlayer = PlayerManager.getPlayer(PlayerManager.getPlayerId(pChar->getId()));
			if (pPlayer != NULL)
			{
				if( pPlayer->isTrialPlayer() )
					return;
			}
			else
			{
				LOGMISSIONACTION("if_no_trial : invalid player row id " + entities[i].toString());
			}
		}
		CMissionActionJump::launch( instance, eventList );
	}
	
	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpNoTrial)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpNoTrial,"if_no_trial");

// ----------------------------------------------------------------------------
class CMissionActionCondJumpRoomInInv : public CMissionActionJump
{
public:
	vector<string> Items;
	vector<sint16> ItemQty;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() < 3 )
		{
			MISLOGSYNTAXERROR("<item1> <qty1>[;<item2> <qty2> ...]:<label>");
			return false;
		}

		vector<string>	args;
		explode(script[1], string(";"), args, true);
		if (args.size() == 0)
		{
			MISLOGSYNTAXERROR("<item1> <qty1>[;<item2> <qty2> ...]:<label>");
			return false;
		}
		for (uint i=0; i<args.size(); ++i)
		{
			vector<string>	retList;
			explode(args[i], string(" "), retList, true);
			if ( retList.size() != 2 )
			{
				MISLOGSYNTAXERROR("<item1> <qty1>[;<item2> <qty2> ...]:<label>");
				return false;
			}
			Items.push_back( retList[0] );
			sint16 quantity;
			NLMISC::fromString(retList[1], quantity);
			ItemQty.push_back(quantity);
		}

		Label = CMissionParser::getNoBlankString(script[script.size()-1]);
		missionData.Jumps.push_back( Label );
		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_room_in_inventory");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		sint16 neededSlotCount = 0;
		for( uint i=0; i<Items.size(); ++i )
		{
			if( Items[i].find(".sitem") != -1 )
			{
				const CStaticItem * itemSheet = CSheets::getForm (CSheetId(Items[i]));
				if( itemSheet )
				{
					neededSlotCount += (sint16) ceil( (float)ItemQty[i] / itemSheet->getMaxStackSize() );
				}
				else
				{
					LOGMISSIONACTION("if_room_in_inventory : invalid item " + Items[i]);
				}
			}
			else // named item
			{
				CGameItemPtr item = CNamedItems::getInstance().getNamedItemRef(Items[i]);
				if( item != NULL )
				{
					neededSlotCount += ItemQty[i];
				}
				else
				{
					LOGMISSIONACTION("if_room_in_inventory : invalid named item " + Items[i]);
				}
			}
		}

		bool jump = false;
		for ( uint i = 0; i < entities.size(); i++ )
		{
			CCharacter * user = PlayerManager.getChar( entities[i] );
			if ( user )
			{
				CInventoryPtr invBag = user->getInventory( INVENTORIES::bag );
				sint16 freeSlotcount = invBag->getFreeSlotCount();
				if( neededSlotCount <= freeSlotcount )
				{
					jump = true;
					break;
				}
			}
		}

		if( jump )
		{
			CMissionActionJump::launch( instance, eventList );
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpRoomInInv)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpRoomInInv,"if_room_in_inventory");

// ----------------------------------------------------------------------------
class CMissionActionSDBSetPVPPath : public IMissionAction
{
	string _SDBPath;

	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
		CMissionGlobalParsingData & globalData, 
		CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		if (script.size() != 2)
		{
			MISLOGSYNTAXERROR("<sdb_path>");
			return false;
		}

		_SDBPath = CMissionParser::getNoBlankString(script[1]);
		nlassert(!_SDBPath.empty());

		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("sdb_set_pvp_path");
		vector<TDataSetRow> entities;
		instance->getEntities(entities);
		
		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("sdb_set_pvp_path : invalid player row id " + entities[i].toString());
				continue;
			}

			pChar->setSDBPvPPath(_SDBPath);
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSDBSetPVPPath)
};
MISSION_REGISTER_ACTION(CMissionActionSDBSetPVPPath,"sdb_set_pvp_path");

// ----------------------------------------------------------------------------
class CMissionActionSDBClearPVPPath : public IMissionAction
{
	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
		CMissionGlobalParsingData & globalData, 
		CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		if (script.size() != 1)
		{
			MISLOGSYNTAXERROR("no param");
			return false;
		}

		return true;
	}

	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("sdb_clear_pvp_path");
		vector<TDataSetRow> entities;
		instance->getEntities(entities);
		
		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("sdb_clear_pvp_path : invalid player row id " + entities[i].toString());
				continue;
			}

			pChar->setSDBPvPPath("");
		}
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionSDBClearPVPPath)
};
MISSION_REGISTER_ACTION(CMissionActionSDBClearPVPPath,"sdb_clear_pvp_path");

// ----------------------------------------------------------------------------
class CMissionActionCondJumpFactionPoint : public CMissionActionJump
{
	string _FPExpr;

	bool buildAction ( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData)
	{
		_SourceLine = line;
		if ( script.size() != 3)
		{
			MISLOGSYNTAXERROR("<fp_expr> : <label>");
			return false;
		}

		_FPExpr = script[1];

		double res;
		if (!evaluateFPExpr(res, NULL))
		{
			MISLOGERROR("FP expression is invalid!");
			return false;
		}

		Label = CMissionParser::getNoBlankString(script[2]);
		missionData.Jumps.push_back( Label );
		return true;
	}
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("if_faction_point");
		// get the player
		std::vector<TDataSetRow> entities;
		instance->getEntities( entities );
		if ( entities.empty() )
			return;

		double res;
		bool testOk = true;
		uint32 nbValidPlayers = 0;
		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("if_faction_point : invalid player row id " + entities[i].toString());
				continue;
			}
			nbValidPlayers++;
			testOk &= evaluateFPExpr(res, pChar);
			testOk &= (res != 0.0);
			if (!testOk)
				break;
		}

		if (!testOk)
		{
			LOGMISSIONACTION("if_faction_point : invalid expression (" + _FPExpr + ")");
		}
		else if (nbValidPlayers > 0)
		{
			// if the expression is true for all players, jump
			CMissionActionJump::launch( instance, eventList );
		}
	}
	
	bool evaluateFPExpr(double & res, CCharacter * c)
	{
		string numExpr;
		uint i = 0;
		while (i < _FPExpr.size())
		{
			if (_FPExpr[i] == '{')
			{
				string factionName;
				uint32 factionPoints;
				i++;
				while (i < _FPExpr.size() && _FPExpr[i] != '}')
				{
					factionName += _FPExpr[i];
					i++;
				}
				if (i == _FPExpr.size() || factionName.empty())
					return false;
				
				PVP_CLAN::TPVPClan clan = PVP_CLAN::fromString(factionName);
				if (clan < PVP_CLAN::BeginClans || clan > PVP_CLAN::EndClans)
					return false;

				if (c != NULL)
					factionPoints = c->getFactionPoint(clan);
				else
					factionPoints = 0;

				numExpr += NLMISC::toString(factionPoints);
			}
			else
			{
				numExpr += _FPExpr[i];
			}

			i++;
		}

		if (CEvalNumExpr().evalExpression(numExpr.c_str(), res, NULL) != CEvalNumExpr::NoError)
			return false;
		return true;
	}

	MISSION_ACTION_GETNEWPTR(CMissionActionCondJumpFactionPoint)
};
MISSION_REGISTER_ACTION(CMissionActionCondJumpFactionPoint,"if_faction_point");

/*
// ----------------------------------------------------------------------------
class CMissionActionHandleRelease : public IMissionAction
{
	TAIAlias GroupAlias;
	
	bool buildAction (	uint32 line, const std::vector< std::string > & script, 
		CMissionGlobalParsingData & globalData, 
		CMissionSpecificParsingData & missionData	)
	{
		_SourceLine = line;
		if ( script.size() != 2 )
		{
			MISLOGSYNTAXERROR("<group>");
			return false;
		}
		string sGroupName = CMissionParser::getNoBlankString( script[1] );
		vector<TAIAlias> aliases;
		CAIAliasTranslator::getInstance()->getGroupAliasesFromName(sGroupName, aliases);
		if (aliases.size() != 1)
		{
			MISLOGERROR1("group name '%s' do not have a unique alias", sGroupName.c_str());
			return false;
		}
		GroupAlias = aliases[0];
		return true;
	}
	
	void launch(CMission* instance, std::list< CMissionEvent * > & eventList)
	{
		LOGMISSIONACTION("handle_release");

		vector<TDataSetRow> entities;
		instance->getEntities(entities);
		
		for (uint32 i = 0; i < entities.size(); ++i)
		{
			CCharacter *pChar = PlayerManager.getChar(entities[i]);
			if (!pChar)
			{
				LOGMISSIONACTION("handle_release : invalid player row id " + entities[i].toString());
				continue;
			}
			pChar->delHandledAIGroup(instance, GroupAlias);
		}
	}
	
	MISSION_ACTION_GETNEWPTR(CMissionActionHandleRelease)
};
MISSION_REGISTER_ACTION(CMissionActionHandleRelease, "handle_release");
*/
