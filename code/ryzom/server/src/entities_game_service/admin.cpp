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



//
// User Privilege set in the mysql database must be like that ":GM:" or "" or ":GM:ADMIN:" ....
//
//
//



//
// Includes
//

#include "stdpch.h"

#include "admin.h"

#include "nel/misc/common.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/eid_translator.h"
#include "nel/misc/algo.h"
#include "nel/misc/sstring.h"

#include "nel/net/admin.h"
#include "nel/net/service.h"

#include "game_share/scores.h"
#include "game_share/send_chat.h"
#include "game_share/time_weather_season/time_date_season_manager.h"
#include "game_share/permanent_ban_magic_number.h"
#include "game_share/fame.h"
#include "game_share/visual_slot_manager.h"
#include "game_share/shard_names.h"
#include "server_share/log_command_gen.h"
#include "server_share/r2_vision.h"

#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_rolemaster_phrase.h"
#include "egs_sheets/egs_static_encyclo.h"

#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "player_manager/character_encyclopedia.h"
#include "creature_manager/creature_manager.h"
#include "phrase_manager/phrase_manager.h"
#include "mission_manager/mission_manager.h"
#include "mission_manager/mission_queue_manager.h"

#include "entities_game_service.h"
#include "player_manager/character_respawn_points.h"
#include "weather_everywhere.h"
#include "phrase_manager/fg_prospection_phrase.h"
#include "team_manager/team_manager.h"
#include "world_instances.h"
#include "egs_variables.h"
#include "player_manager/gm_tp_pending_command.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_member.h"
#include "guild_manager/guild_member_module.h"
#include "guild_manager/guild_char_proxy.h"
#include "guild_manager/fame_manager.h"
#include "mission_manager/mission_solo.h"
#include "mission_manager/mission_solo.h"
#include "position_flag_manager.h"
//#include "name_manager.h"
#include "zone_manager.h"
#include "player_manager/character_game_event.h"
#include "game_event_manager.h"
#include "dyn_chat_egs.h"

#include "pvp_manager/pvp.h"
#include "player_manager/admin_properties.h"
#include "shop_type/offline_character_command.h"
#include "player_manager/item_service_manager.h"
#include "outpost_manager/outpost_manager.h"
#include "primitives_parser.h"
#include "shop_type/named_items.h"
#include "progression/progression_pvp.h"

#include "modules/shard_unifier_client.h"
#include "modules/client_command_forwarder.h"
#include "modules/guild_unifier.h"
#include "server_share/log_command_gen.h"
#include "server_share/log_item_gen.h"
#include "server_share/log_character_gen.h"

//
// Externs
//

extern CPlayerManager			PlayerManager;
extern CCreatureManager			CreatureManager;
extern CPlayerService			*PS;
extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern CBSAIDeathReport			BotDeathReport;
extern NLMISC::CVariable<uint32> FixedSessionId;

//
// Namespaces
//

using namespace NLMISC;
using namespace NLNET;
using namespace std;

extern CVariable<string>	BannerPriv;

//
// Functions
//

//
// ALL THESE COMMANDS ARE USED BY THE CLIENT *AND* THE ADMIN TOOL TO MANAGE A PLAYER
//
// You can assume here that the user can execute the command, you don't have to check it
//
// Don't forget to add your command in the AdminCommandsInit array if you want the command to be executed on the client
//
// If you change parameter or add/remove something, please update the wiki:
//
// http://www.nevrax.net/wiki/index.php/Main/CommandesAdmin
//

// AddEid must be true if you want to have the eid in the args[0] of the command

// Privileges are now stored in a text file (data_shard/client_commands_privileges.txt), do not forget to update it.


// please update the following stuff  if you add new GM titles
// player_manager.cpp : NLMISC_COMMAND(setPriv,"set a privilege to a user using his user id, must be in form :priv1:priv2:priv3:","<uid> <priv>")
// player_manager.cpp : hasBetterCSRGrade
static const struct
{
	const char *	Name;
	bool			AddEId;
}
AdminCommandsInit[] =
{
		// player character accessible commands
		"teamInvite",						true,
		"guildInvite",						true,
		"setGuildMessage",					true,
		"clearGuildMessage",				true,
		"dodge",							true,
		"parry",							true,
		"respawnAfterDeath",				true,
		"resurrected",						true,
		"validateRespawnPoint",				true,
		"summonPet",						true,

		"addPetAnimal",						true,
		"addSkillPoints",					true,
		"addXPToSkill",						true,
		"changeMode",						true,
		"checkTargetSP",					true,
		"clearFriendsList",					true,
		"clearIgnoreList",					true,
		"clearIsFriendOfList",				true,
		"createItemInBag",					true,
		"createItemInInv",					true,
		"createItemInTmpInv",				true,
		"createNamedItemInBag",				true,
		"createFullArmorSet",				true,
		"displayInfosOnTarget",				true,
		"execPhrase",						true,
		"execMemorizedPhrase",				true,
		"executeSabrinaPhrase",				true,
		"forceTargetToDie",					true,
		"learnAllBricks",					true,
		"learnAllRolemasterBricks",			true,
		"learnAllPhrases",					true,
		"learnBrick",						true,
		"unlearnBrick",						true,
		"learnPhrase",						true,
		"learnAllForagePhrases",            true,
		"learnAllFaberPlans",				true,
		"logXpGain",						true,
		"memorizePhrase",					true,
		"resetPowerFlags",					true,
		"setSkillsToMaxValue",				true,
		"displayForageRM",					true,
		"setItemSapLoad",					true,
		"showFBT",							true,
		"allowSummonPet",					true,
		"setPetAnimalSatiety",				true,
		"getPetAnimalSatiety",				true,
		"taskPass",							true,
		"setFamePlayer",					true,
		"guildMOTD",						true,

		// CSR commands
		"motd",								false,
		"broadcast",						false,
		"summon",							true,
		"dismiss",							true,
		"teleport",							true,
		"renamePlayerForEvent",				true,
		"renamePlayer",						true,
		"renameGuild",						true,
		"setGuildDescription",				true,
		"setGuildIcon",						false,
		"killMob",							true,
		"changeVar",						true,
		"root",								true,
		"unroot",							true,
		"ignoreTells",						true, // hierarchy
		"showCSR",							true,
		"monitorMissions",					true,
		"stopMonitorMissions",				true,
		"failMission",						true,
		"progressMission",					true,
		"mute",								true,
		"unmute",							true,
		"muteUniverse",						true,
		"unmuteUniverse",					true,
		"universe",							true,
		"setGMGuild",						true,
		"targetInfos",						true,
		"infos",							true,
		"addPosFlag",						true,
		"setPosFlag",						true,
		"delPosFlag",						true,
		"lPosFlags",						true,
		"listPosFlags",						true,
		"tpPosFlag",						true,
		"updateGuildMembersList",			true,
		"listGuildMembers",					true,
		"addGuildMember",					true,
		"setGuildMemberGrade",				true,
		"setGuildLeader",					true,
		"startEvent",						true,
		"stopEvent",						true,
		"getEventFaction",					true,
		"setEventFaction",					true,
		"clearEventFaction",				true,
		"giveRespawnPoint",					true,
		"provideItemService",				true,
		"dumpFactionPVPDamage",				true,
		"changeHairCut",					true,
		"farTPPush",						true,
		"farTPReplace",						true,
		"farTPReturn",						true,
		"resetPVPTimers",					true,
		"savePlayerActiveChar",				false,
		"reloadPlayer",						false,
		"characterMissionDump",				true,
		"removeMission",					true,
		"addMission",						true,
		// CSR variables
		"Invisible",						true, // not in /who; hierarchy except for user with privilege
		"God",								true,
		"Invulnerable",						true,
		"ShowFactionChannels",				true,
		"CreateCharacterStartSkillsValue",	false,
		"HP",								true,
		"MaxHP",							true,
		"Speed",							true,
		"Money",							true,
		"MoneyGuild",						true,
		"FactionPoint",						true,
		"Name",								true,
		"Position",							true,
		"Priv",								true,
		"PriviledgePVP",					true,
		"FBT",								true,
		"RyzomDate",						false,
		"RyzomTime",						false,
		"addGuildXp",						false,
		"setGuildChargePoint",				false,


		// PUT HERE THE VARIABLE / COMMAND THAT ARE TEMPORARY
		// remove when message of the day interface is ready
		// remove this when death interface is ready
		"respawnCharacter",					true,
		"buyPact",							true,
		"cancelTopPhrase",					true, //temp : to remove
		"EntitiesNoActionFailure",			false,
		"EntitiesNoCastBreak",				false,
		"EntitiesNoResist",					false,
		// aggroable state
		"Aggro",							true,

		// outpost commands
		"outpostBanGuild",					false,
		"outpostBanPlayer",					false,
		"outpostUnbanGuild",				false,
		"outpostUnbanPlayer",				false,
		"outpostChallengeByGuild",			false,
		"outpostSimulateTimer0End",			false,
		"outpostSimulateTimer1End",			false,
		"outpostSimulateTimer2End",			false,
		"outpostSetFightData",				false,
		"setOutpostLevel",					false,
		"outpostAccelerateConstruction",	false,
		"outpostSetOutpostOwner",			false,
		"outpostChallengeOutpost",			true,
		"outpostGiveupOutpost",				true,
		"outpostDisplayGuildOutposts",		true,
		"outpostForceOpenGuildInventory",	true,
		"outpostForceCloseGuildInventory",	true,
		"outpostSetPlayerPvpSide",			true,
		"outpostSetOutpostToPlayer",		true,
		"outpostSelectOutpost",				true,
		"outpostUnselectOutpost",			true,

		"setPvpClan",						true, //remove it when interface fo choose pvp clan is ready
		"setPvPTag",						true, //set pvp tag to true or false

		// stuff added by sadge for testing character read/ write stuff
		"saveToXML",						true,
		"loadFromXML",						true,
		"saveToPDR",						true,
		"loadFromPDR",						true,

		// queues related stuff
		"acceptProposalForQueue",			true,
		"awakePlayerInQueue",				true,

		"displayShopSelector",				true,

		"addFactionAttackableToTarget",		true,
		"eventCreateNpcGroup",				true,
		"eventNpcGroupScript",				true,
		"eventSetBotName",					true,
		"eventSetBotScale",					true,
		"eventSetNpcGroupAggroRange",		true,
		"eventSetFaunaBotAggroRange",		true,
		"eventResetFaunaBotAggroRange",		true,
		"eventSetBotCanAggro",				true,
		"eventSetItemCustomText",			true,
		"eventResetItemCustomText",			true,
		"eventSetBotSheet",					true,
		"eventSetBotFaction",				true,
		"eventSetBotFameByKill",			true,
		"dssTarget",						true,	//ring stuff
		"forceMissionProgress",				true,
		"eventSetBotURL",true,
		"eventSetBotURLName",true,
};

static vector<CAdminCommand>	AdminCommands;
static string					CommandsPrivilegesFileName;
static string					PositionFlagsFileName;
static const char *				DefaultPriv = ":DEV:";


// forward declarations
static void loadCommandsPrivileges(const string & fileName, bool init);
void cbRemoteClientCallback (uint32 rid, const std::string &cmd, const std::string &entityNames);
//


CAdminCommand * findAdminCommand(const string & name)
{
	H_AUTO(findAdminCommand);

	const uint nbCommands = (uint)AdminCommands.size();
	for (uint i = 0; i < nbCommands; i++)
	{
		if (name == AdminCommands[i].Name)
			return &AdminCommands[i];
	}

	return NULL;
}

static void cbCommandsPrivilegesFileChange(const string & fileName)
{
	H_AUTO(cbCommandsPrivilegesFileChange);

	if (fileName != CommandsPrivilegesFileName)
		return;

	loadCommandsPrivileges(fileName, false);
}

void initAdmin ()
{
	const uint nbCommands = sizeof(AdminCommandsInit) / sizeof(AdminCommandsInit[0]);
	for (uint i = 0; i < nbCommands; i++)
	{
		CAdminCommand cmd;
		cmd.Name	= AdminCommandsInit[i].Name;
		cmd.AddEId	= AdminCommandsInit[i].AddEId;
		cmd.Priv	= DefaultPriv;

		AdminCommands.push_back(cmd);
	}

	setRemoteClientCallback (cbRemoteClientCallback);
}

void initCommandsPrivileges(const std::string & fileName)
{
	H_AUTO(initCommandsPrivileges);

	loadCommandsPrivileges(fileName, true);
}

static void loadCommandsPrivileges(const string & fileName, bool init)
{
	H_AUTO(loadCommandsPrivileges);

	CIFile ifile;
	if (!ifile.open(fileName))
	{
		nlwarning ("ADMIN: file '%s' cannot be opened", fileName.c_str());
		return;
	}

	if (init)
	{
		const string filePath = CPath::getFullPath(fileName, false);
		CFile::removeFileChangeCallback(CommandsPrivilegesFileName);
		CFile::addFileChangeCallback(filePath, cbCommandsPrivilegesFileChange);
		CommandsPrivilegesFileName = filePath;
	}

	// reset privileges with default value
	const uint nbCommands = (uint)AdminCommands.size();
	for (uint i = 0; i < nbCommands; i++)
	{
		AdminCommands[i].Priv = DefaultPriv;
	}

	// load file content
	CSString fileContent;
	uint32 fileSize = ifile.getFileSize();
	uint8 * buf = new uint8[fileSize + 1];

	ifile.serialBuffer(buf, fileSize);
	ifile.close();

	buf[fileSize] = 0;
	fileContent = (char *)buf;
	delete [] buf;

	while (!fileContent.empty())
	{
		CSString line = fileContent.strtok("\r\n");

		// remove any comment
		line = line.splitTo("//");

		if (line.strip().empty())
			continue;

		CSString fullLine = line;

		// only extract the first 2 params
		CVectorSString params;
		for (uint i = 0; !line.empty() && i < 2; i++)
		{
			string param = line.strtok(" \t");
			if (param.empty())
				break;
			params.push_back(param);
		}

		if (params.size() < 1)
		{
			nlwarning("ADMIN: invalid entry: '%s'.", fullLine.c_str());
			continue;
		}

		// check second param if it is a forward info
		if (params.size() > 1 && params[1][0] == '[')
		{
			// this is a forward
		}
		else if (params.size() > 2)
		{
			nlwarning("ADMIN: invalid entry: '%s'.", fullLine.c_str());
			continue;
		}
		else
		{
			// no forward
			params.insert(params.begin()+1, "[]");
		}

		if (params.size() < 3)
		{
			// no required privilege
			params.push_back("");
		}

		const string & cmdName = params[0];
		const string & forward = params[1];
		const string & cmdPriv = params[2];

		CAdminCommand * cmd = findAdminCommand(cmdName);
		if (cmd)
		{
			cmd->Priv = cmdPriv;
			cmd->ForwardToservice = forward.substr(1, forward.size()-2);
			nlinfo("ADMIN: command '%s' forwarded to [%s] has new privileges '%s'.", cmdName.c_str(), cmd->ForwardToservice.c_str(), cmdPriv.c_str());
		}
		else
		{
			nlwarning("ADMIN: command '%s' is unknown.", cmdName.c_str());
		}
	}
}

void initPositionFlags(const std::string & fileName)
{
	H_AUTO(initPositionFlags);

	string fileNameAndPath = Bsi.getLocalPath() + fileName;
	if (CFile::fileExists(fileNameAndPath))
	{
		CPositionFlagManager::getInstance().loadFromFile(fileName);
	}
	PositionFlagsFileName = fileName;
}

static void selectEntities (const string &entityName, vector <CEntityId> &entities)
{
	H_AUTO(selectEntities);

	if (entityName.empty ())
		return;

	/*
	 *
	 * valid name formats:
	 *
	 *	- *												Select all entities
	 *	- <entity id(id:type:crea:dyn)>					Select the specified entity using his eid
	 */

	if (entityName == "*")
	{
		// we want all entities
		for (CPlayerManager::TMapPlayers::const_iterator it = PlayerManager.getPlayers().begin(); it != PlayerManager.getPlayers().end(); ++it)
		{
			if ((*it).second.Player != 0)
			{
				const CCharacter *c = (*it).second.Player->getActiveCharacter();
				if (c != 0)
				{
					entities.push_back (c->getId());
				}
			}
		}
	}
	else if (entityName[0] == '(')
	{
		// we want a specific entity id
		CEntityId eid (entityName);
		if (eid != CEntityId::Unknown)
			entities.push_back (eid);
	}
	else
	{
		// try with the name
		CEntityId eid = CEntityIdTranslator::getInstance()->getByEntity(CShardNames::getInstance().makeFullNameFromRelative(TSessionId(FixedSessionId), entityName));
		if (eid != CEntityId::Unknown)
			entities.push_back (eid);
	}
}

#define ENTITY_VARIABLE(__name,__help) \
struct __name##Class : public NLMISC::ICommand \
{ \
__name##Class () : NLMISC::ICommand("variables",#__name, __help, "<entity> [<value>]") { Type = Variable; } \
	virtual bool execute(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human) \
	{ \
		if (args.size () != 1 && args.size () != 2) \
			return false; \
 \
		vector <CEntityId> entities; \
		selectEntities	(args[0], entities); \
 \
		for (uint i = 0; i < entities.size(); i++) \
		{ \
			string value; \
			if (args.size()==2) \
				value = args[1]; \
			else \
				value = "???"; \
			pointer (entities[i], (args.size()==1), value); \
			if (quiet) \
				log.displayNL ("%s %s", entities[i].toString().c_str(), value.c_str()); \
			else \
				log.displayNL ("Entity %s Variable %s = %s", entities[i].toString().c_str(), _CommandName.c_str(), value.c_str()); \
		} \
		return true; \
	} \
	void pointer(CEntityId entity, bool get, std::string &value); \
}; \
__name##Class __name##Instance; \
void __name##Class::pointer(CEntityId entity, bool get, std::string &value)


//
// This macro get an eid and return c that is a pointer to a CCharacter.
//

#define ENTITY_GET_ENTITY \
	TLogContext_Character_AdminCommand commandContext(entity); \
	CEntityBase *e = CEntityBaseManager::getEntityBasePtr(entity); \
	if(e == 0) \
	{ \
		nlwarning ("Unknown entity '%s'", entity.toString().c_str()); \
		if(get) value = "UnknownEntity"; \
		return; \
	} \
	if(!TheDataset.isAccessible(e->getEntityRowId())) \
	{ \
		nlwarning ("'%s' is not valid in mirror", entity.toString().c_str()); \
		if(get) value = "NotValidInMirror"; \
		return; \
	}

#define ENTITY_GET_CHARACTER \
	TLogContext_Character_AdminCommand commandContext(entity); \
	CCharacter *c = PlayerManager.getChar(entity); \
	if(c == 0) \
	{ \
		nlwarning ("Unknown player '%s'", entity.toString().c_str()); \
		if(get) value = "UnknownPlayer"; \
		return; \
	} \
	if(!c->getEnterFlag()) \
	{ \
		nlwarning ("'%s' is not entered", entity.toString().c_str()); \
		if(get) value = "NotEntered"; \
		return; \
	} \
	if(!TheDataset.isAccessible(c->getEntityRowId())) \
	{ \
		nlwarning ("'%s' is not valid in mirror", entity.toString().c_str()); \
		if(get) value = "NotValidInMirror"; \
		return; \
	}

void GET_CHARACTER_Helper(std::string& command, const NLMISC::CEntityId& id, const std::string& adminCommand)
{
	CAdminOfflineCommand::makeStringCommande( command, id, adminCommand );
	COfflineCharacterCommand::getInstance()->addOfflineCommandWithoutApply( command );
}

#define GET_ENTITY \
	if (args.size() < 1) { nlwarning ("Missing argument number 0 that should be the eid"); return false; } \
	CEntityId eid(args[0]); \
	if (eid == CEntityId::Unknown) \
		return true; \
	TLogContext_Character_AdminCommand commandContext(eid); \
	CEntityBase *e = CEntityBaseManager::getEntityBasePtr(eid); \
	if(e == 0) \
	{ \
		nlwarning ("Unknown entity '%s'", eid.toString().c_str()); \
		return true; \
	} \
	if(!TheDataset.isAccessible(e->getEntityRowId())) \
	{ \
		nlwarning ("'%s' is not valid in mirror", eid.toString().c_str()); \
		return true; \
	}

#define TRY_GET_CHARACTER \
	if (args.size() < 1) { nlwarning ("Missing argument number 0 that should be the eid"); return false; } \
	CEntityId eid(args[0]); \
	TLogContext_Character_AdminCommand commandContext(eid); \
	CCharacter *c = PlayerManager.getChar(eid);  \
	if (!c) \
	{ \
		nlwarning("Unknown character '%s'", args[0].c_str()); \
		return false; \
	}



#define CHECK_RIGHT( csr, target ) \
	{\
		if ( !PlayerManager.hasBetterCSRGrade( csr, target ) )\
		{\
			if ( csr )\
				CCharacter::sendDynamicSystemMessage( csr->getEntityRowId(), "CSR_BAD_RIGHTS" );\
			return false;\
		}\
	}

#define GET_GUILD(argPos, onlyLocal, eId) \
	CGuild * guild = CGuildManager::getInstance()->getGuildByName( args[argPos] );\
	if ( guild == NULL )\
	{\
		/* try to find the guild with an id*/ \
		uint32 shardId =0; \
		uint32 guildId =0; \
		sscanf(args[argPos].c_str(), "%u:%u", &shardId, &guildId); \
		guild = CGuildManager::getInstance()->getGuildFromId((shardId<<20)+guildId); \
		\
		if (guild == NULL) \
		{ \
			if (eId != NLMISC::CEntityId::Unknown)\
			{\
				CCharacter::sendDynamicSystemMessage(eId, "GUILD_IS_PROXY");\
			}\
			log.displayNL("Invalid guild '%s'",args[argPos].c_str());\
			return true;\
		} \
	} \
	if (onlyLocal && guild->isProxy())\
	{\
		log.displayNL("The guild '%s' is a foreign guild, operation forbidden", guild->getName().toString().c_str());\
		return true;\
	}\

