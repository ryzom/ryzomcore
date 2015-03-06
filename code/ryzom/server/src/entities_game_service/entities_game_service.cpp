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

/////////////
// INCLUDE //
/////////////
// misc
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/path.h"
#include "nel/misc/eid_translator.h"
#include "nel/misc/sstring.h"

//game share
#include "server_share/msg_object_player_manager.h"
#include "server_share/msg_brick_service.h"
#include "server_share/msg_ai_service.h"
#include "game_share/mode_and_behaviour.h"
#include "server_share/effect_message.h"
#include "server_share/pet_interface_msg.h"
//#include "game_share/chat_static_database.h"
#include "game_share/ryzom_version.h"
#include "game_share/mirror.h"
#include "server_share/effect_manager.h"
#include "game_share/time_weather_season/time_date_season_manager.h"
#include "game_share/time_weather_season/time_and_season.h"
#include "game_share/singleton_registry.h"
#include "game_share/file_description_container.h"
#include "game_share/dyn_chat.h"
#include "server_share/mail_forum_validator.h"
#include "game_share/mainland_summary.h"
#include "game_share/shard_names.h"
#include "server_share/handy_commands.h"

// egs
#include "game_item_manager/game_item_manager.h"
#include "game_item_manager/weapon_craft_parameters.h"
#include "client_messages.h"
#include "entities_game_service.h"
#include "admin.h"
#include "egs_dynamic_sheet_manager.h"

#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "player_manager/db_string_updater.h"
#include "player_manager/cdb_leaf.h"
#include "game_event_manager.h"
#include "egs_mirror.h"
#include "egs_globals.h"
//#include "shutdown_handler.h"

#include "phrase_manager/phrase_manager.h"
#include "phrase_manager/phrase_manager_callbacks.h"

#include "cdb_struct_banks.h"
#include "mission_manager/mission_manager.h"
#include "zone_manager.h"
#include "primitives_parser.h"
#include "team_manager/team_manager.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "player_manager/action_distance_checker.h"
#include "mission_manager/ai_alias_translator.h"
#include "shop_type/static_items.h"

#include "weapon_damage_table.h"
#include "phrase_manager/magic_phrase.h"

#include "harvest_source.h"
#include "weather_everywhere.h"

#include "player_manager/character_respawn_points.h"

#include "entity_matrix.h"
#include "common_shard_callbacks.h"
#include "building_manager/building_manager.h"
#include "guild_manager/fame_manager.h"
#include "phrase_manager/effect_factory.h"
#include "world_instances.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_manager/pvp_manager_2.h"
#include "pvp_manager/pvp_faction_hof.h"
#include "egs_pd.h"
#include "player_manager/character_name_extraction.h"
//#include "name_manager.h"
#include "shop_type/offline_character_command.h"
#include "player_manager/gm_tp_pending_command.h"
#include "shop_type/dynamic_items.h"
#include "dyn_chat_egs.h"
#include "shop_type/shop_type_manager.h"
#include "creature_manager/creature_manager.h"
#include "outpost_manager/outpost_manager.h"

#include "range_selector.h"
#include "shop_type/named_items.h"
//#include "team_manager/team_manager.h"
#include "dyn_chat_egs.h"
#include "mission_manager/mission_queue_manager.h"

#include "projectile_stats.h"
#include "progression/progression_pvp.h"

#include "stat_db.h"

#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"
#include "player_manager/player_manager.h"
#include "modules/r2_give_item.h"

#include "server_share/logger_service_client.h"
#include "server_share/stl_allocator_checker.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;

// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}

extern void cbClientReady(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId);
extern CVariable<bool> EGSLight;

// Local use variable
CVariable<uint32> MonkeyLoadEnable("egs","MonkeyLoadEnable","1 enabling fixed sequence monkey simulation, 2 enabling random sequence monkey simulation, 3 restart Monkey 2/ 0 disabling the Monkey Load simulation", 0, 0, true);
CVariable<uint32> LoadMax("egs","LoadMax", "Max Monkey player", 1000, 0, true );
CVariable<uint32> FirstUser("egs","FirstUser", "First user id used for Monkey player", 100000, 0, true );
CVariable<uint32> LastUser("egs","LastUser", "Last user id used for Monkey player", 199999, 0, true );

CVariable<uint32>	CharacterSavePerTick("egs", "CharacterSavePerTick", "Number of character file saved during the current tick", 0, 300);
CVariable<uint32>	CharacterLoadPerTick("egs", "CharacterLoadPerTick", "Number of character file loaded during the current tick", 0, 300);
uint32 CharacterSaveCounter = 0;
uint32 CharacterLoadCounter = 0;


//--------------------
// MACROS
//--------------------
//#define ADD_PROPERTY_MANAGER_RECEIVER(TYPE,var,value) \
//	{\
//		CPropertyManager * manager = new CSpecializedPropertyManager<TYPE>( var, value); \
//		CContainerPropertyReceiver::addPropertyManager( manager ); \
//	}
//

/////////////
// EXTERN
/////////////

// export deposit contents report
extern bool					ExportDepositContents;

namespace NLNET
{
	extern CVariable<bool>	FlushSendingQueuesOnExit;
};

/////////////
// GLOBALS //
/////////////
// Player service
CPlayerService * PS;

// database global bank
CCDBGroup					DbGroupGlobal;

// managers
CPlayerManager				PlayerManager;
CCreatureManager			CreatureManager;
CTeamManager				TeamManager;

std::string					StatPath;

// skill used when no weapon in hand (hand to hand combat)
SKILLS::ESkills				BarehandCombatSkill = SKILLS::SFMCAHM;

CGenericXmlMsgHeaderManager	GenericMsgManager;

sint32						BitPosAfterDatabaseMsgHeader = 0;

// chat DB
//CChatStaticDatabase			ChatDatabase;

bool						TTSIsUp = false;
bool						IOSIsUp = false;
bool						GPMSIsUp = false;
bool						PDSIsUp = false;

NLNET::TServiceId			FeService(0xff);

// max harvest distance
float						MaxHarvestDistance = 5.5f;

// max mount distance
float						MaxMountDistance = 2.f;

// the max value if Team members hp/sta/sap status window (represents 100% of max value)
uint8						TeamMembersStatusMaxValue = 127;

// Globals variables for the brick manager
uint8						EntityForcedDefaultLevel = 0;
float						DamageFactor = 1.0f;
float						SpeedFactor = 1.0f;

float						CombatCycleLength = 8.0f;
float						MaxEngageMeleeDistance = 20.0f;
float						MaxEngageRangeDistance = 100.0f;

// Item decay related values
float						CarriedItemsDecayRatio = 0.01f; // 1% of maxHp lot every 'rate' s
float						CarriedItemsDecayRate = 360.0; // rate of decay in seconds (default = 6 minutes (so 10%/hour))

bool						UnlimitedDeathPact = false;

// Hits Rate per ten secondes of weapons, rapid weapons = dagger category, medium weapon = sword category, slow weapon = two hands axe category
float						RapidWeaponHit = 5.0f;
float						MediumWeaponHit = 5.0f;
float						SlowWeaponHit = 5.0f;


// Transport class for begin / end of bot chat with player character
CCharacterBotChatBeginEnd	CharacterBotChatBeginEnd;
CCharacterDynChatBeginEnd	CharacterDynChatBeginEnd;
//*** Removed by Sadge ***
//// Transport class for EGS asking information about creatures/Npc
//CCreatureAskInformationMsg	CreatureNpcInformation;
//*** ***
//	Transport class sent to AIS to set bot mode as death.
CBSAIDeathReport			BotDeathReport;
// Transport class sent to AIS to set bot action flags
CChangeActionFlagMsg		ChangeActionFlagMsg;
// Transport class sent to AIS to despawn creatures
CCreatureDespawnMsg			CreatureDespawnMsg;

// For limit people / career accessible during beta
uint8						PeopleAutorisation = 2; //Matis
uint8						CareerAutorisation = 31; //MF + RF + CA + CB + CH

const CStaticXpFactorTable *XpFactorTable = NULL;
CFileDisplayer				EgsStatDisplayer;
CLog						EgsStat;

// Mainlands
vector<CMainlandSummary>	Mainlands;

// List of front-end service disconnections to process
vector<TServiceId>			DisconnectedFSes;

NLMISC_COMMAND( spawnFakePlayers, "Temp", "<nb>" )
{
	if ( args.size() == 0 )
		return false;
	uint nb;
	NLMISC::fromString(args[0], nb);
	sint32 minx=2236000, maxx=6000000, miny=-7000000, maxy=-1000000;

	log.displayNL( "Spawning %u fake players in (%d,%d)-(%d,%d)", nb, minx, miny, maxx, maxy );
	for ( uint i=0; i!=nb; ++i )
	{
		CEntityId entityId( RYZOMID::player, i, 0, NLNET::TServiceId8(IService::getInstance()->getServiceId()).get() );
		Mirror.createEntity( entityId );
		TDataSetRow row = TheDataset.getDataSetRow( entityId );
		CMirrorPropValue<sint32> x( TheDataset, row, DSPropertyPOSX );
		CMirrorPropValue<sint32> y( TheDataset, row, DSPropertyPOSY );
		x = (sint32)(frand( float( maxx-minx ) + (float)minx) );
		y = (sint32)(frand( float( maxy-miny ) + (float)miny) );
		TheDataset.declareEntity( row );
	}
	return true;
}


/// callback called when ther PDS string manager sent a string id for a unicode string
//void onPDSStringReadyCallback (const ucstring& str, uint32 id)
//{
//	if (  CGuildManager::getInstance()->updateGuildStringIds( str ) )
//		return;
//	/// todo guild remove that when player is in PDS, as strings will be managed entirely by PDS
//	if ( PlayerManager.setStringId(str, id) )
//		return;
//}

/*
 * Process the entity events from the mirror (note: the additions/removals done by EGS is not included)
 * (called by the mirror system).
 */
void processMirrorUpdates()
{
	TDataSetRow entityIndex;

	// Additions
	{
		H_AUTO(MirrorAdditions);
		TheDataset.beginAddedEntities();
		while ( (entityIndex = TheDataset.getNextAddedEntity()) != LAST_CHANGED )
		{
			const CEntityId& entityId = TheDataset.getEntityId( entityIndex );

			// Add creature/npc
			if ( entityId.getType() == RYZOMID::creature )
			{
				CCreature *creature = new CCreature();
				creature->setId( entityId );
				creature->addPropertiesToMirror( entityIndex, false ); // init properties + import the sheetid from the mirror
				creature->mirrorizeEntityState( false, entityIndex ); // import the position
				creature->loadSheetCreature( entityIndex );
				CreatureManager.addCreature( entityId, creature );
			}
			else if( entityId.getType() == RYZOMID::player )
			{
				//Mirror ask spawn an entity type player, but only egs can make that !
				nldebug("<processMirrorUpdates> Mirror ask spawn an entity type player, Id %s, RowId %s", entityId.toString().c_str(), entityIndex.toString().c_str());
//				nlstop;
			}
		}
		TheDataset.endAddedEntities();
	}

	// Removals
	{
		H_AUTO(MirrorRemovals);
		CEntityId *pEntityId;
		TheDataset.beginRemovedEntities();
		while ( (entityIndex = TheDataset.getNextRemovedEntity( &pEntityId )) != LAST_CHANGED )
		{
			if ( pEntityId->getType() == RYZOMID::npc || pEntityId->getType() == RYZOMID::creature )
			{
				// Clear the mirror target list (otherwise the list would never be unallocated
				// because the mirror of the AIS does not have access to this property)
				// See also cleanOrphanTargetLists().
				CMirrorPropValueList<uint32> targetList( TheDataset, entityIndex, DSPropertyTARGET_LIST );
				targetList.clear();

				CreatureManager.removeCreature( *pEntityId );
			}
		}
		TheDataset.endRemovedEntities();
	}

	// property changes
	{
		H_AUTO(MirrorPropertyChanges);

		// Z pos (for in water)
		TheDataset.beginChangedValuesForProp( DSPropertyPOSZ );
		while( (entityIndex = TheDataset.getNextChangedValueForProp()) != LAST_CHANGED )
		{
			CCharacter * user = PlayerManager.getChar( entityIndex );
			if (user)
			{
				const bool previous = user->isInWater();
				user->updateIsInWater();
				// if player is going in water, remove all stored Xp from it's attackers and set thier Hp to max value
				if ( ! previous && user->isInWater())
				{
					user->entersWater();
				}
			}
		}
		TheDataset.endChangedValuesForProp();
	}
}


/*
 * Despawn players connected on a down front-end service (called by tickUpdate for proper synchronization)
 */
void applyFSDisconnections()
{
	for ( vector<TServiceId>::const_iterator it=DisconnectedFSes.begin(); it!=DisconnectedFSes.end(); ++it )
	{
		PlayerManager.disconnectFrontend( *it );
	}
	DisconnectedFSes.clear();
}


class CItemDump
{
public:

	NLMISC::CSheetId	SheetId;
	uint32				StackSize;
	uint				Quality;
	uint				Class;
	float				StatEnergy;

	bool	operator == (const CItemDump& dump) const
	{
		return	SheetId == dump.SheetId &&
				StackSize == dump.StackSize &&
				Quality == dump.Quality &&
				Class == dump.Class &&
				StatEnergy == dump.StatEnergy;
	}
};

class CInventoryDump
{
public:

	std::vector<CItemDump>		Items;

	bool	operator == (const CInventoryDump& dump) const
	{
		return Items == dump.Items;
	}
};

class CCharacterDump
{
public:

	std::vector<CInventoryDump>	Inventories;

	bool	operator == (const CCharacterDump& dump) const
	{
		return Inventories == dump.Inventories;
	}
};



void	dumpCharacterContent(CCharacter* character, CCharacterDump& dump)
{
	dump.Inventories.resize(INVENTORIES::NUM_INVENTORY);

	uint	i;
	for (i=0; i<INVENTORIES::NUM_INVENTORY; ++i)
	{
		CInventoryPtr	inventory = character->_Inventory[i];

		if (inventory == NULL)
			continue;

		dump.Inventories[i].Items.resize(inventory->getSlotCount());

		uint	j;
		for (j=0; j<inventory->getSlotCount(); ++j)
		{
			CGameItemPtr	item = inventory->getItem(j);

			if (item == NULL)
				continue;

			dump.Inventories[i].Items[j].SheetId = item->getSheetId();
			dump.Inventories[i].Items[j].StackSize = item->getStackSize();
			dump.Inventories[i].Items[j].Quality = item->quality();
			dump.Inventories[i].Items[j].Class = (uint)item->getItemClass();
			dump.Inventories[i].Items[j].StatEnergy = item->getStatEnergy();
		}
	}
}

struct CPlayerCharId
{
	struct CChar
	{
		uint32	CharId;
		string	File;
		string	Backup;
	};

	uint32			PlayerId;
	vector<CChar>	Chars;
};

/*
 * loadAndResaveCheckCharacters
 */
bool loadAndResaveCheckCharacters( const std::vector<string>& files, NLMISC::CLog& log, bool restoreOriginal )
{
	bool result = true;
	map<uint32, CPlayerCharId>	players;

	log.displayNL("checking save files.");
	uint	i;
	for (i=0; i<files.size(); ++i)
	{
		uint	playerid, charid;
		if (sscanf(CFile::getFilename(files[i]).c_str(), "account_%d_%d_pdr.bin", &playerid, &charid) == 2 && CFile::getExtension(files[i]) == "bin")
		{
			CPlayerCharId	&id = players[playerid];

			id.PlayerId = playerid;
			CPlayerCharId::CChar	myChar;
			myChar.File = files[i];
			myChar.CharId = charid;
			id.Chars.push_back(myChar);
		}
		else
		{
			nlwarning("Unrecognised file name format: %s", files[i].c_str());
		}
	}

	map<uint32, CPlayerCharId>::iterator	it;
	for (it=players.begin(); it!=players.end(); ++it)
	{
		CPlayerCharId&	id = (*it).second;

		if (restoreOriginal)
		{
			// backup file
			for (i=0; i<id.Chars.size(); ++i)
			{
				id.Chars[i].Backup = id.Chars[i].File + ".tmp";
				CFile::copyFile(
					id.Chars[i].Backup,
					id.Chars[i].File);
			}
		}

		for (i=0; i<id.Chars.size(); ++i)
		{
			string	currentState;
			string	currentCommand;
			try
			{
				CCharacterDump	dump1, dump2;

				currentState = "First time";

				// load first time
				currentCommand = toString("loadPlayer %d", id.PlayerId);
				ICommand::execute(currentCommand, log);
				currentCommand = toString("activePlayer %d %d", id.PlayerId, id.Chars[i].CharId);
				ICommand::execute(currentCommand, log);
				currentCommand = toString("simulateClientReady %d %d", id.PlayerId, id.Chars[i].CharId);
				ICommand::execute(currentCommand, log);

				CCharacter*	character = PlayerManager.getChar( id.PlayerId, id.Chars[i].CharId );
				if (character != NULL)
					dumpCharacterContent(character, dump1);

				currentCommand = toString("disconnectPlayer %d", id.PlayerId);
				ICommand::execute(currentCommand, log);

				currentState = "Second time";

				// load second time
				currentCommand = toString("loadPlayer %d", id.PlayerId);
				ICommand::execute(currentCommand, log);
				currentCommand = toString("activePlayer %d %d", id.PlayerId, id.Chars[i].CharId);
				ICommand::execute(currentCommand, log);
				currentCommand = toString("simulateClientReady %d %d", id.PlayerId, id.Chars[i].CharId);
				ICommand::execute(currentCommand, log);

				character = PlayerManager.getChar( id.PlayerId, id.Chars[i].CharId );
				if (character != NULL)
					dumpCharacterContent(character, dump2);

				currentCommand = toString("disconnectPlayer %d", id.PlayerId);
				ICommand::execute(currentCommand, log);

				if (!(dump1 == dump2))
					log.displayNL("WARNING: character %d:%d changed after saving!", id.PlayerId, id.Chars[i].CharId );
			}
			catch (const Exception& e)
			{
				log.displayNL("Exception caught while executing command '%s' (%s): %s", currentCommand.c_str(), currentState.c_str(), e.what());
				result = false;
			}
			catch (...)
			{
				log.displayNL("Critical exception caught while executing command '%s' (%s)", currentState.c_str(), currentCommand.c_str());
				result = false;
			}
		}

		if (restoreOriginal)
		{
			// restore file and delete backup
			for (i=0; i<id.Chars.size(); ++i)
			{
				CFile::copyFile(
					id.Chars[i].File,
					id.Chars[i].Backup);
				CFile::deleteFile(id.Chars[i].Backup);
			}
		}
	}
	return result;
}


CVariable<string> FilenameListOfFilesToConvert( "backup", "FilenameListOfFilesToConvert", "The file should have one filename per line", "files_to_convert.txt", 0, true );
CVariable<uint> NbFilesToConvertPerTick( "backup", "NbFilesToConvertPerTick", "", 1, 0, true );
CVectorSString FilesToConvert;
bool FileConverterEnabled = true;


/*
 *
 */
void initConvertFile()
{
	// Load the list
	CSString listOfFiles;
	FileConverterEnabled = listOfFiles.readFromFile( CSString(FilenameListOfFilesToConvert.get()) );
	nlverify( listOfFiles.splitLines( FilesToConvert ) );
}


/*
 *
 */
void updateConvertFile()
{
	for (uint i=0; i<NbFilesToConvertPerTick.get(); ++i)
	{
		// Process one file
		if ( FilesToConvert.empty() )
		{
			FileConverterEnabled = false;
			return;
		}
		CSString nextFile = FilesToConvert.back();
		nlinfo( "*** Processing file to convert: %s ***", nextFile.c_str() );

		vector< string > files;
		files.push_back( nextFile );
		loadAndResaveCheckCharacters( files, *NLMISC::InfoLog, false );

		// Update the list
		FilesToConvert.pop_back();
		nlinfo( "*** %u files remaining to convert ***", FilesToConvert.size() );
		CSString newListOfFiles;
		newListOfFiles.join( FilesToConvert, "\n" );
		nlverify( newListOfFiles.writeToFile( CSString(FilenameListOfFilesToConvert.get()) ) );
	}
}



//---------------------------------------------------
// egsUpdate :
//
//---------------------------------------------------
void CPlayerService::egsUpdate()
{
	// reset the load/save char/tick counter
	CharacterSavePerTick = CharacterSaveCounter;
	CharacterSaveCounter = 0;
	CharacterLoadPerTick = CharacterLoadCounter;
	CharacterLoadCounter = 0;


	H_AUTO(egsUpdate)
	if ( ! Mirror.mirrorIsReady() )
		return;

	// Init dynamic item in sell store
	if (CDynamicItems::getInstance()->isInit() == false)
		CDynamicItems::getInstance()->init();

	// Init game event manager
	if (CGameEventManager::getInstance().isInit() == false)
		CGameEventManager::getInstance().init();

	// Init mission queue manager
	if (CMissionQueueManager::getInstance()->isInit() == false)
		CMissionQueueManager::getInstance()->init();

	// Init pvp faction reward manager
	if (CPVPFactionRewardManager::getInstance().isInit() == false)
		CPVPFactionRewardManager::getInstance().init();

	if( MonkeyLoadEnable > 0)
		egsLoadMonkey();

	if( FileConverterEnabled )
		updateConvertFile();

	// tmp nico : update projectiles stats
	projStatsTime(NLMISC::CTime::getLocalTime());

	STL_ALLOC_TEST
	{
		H_AUTO(EGSPD_update)
		EGSPD::update();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(applyFSDisconnections)
		applyFSDisconnections();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CSingletonRegistry_tickUpdate)
		CSingletonRegistry::getInstance()->tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CTimeDateSeasonManager_tickUpdate)
		CTimeDateSeasonManager::tickUpdate(); //RyzomTime.updateRyzomClock( CTickEventHandler::getGameCycle() );
	}
	STL_ALLOC_TEST

	PlayerManager.tickUpdate();
	STL_ALLOC_TEST