//
// You can here all variables for a player.
//
// Don't forget to add your command in the AdminCommandsInit array if you want the command to be executed on the client
//

ENTITY_VARIABLE(HP, "Hit points of a player")
{
	ENTITY_GET_ENTITY

	if (get)
	{
		value = toString (e->currentHp());
	}
	else
	{
		e->getScores()._PhysicalScores[SCORES::hit_points].Current = atoi(value.c_str());
	}
}

ENTITY_VARIABLE(MaxHP, "Max hit points of a player")
{
	ENTITY_GET_ENTITY

	if (get)
	{
		value = toString (e->maxHp());
	}
	else
	{
		e->getScores()._PhysicalScores[SCORES::hit_points].Max = atoi(value.c_str());
	}
}

ENTITY_VARIABLE(Speed, "Speed modifier of an entity")
{
	ENTITY_GET_ENTITY

	if (get)
	{
		value = toString (e->getEventSpeedVariationModifier());
	}
	else
	{
		e->setEventSpeedVariationModifier((float)atof(value.c_str()));
	}
}

ENTITY_VARIABLE(Name, "Name of a player")
{
	ENTITY_GET_ENTITY

	if (get)
	{
//		value = CEntityIdTranslator::getInstance()->getByEntity(entity).toString();
		value = e->getName ().toString();
	}
	else
	{
	}
}

ENTITY_VARIABLE(Money, "Money")
{
	ENTITY_GET_CHARACTER

	if (get)
	{
		value = toString("%"NL_I64"u", c->getMoney());
	}
	else
	{
		sint dir = 0;

		if (!value.empty())
		{
			if(value[0] == '+')			dir = +1;
			else if(value[0] == '-')	dir = -1;
			else if(!isdigit(value[0]))
			{
				nlwarning ("Bad money format for %s right format is [<sign>]<value> sign can be +,- or nothing to set the money", value.c_str());
				return;
			}
		}
		else
		{
			nlwarning ("You must provide a [<sign>]<value>");
			return;
		}

		if(dir == 0)
		{
			c->setMoney(NLMISC::atoiInt64(value.c_str()));
		}
		else
		{
			uint64 val = NLMISC::atoiInt64(value.substr(1).c_str());
			if(dir == 1)
				c->giveMoney(val);
			else
				c->spendMoney(val);
		}

		nlinfo ("Player %s money is %"NL_I64"u", entity.toString().c_str(),c->getMoney());
	}
}

ENTITY_VARIABLE(MoneyGuild, "MoneyGuild")
{
	ENTITY_GET_CHARACTER

	CGuild * guild = CGuildManager::getInstance()->getGuildFromId(c->getGuildId());
	if (guild == NULL)
	{
		nlwarning("Invalid guild");
		return;
	}

	if (get)
	{
		value = toString("%"NL_I64"u", guild->getMoney());
	}
	else
	{
		sint dir = 0;

		if (!value.empty())
		{
			if(value[0] == '+')			dir = +1;
			else if(value[0] == '-')	dir = -1;
			else if(!isdigit(value[0]))
			{
				nlwarning ("Bad money format for %s right format is [<sign>]<value> sign can be +,- or nothing to set the money", value.c_str());
				return;
			}
		}
		else
		{
			nlwarning ("You must provide a [<sign>]<value>");
			return;
		}

		if(dir == 0)
		{
			guild->setMoney(NLMISC::atoiInt64(value.c_str()));
		}
		else
		{
			uint32 val = atoi(value.substr(1).c_str());
			if(dir == 1)
				guild->addMoney(val);
			else
				guild->spendMoney(val);
		}

		nlinfo ("Player %s guild money is %"NL_I64"u", entity.toString().c_str(),guild->getMoney());
	}
}

ENTITY_VARIABLE(FactionPoint, "FactionPoint")
{
	ENTITY_GET_CHARACTER

	if (get)
	{
		string sTmp;

		for (uint i = PVP_CLAN::BeginClans; i <= PVP_CLAN::EndClans; ++i)
			sTmp += PVP_CLAN::toString((PVP_CLAN::TPVPClan)i) + " : " + toString(c->getFactionPoint((PVP_CLAN::TPVPClan)i)) + "\n";

		value = sTmp;
	}
	else
	{
		string sFactionName = value.substr(0, value.find(','));
		string sNumber = value.substr(value.find(',')+1);

		if (value.empty() || sFactionName.empty() || sNumber.empty())
		{
			nlwarning ("You must provide : <faction_name> [<sign>]<value>");
			return;
		}

		sint dir = 0;
		if(sNumber[0] == '+')			dir = +1;
		else if(sNumber[0] == '-')	dir = -1;
		else if(!isdigit(sNumber[0]))
		{
			nlwarning ("Bad format for '%s' right format is [<sign>]<value> sign can be +,- or nothing", sNumber.c_str());
			return;
		}

		PVP_CLAN::TPVPClan clan = PVP_CLAN::fromString(sFactionName);
		if ((clan < PVP_CLAN::BeginClans) || (clan > PVP_CLAN::EndClans))
		{
			nlwarning ("'%s' is not a valid faction name", sFactionName.c_str());
			return;
		}

		if(dir == 0)
		{
			c->setFactionPoint(clan, atoi(sNumber.c_str()), true);
		}
		else
		{
			uint32 val = atoi(sNumber.substr(1).c_str());
			if(dir == 1)
				c->setFactionPoint(clan, c->getFactionPoint(clan)+val, true);
			else
				c->setFactionPoint(clan, c->getFactionPoint(clan)-val, true);
		}
	}
}


ENTITY_VARIABLE (Priv, "User privilege")
{
	ENTITY_GET_CHARACTER

	CPlayer *p = PlayerManager.getPlayer(PlayerManager.getPlayerId(c->getId()));
	if (p == 0)
	{
		nlwarning ("Can't find player with UserId %d for checking privilege, assume no priv", PlayerManager.getPlayerId(c->getId()));
		return;
	}

	if (get)
	{
		value = p->getUserPriv();
	}
	else
	{
		p->setUserPriv(value);
		nlinfo ("%s have now this priv '%s'", entity.toString().c_str(), value.c_str());
	}

	return;
}

ENTITY_VARIABLE (FBT, "Focus Beta Tester")
{
	ENTITY_GET_CHARACTER

	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( c->getId() ));
	if (p == NULL)
	{
		nlwarning ("Can't find player with UserId %d", PlayerManager.getPlayerId( c->getId() ));
		return;
	}

	if (get)
	{
		value = NLMISC::toString(p->isBetaTester());
	}
	else
	{
		if (value == "1" || value == "on" || strlwr(value) == "true")
		{
			p->isBetaTester(true);
		}
		else if (value == "0" || value == "off" || strlwr(value) == "false")
		{
			p->isBetaTester(false);
		}
		nlinfo ("%s %s now a Focus Beta Tester", entity.toString().c_str(), (p->isBetaTester() ? "is" : "isn't"));
	}
}

ENTITY_VARIABLE(Position, "Position of a player (in meter) <eid> <posx>,<posy>[,<posz>][,<season>] | <bot_name> | <player_name> | home | <creature>")
{
	H_AUTO(Position);

	ENTITY_GET_ENTITY

	vector<string> res;

	sint32 x = 0, y = 0, z = 0;
	if (get)
	{
		x = e->getState().X() / 1000;
		y = e->getState().Y() / 1000;
		z = e->getState().Z() / 1000;

		value = toString ("%d,%d,%d", x, y, z);
	}
	else
	{
		if ( value.find(',') != string::npos )
		{
			explode (value, string(","), res);
			if (res.size() >= 2)
			{
				x = atoi (res[0].c_str()) * 1000;
				y = atoi (res[1].c_str()) * 1000;
			}
			if (res.size() >= 3)
				z = atoi (res[2].c_str()) * 1000;
		}
		else
		{
			if ( value.find(".creature") != string::npos )
			{
				CSheetId creatureSheetId(value);
				if( creatureSheetId != CSheetId::Unknown )
				{
					double minDistance = -1.;
					CCreature * creature = NULL;

					TMapCreatures::const_iterator it;
					const TMapCreatures& creatures = CreatureManager.getCreature();
					for( it = creatures.begin(); it != creatures.end(); ++it )
					{
						CSheetId sheetId = (*it).second->getType();
						if( sheetId == creatureSheetId )
						{
							double distance = PHRASE_UTILITIES::getDistance( e->getEntityRowId(), (*it).second->getEntityRowId() );
							if( !creature || (creature && distance < minDistance) )
							{
								creature = (*it).second;
								minDistance = distance;
							}
						}
					}
					if( creature )
					{
						x = creature->getState().X();
						y = creature->getState().Y();
						z = creature->getState().Z();
					}
				}
				else
				{
					nlwarning ("<Position> '%s' is an invalid creature", value.c_str());
				}
			}
			else
			{
				// try to find the player name
				CCharacter *c = dynamic_cast<CCharacter*>(e);
				if (c == NULL)
				{
					nlwarning ("<Position> '%s' is not a player", entity.toString().c_str());
					return;
				}
				CEntityBase *entityBase = PlayerManager.getCharacterByName (CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), value));
				if (entityBase == 0)
				{
					// try to find the bot name
					vector<TAIAlias> aliases;
					CAIAliasTranslator::getInstance()->getNPCAliasesFromName( value, aliases );
					if ( aliases.empty() )
					{
						nldebug ("<Position> Ignoring attempt to teleport because no NPC found matching name '%s'", value.c_str());
						return;
					}

					TAIAlias alias = aliases[0];

					const CEntityId & botId = CAIAliasTranslator::getInstance()->getEntityId (alias);
					if ( botId != CEntityId::Unknown )
					{
						entityBase = CreatureManager.getCreature (botId);
					}
					else
					{
						nlwarning ("'%s' has no eId. Is it Spawned???", value.c_str());
						return;
					}

				}
				if (entityBase != 0)
				{
					x = entityBase->getState().X + sint32 (cos (entityBase->getState ().Heading) * 2000);
					y = entityBase->getState().Y + sint32 (sin (entityBase->getState ().Heading) * 2000);
					z = entityBase->getState().Z;
				}
			}
		}

		if (x == 0 && y == 0 && z == 0)
		{
			nlwarning ("'%s' is a bad value for position, don't change position", value.c_str());
			return;
		}

		CCharacter * c = dynamic_cast<CCharacter*>(e);
		if( c )
		{
			CContinent * cont = CZoneManager::getInstance().getContinent(x,y);
			if(c->getCurrentContinent() == CONTINENT::NEWBIELAND )
			{
				if( cont == 0 || cont->getId() != CONTINENT::NEWBIELAND )
				{
					log_Command_TPOutsideNewbieland(c->getId());
//					nlwarning("Position %s player outside NEWBIELAND, this is logged.", c->getId().toString().c_str());
				}
			}

			c->allowNearPetTp();
			if (res.size() >= 4)
			{
				// season included
				c->teleportCharacter(x,y,z,true, false, 0.f, 0xFF, 0, (uint8) atoi(res[3].c_str()));
			}
			else
			{
				c->teleportCharacter(x,y,z,true);
			}

			if ( cont )
			{
				c->getRespawnPoints().addDefaultRespawnPoint( CONTINENT::TContinent(cont->getId()) );
			}
		}
		else
		{
			e->tpWanted (x, y, z);
		}
	}
}

//-----------------------------------------------
// change_mode :
//-----------------------------------------------
NLMISC_COMMAND(changeMode," change_mode","<entity id(id:type:crea:dyn)> <mode>")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		MBEHAV::EMode mode = MBEHAV::EMode(atoi(args[1].c_str()));

		CEntityBase *e = 0;
		if( id.getType() == 0 )
		{
			e = PlayerManager.getChar(id);
		}
		if (e)
		{
			e->setMode( mode, true );
			log.displayNL("Character %s mode changed to %d", id.toString().c_str(), mode);
		}
		else
		{
			log.displayNL("Unknown entity %s ",id.toString().c_str());
		}
		return true;
	}
	return false;
} // change_mode //


//-----------------------------------------------
// checkTargetSP :check target player SP
//-----------------------------------------------
NLMISC_COMMAND(checkTargetSP," check target player skill points (if there is lost SP)","<entity id(id:type:crea:dyn)>")
{
	if( args.size() < 1 )
		return false;

	GET_CHARACTER

	CCharacter *targetPlayer = PlayerManager.getChar( c->getTargetDataSetRow() );
	if (!targetPlayer)
	{
		log.displayNL("No target or invalid target");
		return true;
	}

	uint32 totalEarnedSp = targetPlayer->getTotalEarnedSP();
	uint32 spentSp = targetPlayer->getActionsSPValue() - targetPlayer->getStartActionsSPValue();

	uint32 currentSp = (uint32) targetPlayer->getSP(EGSPD::CSPType::Craft);
	currentSp += (uint32) targetPlayer->getSP(EGSPD::CSPType::Fight);
	currentSp += (uint32) targetPlayer->getSP(EGSPD::CSPType::Magic);
	currentSp += (uint32) targetPlayer->getSP(EGSPD::CSPType::Harvest);

	// log result
	log.displayNL("Player %s : Current SP total = %u, totalEarnedSp = %u, spentTotalSP = %u, MISSING SP = %d", targetPlayer->getId().toString().c_str(), currentSp, totalEarnedSp, spentSp, totalEarnedSp-spentSp-currentSp);

	// send result to asker
	string strTemp = NLMISC::toString("Player %s : Current SP total = %u, totalEarnedSp = %u, spentTotalSP = %d, MISSING SP = %d",targetPlayer->getId().toString().c_str(), currentSp, totalEarnedSp, spentSp, totalEarnedSp-spentSp-currentSp);
	chatToPlayer(eid, strTemp);

	return true;
} // checkSP //


NLMISC_COMMAND (createItemInBag, "Create an item and put it in the player bag", "<eid> <sheetId>|<sheetName> <quantity> <quality> [force]")
{
	if (args.size() < 4 || args.size() > 5) return false;
	GET_CHARACTER

	string sheetName = args[1];
	CSheetId sheet;
	uint32 sheetId = atoi(sheetName.c_str());
	if (sheetId != 0)
	{
		sheet = CSheetId(sheetId);
	}
	else
	{
		if (sheetName.find(".") == string::npos)
			sheetName += ".sitem";
		sheet = CSheetId(sheetName.c_str());
	}

	if (sheet == CSheetId::Unknown)
	{
		log.displayNL("sheetId '%s' is Unknown", sheetName.c_str());
		return false;
	}

	// banners are the only items in game which use privilege
	if( sheetName.find("banner") != string::npos )
	{
		CPlayer * player = PlayerManager.getPlayer( PlayerManager.getPlayerId(eid) );
//		if (player != NULL && !player->havePriv(":DEV:") )
		if (player != NULL && player->havePriv(BannerPriv) )
		{
			if( sheetName.find("_gu") != string::npos && !player->havePriv(":G:") )	return false;
			if( sheetName.find("_sgu") != string::npos && !player->havePriv(":SG:") )	return false;
			if( sheetName.find("_vgu") != string::npos && !player->havePriv(":VG:") )	return false;
			if( sheetName.find("_gm") != string::npos && !player->havePriv(":GM:") )	return false;
			if( sheetName.find("_sgm") != string::npos && !player->havePriv(":SGM:") )	return false;
		}
	}

	const CStaticItem *form = CSheets::getForm (sheet);
	if (form == 0)
	{
		log.displayNL ("sheetId '%s' is not found", sheetName.c_str());
		return false;
	}

	sint quantity = atoi(args[2].c_str());
	if (quantity < 1)
	{
		log.displayNL ("Quantity must be > 0");
		return false;
	}

	sint quality = atoi(args[3].c_str());
	if (quality < 1)
	{
		log.displayNL ("Quality must be > 0");
		return false;
	}

	bool res = c->createItemInInventory(INVENTORIES::bag, quality, quantity, sheet);

	bool forceCreate;
	if (args.size() == 5 && args[4] == "force")
		forceCreate = true;
	else
		forceCreate = false;

	if (!res && forceCreate)
	{
		CGameItemPtr item = GameItemManager.createItem(sheet, quality, true, true);
		if (item != NULL)
		{
			res = c->addItemToInventory(INVENTORIES::bag, item);
			if (!res)
				item.deleteItem();
		}
	}

	if (res)
		log.displayNL ("Item '%s' created and set in the bag of the player %s", sheet.toString ().c_str(), eid.toString().c_str());
	else
		log.displayNL ("Failed to create and set the item '%s' for the player %s", sheet.toString ().c_str(), eid.toString().c_str());

	return res;
}

NLMISC_COMMAND (createItemInTmpInv, "Create an item and put it in the player temp inventory", "<eid> <sheetId>|<sheetName> <quantity> <quality>")
{
	if (args.size () != 4) return false;
	GET_CHARACTER

	string sheetName = args[1];
	CSheetId sheet;
	uint32 sheetId = atoi(sheetName.c_str());
	if (sheetId != 0)
	{
		sheet = CSheetId(sheetId);
	}
	else
	{
		if (sheetName.find(".") == string::npos)
			sheetName += ".item";
		sheet = CSheetId(sheetName.c_str());
	}

	if (sheet == CSheetId::Unknown)
	{
		log.displayNL("sheetId '%s' is Unknown", sheetName.c_str());
		return false;
	}

	// banners are the only items in game which use privilege
	if( sheetName.find("banner") != string::npos )
	{
		CPlayer * player = PlayerManager.getPlayer( PlayerManager.getPlayerId(eid) );
		if (player != NULL && player->havePriv(BannerPriv) )
		{
			if( sheetName.find("_gu") != string::npos && !player->havePriv(":G:") )	return false;
			if( sheetName.find("_sgu") != string::npos && !player->havePriv(":SG:") )	return false;
			if( sheetName.find("_vgu") != string::npos && !player->havePriv(":VG:") )	return false;
			if( sheetName.find("_gm") != string::npos && !player->havePriv(":GM:") )	return false;
			if( sheetName.find("_sgm") != string::npos && !player->havePriv(":SGM:") )	return false;
		}
	}

	const CStaticItem *form = CSheets::getForm (sheet);
	if (form == 0)
	{
		log.displayNL ("sheetId '%s' is not found", sheetName.c_str());
		return false;
	}

	sint quantity = atoi(args[2].c_str());
	if (quantity < 1)
	{
		log.displayNL ("Quantity must be > 0");
		return false;
	}

	sint quality = atoi(args[3].c_str());
	if (quality < 1)
	{
		log.displayNL ("Quality must be > 0");
		return false;
	}

	bool res = c->createItemInInventory(INVENTORIES::temporary, quality, quantity, sheet);

	if (res)
		log.displayNL ("Item '%s' created and set in the temp inventory of the player %s", sheet.toString ().c_str(), eid.toString().c_str());
	else
		log.displayNL ("Failed to create and set the item '%s' for the player %s", sheet.toString ().c_str(), eid.toString().c_str());

	return res;
}

NLMISC_COMMAND (createItemInInv, "Create items and put them in the given inventory", "<eid> <invId> <sheetId>|<sheetName> <quantity> <quality> <nb items>")
{
	if (args.size () != 6) return false;
	GET_CHARACTER

	string sheetName = args[2];
	CSheetId sheet;
	uint32 sheetId = atoi(sheetName.c_str());
	if (sheetId != 0)
	{
		sheet = CSheetId(sheetId);
	}
	else
	{
		if (sheetName.find(".") == string::npos)
			sheetName += ".sitem";
		sheet = CSheetId(sheetName.c_str());
	}

	if (sheet == CSheetId::Unknown)
	{
		log.displayNL("sheetId '%s' is Unknown", sheetName.c_str());
		return false;
	}

	// banners are the only items in game which use privilege
	if( sheetName.find("banner") != string::npos )
	{
		CPlayer * player = PlayerManager.getPlayer( PlayerManager.getPlayerId(eid) );
		if (player != NULL && player->havePriv(BannerPriv) )
		{
			if( sheetName.find("_gu") != string::npos && !player->havePriv(":G:") )	return false;
			if( sheetName.find("_sgu") != string::npos && !player->havePriv(":SG:") )	return false;
			if( sheetName.find("_vgu") != string::npos && !player->havePriv(":VG:") )	return false;
			if( sheetName.find("_gm") != string::npos && !player->havePriv(":GM:") )	return false;
			if( sheetName.find("_sgm") != string::npos && !player->havePriv(":SGM:") )	return false;
		}
	}

	const CStaticItem *form = CSheets::getForm (sheet);
	if (form == 0)
	{
		log.displayNL ("sheetId '%s' is not found", sheetName.c_str());
		return false;
	}

	sint quantity = atoi(args[3].c_str());
	if (quantity < 1)
	{
		log.displayNL ("Quantity must be > 0");
		return false;
	}

	sint quality = atoi(args[4].c_str());
	if (quality < 1)
	{
		log.displayNL ("Quality must be > 0");
		return false;
	}

	uint nbItems = min(uint(atoi(args[5].c_str())), 1000U);
	uint nbCreatedItems = 0;
	for (uint i = 0; i < nbItems; i++)
	{
		if ( !c->createItemInInventory(INVENTORIES::toInventory(args[1]), quality, quantity, sheet, CEntityId::Unknown) )
			break;
		nbCreatedItems++;
	}

	log.displayNL("%u (of %u) items '%s' created and set in the inventory '%s' of the player %s", nbCreatedItems, nbItems, sheet.toString ().c_str(), args[1].c_str(), eid.toString().c_str());

	return true;
}

NLMISC_COMMAND(createNamedItemInBag, "create a named item in bag", "<eId> <item> [<quantity>]")
{
	if (args.size() > 3)
		return false;

	if (args.size() < 2 )
		return false;

	CCharacter * user = PlayerManager.getChar(CEntityId(args[0]));
	if (!user)
	{
		log.displayNL("invalid char '%s'", args[0].c_str());
		return true;
	}

	uint16 quantity;
	if (args.size() == 3)
	{
		quantity = (uint16)atoi(args[2].c_str());
		if (quantity == 0)
		{
			log.displayNL("invalid quantity '%s'", args[2].c_str());
			return true;
		}
	}
	else
	{
		quantity = 1;
	}

	CGameItemPtr item = CNamedItems::getInstance().createNamedItem(args[1], quantity);
	if (item == NULL)
	{
		log.displayNL("invalid item '%s'", args[1].c_str());
		return true;
	}

	user->addItemToInventory(INVENTORIES::bag, item);

	return true;
}

NLMISC_COMMAND (createFullArmorSet, "Create and equip player with chosen full armor set","<eid> <race (fyros/matis/zorai/tryker)> <type (light/medium/heavy)> <quality>")
{
	static string armors[]=
	{
		"boots",
		"gloves",
		"pants",
		"sleeves",
		"vest",
	};
	static string heavyArmors[]=
	{
		"boots",
		"gloves",
		"helmet",
		"pants",
		"sleeves",
		"vest",
	};

	if (args.size () != 4) return false;
	GET_CHARACTER

	const string &race = args[1];
	const string &type = args[2];
	const uint16 quality = atoi(args[3].c_str());

	const uint16 min = quality - (quality-1)%5;
	const string minStr = (min < 10) ? string("0") + toString(min) : toString(min);
	const uint16 max = min+4;
	const string maxStr = (max < 10) ? string("0") + toString(max) : toString(max);

	const string endName = string("_") + minStr + string("_") + maxStr + string(".item");

	// create the armor pieces
	if (type == "heavy")
	{
		for ( int i = 0 ; i < sizeof(heavyArmors)/sizeof(heavyArmors[0]) ; ++i)
		{
			const string sheetName = race + string("_") + type + string("_") + heavyArmors[i] + endName;
			CSheetId sheetId(sheetName);

			if ( !c->createItemInInventory(INVENTORIES::bag, quality, 1, sheetId, eid) )
			{
				log.displayNL ("For entity %s Failed to create item %s", eid.toString().c_str(), sheetName.c_str() );
			}
		}
	}
	else
	{
		for ( int i = 0 ; i < sizeof(armors)/sizeof(armors[0]) ; ++i)
		{
			const string sheetName = race + string("_") + type + string("_") + armors[i] + endName;
			CSheetId sheetId(sheetName);

			if ( !c->createItemInInventory(INVENTORIES::bag, quality, 1, sheetId, eid) )
			{
				log.displayNL ("For entity %s Failed to create item %s", eid.toString().c_str(), sheetName.c_str() );
			}
		}
	}
	return true;
}

NLMISC_COMMAND (displayInfosOnTarget, "display info on entity target", "<eid>")
{
	if (args.size () < 1) return false;
	GET_CHARACTER

	TDataSetRow target = c->getTargetDataSetRow();
	if (TheDataset.isAccessible(target))
	{
		// little slow but acceptable in command
		CMirrorPropValueRO<uint32>	instanceNumber(TheDataset, target, DSPropertyAI_INSTANCE);

		CAIAskForInfosOnEntityMsg msg;
		msg.EntityRowId = target;
		msg.AskerRowID = c->getEntityRowId();

		CWorldInstances::instance().msgToAIInstance(instanceNumber, msg);
	}
	else
		log.displayNL ("<displayInfosOnTarget> Entity %s has no valid target", eid.toString().c_str() );

	return true;
}

NLMISC_COMMAND (forceTargetToDie, "(debug) Force entity target to die", "<eid>")
{
	if (args.size () != 1)
		return false;

	GET_CHARACTER

	// force sending of target as dead (can only be done on creature)
	CCreature *target = CreatureManager.getCreature(c->getTargetDataSetRow());
	if (target)
	{
		if (!target->isDead())
			target->changeCurrentHp(-100000);

		BotDeathReport.Bots.push_back(target->getEntityRowId());
		TDataSetRow emptyRow;
		BotDeathReport.Killers.push_back(emptyRow);
		BotDeathReport.Zombies.push_back(true);
	}

	return true;
}

NLMISC_COMMAND (learnAllBricks, "Specified player learns all possible bricks for it's skill values", "<eid>")
{
	H_AUTO(learnAllBricks);

	if (args.size () != 1)
		return false;

	GET_CHARACTER

	// get all phrases and bypass brick requirements

	vector<CSheetId> phrases;
	vector<CSheetId> tempPhrases;

	static const EGSPD::CPeople::TPeople races[] =
	{
		EGSPD::CPeople::Common,
		EGSPD::CPeople::Tryker,
		EGSPD::CPeople::Fyros,
		EGSPD::CPeople::Matis,
		EGSPD::CPeople::Zorai
	};

	const uint nbRaces = sizeof(races) / sizeof(races[0]);

	for (uint i = 0 ; i < nbRaces ; ++i)
	{
		// fight
		string filter = "bf";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// magic
		filter = "bm";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// craft
		filter = "bc";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// harvest
		filter = "bh";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// special powers
		filter = "bs";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
	}

	// learn all bricks
	uint nbBricks = 0;
	for (uint i = 0; i < phrases.size() ; ++i)
	{
		const CStaticRolemasterPhrase *phrase = CSheets::getSRolemasterPhrase(phrases[i]);
		if(phrase != NULL)
		{
			const uint size = (uint)phrase->Bricks.size();
			for ( uint j = 0; j < size ; ++j)
			{
				c->addKnownBrick( phrase->Bricks[j] );
				++nbBricks;
			}
		}
	}

	log.displayNL ("%s learnt %u bricks", eid.toString().c_str(), nbBricks);

	return true;
}


NLMISC_COMMAND (learnAllRolemasterBricks, "Specified player learns all possible bricks sold by rolemaster for it's skill values", "<eid>")
{
	H_AUTO(learnAllRolemasterBricks);

	if (args.size () != 1)
		return false;

	GET_CHARACTER

		// get all phrases and bypass brick requirements

		vector<CSheetId> phrases;
	vector<CSheetId> tempPhrases;

	static const EGSPD::CPeople::TPeople races[] =
	{
		EGSPD::CPeople::Common,
			EGSPD::CPeople::Tryker,
			EGSPD::CPeople::Fyros,
			EGSPD::CPeople::Matis,
			EGSPD::CPeople::Zorai
	};

	const uint nbRaces = sizeof(races) / sizeof(races[0]);

	for (uint i = 0 ; i < nbRaces ; ++i)
	{
		// fight
		string filter = "bf";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true, false );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// magic
		filter = "bm";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true, false );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// craft
		filter = "bc";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true, false );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// harvest
		filter = "bh";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true, false );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// special powers
		filter = "bs";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true, false );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
	}

	// learn all bricks
	uint nbBricks = 0;
	for (uint i = 0; i < phrases.size() ; ++i)
	{
		const CStaticRolemasterPhrase *phrase = CSheets::getSRolemasterPhrase(phrases[i]);
		if(phrase != NULL)
		{
			const uint size = (uint)phrase->Bricks.size();
			for ( uint j = 0; j < size ; ++j)
			{
				c->addKnownBrick( phrase->Bricks[j] );
				++nbBricks;
			}
		}
	}

	log.displayNL ("%s learnt %u bricks", eid.toString().c_str(), nbBricks);

	return true;
}


NLMISC_COMMAND (learnAllPhrases, "Specified player learns all possible phrases for it's skill values", "<eid>")
{
	H_AUTO(learnAllPhrases);

	if (args.size () != 1)
		return false;

	GET_CHARACTER

	// get all phrases and bypass brick requirements
	vector<CSheetId> phrases;
	vector<CSheetId> tempPhrases;

	static const EGSPD::CPeople::TPeople races[] =
	{
		EGSPD::CPeople::Common,
		EGSPD::CPeople::Tryker,
		EGSPD::CPeople::Fyros,
		EGSPD::CPeople::Matis,
		EGSPD::CPeople::Zorai
	};

	const uint nbRaces = sizeof(races) / sizeof(races[0]);

	for (uint i = 0 ; i < nbRaces ; ++i)
	{
		// fight
		string filter = "bf";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// magic
		filter = "bm";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// craft
		filter = "bc";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// harvest
		filter = "bh";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
		// special powers
		filter = "bs";
		c->getAvailablePhrasesList( filter, tempPhrases, races[i], true );
		phrases.insert( phrases.end(), tempPhrases.begin(), tempPhrases.end() );
		tempPhrases.clear();
	}

	// learn all phrases
	uint16 nbPhrases = 0;
	for (uint i = 0; i < phrases.size() ; ++i)
	{
		uint16 index = c->getFirstFreeSlotInKnownPhrase();

		if ( !c->learnPrebuiltPhrase(phrases[i], index) )
			log.displayNL ("%s FAILED to learn phrase id %s", eid.toString().c_str(), phrases[i].toString().c_str());
		else
			++nbPhrases;
	}

	log.displayNL ("%s learnt %u phrases", eid.toString().c_str(), nbPhrases);

	return true;
}


NLMISC_COMMAND(learnBrick, "Specified player learns given brick","<eid> <brickid>")
{
	if (args.size () != 2) return false;
	GET_CHARACTER

	CSheetId brickId(args[1]);

	c->addKnownBrick(brickId);

	log.displayNL ("%s learnt brick %s", eid.toString().c_str(), brickId.toString().c_str());
	return true;
}


NLMISC_COMMAND(unlearnBrick, "Specified player unlearns given brick","<eid> <brickid>")
{
	if (args.size () != 2) return false;
	GET_CHARACTER

	CSheetId brickId(args[1]);

	c->removeKnownBrick(brickId);

	log.displayNL ("%s unlearnt brick %s", eid.toString().c_str(), brickId.toString().c_str());
	return true;
}


NLMISC_COMMAND (learnPhrase, "Specified player learns a phrase and set it at index knownPhraseIndex", "<eid> <phraseSheet>")
{
	if (args.size () != 2) return false;
	GET_CHARACTER

	CSheetId phraseId(args[1]);
	const CStaticRolemasterPhrase *phrase = CSheets::getSRolemasterPhrase( phraseId );
	if (!phrase)
	{
		log.displayNL ("%s isn't a valid phrase !", phraseId.toString().c_str());
		return false;
	}

	uint16 index = c->getFirstFreeSlotInKnownPhrase();

	if ( c->learnPrebuiltPhrase(phraseId, index) )
		log.displayNL ("%s learnt phrase id %s", eid.toString().c_str(), phraseId.toString().c_str());
	else
		log.displayNL ("%s FAILED to learn phrase id %s", eid.toString().c_str(), phraseId.toString().c_str());
	return true;
}

NLMISC_COMMAND (logXpGain, "log or not xp gain infos for specified player", "<eid> <on/off>")
{
	if (args.size () != 2) return false;
	GET_CHARACTER

	bool flag;
	if (args[1]=="1" || strlwr(args[1])=="on" || strlwr(args[1])=="true" )
	{
		flag = true;
	}
	else if (args[1]=="0" || strlwr(args[1])=="off" || strlwr(args[1])=="false" )
	{
		flag = false;
	}
	else
		return false;

	PlayerManager.logXPGain(c->getEntityRowId(), flag);
	log.displayNL ("Player %s. Flag logXpGain is now %s", eid.toString().c_str(), (flag?"ON":"OFF") );


	return true;
}

NLMISC_COMMAND (learnAllForagePhrases, "Learn all forage phrase, begin at specified index", "<eid> <index>" )
{
	H_AUTO(learnAllForagePhrases);

	if ( args.size() != 2 ) return false;
	GET_CHARACTER

	uint i = atoi( args[1].c_str() );

	uint nbSuccess = 0, nbFail = 0;
	const CAllRolemasterPhrases& phrases = CSheets::getSRolemasterPhrasesMap();
	for ( CAllRolemasterPhrases::const_iterator ip=phrases.begin(); ip!=phrases.end(); ++ip )
	{
		const string phraseCode = (*ip).first.toString();
		if ( phraseCode.find( "abhf" ) == 0 )
		{
			if ( c->learnPrebuiltPhrase( (*ip).first, i ) )
				++nbSuccess;
			else
				++nbFail;
			++i;
		}
	}
	log.displayNL( "%u phrases learned, %u phrases failed", nbSuccess, nbFail );
	return true;
}


/*
NLMISC_COMMAND (learnAllFaberPlans, "Specified player learns all faber plans", "<eid>")
{
	if (args.size () != 1) return false;
	GET_CHARACTER

	c->learnAllFaberPlans();
	log.displayNL ("%s learnt all faber plans", eid.toString().c_str());

	return true;
}
*/

/* Don't use this command, use Money variable instead (same syntax for example  "Money <eid> +10")
NLMISC_COMMAND(giveMoney, "Give money to a player","<eid> <money>")
{
	if (args.size () != 2) return false;
	GET_CHARACTER

	c->giveMoney ( NLMISC::atoiInt64( args[1].c_str() ) );
	log.displayNL ("You gave money to player %s", eid.toString().c_str());

	return true;
}*/

/* Don't use this command, use SP variable instead (same syntax for example  "SP <eid> +10")
NLMISC_COMMAND(giveSP, "Give SP (skill points) to a player","<eid> <SP>")
{
	if (args.size () != 2) return false;
	GET_CHARACTER

	uint32 nbSp = atol(args[1].c_str());

	c->addSP( (double)nbSp );
	log.displayNL ("You gave %u SP to player %s", nbSp, eid.toString().c_str());

	return true;
}*/

NLMISC_COMMAND(respawn, "Same as '/a Position home' but without any privilege","<eid>")
{
	if (args.size () != 1) return false;
	GET_CHARACTER

//TODO rules of spawn point
//	COfflineEntityState state = PlayerManager.getStartState (c->getActiveRole ());
	COfflineEntityState state;
	c->forbidNearPetTp();
	c->tpWanted (state.X, state.Y, state.Z);

	return true;
}

//-----------------------------------------------
// saveToXML
//-----------------------------------------------
NLMISC_CATEGORISED_COMMAND(pdr,saveToXML,"save a character to an XML file","<eid> <file name>")
{
	if (args.size () < 2) return false;
	GET_CHARACTER

	std::string fileName = args[1];

	if( c )
	{
		static CPersistentDataRecordRyzomStore	pdr;
		pdr.clear();
		c->store(pdr);
		pdr.writeToTxtFile((fileName+".xml").c_str());
		return true;
	}

	return false;
}


//-----------------------------------------------
// loadFromXML
//-----------------------------------------------
NLMISC_CATEGORISED_COMMAND(pdr,loadFromXML,"load a character from an XML file","<eid> <file name>")
{
	if (args.size () < 2) return false;
	GET_CHARACTER

	std::string fileName = args[1];

	if( c )
	{
		ucstring			name=	 c->getName();
		uint32				guildId= c->getGuildId();
		NLMISC::CEntityId	id=		 c->getId();

		static CPersistentDataRecord	pdr;
		pdr.clear();
		pdr.readFromTxtFile((fileName+".xml").c_str());
		c->apply(pdr);
		c->setName(name);
		c->setGuildId(guildId);
		c->setId(id);

		c->allowNearPetTp();
		c->tpWanted(c->getX(), c->getY(), c->getZ(), true, c->getHeading());

		return true;
	}

	return false;
}


//-----------------------------------------------
// saveToPDR
//-----------------------------------------------
NLMISC_CATEGORISED_COMMAND(pdr,saveToPDR,"save a character to a binary PDR file","<eid> <file name>")
{
	if (args.size () < 2) return false;
	GET_CHARACTER

		std::string fileName = args[1];

	if( c )
	{
		static CPersistentDataRecordRyzomStore	pdr;
		pdr.clear();
		c->store(pdr);
		pdr.writeToBinFile((fileName+".pdr").c_str());
		return true;
	}

	return false;
}


//-----------------------------------------------
// loadFromPDR
//-----------------------------------------------
NLMISC_CATEGORISED_COMMAND(pdr,loadFromPDR,"load a character from a binary PDR file","<eid> <file name>")
{
	if (args.size () < 2) return false;
	GET_CHARACTER

		std::string fileName = args[1];

	if( c )
	{
		ucstring			name=	 c->getName();
		uint32				guildId= c->getGuildId();
		NLMISC::CEntityId	id=		 c->getId();

		static CPersistentDataRecord	pdr;
		pdr.clear();
		pdr.readFromBinFile((fileName+".pdr").c_str());
		c->apply(pdr);

		c->setName(name);
		c->setGuildId(guildId);
		c->setId(id);

		c->allowNearPetTp();
		c->tpWanted(c->getX(), c->getY(), c->getZ(), true, c->getHeading());

		return true;
	}

	return false;
}


//-----------------------------------------------
// listPDRFiles
//-----------------------------------------------
NLMISC_CATEGORISED_COMMAND(pdr,listPDRFiles,"list files in the current directory","[<wildcard>]")
{
	if (args.size()!=0)
		return false;

	std::vector<std::string> files;
	NLMISC::CPath::getPathContent(".",false,false,true,files);
	for (uint32 i=(uint32)files.size();i--;)
	{
		if (!NLMISC::testWildCard(files[i],"*.xml") && !NLMISC::testWildCard(files[i],"*.pdr"))
		{
			files[i]=files.back();
			files.pop_back();
		}
	}
	std::sort(files.begin(),files.end());
	for (uint32 i=0;i<files.size();++i)
	{
		log.displayNL("%-40s %10d",files[i].c_str(),NLMISC::CFile::getFileSize(files[i]));
	}

	return true;
}


//-----------------------------------------------
// addPetAnimal
//-----------------------------------------------
NLMISC_COMMAND(addPetAnimal,"Add pet animal to character","<eid> <PetTicket>")
{
	if (args.size () < 2) return false;
	GET_CHARACTER

	CSheetId ticket = CSheetId( args[1] );

	if( c )
	{
		if( ticket != CSheetId::Unknown )
		{
			CGameItemPtr item = c->createItemInInventoryFreeSlot(INVENTORIES::bag, 1, 1, ticket);
			if( item != 0 )
				c->addCharacterAnimal( ticket, 0, item );
			return true;
		}
		else
		{
			log.displayNL("<addPetAnimal> command, unknown pet ticket '%s'", args[1].c_str() );
		}
	}

	return false;
} // addPetAnimal //

NLMISC_COMMAND(setPetAnimalSatiety,"Set the satiety of pet animal (petIndex in 0..3)","<eid> <petIndex> full|<value> [<nameForAnswer>]")
{
	if (args.size () < 3) return false;
	GET_CHARACTER

	if ( c )
	{
		// Set the new satiety
		string result;
		uint petIndex = atoi( args[1].c_str() );
		float previousSatiety, maxSatiety;
		if ( c->getAnimalSatiety( petIndex, previousSatiety, maxSatiety ) )
		{
			if ( args[2] == "full" )
				c->setAnimalSatietyToMax( petIndex );
			else
			{
				float value = (float)atof( args[2].c_str() );
				c->setAnimalSatiety( petIndex, value );
			}
			float currentSatiety;
			if ( c->getAnimalSatiety( petIndex, currentSatiety, maxSatiety ) )
				result = toString( "Satiety changed from %.1f to %.1f / %.1f", previousSatiety, currentSatiety, maxSatiety );
			else
				result = toString( "Internal error" );
		}
		else
			result = toString( "Pet %u not found", petIndex );

		// Send result if requested
		if ( args.size() > 3 )
		{
			CCharacter *addressee = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), args[3]) );
			if ( addressee )
			{
				CHECK_RIGHT( addressee, c );
				CCharacter::sendDynamicSystemMessage( addressee->getId(), result );
			}
			else
				result += " nameForAnswer not found";
		}

		log.displayNL( result.c_str() );
	}

	return true;
}

NLMISC_COMMAND(getPetAnimalSatiety,"Set the satiety of pet animal (petIndex in 0..3)","<eid> <petIndex> [<nameForAnswer>]")
{
	if (args.size () < 2) return false;
	GET_CHARACTER

	uint petIndex = atoi( args[1].c_str() );
	if ( c )
	{
		// Get the satiety
		string result;
		float currentSatiety, maxSatiety;
		if ( c->getAnimalSatiety( petIndex, currentSatiety, maxSatiety ) )
			result = toString( "Satiety: %.1f / %.1f", currentSatiety, maxSatiety );
		else
			result = toString( "Pet %u not found", petIndex );

		// Send result if requested
		if ( args.size() > 2 )
		{
			CCharacter *addressee = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), args[2] ));
			if ( addressee )
			{
				CHECK_RIGHT( addressee, c );
				CCharacter::sendDynamicSystemMessage( addressee->getId(), result );
			}
			else
				result += " nameForAnswer not found";
		}

		log.displayNL( result.c_str() );
	}
	return true;
}

NLMISC_COMMAND (addSkillPoints, "add skill points of given type (Fight = 0,	Magic = 1,Craft = 2, Harvest = 3)", "<eid> <SP type [0..3]> <nb SP>")
{
	if (args.size () < 3) return false;
	GET_CHARACTER

	EGSPD::CSPType::TSPType type = (EGSPD::CSPType::TSPType) atoi (args[1].c_str());
	if (type >= EGSPD::CSPType::EndSPType)
	{
		log.displayNL ("Unknown SP type %u", atoi (args[1].c_str()) );
		return false;
	}

	uint32 nbSP = atoi (args[2].c_str());

	c->addSP( nbSP, type );

	log.displayNL ("Added %d skill points of type %u (%s) to player %s", nbSP, (uint8) type, EGSPD::CSPType::toString(type).c_str(), eid.toString().c_str());

	return true;
} // addSkillPoints //