//	CreatureManager.tickUpdate();
	{
		H_AUTO(TeamManager_update)
		TeamManager.update();
	}
	STL_ALLOC_TEST
	CPhraseManager::getInstance().updatePhrases();
	STL_ALLOC_TEST
	CEffectManager::update();
	STL_ALLOC_TEST

	CActionDistanceChecker::getInstance()->tickUpdate();
	{
		H_AUTO(CHarvestSourceManager_tickUpdate)
		CHarvestSourceManager::getInstance()->tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CEnvironmentalEffectManager_tickUpdate)
		CEnvironmentalEffectManager::getInstance()->tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CZoneManager_tickUpdate)
		CZoneManager::getInstance().tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CMissionManager_tickUpdate)
		CMissionManager::getInstance()->tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CFameManager_tickUpdate)
		CFameManager::getInstance().tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CPVPManager_tickUpdate)
		CPVPManager2::getInstance()->tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CPVPFaction_tickUpdate)
		CPVPFactionHOF::getInstance()->tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CGuildManager_clientDBUpdate)
		CGuildManager::getInstance()->clientDBUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(COutpostManager_tickUpdate)
		COutpostManager::getInstance().tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CMissionQueueManager_tickUpdate)
		CMissionQueueManager::getInstance()->tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(CPVPFactionRewardManager_tickUpdate)
		CPVPFactionRewardManager::getInstance().tickUpdate();
	}
	STL_ALLOC_TEST
	{
		H_AUTO(DbGroupGlobal_update)
		DbGroupGlobal.sendDeltas(~0, CCDBGroup::SendDeltasToAll);
	}
	STL_ALLOC_TEST

	CDynamicItems::getInstance()->tickUpdate();
	STL_ALLOC_TEST
	// check file changes every 5 sec.
	// that can be a source of lag, disable until we can check that, anyway others service not use that
//	CFile::checkFileChange(5000);

	// bot chat begin / end
	if ( (! CharacterBotChatBeginEnd.BotChatStart.empty()) || (! CharacterBotChatBeginEnd.BotChatEnd.empty()) )
	{
		// NB : this message is broadcasted to all AIS
		CharacterBotChatBeginEnd.send( "AIS" );
		CharacterBotChatBeginEnd.BotChatStart.clear();
		CharacterBotChatBeginEnd.BotChatEnd.clear();
	}
	STL_ALLOC_TEST

	// dyn chat begin / end
	if ( (! CharacterDynChatBeginEnd.DynChatStart.empty()) || (! CharacterDynChatBeginEnd.DynChatEnd.empty()) )
	{
		// NB : this message is broadcasted to all AIS
		CharacterDynChatBeginEnd.send( "AIS" );
		CharacterDynChatBeginEnd.DynChatStart.clear();
		CharacterDynChatBeginEnd.DynChatEnd.clear();
	}
	STL_ALLOC_TEST

	{
		H_AUTO(BotDeathReport)
			// bot death.
		if	( !BotDeathReport.Bots.empty()	)
		{
			nlassert(BotDeathReport.Zombies.size() == BotDeathReport.Bots.size());
			nlassert(BotDeathReport.Killers.size() == BotDeathReport.Bots.size());

			// NB : this message is broadcasted to all AIS
			BotDeathReport.send("AIS");
	//#ifdef NL_DEBUG
			// looking for ZOMBIE bug
			for (uint i = 0 ; i < BotDeathReport.Bots.size() ; ++i)
			{
				CCreature *creature = CreatureManager.getCreature(BotDeathReport.Bots[i]);
				if (creature)
				{
					creature->deathReportSent();
					if (BotDeathReport.Zombies[i])
					{
						nlinfo("ZOMBIE : send again death report for zombie creature %s to AIS", creature->getId().toString().c_str());
					}
				}
				else
				{
					nlwarning("ZOMBIE : sending death report for entity rowId %u but cannot find CCreature object",(uint32)BotDeathReport.Bots[i].getIndex());
				}
			}
	//#endif
			BotDeathReport.Bots.clear();
			BotDeathReport.Killers.clear();
			BotDeathReport.Zombies.clear();
		}
	}
	STL_ALLOC_TEST

	// action flags change
	if ( !ChangeActionFlagMsg.Entities.empty() )
	{
		ChangeActionFlagMsg.send("AIS");
		ChangeActionFlagMsg.Entities.clear();
		ChangeActionFlagMsg.ActionFlags.clear();
		ChangeActionFlagMsg.Values.clear();
	}
	STL_ALLOC_TEST

	// despawn creatures
	if (!CreatureDespawnMsg.Entities.empty())
	{
		CreatureDespawnMsg.send("AIS");
		CreatureDespawnMsg.Entities.clear();
	}
	STL_ALLOC_TEST
	// check shard must be shut down
//	{
//		H_AUTO(CShutdownHandler_update)
//		CShutdownHandler::update();
//		CAutomaticShutdownHandler::update();
//	}

	{
		H_AUTO(CStatDB_tickUpdate);
		CStatDB::getInstance()->tickUpdate();
	}
	STL_ALLOC_TEST

	{
		H_AUTO(CCharacterProgressionPVP_tickUpdate);
		PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->tickUpdate();
	}
	STL_ALLOC_TEST

} // egsUpdate //


//---------------------------------------------------
// egs load monkey
//
//---------------------------------------------------
void CPlayerService::egsLoadMonkey()
{
	if( PlayerManager.getNumberPlayers() < LoadMax || MonkeyLoadEnable >= 2 )
	{
		// add a random player
		egsAddMonkeyPlayer();
	}
	else
	{
		removeMonkeyPlayer();
	}
}

//----------------------------------------------------
void CPlayerService::egsAddMonkeyPlayer()
{
	static std::vector<uint32> RandomUserSequence;
	static uint32 idx = 0;
	static uint32 maxLoop = 0;

	static uint32 nextUserId = (uint32) ((rand() * (LastUser - FirstUser) / RAND_MAX ) + FirstUser );

	uint32 userId;

	if( MonkeyLoadEnable == 3 )
	{
		if( PlayerManager.getNumberPlayers() == 0 )
		{
			idx = 0;
			MonkeyLoadEnable = 2;
		}
	}

	if( MonkeyLoadEnable == 2 && LoadMax != RandomUserSequence.size() )
	{
		RandomUserSequence.resize(LoadMax);
		for( uint32 i = 0; i < LoadMax; ++i )
		{
			RandomUserSequence[ i ] = (uint32)~0;
		}
		idx = 0;
	}

	if( MonkeyLoadEnable >= 2 && idx >= RandomUserSequence.size() )
	{
		if( PlayerManager.getNumberPlayers() == 0 )
			return;
		else
			removeMonkeyPlayer();
		return;
	}

	if( MonkeyLoadEnable == 1 )
	{
		if( nextUserId > LastUser )
			nextUserId = FirstUser;
		userId = nextUserId;
		++nextUserId;
	}
	else
	{
		if( idx < RandomUserSequence.size() )
		{
			if( RandomUserSequence[ idx ] == (uint32)~0 )
			{
				if( nextUserId > LastUser )
					nextUserId = FirstUser;
				userId = nextUserId;
				++nextUserId;
				if( PlayerManager.getPlayer( userId ) != 0 )
					return;
				RandomUserSequence[ idx ] = userId;
			}
			else
			{
				userId = RandomUserSequence[ idx ];
			}
			idx++;
		}
	}

	if( PlayerManager.getPlayer( userId ) != 0 )
		return;

	// set the front end id of this player
	PlayerManager.setPlayerFrontEndId( userId, NLNET::TServiceId(128) );

	CPlayer *player = new CPlayer;
	player->setId( userId );
	// add player then load (must do the add before else assert in CCharacter::postLoadTreatment())
	PlayerManager.addPlayer( userId, player );

	if (!UseAsyncBSPlayerLoading.get())
	{
		// load character of player
		PlayerManager.loadPlayer( player );

		// set status connexion of player to connected
		player->setPlayerConnection( true );
	}
	else
	{
		string languageId = string("FR");
		CLoginCookie	cookie;
		bool AllAutorized = true;
		PlayerManager.asyncLoadPlayer(player, userId, languageId, cookie, AllAutorized);
	}
}

//----------------------------------------------------
void CPlayerService::egsAddMonkeyPlayerCallback(uint32 userId)
{
	//activate character
	CCharacter * ch = PlayerManager.getChar( userId, 0);
	if( ch )
	{
		PlayerManager.setActiveCharForPlayer( userId, 0, ch->getId() );

		// simulate client ready
		CEntityId characterId = ch->getId();
		NLNET::CMessage msg( "CLIENT:RDY" );
		msg.serial( characterId );
		msg.invert();
		cbClientReady(msg, "", TServiceId(0));
	}
}

//----------------------------------------------------
void CPlayerService::removeMonkeyPlayer()
{
	uint32 playerNumber = PlayerManager.getNumberPlayers();
	if( playerNumber != 0 )
	{
		playerNumber = (uint32) (rand() * (playerNumber-1) / RAND_MAX);
		uint32 i = 0;
		CPlayerManager::TMapPlayers::const_iterator it;
		for( it = PlayerManager.getPlayers().begin(); playerNumber != i ; ++it )
			++i;
		if( (*it).second.Player != 0 )
		{
			if( (*it).second.Player->getActiveCharacter() != 0 )
			{
				if( (*it).second.Player->getActiveCharacter()->getEnterFlag() )
				{
					PlayerManager.disconnectPlayer( (*it).second.Player->getUserId() );
				}
			}
		}
	}
}

//---------------------------------------------------
// egsSync :
//
//---------------------------------------------------
void CPlayerService::egsSync()
{
} // egsSync //

//#include <crtdbg.h>

//---------------------------------------------------
// update regen due to variable change in cfg :
//
//---------------------------------------------------
void CPlayerService::updateRegen(IVariable &var)
{
	PlayerManager.updateRegen();
}

//---------------------------------------------------
// cfgSkillProgressionFactorCB :
// CallBack to change the Skill Progression Factor.
//---------------------------------------------------
float SkillProgressionFactor = 1.0;
void cfgSkillProgressionFactorCB( CConfigFile::CVar& var )
{
	SkillProgressionFactor = var.asFloat();
	nlinfo( "EGS_CFG: SkillProgressionFactor is now : '%f'.", SkillProgressionFactor );
}// cfgSkillProgressionFactorCB //

//---------------------------------------------------
// cfgDefaultCastingTimeCB :
// CallBack to change the default casting time.
//---------------------------------------------------
void cfgDefaultCastingTimeCB( CConfigFile::CVar& var )
{
	// Change the default casting time.
	CMagicPhrase::defaultCastingTime(var.asFloat());
	nlinfo( "EGS_CFG: Default Casting Time is now : '%f'.", CMagicPhrase::defaultCastingTime() );
}// cfgDefaultCastingTimeCB //

//---------------------------------------------------
// cfgClientCommandsPrivilegesFileCB :
// CallBack to change the client commands privileges file.
//---------------------------------------------------
void cfgClientCommandsPrivilegesFileCB(CConfigFile::CVar & var)
{
	string fileName = var.asString();
	try
	{
		fileName = CPath::lookup(fileName);
		initCommandsPrivileges(fileName);
	}
	catch (const EPathNotFound & e)
	{
		nlwarning("ADMIN: %s", e.what());
	}
	nlinfo("EGS_CFG: ClientCommandsPrivilegesFile is now : '%s'.", fileName.c_str());

}// cfgClientCommandsPrivilegesFileCB //


//---------------------------------------------------
// initConfigFileVars :
//
//---------------------------------------------------
void CPlayerService::initConfigFileVars()
{
//	try
//	{
//		CConfigFile::CVar& cvDecay = ConfigFile.getVar("DecayDelay");
//		GameItemManager.DecayDelay = cvDecay.asInt() * 60 * 10;
//	}
//	catch(const EUnknownVar &)
//	{
//		//nlwarning("<CPlayerService> var DecayDelay not found");
//	}

//	try
//	{
//		CConfigFile::CVar& cvCorpseToCarrion = ConfigFile.getVar("CorpseToCarrionDelay");
//		GameItemManager.CorpseToCarrionDelay = cvCorpseToCarrion.asInt() * 60 * 10;
//	}
//	catch(const EUnknownVar &)
//	{
//		//nlwarning("<CPlayerService> var CorpseToCarrionDelay not found");
//	}

//	try
//	{
//		CConfigFile::CVar& cvCarrionDecay = ConfigFile.getVar("CarrionDecayDelay");
//		GameItemManager.CarrionDecayDelay = cvCarrionDecay.asInt() * 60 * 10;
//	}
//	catch(const EUnknownVar &)
//	{
//		//nlwarning("<CPlayerService> var CarrionDecayDelay not found");
//	}

//	try
//	{
//		CConfigFile::CVar& cvCorpse = ConfigFile.getVar("CorpseMaxCount");
//		GameItemManager.CorpseMaxCount = cvCorpse.asInt();
//	}
//	catch(const EUnknownVar &)
//	{
//		//nlwarning("<CPlayerService> var CorpseMaxCount not found");
//	}

	// init the DamageFactor
	if (ConfigFile.getVarPtr("DamageFactor") != NULL)
		DamageFactor = ConfigFile.getVar("DamageFactor").asFloat();
	else
		DamageFactor = 1.0f;

	// init the SpeedFactor
	if (ConfigFile.getVarPtr("SpeedFactor") != NULL)
		SpeedFactor = ConfigFile.getVar("SpeedFactor").asFloat();
	else
		SpeedFactor = 1.0f;

	// init the CombatCycleLength
	if (ConfigFile.getVarPtr("CombatCycleLength") != NULL)
		CombatCycleLength = ConfigFile.getVar("CombatCycleLength").asFloat();
	else
		CombatCycleLength = 8.0f;


//	// init chat static database
//	string chatDbFilename;
//	if (ConfigFile.getVarPtr("ChatStaticDBFilename") != NULL)
//		chatDbFilename = ConfigFile.getVar("ChatStaticDBFilename").asString();
//	else
//		chatDbFilename = "chat_static.cdb";
//
//	const string path = CPath::lookup( chatDbFilename );
//	ChatDatabase.load( path );

	if (ConfigFile.getVarPtr("UnlimitedDeathPact") != NULL)
	{
		UnlimitedDeathPact = ConfigFile.getVar("UnlimitedDeathPact").asInt() != 0;
		nlinfo("Forced Death pact to unlimited number");
	}
	else
		UnlimitedDeathPact = false;

	string positionFlagsFileName;
	if (ConfigFile.getVarPtr("PositionFlagsFile") != NULL)
		positionFlagsFileName = ConfigFile.getVar("PositionFlagsFile").asString();
	else
		positionFlagsFileName = "position_flags.xml";
	initPositionFlags(positionFlagsFileName);

	CConfigFile::CVar *varMaxNbPlayers = ConfigFile.getVarPtr("NbPlayersLimit");
	if ( varMaxNbPlayers )
		MaxNbPlayers = varMaxNbPlayers->asInt();
	else
		MaxNbPlayers = 300;

	CConfigFile::CVar *varMaxNbGuilds = ConfigFile.getVarPtr("NbGuildLimit");
	if ( varMaxNbGuilds )
		MaxNbGuilds = varMaxNbGuilds->asInt();
	else
		MaxNbGuilds = 1000;

	CConfigFile::CVar *varMaxNbObjects = ConfigFile.getVarPtr("NbObjectsLimit");
	if ( varMaxNbPlayers )
		MaxNbObjects= varMaxNbPlayers->asInt();
	else
		MaxNbObjects = 1000;
	CConfigFile::CVar *varMaxNbNpcSpawnedByEGS = ConfigFile.getVarPtr("NbNpcSpawnedByEGSLimit");
	if ( varMaxNbPlayers )
		MaxNbNpcSpawnedByEGS = varMaxNbPlayers->asInt();
	else
		MaxNbNpcSpawnedByEGS = 50;
	CConfigFile::CVar *varMaxNbForageSources = ConfigFile.getVarPtr("NbForageSourcesLimit");
	if ( varMaxNbForageSources )
		MaxNbForageSources = varMaxNbForageSources->asInt();
	else
		MaxNbForageSources = 2000;
	CConfigFile::CVar *varMaxNbToxicClouds = ConfigFile.getVarPtr("NbToxicCloudsLimit");
	if ( varMaxNbToxicClouds )
		MaxNbToxicClouds = varMaxNbToxicClouds->asInt();
	else
		MaxNbToxicClouds = 1000;
	nlinfo( "NbPlayersLimit=%u NbObjectsLimit=%u NbNpcSpawnedByEGSLimit=%u NbForageSourcesLimit=%u NbToxicCloudsLimit=%u NbGuildLimit=%u", MaxNbPlayers, MaxNbObjects, MaxNbNpcSpawnedByEGS, MaxNbForageSources, MaxNbToxicClouds, MaxNbGuilds );

	CConfigFile::CVar *varExportDepositContents = ConfigFile.getVarPtr("ExportDepositContents");
	if ( varExportDepositContents )
	{
		nlinfo( "Deposit contents report will be exported" );
		ExportDepositContents = (varExportDepositContents->asInt() == 1);
	}

	// available mainlands
	try
	{
		CConfigFile::CVar& mainlands = ConfigFile.getVar("Mainlands");
		sint mlsz = mainlands.size();
		if( mlsz%4 == 0 && mlsz>=4 )
		{
			for ( uint i = 0; (sint)i<mainlands.size(); i+=4 )
			{
				CMainlandSummary mlSm;
				uint32 sessionId;
				NLMISC::fromString(mainlands.asString(i), sessionId);
				mlSm.Id = TSessionId(sessionId);
				mlSm.Name = ucstring::makeFromUtf8(mainlands.asString(i+1));
				mlSm.Description = ucstring::makeFromUtf8(mainlands.asString(i+2));
				mlSm.LanguageCode = mainlands.asString(i+3);
				Mainlands.push_back( mlSm );
			}
		}
		else
		{
			nlwarning("<CPlayerService::initConfigFileVars> bad size for var Mainlands : %d",mlsz);
		}
	}
	catch(const EUnknownVar &)
	{
		nlwarning("<CPlayerService::initConfigFileVars> var Mainlands not found");
	}

	// Available HomeMainLands names
	CShardNames::getInstance().init(ConfigFile);

} // initConfigFileVars //