NLMISC_COMMAND (addXPToSkill, "Gain experience in a given skills", "<eid> <xp> <skill> [<count>]")
{
	if (args.size () < 3) return false;
	GET_CHARACTER

	uint32 xp = atoi (args[1].c_str());

	string skill = args[2];

	uint count;
	if(args.size()==3)
		count = 1;
	else
		count = atoi(args[3].c_str());

	count = min( count, (uint)100);

	uint i;
	for( i=0; i<count; ++i)
	{
		c->addXpToSkill( (double) xp, skill );
	}
	log.displayNL ("Added %d xp skill '%s' to player %s %d time(s)", xp, skill.c_str(), eid.toString().c_str(),count);

	return true;
}

NLMISC_COMMAND(setSkillsToMaxValue, "Set skills character to max value of each skill","<eid>")
{
	if (args.size () != 1) return false;
	GET_CHARACTER

		c->setSkillsToMaxValue();
	log.displayNL ("You set skills of player %s to max value of each skill", eid.toString().c_str());

	return true;
}

NLMISC_COMMAND(cancelTopPhrase, "cancelTopPhrase","<eid>")
{
	if (args.size () != 1) return false;
	GET_CHARACTER

	CPhraseManager::getInstance().cancelTopPhrase( c->getEntityRowId());

	return true;
}

NLMISC_COMMAND (clearFriendsList, "clear the friend list of a player", "<eid>")
{
	if ( args.size() < 1 ) return false;
	GET_CHARACTER

	if(c != 0)
	{
		c->clearFriendList();
		log.displayNL("player %s friends list cleared",eid.toString().c_str());
	}

	return true;
}


NLMISC_COMMAND (clearIgnoreList, "clear the ignore list of a player", "<eid>")
{
	if ( args.size() < 1 ) return false;
	GET_CHARACTER

	if(c != 0)
	{
		c->clearIgnoreList();
		log.displayNL("player %s ignore list cleared",eid.toString().c_str());
	}

	return true;
}

NLMISC_COMMAND (clearIsFriendOfList, "clear the ignore list of a player", "<eid>")
{
	if ( args.size() < 1 ) return false;
	GET_CHARACTER

	if(c != 0)
	{
		c->clearFriendOfList();
		log.displayNL("player %s 'IsFriendOf' list cleared",eid.toString().c_str());
	}

	return true;
}


//-----------------------------------------------
// set the defense mode to dodge
//-----------------------------------------------
NLMISC_COMMAND(dodge,"set the defense mode to dodge","<player id(id:type:crea:dyn)>")
{
	if ( args.size() >= 1 )
	{
		GET_CHARACTER

		c->dodgeAsDefense(true);

		log.displayNL("player id %s defense mode is now dodge",eid.toString().c_str());

		return true;
	}
	return false;
} // dodge //

//-----------------------------------------------
// set the defense mode to parry
//-----------------------------------------------
NLMISC_COMMAND(parry,"set the defense mode to parry","<player id(id:type:crea:dyn)>")
{
	if ( args.size() >= 1 )
	{
		GET_CHARACTER
		c->dodgeAsDefense(false);
		log.displayNL("player id %s defense mode is now parry",c->getId().toString().c_str());

		return true;
	}
	return false;
} // parry //


//-----------------------------------------------
// reset the ineffective aura and the power flags for given player
//-----------------------------------------------
NLMISC_COMMAND(resetPowerFlags,"reset the ineffective aura and the power flags for given character","<eId>")
{
	if ( args.size() == 1)
	{
		GET_CHARACTER

		c->resetPowerFlags();
		log.displayNL("resetPowerFlags for character %s ", c->getId().toString().c_str());
		return true;
	}
	return false;
} // resetPowerFlags


//-----------------------------------------------
// excute a sabrina phrase
//-----------------------------------------------
NLMISC_COMMAND(execPhrase,"execute a sabrina phrase","<player id(id:type:crea:dyn)> <cyclic 0/1> [<brick ids>...]")
{
	if ( args.size() >= 3 )
	{
		GET_CHARACTER
		bool cyclic = atoi(args[1].c_str()) != 0;

		vector<CSheetId> brickIds;
		for (uint i = 2 ; i < args.size() ; ++i)
		{
			CSheetId sheet(args[i]);
			brickIds.push_back( sheet );
		}

		CPhraseManager::getInstance().executePhrase(c->getEntityRowId(), c->getTargetDataSetRow(), brickIds, cyclic);

		return true;
	}
	return false;
} // execPhrase //

//-----------------------------------------------
// excute a sabrina phrase
//-----------------------------------------------
NLMISC_COMMAND(executeSabrinaPhrase,"execute a sabrina phrase, an sphrase","<player id(id:type:crea:dyn)> <cyclic 0/1> <phraseId>")
{
	if ( args.size() == 3 )
	{
		GET_CHARACTER

		bool cyclic = atoi(args[1].c_str()) != 0;

		CSheetId phraseSheet(args[2]);
		CPhraseManager::getInstance().executePhrase(c->getEntityRowId(), c->getTargetDataSetRow(), phraseSheet, cyclic);

		return true;
	}
	return false;
} // executeSabrinaPhrase //


//-----------------------------------------------
// memorize a sabrina phrase
//-----------------------------------------------
NLMISC_COMMAND(memorizePhrase,"memorize a sabrina phrase","<player id(id:type:crea:dyn)> <index> [<brick ids>...]")
{
/*	if ( args.size() >= 3 )
	{
		CEntityId id;
		id.fromString(args[0].c_str());

		uint8 index = (uint8) atoi(args[1].c_str());

		CCharacter * c = PlayerManager.getChar( id );
		if (c )
		{
			vector<CSheetId> brickIds;
			for (uint i = 2 ; i < args.size() ; ++i)
			{
				CSheetId sheet(args[i]);
				brickIds.push_back( sheet );
			}

			c->memorize(index, brickIds );
		}
		else
		{
			log.displayNL("invalid player id %s",id.toString().c_str());
		}
		return true;
	}
*/
	return false;
} // memorizePhrase //


//-----------------------------------------------
// execute a memorized sabrina phrase
//-----------------------------------------------
NLMISC_COMMAND(execMemorizedPhrase,"memorize a memorized sabrina phrase","<player id(id:type:crea:dyn)> <index> <cyclic 0/1>")
{
/*	if ( args.size() >= 2 )
	{
		CEntityId id;
		id.fromString(args[0].c_str());

		uint8 index = (uint8) atoi(args[1].c_str());
		bool cyclic = (atoi(args[2].c_str()) == 1);

		CCharacter * c = PlayerManager.getChar( id );
		if (c )
		{
			c->executeMemorizedPhrase(index, cyclic,false);
		}
		else
		{
			log.displayNL("invalid player id %s",id.toString().c_str());
		}
		return true;
	}
*/	return false;
} // execMemorizedPhrase //

//////////////////////////////////////////////////////////////////////////////
//             Death commands until client interface is ready               //
//////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------
// Simulate character choice a re-spawn point until UI is ready
//-----------------------------------------------
NLMISC_COMMAND(respawnAfterDeath,"respawnAfterDeath at re-spawn point name, it must be valid (validated by PC and usable)","<player id(id:type:crea:dyn)><Respawn idx>")
{
	if ( args.size() == 2 )
	{
		GET_CHARACTER

		// A death character can choose to re-spawn to a previously validated re-spawn point, a last of one re-spawn point is always valid
		c->respawn( atoi(args[1].c_str() ) );

		return true;
	}
	return false;
} // execMemorizedPhrase //


//-----------------------------------------------
// Simulate Resurrection by other PC until UI is ready
//-----------------------------------------------
NLMISC_COMMAND(resurrected,"Another PC resurrect PC by giving some energy","<player id(id:type:crea:dyn)><Hp gived><Sta gived><Sap gived><Focus gived>")
{
	if( args.size() == 1 )
	{
		GET_CHARACTER
		c->resurrected();

		return true;
	}
	return false;
}


//-----------------------------------------------
// Simulate buy kami / karavan pact until UI is ready
//-----------------------------------------------
NLMISC_COMMAND(buyPact,"Buy Kami or Karavan pact","<player id(id:type:crea:dyn)><Pact Name>")
{
	if( args.size() == 3 )
	{
		GET_CHARACTER

		// Character buy pact with Kami / Karavan by botchat (same has trade)
		// effect is validate re-spawn point for one use
		c->buyPact( args[1] );

		return true;
	}
	return false;
}


//-----------------------------------------------
// Simulate validate re-spawn point until UI is ready
//-----------------------------------------------
NLMISC_COMMAND(validateRespawnPoint,"Validate re-spawn point","<player id(id:type:crea:dyn)><Re-spawn point idx>")
{
	if( args.size() == 2 )
	{
		GET_CHARACTER

		c->getRespawnPoints().addRespawnPoint(atoi( args[1].c_str() ));
		return true;
	}
	return false;
}

NLMISC_COMMAND(displayForageRM,"Display forageable raw materials near by or at the exact position of a player","<eid> <exactPos=1> <extendedInfo=0>")
{
	if ( args.size() < 1 ) return false;
	GET_CHARACTER

	bool exactPos = ! ((args.size() > 1) && (args[1] == "0"));
	bool extendedInfo = (args.size() > 2) && (args[2] == "1");
	CFgProspectionPhrase::displayRMNearBy( c, exactPos, extendedInfo, &log );
	return true;
}


NLMISC_COMMAND(setItemSapLoad,"set an item sap load","<eId><slot index in bag (starts at 0)><float value>")
{
	if ( args.size() != 3)
		return false;

	GET_CHARACTER
	uint slot = atoi(args[1].c_str());
	float value = (float)atof(args[2].c_str());
//	vector<CGameItemPtr>& items = (vector<CGameItemPtr> &)c->getInventory()[INVENTORIES::bag]->getChildren();
	CInventoryPtr invent = c->getInventory(INVENTORIES::bag);
//	if ( slot >= invent->getNumChildren() )
	if ( slot >= invent->getSlotCount() )
	{
//		log.displayNL("Invalid slot %u max = %u",slot, items.size()-1);
		log.displayNL("Invalid slot %u max = %u",slot, invent->getSlotCount()-1);
		return true;
	}
//	if( items[slot] == NULL )
	if( invent->getItem(slot) == NULL )
	{
		log.displayNL("empty slot %u",slot);
		return true;
	}

	uint32 sapLoad = 0;
//	CGameItemPtr item = invent->getChildItem(slot);
	CGameItemPtr item = invent->getItem(slot);
//	if ( items[slot]->getSheetId() == CSheetId("stack.sitem") )
//	if (item->getSheetId() == CSheetId("stack.sitem") )
//	{
////		for ( uint i = 0; i < items[slot]->getChildren().size(); i++ )
//		for ( uint i = 0; i < item->getNumChildren(); i++ )
//		{
////			if ( items[slot]->getChildren()[i] != NULL)
//			if ( item->getChildItem(i) != NULL)
//			{
////				items[slot]->getChildren()[i]->setMaxSapLoad( value );
//				item->getChildItem(i)->setMaxSapLoad( value );
////				sapLoad = items[slot]->getChildren()[i]->maxSapLoad();
//				sapLoad = item->getChildItem(i)->maxSapLoad();
//			}
//		}
//	}
//	else
	{
//		items[slot]->setMaxSapLoad( value );
		item->setMaxSapLoad( value );
//		sapLoad = items[slot]->maxSapLoad();
		sapLoad = item->maxSapLoad();
	}
//trap	c->incSlotVersion( INVENTORIES::bag,slot);
	log.displayNL("item in slot %u has now a sap load of %u",slot,sapLoad);

	return true;
}


NLMISC_COMMAND( showFBT, "show Focus Beta Tester title if the player is a FBT", "<FBT id>" )
{
	if ( args.size() != 1 )
		return false;
	GET_CHARACTER
	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( c->getId() ) );
	if (p == NULL)
	{
		nlwarning ("ADMIN: Can't find player with UserId %d", PlayerManager.getPlayerId(c->getId()));
		return false;
	}

	if (!p->isBetaTester())
	{
		nlwarning ("ADMIN: UserId %d is not a FBT", PlayerManager.getPlayerId(c->getId()));
		return false;
	}

	c->setTitle(CHARACTER_TITLE::FBT);
	c->registerName();
	return true;
}

NLMISC_COMMAND( summonPet, "player can summon it's pet one time only", "<eid><Pet Number>" )
{
	if ( args.size() != 2 )
		return false;
	GET_CHARACTER

	uint32 index = atoi(args[1].c_str()) - 1;
	const std::vector< CPetAnimal >& pet = c->getPlayerPets();
	if( index < pet.size() )
	{
		if( pet[ index ].PetStatus != CPetAnimal::not_present )
		{
			if( pet[ index ].IsTpAllowed )
			{
				c->sendPetCommand( CPetCommandMsg::DESPAWN, index, true );
				c->setPetStatus( index, CPetAnimal::waiting_spawn );
				c->setSpawnPetFlag( index );
				c->petTpAllowed( index, false );
			}
		}
	}
	return true;
}

NLMISC_COMMAND( allowSummonPet, "autorize player summon it's pet one time only", "<eid><Pet Number>" )
{
	if ( args.size() != 2 )
		return false;
	GET_CHARACTER

	uint32 index = atoi(args[1].c_str()) - 1;

	const std::vector< CPetAnimal >& pet = c->getPlayerPets();
	if( index < pet.size() )
	{
		if( pet[ index ].PetStatus != CPetAnimal::not_present )
		{
			c->petTpAllowed( index, true );
		}
	}
	return true;
}


//
//
// You can add here all variables to administrate this service.
//
//


NLMISC_DYNVARIABLE (float, RyzomTime, "Current ryzom time")
{
	if (get)
	{
		*pointer = CTimeDateSeasonManager::getRyzomTimeReference().getRyzomTime ();
	}
/*	else
	{
		CMessage msgout ("SET_RYZOM_TIME");
		msgout.serial (*pointer);
		CUnifiedNetwork::getInstance()->send( "WOS", msgout );
	}
*/
}


NLMISC_DYNVARIABLE (uint32, RyzomDate, "Current ryzom date")
{
	if (get)
	{
		*pointer = CTimeDateSeasonManager::getRyzomTimeReference().getRyzomDay ();
	}
/*	else
	{
		CMessage msgout ("SET_RYZOM_DAY");
		msgout.serial (*pointer);
		CUnifiedNetwork::getInstance()->send( "WOS", msgout );
	}
*/
}


//
//
//
//

// all admin /a /b commands executed by the client go in this callback
void cbClientAdmin (NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientAdmin);


	CEntityId eid;
	msgin.serial (eid);

	bool onTarget;
	msgin.serial (onTarget);

	CSString cmdName, arg;
	msgin.serial (cmdName, arg);

	nlinfo("ADMIN: Executing admin command: eid=%s onTarget=%s cmdName=%s arg=%s",eid.toString().c_str(),onTarget?"true":"false",cmdName.quote().c_str(),arg.quote().c_str());
	TLogContext_Command_ExecCtx logContext(eid);

	// find the character
	CCharacter *c = PlayerManager.getChar( eid );
	if (c == 0)
	{
		nlwarning ("ADMIN: Unknown player %s", eid.toString().c_str());
		chatToPlayer (eid, "Unknown player");
		return;
	}

	// test character is ready and valid
	if (!c->getEnterFlag())
	{
		nlwarning("ADMIN: player %s not ready", eid.toString().c_str());
		return;
	}
	if (!TheDataset.isAccessible(c->getEntityRowId()))
	{
		nlwarning("ADMIN: player %s not ready in mirror (invalid rowid)", eid.toString().c_str());
		return;
	}

	// find if the command is available
	const CAdminCommand * cmd = findAdminCommand(cmdName);
	if (!cmd)
	{
		nlinfo("ADMIN: Player %s tried to execute a no valid client admin command '%s'", eid.toString().c_str(), cmdName.c_str());
		chatToPlayer (eid, "Unknown command");
		return;
	}

	if (!c->havePriv(cmd->Priv))
	{
		nlinfo("ADMIN: Player %s doesn't have privilege to execute the client admin command '%s' ", eid.toString().c_str(), cmdName.c_str());
		chatToPlayer (eid, "You don't have privilege to execute this command");
		return;
	}

	if (!cmd->ForwardToservice.empty())
	{
		// we need to forward the command to another service
		if (IClientCommandForwader::getInstance())
		{
			IClientCommandForwader::getInstance()->sendCommand(cmd->ForwardToservice, cmdName, eid, onTarget, onTarget ? c->getTarget() : CEntityId::Unknown, arg);
		}
	}
	else
	{
		// execute locally
		// create the command line
		string res = cmdName + " ";

		if( cmdName == string("Position") )
		{
			// check validity of Position command
			if( onTarget )
			{
				if( !c->havePriv(":DEV:SGM:GM:EM:") )
				{
					nlwarning ("ADMIN: Player %s doesn't have privilege to execute the client admin command /b '%s' ", eid.toString().c_str(), cmdName.c_str());
					chatToPlayer (eid, "You don't have privilege to execute this command");
					return;
				}
			}
		}

		std::string targetName = string("implicite");

		// add the eid of the player or target if necessary
		if (cmd->AddEId)
		{
			if(onTarget)
			{
				log_Command_ExecOnTarget(c->getTarget(), cmdName, arg);
				res += c->getTarget().toString();
				targetName = NLMISC::toString("(%s,%s)", c->getTarget().toString().c_str(), CEntityIdTranslator::getInstance()->getByEntity(c->getTarget()).toString().c_str());
			}
			else
			{
				log_Command_Exec(cmdName, arg);
				res += eid.toString();
				targetName = string("Himself");
			}
			res += " ";
		}

		res += arg;

		TLogContext_Item_Command itemCtx(onTarget ? c->getTarget() : eid);
		TLogContext_Character_BuyRolemasterPhrase characterCtx(onTarget ? c->getTarget() : eid);
		std::string csName = CEntityIdTranslator::getInstance()->getByEntity(eid).toString();

		nlinfo ("ADMIN: Player (%s,%s) will execute client admin command '%s' on target %s", eid.toString().c_str(), csName.c_str(), res.c_str(), targetName.c_str());

		CLightMemDisplayer *CmdDisplayer = new CLightMemDisplayer("CmdDisplayer");
		CLog *CmdLogger = new CLog( CLog::LOG_NO );
		CmdLogger->addDisplayer( CmdDisplayer );
		NLMISC::ICommand::execute(res, *CmdLogger, true);
		const std::deque<std::string>	&strs = CmdDisplayer->lockStrings ();
		for (uint i = 0; i < strs.size(); i++)
		{
			InfoLog->displayNL(trim(strs[i]).c_str());

			SM_STATIC_PARAMS_1(params,STRING_MANAGER::literal);
			params[0].Literal = trim(strs[i]);
			CCharacter::sendDynamicSystemMessage( eid, "LITERAL", params );
		}
		CmdDisplayer->unlockStrings();
		CmdLogger->removeDisplayer (CmdDisplayer);
		delete CmdDisplayer;
		delete CmdLogger;
	}
}

// all admin /c commands executed by the client go in this callback
void cbClientAdminOffline (NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientAdminOffline);

	CEntityId eid;
	msgin.serial (eid);

	string characterName;
	msgin.serial(characterName);

	string cmdName, arg;
	msgin.serial (cmdName, arg);

	nlinfo("ADMIN: Executing admin /c command: eid=%s onTarget=%s cmdName=%s arg=%s",eid.toString().c_str(),characterName.c_str(),cmdName.c_str(),arg.c_str());

	// find the character
	CCharacter *c = PlayerManager.getChar( eid );
	if (c == 0)
	{
		nlwarning ("ADMIN: Unknown player %s", eid.toString().c_str());
		chatToPlayer (eid, "Unknown player");
		return;
	}

	// test character is ready and valid
	if (!c->getEnterFlag())
	{
		nlwarning("ADMIN: player %s not ready", eid.toString().c_str());
		return;
	}
	if (!TheDataset.isAccessible(c->getEntityRowId()))
	{
		nlwarning("ADMIN: player %s not ready in mirror (invalid rowid)", eid.toString().c_str());
		return;
	}

	// find if the command is available
	const CAdminCommand * cmd = findAdminCommand(cmdName);
	if (!cmd)
	{
		nlwarning ("ADMIN: Player %s tried to execute a no valid client admin command '%s'", eid.toString().c_str(), cmdName.c_str());
		chatToPlayer (eid, "Unknown command");
		return;
	}

	if (!c->havePriv(cmd->Priv))
	{
		nlwarning ("ADMIN: Player %s doesn't have privilege to execute the client admin command '%s' ", eid.toString().c_str(), cmdName.c_str());
		chatToPlayer (eid, "You don't have privilege to execute this command");
		return;
	}

	// find the character eid
	CEntityId charEid = CEntityIdTranslator::getInstance()->getByEntity(characterName);
	if (charEid == CEntityId::Unknown)
	{
		nlwarning ("ADMIN: Unknown character %s", characterName.c_str());
		return;
	}

	if( cmdName == string("Position") )
	{
		// check validity of Position command
		if( !c->havePriv(":DEV:SGM:GM:") )
		{
			nlwarning ("ADMIN: Player %s doesn't have privilege to execute the client admin command /c '%s' ", eid.toString().c_str(), cmdName.c_str());
			chatToPlayer (eid, "You don't have privilege to execute this command");
			return;
		}
	}

	// create the command line
	string res = cmdName + " ";

	// add the eid of the player or target if necessary
	if (cmd->AddEId)
	{
		res += charEid.toString();
		res += " ";
	}

	res += arg;

	std::string csName = CEntityIdTranslator::getInstance()->getByEntity(eid).toString();
	std::string targetName = NLMISC::toString("(%s,%s)", CEntityIdTranslator::getInstance()->getByEntity( ucstring(characterName) ).toString().c_str(), characterName.c_str() );

	nlinfo("ADMINOFFLINE: Player (%s,%s) will execute client admin command '%s' on target %s", eid.toString().c_str(), csName.c_str(), res.c_str(), targetName.c_str());
	NLMISC::ICommand::execute(res, *InfoLog);
}