//---------------------------------------------------
// init :
//
//---------------------------------------------------
void CPlayerService::init()
{
	// activate logs
	LGS::ILoggerServiceClient::startLoggerComm();
	// a little boolean set to true if we are just packing sheets and then exitting, allowing us to skip stuff that we don't really need to do
	bool packingSheets= haveArg('Q');

	setVersion (RYZOM_VERSION);

	StatPath = "data_shard_local/statistics/egs_stat.log";
	CFile::createDirectory("data_shard_local");
	CFile::createDirectory("data_shard_local/statistics");

	EgsStatDisplayer.setParam(StatPath);
	EgsStat.addDisplayer(&EgsStatDisplayer);

	// Init CSheetId
	CSheetId::init(0);

    // Init singleton manager
	CSingletonRegistry::getInstance()->init();

	// register the NLLIGO classes
	NLLIGO::Register();

	// keep pointer on class
	PS = this;

	TTSIsUp = false;

	initAdmin ();

	//uint16 i = SKILLS::NUM_SKILLS;

	// Init mirror
	initMirror( egsUpdate, egsSync );

//	BSUTILITIES::loadLocalisationTable( CPath::lookup("localisation.localisation_table" ) );

	CUnifiedNetwork::getInstance()->setServiceUpCallback( "*", cbConnection, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "*", cbDisconnection, 0);
	Mirror.setServiceMirrorUpCallback("*", cbMirrorUp, 0);
	Mirror.setServiceMirrorDownCallback("*", cbMirrorDn, 0);

	// register the shared class
	TRANSPORT_CLASS_REGISTER (CDelHandledAIGroupMsg);
	TRANSPORT_CLASS_REGISTER (CAddHandledAIGroupMsg);
	TRANSPORT_CLASS_REGISTER (CHandledAIGroupSpawnedMsgImp);
	TRANSPORT_CLASS_REGISTER (CHandledAIGroupDespawnedMsgImp);

	TRANSPORT_CLASS_REGISTER (CCreatureDespawnMsg);
	TRANSPORT_CLASS_REGISTER (CEGSExecuteMsgImp);
	TRANSPORT_CLASS_REGISTER (CBSAIEventReportMsg);
	TRANSPORT_CLASS_REGISTER (CEGSExecutePhraseMsg);
	TRANSPORT_CLASS_REGISTER (CEGSExecuteAiActionMsgImp);

//	TRANSPORT_CLASS_REGISTER (CNpcBotDescriptionImp);
	TRANSPORT_CLASS_REGISTER (CCharacterBotChatBeginEnd);
	TRANSPORT_CLASS_REGISTER (CCharacterDynChatBeginEnd);

//	TRANSPORT_CLASS_REGISTER ( CDynMissionDescMsgImp );
//	TRANSPORT_CLASS_REGISTER ( CDynMissionRequestMsg );

	TRANSPORT_CLASS_REGISTER (CAddEffectsMessage);
	TRANSPORT_CLASS_REGISTER (CRemoveEffectsMessage);

	TRANSPORT_CLASS_REGISTER (CCAisActionMsgImp);
	TRANSPORT_CLASS_REGISTER (CAITauntMsg);
	TRANSPORT_CLASS_REGISTER (CAICalmCreatureMsg);
	TRANSPORT_CLASS_REGISTER (CUserEventMsg);
	TRANSPORT_CLASS_REGISTER (CFaunaBotDescriptionImp);
	TRANSPORT_CLASS_REGISTER (CSetEscortTeamId);

	TRANSPORT_CLASS_REGISTER (CPetSpawnMsg);
	TRANSPORT_CLASS_REGISTER (CPetSpawnConfirmationImp);
	TRANSPORT_CLASS_REGISTER (CPetCommandMsg);

	TRANSPORT_CLASS_REGISTER (CBSAIDeathReport);
	TRANSPORT_CLASS_REGISTER (CAILostAggroMsgImp);
	TRANSPORT_CLASS_REGISTER (CAIGainAggroMsgImp);

	TRANSPORT_CLASS_REGISTER (CSetBotHeadingMsg);
	TRANSPORT_CLASS_REGISTER (CAIPlayerRespawnMsg);

	TRANSPORT_CLASS_REGISTER (CAIAskForInfosOnEntityMsg);
	TRANSPORT_CLASS_REGISTER (CAIInfosOnEntityMsgImp);
	TRANSPORT_CLASS_REGISTER (CEnableAggroOnPlayerMsg);
	TRANSPORT_CLASS_REGISTER (CPetSetOwner);
	TRANSPORT_CLASS_REGISTER (CChangeActionFlagMsg);
	TRANSPORT_CLASS_REGISTER (CReportAICollisionAvailableMsgImp);
	TRANSPORT_CLASS_REGISTER (CReportStaticAIInstanceMsgImp);
	TRANSPORT_CLASS_REGISTER (CReportAIInstanceDespawnMsgImp);
	TRANSPORT_CLASS_REGISTER (CWarnBadInstanceMsg);
	TRANSPORT_CLASS_REGISTER (CCreatureCompleteHealImp);
	TRANSPORT_CLASS_REGISTER (CChangeCreatureMaxHPImp);
	TRANSPORT_CLASS_REGISTER (CChangeCreatureHPImp);
	TRANSPORT_CLASS_REGISTER (CCreatureSetUrlImp);
	TRANSPORT_CLASS_REGISTER (CChangeCreatureModeMsg);
	TRANSPORT_CLASS_REGISTER (CGiveItemRequestMsgImp);
	TRANSPORT_CLASS_REGISTER (CReceiveItemRequestMsgImp);
	TRANSPORT_CLASS_REGISTER (CQueryEgsImp);



	// add generated class callback
	NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(GenNpcDescCbTable, 1);

//	// init shutdown management
//	CShutdownHandler::init();

	// add common callbacks
	CCommonShardCallbacks::init();

	// init pd lib ( set the factory first )
	RY_PDS::IPDBaseData* missionFactoryPDSolo();
	RY_PDS::IPDBaseData* missionFactoryPDGroup();
	RY_PDS::IPDBaseData* missionFactoryPDGuild();

	EGSPD::CMissionPD::setFactory(CMissionManager::missionFactoryPD);
	EGSPD::CMissionSoloPD::setFactory(CMissionManager::missionFactoryPDSolo);
	EGSPD::CMissionTeamPD::setFactory(CMissionManager::missionFactoryPDTeam);
	EGSPD::CMissionGuildPD::setFactory(CMissionManager::missionFactoryPDGuild);
	EGSPD::CGuildMemberPD::setFactory(CGuildManager::guildMemberFactoryPD);
	EGSPD::CGuildPD::setFactory(CGuildManager::guildFactoryPD);
	if (!packingSheets) EGSPD::init(0);
//	EGSPD::PDSLib.getStringManager().setCallback( onPDSStringReadyCallback );

	// load the sart position and other parameters
	initConfigFileVars();
	// Add callback to change the progression factor without to stop the service.
	ConfigFile.setCallback("SkillProgressionFactor", cfgSkillProgressionFactorCB);
	cfgSkillProgressionFactorCB( ConfigFile.getVar( "SkillProgressionFactor" ) );
	// Add callback to change the default casting time without to stop the service.
	ConfigFile.setCallback("DefaultCastingTime", cfgDefaultCastingTimeCB);
	cfgDefaultCastingTimeCB( ConfigFile.getVar( "DefaultCastingTime" ) );
	// Add callback to change the client commands privileges file
	ConfigFile.setCallback("ClientCommandsPrivilegesFile", cfgClientCommandsPrivilegesFileCB);
	cfgClientCommandsPrivilegesFileCB( ConfigFile.getVar("ClientCommandsPrivilegesFile") );
	if ( ! FlushSendingQueuesOnExit.get() )
		nlwarning( "FlushSendingQueuesOnExit is OFF => some data may remain unsaved on exit" );

	if (IService::getInstance()->haveArg('S'))
	{
		uint32 shardId;
		NLMISC::fromString(IService::getInstance()->getArg('S'), shardId);
		IService::getInstance()->anticipateShardId(shardId);
	}
	else if (ConfigFile.getVarPtr("ShardId") != NULL)
	{
		IService::getInstance()->anticipateShardId( ConfigFile.getVarPtr("ShardId")->asInt());
	}
	// Connect to MFS (don't wait for guild loading callback as addService() can't be called in callbacks for now)
	CMailForumValidator::init();
	// load SDB
	if (!packingSheets)
		CStatDB::getInstance()->load();

	// init entity world matrix patterns
	CEntityMatrixPatternSymetrical::initMatrixPatterns();

	// Init database
	nlinfo( "Loading database..." );
	CCDBStructBanks::init( CPath::lookup("database.xml") );

	// Database group init
//	DbGroupGlobal.init( CDBGlobal );

// TEMP : DEBUG CODE
// check the GUILD:DESCRIPTION entry
	ICDBStructNode *node = CCDBStructBanks::instance()->getICDBStructNodeFromName(CDBGuild, "GUILD:NAME");
	nlassert(node != NULL);
	CCDBStructNodeLeaf *nodeLeaf = dynamic_cast<CCDBStructNodeLeaf *>(node);

nlassert(nodeLeaf->getType() == ICDBStructNode::TEXT);


	//init distance checker
	CActionDistanceChecker::init();

	// get and init the phrase manage
	CPhraseManager::getInstance().init();

	//instanciate CDynamicSheetManager
	CDynamicSheetManager::getInstance();

	// Init Sheets manager
	CSheets::init();
	//CCharacter::initMountInventoryBulkMax(); // must be called after CSheets::init()
	// Init item manager
//	GameItemManager.init();
	// Init build static items for sell
	if (!packingSheets) CStaticItems::buildStaticItem();
	// init models of npc specific items
	if (!packingSheets) CGameItemManager::buildNpcSpecificItems();
	// Init shop category
	if (!packingSheets) CShopTypeManager::initShopBase();
//	// Init dynamic item in sell store
//	if (!packingSheets) CDynamicItems::getInstance()->init();
	// init success tables
	CStaticSuccessTable::initTables();
	// init XpFactor Table
	CSheetId xpFactorSheet("default.action_xp_factor");
	XpFactorTable = CSheets::getXpFactorTableForm( xpFactorSheet );
	nlassert( XpFactorTable );

	// init player manager data
	if (!packingSheets) PlayerManager.init();
	// Add commons callbacks for all entities type
	if (!packingSheets) (( CEntityBaseManager& ) PlayerManager).addEntityCallback();
	// Add callback for client & characters management
	if (!packingSheets) PlayerManager.addClientCallback();

	///init the primitive parser
	if (!packingSheets) CPrimitivesParser::getInstance().init();
	///init the AI Id translator
	if (!packingSheets) CAIAliasTranslator::init();
	// init PVP subsystem
	if (!packingSheets) CPVPManager::init();
	if (!packingSheets) CPVPManager2::init();
	if (!packingSheets) CPVPFactionHOF::init();
	///init the zone manager
	if (!packingSheets) CZoneManager::getInstance().init();
	/// Init Time and season manager
	if (!packingSheets) CTimeDateSeasonManager::init();
	/// Init weather look-up helper
	if (!packingSheets) WeatherEverywhere.init();

	/// for init of the named items
	if (!packingSheets) CNamedItems::getInstance();

	if (!packingSheets) CBuildingManager::init();
	if (!packingSheets) CMissionManager::init();

	if (!packingSheets) CMissionQueueManager::getInstance()->init();
	CConfigFile::CVar *varPtr = IService::getInstance()->ConfigFile.getVarPtr("ParseMissionsOnly");
	if ( varPtr && varPtr->asInt() != 0)
		IService::getInstance()->exit();

	// init guild manager
	// WARNING: guilds will be loaded later (see guildManagerLoadCallback in guild_manager.cpp)
	if (!packingSheets) CGuildManager::init();

	if (!packingSheets) CPVPManager::getInstance()->initPVPIslands();

	// Add callback for npc and creature management
	if (!packingSheets) CreatureManager.addCreatureCallback();

	// Init generic msg manager
	nlinfo( "Loading generic message manager..." );
	const string pathMsg = CPath::lookup( "msg.xml" );
	if (!packingSheets) GenericMsgManager.init( pathMsg );

	nlinfo( "Loading the remaining..." );

	RandomGenerator.srand( (uint) CTime::getLocalTime() );
	srand( (uint) CTime::getLocalTime() );

	// init messages from IOS
	if (!packingSheets) CClientMessages::init();

	// init weapon damage table
	if (!packingSheets) CWeaponDamageTable::getInstance().init();

	// init gm tp command
	if (!packingSheets) CGmTpPendingCommand::getInstance();

	//nlassert(_CrtCheckMemory());

	// load and do check coherency of CEntityIdTranslator
	bool needCheckCoherency = false;
	if(ConfigFile.exists("CheckEntityIdTranslatorCoherency"))
	{
		needCheckCoherency = (ConfigFile.getVar("CheckEntityIdTranslatorCoherency").asInt() == 1);
	}

	EgsStat.displayNL("Start Stat Log");

	// Init the world instance callbacks
	if (!packingSheets) CWorldInstances::instance().registerAiInstanceReadyCallback(this);

	// load PDS strings. DEPECATED : done during lib init
//	EGSPD::PDSLib.getStringManager().load();

	if (!packingSheets) DynChatEGS.init();

	// load outposts from primitives here (guilds are not loaded yet)
	nlinfo("Loading outposts from primitives...");
	if (!packingSheets) COutpostManager::getInstance().loadOutpostPrimitives();

	if ( FileConverterEnabled )
		initConvertFile();


	// create the ring editor/animator channels
	if (IsRingShard)
	{
		const CShardNames::TSessionNames &shardNames = CShardNames::getInstance().getSessionNames();

		for (uint i=0; i<shardNames.size(); ++i)
		{
			const CShardNames::TSessionName &sn = shardNames[i];

			// create an unified dynamic channel with a well known chanId
			DynChatEGS.addLocalizedChan("RING_"+toUpper(sn.ShortName), CEntityId(RYZOMID::dynChatGroup, RingDynChanOffset+sn.SessionId.asInt()), false, false, true);
		}
	}

	setCurrentStatus("WaitingMirrorReady");
} // init //


//-----------------------------------------------------------------------------
// NLMISC_COMMAND (reportMemoryLeak, "report memory leak", "")
// {
// 	NLMEMORY::ReportMemoryLeak ();
// 	return true;
// }


//-----------------------------------------------------------------------------
// NLMISC_COMMAND (statisticsReport, "Report static usage of memory", "<filanme.csv> <dump>")
// {
// 	if( args.size() == 0 )
// 		return false;
// 
// 	NLMEMORY::StatisticsReport( args[0].c_str(), args.size() > 1 );
// 	return true;
//}


NLMISC_COMMAND(loadAndReSaveCharacters,"load and resave the complete set of player characters","")
{
	H_AUTO(LoadAndReSaveCharacters);
	nlinfo ("Loading and re-saving player character save game files");
	set<uint32> playerIds;

	vector< string > files;
	CPath::getPathContent( BsiGlobal.getLocalPath() + "characters", true, false, true, files );

	for( uint32 i = 0; i < files.size(); ++i )
	{
		files[ i ] = files[ i ].substr( files[ i ].find("account_") + strlen("account_"), string::npos );
		files[ i ] = files[ i ].substr(0 , files[ i ].find('_') );
		uint32 account;
		NLMISC::fromString(files[ i ], account);
		if ( playerIds.find( account ) == playerIds.end() )
			playerIds.insert( account );
	}

	uint32 size= (uint32)playerIds.size();
	uint32 i = 0;
	for( set<uint32>::iterator it=playerIds.begin(); it != playerIds.end(); ++i, ++it )
	{
		H_AUTO(LoadAndReSaveCharactersPlayerLoop);
		CPlayer *player = new CPlayer;
		nlassert(player!=NULL);

		player->setId( *it );
		PlayerManager.setPlayerFrontEndId( (uint32) ( *it ), NLNET::TServiceId(255) );

		{
			H_AUTO(LoadAndReSaveCharactersLoadPlayer);
			// add player then load (must do the add before else assert in CCharacter::postLoadTreatment())
			PlayerManager.addPlayer( *it, player );
			PlayerManager.loadPlayer( player );
		}

		std::string txt= NLMISC::toString("USER:%06d(%06x) NAME:%s",*it,*it,player->getUserName().c_str());
		for( uint32 c = 0; c < player->getCharacterReference().size(); ++c )
		{
			if( player->getCharacterReference()[ c ] )
			{
				H_AUTO(LoadAndReSaveCharactersCharacterLoop);
				CEntityId charId = PlayerManager.createCharacterId( *it, (uint8)c );
				ucstring characterName( player->getCharacterReference()[ c ]->getName() );
				std::string s;
				characterName.fromUtf8(s);
				txt+= NLMISC::toString(" CHAR%d:%s",c,s.c_str());
				{
					H_AUTO(LoadAndReSaveCharactersCharacterSaveCharacter);
					PlayerManager.savePlayerChar(*it,c);
				}
			}
		}
		nlinfo("%5i/%5i: Disconnecting Player: %s",i,playerIds.size(),txt.c_str());
		{
			H_AUTO(LoadAndReSaveCharactersDisconnectPlayer);
			PlayerManager.disconnectPlayer( *it );
		}
	}

	return true;
}

NLMISC_COMMAND(loadCharacterNames,"load all character save games and extract names","")
{
	H_AUTO(LoadCharacterNames);
	nlinfo ("Loading character names from save game files");

	// get a list of files matching the filespecs that we're interested in
	std::vector<std::string> wildcards;
	wildcards.push_back("account_*_?_pdr.bin");
	wildcards.push_back("account_*_?_pdr.xml");
	CFileDescriptionContainer fdc;
	fdc.addFiles(Bsi.getLocalPath() + "characters", wildcards, true);

	// build a map of character ids to file names (using newest file in case of multiple options)
	typedef std::map<uint32,CFileDescription> TFilesMap;
	TFilesMap files;
	for (uint32 i=0;i<fdc.size();++i)
	{
		CSString s=fdc[i].FileName;
		s.strtok("_");
		uint32 account= s.strtok("_").atoi();
		uint32 slot= s.strtok("_").atoi();
		uint32 characterId=16*account+slot;

		if (fdc[i].FileTimeStamp>=files[characterId].FileTimeStamp)
			files[characterId]= fdc[i];
	}

	// iterate over files
	uint32 numFiles= (uint32)files.size();
	uint32 i=0;
	for (TFilesMap::iterator it=files.begin(); it!=files.end(); ++it, ++i)
	{
		static CPersistentDataRecord	pdr;
		pdr.clear();
		{
			H_AUTO(LoadCharacterNamesLoadFile);
			pdr.readFromFile((*it).second.FileName.c_str());
		}
		CCharacterNameExtraction nameExtractor;
		{
			H_AUTO(LoadCharacterNamesApply);
			nameExtractor.apply(pdr);
		}

		uint32 account= (*it).first/16;
		uint32 slot= (*it).first%16;

		nlinfo("[%d/%d] account_%d_%d: %s",i,numFiles,account,slot,nameExtractor.Name.c_str());
	}

	return true;
}


//---------------------------------------------------
//NLMISC_COMMAND(loadAndReSaveAll,"load and resave all .bin in the save_shard","")
//{
//	H_AUTO(LoadAndReSaveAll);
//	nlinfo ("Loading and re-saving all .bin, .offline_commands and .ticks files");
//
//	vector< string > files;
//	string	saveDir= Bsi.getLocalPath();
//	CPath::getPathContent( saveDir, true, false, true, files );
//
//	// for all bin files
//	for( uint32 i = 0; i < files.size(); ++i )
//	{
//		string	file= CFile::getFilename(files[i]);
//		if(testWildCard(file, "*.bin") || testWildCard(file, "*.ticks") || testWildCard(file, "*.offline_commands"))
//		{
//			nlinfo("reloading %s", file.c_str());
//			// First Load
//			{
//				// std read tst
//				static CPersistentDataRecord pdrRead("");
//				pdrRead.clear();
//				pdrRead.readFromBinFile(files[i].c_str());
//				pdrRead.writeToTxtFile((saveDir + "test/txt_read/" + CFile::getFilenameWithoutExtension(file) + ".txt").c_str(), CPersistentDataRecord::LINES_STRING);
//
//				// read write tst (even with a bad used RyzomStore class)
//				static CPersistentDataRecordRyzomStore pdr;
//				pdr.clear();
//				pdr.readFromBinFile(files[i].c_str());
//				TTime	t0= CTime::getLocalTime();
//				pdr.writeToBinFile((saveDir + "test/bin_new/" + file).c_str());
//				TTime	t1= CTime::getLocalTime();
//				nlinfo("resaved %s in %d ms", file.c_str(), uint32(t1-t0));
//				pdr.writeToTxtFile((saveDir + "test/txt_before/" + CFile::getFilenameWithoutExtension(file) + ".txt").c_str(), CPersistentDataRecord::LINES_STRING);
//			}
//			// ReLoad
//			{
//				static CPersistentDataRecordRyzomStore pdr;
//				pdr.clear();
//				pdr.readFromBinFile((saveDir + "test/bin_new/" + file).c_str());
//				pdr.writeToTxtFile((saveDir + "test/txt_after/" + CFile::getFilenameWithoutExtension(file) + ".txt").c_str(), CPersistentDataRecord::LINES_STRING);
//			}
//		}
//	}
//
//	return true;
//}


//---------------------------------------------------
//NLMISC_COMMAND(dumpAllCharToTxt,"load and resave all .bin in the save_shard","")
//{
//	H_AUTO(dumpAllCharToTxt);
//
//	for(uint t=0;t<2;t++)
//	{
//		vector< string > files;
//		string	saveDir= Bsi.getLocalPath();
//		if(t==0)
//			saveDir+= "characters";
//		else
//			saveDir+= "bkup_characters";
//		CPath::getPathContent( saveDir, true, false, true, files );
//
//		// for all bin files
//		for( uint32 i = 0; i < files.size(); ++i )
//		{
//			string	file= CFile::getFilename(files[i]);
//			if(testWildCard(file, "*.bin"))
//			{
//				nlinfo("dump %s to txt", file.c_str());
//				// First Load
//				{
//					static CPersistentDataRecord pdr;
//					pdr.clear();
//					pdr.readFromFile(files[i].c_str());
//					string txtFile= files[i];
//					strFindReplace(txtFile, ".bin", ".txt");
//					pdr.writeToTxtFile(txtFile.c_str(), CPersistentDataRecord::LINES_STRING);
//				}
//			}
//		}
//	}
//
//	return true;
//}



//---------------------------------------------------
// update :
//
//---------------------------------------------------
bool CPlayerService::update()
{
	CSingletonRegistry::getInstance()->serviceUpdate();
	return true;
} // update //


//---------------------------------------------------
// release :
//
//---------------------------------------------------
void CPlayerService::release()
{
	bool packingSheets= haveArg('Q');

	// release the world instance callback
	CWorldInstances::instance().registerAiInstanceReadyCallback(NULL);

	if (!packingSheets) PlayerManager.saveAllPlayer(); // this can produce huge network messages and needs that the FlushSendingQueuesOnExit variable is on!
	if (!packingSheets) CDynamicItems::getInstance()->saveAll(); // same
	if (!packingSheets) CStatDB::getInstance()->saveAll();
	if (!packingSheets) COutpostManager::getInstance().saveAll();

	CSingletonRegistry::getInstance()->release();

	CDynamicSheetManager::getInstance()->release();

	CCommonShardCallbacks::release();
	CHarvestSourceManager::release();
	CEnvironmentalEffectManager::release();

	if (!packingSheets) CPVPManager::release();
	if (!packingSheets) CPVPManager2::release();
	if (!packingSheets) CPVPFactionHOF::release();
	if (!packingSheets) CGuildManager::release();
	if (!packingSheets) CBuildingManager::release();

	if (!packingSheets) CMissionManager::release();
	CActionDistanceChecker::release();
	if (!packingSheets) CMissionManager::release();

	CEffectManager::release();
	if (!packingSheets) CZoneManager::getInstance().release();

//	CShutdownHandler::release();

	if (!packingSheets) CClientMessages::release();
	Mirror.release();
	CCDBStructBanks::release();

	EGSPD::release();


	// dump the instance counter
	if (!packingSheets) ICommand::execute("displayInstanceCounter", *NLMISC::InfoLog);

} // release //

//---------------------------------------------------
// cbConnection
//---------------------------------------------------
void cbConnection( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
	// inform player about the service event that occured
#if !FINAL_VERSION
	PlayerManager.broadcastMessage( 1, 0, 0, string("System event : Service UP : ")+serviceName);
#endif

	if( serviceName == string("TTS") )
	{
		TTSIsUp = true;
		nlinfo("TTS connection, serviceId %d : TTSIsUp = %d",serviceId.get(), TTSIsUp);
		return;
	}
/* Do not use cbConnection for AIS use CWorldInstances::aiInstanceReady and check instance number (see below)
	else if( serviceName == string("AIS") )
	{
		// build outposts
		nlinfo("AIS is up : I can init the outpost manager");
	}
*/
	else if( serviceName == string("FS") )
	{
		FeService = serviceId;
	}
	else if (serviceName == "MFS")
	{
		CStatDB::getInstance()->cbMFServiceUp();
	}

} // cbConnection //

//---------------------------------------------------
// Called after a AIS crash for a specific AiInstance
void CPlayerService::onAiInstanceReady(const CReportStaticAIInstanceMsg &msg)
{
	nlinfo(" AI Instance %u is up", msg.InstanceNumber );

	// respawn needed character pet on this instance
	PlayerManager.respawnPetForInstance( msg.InstanceNumber, msg.InstanceContinent );
	// respawn needed bot on this instance (handledAIgroup)
	COutpostManager::getInstance().onAIInstanceReadyOrDown( msg.InstanceNumber, true );
	PlayerManager.respawnHandledAIGroupForInstance( msg.InstanceNumber );
}

//---------------------------------------------------
void CPlayerService::onAiInstanceDown(const CReportStaticAIInstanceMsg &msg)
{
	COutpostManager::getInstance().onAIInstanceReadyOrDown( msg.InstanceNumber, false );
}

// An AI instance is up
/*void CWorldInstances::aiInstanceReady( const CReportStaticAIInstanceMsgImp &msg ) const
{
	// build outposts for instance
	nlinfo("AIS is up : I can init the outpost manager");

	// respawn needed character pet on this instance
	PlayerManager.respawnPetForInstance( msg.InstanceNumber, msg.InstanceContinent );
}
*/


/*-----------------------------------------------------------------*\
						cbMirrorUp
\*-----------------------------------------------------------------*/
void cbMirrorUp( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
	nlinfo("Tick %d - Connection of the service (name %s, Id %d)", CTickEventHandler::getGameCycle(), serviceName.c_str(), serviceId.get());

	if( serviceName == string("AIS") )
	{
		// register service for effects
		CEffectManager::registerService( serviceId );

		// clean orphan target lists
		cleanOrphanTargetLists( serviceId );

		return;
	}

//	if( serviceName == string("PDS") )
//	{
//		PDSIsUp = true;
//		CGuildManager::getInstance()->onPDSConnection();
//		return;
//	}

	if( serviceName == string("GPMS") )
	{
		GPMSIsUp = true;
		// Add character in player manager to GPMS and Subscribe to positions
		PlayerManager.gpmsConnexion();

		// Inform creature manager of GPMS connexion
		CreatureManager.gpmsConnexion();

		// get list objects on the ground
//		list<CGameItemPtr > itemsOnGround;
//		GameItemManager.getItemsOnTheGround( itemsOnGround );

		CBuildingManager::getInstance()->gpmsConnection();


		// register the building cells
		/*list<CGameItemPtr >::iterator itIt;
		for( itIt = itemsOnGround.begin(); itIt != itemsOnGround.end(); ++itIt )
		{
			// add these items in the GPMS
			CMessage msgout("ADD_ENTITY");
			msgout.serial( (**itIt).getId() );
			msgout.serial( (**itIt).Loc.Pos.X );
			msgout.serial( (**itIt).Loc.Pos.Y );
			msgout.serial( (**itIt).Loc.Pos.Z );
			float heading = 0;
			msgout.serial( heading );
			msgout.serial( (**itIt).getSheetId() );
			sendMessageViaMirror(serviceId, msgout);
		}*/

		return;
	}

	if( serviceName == string("IOS") )
	{
		IOSIsUp = true;
		nlinfo("IOS connection, serviceId %d",serviceId.get());
		CGuildManager::getInstance()->onIOSConnection();
		CPVPManager2::getInstance()->onIOSMirrorUp();
		// add all teams to chat groups
		TeamManager.addAllTeamsToChatGroup();
		PlayerManager.registerCharacterName();
		// register ALL characters for string ids
//		PlayerManager.addAllCharForStringIdRequest();
		CZoneManager::getInstance().iosConnection();
//		RY_PDS::CPDStringManager::buildStringAssociation();
		DynChatEGS.iosConnection();
		CAIAliasTranslator::getInstance()->sendAliasToIOS();

		// update all the 'TEXT' property in the database
		CDBStringUpdater::getInstance().onIOSUp();
	}
} // cbMirrorUp //


/*-----------------------------------------------------------------*\
						cbMirrorDn
\*-----------------------------------------------------------------*/
void cbMirrorDn( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
	if( serviceName == string("AIS") )
	{
		// unregister service for effects
		CEffectManager::unregisterService( serviceId );

		// remove pets
		for( CPlayerManager::TMapPlayers::const_iterator it = PlayerManager.getPlayers().begin(); it != PlayerManager.getPlayers().end(); ++it )
		{
			CPlayerManager::SCPlayer scPlayer=(*it).second;

			if (scPlayer.Player)
			{
				CCharacter	*activePlayer=scPlayer.Player->getActiveCharacter();
				if (activePlayer)
				{
					activePlayer->removeSpawnedPet(serviceId);
				}
			}
		}
		// inform mission system that an AI disconnects. It is treated as a crash from the mission manager point of view
		CMissionManager::getInstance()->applyAICrashConsequences( serviceId );

		// release ai instance and collision information
		CWorldInstances::instance().aiDisconnection( serviceId);
		//release DynamicSheetManager datas previously received by a given AIS
		CDynamicSheetManager::getInstance()->releaseCustomDataByServiceId(serviceId);
		return;
	}
	else if( serviceName == "IOS" )
	{
		IOSIsUp = false;
		CDBStringUpdater::getInstance().onIOSDown();
	}
	else if( serviceName == string("GPMS") )
	{
		GPMSIsUp = false;
	}
	else if( serviceName == string("PDS") )
	{
		PDSIsUp = false;
	}

	// callback the dynamic chat manager
	DynChatEGS.onServiceDown(TServiceId(serviceId));

} // cbMirrorDn //


/*-----------------------------------------------------------------*\
						cbDisconnection
\*-----------------------------------------------------------------*/
void cbDisconnection( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
	// warn the player about the event
#if !FINAL_VERSION
	PlayerManager.broadcastMessage( 1, 0, 0, string("System event : Service DOWN : ")+serviceName);
#endif

	nlinfo("Tick %d - disconnection of the service (name %s, Id %d)", CTickEventHandler::getGameCycle(), serviceName.c_str(), serviceId.get());

	if ( serviceName == "FS" )
	{
		// if the service is a Frontend, backup and remove its clients at next tick update		DisconnectedFSes.push_back( (uint8)serviceId );
		DisconnectedFSes.push_back( TServiceId(serviceId) );
	}
	else if( serviceName == string("TTS") )
	{
		TTSIsUp = false;
		nlinfo("TTS disconnection, serviceId %d : TTSIsUp = %d",serviceId.get(), TTSIsUp);
	}
	else if (serviceName == "MFS")
	{
		CStatDB::getInstance()->cbMFServiceDown();
	}
	else if (IsRingShard && (serviceName == "DSS"))
	{
		CBitMemStream bitstream;
		GenericMsgManager.pushNameToStream( "RING_MISSION:DSS_DOWN", bitstream );
		CMessage msgout( "CDB_MULTI_IMPULSION" ); // broadcast
		uint8 sendFlags = CCDBGroup::SendDeltasToAll;
		msgout.serial( sendFlags );
		msgout.serialBufferWithSize( (uint8*)bitstream.buffer(), bitstream.length() );
		CUnifiedNetwork::getInstance()->send( "FS", msgout );
	}
}


/*-----------------------------------------------------------------*\
						NLNET_SERVICE_MAIN
\*-----------------------------------------------------------------*/
NLNET_SERVICE_MAIN( CPlayerService, "EGS", "entities_game_service", 0, EmptyCallbackArray, "", "" );


//-----------------------------------------------
// plr_stat :
//
//-----------------------------------------------
NLMISC_COMMAND(plrStat,"get or set a player stat","<pid><stat><value>")
{
	// display
	if( args.size() == 0 )
	{
		// display all the variable name

		return true;
	}

	// get
	if( args.size() == 2 )
	{
		uint32 pid;
		NLMISC::fromString(args[0], pid);
		CEntityId playerId(RYZOMID::player,pid);
		string stat( args[1] );
		string value = PlayerManager.getValue( playerId, stat );
		log.displayNL(value.c_str());
		return true;
	}

	// set
	if( args.size() > 2 )
	{
		uint32 pid;
		NLMISC::fromString(args[0], pid);
		CEntityId playerId(RYZOMID::player,pid);
		string stat( args[1] );
		string value = args[2];
		PlayerManager.setValue( playerId, stat, value );
		return true;
	}

	return false;

} // plr_stat //


/*
//-----------------------------------------------
// obj_stat :
//
//-----------------------------------------------
NLMISC_COMMAND(obj_stat,"get or set an object stat","<oid><stat>[type|loc|quality|hp]")
{
	// display
	if( args.size() == 0 )
	{
		return false;
	}

	// get
	if( args.size() == 2 )
	{
		uint32 oid;
		NLMISC::fromString(args[0], oid);
		CEntityId objectId(player,oid);
		string stat( args[1] );
		string result;
		if( args[1] == "type" ) result = toString(WorldObjectManager.getObject(objectId).getType());
		//if( args[1] == "loc" ) result = WorldObjectManager.getObject(objectId).getLocation().toString());
		if( args[1] == "quality" ) result = toString(WorldObjectManager.getObject(objectId).getQuality());
		if( args[1] == "hp" ) result = toString(WorldObjectManager.getObject(objectId).getHP());
		log.displayNL(result.c_str());
		return true;
	}

	// set
	if( args.size() > 2 )
	{
		uint32 oid;
		NLMISC::fromString(args[0], oid);
		CEntityId objectId(object,oid);
		if( args[1] == "type" ) WorldObjectManager.getObject(objectId).setType(NLMISC::fromString(args[2].c_str()));
		//if( args[1] == "loc" )
		if( args[1] == "quality" ) WorldObjectManager.getObject(objectId).setQuality(NLMISC::fromString(args[2].c_str()));
		if( args[1] == "hp" ) WorldObjectManager.getObject(objectId).setHP(NLMISC::fromString(args[2].c_str()));
		return true;
	}

	return false;

} // obj_stat //


//-----------------------------------------------
// create_obj :
//
//-----------------------------------------------
NLMISC_COMMAND(create_obj,"create a new object","<type>")
{
	if( args.size() > 0 )
	{
		CWorldObjectLocation loc;
		uint16 quality = 0;
		uint32 hp = 0;
		WorldObjectManager.createObject(NLMISC::fromString(args[2].c_str()),loc,quality,hp);
		return true;
	}
	return false;

} // create_obj //


//-----------------------------------------------
// remove_obj :
//
//-----------------------------------------------
NLMISC_COMMAND(remove_obj,"remove an object","<id>")
{
	if( args.size() > 0 )
	{
		uint32 oid;
		NLMISC::fromString(args[0], oid);
		CEntityId objId(object,oid);
		WorldObjectManager.removeObject( objId );
		return true;
	}
	else
	{
		return false;
	}

} // remove_obj //


//-----------------------------------------------
// dump_objects :
//
//-----------------------------------------------
NLMISC_COMMAND(dump_objects,"dump world object list to a text file","<file name>")
{
	if( args.size() > 0 )
	{
		WorldObjectManager.dumpWorldObjectList(args[0]);
		return true;
	}
	return false;

} // dump_objects //


//-----------------------------------------------
// dump_stats :
//
//-----------------------------------------------
NLMISC_COMMAND(dump_stats,"dump stats of an object to the output win or log","<object id>[<file name>]")
{
	if( args.size() > 0 )
	{
		CEntityId objId = CEntityId(object,NLMISC::fromString(args[0].c_str()));
		if( args.size() > 1 )
		{
			WorldObjectManager.getObject(objId).dumpWorldObjectStats(args[1]);
		}
		else
		{
			WorldObjectManager.getObject(objId).dumpWorldObjectStats();
		}
		return true;
	}
	return false;

} // dump_stats //
*/

/*
//-----------------------------------------------
// change_mode :
//-----------------------------------------------
NLMISC_COMMAND(changeMode," change_mode","<entity id(id:type:crea:dyn)> <mode>")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		MBEHAV::EMode mode = MBEHAV::EMode(NLMISC::fromString(args[1].c_str()));

		CEntityBase *e;
		if( id.getType() == 0 )
		{
			e = PlayerManager.getChar(id);
		}
		else
		{
			e = CreatureManager.getCreature(id);
		}
		if (e)
		{
			e->setMode( mode, true );
			log.displayNL("Entity %s mode changed to %d", id.toString().c_str(), mode);
		}
		else
		{
			log.displayNL("Unknown entity %s ",id.toString().c_str());
		}
		return true;
	}
	return false;
} // change_mode //
*/

//-----------------------------------------------
// display_mode :
//-----------------------------------------------
NLMISC_COMMAND(displayMode," display_mode","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CEntityBase *e = NULL;
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
		if (e)
		{
			log.displayNL("Entity %s have mode %s", id.toString().c_str(), toStringEnum(e->getMode()).c_str());
		}
		else
		{
			log.displayNL("Unknown entity %s ",id.toString().c_str());
		}

		return true;
	}
	return false;
} // display_mode //


//-----------------------------------------------
// change_behaviour :
//-----------------------------------------------
NLMISC_COMMAND(changeBehaviour," change entity behaviour","<entity id(id:type:crea:dyn)> <behaviour>")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		sint behav;
		NLMISC::fromString(args[1], behav);
		MBEHAV::EBehaviour behaviour = MBEHAV::EBehaviour(behav);

		CEntityBase *e = NULL;
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
		if (e)
		{
			e->setBehaviour( behaviour );
			log.displayNL("Entity %s have behaviour %s", id.toString().c_str(), toString( e->getBehaviour() ).c_str() );
		}
		else
		{
			log.displayNL("Unknown entity id %s ",id.toString().c_str());
		}

		return true;
	}
	return false;
} // change_behaviour //


//-----------------------------------------------
// display_behaviour :
//-----------------------------------------------
NLMISC_COMMAND(displayBehaviour," display entity behaviour","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CEntityBase *e = NULL;
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
		if(e)
		{
			log.displayNL("Entity %s have behaviour %s", id.toString().c_str(), toString( e->getBehaviour() ).c_str() );
		}
		else
		{
			log.displayNL("Unknown entity %s ",id.toString().c_str());
		}
		return true;
	}
	return false;
} // display_behaviour //

/*
//-----------------------------------------------
// change_var :
//-----------------------------------------------
NLMISC_COMMAND(changeVar," change a variable for entity","<entity id(id:type:crea:dyn)> <var> <new val>")
{
	if( args.size() == 3 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		string var = args[1].c_str();
		string value = args[2].c_str();

		CEntityBase *e;
		if( id.getType() == 0 )
		{
			e = PlayerManager.getChar(id);
		}
		else
		{
			e = CreatureManager.getCreature(id);
		}
		if(e)
		{
			if ( e->setValue(var, value) )
				log.displayNL("Change Var of Entity %s Var %s Value %s", id.toString().c_str(), var.c_str(), value.c_str());
		}
		else
		{
			log.displayNL("Unknown entity %s",id.toString().c_str());
		}

		return true;
	}
	return false;
} // change_var //
*/

//-----------------------------------------------
// modify var :
//-----------------------------------------------
NLMISC_COMMAND(modifyVar," modify a variable for entity","<entity id(id:type:crea:dyn)> <var> <delta>")
{
	if( args.size() == 3 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		string var = args[1].c_str();
		string delta = args[2].c_str();

		CEntityBase *e = NULL;
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
		if(e)
		{
			if ( e->modifyValue( var, delta ) )
				log.displayNL("Change Var of Entity %s Var %s Delta %s", id.toString().c_str(), var.c_str(), delta.c_str());
		}
		else
		{
			log.displayNL("Unknown entity %s",id.toString().c_str());
		}

		return true;
	}
	return false;
} // change_var //


//-----------------------------------------------
// display_var :
//-----------------------------------------------
NLMISC_COMMAND(displayVar," display a Var for entity","<entity id(id:type:crea:dyn)> <var>")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		string var = args[1].c_str();
		string value;

		CEntityBase *e = 0;
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
		if(e)
		{
			e->getValue( var, value );
			log.displayNL( "Var %s = %s for entity %s", var.c_str(), value.c_str(), id.toString().c_str() );
		}
		else
		{
			log.displayNL("Unknown entity %s",id.toString().c_str());
		}

		return true;
	}

	return false;
} // display_var //


//-----------------------------------------------
// change_var :
//-----------------------------------------------
NLMISC_COMMAND(changeVarEntities," change_var","<var> <new val>")
{
	if( args.size() == 2 )
	{
		string var = args[0].c_str();
		string value = args[1].c_str();

		for( TMapCreatures::const_iterator it = CreatureManager.getCreature().begin(); it != CreatureManager.getCreature().end(); ++it )
		{
			(*it).second->setValue(var, value);
		}
		return true;
	}
	return false;
} // change_var //


//-----------------------------------------------
// Change visual propertyA :
//-----------------------------------------------
NLMISC_COMMAND(changeVisualPropertyA, " changeVisualPropertyA","<entity id(id:type:crea:dyn)> <VisualSubPropertyName, value>")
{
	if( args.size() == 3 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		string name = args[1];
		uint8 value;
		NLMISC::fromString(args[2], value);

		CCharacter *c;
		c = PlayerManager.getChar(id);
		if(c && c->getEnterFlag())
		{
			if( name == string("JacketModel") )
			{
				SET_STRUCT_MEMBER( c->getVisualPropertyA(), PropertySubData.JacketModel, value );
			}
			else if( name == string("JacketColor") )
			{
				SET_STRUCT_MEMBER( c->getVisualPropertyA(), PropertySubData.JacketColor, value );
			}
			else if( name == string("TrouserModel") )
			{
				SET_STRUCT_MEMBER( c->getVisualPropertyA(), PropertySubData.TrouserModel, value );
			}
			else if( name == string("TrouserColor") )
			{
				SET_STRUCT_MEMBER( c->getVisualPropertyA(), PropertySubData.TrouserColor, value );
			}
			else if( name == string("WeaponRightHand") )
			{
				SET_STRUCT_MEMBER( c->getVisualPropertyA(), PropertySubData.WeaponRightHand, value );
			}
			else if( name == string("WeaponLeftHand") )
			{
				SET_STRUCT_MEMBER( c->getVisualPropertyA(), PropertySubData.WeaponLeftHand, value );
			}
		}
		else
		{
			log.displayNL("Unknown entity %s",id.toString().c_str());
		}

		return true;
	}

	return false;
} // display_var //


//-----------------------------------------------
// Display visual propertyA :
//-----------------------------------------------
NLMISC_COMMAND(displayVisualPropertyA, " displayVisualPropertyA","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *c;
		c = PlayerManager.getChar(id);
		if(c && c->getEnterFlag())
		{
			log.displayNL(" Sex = %d", c->getVisualPropertyA()().PropertySubData.Sex );

			log.displayNL(" JacketModel = %d", c->getVisualPropertyA()().PropertySubData.JacketModel );
			log.displayNL(" JacketColor = %d", c->getVisualPropertyA()().PropertySubData.JacketColor );
			log.displayNL(" TrouserModel = %d", c->getVisualPropertyA()().PropertySubData.TrouserModel );
			log.displayNL(" TrouserColor = %d", c->getVisualPropertyA()().PropertySubData.TrouserColor );
			log.displayNL(" WeaponRightHand = %d", c->getVisualPropertyA()().PropertySubData.WeaponRightHand );
			log.displayNL(" WeaponLeftHand = %d", c->getVisualPropertyA()().PropertySubData.WeaponLeftHand );
			log.displayNL(" ArmModel = %d", c->getVisualPropertyA()().PropertySubData.ArmModel );
			log.displayNL(" ArmColor = %d", c->getVisualPropertyA()().PropertySubData.ArmColor );
			log.displayNL(" HatModel = %d", c->getVisualPropertyA()().PropertySubData.HatColor );
			log.displayNL(" HatColor = %d", c->getVisualPropertyA()().PropertySubData.HatColor );
		}
		else
		{
			log.displayNL("Unknown entity %s",id.toString().c_str());
		}

		return true;
	}

	return false;
} // display_var //