void cbRemoteClientCallback (uint32 rid, const std::string &cmd, const std::string &entityNames)
{
	vector <CEntityId> entities;
	selectEntities	(entityNames, entities);

	for (uint i = 0; i < entities.size(); i++)
	{
		CCharacter *c = PlayerManager.getChar(entities[i]);
		if (c != 0)
		{
			nlinfo ("ADMIN: I have to send a request for admin to a client : rid %d cmd '%s' eid %s", rid, cmd.c_str(), entities[i].toString().c_str());

			CMessage msgout ("IMPULSION_ID");
			msgout.serial (entities[i]);
			CBitMemStream bms;

			if (!GenericMsgManager.pushNameToStream ("COMMAND:REMOTE_ADMIN", bms))
				nlstopex (("Missing a message in the msg.xml"));

			bms.serial (rid);
			bms.serial (const_cast<string&>(cmd));

			msgout.serialBufferWithSize ((uint8*)bms.buffer(), bms.length());
			sendMessageViaMirror (NLNET::TServiceId(entities[i].getDynamicId()), msgout);
		}
	}
}

void cbClientRemoteAdmin( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientRemoteAdmin);

	CEntityId eid;
	msgin.serial (eid);

	uint32 rid;
	msgin.serial (rid);

	string cmd;
	msgin.serial (cmd);

	string answer;
	msgin.serial (answer);

	vector<string> vara, vala;

	if (ICommand::isCommand(cmd))
	{
		vara.push_back ("__log");
		vala.push_back ("----- Result from "+eid.toString()+" "+IService::getInstance()->getServiceUnifiedName()+" of command '"+cmd+"'\n");
		vala.push_back (answer);
	}
	else
	{
		vara.push_back ("service");
		vala.push_back (IService::getInstance ()->getServiceUnifiedName());

		vara.push_back ("entity");
		vala.push_back (eid.toString());

		vara.push_back (cmd);
		vala.push_back (answer);
	}

	nlinfo ("ADMIN: received an answer from %s for command '%s' that is '%s' for rid %d", eid.toString().c_str(), cmd.c_str(), answer.c_str(), rid);

	addRequestAnswer (rid, vara, vala);
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( displayWeather, "Display current weather at specified pos", "<x> <y>" )
{
	if ( args.size() < 2 )
		return false;
	CVector pos;
	pos.x = (float)atof( args[0].c_str() );
	pos.y = (float)atof( args[1].c_str() );
	pos.z = 0;
	CRyzomTime::EWeather weather = WeatherEverywhere.getWeather( pos, CTimeDateSeasonManager::getRyzomTimeReference() );
	log.displayNL( "Weather: %u", (uint)weather );

	return true;
}



/***********************************************************************************************************************

		BELOW ARE OFFICIAL CSR COMMANDS

 ***********************************************************************************************************************/

NLMISC_COMMAND( csrCmd, "Invoke a CSR command from service console or admin tool without the need to have a CSR logged in", "<csrCommandName> <command params>+" )
{
	if (args.empty())
		return false;

	// build the command line
	string command = args[0];
	command += string(" (0x0000000000:00:00:00) ");
	for (uint i=1; i<args.size(); ++i)
	{
		command += "\""+args[i]+"\"";
		if (i<args.size()-1)
			command += " ";
	}

	ICommand::execute(command, log, quiet, human);

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND( motd, "set the current message of the day", "<message to be displayed (string)>" )
{
	if ( args.empty() )
		return false;

	string msg = args[0];
	for ( uint i = 1; i < args.size(); i++ )
	{
		msg+= " " + args[i];
	}
	MessageOfTheDay = msg;
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( summon, "summon a player in front of the CSR", "<CSR eId><player name>" )
{
	if ( args.size() !=2 )
		return false;
	GET_CHARACTER
	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_PENDING_CHARACTER_LOG" );

		COfflineEntityState state;
		state.X = c->getState().X;
		state.Y = c->getState().Y;
		state.Z = c->getState().Z;
		state.Heading = c->getState().Heading;
		CGmTpPendingCommand::getInstance()->addTpPendingforCharacter( args[1], state );

		return true;
	}
	CHECK_RIGHT( c,target );

	if(target->getCurrentContinent() == CONTINENT::NEWBIELAND )
	{
		if( c->getCurrentContinent() != CONTINENT::NEWBIELAND )
		{
			log.displayNL( "Summon player outside NEWBIELAND is forbiden, <CSR eId %s> try to made an illegal action, this is loged.", eid.toString().c_str());
			chatToPlayer (eid, "You don't have privilege to execute this command");
			return true;
		}
	}

	COfflineEntityState state;
	state.X = target->getState().X;
	state.Y = target->getState().Y;
	state.Z = target->getState().Z;
	state.Heading = target->getState().Heading;
	PlayerManager.addSummonnedUser( target->getEntityRowId(), state  );

	state.X = c->getState().X + sint32 (cos (c->getState().Heading) * 2000);
	state.Y = c->getState().Y + sint32 (sin (c->getState().Heading) * 2000);
	state.Z = c->getState().Z;
	state.Heading = c->getState().Heading + (float)NLMISC::Pi;
	if ( state.Heading >= (float)NLMISC::Pi )
		state.Heading -= 2* (float)NLMISC::Pi;

	target->allowNearPetTp();
	target->tpWanted( state.X, state.Y, state.Z, true, state.Heading );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( dismiss, "teleport a player back to its former position ( before last call to summonPlayer )", "<CSR eId><player name>" )
{
	if ( args.size() !=2 )
		return false;
	TRY_GET_CHARACTER
	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}
	CHECK_RIGHT( c,target );
	COfflineEntityState state;
	if ( PlayerManager.getDismissCoords( target->getEntityRowId(), state ) )
	{
		target->allowNearPetTp();
		target->tpWanted( state.X, state.Y, state.Z, true, state.Heading );
		CContinent * cont = CZoneManager::getInstance().getContinent(state.X, state.Y);
		if ( cont )
		{
			target->getRespawnPoints().addDefaultRespawnPoint( CONTINENT::TContinent(cont->getId()) );
		}
		PlayerManager.removeSummonedUser( target->getEntityRowId() );
	}
	else
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_NOT_SUMMONED" );
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( teleport, "teleport the CSR in front of a player", "<CSR eId><player name>" )
{
	if ( args.size() !=2 )
		return false;
	GET_CHARACTER
	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}
	float heading = target->getState().Heading;
	sint32 x = target->getState().X + sint32 (cos (target->getState ().Heading) * 2000);
	sint32 y = target->getState().Y + sint32 (sin (target->getState ().Heading) * 2000);
	sint32 z = target->getState().Z;
	heading += (float)NLMISC::Pi;
	if ( heading >= (float)NLMISC::Pi )
		heading -= 2* (float)NLMISC::Pi;

	c->allowNearPetTp();
	c->teleportCharacter(x, y, z, true, true, heading);
	CContinent * continent = CZoneManager::getInstance().getContinent(x, y);
	if (continent != NULL)
	{
		c->getRespawnPoints().addDefaultRespawnPoint( CONTINENT::TContinent(continent->getId()) );
	}
	return true;
}

//----------------------------------------------------------------------------
//static string capitalize(const string & s)
//{
//	if ( s.empty() )
//		return s;
//
//	return toUpper( s.substr(0,1) ) + toLower( s.substr(1,string::npos) );
//}

//----------------------------------------------------------------------------
NLMISC_COMMAND( renamePlayerForEvent, "rename a player for the event", "<CSR eId><player name><new playerName>" )
{
	if ( args.size() != 3 )
		return false;

	ucstring newName( args[2] );
	TRY_GET_CHARACTER
	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}
	CHECK_RIGHT( c,target );

	// assign new name to player

//	PlayerManager.addEntityForStringIdRequest( target->getId() );
	target->registerName(newName);

	return true;
}

//----------------------------------------------------------------------------
//NLMISC_COMMAND( renamePlayer, "rename a player", "<CSR eId><player name><new playerName>" )
//{
//	if ( args.size() != 3 )
//		return false;
//
//	ucstring oldName( capitalize(args[1]) );
//	ucstring newName( capitalize(args[2]) );
//	TRY_GET_CHARACTER
//	CCharacter * target = PlayerManager.getCharacterByName( args[1] );
//	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
//	{
//		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
//		return true;
//	}
//	CHECK_RIGHT( c,target );
//
//	if ( CEntityIdTranslator::getInstance()->entityNameExists( newName ) )
//	{
//		CCharacter::sendDynamicSystemMessage( eid, "CSR_NAME_EXISTS" );
//		return true;
//	}
//
//	// assign new name to player
//	bool result = CNameManager::getInstance()->assignName( target->getId(), newName );
//	if ( !result )
//	{
//		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_NAME" );
//		nlwarning("assignName failed for name: %s", newName.toUtf8().c_str() );
//		return true;
//	}
//	PlayerManager.addEntityForStringIdRequest( target->getId() );
//	target->setName( newName );
//	target->registerName();
//
//	if ( toLower(newName) != toLower(oldName) )
//	{
//		// liberate old name
//		CNameManager::getInstance()->liberateName( target->getId(), oldName );
//	}
//
//	return true;
//}