//-----------------------------------------------
// Display visual propertyB :
//-----------------------------------------------
NLMISC_COMMAND(displayVisualPropertyB, " displayVisualPropertyB","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *c;
		c = PlayerManager.getChar(id);
		if(c && c->getEnterFlag())
		{
			log.displayNL(" HandsModel = %d", c->getVisualPropertyB()().PropertySubData.HandsModel );
			log.displayNL(" HandsColor = %d", c->getVisualPropertyB()().PropertySubData.HandsColor );
			log.displayNL(" FeetModel = %d", c->getVisualPropertyB()().PropertySubData.FeetModel );
			log.displayNL(" FeetColor = %d", c->getVisualPropertyB()().PropertySubData.FeetColor );
		}
		else
		{
			log.displayNL("Unknown entity %s",id.toString().c_str());
		}

		return true;
	}

	return false;
} // display_var //


//-----------------------------------------------
// Display visual propertyC :
//-----------------------------------------------
NLMISC_COMMAND(displayVisualPropertyC, " displayVisualPropertyC","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *c;
		c = PlayerManager.getChar(id);
		if(c && c->getEnterFlag())
		{
			log.displayNL(" MorphTarget1 = %d", c->getVisualPropertyC()().PropertySubData.MorphTarget1 );
			log.displayNL(" MorphTarget2 = %d", c->getVisualPropertyC()().PropertySubData.MorphTarget2 );
			log.displayNL(" MorphTarget3 = %d", c->getVisualPropertyC()().PropertySubData.MorphTarget3 );
			log.displayNL(" MorphTarget4 = %d", c->getVisualPropertyC()().PropertySubData.MorphTarget4 );
			log.displayNL(" MorphTarget5 = %d", c->getVisualPropertyC()().PropertySubData.MorphTarget5 );
			log.displayNL(" MorphTarget6 = %d", c->getVisualPropertyC()().PropertySubData.MorphTarget6 );
			log.displayNL(" MorphTarget7 = %d", c->getVisualPropertyC()().PropertySubData.MorphTarget7 );
			log.displayNL(" MorphTarget8 = %d", c->getVisualPropertyC()().PropertySubData.MorphTarget8 );
			log.displayNL(" EyesColor = %d",	c->getVisualPropertyC()().PropertySubData.EyesColor );
			log.displayNL(" Tattoo = %d",		c->getVisualPropertyC()().PropertySubData.Tattoo );
			log.displayNL(" CharacterHeight = %d", c->getVisualPropertyC()().PropertySubData.CharacterHeight );
			log.displayNL(" TorsoWidth = %d", c->getVisualPropertyC()().PropertySubData.TorsoWidth );
			log.displayNL(" ArmsWidth = %d", c->getVisualPropertyC()().PropertySubData.ArmsWidth );
			log.displayNL(" LegsWidth = %d", c->getVisualPropertyC()().PropertySubData.LegsWidth );
			log.displayNL(" BreastSize = %d", c->getVisualPropertyC()().PropertySubData.BreastSize );
		}
		else
		{
			log.displayNL("Unknown entity %s",id.toString().c_str());
		}

		return true;
	}

	return false;
} // display_var //