//----------------------------------------------------------------------------
NLMISC_COMMAND( renameGuild, "rename a guild", "<CSR_eId> <guild_name>|<shardId:guildId> <new_guild_name>" )
{
	if ( args.size() != 3 )
		return false;
	if (args[0]!="admin_tool")
	{
		TRY_GET_CHARACTER
	}
	GET_GUILD(1, true, NLMISC::CEntityId::Unknown);

	ucstring name( args[2] );
	// check if name already exists in the player list
	if ( NLMISC::CEntityIdTranslator::getInstance()->entityNameExists( name ) /*|| EGSPD::PDSLib.getStringManager().stringExists(name, RYZOMID::guildName)*/ )
	{
		if (args[0]=="admin_tool")
		{
			log.displayNL("renameGuild failed because the name is already taken: ",args[2].c_str());
		}
		else
		{
			TRY_GET_CHARACTER
			CCharacter::sendDynamicSystemMessage( eid,"CSR_NAME_EXISTS" );
		}
		return false;
	}

	guild->setName( name );

	if (IGuildUnifier::getInstance() != NULL)
		IGuildUnifier::getInstance()->broadcastGuildUpdate(guild);
//	CGuildManager::getInstance()->addGuildsAwaitingString(name,guild->getId());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( setGuildDescription, "set a guild description", "<guild_name>|<shardId:guildId> <new_guild_description>" )
{
	if ( args.size() != 2)
		return false;
	GET_GUILD(0, true, NLMISC::CEntityId::Unknown);
	ucstring desc( args[1] );
	guild->setDescription( desc );

	if (IGuildUnifier::getInstance() != NULL)
		IGuildUnifier::getInstance()->broadcastGuildUpdate(guild);

//	CGuildManager::getInstance()->addGuildsAwaitingString(desc,guild->getId());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( setGuildIcon, "set a guild icon", "<guild_name>|<shardId:guildId> <new_icon_code(on 64 bits)>" )
{
	if ( args.size() != 2)
		return false;
	GET_GUILD(0, true, NLMISC::CEntityId::Unknown);

	uint64 icon = atoiInt64( args[1].c_str() );
	guild->setIcon( icon );

	if (IGuildUnifier::getInstance() != NULL)
		IGuildUnifier::getInstance()->broadcastGuildUpdate(guild);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( killMob, "kill a mob ( /a killMob )", "<CSR eId>" )
{
	if ( args.size() != 1 )
		return false;
	TRY_GET_CHARACTER

	CCreature * creature = CreatureManager.getCreature( c->getTarget() );
	if( creature == 0 )
	{
		nlwarning ("Unknown creature '%s'", c->getTarget().toString().c_str() );
		return false;
	}
	if(!TheDataset.isAccessible(creature->getEntityRowId()))
	{
		nlwarning ("'%s' is not valid in mirror", c->getTarget().toString().c_str());
		return false;
	}
	if ( !creature->getContextualProperty().directAccessForStructMembers().attackable() )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_NOT_ATTACKABLE" );
		return true;
	}
	creature->getScores()._PhysicalScores[SCORES::hit_points].Current = 0;
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND( dssTarget, "target a mob and send information to dss( /b dssTarget )", "<ring player eId>[<cmd>]" )
{
	if (  args.size() < 1 )
		return false;

	TRY_GET_CHARACTER

	CCreature * creature = CreatureManager.getCreature( c->getTarget() );
	if( creature == 0 )
	{
		nlwarning ("Unknown creature '%s'", c->getTarget().toString().c_str() );
		return false;
	}
	if(!TheDataset.isAccessible(creature->getEntityRowId()))
	{
		nlwarning ("'%s' is not valid in mirror", c->getTarget().toString().c_str());
		return false;
	}


	NLMISC::CEntityId creatureId = creature->getId();
	TAIAlias alias = CAIAliasTranslator::getInstance()->getAIAlias(creatureId);
	TDataSetRow entityRowId = creature->getEntityRowId();

	if (alias == CAIAliasTranslator::Invalid)
	{
		nlwarning ("'%s' has no alias translation", creatureId.toString().c_str());
		return false;
	}
	CMessage msgout("DSS_TARGET");
	msgout.serial(eid); //eid of the player
	msgout.serial(creatureId); // eid of the mob
	msgout.serial(alias); //mob targeted
	msgout.serial(entityRowId); //datasetrow

	uint32 args_size = (uint32)args.size() - 1;
	uint32 i = 0;
	msgout.serial(args_size);
	for ( ; i != args_size ; ++i)
	{
		std::string str = args[1+i];
		msgout.serial(str);
	}

	CUnifiedNetwork::getInstance()->send("DSS", msgout);


	return true;
}




//----------------------------------------------------------------------------
NLMISC_COMMAND( root, "root a player", "<CSR id><player name><time in second>" )
{
	if ( args.size() != 3 )
		return false;
	TRY_GET_CHARACTER
	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}
	CHECK_RIGHT( c,target );

	NLMISC::TGameCycle cycle = (NLMISC::TGameCycle) ( atoi(args[2].c_str()) / CTickEventHandler::getGameTimeStep() + CTickEventHandler::getGameCycle() );
	PlayerManager.addGMRoot( eid, target->getId(), cycle );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( unroot, "stop rooting a player", "<CSR id> <player name>" )
{
	if ( args.size() != 2 )
		return false;
	TRY_GET_CHARACTER
	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}
	CHECK_RIGHT( c,target );
	PlayerManager.removeGMRoot(eid , target->getId() );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( ignoreTells, "ignore incoming tell", "<CSR id> <0 / false / 1 / true >" )
{
	if ( args.size() != 2 )
		return false;
	GET_CHARACTER

	CMessage msgout("IGNORE_TELL_MODE");
	msgout.serial(eid);
	bool ignore = (args[1]=="1" || strlwr(args[1])=="on" || strlwr(args[1])=="true" );
	msgout.serial( ignore );
	CUnifiedNetwork::getInstance()->send("IOS", msgout);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( showCSR, "activate GM title", "<CSR id>" )
{
	if ( args.size() != 1 )
		return false;
	GET_CHARACTER
	CPlayer *p = PlayerManager.getPlayer(PlayerManager.getPlayerId( eid ) );
	if (p == NULL)
	{
		nlwarning ("ADMIN: Can't find player with UserId %d", PlayerManager.getPlayerId(eid));
		return false;
	}
	CHARACTER_TITLE::ECharacterTitle title = CHARACTER_TITLE::getGMTitleFromPriv( p->getUserPriv() );
	if ( title == CHARACTER_TITLE::NB_CHARACTER_TITLE )
	{
		nlwarning ("ADMIN: UserId %d has an invalid privilege %s", PlayerManager.getPlayerId(eid), p->getUserPriv().c_str() );
		return false;
	}
	c->setTitle( title );
	c->registerName();
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( monitorMissions, "monitor a player missions", "<CSR id><player name>" )
{
	if ( args.size() != 2 )
		return false;
	GET_CHARACTER;
	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}
	CHECK_RIGHT( c,target );

	// CSR must have no missions to monitor a player
	if ( c->getMissionsBegin() ==  c->getMissionsEnd() )
	{
		CCharacter::sendDynamicSystemMessage( c->getEntityRowId() , "CSR_HAS_MISSION" );
		return true;
	}

	// target missions must not be monitored
	if ( target->getMonitoringCSR() != TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW ) )
	{
		CCharacter::sendDynamicSystemMessage( c->getEntityRowId() , "CSR_MISSION_MONITORED" );
		return true;
	}
	target->setMonitoringCSR( c->getEntityRowId() );
	c->getAdminProperties().setMissionMonitoredUser( target->getEntityRowId() );

	for ( map<TAIAlias, CMission*>::iterator it = target->getMissionsBegin(); it != target->getMissionsEnd(); ++it )
	{
		(*it).second->updateUsersJournalEntry();
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( stopMonitorMissions, "monitor a player missions", "<CSR id>" )
{
	if ( args.size() != 1 )
		return false;
	GET_CHARACTER;

	CCharacter * target = PlayerManager.getChar( c->getAdminProperties().getMissionMonitoredUser() );
	c->getAdminProperties().setMissionMonitoredUser( TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW) );
	if ( target )
		target->setMonitoringCSR( TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW ) );

	/// todo mission
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( failMission, "force mission failure", "<CSR id><mission idx>" )
{
	if ( args.size() != 2 )
		return false;
	GET_CHARACTER;
	CMission * mission = c->getAdminProperties().getMission( atoi(args[1].c_str() ) );
	if ( !mission )
	{
		CCharacter::sendDynamicSystemMessage( c->getEntityRowId(), "CSR_BAD_MISSION" );
		return true;
	}
	mission->onFailure(true);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND( progressMission, "force mission progression", "<CSR id><mission idx>[repeat]")
{
	if ( args.size() != 2  || args.size() != 3 )
		return false;
	GET_CHARACTER;
	CMission * mission = c->getAdminProperties().getMission( atoi(args[1].c_str() ) );
	if ( !mission )
	{
		CCharacter::sendDynamicSystemMessage( c->getEntityRowId(), "CSR_BAD_MISSION" );
		return true;
	}
	CCharacter * user = mission->getMainEntity();
	uint repeat = 1;
	if ( args.size() == 3 )
		repeat = (uint)atoi( args[2].c_str() );
	CMissionEventDebug event;
	for ( uint i = 0; i < repeat; i++ )
		user->processMissionEvent(event);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (changeVar, "change a variable of a player", "<eid> <var> <val>")
{
	if (args.size() != 3)
		return false;

	CEntityId eid (args[0]);

	string var = args[1];
	string value = args[2];

	CCharacter *e = PlayerManager.getChar(eid);
	if(e != 0)
	{
		if (e->setValue (var, value))
			log.displayNL("Change Var of Entity %s Var %s Value %s", eid.toString().c_str(), var.c_str(), value.c_str());
	}
	else
	{
		log.displayNL("Unknown entity %s (not a player or a creature)", eid.toString().c_str());
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (mute, "mute a user", "<csr id> <player name><duration>")
{
	if ( args.size() != 3 )
		return false;
	GET_CHARACTER;
	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}
	CHECK_RIGHT( c,target );
	NLMISC::TGameCycle cycle = (NLMISC::TGameCycle) ( atoi(args[2].c_str()) / CTickEventHandler::getGameTimeStep() + CTickEventHandler::getGameCycle() );
	PlayerManager.addGMMute( eid, target->getId(), cycle );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (unmute, "unmute a user", "<csr id> <player name>")
{
	if ( args.size() != 2 )
		return false;
	GET_CHARACTER;
	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}

	PlayerManager.removeGMMute( eid, target->getId() );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (universe, "chat in universe mode", "<boolean>")
{
	if ( args.size() != 2 )
		return false;
	GET_CHARACTER;
	bool on = (args[1]=="1" || strlwr(args[1])=="on" || strlwr(args[1])=="true" );

	CMessage msgOut("UNIVERSE_MODE");

	msgOut.serial( const_cast<CEntityId&>(eid) );
	msgOut.serial( on );
	CUnifiedNetwork::getInstance()->send( "IOS", msgOut );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (muteUniverse, "mute the univers chat", "<csr id><player name><duration>")
{
	if ( args.size() != 3 )
		return false;
	GET_CHARACTER;

	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}

	// find if the command is available
	const CAdminCommand * cmd = findAdminCommand("muteUniverse");
	if (!cmd)
	{
		nlwarning ("ADMIN: Player %s tried to execute a no valid client admin command '%s'", c->getId().toString().c_str(), "muteUniverse");
		chatToPlayer (c->getId(), "Unknown command");
		return true;
	}

	if (!c->havePriv(cmd->Priv))
	{
		nlwarning ("ADMIN: Player %s doesn't have privilege to execute the client admin command '%s' ", c->getId().toString().c_str(), "muteUniverse");
		chatToPlayer (c->getId(), "You don't have privilege to execute this command");
		return true;
	}

	NLMISC::TGameCycle cycle = (NLMISC::TGameCycle) ( atoi(args[2].c_str()) / CTickEventHandler::getGameTimeStep() + CTickEventHandler::getGameCycle() );
	PlayerManager.muteUniverse( eid, cycle, target->getId() );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (unmuteUniverse, "unmute the univers chat", "<csr id><player name>")
{
	if ( args.size() != 2 )
		return false;
	GET_CHARACTER;

	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1]) );
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}

	// find if the command is available
	const CAdminCommand * cmd = findAdminCommand("unmuteUniverse");
	if (!cmd)
	{
		nlwarning ("ADMIN: Player %s tried to execute a no valid client admin command '%s'", c->getId().toString().c_str(), "unmuteUniverse");
		chatToPlayer (c->getId(), "Unknown command");
		return true;
	}

	if (!c->havePriv(cmd->Priv))
	{
		nlwarning ("ADMIN: Player %s doesn't have privilege to execute the client admin command '%s' ", c->getId().toString().c_str(), "unmuteUniverse");
		chatToPlayer (c->getId(), "You don't have privilege to execute this command");
		return true;
	}

	PlayerManager.unmuteUniverse(eid, target->getId());
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (setGMGuild, "set the current GM guild", "")
{
	if ( args.size() != 1 )
		return false;
	GET_CHARACTER;
	uint32 guildId = c->getGuildId();
	CGuildManager::getInstance()->setGMGuild( guildId );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (targetInfos, "give infos on the target", "")
{
	if ( args.size() != 1 )
		return false;
	GET_CHARACTER;

	string answer = "displaying info on target : \n";
	if ( c->getTarget().getType() == RYZOMID::creature || c->getTarget().getType() == RYZOMID::npc )
	{
		CCreature * target = CreatureManager.getCreature( c->getTarget() );
		if ( !target )
			answer = "invalid creature target";
		else
		{
			answer = NLMISC::toString( "HP : %u / %u",
				target->getScores()._PhysicalScores[SCORES::hit_points].Current(),
				target->getScores()._PhysicalScores[SCORES::hit_points].Max() );
		}
	}
	else if ( c->getTarget().getType() == RYZOMID::player )
	{
		CCharacter * target = PlayerManager.getChar( c->getTarget() );
		if ( !target )
			answer += "invalid player target";
		else
		{
			const std::vector< SCharacteristicsAndScores > & scores = target->getScores()._PhysicalScores;
			answer = NLMISC::toString( "HP :	%d / %d \nSAP :	%d / %d \nSTA :	%d / %d \nFOCUS :	%d / %d \n",
										scores[SCORES::hit_points].Current(),scores[SCORES::hit_points].Max(),
										scores[SCORES::sap].Current(),scores[SCORES::sap].Max(),
										scores[SCORES::stamina].Current(),scores[SCORES::stamina].Max(),
										scores[SCORES::focus].Current(),scores[SCORES::focus].Max() );
			answer += "\n Displaying skills > 1:\n";
			for ( uint i = 0; i < target->getSkills()._Skills.size();i++ )
			{
				sint32 base = target->getSkillBaseValue( ( SKILLS::ESkills ) i );
				sint32 current = target->getSkillValue( ( SKILLS::ESkills ) i );
				if ( base > 1 || current > 1 )
				{
					answer+= NLMISC::toString( "%s: %d / %d\n", SKILLS::toString( i ).c_str(), current,base );
				}

			}
		}
	}
	else
		answer += "invalid target type";

	SM_STATIC_PARAMS_1(params,STRING_MANAGER::literal);
	params[0].Literal = answer;

	CCharacter::sendDynamicSystemMessage( eid,"LITERAL", params );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND (infos, "give info on character (GodMode, Invisible...)", "")
{
	CSString str("GM STATUS: ");
	GET_CHARACTER
	if( c->invulnerableMode() )
	{
		str << "INVULNERABLE MODE ";
	}
	if( c->godMode() )
	{
		str << "GOD MODE ";
	}
	else
	{
		if( c->invulnerableMode() == false )
			str << "VULNERABLE ";
	}

	if( R2_VISION::isEntityVisibleToPlayers(c->getWhoSeesMe()) )
	{
		str << "VISIBLE ";
	}
	else
	{
		if (IsRingShard)
		{
			str << "INVISIBLE(" <<R2_VISION::extractInvisibilityLevel(c->getWhoSeesMe()) << ")";
		}
		else
		{
			str << "INVISIBLE";
		}
	}

	if ( IsRingShard && R2_VISION::extractVisionLevel(c->getWhoSeesMe())!=R2_VISION::VISIBLE )
	{
		str << "SEEINVIS(" << R2_VISION::extractVisionLevel(c->getWhoSeesMe()) << ")";
	}

	if (c->getAggroableSave())
	{
		str << " AGGROABLE";
	}
	else
	{
		str << " NOT_AGGROABLE";
	}

	SM_STATIC_PARAMS_1(params,STRING_MANAGER::literal);
	params[0].Literal = str;
	CCharacter::sendDynamicSystemMessage( eid,"LITERAL", params );
	return true;
}

//----------------------------------------------------------------------------
//NLMISC_COMMAND(addGuildXp,"add xp of a guild","<guild_name>|<shardId:guildId> <xp ( positive or negative )>")
//{
//	if ( args.size() != 2 )
//		return false;
//
//	GET_GUILD(0, true, NLMISC::CEntityId::Unknown);
//	sint xp = atoi( args[1].c_str() );
//	if ( xp < 0 )
//	{
//		uint32 uXP = (uint32)(-xp);
//		if ( uXP > guild->getXP() )
//			guild->spendXP( guild->getXP() );
//		else
//			guild->spendXP( uXP );
//	}
//	else
//	{
//		uint32 uXP = (uint32)xp;
//		guild->addXP ( uXP );
//	}
//	log.displayNL( "added %d xp. Current xp is %u", xp, guild->getXP()  );
//	return true;
//}

//----------------------------------------------------------------------------
//NLMISC_COMMAND(setGuildChargePoint,"set the charge points of a guild","<guild_name>|<shardId:guildId> <points>")
//{
//	if ( args.size() != 2 )
//		return false;
//	GET_GUILD(0, true, NLMISC::CEntityId::Unknown);
//	uint32 points = uint32(atoi( args[1].c_str() ));
//	guild->clearChargePoints();
//	guild->addChargePoints( points );
//	log.displayNL( "set charge points to %u", points );
//	return true;
//}

//----------------------------------------------------------------------------
ENTITY_VARIABLE(Invisible, "Invisibility of a player")
{
	ENTITY_GET_ENTITY

	if (get)
	{
		value = R2_VISION::isEntityVisibleToPlayers(e->getWhoSeesMe()) ?"0":"1";
	}
	else
	{
		TDataSetRow userRow = TheDataset.getDataSetRow( e->getId() );
		if ( !TheDataset.isAccessible( userRow ) )
			return;

		// may be null !
		CCharacter *c = dynamic_cast<CCharacter*>(e);

		uint64 val;

		if (value=="1" || value=="on" || strlwr(value)=="true" )
		{
			if (c != NULL)
				c->setInvisibility(true);

			if (IsRingShard)
			{
				// seup the default value to use
				val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_PLAYER,false);

				CPlayer * player = PlayerManager.getPlayer( PlayerManager.getPlayerId(entity) );
				if (player != NULL )
				{
					if( player->havePriv(":VG:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_VG,false);
					if( player->havePriv(":SG:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_SG,false);
					if( player->havePriv(":EG:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_EG,false);
					if( player->havePriv(":EM:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_EM,false);
					if( player->havePriv(":GM:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_GM,false);
					if( player->havePriv(":SGM:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_SGM,false);
					if( player->havePriv(":DEV:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_DEV,false);
				}
			}
			else
			{
				val=0;
			}
		}
		else if (value=="0" || value=="off" || strlwr(value)=="false" )
		{
			if (c != NULL)
				c->setInvisibility(false);

			if (IsRingShard)
			{
				// seup the default value to use
				val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_PLAYER,false);

				CPlayer * player = PlayerManager.getPlayer( PlayerManager.getPlayerId(entity) );
				if (player != NULL )
				{
					if( player->havePriv(":VG:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_VG,false);
					if( player->havePriv(":SG:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_SG,false);
					if( player->havePriv(":EG:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_EG,false);
					if( player->havePriv(":EM:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_EM,false);
					if( player->havePriv(":GM:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_GM,false);
					if( player->havePriv(":SGM:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_SGM,false);
					if( player->havePriv(":DEV:") )	val = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_DEV,false);
				}
			}
			else
			{
				val=~0;
			}
		}
		else
			return;

		// Set visibility of character
		e->setWhoSeesMe( val );

		// Force unmounting (can't be invisible on a mount)
		if ( c )
		{
			CEntityBase *mount = c->getMountEntity();
			if ( mount )
				c->unmount( true );
		}

		// Ignore tells if invisible
		CMessage msgout("IGNORE_TELL_MODE");
		msgout.serial( const_cast<CEntityId&>(e->getId()) );
		bool ignore = (val == 0);
		msgout.serial( ignore );
		CUnifiedNetwork::getInstance()->send("IOS", msgout);

		nlinfo ("%s has now '%"NL_I64"X' in invisibility", entity.toString().c_str(), e->getWhoSeesMe());
	}
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(broadcast,"[repeat=<num repeat> or during=<time in seconds>] [every=<delay in secondes>] message","<message>")
{
	if( args.size() < 1 )
	{
		return false;
	}

	string message;
	uint i;
	for( i = 0; i< args.size(); i++ )
	{
		message += args[i];
		if( i != args.size() - 1 )
		{
			message += " ";
		}
	}

	uint32 repeat = 1;
	uint32 during = 0;
	uint32 every = 0;

	string::size_type posMessage = 0;

	string::size_type pos = message.find("repeat");
	if( pos != string::npos )
	{
		string::size_type posEgale = message.find("=", pos);
		if( posEgale != string::npos )
		{
			repeat = atoi( message.substr( posEgale+1 ).c_str() );
			if( posEgale + 1 > posMessage )
				posMessage = posEgale + 1;
		}
	}

	pos = message.find("during");
	if( pos != string::npos )
	{
		string::size_type posEgale = message.find("=", pos);
		if( posEgale != string::npos )
		{
			during = atoi( message.substr( posEgale+1 ).c_str() );
			if( posEgale + 1 > posMessage )
				posMessage = posEgale + 1;
		}
	}

	pos = message.find("every");
	if( pos != string::npos )
	{
		string::size_type posEgale = message.find("=", pos);
		if( posEgale != string::npos )
		{
			every = atoi( message.substr( posEgale+1 ).c_str() );
			if( posEgale + 1 > posMessage )
				posMessage = posEgale + 1;
		}
	}

	if( posMessage != 0 )
	{
		posMessage = message.find( " ", posMessage+1 );
		if( posMessage != string::npos )
		{
			message = message.substr( posMessage+1 );
		}
		else
		{
			message.clear();
		}
	}

	if( message.size() == 0 )
	{
		log.displayNL("You must enter a not empty message");
		return false;
	}

	if( repeat > 1 && during != 0 )
	{
		log.displayNL("You can't use repeat and during option in same time");
		return false;
	}

	if( ( ( repeat > 1 || during > 0 ) && every == 0 ) || ( ( every > during ) && during > 0 ) )
	{
		log.displayNL("Can't execute broadcast command, check you'r repeat/during/every parameters");
		return false;
	}

	log.displayNL("Execute Broadcast: repeat=%d during=%d every=%d message=%s", repeat, during, every, message.c_str() );
	PlayerManager.broadcastMessage( repeat, during, every, message );
	return true;
}

//----------------------------------------------------------------------------
ENTITY_VARIABLE (God, "God mode, invulnerability")
{
	ENTITY_GET_CHARACTER

	if (get)
	{
		value = c->godMode()?"1":"0";
	}
	else
	{
		if (value=="1" || value=="on" || strlwr(value)=="god" || strlwr(value)=="true" )
		{
			c->setGodModeSave(true);
			c->setGodMode(true);
		}
		else if (value=="0" || value=="off" || strlwr(value)=="false" )
		{
			c->setGodModeSave(false);
			c->setGodMode(false);
		}
		nlinfo ("%s %s now in god mode", entity.toString().c_str(), c->godMode()?"is":"isn't");
	}
}

//----------------------------------------------------------------------------
ENTITY_VARIABLE (Invulnerable, "Invulnerable mode, invulnerability too all")
{
	ENTITY_GET_CHARACTER

	if (get)
	{
		value = c->invulnerableMode()?"1":"0";
	}
	else
	{
		if (value=="1" || value=="on" || strlwr(value)=="invulnerable" || strlwr(value)=="true" )
		{
			c->setInvulnerableMode(true);
		}
		else if (value=="0" || value=="off" || strlwr(value)=="false" )
		{
			c->setInvulnerableMode(false);
		}
		nlinfo ("%s %s now in invulnerable mode", entity.toString().c_str(), c->invulnerableMode()?"is":"isn't");
	}
}

//----------------------------------------------------------------------------
ENTITY_VARIABLE (ShowFactionChannels, "Show faction channels")
{
	ENTITY_GET_CHARACTER

	if (get)
	{
		value = c->showFactionChannelsMode()?"1":"0";
	}
	else
	{
		if (value=="1" || value=="on" || strlwr(value)=="true" )
			c->setShowFactionChannelsMode(true);
		else if (value=="0" || value=="off" || strlwr(value)=="false" )
			c->setShowFactionChannelsMode(false);
		nlinfo ("%s %s now in show faction channel mode", entity.toString().c_str(), c->showFactionChannelsMode()?"is":"isn't");
	}
}

//----------------------------------------------------------------------------
ENTITY_VARIABLE (PriviledgePVP, "Priviledge Pvp Mode")
{
	ENTITY_GET_CHARACTER

	if (get)
	{
		value = c->priviledgePVP()?"1":"0";
	}
	else
	{
		if (value=="1" || value=="on" || strlwr(value)=="pvp" || strlwr(value)=="true" )
			c->setPriviledgePVP(true);
		else if (value=="0" || value=="off" || strlwr(value)=="false" )
			c->setPriviledgePVP(false);
		nlinfo ("%s %s now in pvp mode", entity.toString().c_str(), c->priviledgePVP()?"is":"isn't");
	}
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(addPosFlag, "add a new position flag", "<csr_eid> <flag_name>")
{
	if (args.size() != 2)
		return false;
	GET_CHARACTER

	const string & flagName = args[1];
	if (CPositionFlagManager::getInstance().flagExists(flagName))
	{
		CCharacter::sendDynamicSystemMessage(eid, "CSR_POS_FLAG_EXISTS");
		return false;
	}

	CFlagPosition flagPos( c->getState().X()/1000, c->getState().Y()/1000, c->getState().Z()/1000 );
	CPositionFlagManager::getInstance().setFlag(flagName, flagPos);
	CPositionFlagManager::getInstance().saveToFile(PositionFlagsFileName);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setPosFlag, "set a position flag", "<csr_eid> <flag_name>")
{
	if (args.size() != 2)
		return false;
	GET_CHARACTER

	const string & flagName = args[1];
	if (!CPositionFlagManager::getInstance().flagExists(flagName))
	{
		CCharacter::sendDynamicSystemMessage(eid, "CSR_POS_FLAG_NOT_EXISTS");
		return false;
	}

	CFlagPosition flagPos( c->getState().X()/1000, c->getState().Y()/1000, c->getState().Z()/1000 );
	CPositionFlagManager::getInstance().setFlag(flagName, flagPos);
	CPositionFlagManager::getInstance().saveToFile(PositionFlagsFileName);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(delPosFlag, "delete a position flag", "<csr_eid> <flag_name>")
{
	if (args.size() != 2)
		return false;
	GET_CHARACTER

	const string & flagName = args[1];
	if (!CPositionFlagManager::getInstance().flagExists(flagName))
	{
		CCharacter::sendDynamicSystemMessage(eid, "CSR_POS_FLAG_NOT_EXISTS");
		return false;
	}

	CPositionFlagManager::getInstance().removeFlag(flagName);
	CPositionFlagManager::getInstance().saveToFile(PositionFlagsFileName);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(lPosFlags, "list position flags (short format)", "<csr_eid> [<radius_in_meters>]")
{
	if (args.size() < 1 || args.size() > 2)
		return false;
	GET_CHARACTER

	uint32 radius = 0;
	if (args.size() == 2)
	{
		int arg = atoi(args[1].c_str());
		if (arg > 0)
			radius = arg;
	}

	CPositionFlagManager::getInstance().sendFlagsList(eid, true, radius);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(listPosFlags, "list position flags (long format)", "<csr_eid> [<radius_in_meters>]")
{
	if (args.size() < 1 || args.size() > 2)
		return false;
	GET_CHARACTER

	uint32 radius = 0;
	if (args.size() == 2)
	{
		int arg = atoi(args[1].c_str());
		if (arg > 0)
			radius = arg;
	}

	CPositionFlagManager::getInstance().sendFlagsList(eid, false, radius);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(tpPosFlag, "teleport a player to a position flag", "<eid> <flag_name>")
{
	if (args.size() != 2)
		return false;
	GET_CHARACTER

	const string & flagName = args[1];
	const CFlagPosition * flagPos = CPositionFlagManager::getInstance().getFlagPosition(flagName);
	if (flagPos == NULL)
	{
		CCharacter::sendDynamicSystemMessage(eid, "CSR_POS_FLAG_NOT_EXISTS");
		return false;
	}

	const sint32 x = flagPos->X * 1000;
	const sint32 y = flagPos->Y * 1000;
	const sint32 z = flagPos->Z * 1000;

	c->allowNearPetTp();
	c->teleportCharacter(x, y, z, true);

	CContinent * continent = CZoneManager::getInstance().getContinent(x, y);
	if (continent != NULL)
	{
		c->getRespawnPoints().addDefaultRespawnPoint( CONTINENT::TContinent(continent->getId()) );
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(updateGuildMembersList, "update guild members list on members clients", "<csr eid> <guild_name>|<shardId:guildId>")
{
	if (args.size() != 2)
		return false;

	TRY_GET_CHARACTER;

	GET_GUILD(1, false, eid);

	guild->updateMembersStringIds();

	std::map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD*>::iterator it;
	for (it = guild->getMembersBegin(); it != guild->getMembersEnd(); ++it)
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember *>( (*it).second );
		BOMB_IF( !member, "null guild member!!!", continue );
		guild->setMemberClientDB( member );
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(listGuildMembers, "display guild members list", "<csr eid> <guild_name>|<shardId:guildId>")
{
	if (args.size() != 2)
		return false;

	GET_CHARACTER;

	GET_GUILD(1, false, eid);

	std::map<EGSPD::TCharacterId,EGSPD::CGuildMemberPD*>::iterator it;
	for (it = guild->getMembersBegin(); it != guild->getMembersEnd(); ++it)
	{
		CGuildMember * member = EGS_PD_CAST<CGuildMember *>( (*it).second );
		BOMB_IF( !member, "null guild member!!!", continue );

		const string memberName = CEntityIdTranslator::getInstance()->getByEntity( member->getIngameEId() ).toUtf8();

		SM_STATIC_PARAMS_2(params, STRING_MANAGER::literal, STRING_MANAGER::literal);
		params[0].Literal.fromUtf8( memberName );
		params[1].Literal = EGSPD::CGuildGrade::toString( member->getGrade() );
		CCharacter::sendDynamicSystemMessage(eid, "CSR_GUILD_MEMBER_LIST", params);
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(guildInvite, "send a guild invite to a player character", "<eid> <member name>")
{
	if(args.size() != 2 )
		return false;

	CEntityId eId;
	eId.fromString(args[0].c_str());

	CCharacter * user = PlayerManager.getChar( eId );
	if (!user)
	{
		log.displayNL("<GUILDINVITE>'%s' is not a valid char. Cant process command",eId.toString().c_str());
		return true;
	}
	if (!user->getEnterFlag())
	{
		log.displayNL("'%s' is not entered", eId.toString().c_str());
		return true;
	}
	if (!TheDataset.isAccessible(user->getEntityRowId()))
	{
		log.displayNL("'%s' is not valid in mirror", eId.toString().c_str());
		return true;
	}

	CGuildMemberModule * gmModule = NULL;
	if ( !user->getModuleParent().getModule( gmModule ))
	{
		log.displayNL("<GUILD>'%s' has no valid guild module, Cant process command",eId.toString().c_str());
		return true;
	}

	uint32 guildId = user->getGuildId();
	CGuild *guild = CGuildManager::getInstance()->getGuildFromId(guildId);
	if ( guild == NULL)
	{
		log.displayNL("<GUILD>'%s' Failed to find guild %u",eId.toString().c_str(), guildId);
		return true;
	}

	// make sure the guild is local
	if (guild->isProxy() )
	{
		log.displayNL("<GUILD>'%s' is in a foreign guild, Cant process command", eId.toString().c_str());
		CCharacter::sendDynamicSystemMessage( eId, "GUILD_INVITE_ONLY_LOCAL_GUILD" );
		return true;
	}

	gmModule->inviteCharacterInGuild(PlayerManager.getCharacterByName(CShardNames::getInstance().makeFullNameFromRelative(user->getHomeMainlandSessionId(), args[1])));
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(addGuildMember, "add a new member to a guild", "<csr eid> <guild_name>|<shardId:guildId> <member name>")
{
	if (args.size() != 3)
		return false;

	TRY_GET_CHARACTER;

	GET_GUILD(1, true, eid);

	string memberName = CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), args[2]);
	CCharacter * memberChar = PlayerManager.getCharacterByName( memberName);
	if ( !memberChar || !memberChar->getEnterFlag() || !TheDataset.isAccessible(memberChar->getEntityRowId()) )
	{
		if ( CEntityIdTranslator::getInstance()->getByEntity( ucstring(memberName) ) == CEntityId::Unknown )
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal.fromUtf8( memberName );
			CCharacter::sendDynamicSystemMessage(eid, "CSR_UNKNOWN_PLAYER", params);
		}
		else
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal.fromUtf8( memberName );
			CCharacter::sendDynamicSystemMessage(eid, "CSR_OFFLINE_PLAYER", params);
		}
		return true;
	}

	CGuildCharProxy proxy(memberChar);

	CGuildMemberModule * module;
	if ( proxy.getModule(module) )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
		params[0].setEIdAIAlias( proxy.getId(), CAIAliasTranslator::getInstance()->getAIAlias(proxy.getId()) );
		CCharacter::sendDynamicSystemMessage(eid, "CSR_GUILD_ALREADY_MEMBER", params);
		return true;
	}

	// message for all guild members about the new member
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
		params[0].setEIdAIAlias( proxy.getId(), CAIAliasTranslator::getInstance()->getAIAlias(proxy.getId()) );
		guild->sendMessageToGuildMembers("GUILD_JOIN", params);
	}

	proxy.setGuildId( guild->getId() );

	// message for the new member
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::string_id);
		params[0].StringId = guild->getNameId();
		proxy.sendSystemMessage("GUILD_YOU_JOIN", params);
	}

	CGuildMember * member = guild->newMember( proxy.getId() );
	BOMB_IF( !member, "created null guild member!", return true );

	// ask the client to open it's guild interface
	PlayerManager.sendImpulseToClient( proxy.getId(),"GUILD:OPEN_GUILD_WINDOW" );

//	module = new CGuildMemberModule( proxy, member );
//	BOMB_IF( !module, "created null guild member module!", return true );
//
//	member->setMemberGrade( EGSPD::CGuildGrade::Member );
//	guild->setMemberOnline( member, proxy.getId().getDynamicId() );

	log.displayNL("%s now is a member of guild '%s'", memberName.c_str(), guild->getName().toString().c_str() );
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::player, STRING_MANAGER::string_id);
		params[0].setEIdAIAlias( proxy.getId(), CAIAliasTranslator::getInstance()->getAIAlias(proxy.getId()) );
		params[1].StringId = guild->getNameId();
		CCharacter::sendDynamicSystemMessage(eid, "CSR_GUILD_NEW_MEMBER", params);
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setGuildMemberGrade, "set the grade of a guild member", "<csr eid> <guild_name>|<shardId:guildId> <memberName> <grade = Member/Officer/HighOfficer/Leader>")
{
	if (args.size() != 4)
		return false;

	TRY_GET_CHARACTER;

	GET_GUILD(1, true, eid);

	string memberName = CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), args[2]);
	CEntityId memberEId = CEntityIdTranslator::getInstance()->getByEntity( ucstring(memberName) );
	if (memberEId == CEntityId::Unknown)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
		params[0].Literal.fromUtf8( memberName );
		CCharacter::sendDynamicSystemMessage(eid, "CSR_UNKNOWN_PLAYER", params);
		return true;
	}

	EGSPD::CGuildGrade::TGuildGrade grade = EGSPD::CGuildGrade::fromString( args[3] );

	CGuildMember * member = guild->getMemberFromEId( memberEId );
	if ( !member )
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::literal, STRING_MANAGER::string_id);
		params[0].Literal.fromUtf8( memberName );
		params[1].StringId = uint32(guild->getNameId());
		CCharacter::sendDynamicSystemMessage(eid, "CSR_GUILD_NOT_MEMBER", params);
		return true;
	}

	guild->setMemberGrade( member, grade, &log, eid );

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setGuildLeader, "set the leader of a guild", "<csr eid> <guild_name>|<shardId:guildId> <member name>")
{
	if (args.size() != 3)
		return false;

	TRY_GET_CHARACTER;

	GET_GUILD(1, true, eid);

	string memberName = CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(), args[2]);
	CEntityId memberEId = CEntityIdTranslator::getInstance()->getByEntity( ucstring(memberName) );
	if (memberEId == CEntityId::Unknown)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
		params[0].Literal.fromUtf8( memberName );
		CCharacter::sendDynamicSystemMessage(eid, "CSR_UNKNOWN_PLAYER", params);
		return true;
	}

	CGuildMember * member = guild->getMemberFromEId( memberEId );
	if ( !member )
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::literal, STRING_MANAGER::string_id);
		params[0].Literal.fromUtf8( memberName );
		params[1].StringId = guild->getNameId();
		CCharacter::sendDynamicSystemMessage(eid, "CSR_GUILD_NOT_MEMBER", params);
		return true;
	}

	if (member->getGrade() == EGSPD::CGuildGrade::Leader)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
		params[0].Literal.fromUtf8( memberName );
		CCharacter::sendDynamicSystemMessage(eid, "CSR_GUILD_ALREADY_HAS_GRADE", params);
		return true;
	}

	CGuildMember * leader = guild->getLeader();
	if (leader)
		guild->setMemberGrade( leader, EGSPD::CGuildGrade::Member, &log, eid );

	guild->setMemberGrade( member, EGSPD::CGuildGrade::Leader, &log, eid );

	return true;
}

//----------------------------------------------------------------------------
/*
NLMISC_COMMAND(setPvpClan, "set the pv clan for player", "<csr eid> <clan number 0=neutral, 1=clan1, 2=clan2 >")
{
	if (args.size() != 2)
		return false;

	TRY_GET_CHARACTER;

	uint8 clan = atoi(args[1].c_str());

	if ( c && c->getEnterFlag() )
	{
		if ( c->getPVPInterface().isValid() )
			c->getPVPInterface().setUserClan( clan );
	}
	return true;
}
*/

//----------------------------------------------------------------------------
NLMISC_COMMAND(startEvent, "start an event with the given name", "<csr eid> <event name> [<faction1> <faction2> [<Faction 1 GM channel name> <faction2 GM channel Name> [<zone only ?>]]]")
{
	if (args.size() != 2 && args.size() < 4)
		return false;

	TRY_GET_CHARACTER;

	const string & eventName = args[1];
	string eventFaction1;
	string eventFaction2;
	string eventFaction1ChannelName;
	string eventFaction2ChannelName;
	bool factionChanelInZoneOnly = false;

	if( args.size() > 3 )
	{
		eventFaction1 = args[2];
		eventFaction2 = args[3];

		if( args.size() > 5 )
		{
			eventFaction1ChannelName = args[4];
			eventFaction2ChannelName = args[5];
		}

		if( args.size() > 6 )
		{
			factionChanelInZoneOnly = atoi(args[6].c_str()) != 0;
		}
	}

	CGameEventManager::getInstance().resetGameEvent(eventName, eventFaction1, eventFaction2, eventFaction1ChannelName, eventFaction2ChannelName, factionChanelInZoneOnly);

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
	params[0].Literal.fromUtf8(eventName);
	CCharacter::sendDynamicSystemMessage(eid, "CSR_START_EVENT", params);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(stopEvent, "stop previous started event", "<csr eid>")
{
	if (args.size() != 1)
		return false;

	TRY_GET_CHARACTER;

	const string & eventName = CGameEventManager::getInstance().getGameEventName();
	CGameEventManager::getInstance().resetGameEvent(string(""),string(""),string(""),string(""),string(""),false);

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
	params[0].Literal.fromUtf8(eventName);
	CCharacter::sendDynamicSystemMessage(eid, "CSR_STOP_EVENT", params);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setEventFaction, "set the event faction of player", "<csr eid> <player name> <event faction>")
{
	if (args.size() != 3)
		return false;

	TRY_GET_CHARACTER;

	const string & playerName	= args[1];
	const string & eventFaction	= args[2];

	CCharacter * player = PlayerManager.getCharacterByName(CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),playerName));
	if (!player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
		params[0].Literal.fromUtf8( playerName );
		CCharacter::sendDynamicSystemMessage(eid, "CSR_UNKNOWN_PLAYER", params);
		return true;
	}

	player->getGameEvent().setEventFaction(eventFaction);

	SM_STATIC_PARAMS_2(params, STRING_MANAGER::player, STRING_MANAGER::event_faction);
	params[0].setEIdAIAlias( player->getId(), CAIAliasTranslator::getInstance()->getAIAlias(player->getId()) );
	params[1].Identifier = eventFaction;
	CCharacter::sendDynamicSystemMessage(eid, "CSR_GET_EVENT_FACTION", params);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(clearEventFaction, "clear the event faction of player", "<csr eid> <player name>")
{
	if (args.size() != 2)
		return false;

	TRY_GET_CHARACTER;

	const string & playerName = args[1];

	CCharacter * player = PlayerManager.getCharacterByName(CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),playerName));
	if (!player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
		params[0].Literal.fromUtf8( playerName );
		CCharacter::sendDynamicSystemMessage(eid, "CSR_UNKNOWN_PLAYER", params);
		return true;
	}

	player->getGameEvent().clearEventFaction();

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	params[0].setEIdAIAlias( player->getId(), CAIAliasTranslator::getInstance()->getAIAlias(player->getId()) );
	CCharacter::sendDynamicSystemMessage(eid, "CSR_CLEAR_EVENT_FACTION", params);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(getEventFaction, "get the event faction of player", "<csr eid> <player name>")
{
	if (args.size() != 2)
		return false;

	GET_CHARACTER;

	const string & playerName = args[1];

	CCharacter * player = PlayerManager.getCharacterByName(CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),playerName));
	if (!player)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
		params[0].Literal.fromUtf8( playerName );
		CCharacter::sendDynamicSystemMessage(eid, "CSR_UNKNOWN_PLAYER", params);
		return true;
	}

	SM_STATIC_PARAMS_2(params, STRING_MANAGER::player, STRING_MANAGER::event_faction);
	params[0].setEIdAIAlias( player->getId(), CAIAliasTranslator::getInstance()->getAIAlias(player->getId()) );
	params[1].Identifier = player->getGameEvent().getEventFaction();
	CCharacter::sendDynamicSystemMessage(eid, "CSR_GET_EVENT_FACTION", params);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(provideItemService, "provide a service from an item to a player", "<eid> <sheet name>")
{
	if (args.size() != 2)
		return false;

	GET_CHARACTER;

	CSheetId sheetId(args[1]);
	const CStaticItem * form = CSheets::getForm(sheetId);
	if (form)
	{
		CItemServiceManager::getInstance()->provideService(form, c);
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(dumpFactionPVPDamage, "dump damage made in Faction PvP", "<eid>")
{
	if (args.size() != 1)
		return false;

	GET_CHARACTER;

	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->dumpPlayerDamageScoreTable(c, log);
	log.displayNL("----------------------------------------------------------------------------");

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(changeHairCut, "change the haircut of a player", "<eid> <sheet name>")
{
	if (args.size() != 2)
		return false;

	GET_CHARACTER;

	CSheetId sheetId(args[1]);
	const CStaticItem * form = CSheets::getForm(sheetId);
	if (form == NULL)
	{
		nlwarning("unknown item : '%s'", sheetId.toString().c_str());
		return true;
	}

	if (form->Type != ITEM_TYPE::HAIR_MALE && form->Type != ITEM_TYPE::HAIR_FEMALE)
	{
		nlwarning("'%s' is not a haircut item", sheetId.toString().c_str());
		return true;
	}

	uint32 hairValue = CVisualSlotManager::getInstance()->sheet2Index(form->SheetId, SLOTTYPE::HEAD_SLOT);
	if (c->setHair(hairValue))
	{
		c->resetHairCutDiscount();
	}

	return true;
}

/***********************************************************************************************************************

		END OF OFFICIAL CSR COMMANDS

 ***********************************************************************************************************************/


///////////////////////
// DYN CHAT COMMANDS //
///////////////////////

//============================================================================================================
// create a new channel with a localized name
NLMISC_COMMAND(addLocalizedChan,"Add a new dyn chat channel","<string name of the channel localized name>")
{
	if (args.size() != 1) return false;
	TChanID chanID = DynChatEGS.addLocalizedChan(args[0]);
	if (chanID == DYN_CHAT_INVALID_CHAN)
	{
		nlwarning("Can't create localized channel %s", args[0].c_str());
		return true;
	}
	return true;
}

//============================================================================================================
// create a new channel
NLMISC_COMMAND(addChan,"Add a new dyn chat channel","<string name of the channel localized name>")
{
	if (args.size() < 2) return false;
	ucstring mess;
	for (uint k = 2; k < args.size(); ++k)
	{
		if (k != 0) mess += ucstring(" ");
		mess += ucstring(args[k]);
	}
	TChanID chanID = DynChatEGS.addChan(args[0], mess);
	if (chanID == DYN_CHAT_INVALID_CHAN)
	{
		nlwarning("Can't create channel %s", args[0].c_str());
		return true;
	}
	return true;
}


//============================================================================================================
// remove a channel
NLMISC_COMMAND(removeChan, "Remove a new dyn chat channel","<string name of the channel localized name>")
{
	if (args.size() != 1) return false;
	TChanID chanID = DynChatEGS.getChanIDFromName(args[0]);
	if (chanID == DYN_CHAT_INVALID_CHAN)
	{
		nlwarning("Unknown channel : %s", args[1].c_str());
		return true;
	}
	bool res = DynChatEGS.removeChan(chanID);
	if (!res)
	{
		nlwarning("Couldn't remove chan : %s", args[1].c_str());
	}
	return true;
}

//============================================================================================================
// set historic size for a channel
NLMISC_COMMAND(setChanHistoricSize, "Set size of the historic for a localized channel", "<string name of the channel localized name><size>")
{
	if (args.size() != 2) return false;
	TChanID chanID = DynChatEGS.getChanIDFromName(args[0]);
	if (chanID == DYN_CHAT_INVALID_CHAN)
	{
		nlwarning("Unknown channel : %s", args[0].c_str());
		return true;
	}
	uint historicSize = atoi(args[1].c_str());
	//
	if (historicSize > 1000)
	{
		nlwarning("Historic size too big");
		return true;
	}
	DynChatEGS.setHistoricSize(chanID, historicSize);
	return true;
}

//============================================================================================================
// add a client to a channel
NLMISC_COMMAND(addChanClient, "add a client to a channel", "<client name or user id><string name of the channel localized name>[1=Read/Write,0=ReadOnly(default)]")
{
	if (args.size() != 2) return false;
	GET_CHARACTER
	TChanID chanID = DynChatEGS.getChanIDFromName(args[1]);
	if (chanID == DYN_CHAT_INVALID_CHAN)
	{
		nlwarning("Unknown channel : %s", args[1].c_str());
		return true;
	}
	bool writeRight = true;
	if (args.size() > 2 && atoi(args[2].c_str()) == 1)
	{
		writeRight = true;
	}
	bool res = DynChatEGS.addSession(chanID, c->getEntityRowId(), writeRight);
	if (!res)
	{
		nlwarning("Couldn't add character %s to channel %s", args[0].c_str(), args[1].c_str());
	}
	return true;
}

//============================================================================================================
// remove clients from channels
NLMISC_COMMAND(removeChanClient, "remove a client to a channel", "<client name or user id><string name of the channel localized name>")
{
	if (args.size() != 2) return false;
	GET_CHARACTER
	TChanID chanID = DynChatEGS.getChanIDFromName(args[1]);
	if (chanID == DYN_CHAT_INVALID_CHAN)
	{
		nlwarning("Unknown channel : %s", args[1].c_str());
		return true;
	}
	bool res = DynChatEGS.removeSession(chanID, c->getEntityRowId());
	if (!res)
	{
		nlwarning("Couldn't remove character %s from channel %s", args[0].c_str(), args[1].c_str());
	}
	return true;
}

//============================================================================================================
// Set the read only for a client in a channel
NLMISC_COMMAND(setChanClientWriteRight, "set write right for a client in the given channel", "<client name or user id><string name of the channel localized name><1=Read/Write,0=ReadOnly(default)>")
{
	if (args.size() != 3) return false;
	GET_CHARACTER
	TChanID chanID = DynChatEGS.getChanIDFromName(args[1]);
	if (chanID == DYN_CHAT_INVALID_CHAN)
	{
		nlwarning("Unknown channel : %s", args[1].c_str());
		return true;
	}
	bool res = DynChatEGS.setWriteRight(chanID, c->getEntityRowId(), atoi(args[2].c_str()) != 0);
	if (!res)
	{
		nlwarning("Couldn't remove character %s from channel %s", args[0].c_str(), args[1].c_str());
	}
	return true;
}

//============================================================================================================
// Display the list of all channels
NLMISC_COMMAND(chanList, "display the list of all channels", "<>")
{
	if (!args.empty()) return false;
	std::vector<CDynChatChan *> chans;
	DynChatEGS.getChans(chans);
	for(uint k = 0; k < chans.size(); ++k)
	{
		ucstring name = DynChatEGS.getChanNameFromID(chans[k]->getID());
		nlinfo("Channel name : %s, num sessions = %d, historic size = %d", name.toString().c_str(), (int) chans[k]->getSessionCount(), (int) chans[k]->HistoricSize);
	}
	return true;
}

//----------------------------------------------------------------------------
ENTITY_VARIABLE (Aggro, "Aggroable by creatures")
{
	ENTITY_GET_CHARACTER

		if (get)
		{
			value = toString (c->getAggroableOverride());
		}
		else
		{
			TDataSetRow userRow = TheDataset.getDataSetRow( c->getId() );
			if ( !TheDataset.isAccessible( userRow ) )
				return;
			c->setAggroableOverride(atoi(value.c_str()));
			c->setAggroableSave(atoi(value.c_str()));

			sint8 aggroable = c->getAggroableOverride();
			nlinfo ("%s aggroable = %d", entity.toString().c_str(), aggroable);
			if (aggroable>0)
				nlinfo ("%s is now aggroable", entity.toString().c_str());
			else if (aggroable<0)
				nlinfo ("%s aggroable is defined by privilege", entity.toString().c_str());
			else
				nlinfo ("%s is now non aggroable", entity.toString().c_str());
		}
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(acceptProposalForQueue, "player accept to enter critical part (for queue Id i)", "<eid> <accept (0/1)> [<queueId>]")
{
	if (args.size() < 2 || args.size() > 3 ) return false;

	CEntityId eid(args[0]);
	bool accept = atoi(args[1].c_str()) != 0;

	uint32 queueId = 0;
	if (args.size() == 3)
		queueId = atoi(args[1].c_str());
	else
	{
		CCharacter *player = PlayerManager.getChar(eid);
		if (player)
			queueId = player->getEnterCriticalZoneProposalQueueId();
	}

	CMissionQueueManager::getInstance()->playerEntersCriticalArea(eid, queueId, accept);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(awakePlayerInQueue, "awake player in given queue", "<eid> <queueId>")
{
	if (args.size() != 2) return false;

	CEntityId eid(args[0]);
	uint32 queueId = atoi(args[1].c_str());

	CMissionQueueManager::getInstance()->playerWakesUp( eid, queueId);

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(debugMissionsQueues, "dump mission queues for debug", "")
{
	CMissionQueueManager::getInstance()->dump();

	return true;
}

static bool serialMagicNumberMsg(const std::string &msgName, const std::string &userID, uint64 magicNumber)
{
	CMessage msgout( "IMPULSION_ID" );
	CEntityId eid(userID);
	msgout.serial(eid);
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream(msgName, bms) )
	{
		nlwarning("Msg name %s not found", msgName.c_str());
		return false;
	}
	bms.serial(magicNumber);
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send(NLNET::TServiceId(eid.getDynamicId()), msgout );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(permanentBan, "permanently ban a player (player must be online)", "")
{
	if (args.size() != 1) return false;
	return serialMagicNumberMsg("CONNECTION:PERMANENT_BAN", args[0], PermanentBanMSGMagicNumber);
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(permanentUnban, "unban a player (player must be online)", "")
{
	if (args.size() != 1) return false;
	return serialMagicNumberMsg("CONNECTION:UNBAN", args[0], PermanentUnbanMSGMagicNumber);
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(nbKnownPhrases, "display nb of known phrases for player", "<eid>")
{
	if (args.size() != 1)
		return false;

	CEntityId eid(args[0]);

	CCharacter *player= PlayerManager.getChar(eid);
	if (!player)
	{
		nlwarning("Player %s not found", eid.toString().c_str());
		return true;
	}

	const vector<CKnownPhrase> &kp = player->getKnownPhrases();
	nlinfo("For player %s, size of Known Phrases vector = %u", eid.toString().c_str(), kp.size());

	uint count = 0;
	for (uint i = 0;i < kp.size() ; ++i )
	{
		if (!kp[i].empty())
			++count;
	}
	nlinfo("Total nb of non null known phrases = %u", count);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(displayShopSelector, "display shop selector for a NPC", "")
{
	if (args.size() != 1) return false;

	CEntityId eid(args[0]);
	CCreature *c = CreatureManager.getCreature( eid );
	if( c != 0 )
	{
		c->displayShopSelectors( log );
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(addFactionAttackableToTarget, "Add a faction to attackable list to given entity target creature/npc", "<player eid> <faction name> <fameLevel -600+600> <0/1 0 = below 1 = above>")
{
	if (args.size () < 4) return false;
	GET_CHARACTER

	// get faction
	uint factionIndex = CStaticFames::getInstance().getFactionIndex(args[1]);
	if (factionIndex != CStaticFames::INVALID_FACTION_INDEX)
	{
		const CEntityId &target = c->getTarget();
		if (target != CEntityId::Unknown && target.getType() != RYZOMID::player)
		{
			CCreature *creature = CreatureManager.getCreature(target);
			if (creature)
			{
				const sint32 fameLevel = sint32( atoi(args[2].c_str())*FameAbsoluteMax / 100 );
				bool above = (atoi(args[3].c_str()) ? true : false);
				creature->addFactionAttackable(factionIndex, fameLevel, above);
			}
		}
	}

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(taskPass, "pass a task from a rite", "<player eid> <rite> [<task>]")
{
	if (args.size () != 2 && args.size () != 3) return false;
	GET_CHARACTER

	string rite = args[1];
	uint32 nAlbum = 0;
	uint32 nThema = 0;
	uint32 taskCount = 0;
	CSheets::getEncyclopedia().getRiteInfos( rite, nAlbum, nThema, taskCount );

	if( nAlbum!=0 && nThema!=0 && taskCount!=0 )
	{
		CCharacterEncyclopedia &rEncy = c->getEncyclopedia();
		if( args.size() == 3 )
		{
			uint32 nTask = atoi( args[2].c_str() );
			if( nTask > 7 )
			{
				nlwarning("<taskPass> task index too high : %d",nTask);
				return false;
			}
			rEncy.updateTask(nAlbum, nThema, nTask, 2, true);
		}
		else
		{
			uint i;
			for( i=0; i<=taskCount; ++i )
			{
				rEncy.updateTask(nAlbum, nThema, i, 2, true);
			}
		}
	}
	else
	{
		nlwarning("<taskPass> wrong rite ? %s",rite.c_str());
	}

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(setFamePlayer, "set the fame value of a player in the given faction", "<player eid> <faction> <fame>")
{
	if (args.size () != 3)
		return false;

	GET_CHARACTER

	string faction = args[1];
	uint32 factionIndex	= PVP_CLAN::getFactionIndex( PVP_CLAN::fromString(faction) );
	sint32 fame = atoi( args[2].c_str() );

	CFameManager::getInstance().setEntityFame(c->getId(), factionIndex, fame, true);

	return true;

}



//----------------------------------------------------------------------------
NLMISC_COMMAND(eventCreateNpcGroup, "create an event npc group", "<player eid> <nbBots> <sheet> [<dispersionRadius=10m> [<spawnBots=true>]]")
{
	if (args.size () < 3) return false;
	GET_ENTITY

	uint32 instanceNumber = e->getInstanceNumber();
	sint32 x = e->getX();
	sint32 y = e->getY();
	sint32 z = e->getZ();

	uint32 nbBots = NLMISC::atoui(args[1].c_str());
	if (nbBots<=0)
	{
		log.displayNL("invalid bot count");
		return true;
	}

	NLMISC::CSheetId sheetId(args[2]);
	if (sheetId==NLMISC::CSheetId::Unknown)
		sheetId = args[2] + ".creature";
	if (sheetId==NLMISC::CSheetId::Unknown)
	{
		log.displayNL("invalid sheet id");
		return true;
	}

	double dispersionRadius = 10.;
	if (args.size()>3)
	{
		dispersionRadius = atof(args[3].c_str());
		if (dispersionRadius<0.)
		{
			log.displayNL("invalid dispersion radius");
			return true;
		}
	}

	bool spawnBots = true;
	if (args.size()>4)
	{
		NLMISC::fromString(args[4], spawnBots);
	}

	CMessage msgout("EVENT_CREATE_NPC_GROUP");
	uint32 messageVersion = 1;
	msgout.serial(messageVersion);
	msgout.serial(instanceNumber);
	msgout.serial(x);
	msgout.serial(y);
	msgout.serial(z);
	msgout.serial(nbBots);
	msgout.serial(sheetId);
	msgout.serial(dispersionRadius);
	msgout.serial(spawnBots);
	CWorldInstances::instance().msgToAIInstance2(instanceNumber, msgout);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventNpcGroupScript, "executes a script on an event npc group", "<bot eid> <script>")
{
	if (args.size () < 1) return false;
	GET_ENTITY

	uint32 instanceNumber = e->getInstanceNumber();

	uint32 nbString = (uint32)args.size();

	CMessage msgout("EVENT_NPC_GROUP_SCRIPT");
	uint32 messageVersion = 1;
	msgout.serial(messageVersion);
	msgout.serial(nbString);
	for (uint32 i=0; i<nbString; ++i)
	{
		string arg = args[i];
		msgout.serial(arg);
	}
	CWorldInstances::instance().msgToAIInstance2(instanceNumber, msgout);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetBotName, "changes the name of a bot", "<bot eid> <name>")
{
	if (args.size () < 2) return false;
	GET_ENTITY

	TDataSetRow row = e->getEntityRowId();
	ucstring name;
	name.fromUtf8(args[1]);
	NLNET::CMessage	msgout("CHARACTER_NAME");
	msgout.serial(row);
	msgout.serial(name);
	sendMessageViaMirror("IOS", msgout);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetBotScale, "changes the scale of a bot (in % up to 255)", "<bot eid> <scale in %>")
{
	if (args.size () < 2) return false;
	GET_ENTITY

	TDataSetRow row = e->getEntityRowId();
	uint32 scale = atoi(args[1].c_str());
	if (scale>255)
		scale = 0;
 	CMirrorPropValue< SAltLookProp2, CPropLocationPacked<2> > visualPropertyB( TheDataset, row, DSPropertyVPB );
	SET_STRUCT_MEMBER( visualPropertyB, PropertySubData.Scale, scale );

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetNpcGroupAggroRange, "changes the aggro range of a npc group", "<bot eid> <range>")
{
	if (args.size() < 2) return false;
	GET_ENTITY

	uint32 instanceNumber = e->getInstanceNumber();

	std::vector<std::string> args2;

	args2.push_back(args[0]);
	args2.push_back(NLMISC::toString("()setAggro(%f, 0);", atof(args[1].c_str())));

	uint32 nbString = (uint32)args2.size();

	CMessage msgout("EVENT_NPC_GROUP_SCRIPT");
	uint32 messageVersion = 1;
	msgout.serial(messageVersion);
	msgout.serial(nbString);
	for (uint32 i=0; i<nbString; ++i)
	{
		string arg = args2[i];
		msgout.serial(arg);
	}
	CWorldInstances::instance().msgToAIInstance2(instanceNumber, msgout);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetFaunaBotAggroRange, "changes the aggro range of a fauna bot", "<bot eid> <not hungry range> [<hungry range> [<hunting range>]]")
{
	if (args.size() < 2) return false;
	GET_ENTITY

	uint32 instanceNumber = e->getInstanceNumber();

	uint32 messageVersion = 1;
	string botName = args[0];
	float notHungryRadius = (float)atof(args[1].c_str());
	float hungryRadius = -1.f;
	if (args.size() > 2)
		hungryRadius = (float)atof(args[2].c_str());
	float huntingRadius = -1.f;
	if (args.size() > 3)
		huntingRadius = (float)atof(args[3].c_str());

	CMessage msgout("EVENT_FAUNA_BOT_SET_RADII");
	msgout.serial(messageVersion);
	msgout.serial(botName);
	msgout.serial(notHungryRadius);
	msgout.serial(hungryRadius);
	msgout.serial(huntingRadius);
	CWorldInstances::instance().msgToAIInstance2(instanceNumber, msgout);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventResetFaunaBotAggroRange, "reset the aggro range of a fauna bot to sheet defaults", "<bot eid>")
{
	if (args.size() < 1) return false;
	GET_ENTITY

	uint32 instanceNumber = e->getInstanceNumber();

	uint32 messageVersion = 1;
	string botName = args[0];

	CMessage msgout("EVENT_FAUNA_BOT_RESET_RADII");
	msgout.serial(messageVersion);
	msgout.serial(botName);
	CWorldInstances::instance().msgToAIInstance2(instanceNumber, msgout);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetBotCanAggro, "tells the bot if he can aggro or not", "<bot eid> <can aggro>")
{
	if (args.size() < 2) return false;
	GET_ENTITY

	uint32 instanceNumber = e->getInstanceNumber();

	uint32 messageVersion = 1;
	string botName = args[0];
	bool canAggro = true;
	NLMISC::fromString(args[1], canAggro);

	CMessage msgout("EVENT_BOT_CAN_AGGRO");
	msgout.serial(messageVersion);
	msgout.serial(botName);
	msgout.serial(canAggro);
	CWorldInstances::instance().msgToAIInstance2(instanceNumber, msgout);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetBotSheet, "Change the sheet of a bot", "<bot eid> <sheet id>")
{
	if (args.size() < 2) return false;
	GET_ENTITY

	uint32 instanceNumber = e->getInstanceNumber();

	uint32 messageVersion = 3;
	bool bAutoSpawnDespawn = false;
	string botName = args[0];
	string sCustomName; // Not needed here
	CSheetId sheetId(args[1]);
	if (sheetId==CSheetId::Unknown)
		sheetId = CSheetId(args[1]+".creature");
	if (sheetId==CSheetId::Unknown)
	{
		log.displayNL("Unknown sheet id '%s'", args[1].c_str());
		return true;
	}

	CMessage msgout("EVENT_BOT_SHEET");
	msgout.serial(messageVersion);
	msgout.serial(botName);
	msgout.serial(sheetId);
	msgout.serial(bAutoSpawnDespawn);
	msgout.serial(sCustomName);
	CWorldInstances::instance().msgToAIInstance2(instanceNumber, msgout);

	return true;
}

//----------------------------------------------------------------------------
extern sint32 clientEventSetItemCustomText(CCharacter* character, INVENTORIES::TInventory inventory, uint32 slot, ucstring const& text);

NLMISC_COMMAND(eventSetItemCustomText, "set an item custom text, which replaces help text", "<eId> <inventory> <slot in inventory> <text>")
{
	if (args.size() < 4)
		return false;

	GET_CHARACTER
	if (!c)
	{
		log.displayNL("Invalid character '%s'", args[0].c_str());
		return true;
	}
	INVENTORIES::TInventory	inventory;
	uint32 slot;
	ucstring text;
	inventory = INVENTORIES::toInventory(args[1]);
	slot = atoi(args[2].c_str());
	text.fromUtf8(args[3]);

	sint32 ret = clientEventSetItemCustomText(c, inventory, slot, text);

	switch (ret)
	{
	case 0:
		log.displayNL("Item in slot %u has now the custom text \"%s\"", slot, text.toUtf8().c_str());
		break;
	case -1:
		log.displayNL("'%s' is not a valid inventory name", args[1].c_str());
		break;
	case -2:
		log.displayNL("Invalid slot %u", slot);
		break;
	case -3:
		log.displayNL("Empty slot %u", slot);
		break;
	default:
		log.displayNL("Unknown error");
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventResetItemCustomText, "set an item custom text, which replaces help text", "<eId> <inventory> <slot in inventory>")
{
	if (args.size() < 3)
	{
		log.displayNL("not enough parameters");
		return false;
	}

	GET_CHARACTER
	INVENTORIES::TInventory	inventory = INVENTORIES::toInventory(args[1]);
	if (inventory==INVENTORIES::UNDEFINED)
	{
		log.displayNL("'%s' is not a valid inventory name", args[1].c_str());
		return true;
	}
	uint32 slot = atoi(args[2].c_str());
	CInventoryPtr invent = c->getInventory(inventory);
	if (slot >= invent->getSlotCount())
	{
		log.displayNL("Invalid slot %u max = %u", slot, invent->getSlotCount()-1);
		return true;
	}
	if (invent->getItem(slot) == NULL)
	{
		log.displayNL("empty slot %u", slot);
		return true;
	}

	CGameItemPtr item = invent->getItem(slot);
	item->setCustomText(ucstring());
	// Following line was commented out by trap, reason unknown
	c->incSlotVersion(INVENTORIES::bag, slot);
	log.displayNL("item in slot %u has now its default text displayed", slot);

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(useCatalyser, "use an xp catalyser", "<eId> [<slot in bag>]")
{
	if (args.size() != 2)
	{
		log.displayNL("not enough parameters");
		return false;
	}

	GET_CHARACTER
	sint32 slot = atoi(args[1].c_str());
	if( slot > 0 )
	{
		c->useItem( (uint32)slot );
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetBotFaction, "changes the faction of a bot", "<bot eid> <faction>|default")
{
	if (args.size() < 2) return false;

	CEntityId eid(args[0]);
	CCreature* creature = CreatureManager.getCreature(eid);
	if (creature)
	{
		if (args[1]=="default")
			creature->resetFaction();
		else
			creature->setFaction(CStaticFames::getInstance().getFactionIndex(args[1]));
	}
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetBotFameByKill, "set the fame a player win after killing a bot", "<bot eid> <fame>|default")
{
	if (args.size() < 2) return false;

	CEntityId eid(args[0]);
	sint32 fame = 0;
	NLMISC::fromString(args[1], fame);
	CCreature* creature = CreatureManager.getCreature(eid);
	if (creature)
	{
		if (args[1]=="default")
		{
			creature->resetFameByKill();
			creature->resetFameByKillValid();
		}
		else
		{
			creature->setFameByKill(fame);
			creature->setFameByKillValid(true);
		}
	}
	return true;
}

// Dev only ------------------------------------------------------------------
NLMISC_COMMAND(displayPositionStack, "Display the position stack of a character (top to bottom) (works as soon as player is connected)", "<eid>")
{
	if ( args.size() < 1 )
		return false;

	// Get character without using GET_CHARACTER to prevent from issueing an "offline command"
	CEntityId eid(args[0]);
	if (eid == CEntityId::Unknown)
		return true;
	CCharacter *c = PlayerManager.getChar(eid);
	if(c == 0)
	{
		log.displayNL( "Character not found" );
		return true;
	}
	if ( c->sessionId() == SessionLockPositionStack )
	{
		COfflineEntityState st( c->getState() );
		log.displayNL( "[STACK LOCKED] Current pos: %s", st.toString().c_str() );
	}
	for ( sint p=((sint)(c->PositionStack.size()))-1; p>=0; --p )
	{
		log.displayNL( "%u: Session %u, %s", p, c->PositionStack[p].SessionId.asInt(), c->PositionStack[p].PosState.toString().c_str() );
	}
	return true;
}

// Dev only ------------------------------------------------------------------
NLMISC_COMMAND(popPosition, "Remove the top position in the stack of a character", "<eid>")
{
	if ( args.size() < 1 )
		return false;

	GET_CHARACTER
	if ( ! c->PositionStack.empty() )
	{
		c->PositionStack.pop();
		log.displayNL( "OK, %u remaining in stack", c->PositionStack.size() );
	}
	else
		log.displayNL( "No pos in stack" );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(farTPPush, "Far TP a character, but let the current position in the stack for returning. Pos in meters.", "<eid> <destSessionId> [<X> <Y> [<Z> [<Heading>]]]")
{
	if ( args.size() < 2 )
		return false;

	GET_CHARACTER

	// Push the new position into the stack
	c->pushCurrentPosition();
	c->PositionStack.topToModify().SessionId = (TSessionId)(uint32)atoi(args[1].c_str());
	if ( args.size() > 3 )
	{
		c->PositionStack.topToModify().PosState.X = atoi(args[2].c_str()) * 1000;
		c->PositionStack.topToModify().PosState.Y = atoi(args[3].c_str()) * 1000;
		if ( args.size() > 4 )
		{
			c->PositionStack.topToModify().PosState.Z = atoi(args[4].c_str()) * 1000;
			if ( args.size() > 5 )
			{
				c->PositionStack.topToModify().PosState.Heading = (float)atof(args[5].c_str());
			}
		}
	}

	// Lock the stack to save it in this state, and make the client Far TP
	c->setSessionId( SessionLockPositionStack );
	c->requestFarTP( c->PositionStack.topToModify().SessionId );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(farTPReplace, "Far TP a character. Pos in meters. Default values = current values", "<eid> [<destSessionId> [<X> <Y> [<Z> [<Heading>]]]]")
{
	if ( args.size() < 1 )
		return false;

	GET_CHARACTER

	// Modify the top position in the stack
	if ( c->PositionStack.empty() )
		c->pushCurrentPosition();
	if ( args.size() > 1 ) // with the same session id: will make the client reconnect
	{
		c->PositionStack.topToModify().SessionId = (TSessionId)(uint32)atoi(args[1].c_str());
		if ( args.size() > 3 )
		{
			c->PositionStack.topToModify().PosState.X = atoi(args[2].c_str()) * 1000;
			c->PositionStack.topToModify().PosState.Y = atoi(args[3].c_str()) * 1000;
			if ( args.size() > 4 )
			{
				c->PositionStack.topToModify().PosState.Z = atoi(args[4].c_str()) * 1000;
				if ( args.size() > 5 )
				{
					c->PositionStack.topToModify().PosState.Heading = (float)atof(args[5].c_str());
				}
			}
		}
	}

	// Lock the stack to save it in this state, and make the client Far TP
	c->setSessionId( SessionLockPositionStack );
	c->requestFarTP( c->PositionStack.topToModify().SessionId, false );
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(farTPReturn, "Far TP a character back to the previous session in the stack", "")
{
	if ( args.size() < 1 )
		return false;

	GET_CHARACTER
	c->returnToPreviousSession(~0, ~0, 0);
	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(farTPSubst, "Substitute a position in the stack (no immediate far TP, does not lock the stack, works as soon as player is connected). Pos in meters. Default values = current values", "<eid> index <sessionId> [<X> <Y> [<Z> [<Heading>]]]")
{
	if ( args.size() < 3 )
		return false;

	// Get character without using GET_CHARACTER to prevent from issueing an "offline command"
	CEntityId eid(args[0]);
	if (eid == CEntityId::Unknown)
		return true;
	CCharacter *c = PlayerManager.getChar(eid);
	if(c == 0)
	{
		log.displayNL( "Character not found" );
		return true;
	}

	// Access the specified position in the stack
	uint index = atoi(args[1].c_str());
	if ( c->PositionStack.size() <= index )
	{
		log.displayNL( "Index out of bounds" );
		return true;
	}

	// Modify
	CFarPosition newFarPos = c->PositionStack[index];
	newFarPos.SessionId = (TSessionId)(uint32)atoi(args[2].c_str());
	if ( args.size() > 3 )
	{
		newFarPos.PosState.X = atoi(args[3].c_str()) * 1000;
		newFarPos.PosState.Y = atoi(args[4].c_str()) * 1000;
		if ( args.size() > 5 )
		{
			newFarPos.PosState.Z = atoi(args[5].c_str()) * 1000;
			if ( args.size() > 6 )
			{
				newFarPos.PosState.Heading = (float)atof(args[6].c_str());
			}
		}
	}
	c->PositionStack.substFarPosition( index, newFarPos );
	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(teamInvite, "send a team invite to a player character", "<eid> <member name>")
{
	if(args.size() != 2 )
		return false;

	CEntityId eId;
	eId.fromString(args[0].c_str());

	// check user
	CCharacter * user = PlayerManager.getChar( eId );
	if (!user)
	{
		log.displayNL("<TEAMINVITE>'%s' is not a valid char. Cant process command",eId.toString().c_str());
		return true;
	}
	if (!user->getEnterFlag())
	{
		log.displayNL("'%s' is not entered", eId.toString().c_str());
		return true;
	}
	if (!TheDataset.isAccessible(user->getEntityRowId()))
	{
		log.displayNL("'%s' is not valid in mirror", eId.toString().c_str());
		return true;
	}

	// Get target
	CCharacter	*invitedCharacter= PlayerManager.getCharacterByName(CShardNames::getInstance().makeFullNameFromRelative(user->getHomeMainlandSessionId(), args[1]));
	if(invitedCharacter == 0 || invitedCharacter->getEnterFlag() == false )
	{
		CCharacter::sendDynamicSystemMessage( user->getId(),"TEAM_INVITED_CHARACTER_MUST_BE_ONLINE" );
		return true;
	}

	// Join
	user->setAfkState(false);
	TeamManager.joinProposal( user, invitedCharacter->getId() );

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(resetPVPTimers, "reset the pvp timers of a player", "<CSR eId><player name>")
{
	if ( args.size() < 1 )
		return false;

	GET_CHARACTER

	CCharacter * target = PlayerManager.getCharacterByName( CShardNames::getInstance().makeFullNameFromRelative(c->getHomeMainlandSessionId(),args[1] ));
	if ( !target || !TheDataset.isAccessible( target->getEntityRowId() ) )
	{
		CCharacter::sendDynamicSystemMessage( eid, "CSR_BAD_TARGET" );
		return true;
	}
	CHECK_RIGHT( c,target );

	target->resetPVPTimers();

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(revive, "player revives at full health at his location", "<eId>")
{
	if ( args.size() < 1 )
		return false;

	GET_CHARACTER

	c->revive();

	return true;
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(quitDelay, "Inform the player that the shard will be stopped in N seconds", "<timeBeforeShutdown>")
{
	if (args.size() != 1)
		return false;

	sint delay = atoi(args[0].c_str());

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);

	params[0].Int = delay;

	IPlayerManager &pm = CPlayerManager::getInstance();

	const IPlayerManager::TMapPlayers &players = pm.getPlayers();

	IPlayerManager::TMapPlayers::const_iterator first(players.begin()), last(players.end());
	for (; first != last; ++first)
	{
		CCharacter * character = pm.getActiveChar(first->first);

		if (character != NULL)
		{
			CCharacter::sendDynamicSystemMessage(TheDataset.getDataSetRow(character->getId()), "SHUTDOWN_WARNING", params);
		}
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetBotURL, "changes the url of a bot", "<bot eid> [<url>]")
{
	if (args.size() < 1) return false;

	CEntityId eid(args[0]);
	CCreature* creature = CreatureManager.getCreature(eid);
	if (!creature)
	{
		log.displayNL("Not a creature");
		return false;
	}

	uint32 program = creature->getBotChatProgram();
	if(!(program & (1<<BOTCHATTYPE::WebPageFlag))) {
		if(program != 0)
		{
			log.displayNL("This creature already had a program 0x%x, cannot add a web program", program);
			return false;
		}

		log.displayNL("Add web program on this creature");
		program |= 1 << BOTCHATTYPE::WebPageFlag;
		creature->setBotChatProgram(program);
	}

	const string &wp = creature->getWebPage();
	if(args.size() < 2) {
		log.displayNL("Remove web program on this creature");
		(string &)wp = "";
		program &= ~(1 << BOTCHATTYPE::WebPageFlag);
		creature->setBotChatProgram(program);
	}
	else
		(string &)wp = args[1];

	log.displayNL("Set url '%s'", creature->getWebPage().c_str());

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(eventSetBotURLName, "changes the url name of a bot", "<bot eid> <name>")
{
	if (args.size() < 2) return false;

	CEntityId eid(args[0]);
	CCreature* creature = CreatureManager.getCreature(eid);
	if (!creature)
	{
		log.displayNL("Not a creature");
		return false;
	}

	uint32 program = creature->getBotChatProgram();
	if(!(program & (1<<BOTCHATTYPE::WebPageFlag))) {
		log.displayNL("This creature is not flagged as chat in web 0x%x", program);
		return false;
	}

	const string &wpn = creature->getWebPageName();
	(string &)wpn = args[1];

	log.displayNL("Set url name '%s'", creature->getWebPageName().c_str());

	return true;
}