//-----------------------------------------------
// display_entity_characteristics :
//-----------------------------------------------
NLMISC_COMMAND(displayEntityCharacteristics," displayEntityCharacteristics","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CEntityBase *e = 0;
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

		if( e )
		{
			string value;
			string var;
			log.displayNL( "Entity %s characteristics:", id.toString().c_str() );
			for( uint16 i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
			{
				var = string("Base") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics)i );
				e->getValue( var, value );
				log.displayNL( "	Var %s = %s", var.c_str(), value.c_str() );
				var = string("Max") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics)i );
				e->getValue( var, value );
				log.displayNL( "	Var %s = %s", var.c_str(), value.c_str() );
				var = string("Modifier") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics)i );
				e->getValue( var, value );
				log.displayNL( "	Var %s = %s", var.c_str(), value.c_str() );
				var = string("Current") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics)i );
				e->getValue( var, value );
				log.displayNL( "	Var %s = %s", var.c_str(), value.c_str() );
			}
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
// display_entity_scores :
//-----------------------------------------------
NLMISC_COMMAND(displayEntityScores," displayEntityScores","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CEntityBase *e = 0;
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

		if( e )
		{
			string value;
			string var;
			log.displayNL( "Entity %s scores:", id.toString().c_str() );
			for( uint16 i = 0; i < SCORES::NUM_SCORES; ++i )
			{
				var = string("Base") + SCORES::toString( i );
				e->getValue( var, value );
				log.displayNL( "	Var %s = %s", var.c_str(), value.c_str() );
				var = string("Max") + SCORES::toString( i );
				e->getValue( var, value );
				log.displayNL( "	Var %s = %s", var.c_str(), value.c_str() );
				var = string("Modifier") + SCORES::toString( i );
				e->getValue( var, value );
				log.displayNL( "	Var %s = %s", var.c_str(), value.c_str() );
				var = string("Current") + SCORES::toString( i );
				e->getValue( var, value );
				log.displayNL( "	Var %s = %s", var.c_str(), value.c_str() );
			}
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
// display_entity_scores :
//-----------------------------------------------
NLMISC_COMMAND(displayEntitySkills,"displayEntitySkills","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *c = PlayerManager.getChar(id);
		if(c )
		{
			log.displayNL( "Entity %s skills:", id.toString().c_str() );
			for( int i = 0; i < SKILLS::NUM_SKILLS; ++i )
			{
				log.displayNL( "	Skill %s = Base %d Max Child %d", SKILLS::toString(i).c_str(), c->getSkills()._Skills[ i ].Base, c->getSkills()._Skills[ i ].MaxLvlReached );
			}
		}
		else
		{
			log.displayNL("Unknown entity %s ",id.toString().c_str());
		}
		return true;
	}
	return false;
} // change_mode //

//------------------------------------------------------
// display all attributes modified by user model script
//------------------------------------------------------
NLMISC_COMMAND(displayModifiedEntity, "displayModifiedEntity", "<creature id(id:type:crea:dyn)>")
{
	if (args.size() == 1)
	{
		CEntityId id;
		id.fromString(args[0].c_str());

		CCreature *e = 0;
		if( id.getType() == 0 )
		{
			log.displayNL("Player can't be modified by UserModel script.");
			return false;
		}
		else
		{
			e = CreatureManager.getCreature(id);
		}

		if (e)
		{
			e->displayModifiedAttributes(id, log);
			return true;

		}
		else
		{
			log.displayNL("Unknown entity %s or no user model applied", id.toString().c_str());
			return true;
		}
	}
	return false;
}





//-----------------------------------------------
// add player entity :
//-----------------------------------------------
NLMISC_COMMAND(createPlayer," create a player","<playerId, characterName, race, sex>")
{
	if( args.size() == 4 )
	{
		uint32 playerId;
		NLMISC::fromString(args[0], playerId);
		string characterName = args[1];
		EGSPD::CPeople::TPeople race = EGSPD::CPeople::fromString( args[2] );
		if( race == EGSPD::CPeople::EndPeople ) return false;

		GSGENDER::EGender gender = GSGENDER::stringToEnum( args[3] );
		if( gender == GSGENDER::unknown ) return false;

		CPlayer *player = PlayerManager.getPlayer( playerId );
		if( player == 0 )
		{
			player = new CPlayer;
			player->setId( playerId );
			PlayerManager.setPlayerFrontEndId( (uint32) ( playerId ), FeService );
			PlayerManager.addPlayer( playerId, player );
		}

		CEntityId id = player->createCharacter( characterName, race, gender );
		if( Mirror.addEntity( false, id ) ) // Untested here
		{
			PlayerManager.setActiveCharForPlayer( playerId, (uint32) (id.getShortId() & 0xf), id );
			player->getActiveCharacter()->addPropertiesToMirror( TheDataset.getDataSetRow( id ) );
			player->getActiveCharacter()->mirrorizeEntityState(); // write the initial position into the mirror
			player->getActiveCharacter()->setEnterFlag( true );
			return true;
		}
		else
		{
			PlayerManager.disconnectPlayer( playerId );
		}
	}
	return false;
}

//-----------------------------------------------
// add player entity :
//-----------------------------------------------
NLMISC_COMMAND(spawnCharacters," spawn somes characters","<Number, characterName, race, sex>")
{
	string command;

	if( args.size() == 4 )
	{
		uint32 NbPlayer;
		NLMISC::fromString(args[0], NbPlayer);
		for( uint32 i = 0; i < NbPlayer; ++i )
		{
			command = string("createPlayer ") + NLMISC::toString(i) + string(" ") + args[1] + NLMISC::toString(i) + string(" ") + args[2] + string(" ") + args[3];
			ICommand::execute(command, log);
		}
		return true;
	}
	return false;
}



//-----------------------------------------------
// replace player entity :
//-----------------------------------------------
/*NLMISC_COMMAND(replaceCharacter," replace a character","<playerId, characterName, race, sex, role, level>")
{
	if( args.size() == 6 )
	{
		uint32 playerId;
		NLMISC::fromString(args[0], playerId);
		string characterName = args[1];
		EGSPD::CPeople::TPeople race = EGSPD::CPeople::fromString( args[2] );
		if( race == EGSPD::CPeople::EndPeople ) return false;

		GSGENDER::EGender gender = GSGENDER::stringToEnum( args[3] );
		if( gender == GSGENDER::unknown ) return false;

		ROLES::ERole role = ROLES::toRoleId( args[4] );
		if( role == ROLES::role_unknown ) return false;

		uint16 level;
		NLMISC::fromString(args[5], level);

		CPlayer *player = PlayerManager.getPlayer( playerId );
		if( player != 0 )
		{
			player->replaceCharacter( characterName, race, gender, role, level );
			return true;
		}
	}
	return false;
}*/


//-----------------------------------------------
// save playerActiveChar :
//-----------------------------------------------
NLMISC_COMMAND(savePlayerActiveChar," save player with regular character filename","uid[,filename]")
{
	if( args.size() >= 1 )
	{
		uint32 userId;
		NLMISC::fromString(args[0], userId);
		if(args.size() == 2)
		{
			string filename = args[1];
			PlayerManager.savePlayerActiveChar( userId, &filename );
		}
		else
		{
			PlayerManager.savePlayerActiveChar( userId );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// save player :
//-----------------------------------------------
NLMISC_COMMAND(saveAllPlayerChars," saveAllPlayerChars","uid")
{
	if( args.size() == 1 )
	{
		uint32 userId;
		NLMISC::fromString(args[0], userId);

		// If the user really exist
		CPlayer	*player= PlayerManager.getPlayer(userId);
		if(player)
		{
			// Save all char of this user
			std::string txt= NLMISC::toString("USER:%06d(%06x) NAME:%s",userId,userId,player->getUserName().c_str());
			for( uint32 c = 0; c < player->getCharacterReference().size(); ++c )
			{
				// if exist
				if( player->getCharacterReference()[ c ] )
				{
					// dbg text
					ucstring characterName( player->getCharacterReference()[ c ]->getName() );
					std::string s= characterName.toUtf8();
					txt+= NLMISC::toString(" CHAR%d:%s",c,s.c_str());
					// save the character
					PlayerManager.savePlayerChar(userId,c);
				}
			}
			nlinfo("Save All Player Chars: %s",txt.c_str());
		}
		else
			nlinfo("Save All Player Chars failed. User %d not found", userId);

		return true;
	}
	return false;
}

//-----------------------------------------------
// load player :
//-----------------------------------------------
NLMISC_COMMAND(reloadPlayer," reload a connected player character, a previous saved played can be loaded at next connexion/reloaded by indicate charIndex and filename ", "uid[, charIndex, filename]")
{
	if( args.size() == 1 || args.size() == 3 )
	{
		uint32 userId;
		NLMISC::fromString(args[0], userId);
		if(args.size() == 3)
		{
			uint32 charIndex;
			NLMISC::fromString(args[1], charIndex);
			string filename = args[2];
			PlayerManager.reloadPlayerActiveChar( userId, charIndex, &filename );
		}
		else
		{
			PlayerManager.reloadPlayerActiveChar( userId );
		}
		return true;
	}
	return false;
}

//-----------------------------------------------
// load player :
//-----------------------------------------------
NLMISC_COMMAND(loadPlayer," load a player","uid")
{
	if( args.size() == 1 )
	{
		uint32 userId;
		NLMISC::fromString(args[0], userId);

		// set the front end id of this player
		PlayerManager.setPlayerFrontEndId( userId, NLNET::TServiceId(0xff) );

		CPlayer *player = new CPlayer;
		player->setId( userId );
		// add player then load (must do the add before else assert in CCharacter::postLoadTreatment())
		PlayerManager.addPlayer( userId, player );
		PlayerManager.loadPlayer( player );

		// set status connexion of player to connected
		player->setPlayerConnection( true );

		return true;
	}
	return false;
}


//-----------------------------------------------
// activePlayer :
//-----------------------------------------------
NLMISC_COMMAND(activePlayer," active a character (same has choosed by player)","client id, character number")
{
	if( args.size() == 2 )
	{
		uint32 userId;
		NLMISC::fromString(args[0], userId);
		uint32 characterIndex;
		NLMISC::fromString(args[1], characterIndex);

		CCharacter * ch = PlayerManager.getChar( userId, characterIndex );
		if( ch )
		{
			PlayerManager.setActiveCharForPlayer( userId, characterIndex, ch->getId() );
			log.displayNL( "Character id %s is loaded (Player Account %hu, Character index %hu)", PlayerManager.getActiveChar( userId )->getId().toString().c_str(), userId, (uint16)characterIndex );
		}
		else
		{
			log.displayNL( "Character index %d not exist for player %d", characterIndex, userId );
		}
		return true;
	}
	return false;
}

//-----------------------------------------------
// simulateClientReady :
//-----------------------------------------------
NLMISC_COMMAND(simulateClientReady,"Simulate clientReady for a character","client id, character number")
{
	if( args.size() == 2 )
	{
		uint32 userId;
		NLMISC::fromString(args[0], userId);
		uint32 characterIndex;
		NLMISC::fromString(args[1], characterIndex);

		CCharacter * ch = PlayerManager.getChar( userId, characterIndex );
		if( ch )
		{
			CEntityId characterId = ch->getId();
			NLNET::CMessage msg( "CLIENT:RDY" );
			msg.serial( characterId );
			msg.invert();
			cbClientReady(msg, "", NLNET::TServiceId(0));
		//	log.displayNL( "Character id %s is loaded (Player Account %hu, Character index %hu)", PlayerManager.getActiveChar( userId )->getId().toString().c_str(), userId, (uint16)characterIndex );
			log.displayNL( "simulateClientReady done for character id %s (Player Account %hu, Character index %hu)", PlayerManager.getActiveChar( userId )->getId().toString().c_str(), userId, (uint16)characterIndex );
		}
		else
		{
			log.displayNL( "Character index %d not exist for player %d", characterIndex, userId );
		}
		return true;
	}
	return false;
}


// predicate for following command
struct TIsNotACharFile : std::unary_function<string, bool>
{
	bool operator ()(const std::string &fileName) const
	{
		if (fileName.find("_pdr.bin") != fileName.size()-8)
			return true;
		if (fileName.find("account_") == string::npos)
			return true;
		return false;
	}
};

NLMISC_COMMAND(moveCharAndOfflineCmdToHashTable, "Move all character and offline commands file into the new hash table","")
{
	string savePath = BsiGlobal.getLocalPath() + "characters";

	vector<string> allChars;
	CPath::getPathContent(savePath, false, false, true, allChars);

	for (uint i=0; i<allChars.size(); ++i)
	{
		if (testWildCard(CFile::getFilename(allChars[i]), "account*.bin"))
		{
			//extract account id
			vector<string> parts;
			explode(CFile::getFilename(allChars[i]), string("_"), parts);
			if (parts.size() == 4)
			{
				uint32 userId;
				NLMISC::fromString(parts[1], userId);

				// make sure the dest path exist
				CFile::createDirectory(PlayerManager.getCharacterPath(userId, false));
				// move the file
				CFile::moveFile(
					(PlayerManager.getCharacterPath(userId, false)+CFile::getFilename(allChars[i])).c_str(),
					allChars[i].c_str()
					);
			}
		}
	}

	savePath = Bsi.getLocalPath() + "characters_offline_commands";
	vector<string> allCommands;
	CPath::getPathContent(savePath, false, false, true, allCommands);

	for (uint i=0; i<allCommands.size(); ++i)
	{
		if (testWildCard(CFile::getFilename(allCommands[i]), "account*.offline_commands"))
		{
			//extract account id
			vector<string> parts;
			explode(CFile::getFilename(allCommands[i]), string("_"), parts);
			if (parts.size() == 4)
			{
				uint32 userId;
				NLMISC::fromString(parts[1], userId);

				// make sure the dest path exist
				CFile::createDirectory(PlayerManager.getOfflineCommandPath(userId, false));
				// move the file
				CFile::moveFile(
					(PlayerManager.getOfflineCommandPath(userId, false)+CFile::getFilename(allCommands[i])).c_str(),
					allCommands[i].c_str()
					);
			}
		}
	}

	return true;
}


NLMISC_COMMAND(loadAllPlayerAndReady,"Load all the player saves (all account, all slots) and call 'clientReady'","")
{
	string savePath = BsiGlobal.getLocalPath() + "characters";

	vector<string> allChars;
	CPath::getPathContent(savePath, false, false, true, allChars);

	// remove any unwanted file
	allChars.erase(std::remove_if(allChars.begin(), allChars.end(), TIsNotACharFile()), allChars.end());
	// sort the resulting file list
	std::sort(allChars.begin(), allChars.end());

	log.displayNL("Starting to load %u players chars", allChars.size());

	for (uint i=0; i<allChars.size(); ++i)
	{
		CSString fileName = CFile::getFilename(allChars[i]);

		// account file name as the following form : 'account_<account_num>_<slot_num>*'
		vector<string> parts;
		explode(string(fileName), string("_"), parts, false);

		if (parts.size() < 3)
		{
			log.displayNL("Couldn't parse file name '%s' for account and slot number",
				fileName.c_str());
			continue;
		}

		// get the account number
		string accountStr = parts[1];
		// get the slot number
		string slotStr = parts[2];

		uint32 account, slot;
		NLMISC::fromString(accountStr, account);
		NLMISC::fromString(slotStr, slot);

		nldebug("Loading account %u, slot %u", account, slot);

		// call other NeL command to do the job
		string cmd("loadPlayer ");
		cmd += accountStr;
		ICommand::execute(cmd, log);

		cmd = "activePlayer ";
		cmd += accountStr +" "+slotStr;
		ICommand::execute(cmd, log);

		cmd = "simulateClientReady ";
		cmd += accountStr+" "+slotStr;
		ICommand::execute(cmd, log);

		// Now, found the activated player
		CEntityId eid = PlayerManager.createCharacterId(account, slot);

		cmd = "disconnectPlayer ";
		cmd += toString("%u", PlayerManager.getPlayerId(eid));
		ICommand::execute(cmd, log);

		// dump instance counter every 100 account loaded
		if ((i%100) == 0)
		{
			cmd = "displayInstanceCounter";
			ICommand::execute(cmd, log);
		}
	}

	return true;
}



// predicate for following command
struct TIsNotAOldCharFile : std::unary_function<string, bool>
{
	bool operator ()(const std::string &fileName) const
	{
		if (fileName.find("_pdr.bin") != string::npos)
			return true;
		if (fileName.find(".bin") != fileName.size()-4)
			return true;
		if (fileName.find("account_") == string::npos)
			return true;
		return false;
	}
};

NLMISC_COMMAND(convertAllOldCharacterSaves,"Load all the old (.bin) not already converted characters saves, call 'clientReady' and save in pdr","")
{
	string savePath = BsiGlobal.getLocalPath() + "characters";

	vector<string> allChars;
	CPath::getPathContent(savePath, true, false, true, allChars);

	// prepare a set for fast checking of already converted char file
	set<string>		fileIndex(allChars.begin(), allChars.end());

	// remove any unwanted file
	allChars.erase(std::remove_if(allChars.begin(), allChars.end(), TIsNotAOldCharFile()), allChars.end());
	// sort the resulting file list
	std::sort(allChars.begin(), allChars.end());

	log.displayNL("Starting to load at most %u players chars", allChars.size());

	for (uint i=0; i<allChars.size(); ++i)
	{
		CSString fileName = CFile::getFilename(allChars[i]);

		// account file name as the following form : 'account_<account_num>_<slot_num>*'
		vector<string> parts;
		explode(string(fileName), string("_"), parts, false);

		if (parts.size() < 3)
		{
			log.displayNL("Couldn't parse file name '%s' for account and slot number",
				fileName.c_str());
			continue;
		}

		// get the account number
		string accountStr = parts[1];
		// get the slot number
		string slotStr = parts[2];

		uint32 account, slot;
		NLMISC::fromString(accountStr, account);
		NLMISC::fromString(slotStr, slot);

		// check if the character is already converted
		string newFileName = PlayerManager.getCharacterPath(account, false)+toString("/account_%u_%u_pdr.bin", account, slot);

		if (fileIndex.find(newFileName) != fileIndex.end())
		{
			log.displayNL("Character '%s' is already converted, skiping", fileName.c_str());
			continue;
		}

		nldebug("Loading account %u, slot %u", account, slot);

		// call other NeL command to do the job
		string cmd("loadPlayer ");
		cmd += accountStr;
		ICommand::execute(cmd, log);

		cmd = "activePlayer ";
		cmd += accountStr +" "+slotStr;
		ICommand::execute(cmd, log);

		cmd = "simulateClientReady ";
		cmd += accountStr+" "+slotStr;
		ICommand::execute(cmd, log);

		// Now, found the activated player
		CEntityId eid = PlayerManager.createCharacterId(account, slot);

		cmd = "disconnectPlayer ";
		cmd += toString("%u", PlayerManager.getPlayerId(eid));
		ICommand::execute(cmd, log);

		if ((i%50) == 0)
		{
			log.displayNL("Progress : %u/%u (%.2f%%)", i+1, allChars.size(), float(i)/allChars.size());
			cmd = "displayInstanceCounter";
			ICommand::execute(cmd, log);
		}
	}

	return true;
}

//-----------------------------------------------
// simulate connect player:
//-----------------------------------------------
NLMISC_COMMAND(simulatePlayerConnect,"simulate connect players","<first user id><last user id><nb times><FE Id>")
{
	if( args.size() != 4 )
		return false;

	uint32 fid, lid, rep;
	NLMISC::fromString(args[0], fid);
	NLMISC::fromString(args[1], lid);
	NLMISC::fromString(args[2], rep);
	uint16 sid;
	NLMISC::fromString(args[3], sid);
	NLNET::TServiceId feid(sid);

	if( rep != 0 && lid >= fid )
	{
		for( uint32 i = 0; i < rep; ++i )
		{
			sint32 finalizeClientReadyCounter = 0;

			for( uint32 p = fid; p <= lid; ++p )
			{
				{ // load player
					// set the front end id of this player
					PlayerManager.setPlayerFrontEndId( p, feid );

					CPlayer *player = new CPlayer;
					player->setId( p );
					// add player then load (must do the add before else assert in CCharacter::postLoadTreatment())
					PlayerManager.addPlayer( p, player );
					PlayerManager.loadPlayer( player );

					// set status connexion of player to connected
					player->setPlayerConnection( true );
				}

				{ //activate character
					CCharacter * ch = PlayerManager.getChar( p, 0);
					if( ch )
					{
						PlayerManager.setActiveCharForPlayer( p, 0, ch->getId() );
					}
				}
/*
				{ // simulate client ready
					CCharacter * ch = PlayerManager.getChar( p, 0 );
					if( ch )
					{
						CEntityId characterId = ch->getId();
						NLNET::CMessage msg( "CLIENT:RDY" );
						msg.serial( characterId );
						msg.invert();
						cbClientReady(msg, "", 0);
					}
				}
*/			}

			for( uint32 p = fid; p <= lid; ++p )
			{
				{ // disconnect player
					if( PlayerManager.getPlayer( p ) != 0)
					{
						PlayerManager.disconnectPlayer( p );
					}
				}
			}
		}
	}
	return true;
}


//-----------------------------------------------
// remove entity from EGS:
//-----------------------------------------------
NLMISC_COMMAND(disconnectPlayer,"remove player from egs","<user id>")
{
	if( args.size() != 1 )
		return false;

	uint32 id;
	NLMISC::fromString(args[0], id);
	if( PlayerManager.getPlayer( id ) != 0)
	{
		PlayerManager.savePlayerActiveChar( id );
		PlayerManager.disconnectPlayer( id );
	}
	return true;
}


//-----------------------------------------------
// Display all Player Character in EGS:
//-----------------------------------------------
NLMISC_COMMAND(displayPlayers,"display the player ids used, and short info about them","")
{
	if( args.size() == 0 )
	{
		const CPlayerManager::TMapPlayers& player = PlayerManager.getPlayers();
		for( CPlayerManager::TMapPlayers::const_iterator it = player.begin(); it != player.end(); ++it )
		{
			const CCharacter *character = it->second.Player->getActiveCharacter();
			if( character )
			{
				uint32 sessionId = 0;

				// retrieve the session id from the position stack
				if (!character->getPositionStack().empty())
				{
					sessionId = character->getPositionStack().top().SessionId;
					if (sessionId == SessionLockPositionStack)
						sessionId = character->currentSessionId();
				}

				log.displayNL( "Player: %d Name: %s ID: %s FE: %d Sheet: %s - %d Priv: '%s' Pos: %i,%i,%i Session: %u",
					it->second.Player->getUserId(),
					character->getName().toString().c_str(),
					character->getId().toString().c_str(),
					PlayerManager.getPlayerFrontEndId( it->second.Player->getUserId() ).get(),
					character->getType().toString().c_str(),
					character->getType().asInt(),
					it->second.Player->getUserPriv().c_str(),
					character->getX()/1000, character->getY()/1000, character->getZ()/1000,
					sessionId );
			}
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// Display all Creatures in EGS:
//-----------------------------------------------
NLMISC_COMMAND(displayCreatures," displayCreatures","")
{
	if( args.size() == 0 )
	{
		const TMapCreatures& creature = CreatureManager.getCreature();
		for( TMapCreatures::const_iterator it = creature.begin(); it != creature.end(); ++it )
		{
			log.displayNL( "Creature: %s Name: %s Sheet: %s - %d",
				it->first.toString().c_str(),
				it->second->getName().toString().c_str(),
				it->second->getType().toString().c_str(),
				it->second->getType().asInt() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// Count number of player character in EGS:
//-----------------------------------------------
NLMISC_DYNVARIABLE( uint32, NbPlayers, "Number of connected players" )
{
	if ( get )
		*pointer = PlayerManager.getNumberPlayers();
}


//-----------------------------------------------
// Count number of creature in EGS:
//-----------------------------------------------
NLMISC_DYNVARIABLE( uint32, Creatures, "Number of creatures" )
{
	if ( get )
		*pointer = CreatureManager.getNumberCreature();
}

//-----------------------------------------------
// Count number of entities in phrase manager
//-----------------------------------------------
NLMISC_DYNVARIABLE( uint32, NbEntitiesInPhraseManager, "Number of entities currently managed in phrase manager" )
{
	if ( get )
		*pointer = CPhraseManager::getInstance().getNbEntitiesInManager();
}


//-----------------------------------------------
// Harvest simulation
//-----------------------------------------------
NLMISC_COMMAND( harvest," harvest","entity id(id:type:crea:dyn), Index Mp, Qty")
{
	if( args.size() == 3 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		uint8 index;
		NLMISC::fromString(args[1], index);

		uint16 qty;
		NLMISC::fromString(args[2], qty);

		CCharacter *character = PlayerManager.getChar( id );
		if( character )
		{
			character->harvest( index, qty );
		}
		else
		{
			log.displayNL( "character %s not found", args[0].c_str() );
			return false;
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// Harvest simulation
//-----------------------------------------------
NLMISC_COMMAND( pickup," pickup","player id(id:type:crea:dyn) entity id(id:type:crea:dyn)")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CEntityId id2;
		id2.fromString( args[1].c_str() );

		CCharacter * c = PlayerManager.getChar( id );
		if( c )
		{
			c->pickUpItem( id2 );
			return true;
		}
	}
	return false;
}

//-----------------------------------------------
// Harvest simulation
//-----------------------------------------------
NLMISC_COMMAND( createItemInBagTest," create_item_in_bag", "player id(id:type:crea:dyn) sheet quantity quality")
{
	if( args.size() == 4)
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CSheetId sheet;

		uint32 sheetId;
		NLMISC::fromString(args[1], sheetId);
		if (sheetId)
			sheet = CSheetId(sheetId);
		else
		{
			string sheetName = args[1];
			if (sheetName.find(".") == string::npos)
				sheetName += string(".item");
			sheet = CSheetId(sheetName.c_str());
		}
		uint quantity, quality;
		NLMISC::fromString(args[2], quantity);
		NLMISC::fromString(args[3], quality);

		const CStaticItem* form = CSheets::getForm( sheet );
		if (form == NULL)
		{
			log.displayNL("invalid sheet id");
			return true;
		}

		CCharacter * c = PlayerManager.getChar( id );
		if( c )
		{
			{
//				CSheetId idSheetStack("stack.sitem");
//				CGameItemPtr stackItem = GameItemManager.createItem( idSheetStack, (uint16)1, CEntityId::Unknown, (sint16)0, false,false, CEntityId::Unknown );
//				if( stackItem == NULL )
//				{
//					nlwarning("<createItemInBag> Error while creating stack bag %s -> returned a NULL pointer", idSheetStack.toString().c_str() );
//					return false;
//				}
//				else
//				{
//					for( uint q = 0; q < quantity; ++q )
//					{
//						CGameItemPtr itemTmp = GameItemManager.createItem( sheet , 1, CEntityId::Unknown, (sint16)-1, true, true, CEntityId::Unknown );
						CGameItemPtr itemTmp = GameItemManager.createItem( sheet , 1, true, true);
						if( itemTmp == 0 )
						{
							nlwarning("<createItemInBag> Error while creating item %s -> returned a NULL pointer", sheet.toString().c_str() );
							return false;
						}
						itemTmp->setStackSize(quantity);
//					}
//					CGameItemPtr itemBefore = stackItem;
//					if( c->addItemToBag( stackItem ) )
						if( c->addItemToInventory( INVENTORIES::bag, itemTmp ) )
					{
//						nlassert(stackItem != 0);
//						if( itemBefore != stackItem )
//						{
//							nlwarning("<createItemInBag> Item are added in stack");
//						}
//						else
						{
							nlwarning("<createItemInBag> Item or part of stack are added in bag");
						}

					}
					else
					{
						nlwarning("<createItemInBag> addItemToBag return false");
					}
//				}
			}
			return true;
		}
	}
	return false;
}

/*
//-----------------------------------------------
// Dynamic variable Ryzom_Day
//-----------------------------------------------
NLMISC_DYNVARIABLE( uint32, RyzomDay, "" )
{
	if ( get )
		*pointer = PS->RyzomTime.getRyzomDay();
}

//-----------------------------------------------
// Dynamic variable Ryzom_Time
//-----------------------------------------------
NLMISC_DYNVARIABLE( float, RyzomTime, "" )
{
	if ( get )
		*pointer = PS->RyzomTime.getRyzomTime();
}
*/

//-----------------------------------------------
// Dynamic variable Character create level
//-----------------------------------------------
NLMISC_DYNVARIABLE( uint16, CharacterCreateLevel, "" )
{
	if ( get )
	{
		*pointer = PlayerManager.characterCreateLevel();
	}
	else
	{
		PlayerManager.characterCreateLevel( *pointer );
	}
}


//-----------------------------------------------
// Dynamic variable People autorized
//-----------------------------------------------
NLMISC_DYNVARIABLE( uint32, PeopleAutorized, "" )
{
	if ( get )
		*pointer = PeopleAutorisation;
	else
	{
		PeopleAutorisation = (uint8) *pointer;
	}
}

//-----------------------------------------------
// Dynamic variable Career autorized
//-----------------------------------------------
NLMISC_DYNVARIABLE( uint32, CareerAutorized, "" )
{
	if ( get )
		*pointer = CareerAutorisation;
	else
	{
		CareerAutorisation = (uint8) *pointer;
	}
}


//-----------------------------------------------
// display_database_entry :
//-----------------------------------------------
NLMISC_COMMAND(displayDatabaseEntry," display a database entry value","<entity id(id:type:crea:dyn)> <db_entry>")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *e = PlayerManager.getChar(id);
		if( e )
		{
			string entry = args[1];
			try
			{
				sint64 value = e->_PropertyDatabase.x_getProp(entry);
				log.displayNL("For player %s, buffer %s : value %"NL_I64"d", id.toString().c_str(), entry.c_str(), value );
			}
			catch (const CCDBSynchronised::ECDBNotFound &)
			{
				log.displayNL("%s isn't a valid database entry", entry.c_str());
			}
		}
		else
		{
			log.displayNL("Unknown entity %s ",id.toString().c_str());
		}
		return true;
	}
	return false;
} // display_database_entry //


//-----------------------------------------------
// db : display or set a value in the database
//-----------------------------------------------
NLMISC_COMMAND( db, "Display or set the value of a property in the database", "<entity id(id:type:crea:dyn)> <db_entry> [<value> [<dontsend>=0]]" )
{
	if ( args.size() < 2 )
		return false;

	CEntityId id;
	id.fromString( args[0].c_str() );

	CCharacter *e = PlayerManager.getChar(id);
	if ( e )
	{
		bool res;
		string entry = args[1];

		if ( args.size() == 2 )
		{
			// Display
			try
			{
				sint64 value = e->_PropertyDatabase.x_getProp( entry );
				log.displayNL( "%"NL_I64"d", value );
				res = true;
			}
			catch (const CCDBSynchronised::ECDBNotFound& )
			{
				res = false;
			}
		}
		else
		{
			// Set
			sint64 value;
			sscanf( args[2].c_str(), "%"NL_I64"d", &value );
			if ( (args.size() > 3) && (args[3]!="0") )
			{
				res = e->_PropertyDatabase.x_setPropButDontSend( entry, value );
			}
			else
			{
				res = e->_PropertyDatabase.x_setProp( entry, value );
			}
		}
		if ( ! res )
			log.displayNL( "Entry not found" );
	}
	else
	{
		log.displayNL("Unknown entity %s ",id.toString().c_str());
	}
	return true;
}


//-----------------------------------------------
// spend money
//-----------------------------------------------
NLMISC_COMMAND(spendMoney," spend money","<entity id(id:type:crea:dyn)> <money>")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *e = PlayerManager.getChar(id);
		if( e )
		{
			e->spendMoney( NLMISC::atoiInt64( args[1].c_str() ) );
		}
		else
		{
			log.displayNL("Spend money command, unknown enity '%s'", id.toString().c_str() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// give seed
//-----------------------------------------------
/*NLMISC_COMMAND(giveSeed," give seed","<entity id(id:type:crea:dyn)> <very big seed, big seed, medium seed, small seed>")
{
	if( args.size() == 5 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		sint32 vb, b, m, s;
		NLMISC::fromString(args[1], vb);
		NLMISC::fromString(args[2], b);
		NLMISC::fromString(args[3], m);
		NLMISC::fromString(args[4], s);

		CCharacter *e = PlayerManager.getChar(id);
		if( e )
		{
			e->giveSeed( CSeeds( vb, b, m, s ) );
		}
		else
		{
			log.displayNL("giveSeed command, unknown enity '%s'", id.toString().c_str() );
		}
		return true;
	}
	return false;
}*/

//-----------------------------------------------
// display money
//-----------------------------------------------
NLMISC_COMMAND(displayMoney," display_seed","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *e = PlayerManager.getChar(id);
		if( e )
		{
			log.displayNL("displayMoney: %"NL_I64"%u", e->getMoney() );
		}
		else
		{
			log.displayNL("displayMoney command, unknown enity '%s'", id.toString().c_str() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// break the spell of specified entity
//-----------------------------------------------
NLMISC_COMMAND(breakSpell,"break current spell for specified entity","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

//		CBrickSentenceManager::breakCast( id );
		return true;
	}
	return false;
}


//-----------------------------------------------
// read/set the DamageFactor
//-----------------------------------------------
NLMISC_COMMAND(damageFactor,"read or set the damage factor","new factor")
{
	if( args.size() == 0 )
	{
		log.displayNL("DamageFactor = %g", DamageFactor);
		return true;
	}
	else if( args.size() == 1 )
	{
		DamageFactor = (float)atof(args[0].c_str());
		log.displayNL("DamageFactor set to %g", DamageFactor);
		return true;
	}

	return false;
}


//-----------------------------------------------
// read/set the SpeedFactor
//-----------------------------------------------
NLMISC_COMMAND(speedFactor,"read or set the speed factor","new factor")
{
	if( args.size() == 0 )
	{
		log.displayNL("SpeedFactor = %g", SpeedFactor);
		return true;
	}
	else if( args.size() == 1 )
	{
		SpeedFactor = (float)atof(args[0].c_str());
		log.displayNL("SpeedFactor set to %g", SpeedFactor);
		return true;
	}

	return false;
}


//-----------------------------------------------
// read/set the CombatCycleLength
//-----------------------------------------------
NLMISC_COMMAND(combatCycleLength,"read or set the combat_cycle_length","new length in seconds")
{
	if( args.size() == 0 )
	{
		log.displayNL("CombatCycleLength = %g s", CombatCycleLength);
		return true;
	}
	else if( args.size() == 1 )
	{
		CombatCycleLength = (float)atof(args[0].c_str());
		log.displayNL("CombatCycleLength set to %g s", SpeedFactor);
		return true;
	}

	return false;
}


//-----------------------------------------------
// Display the list of services registered to Event reports
//-----------------------------------------------
NLMISC_COMMAND(displayRegisteredServices,"Display the list of services registered to Event reports","")
{
	const set<NLNET::TServiceId> &services = CPhraseManager::getInstance().registeredService();
	set<NLNET::TServiceId>::const_iterator it = services.begin();
	const set<NLNET::TServiceId>::const_iterator itEnd = services.end();

	log.displayNL("Registered services for event reports :");
	for ( ; it != itEnd ; ++it)
	{
		log.displayNL("	%u", it->get() );
	}

	return true;
}


//-----------------------------------------------
//Display the list of services registered to AI Event reports
//-----------------------------------------------
NLMISC_COMMAND(displayAIRegisteredServices,"Display the list of services registered to AI Event reports","")
{
	const set<NLNET::TServiceId> &services = CPhraseManager::getInstance().registeredServiceForAI();
	set<NLNET::TServiceId>::const_iterator it = services.begin();
	const set<NLNET::TServiceId>::const_iterator itEnd = services.end();

	log.displayNL("Registered services for AI event reports :");
	for ( ; it != itEnd ; ++it)
	{
		log.displayNL("	%u",it->get() );
	}

	return true;
}


//-----------------------------------------------
// Make the specified character learn all bricks
//-----------------------------------------------
/*NLMISC_COMMAND(learnAllBricks,"Specified player learn all bricks","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *e = PlayerManager.getChar(id);
		if( e )
		{
			GameItemManager.learnAllBricks( e );
			log.displayNL("learnAllBricks command, player '%s' learnt all existing bricks", id.toString().c_str() );
		}
		else
		{
			log.displayNL("learnAllBricks command, unknown player '%s'", id.toString().c_str() );
		}
		return true;
	}
	return true;
}
*/

//-----------------------------------------------
// Make the specified character learn all bricks for the speciefied role
//-----------------------------------------------
//Deprecated
/*NLMISC_COMMAND(displayJobsName,"Display all the existing jobs names","")
{
	log.displayNL("List of the %d existings jobs :",JOBS::NbJobs);
	for (uint16 i = 0 ; i < JOBS::NbJobs ; ++i)
	{
		log.displayNL("	%s", JOBS::toString( (JOBS::TJob)i).c_str() );
	}
	return true;
}
*/

//-----------------------------------------------
// Make the specified character learn all bricks for the speciefied role
//-----------------------------------------------
//Deprecated
/*NLMISC_COMMAND(learnJobBricks,"Specified player learn all bricks for the speciefied job and role (up to given level)","<entity id(id:type:crea:dyn)> <jobName> <level>")
{
	if( args.size() == 3 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		const JOBS::TJob job = JOBS::toJob( args[1] );
		if (job == JOBS::Unknown )
		{
			return false;
		}

		uint16 level;
		NLMISC::fromString(args[2], level);

		CCharacter *e = PlayerManager.getChar(id);
		if( e )
		{
//			GameItemManager.learnAllBricksForJob( e, job, level );
			log.displayNL("learnAllBricks command, player '%s' learnt all existing bricks for job %s (role %s) up to level %d", id.toString().c_str(), JOBS::toString(job).c_str(), ROLES::toString( JOBS::getAssociatedRole(job)).c_str(), level );
		}
		else
		{
			log.displayNL("learnAllBricks command, unknown player '%s'", id.toString().c_str() );
		}
		return true;
	}

	return false;
}
*/

//-----------------------------------------------
// read/set the EntityForcedDefaultLevel
//-----------------------------------------------
NLMISC_COMMAND(entityForcedDefaultLevel,"read or set the EntityForcedDefaultLevel (the entity level returned when real level is 0)","new null level")
{
	if( args.size() == 0 )
	{
		log.displayNL("EntityForcedDefaultLevel = %d", EntityForcedDefaultLevel);
		return true;
	}
	else if( args.size() == 1 )
	{
		NLMISC::fromString(args[0], EntityForcedDefaultLevel);
		log.displayNL("EntityForcedDefaultLevel set to %d", EntityForcedDefaultLevel);
		return true;
	}

	return false;
}

//-----------------------------------------------
// read/set the MaxHarvestDistance
//-----------------------------------------------
NLMISC_COMMAND(maxHarvestDistance,"read or set the MaxHarvestDistance (the max distance for a char to harvest a creature corpse)","new distance in meters")
{
	if( args.size() == 0 )
	{
		log.displayNL("MaxHarvestDistance = %f meters", MaxHarvestDistance);
		return true;
	}
	else if( args.size() == 1 )
	{
		MaxHarvestDistance = (float)atof(args[0].c_str());
		log.displayNL("MaxHarvestDistance set to %f meters", MaxHarvestDistance);
		return true;
	}

	return false;
}


//-----------------------------------------------
// read/set the MaxMountDistance
//-----------------------------------------------
NLMISC_COMMAND(maxMountDistance,"read or set the MaxMountDistance (the max distance for a char to mount a creature)","new distance in meters")
{
	if( args.size() == 0 )
	{
		log.displayNL("MaxMountDistance = %f meters", MaxMountDistance);
		return true;
	}
	else if( args.size() == 1 )
	{
		MaxMountDistance = (float)atof(args[0].c_str());
		log.displayNL("MaxMountDistance set to %f meters", MaxMountDistance);
		return true;
	}

	return false;
}



//-----------------------------------------------
// set all chosen player job levels to given value
//-----------------------------------------------
NLMISC_COMMAND(setAllJobsLevel,"set all chosen player job levels to given value (all jobs and careers will have this level)","<entity id(id:type:crea:dyn)> <new level>")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		uint8 level;
		NLMISC::fromString(args[1], level);

		CCharacter *e = PlayerManager.getChar(id);
		if( e )
		{
			log.displayNL("Player %s jobs are all set to level %u", id.toString().c_str(), level);
			for ( uint career = 0; career < 16 ; ++career )
			{
				for ( uint job = 0 ; job < 8 ; ++job)
				{
					e->_PropertyDatabase.x_setProp( (string("CHARACTER_INFO:CAREER") + toString( career ) + string(":JOB") + toString(job) + string(":JOB_CAP")).c_str(), 10*(level+1) );
				}
			}
		}
		else
		{
			log.displayNL("setAllJobsLevel command, unknown player '%s'", id.toString().c_str() );
		}

		return true;
	}

	return false;
}

//-----------------------------------------------
// set all chosen player skills to given level
//-----------------------------------------------
NLMISC_COMMAND(setAllSkillsToValue,"set all skills to value","<entity id(id:type:crea:dyn)> <value>")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		sint32 value;
		NLMISC::fromString(args[1], value);

		CCharacter *e = PlayerManager.getChar(id);
		if( e )
		{
			log.displayNL("Player %s skills are all set to value %u", id.toString().c_str(), value);
			e->setSkillsToValue(value);
		}
		else
		{
			log.displayNL("setAllSkillstoValue command, unknown player '%s'", id.toString().c_str() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// make player learn all faber plans
//-----------------------------------------------
/*NLMISC_COMMAND(learnAllFaberPlans,"learn all faber plans for given player","<entity id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *e = PlayerManager.getChar(id);
		if( e )
		{
			e->learnAllFaberPlans();
		}
		else
		{
			log.displayNL("learnAllFaberPlans command, unknown player '%s'", id.toString().c_str() );
		}
		return true;
	}
	return false;
} // learnAllFaberPlans //
*/


//-----------------------------------------------
// displayItemInInventory :
//-----------------------------------------------
NLMISC_COMMAND(displayItemInInventory,"display item characteristics (item is in a player inventory)","<player id(id:type:crea:dyn)> <inventory id or string> <slot>")
{
	// Get args
	if( args.size() != 3 )
		return false;
	CEntityId id;
	id.fromString( args[0].c_str() );
	CCharacter *player = PlayerManager.getChar(id);
	if( !player )
	{
		log.displayNL("<changeItemHP> command, unknown player '%s'", id.toString().c_str() );
		return true;
	}
	INVENTORIES::TInventory inventory = INVENTORIES::toInventory( args[1].c_str() );
	if( inventory == INVENTORIES::UNDEFINED )
	{
		sint invId;
		NLMISC::fromString(args[1], invId);
		inventory = INVENTORIES::TInventory(invId);
		if( inventory >= INVENTORIES::UNDEFINED )
			return false;
	}
	uint32 slot;
	NLMISC::fromString(args[2], slot);

	const CGameItemPtr item = player->getInventory( inventory)->getItem(slot);
	if (item != NULL)
	{
		item->displayInLog(log);
	}
	else
	{
		log.displayNL("<displayItemInInventory> cannot find item in inventory %d, slot %d, for player '%s'", inventory, slot, id.toString().c_str() );
	}

	return true;
} // displayItemInInventory //


//-----------------------------------------------
// destroyItemInInventory
//-----------------------------------------------
NLMISC_COMMAND(destroyItemInInventory,"destroy the specified item","<player id(id:type:crea:dyn)> <inventory id> <slot> <Quantity (1 by default)>")
{
	if( args.size() == 3 || args.size() == 4 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *c = PlayerManager.getChar(id);
		if( !c )
		{
			log.displayNL("<destroyItemInInventory> command, unknown character '%s'", id.toString().c_str() );
			return true;
		}
		const INVENTORIES::TInventory inventory = INVENTORIES::toInventory(args[1].c_str());
		uint32 slot;
		NLMISC::fromString(args[2], slot);

		uint32 qty = 1;
		if (args.size() == 4)
			NLMISC::fromString(args[3], qty);

		CGameItemPtr itemPtr = c->getInventory(inventory)->getItem(slot);
		if (itemPtr != NULL)
		{
			// release our ref before we destroy the item
			itemPtr = NULL;

			c->destroyItem(inventory, slot, qty);
			log.displayNL("<destroyItemInInventory> destroyed %d items in inventory %d, slot %d, for character '%s'", qty, inventory, slot, id.toString().c_str() );
		}
		else
		{
			log.displayNL("<destroyItemInInventory> destroyItemInInventory command, cannot find item in inventory %d, slot %d, for character '%s'", inventory, slot, id.toString().c_str() );
		}

		return true;
	}
	return false;
} // destroyItemInInventory //


//-----------------------------------------------
// swapItemInInventory
//-----------------------------------------------
NLMISC_COMMAND(swapItemInInventory,"swap item in specified inventory / slot","<player id(id:type:crea:dyn)> <inventory id> <slot> <inventory id> <slot> <Quantity (1 by default)>")
{
	if( args.size() == 5 || args.size() == 6 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *c = PlayerManager.getChar(id);
		if( !c )
		{
			log.displayNL("<swapItemInInventory> command, unknown character '%s'", id.toString().c_str() );
			return true;
		}

		const INVENTORIES::TInventory inventory1 = INVENTORIES::toInventory(args[1]);
		uint32 slot1;
		NLMISC::fromString(args[2], slot1);
		const INVENTORIES::TInventory inventory2 = INVENTORIES::toInventory(args[3]);
		uint32 slot2;
		NLMISC::fromString(args[4], slot2);

		uint32 qty = 1;
		if (args.size() == 6)
			NLMISC::fromString(args[5], qty);

		c->moveItem( inventory1, slot1, inventory2, slot2, qty );

		return true;
	}
	return false;
} //swapItemInInventory


//-----------------------------------------------
// MaxEngageMeleeDistance
//-----------------------------------------------
NLMISC_COMMAND(maxEngageMeleeDistance,"display/change max melee engage distance","<new distance in meters>")
{
	if( args.size() == 1 )
	{
		MaxEngageMeleeDistance = (float) atof(args[0].c_str());
		log.displayNL("<maxEngageMeleeDistance>, maxEngageMeleeDistance set to %f meters", MaxEngageMeleeDistance);
	}
	else
	{
		log.displayNL("<maxEngageMeleeDistance>, maxEngageMeleeDistance = %f meters", MaxEngageMeleeDistance);
	}

	return true;
} // maxEngageMeleeDistance //

//-----------------------------------------------
// CarriedItemsDecayRatio
//-----------------------------------------------
NLMISC_COMMAND(carriedItemsDecayRatio,"display/change the ratio of decaying of carried items (0.2 = 20% of MaxHp lost every 'rate')","<ratio>")
{
	if( args.size() == 1 )
	{
		CarriedItemsDecayRatio = (float) atof(args[0].c_str());
		log.displayNL("<carriedItemsDecayRatio>, CarriedItemsDecayRatio set to %f", CarriedItemsDecayRatio);
	}
	else
	{
		log.displayNL("<carriedItemsDecayRatio>, CarriedItemsDecayRatio = %f", CarriedItemsDecayRatio);
	}

	return true;
} // carriedItemsDecayRatio //

//-----------------------------------------------
// carriedItemsDecayRate
//-----------------------------------------------
NLMISC_COMMAND(carriedItemsDecayRate,"display/change the rate of carried items decay (in seconds)","<new rate in seconds (float value)>")
{
	if( args.size() == 1 )
	{
		CarriedItemsDecayRate = (float) atof(args[0].c_str());
		const uint32 hours = (uint32)(CarriedItemsDecayRate/3600);
		const uint32 mins = (uint32)(CarriedItemsDecayRate/60 - hours*60);
		const float s = CarriedItemsDecayRate - hours*3600 - mins*60;
		log.displayNL("<carriedItemsDecayRate>, CarriedItemsDecayRate set to %fs (%d hours, %d min, %fs)", CarriedItemsDecayRate, hours,mins,s);
	}
	else
	{
		const uint32 hours = (uint32) ( CarriedItemsDecayRate/3600.0f );
		const uint32 mins = (uint32) ( CarriedItemsDecayRate/60.0f ) - hours*60;
		const float s = CarriedItemsDecayRate - hours*3600.0f - mins*60.0f;
		log.displayNL("<carriedItemsDecayRate>, CarriedItemsDecayRate = %fs (%d hours, %d min, %fs)", CarriedItemsDecayRate, hours,mins,s);
	}

	return true;
} // carriedItemsDecayRate //



//-----------------------------------------------
// changeItemVar
//-----------------------------------------------
/*NLMISC_COMMAND(changeItemVar,"change an item var","<player id(id:type:crea:dyn)> <inventory id> <slot> <var name (HP, Quality, StandardHP, StandardQuality)> <new value>")
{
	if( args.size() != 5 )
	{
		return false;
	}

	CEntityId id;
	id.fromString( args[0].c_str() );

	CCharacter *player = PlayerManager.getChar(id);
	if( !player )
	{
		log.displayNL("<changeItemVar> command, unknown player '%s'", id.toString().c_str() );
		return true;
	}
	uint16 inventory;
	NLMISC::fromString(args[1], inventory);
	uint16 slot;
	NLMISC::fromString(args[2], slot);
	const string &var = args[3];
	sint32 value;
	NLMISC::fromString(args[4], value);

	CGameItemPtr itemPtr = player->getItem( inventory, slot);
	if (itemPtr != NULL)
	{
		if ( var == "HP")
		{
			itemPtr->setHp( value );
			log.displayNL("Item %s hp set to %d", itemPtr->getSheetId().toString().c_str(), itemPtr->hp() );
		}
		else if ( var == "Quality")
		{
			itemPtr->setQuality( (uint16)value );
			log.displayNL("Item %s quality set to %d", itemPtr->getSheetId().toString().c_str(), itemPtr->quality() );
		}
		else if ( var == "Hp" )
		{
			itemPtr->setHp( value );
			log.displayNL("Item %s HP set to %d", itemPtr->getSheetId().toString().c_str(), itemPtr->hp() );
		}
		else
		{
			log.displayNL("changeItemVar command, Invalid var '%s'", var.c_str() );
		}
	}
	else
	{
		log.displayNL("changeItemVar command, cannot find item in inventory %d, slot %d, for player '%s'", inventory, slot, id.toString().c_str() );
	}

	return true;
} // changeItemVar
*/


//-----------------------------------------------
// sendPetAnimalCommand
//-----------------------------------------------
NLMISC_COMMAND(sendPetAnimalCommand,"Send a pet animal command","<player id(id:type:crea:dyn)> <Pet index> <Command>")
{
	if( args.size() == 3 )
	{
		CEntityId id;
		uint32 petIndex;
		uint32 petCommand;

		id.fromString( args[0].c_str() );
		NLMISC::fromString(args[1], petIndex);
		NLMISC::fromString(args[2], petCommand);

		CCharacter *c = PlayerManager.getChar(id);
		if( c )
		{
			c->sendPetCommand( (CPetCommandMsg::TCommand) petCommand, petIndex, true );
		}
	}
	return false;
}


//-----------------------------------------------
// testProgression
//-----------------------------------------------
NLMISC_COMMAND(testProgression,"testProgression of skill","<player id(id:type:crea:dyn)> <Cycles between each xp gain> <skill>")
{
	TGameCycle Rate = 1;

	if( args.size() == 1 || args.size() == 3 )
	{
		CEntityId id;

		id.fromString( args[0].c_str() );

		CCharacter *c = PlayerManager.getChar(id);
		if( c )
		{
			if( args.size() == 1 ) c->TestProgression = false;
			else
			{
				c->TestProgression = SKILLS::toSkill( c->TestProgressSkill ) != SKILLS::unknown;
				NLMISC::fromString(args[1], Rate);
				c->XpGainRate = Rate;
				c->TestProgressSkill = args[2];

				if( Rate == 0)
				{
					c->TestProgression = false;
					if( SKILLS::toSkill( c->TestProgressSkill ) == SKILLS::unknown )
					{
						return false;
					}
					TReportAction ra;
					ra.ActorRowId = c->getEntityRowId();
					ra.ActionNature = ACTNATURE::CRAFT;
					ra.DeltaLvl = 0;
					ra.Skill = SKILLS::toSkill( c->TestProgressSkill );
					PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( ra );
					PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(ra);
					return true;
				}
			}
		}
		else
		{
			log.displayNL("testProgression command: character %s not found", args[0].c_str() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// Teleport a player character
//-----------------------------------------------
NLMISC_COMMAND(teleportPlayerCharacter,"Teleport online player character","<character id(id:type:crea:dyn)> <X Coordinate in meters> <Y Coordinate in meters> <Z Coordinate in meters>")
{
	if( args.size() == 4 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *c = PlayerManager.getChar(id);
		if( c )
		{
			sint32 x, y, z;
			NLMISC::fromString(args[1], x);
			x *= 1000;
			NLMISC::fromString(args[2], y);
			y *= 1000;
			NLMISC::fromString(args[3], z);
			z *= 1000;

			if( c->getEnterFlag() )
			{
				c->allowNearPetTp();
				c->tpWanted( x, y, z );
			}
			else
			{

				c->getState().X = x;
				c->getState().Y = y;
				c->getState().Z = z;
			}
			log.displayNL( "teleportPlayerCharacter command: Teleport Done !" );
		}
		else
		{
			log.displayNL("teleportPlayerCharacter command: character %s not found", args[0].c_str() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// set the life bar of an entity
//-----------------------------------------------
NLMISC_COMMAND(setHPBar,"set the value of an entity HP bar (0..100)","<entity id(id:type:crea:dyn)> <bar value>")
{
	if( args.size() == 2 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CEntityBase *entity = NULL;

		// modify the var
		switch( id.getType() )
		{
		case RYZOMID::player:
			entity = PlayerManager.getChar( id );
			break;

		case RYZOMID::npc:
		case RYZOMID::creature:
			entity = CreatureManager.getCreature( id );
			break;

		default:
			log.displayNL("Warning : Unknown entity type (nor a player nor a creature) %s", id.toString().c_str() );
			return false;
		}

		if( entity == NULL )
		{
			log.displayNL("Warning : Unknown entity id %s", id.toString().c_str() );
			return false;
		}

		sint32 barValue;
		NLMISC::fromString(args[1], barValue);

		entity->setScoreBar( SCORES::hit_points, (uint32)(barValue * 1023 / 100) );
		log.displayNL("for entity id %s, new hpBar value : %d", id.toString().c_str(), barValue );

		return true;
	}
	return false;
}


//-----------------------------------------------
// clear the given player beasts train/convoy
//-----------------------------------------------
NLMISC_COMMAND(clearBeastTrain,"disband a beasts convoy","<player id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *player = PlayerManager.getChar( id );
		if (player)
		{
			player->clearBeastTrain();
			log.displayNL("Clear the beasts convo for player %s", id.toString().c_str() );
		}
		else
		{
			log.displayNL("Unknown player %s", id.toString().c_str() );
			return false;
		}

		return true;
	}
	return false;
} // clearBeastTrain //

//-----------------------------------------------
// dumpEntityPhrases
//-----------------------------------------------
NLMISC_COMMAND(dumpEntityPhrases,"dump all infos about given entity phrases","<eid id(id:type:crea:dyn)>")
{
	if( args.size() != 1 )
	{
		return false;
	}

	CEntityId id;
	id.fromString( args[0].c_str() );
	const CEntityPhrases *phrases = CPhraseManager::getInstance().getEntityPhrases(TheDataset.getDataSetRow(id));
	if (phrases)
	{
		log.displayNL("Entity %s phrases :",id.toString().c_str());
		phrases->dumpPhrasesInfos(log);
	}
	else
	{
		log.displayNL("Entity %s has no phrases in execution",id.toString().c_str());
	}
	return true;
}

//-----------------------------------------------
// dumpMemoryStats
//-----------------------------------------------
// NLMISC_COMMAND(dumpMemoryStats,"dump the memory stats","dump file")
// {
// 	if( args.size() == 1 )
// 	{
// 		NLMEMORY::StatisticsReport(args[0].c_str(),true);
// 		return true;
// 	}
// 	return false;
// } // dumpMemoryStats //


//-----------------------------------------------
// clear the given player beasts train/convoy
//-----------------------------------------------
NLMISC_COMMAND(dumpUnaffectedFaunaGroups,"dump the un affected fauna description","")
{
	if( args.size() == 0 )
	{
		CreatureManager.dumpUnaffectedFaunaGroups(log);
		return true;
	}
	return false;
} // dumpUnaffectedFaunaDescription

//-----------------------------------------------
// displayBricksInDb
//-----------------------------------------------
NLMISC_COMMAND(displayBricksInDb,"display the bricks in DB for given player","<player id(id:type:crea:dyn)>")
{
	if( args.size() == 1 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );

		CCharacter *player = PlayerManager.getChar( id );
		if (!player)
		{
			log.displayNL("Unknown player %s", id.toString().c_str() );
			return false;
		}

		// keep set of brickIds
		set<CSheetId> bricksInDb;

		//
		char buffer[128];
		for ( uint16 family = 0 ; family < 1024 ; ++family)
		{
			sprintf( buffer, "BRICK_FAMILY:%d:BRICKS", family );
			sint64 prop = player->_PropertyDatabase.x_getProp(buffer);

			for (uint16 index = 0 ; index < 63 ; ++index)
			{
				uint64 mask = prop & ( (sint64)1 << (sint64)index);
				if (mask != 0)
				{
					const CStaticBrick* brickForm = CStaticBrick::getBrickFromFamilyIndex( family, index+1 );
					if( brickForm )
					{
						bricksInDb.insert(brickForm->SheetId);
						log.displayNL("brick %s, family %u (%s), index %u", brickForm->SheetId.toString().c_str(), family,BRICK_FAMILIES::toString( (BRICK_FAMILIES::TBrickFamily) family).c_str(), index+1 );
					}
					else
						log.displayNL("ERROR, unknown brick index %u family %u", index, family );
				}
			}
		}

		// get set of known bricks
		const set<CSheetId> &bricksKnown = player->getKnownBricks();

		log.displayNL("\nCHECK COHERENCY\n");
		// compare both sets (check coherency)
		set<CSheetId>::const_iterator it1 = bricksInDb.begin();
		set<CSheetId>::const_iterator it1End = bricksInDb.end();
		set<CSheetId>::const_iterator it2 = bricksKnown.begin();
		set<CSheetId>::const_iterator it2End = bricksKnown.end();

		uint16 size1 = (uint16)bricksInDb.size();
		uint16 size2 = (uint16)bricksKnown.size();

		bool ok = true;

		for ( ; it1 != it1End ; ++it1)
		{
			if (bricksKnown.find(*it1) == it2End)
			{
				log.displayNL("brick %s is in DB but is unknown !!!", (*it1).toString().c_str() );
				ok = false;
			}
		}
		log.displayNL("");
		for ( ; it2 != it2End ; ++it2 )
		{
			if (bricksInDb.find(*it2) == it1End)
			{
				log.displayNL("brick %s is Known but not in DB !!!", (*it2).toString().c_str() );
				ok = false;
			}
		}

		if (ok)
			log.displayNL("Coherency is ok");


		return true;
	}
	return false;
} // displayBricksInDb //

//-----------------------------------------------
// setPlayerTeam
//-----------------------------------------------
NLMISC_COMMAND(setPlayerTeam,"create a fake team with the player ","<player id(id:type:crea:dyn)>")
{
	if ( args.size() == 1 )
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CCharacter * c = PlayerManager.getChar( id );
		if (c )
		{
			if ( TeamManager.getTeam( c->getTeamId() ) != NULL)
			{
				log.displayNL("player %s is already in a team",id.toString().c_str());
			}
			else
			{
				TeamManager.addFakeTeam( c );
				log.displayNL("player %s is now in team %u",id.toString().c_str(), c->getTeamId() );
			}
		}
		else
		{
			log.displayNL("invalid player id %s",id.toString().c_str());
		}
		return true;
	}
	return false;
} // setPlayerTeam //

//-----------------------------------------------
// consumeAmmos
//-----------------------------------------------
NLMISC_COMMAND(consumeAmmos,"consume ammos for given player ","<player id(id:type:crea:dyn)> <ammo nb>")
{
	if ( args.size() == 2 )
	{
		CEntityId id;
		id.fromString(args[0].c_str());

		CCharacter * c = PlayerManager.getChar( id );
		if (c )
		{
			uint8 nb;
			NLMISC::fromString(args[1], nb);

			c->consumeAmmo(nb);
			log.displayNL("Player id %s consumed %u ammos",id.toString().c_str(), nb);
		}
		else
		{
			log.displayNL("invalid player id %s",id.toString().c_str());
		}
		return true;
	}
	return false;
} // consumeAmmos //


//-----------------------------------------------
// addTextContextMessages
//-----------------------------------------------
NLMISC_COMMAND(addTextContextMessages,"Debug : add context message ","<name><title><details>")
{
	if ( args.size() != 3 )
		return false;

	vector<TAIAlias> aliases;
	CAIAliasTranslator::getInstance()->getNPCAliasesFromName( args[0], aliases );
	if ( aliases.empty() )
	{
		log.displayNL("invalid bot name %s", args[0].c_str() );
		return false;
	}

	TAIAlias alias = aliases[0];
	CEntityId id = CAIAliasTranslator::getInstance()->getEntityId( alias );
	if ( id == CEntityId::Unknown )
	{
		log.displayNL( "invalid bot id %s (doublon?)", id.toString().c_str() );
		return false;
	}
	CCreature * c = CreatureManager.getCreature( id );
	if ( !c )
	{
		log.displayNL("invalid bot alias %s (doublon?)", CPrimitivesParser::aliasToString(alias).c_str());
		return false;
	}

	std::vector< std::pair< std::string , std::string > >& texts = (std::vector< std::pair< std::string , std::string > >&)  c->getContextTexts();
	texts.push_back( make_pair( args[1],args[2] ) );
	return true;
} // addTextContextMessages //


//-----------------------------------------------
// entity execute an AIAction on it's target or given entity
//-----------------------------------------------
NLMISC_COMMAND(executeAiAction,"execute an AIAction on it's target or given entity","<eId> <action sheet> [<target eId>]")
{
	if ( args.size() >= 2)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CEntityBase * source = CEntityBaseManager::getEntityBasePtr( id );
		if ( !source )
		{
			log.displayNL( "invalid entity %s", args[0].c_str() );
			return true;
		}

		CSheetId actionSheet(args[1]);
		if (actionSheet == CSheetId::Unknown)
		{
			log.displayNL( "invalid action sheet %s", args[1].c_str() );
			return true;
		}

		if ( args.size() >= 3)
		{
			id.fromString(args[2].c_str());
		}
		else
		{
			id = source->getTarget();
		}

		CEntityBase * target = CEntityBaseManager::getEntityBasePtr( id );
		if ( !target )
		{
			log.displayNL( "invalid entity %s", id.toString().c_str() );
			return true;
		}

		CPhraseManager::getInstance().executeAiAction( source->getEntityRowId(), target->getEntityRowId(), actionSheet);

		return true;
	}
	return false;
} // executeAiAction


//-----------------------------------------------
// Crystallize an action for enchantment
//-----------------------------------------------
NLMISC_COMMAND(crystallizeAction,"create crystallized action in temp inventory","<eId> <action sheet>")
{
	if ( args.size() == 2)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CCharacter * player = dynamic_cast< CCharacter * > ( CEntityBaseManager::getEntityBasePtr( id ) );
		if( player )
		{
			const CStaticRolemasterPhrase * phrase = CSheets::getSRolemasterPhrase( CSheetId( args[ 1 ] ) );
			if( phrase )
			{
				player->createCrystallizedActionItem( phrase->Bricks );
			}
			else
			{
				log.displayNL( "invalid action sheet %s", args[1].c_str() );
			}
		}
		else
		{
			log.displayNL( "invalid entity %s", id.toString().c_str() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// create Sap recharge item
//-----------------------------------------------
NLMISC_COMMAND(createSapRecharge,"create sap recharge item in temp inventory","<eId> <quantity Sap recharged>")
{
	if ( args.size() == 2)
	{
		CEntityId id;
		uint32 sapRecharged;
		id.fromString(args[0].c_str());
		NLMISC::fromString(args[1], sapRecharged);
		CCharacter * player = dynamic_cast< CCharacter * > ( CEntityBaseManager::getEntityBasePtr( id ) );
		if( player )
		{
			player->createRechargeItem( sapRecharged );
		}
		else
		{
			log.displayNL( "invalid entity %s", id.toString().c_str() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// enchant an item
//-----------------------------------------------
NLMISC_COMMAND(enchantItem,"enchantItem with crystallized action from bag","<eId> <slot of crystallized action in bag>")
{
	if ( args.size() == 2)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		uint16 slot;
		NLMISC::fromString(args[1], slot);
		CCharacter * player = dynamic_cast< CCharacter * > ( CEntityBaseManager::getEntityBasePtr( id ) );
		if( player )
		{
			player->enchantItem( INVENTORIES::bag, slot );
		}
		else
		{
			log.displayNL( "invalid entity %s", id.toString().c_str() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// recharge an item
//-----------------------------------------------
NLMISC_COMMAND(rechargeItem,"recharge sapLaod of an item with sap recharge item from bag","<eId> <slot of recharge item in bag>")
{
	if ( args.size() == 2)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		uint16 slot;
		NLMISC::fromString(args[1], slot);
		CCharacter * player = dynamic_cast< CCharacter * > ( CEntityBaseManager::getEntityBasePtr( id ) );
		if( player )
		{
			player->rechargeItem( INVENTORIES::bag, slot );
		}
		else
		{
			log.displayNL( "invalid entity %s", id.toString().c_str() );
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// proc enchantment from item in right hand
//-----------------------------------------------
NLMISC_COMMAND(procEnchantment,"proc enchantment of item in right hand","<eId>")
{
	if ( args.size() == 1)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CCharacter * player = dynamic_cast< CCharacter * > ( CEntityBaseManager::getEntityBasePtr( id ) );
		if( player )
		{
			player->procEnchantment();
		}
		return true;
	}
	return false;
}


//-----------------------------------------------
// stun entity
//-----------------------------------------------
NLMISC_COMMAND(stun,"stun an entity","<eId>")
{
	if ( args.size() >= 1)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CEntityBase * entity = CEntityBaseManager::getEntityBasePtr( id );
		if ( !entity )
		{
			log.displayNL( "invalid entity %s", args[0].c_str() );
			return true;
		}

		entity->stun();

		return true;
	}
	return false;
} // stun

//-----------------------------------------------
// wake entity
//-----------------------------------------------
NLMISC_COMMAND(wake,"wake an entity","<eId>")
{
	if ( args.size() >= 1)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CEntityBase * entity = CEntityBaseManager::getEntityBasePtr( id );
		if ( !entity )
		{
			log.displayNL( "invalid entity %s", args[0].c_str() );
			return true;
		}

		entity->wake();

		return true;
	}
	return false;
} // wake


//-----------------------------------------------
//hasDeathReportBeenSent for creature ?
//-----------------------------------------------
NLMISC_COMMAND(hasDeathReportBeenSent,"hasDeathReportBeenSent for creature ? ","<eId>")
{
	if ( args.size() >= 1)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CCreature * creature = CreatureManager.getCreature(id);
		if ( !creature )
		{
			log.displayNL( "invalid creature %s", args[0].c_str() );
			return true;
		}

		if (creature->deathReportHasBeenPushed())
		{
			log.displayNL( "ZOMBIE : DEATH report pushed for creature %s", args[0].c_str() );
		}
		else
		{
			log.displayNL( "ZOMBIE : DEATH report NOT pushed for creature %s", args[0].c_str() );
		}

		if (creature->deathReportHasBeenSent())
		{
			log.displayNL( "ZOMBIE : DEATH report already been sent for creature %s", args[0].c_str() );
		}
		else
		{
			log.displayNL( "ZOMBIE : DEATH report NOT sent for creature %s", args[0].c_str() );
		}

		return true;
	}
	return false;
} // hasDeathReportBeenSent

//-----------------------------------------------
// check if entity _IsDead flag is set
//-----------------------------------------------
NLMISC_COMMAND(isEntityDead,"check if entity _IsDead flag is set","<eId>")
{
	if ( args.size() >= 1)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CEntityBase * entity = CEntityBaseManager::getEntityBasePtr( id );
		if ( !entity )
		{
			log.displayNL( "invalid entity %s", args[0].c_str() );
			return true;
		}

		if (entity->isDead())
		{
			log.displayNL( "Entity %s is DEAD", args[0].c_str() );
		}
		else
		{
			log.displayNL( "Entity %s is still ALIVE", args[0].c_str() );
		}

		return true;
	}
	return false;
} // isEntityDead



//-----------------------------------------------
// display wearPenalty of a character
//-----------------------------------------------
NLMISC_COMMAND(displayWearPenalty,"display wear penalty","<eId>")
{
	if ( args.size() == 1)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CCharacter * player = dynamic_cast< CCharacter * > ( CEntityBaseManager::getEntityBasePtr( id ) );
		if ( !player )
		{
			log.displayNL( "invalid player character %s", args[0].c_str() );
			return true;
		}
		log.displayNL(" Wear Penalty for character %s is %1.2f", args[0].c_str(), player->wearMalus() );
		return true;
	}
	return false;
} // displayWearPenalty


//-----------------------------------------------
// display wearPenalty of a character
//-----------------------------------------------
NLMISC_COMMAND(displayPowerFlags,"displayPowerFlags","<eId>")
{
	if ( args.size() == 1)
	{
		CEntityId id;
		id.fromString(args[0].c_str());
		CCharacter * player = dynamic_cast< CCharacter * > ( CEntityBaseManager::getEntityBasePtr( id ) );
		if ( !player )
		{
			log.displayNL( "invalid player character %s", args[0].c_str() );
			return true;
		}
		player->displayPowerFlags();
		return true;
	}
	return false;
} // displayPowerFlags


//-----------------------------------------------
// get the total number of SP a player can gain beyond a given skill
//-----------------------------------------------
NLMISC_COMMAND(displaySPGainBeyondSkill,"displaySPGainBeyondSkill","<skill name>")
{
	if ( args.size() == 1)
	{
		SKILLS::ESkills skill = SKILLS::toSkill(args[0]);
		if (skill == SKILLS::unknown)
		{
			log.displayNL("Skill %s is unknown", args[0].c_str() );
			return true;
		}

		// get pointer on static skills tree definition
		CSheetId sheet("skills.skill_tree");
		const CStaticSkillsTree * SkillsTree = CSheets::getSkillsTreeForm( sheet );
		if( !SkillsTree )
			return true;

		uint32 SP = SkillsTree->getTreeSkillPointsUnderSkill(skill);

		log.displayNL("Total skill points (FACTOR NOT INCLUDED !) beyond skill %s : %u", args[0].c_str(), SP);

		return true;
	}
	return false;
} // displaySPGainBeyondSkill


NLMISC_COMMAND (displayInfosOnEntity, "display info on entity", "<eid>")
{
	if (args.size () != 1)
		return false;

	CEntityId eid (args[0]);

	CEntityBase	*entity = CEntityBaseManager::getEntityBasePtr(eid);
	if(entity == 0)
	{
		log.displayNL ("Unknown entity %s", eid.toString().c_str());
		return false;
	}

	CAIAskForInfosOnEntityMsg msg;
	msg.EntityRowId = entity->getEntityRowId();
	if ( TheDataset.isAccessible( msg.EntityRowId ))
	{
		CMirrorPropValueRO<uint32>	instanceNumber(TheDataset, msg.EntityRowId, DSPropertyAI_INSTANCE);
		CWorldInstances::instance().msgToAIInstance(instanceNumber, msg);
//	msg.send("AIS");
	}
	else
	{
		log.display("The ientity row id is invalid. Can't send the message.");
	}


	return true;
}

NLMISC_COMMAND (createEffectOnEntity, "create given effect on entity", "<eid> <effectFamily> <EffectDuration (s)> <effect value>")
{
	if (args.size () != 4)
		return false;

	CEntityId eid (args[0]);
	CEntityBase	*entity = CEntityBaseManager::getEntityBasePtr(eid);
	if(entity == 0)
	{
		log.displayNL ("Unknown entity %s", eid.toString().c_str());
		return false;
	}

	EFFECT_FAMILIES::TEffectFamily family = EFFECT_FAMILIES::toEffectFamily( args[1] );
	if (family == EFFECT_FAMILIES::Unknown)
	{
		log.displayNL ("Unknown effect family %s", args[1].c_str());
		return false;
	}

	uint32 duration;
	NLMISC::fromString(args[2], duration);
	duration = (uint32)(NLMISC::TGameCycle(duration) / CTickEventHandler::getGameTimeStep());
	uint32 value;
	NLMISC::fromString(args[3], value);

	CSTimedEffect *effect = IEffectFactory::buildEffect(family);
	if (effect)
	{
		effect->setFamily(family);
		effect->setTargetRowId(entity->getEntityRowId());
		effect->setParamValue(value);
		effect->setEndDate( CTickEventHandler::getGameCycle() + duration );

		entity->addSabrinaEffect(effect);

		log.displayNL ("Create effect family %s", args[1].c_str());
	}
	else
		log.displayNL ("FAILED to create effect family %s", args[1].c_str());

	return true;
}

NLMISC_COMMAND (checkTeleportItems, "check all teleport items", "")
{
	if (args.size () != 0)
		return false;
	CAllStaticItems::const_iterator it = CSheets::getItemMapForm().begin();
	for (; it != CSheets::getItemMapForm().end(); ++it )
	{
		uint16 idx = CZoneManager::getInstance().getTpSpawnZoneIdByName( (*it).second.Destination );
		if ( CZoneManager::getInstance().getTpSpawnZone( idx ) == NULL )
		{
			log.displayNL("invalid zone '%s' in item '%s'",(*it).second.Destination.c_str(), (*it).first.toString().c_str() );
		}
	}
	return true;
}

NLMISC_COMMAND (dumpRespawnPoints, "dump all the respawn points of a user", "")
{
	if (args.size () != 1)
		return false;

	CEntityId id;
	id.fromString( args[0].c_str() );

	CCharacter *c = PlayerManager.getChar(id);
	if (!c || !c->getEnterFlag())
	{
		log.displayNL("Invalid character");
		return true;
	}

	c->getRespawnPoints().dumpRespawnPoints(log);

	return true;
}


NLMISC_COMMAND (deleteFile, "ask to backup service to 'delete' a file (really delete or make backup)", "<file with path relative to SaveFilesDirectory> <keepBackup=1> <globalDir=0>")
{
	if (args.size() < 1)
		return false;

	bool keepBackupOfFile = true;
	if (args.size() > 1)
		keepBackupOfFile = (args[1]!="0");

	bool globalDir = false;
	if (args.size() > 2)
		globalDir = (args[2]!="0");

	if (globalDir)
		BsiGlobal.deleteFile( args[0], keepBackupOfFile );
	else
		Bsi.deleteFile( args[0], keepBackupOfFile );

	return true;
}



//-----------------------------------------------
// skillProgressionFactor : change or just display the Factor for the Skill Progression.
//-----------------------------------------------
NLMISC_VARIABLE(float,SkillProgressionFactor,"change or just display the Factor for the Skill Progression.");


/*
NLMISC_COMMAND(skillProgressionFactor, "change or just display the Factor for the Skill Progression.","<float> = new skill progression factor (not needed if only want to display it).")
{
	// This command should have 0 or 1 parameter.
	if( args.size() > 1 )
		return false;
	// Change Value
	if(args.size() == 1)
		SkillProgressionFactor = (float)atof(args[0].c_str());
	// Display
	nlinfo( "EGS_CMD: SkillProgressionFactor is now : '%f'.", SkillProgressionFactor );
	// Command well done.
	return true;
}// skillProgressionFactor //
*/

//-----------------------------------------------
// defaultCastingTime : change or just display the default casting time.
//-----------------------------------------------
NLMISC_DYNVARIABLE(float,DefaultCastingTime,"change or just display the Factor for the Skill Progression.")
{
	if(get)
	{
		*pointer = CMagicPhrase::defaultCastingTime();
	}
	else
	{
		CMagicPhrase::defaultCastingTime(*pointer);
	}
}

NLMISC_COMMAND (displayPhraseManagerStats, "display current and max nb of entities in phrase manager", "")
{
	log.displayNL("Current number of entities in phrase manager : %u", CPhraseManager::getInstance().getNbEntitiesInManager());
	log.displayNL("Max number of entities in phrase manager : %u", CPhraseManager::getInstance().getMaxNbEntitiesInManager());

	return true;
}

/*
NLMISC_COMMAND(defaultCastingTime, "change or just display the default casting time.","<float> = new default casting time (not needed if only want to display it).")
{
	// This command should have 0 or 1 parameter.
	if( args.size() > 1 )
		return false;
	// Change Value
	if(args.size() == 1)
		CMagicPhrase::defaultCastingTime((float)atof(args[0].c_str()));
	// Display
	nlinfo( "EGS_CMD: Default Casting Time is now : '%f'.", CMagicPhrase::defaultCastingTime() );
	// Command well done.
	return true;
}// defaultCastingTime //
*/


NLMISC_COMMAND( loadResaveAndCheckCharacters, "Check EGS can load all saves, and that items are not broken once they are saved", "" )
{
	vector< string > files;
	CPath::getPathContent( BsiGlobal.getLocalPath() + "characters", true, false, true, files );
	loadAndResaveCheckCharacters( files, log, true );
	return true;
}

NLMISC_COMMAND(setAnimalPeople, "refresh all the animal people","<eid>")
{
	if (args.size() != 1)
		return false;

	CEntityId id;
	id.fromString( args[0].c_str() );

	CCharacter *c = PlayerManager.getChar(id);
	if ( c && c->getEnterFlag() )
	{
		c->setAnimalPeople( 0 );
		c->setAnimalPeople( 1 );
		c->setAnimalPeople( 2 );
		c->setAnimalPeople( 3 );
	}
	return true;
}

NLMISC_COMMAND(execScript, "Execute a script file (.cmd)","<FileName>")
{
	if (args.size() != 1)
		return false;

	const string & fileName = args[0];
	CIFile iFile;

	if (iFile.open(fileName) || iFile.open(CPath::lookup(fileName, false)))
	{
		if (iFile.getFileSize() > 1000000)
		{
			log.displayNL("script '%s' is too big!", iFile.getStreamName().c_str());
			return true;
		}

		string script;
		script.resize(iFile.getFileSize());
		iFile.serialBuffer((uint8 *)&script[0], (uint)script.size());


		vector<string> scriptLines;
		string scriptLine;

		bool inSLComment = false; // in single line comment
		bool inMLComment = false; // in multi lines comment
		for (uint i = 0; i < script.size(); i++)
		{
			// remove comments
			if (inMLComment)
			{
				if (i+1 < script.size() && script[i] == '*' && script[i+1] == '/')
				{
					inMLComment = false;
					i++;
				}
				continue;
			}

			if (inSLComment)
			{
				if (script[i] == '\n')
				{
					inSLComment = false;
					if (!scriptLine.empty())
						scriptLines.push_back(scriptLine);
					scriptLine.clear();
				}
				continue;
			}

			// find comments in begin of lines only
			if (scriptLine.empty() && i+1 < script.size() && script[i] == '/')
			{
				if (script[i+1] == '*')
				{
					inMLComment = true;
					i++;
					continue;
				}
				if (script[i+1] == '/')
				{
					inSLComment = true;
					i++;
					continue;
				}
			}

			switch (script[i])
			{
			case '\r':
				break;

			case '\n':
				if (!scriptLine.empty())
					scriptLines.push_back(scriptLine);
				scriptLine.clear();
				break;

			default:
				scriptLine.push_back(script[i]);
			}
		}
		if (!scriptLine.empty())
			scriptLines.push_back(scriptLine);

		for (uint i = 0; i < scriptLines.size(); i++)
		{
			ICommand::execute(scriptLines[i], log);
		}
	}
	else
	{
		log.displayNL("Cannot open file '%s'", fileName.c_str());
	}

	return true;
}

NLMISC_COMMAND(fixAltar, "Remove fames restrictions on altar accessible to neutral","")
{
	CreatureManager.fixAltar();
	return true;
}


//NLMISC_COMMAND( dumpVectorMemory, "Dump memory used in vector", "")
//{
//	static char buffer[1024*16];
//	_vector_memory_usage::dump_memory(buffer, 1024*16);
//
//	vector<string> lines;
//	explode(buffer, "\n", lines);
//
//	log.displayNL("Vector memory usage (%u classes):", buffer, lines.size());
//
//	for (uint i=0; i<lines.size(); ++i)
//	{
//		log.displayNL("  %s", lines[i].c_str());
//	}
//
//	return true;
//}
