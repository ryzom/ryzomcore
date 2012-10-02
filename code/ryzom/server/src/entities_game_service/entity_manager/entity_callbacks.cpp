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

// net
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/eid_translator.h"

#include "nel/net/login_cookie.h"
// game share
#include "game_share/ryzom_entity_id.h"
#include "game_share/mode_and_behaviour.h"
#include "server_share/msg_brick_service.h"
#include "server_share/event_report.h"
#include "game_share/loot_harvest_state.h"
#include "game_share/msg_client_server.h"
#include "server_share/combat_state.h"
#include "game_share/seeds.h"
#include "game_share/client_action_type.h"
#include "server_share/mail_forum_validator.h"
#include "server_share/used_continent.h"
#include "game_share/entity_types.h"
#include "game_share/mainland_summary.h"
#include "game_share/shard_names.h"
#include "server_share/testing_tool_structures.h"

#include "server_share/r2_vision.h"
#include "game_share/r2_share_itf.h"

#include "phrase_manager/phrase_utilities_functions.h"
#include "entities_game_service.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "player_manager/character_encyclopedia.h"
#include "game_item_manager/player_inv_temp.h"
#include "player_manager/admin_properties.h"
#include "phrase_manager/phrase_manager.h"
#include "creature_manager/creature_manager.h"
#include "game_event_manager.h"
#include "team_manager/team_manager.h"
#include "entity_manager/entity_callbacks.h"
#include "egs_mirror.h"
#include "mission_manager/mission_manager.h"
#include "zone_manager.h"
#include "player_manager/character_respawn_points.h"
#include "guild_manager/fame_manager.h"
#include "building_manager/building_manager.h"
#include "world_instances.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_manager/pvp_manager_2.h"
#include "pvp_manager/pvp.h"
#include "egs_variables.h"
#include "guild_manager/guild_char_proxy.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
//#include "name_manager.h"
#include "shop_type/offline_character_command.h"
#include "harvest_source.h"
#include "mission_manager/mission_queue_manager.h"
#include "modules/shard_unifier_client.h"
#include "modules/character_control.h"

#include "egs_sheets/egs_sheets.h"
#include "server_share/log_player_gen.h"
#include "server_share/log_character_gen.h"
#include "server_share/log_item_gen.h"



using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CTeamManager					TeamManager;
extern CPlayerManager				PlayerManager;
extern CCreatureManager				CreatureManager;
extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern CSheetId						FyrosMeleeFighterForcedSheetId;
extern CPlayerService				*PS;
extern float						MaxHarvestDistance;

extern uint8						PeopleAutorisation;
extern uint8						CareerAutorisation;

extern std::string DummyOfflineCommand;

extern CVariable<string>	NeverAggroPriv;
extern CVariable<string>	AlwaysInvisiblePriv;

extern vector<CMainlandSummary>		Mainlands;

extern CVariable<bool> DontUseSU;

// For shard names
extern NLMISC::CVariable<uint32> FixedSessionId;

// Send to client if name is valide (true for valide)
void sendIfNameIsValide( uint32 userId, bool nameValide );

// finish create char
void cbCreateChar_part2(uint32 userId, const CCreateCharMsg &createCharMsg, bool ok);

// (!IsRingShard only) (For mainland shard)
// This parameter controls what we do we characters coming from another mainland shard.
// It allows to test character files from a live shard on a private shard.
// If the stored session id does not match the FixedSessionId:
// AllowCharsFromAllSessions = 0; // => teleport to the stored session id.
// AllowCharsFromAllSessions = 1; // => let the character play anyway, but leave the stored session id unchanged.
// AllowCharsFromAllSessions = 2; // => assign the stored session id with FixedSessionId and let play.
NLMISC::CVariable<uint32> AllowCharsFromAllSessions( "egs", "AllowCharsFromAllSessions",
"(!IsRingShard only) (For mainland shard) If the stored session id does not match the FixedSessionId: 0: teleport to the stored session id; 1: let the character play anyway, but leave the stored session id unchanged; 2: assign the stored session id with FixedSessionId and let play", 0, 0, true );


/*
//---------------------------------------------------
// Debug stuff :
//---------------------------------------------------

#define IFLOG(eid) for (int ___i=0;___i<VerboseCallBackLogEntities.size();++__i) if (VerboseCallBackLogEntities[__i]==eid)
static std::vector <NLMISC::CEntityId> VerboseCallBackLogEntities;


NLMISC_COMMAND(verboseCallbackLogAdd,"turn on verbose entity callback logging for an entity","<entity id>")
{
	if( args.size() != 1 ) return false;
	VerboseCallBackLogEntities.push_back(NLMISC::CEntityId(args[0]));
	return true;

} // dump_objects //

NLMISC_COMMAND(verboseCallbackLogRemove,"turn off verbose entity callback logging for an entity","<entity id>")
{
	if( args.size() != 1 ) return false;
	VerboseCallBackLogEntities.push_back(NLMISC::CEntityId(args[0]));
	return true;

} // dump_objects //

NLMISC_COMMAND(verboseCallbackLogAll,"turn on verbose entity callback logging for all entities","")
{
	if( args.size() != 1 ) return false;
	VerboseCallBackLogEntities.push_back(NLMISC::CEntityId(args[0]));
	return true;

} // dump_objects //

  NLMISC_COMMAND(verboseCallbackLogClear,"turn off verbose entity callback loggin for all entities","")
{
	if( args.size() != 1 ) return false;
	VerboseCallBackLogEntities.push_back(NLMISC::CEntityId(args[0]));
	return true;


} // dump_objects //
*/


/////////////////////// Callback for characters messages //////////////////////
/////////////////////// Callback for characters messages //////////////////////
/////////////////////// Callback for characters messages //////////////////////

//---------------------------------------------------
// cbClientConnection :
// when a client connects to shard, we send him his characters(if they exist)
//---------------------------------------------------
void cbClientConnection(CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientConnection);

	uint32 userId;
	string userName, userPriv, userExtended, languageId;
	string fromAddr;
	bool AllAutorized = false;
	CLoginCookie	cookie;

	msgin.serial( userId );
	msgin.serial( userName, userPriv, userExtended, languageId);
	msgin.serial( fromAddr );
	msgin.serial( cookie );
#ifdef NL_DEBUG
	egs_ecinfo("WEB: received cookie %s from player %u", cookie.toString().c_str(), userId);
#endif
	//Bsi.append( StatPath, NLMISC::toString("[UC] %u %s %s %s %s %s", userId, userName.c_str(), userPriv.c_str(), languageId.c_str(), fromAddr.c_str(), cookie.toString().c_str()) );
	//EgsStat.displayNL("[UC] %u %s %s %s %s %s", userId, userName.c_str(), userPriv.c_str(), languageId.c_str(), fromAddr.c_str(), cookie.toString().c_str());
//	EGSPD::userConnected(userId, userName, userPriv, languageId, fromAddr, cookie.toString());
	log_Player_Connect(userId, fromAddr);

	if( PlayerManager.getStallMode() == true && userPriv.size() == 0 )
	{
		egs_ecinfo("Server are in stall mode, refuse connection for user %u", userId);
		CMessage msgOut("DISCONNECT_CLIENT");
		msgOut.serial( userId );
		CUnifiedNetwork::getInstance()->send( serviceId, msgOut );
		return;
	}

	// TODO Login service send restriction (restricted account = false, true for all authorized)
//	msgin.serial( AllAutorized );

	CPlayer *player = PlayerManager.getPlayer( userId );
	if( player != 0 )
	{
		nlwarning("<cbClientConnection> User %u Received client connection but is already connected !!!", userId );
		CMessage msgOut("DISCONNECT_CLIENT");
		msgOut.serial( userId );
		CUnifiedNetwork::getInstance()->send( serviceId, msgOut );
		return;
//		disconnectUser(userId);
	}

	// set the front end id of this player
	PlayerManager.setPlayerFrontEndId( userId, serviceId );

	const bool betaTester = (userExtended.find(":FBT:") != string::npos);
	const bool preOrder = (userExtended.find(":PO:") != string::npos);
	const bool windermeerCommunity = (userExtended.find(":WIND:") != string::npos);
	const bool trialPlayer = (userExtended.find(":TRB:") != string::npos);

	// load player infos
	player = new CPlayer;
	player->setId( userId );
	player->setUserName( userName );
	player->setUserPriv ( userPriv );
	player->setUserLanguage(languageId);
	player->isBetaTester(betaTester);
	player->isPreOrder(preOrder);
	player->isWindermeerCommunity(windermeerCommunity);
	player->isTrialPlayer(trialPlayer);

	// add player
	PlayerManager.addPlayer( userId, player );

	if (!UseAsyncBSPlayerLoading.get())
	{
		// load character of player
		PlayerManager.loadPlayer( player );

		// set status connexion of player to connected
		player->setPlayerConnection( true );

		// set player cookie, for later web activation
		player->setLoginCookie( cookie );

		NLNET::TServiceId frontEndId = PlayerManager.getPlayerFrontEndId( player->getUserId() );
		// tell IOS which language the user chose
		CMessage msgLang("USER_LANGUAGE");
		msgLang.serial( userId );
		msgLang.serial( frontEndId );
		msgLang.serial( languageId );
		CUnifiedNetwork::getInstance()->send("IOS",msgLang);

		player->updateCharactersInRingDB();

		// character summary is sent when SU tell that all name errors have been fixed
//		sendCharactersSummary( player, AllAutorized );
	}
	else
	{
		PlayerManager.asyncLoadPlayer(player, userId, languageId, cookie, AllAutorized);
	}
}

//---------------------------------------------------
// cbClientEnter : //TODO at this time, this callback is not used
//
//---------------------------------------------------
//void cbClientEnter( CMessage& msgin, const std::string &serviceName, uint16 serviceId )
//{
///*
//	// read the player id
//	uint64 longUserId;
//	uint32 userId;
//	CEntityId charId;
//
//	msgin.serial( longUserId );
//	userId = (uint32) longUserId;
//
//	CCharacter * c = PlayerManager.getActiveChar( userId );
//	if( c != 0 )
//	{
//		charId = c->getId();
//
//		// send userId/charId association to the FE
//		CMessage msgout("CL_ID");
//		msgout.serial(userId);
//		msgout.serial( charId );
//		uint16 frontEndId = PlayerManager.getPlayerFrontEndId( userId );
//		CUnifiedNetwork::getInstance()->send(frontEndId, msgout);
//
//		// send acknowledge to client for received Enter message
//		CBitMemStream bms;
//		if ( ! GenericMsgManager.pushNameToStream( "CONNECTION:READY", bms) )
//		{
//			nlwarning("<cbSelectChar> Msg name CONNECTION:READY not found");
//			return;
//		}
//		CMessage msgout2( "IMPULSION_ID" );
//		msgout2.serial( charId );
//		msgout2.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
//		CUnifiedNetwork::getInstance()->send( frontEndId, msgout2 );
//
//		egs_ecinfo("Mapping UID %u => Sid %s", userId, charId.toString().c_str() );
//
//		// Remove user language from IOS, we don't need it anymore
//		CMessage msgout3("REMOVE_USER_LANGUAGE");
//		msgout3.serial(userId);
//		CUnifiedNetwork::getInstance()->send("IOS", msgout3);
//	}
//*/
//} // cbEnter //



//---------------------------------------------------
// cbClientReady :
//
//---------------------------------------------------
void cbClientReady( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbClientReady);

	// read the player id
	CEntityId characterId;
	msgin.serial( characterId );
	CCharacter *c = PlayerManager.getChar( characterId );
	if( c == 0 )
	{
		nlwarning("<cbClientReady> Can't find character %s", characterId.toString().c_str() );
		disconnectUser( PlayerManager.getPlayerId( characterId ) );
		return;
	}

	// add the character in the GPMS
	COfflineEntityState state;
	PlayerManager.getState( characterId ).setCOfflineEntityState( state ); // state properties with temp storage
	uint32 type = PlayerManager.getType( characterId ).asInt();

	CPlayer	*player = PlayerManager.getPlayer( PlayerManager.getPlayerId( c->getId() ) );
	if ( player == NULL )
	{
		nlwarning( "invalid player for char %s", c->getId().toString().c_str() );
		disconnectUser( PlayerManager.getPlayerId( characterId ) );
		return;
	}

	{
		// validate player web account
		// \todo this is unsafe, because name is an ucstring which might be fucked up when casted into string
		CMailForumValidator::validateUserEntry( c->getHomeMainlandSessionId(), c->getName().toString(), player->getLoginCookie().toString() );

		NLNET::CMessage	msgout( "IMPULSION_ID" );
		CEntityId		id = c->getId();
		msgout.serial( id );
		CBitMemStream bms;
		if ( GenericMsgManager.pushNameToStream( "CONNECTION:SHARD_ID", bms) )
		{
			uint32	shardId = IService::getInstance()->getShardId();
			bms.serial(shardId);

			CConfigFile::CVar	*var = IService::getInstance()->ConfigFile.getVarPtr("WebSrvHost");
			string	webHost = (var != NULL ? var->asString() : "");
			bms.serial(webHost);

			msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
			CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(c->getId().getDynamicId()), msgout ); // sendMessageViaMirror() not needed to FS in general
		}
		else
		{
			nlwarning("Unable to send CONNECTION:SHARD_ID to player %s", id.toString().c_str());
		}

	}

	// Add player in the mirror // createEntity() will produce warnings if it fails
	if ( ! Mirror.createEntity( characterId ) )
	{
		nlwarning( "Unable to spawn player %s in mirror, disconnecting the player", c->getId().toString().c_str() );
		disconnectUser( PlayerManager.getPlayerId( characterId ) );
		return;
	}

	// check name manager integrity
//	CNameManager::getInstance()->checkCharacterSlot( c->getId(), c->getName() );

	TDataSetRow entityIndex = TheDataset.getDataSetRow( characterId );

	c->setState( state );
	c->mirrorizeEntityState(); // write the initial position into the mirror
	//c->loadSheetEntity( CSheetId(type) ); // commented out because that was wrong!
	c->addPropertiesToMirror( entityIndex, true );
	// send CEntityId/name association to the IOS
	c->registerName();

	// deal with fame dataset.
	TheFameDataset.declareEntity(TheFameDataset.getDataSetRow(characterId));
	CFameManager::getInstance().addPlayer(characterId, c->getPlayerFamesContainer(), c->getRace());

	// get the predefined startup instance
	uint32 in = c->getStartupInstance();
	if (in != INVALID_AI_INSTANCE)
	{
		nldebug("Setting player %s in ring instance #%u",
			c->getId().toString().c_str(),	in);

		// set instance id for ring session
		c->setRingShardInstanceNumber(in);
	}
	else
	{
		// startup instance not predefined, compute it from the player position
		CContinent * cont = CZoneManager::getInstance().getContinent( state.X,state.Y );
		if ( cont )
		{
			in = CUsedContinent::instance().getInstanceForContinent( (CONTINENT::TContinent)cont->getId() );
		}
		if ( in == INVALID_AI_INSTANCE)
		{
			nlwarning("user %s is in invalid continent '%s'", c->getId().toString().c_str(), strlwr( CONTINENT::toString(c->getCurrentContinent()) ).c_str() );
		}

		nldebug("Setting player %s in instance #%u for continent '%s'",
			c->getId().toString().c_str(),	in,	strlwr( CONTINENT::toString(c->getCurrentContinent())).c_str());
		c->setInstanceNumber( in );
	}

	if( !IsRingShard ) // for ring shard, we must doesn't spawn pet.
		c->respawnPet();

	c->initAnimalHungerDb();

	c->initFactionPointDb();
	c->initPvpPointDb();
	c->initOrganizationInfos();

	c->updateOutpostAdminFlagInDB();

	if ( !player->getUserPriv().empty() && !player->havePriv(":DEV:") )
	{
		const std::string & privStr = player->getUserPriv();
		PlayerManager.addSpecialUser( privStr,c->getId() );
		c->getAdminProperties().init();
	}
	if (player->havePriv(NeverAggroPriv))
	{
		c->setGodMode( true );
		c->setAggroableOverride(0);
	}
	else
	{
		// reset with loaded value to activate effect if needed
		c->setGodMode(c->getGodModeSave());
		c->setAggroableOverride(c->getAggroableSave());
	}

	if ( !player->getUserPriv().empty() && player->havePriv(":SGM:GM:VG:SG:G:PR:OBSERVER:") )
	{
		c->setInvulnerableMode( true );
	}

	// set the invisibility mode
	if (IsRingShard)
	{
		static const bool visibleToMobs = false;
		uint64 whoSeesMe= R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_PLAYER,visibleToMobs);
		if( player->havePriv(":SG:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(c->getInvisibility()? R2_VISION::WHOSEESME_VISIBLE_SG:R2_VISION::WHOSEESME_INVISIBLE_SG,visibleToMobs);
		if( player->havePriv(":VG:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(c->getInvisibility()? R2_VISION::WHOSEESME_VISIBLE_VG:R2_VISION::WHOSEESME_INVISIBLE_VG,visibleToMobs);
		if( player->havePriv(":EG:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(c->getInvisibility()? R2_VISION::WHOSEESME_VISIBLE_EG:R2_VISION::WHOSEESME_INVISIBLE_EG,visibleToMobs);
		if( player->havePriv(":EM:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(c->getInvisibility()? R2_VISION::WHOSEESME_VISIBLE_EM:R2_VISION::WHOSEESME_INVISIBLE_EM,visibleToMobs);
		if( player->havePriv(":GM:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(c->getInvisibility()? R2_VISION::WHOSEESME_VISIBLE_GM:R2_VISION::WHOSEESME_INVISIBLE_GM,visibleToMobs);
		if( player->havePriv(":SGM:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(c->getInvisibility()? R2_VISION::WHOSEESME_VISIBLE_SGM:R2_VISION::WHOSEESME_INVISIBLE_SGM,visibleToMobs);
		if( player->havePriv(":DEV:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(c->getInvisibility()? R2_VISION::WHOSEESME_VISIBLE_DEV:R2_VISION::WHOSEESME_INVISIBLE_DEV,visibleToMobs);
	//	if (player->havePriv(AlwaysInvisiblePriv))	whoSeesMe = R2_VISION::buildWhoSeesMe(c->getInvisibility()? R2_VISION::WHOSEESME_VISIBLE_SG:R2_VISION::WHOSEESME_INVISIBLE_SG,false);
		c->setWhoSeesMe(whoSeesMe);
	}
	else
	{
		if (c->getInvisibility())
		{
			c->setWhoSeesMe(UINT64_CONSTANT(0x0000000000000000));
		}
		else
		{
			c->setWhoSeesMe(UINT64_CONSTANT(0xffffffffffffffff));
		}
	}

	if (!IsRingShard && player->havePriv(AlwaysInvisiblePriv))
	{
		c->setWhoSeesMe(uint64(0));
		c->setInvisibility(true);
	}

	TheDataset.declareEntity( entityIndex ); // after the writing of properties to mirror

//#ifdef NL_DEBUG
	egs_ecinfo("Client ready (entity %s (Row %u) added to mirror)", characterId.toString().c_str(), entityIndex.getIndex() );
//#endif

	// player enter the game
	PlayerManager.setEnterFlag( characterId, true );

	// ask backup for offline commands file
	COfflineCharacterCommand::getInstance()->characterOnline( characterId );

	c->onConnection();
} // cbClientReady //


//---------------------------------------------------
// finalizeClientReady :
//
//---------------------------------------------------
void finalizeClientReady( uint32 userId, uint32 index )
{
	H_AUTO(finalizeClientReady);

	CPlayer *player = PlayerManager.getPlayer(userId);
	BOMB_IF( player == 0,"Failed to get player from user Id",return );

	BOMB_IF( index==-1, "Failed to find char with given index for this player", return );

	// get the char infos
	CCharacter * c = PlayerManager.getChar( userId, index );
	BOMB_IF( c == NULL, toString("Character %u of user %u not created", index, userId), return);

	// get character CEntityId
	CEntityId characterId = c->getId();

	c->sendDynamicSystemMessage( c->getId(), "OPS_WELCOME" );
//	c->sendMessageToClient( c->getId(), "OPS_WELCOME" );

	c->sendMessageOfTheDay();

	c->checkSkillTreeForLockedSkill();

	c->setDatabase();

	// add recipient for new character
	DbGroupGlobal.addRecipient( c->getId() );

	CBuildingManager::getInstance()->registerPlayer( c );

	// Check Client Phrases
	c->checkPhrases();
	// send the known and memorized phrases
	c->sendPhrasesToClient();

	// send FBT status
	c->sendBetaTesterStatus();

	// send Windermeer status
	c->sendWindermeerStatus();

	if ( c->getGuildId() != 0 )
	{
		CGuild * guild = CGuildManager::getInstance()->getGuildFromId( c->getGuildId() );
		if ( guild )
		{
			CFameManager::getInstance().setPlayerGuild( c->getId(),guild->getEId(), false );
			/// todo guild : send startup message
			///guild->sendStartupMessage( c->getId() );
		}
		else
		{
			nlwarning("<cbclientReady : user %s is not in his guild>");
		}
	}

	c->resetFameDatabase();

	// notify player respawn points system that player is ready
	// WARNING: do this after setting fame manager, because available respawn points depend on fame
	c->getRespawnPoints().cbClientReady();

	CGuildCharProxy guildProxy( c );
	CGuildManager::getInstance()->playerConnection( guildProxy );

	c->updateSavedMissions();

	// force zone update for the player to set its region
	CZoneManager::getInstance().updateCharacterPosition(c);

	// assign the welcome mission at the first connection
	// assignWelcomeMission() gives the welcome mission the first time, then it does nothing
	c->assignWelcomeMission();

	// ANTIBUG used to avoid player spawning in invalid zones
	if ( CorrectInvalidPlayerPositions == true )
	{
		CContinent * cont = CZoneManager::getInstance().getContinentFromId( c->getCurrentContinent() );
		if ( cont == NULL || c->getCurrentContinent() == CONTINENT::INDOORS )
		{
			nlwarning("<ANTIBUG>respawn : user %s is on invalid position %d %d",c->getId().toString().c_str(), c->getState().X(),c->getState().Y() );
			vector<uint16> vect;
			for ( uint i = 0; i < (uint) CONTINENT::NB_CONTINENTS; i++ )
			{
				c->getRespawnPoints().getUsableRespawnPoints(CONTINENT::TContinent(i), vect);
				if( !vect.empty() )
					break;
			}
			if ( vect.empty() )
				nlwarning("<ANTIBUG>respawn : user %s is on invalid position %d %d and has no respawn points at all",c->getId().toString().c_str(), c->getState().X(),c->getState().Y() );
			else
			{
				const CTpSpawnZone * zone = NULL;
				uint i = 0;
				for ( ; i < vect.size(); i++ )
				{
					zone = CZoneManager::getInstance().getTpSpawnZone( vect[0] );
					if ( zone == NULL )
						nlwarning("<ANTIBUG>respawn : user %s has an invalid respawn point of id %u",c->getId().toString().c_str(), vect[i] );
					else
					{
						sint32 x,y,z;
						float heading;
						zone->getRandomPoint(x,y,z,heading);
						c->allowNearPetTp();
						c->tpWanted(x,y,z,true,heading);
						break;
					}
				}
				if ( i == vect.size() )
					nlwarning("<ANTIBUG>respawn : user %s all respawn points are invalid",c->getId().toString().c_str() );
			}
		}
	}

	// check sell store
	c->checkSellStore();

	// send the whole encyclopedia to the client
	c->getEncyclopedia().sendEncycloToClient();

	// update newbieland flag (defailt to 1 if there's aproblem determining the true value)
	nlinfo("Updating IS_NEWBIE flag for character: %s",c->getId().toString().c_str());
//	c->_PropertyDatabase.setProp("USER:IS_NEWBIE", c->isNewbie());
	CBankAccessor_PLR::getUSER().setIS_NEWBIE(c->_PropertyDatabase, c->isNewbie());
	bool trialPlayer = player->isTrialPlayer();
//	c->_PropertyDatabase.setProp("USER:IS_TRIAL", trialPlayer);
	CBankAccessor_PLR::getUSER().setIS_TRIAL(c->_PropertyDatabase, trialPlayer);


	if (IsRingShard)
	{
		breakable
		{
			// warn the DSS that the character is ready
			BOMB_IF(ICharacterControl::getInstance() == NULL, "The character control module is not instantiated", break);

			ICharacterControl::getInstance()->characterReady(c->getId());
		}
	}

	CPVPManager2::getInstance()->updateFactionChannel( c );
	CPVPManager2::getInstance()->setPVPModeInMirror( c );
	c->updatePVPClanVP();

	// Add character to event channel if event is active
	CGameEventManager::getInstance().addCharacterToChannelEvent( c );

	// for GM player, trigger a 'infos' command to remember their persistent state
	if (!PlayerManager.getPlayer(uint32(c->getId().getShortId())>>4)->getUserPriv().empty())
	{
		string res = toString("infos %s", c->getId().toString().c_str()).c_str();
		CLightMemDisplayer *CmdDisplayer = new CLightMemDisplayer("CmdDisplayer");
		CLog *CmdLogger = new CLog( CLog::LOG_NO );
		CmdLogger->addDisplayer( CmdDisplayer );
		NLMISC::ICommand::execute(res, *CmdLogger, true);
		const std::deque<std::string>	&strs = CmdDisplayer->lockStrings ();
		for (uint i = 0; i < strs.size(); i++)
		{
			InfoLog->displayNL("%s", trim(strs[i]).c_str());

			SM_STATIC_PARAMS_1(params,STRING_MANAGER::literal);
			params[0].Literal = trim(strs[i]);
			CCharacter::sendDynamicSystemMessage( c->getId(), "LITERAL", params );
		}
		CmdDisplayer->unlockStrings();
		CmdLogger->removeDisplayer (CmdDisplayer);
		delete CmdDisplayer;
		delete CmdLogger;
	}
	c->setFinalized(true);

} // finalizeClientReady //


//-----------------------------------------------
// cbClientDisconnection :
//
//-----------------------------------------------
void cbClientDisconnection(CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientDisconnection);

	// read the player id
	uint32 userId;
	msgin.serial( userId );
	bool crashed;
	msgin.serial( crashed );

	//Bsi.append( StatPath, NLMISC::toString("[UD] %u %d", userId, (crashed?1:0)) );
	//EgsStat.displayNL("[UD] %u %d", userId, (crashed?1:0));
//	EGSPD::userDisconnected(userId, crashed);
	log_Player_Disconnect(userId, crashed);

	// if character still present it means that crashed = true
	CCharacter *c = PlayerManager.getActiveChar( userId );
//	if( c )
//		{
//		if( c->getEnterFlag() )
//		{
//			// Callback to disconnect a character
//			c->onDisconnection(crashed);
//		}
//	}
	PlayerManager.userDisconnected( userId );
	//disconnectUser(userId);
	/////////////////////////////////////////////////////////////
	// DO NOT ADD ANYTHING HERE: see disconnectUser()
	/////////////////////////////////////////////////////////////
}


//-----------------------------------------------
// disconnect and remove a client:
//
//-----------------------------------------------
void disconnectUser(uint32 userId)
{
	H_AUTO(disconnectUser);

	PlayerManager.disconnectPlayer( userId );
}

//---------------------------------------------------
// cbSelectChar :
// the client choose one of his chars
//---------------------------------------------------
void cbSelectChar( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbSelectChar);

	// read the player id
//	uint64 longUserId;
	uint32 userId;
	msgin.serial( userId );
//	userId = (uint32) longUserId;

	// read the char index
	uint8 c;
	msgin.serial( c );

	// read the instance id
	uint32 instanceId;
	msgin.serial(instanceId);

	try
	{
		CPlayer *p = PlayerManager.getPlayer(userId);
		if( p != 0 )
		{
			sint32 index = c;

			if( index != -1 )
			{
				// get the char infos
				CCharacter * ch = PlayerManager.getChar( userId, index );

				if( ch != 0 )
				{
					// Check if not loading an old file with no normal positions
					if ( ch->PositionStack.empty() )
					{
						// Here we could fix it by taking the backup mainland sessionId from the database,
						// and teleporting the character to the default position.
						// For the moment, disconnect.
						nlwarning( "NO POSITION found in stack for %s", ch->getId().toString().c_str() );
						PlayerManager.disconnectPlayer( userId );
						return;
					}

					if ( ! IsRingShard )
					{
						// Play using the stored normal position
						ch->applyTopOfPositionStack(); // assumes the position stack is not empty. See "regularize" above.

						ch->setSessionUserRole( R2::TUserRole::ur_player );

						/// Cjeck if the character is in an active anim session
						TSessionId destSession = ch->getActiveAnimSessionId().asInt() != 0 ? ch->getActiveAnimSessionId() : ch->sessionId();

						// Handle the case when the stored session id does not match FixedSessionId.
						if ( AllowCharsFromAllSessions.get() == 2 )
							ch->setSessionId( TSessionId(FixedSessionId.get()) );
						if ( (destSession.asInt() != FixedSessionId.get()) && (AllowCharsFromAllSessions.get() == 0) )
						{
							// If the chosen character is on a different shard, far TP now!
							// We don't need any additional check because if the client
							// teleports to a shard with a hacked session id, the next shard
							// will come here again and teleport it again (user char will
							// not be sent until the session id is valid).
							// Note: at this time, we have not begun the 'select char' process
							// thus the client can send SELECT_CHAR again to choose another character
							// Consequently the Far TP request must be sent via userId.
							ch->requestFarTP( destSession, (ch->PositionStack.size() > 1), true );
							return;
						}
					}

					// store the ai instance
					ch->setStartupInstance(instanceId);

					// Send userId/charId association to the FE (IMPULSION_ID becomes valid from now on)
					CEntityId charId = ch->getId();
					PlayerManager.setActiveCharForPlayer( userId, index, charId );
					CMessage msgout2("CL_ID");
					msgout2.serial( userId );
					msgout2.serial( charId );
					NLNET::TServiceId frontEndId = PlayerManager.getPlayerFrontEndId( userId );
					CUnifiedNetwork::getInstance()->send(frontEndId, msgout2);
					egs_ecinfo("Mapping UID %u => Sid %s", userId, charId.toString().c_str() );

					// Callback for the game event manager
					CGameEventManager::getInstance().characterLoadedCallback(PlayerManager.getActiveChar(userId));

					// Callback for the mission queue manager
					CMissionQueueManager::getInstance()->characterLoadedCallback(PlayerManager.getActiveChar(userId));

					if ( IsRingShard )
					{
						// Ask the DSS if he wants give us a position or if he wants us to load a position (+ season)
						// Give the last saved session id of the character so that the DSS will know if resuming of teleporting.
						ICharacterControl* characterControlModule = ICharacterControl::getInstance();
						nlassert( characterControlModule );
						TSessionId lastStoredSessionId = ch->PositionStack.top().SessionId; // assumes the position stack is not empty. See "regularize" above.
						characterControlModule->requestStartParams( charId, lastStoredSessionId );

						// Follow-up
						// in CCharacterControl::setUserCharStartParamsSetNewPos()
						// or CCharacterControl::setUserCharStartParamsReloadPos()
					}
					else
					{
						// Continue on same shard => send user char data (start pos, etc.)
						ch->sendUserChar( userId, 0 /*auto*/, R2::TUserRole::ur_player );
					}

					// send CEntityId/name association to the IOS
					//			ch->registerName();

					// send acknowledge to client for received Enter message (new: it includes version handshake)
					CBitMemStream bms2;
					nlverify(GenericMsgManager.pushNameToStream( "CONNECTION:READY", bms2));
					ch->fillHandshake( bms2 );
					CMessage msgout3( "IMPULSION_ID" );
					msgout3.serial( charId );
					msgout3.serialBufferWithSize((uint8*)bms2.buffer(), bms2.length());
					CUnifiedNetwork::getInstance()->send( frontEndId, msgout3 );

//					// Remove user language from IOS, we don't need it anymore
//					CMessage msgout4("REMOVE_USER_LANGUAGE");
//					msgout4.serial(userId);
//					CUnifiedNetwork::getInstance()->send("IOS", msgout4);

					if( CPVPManager2::getInstance()->inSafeZone( ch->getPosition() ) )
					{
						// character must be safe in PVP until he leave safe zone
						ch->setPvPSafeZoneActive();
					}

					// whether client has to use female titles for female characters
					bool bFemaleTitles = UseFemaleTitles;
					PlayerManager.sendImpulseToClient(charId,"GUILD:USE_FEMALE_TITLES",bFemaleTitles);

					// update mision giver icon timer if non-default
					ch->sendNpcMissionGiverTimer(false);

					// log this event
					log_Character_Select(uint32(ch->getId().getShortId()>>4), ch->getId(), ch->getName().toUtf8());
					return;
				}
			}
		}
	}
	catch(const Exception &e)
	{
		nlwarning( "(PS)<cbGetChar> Error: %s", e.what() );
	}

	PlayerManager.disconnectPlayer( userId );
} // cbSelectChar //


//---------------------------------------------------
// cbCheckName :
// the client sent a check name request
//---------------------------------------------------
void cbCheckName( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbCheckName);

	uint64 longUid;
	uint32 userId;
	CCheckNameMsg checkNameMsg;

	msgin.serial( longUid );
	userId = (uint32) longUid;
	msgin.serial( checkNameMsg );

	// acceptable name if: first char is an upper case, all the name is lower case
	for (uint i = 0; i < checkNameMsg.Name.size(); i++)
	{
		if(i==0 && checkNameMsg.Name[i]>='a'&&checkNameMsg.Name[i]<='z')
		{
			// must be upper
			checkNameMsg.Name[i] = checkNameMsg.Name[i] - 'a' + 'A';
		}
		if(i!=0 && checkNameMsg.Name[i]>='A'&&checkNameMsg.Name[i]<='Z')
		{
			// must be lower
			checkNameMsg.Name[i] = checkNameMsg.Name[i] - 'A' + 'a';
		}
	}

	if (IShardUnifierEvent::getInstance())
		IShardUnifierEvent::getInstance()->validateCharacterNameBeforeCreate(userId, 0xff, checkNameMsg.Name, checkNameMsg.HomeSessionId.asInt());
	else
		sendIfNameIsValide( userId, false ); // don't accept character creation while SU is down
}


//---------------------------------------------------
// cbCreateChar :
// the client sent a player creation request
//---------------------------------------------------
void cbCreateChar( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbCreateChar);

	// read the player id
	uint64 longUserId;
	uint32 userId;
	msgin.serial( longUserId );
	userId = (uint32) longUserId;

	CCreateCharMsg createCharMsg;
	createCharMsg.serial( msgin );

	// Yoyo: fix to force new newbieland.
	if(UseNewNewbieLandStartingPoint)
	{
		createCharMsg.StartPoint= RYZOM_STARTING_POINT::starting_city;
	}

	// acceptable name if: first char is an upper case, all the name is lower case
	for (uint i = 0; i < createCharMsg.Name.size(); i++)
	{
		if(i==0 && createCharMsg.Name[i]>='a'&&createCharMsg.Name[i]<='z')
		{
			// must be upper
			createCharMsg.Name[i] = createCharMsg.Name[i] - 'a' + 'A';
		}
		if(i!=0 && createCharMsg.Name[i]>='A'&&createCharMsg.Name[i]<='Z')
		{
			// must be lower
			createCharMsg.Name[i] = createCharMsg.Name[i] - 'A' + 'a';
		}
	}

	CCreateCharErrorMsg createCharErrorMsg;
	if( ! CCharacter::checkCreateParams( createCharMsg, createCharErrorMsg, userId ) )
	{
		// Some createCharMsg are false, send error msg to client
/*		Send CONNECTION:CREATE_CHAR_ERROR later when client is ready for that
		CBitMemStream bms;
		if ( ! GenericMsgManager.pushNameToStream( "CONNECTION:CREATE_CHAR_ERROR", bms) )
		{
			nlwarning("<cbCreateChar> Msg name CONNECTION:CREATE_CHAR_ERROR not found");
			return;
		}
		bms.serial( createCharErrorMsg );

		CMessage msgout( "IMPULSION_UID" );
		msgout.serial( userId );
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());

		try
		{
			CUnifiedNetwork::getInstance()->send(serviceId, msgout);
		}
		catch(const Exception &e)
		{
			nlwarning( "(PS)<cbCreateChar> Error: %s", e.what() );
		}
*/
//		sendCharactersSummary( PlayerManager.getPlayer( userId ) );
//		return;
		goto CreationFailed;
	}

	// now, validate the name on the NameUnifier and continue when we have the response
	if (IShardUnifierEvent::getInstance() == NULL)
	{
		// we can't create new character when Name unifier is offline
		nlinfo("VALID_NAME::EC::cbCreateChar name %s rejected because we have no SU instance", createCharMsg.Name.toString().c_str());
		goto CreationFailed;
	}

	if( DontUseSU.get() == 0)
	{
		IShardUnifierEvent::getInstance()->validateCharacterCreation(userId, createCharMsg.Slot, createCharMsg);
	}
	else
	{
		// if we not use SU, we validate any character name
		cbCreateChar_part2(userId, createCharMsg, true);
	}

	return;

CreationFailed:
	// The code come here in case of error in the received info
	sendCharactersSummary( PlayerManager.getPlayer( userId ) );
	return;
}

void cbCreateChar_part2(uint32 userId, const CCreateCharMsg &createCharMsg, bool ok)
{
	TLogNoContext_Character noContextCharacter;
	TLogNoContext_Item noContextItem;

	if (!ok)
	{
		nlwarning("cbCreateChar_part2 : user %u : SU has failed to validate the character creation with name '%s'! ",
			userId, createCharMsg.Name.toUtf8().c_str());
		// The code come here in case of error in the received info
		sendCharactersSummary( PlayerManager.getPlayer( userId ) );
		return;
	}

	// The name unifier has validated the name, we can proceed to creation

	// create a new char
	CCharacter * ch = new CCharacter();

	createCharMsg.Slot;

	CEntityId charId = PlayerManager.createCharacterId( userId, createCharMsg.Slot );

	ch->setId( charId );
	ch->setName( createCharMsg.Name );
//	ch->setSurname( string("unknown") );

	ch->initPDStructs();

	// Ask for string Id
//	PlayerManager.addEntityForStringIdRequest(charId);

//	CEntityIdTranslator::getInstance()->registerEntity (charId, createCharMsg.Name, (sint8)createCharMsg.Slot, userId, PlayerManager.getPlayer( userId )->getUserName());
//	CNameManager::getInstance()->assignName( charId, createCharMsg.Name );

	// Set start statistics and other params on character
	ch->setStartStatistics( createCharMsg );

	// add char to player chars
	PlayerManager.addChar( userId, charId, ch, createCharMsg.Slot );

	CPlayer * player = PlayerManager.getPlayer( userId );
	if ( !player )
	{
		nlwarning("create char : invalid user id %u",userId );
		return;
	}
	CHARACTER_TITLE::ECharacterTitle gmTitle = CHARACTER_TITLE::getGMTitleFromPriv( player->getUserPriv() );
	if ( gmTitle != CHARACTER_TITLE::NB_CHARACTER_TITLE )
		ch->setTitle( gmTitle );
	sendCharactersSummary( PlayerManager.getPlayer( userId ) );

	try
	{
		// send back infos
		ICharacter * cItf = ICharacter::getInterface( ch, false );
		cItf->sendUserChar( userId, 0 /*auto*/, R2::TUserRole::ur_player );
	}
	catch(const Exception &e)
	{
		nlwarning( "(PS)<cbCreateChar> Error: %s", e.what() );
	}

	PlayerManager.savePlayerChar( userId, createCharMsg.Slot );

	// update the ring database with the new char
	if (IShardUnifierEvent::getInstance())
	{
		CHARSYNC::TCharInfo charInfo;
		ch->fillCharInfo(charInfo);
		// overload some values
		charInfo.setHomeSessionId(createCharMsg.Mainland);
//		charInfo.setCharEId(ch->getId());
//		charInfo.setCharName(ch->getName().toUtf8());
//		charInfo.setBestCombatLevel(0);			// set to 0, because it's a new char
//		charInfo.setGuildId(ch->getGuildId());
//		charInfo.setRace(CHARSYNC::TRace(ch->getRace());
//		std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance = ch->getAllegiance();
//		charInfo.setCivilisation(ch->
//		charInfo.setCult(
		// no respawn points
		IShardUnifierEvent::getInstance()->onNewChar(charInfo);
	}
} // cbCreateChar //


//---------------------------------------------------
// sendIfNameIsValide :
// Send to client if name is valide (true for valide)
//---------------------------------------------------
void sendIfNameIsValide( uint32 userId, bool nameValide )
{
	H_AUTO(sendIfNameIsValide);

	CBitMemStream bmsName;
	if ( ! GenericMsgManager.pushNameToStream( "CONNECTION:VALID_NAME", bmsName) )
	{
		nlwarning("<cbCreateChar> Msg name CONNECTION:VALID_NAME not found");
		return;
	}
	uint8 valide = nameValide;
	bmsName.serial( valide );

	CMessage msgoutName( "IMPULSION_UID" );
	msgoutName.serial( userId );
	msgoutName.serialBufferWithSize((uint8*)bmsName.buffer(), bmsName.length());

	try
	{
		CUnifiedNetwork::getInstance()->send( PlayerManager.getPlayerFrontEndId( userId ), msgoutName );
	}
	catch(const Exception &e)
	{
		nlwarning( "(PS)<cbCreateChar> Error: %s", e.what() );
	}
}


//-----------------------------------------------
// cbDeleteChar :
//
//-----------------------------------------------
void cbDeleteChar( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbDeleteChar);

	// read the player id
	uint64 longUserId;
	uint32 userId;
	msgin.serial( longUserId );
	userId = (uint32) longUserId;

	uint8 characterIndex;
	msgin.serial( characterIndex );

	CPlayer* player = PlayerManager.getPlayer( userId );
	if ( player == NULL )
	{
		nlwarning("invalid user %"NL_I64"u %u %hu", longUserId,userId,(uint16)characterIndex);
		return;
	}

	string charName;
	sint32 index = characterIndex;
	CCharacter *character = player->getCharacter(characterIndex);
	if (character != NULL)
		charName = character->getName().toUtf8();

	PlayerManager.deleteCharacter( userId, index );

	// update the ring database
	if (IShardUnifierEvent::getInstance())
	{
		IShardUnifierEvent::getInstance()->onDeleteChar((userId<<4)+index);
	}

	sendCharactersSummary(player);

	string fileName = NLMISC::toString( "account_%u_%d.bin", userId, index );
	BsiGlobal.deleteFile( fileName );

	string charPath = PlayerManager.getCharacterPath(userId, true);

	fileName = charPath+NLMISC::toString( "account_%u_%d.xml", userId, index );
	BsiGlobal.deleteFile( fileName );

	fileName = charPath+NLMISC::toString( "account_%u_%d.xml.backup", userId, index );
	BsiGlobal.deleteFile( fileName );

	fileName = charPath+NLMISC::toString( "account_%u_%d.bin", userId, index );
	BsiGlobal.deleteFile( fileName );

	fileName = charPath+NLMISC::toString( "account_%u_%d.bin.backup", userId, index );
	BsiGlobal.deleteFile( fileName );

	fileName = charPath+NLMISC::toString( "account_%u_%d_pdr.bin", userId, index );
	BsiGlobal.deleteFile( fileName );

	fileName = charPath+NLMISC::toString( "account_%u_%d_pdr.xml", userId, index );
	BsiGlobal.deleteFile( fileName );

	fileName = charPath+NLMISC::toString( "account_%u_%d_pdr.bin.backup", userId, index );
	BsiGlobal.deleteFile( fileName );

	fileName = charPath+NLMISC::toString( "account_%u_%d_pdr.xml.backup", userId, index );
	BsiGlobal.deleteFile( fileName );

	// log this event
	log_Character_Delete(userId, CEntityId(RYZOMID::player, (userId<<4)+index), charName);

} // cbDeleteChar //


//-----------------------------------------------
// Send characters summary to client :
//
//-----------------------------------------------
void sendCharactersSummary( CPlayer *player, bool AllAutorized, uint32 bitfieldOwnerOfActiveAnimSession, uint32 bitfieldOwnerOfEditSession )
{
	H_AUTO(sendCharactersSummary);

	if ( player == NULL )
		return;

	// Fill the vector of CCharacterSummary
	vector<CCharacterSummary> chars;
	player->getCharactersSummary( chars );
	if (bitfieldOwnerOfActiveAnimSession != 0)
	{
		for ( uint i=0, len=(uint)chars.size(); i!=len; ++i )
		{
			chars[i].InRingSession = ((bitfieldOwnerOfActiveAnimSession & (1 << i)) != 0);
		}
	}

	if (bitfieldOwnerOfEditSession != 0)
	{
		for ( uint i=0, len=(uint)chars.size(); i!=len; ++i )
		{
			chars[i].HasEditSession = ((bitfieldOwnerOfEditSession & (1 << i)) != 0);
		}
	}

	// Build the message
	CBitMemStream bms;
	if( chars.size() > 0 )
	{
		if ( ! GenericMsgManager.pushNameToStream( "CONNECTION:USER_CHARS", bms) )
		{
			nlwarning("<cbClientConnection> Msg name CONNECTION:USER_CHARS not found");
			return;
		}
		if( AllAutorized )
		{
			uint8 allAut = 255;
			bms.serial( allAut );
			bms.serial( allAut );
		}
		else
		{
			bms.serial( PeopleAutorisation );
			bms.serial( CareerAutorisation );
		}
		bms.serialCont( chars );
	}
	else
	{
		if ( ! GenericMsgManager.pushNameToStream( "CONNECTION:NO_USER_CHAR", bms) )
		{
			nlwarning("<cbClientConnection> Msg name CONNECTION:NO_USER_CHAR not found");
			return;
		}
		if( AllAutorized )
		{
			uint8 allAut = 255;
			bms.serial( allAut );
			bms.serial( allAut );
		}
		else
		{
			bms.serial( PeopleAutorisation );
			bms.serial( CareerAutorisation );
		}
	}

	// send shard name summaries, in all case
	std::vector<string>		shardNames;
	CShardNames::getInstance().saveShardNames(shardNames);
	bms.serialCont (shardNames);

	// send privileges in all cases
	std::string priv;
	priv = player->getUserPriv();
	bms.serial(priv);

	// send the 'free trial' flag
	bool freeTrial = player->isTrialPlayer();
	bms.serial(freeTrial);

	// send available mainlands
	bms.serialCont(Mainlands);

	CMessage msgout( "IMPULSION_UID" );
	uint32 userId = player->getUserId();
	msgout.serial( userId );
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	NLNET::TServiceId frontEndId = PlayerManager.getPlayerFrontEndId( player->getUserId() );
	try
	{
		CUnifiedNetwork::getInstance()->send( frontEndId, msgout);
	}
	catch(const Exception &e)
	{
		nlwarning( "<sendCharactersSummary> Error: %s", e.what() );
	}
}


//-----------------------------------------------
// cbEntityPos :
//
//-----------------------------------------------
void cbEntityPos(CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbEntityPos);

	CEntityId id;
	sint32 x, y, z;
	float t;

	msgin.serial( id );
	msgin.serial( x );
	msgin.serial( y );
	msgin.serial( z );
	msgin.serial( t );

	// Set state for active character of player
	CCharacter * c = PlayerManager.getChar( id );
	if( c )
	{
		CEntityState& state = c->getState();
		state.X = x;
		state.Y = y;
		state.Z = z;
		state.Heading = t;
		c->setAfkState(false);
	}

	// check player connexion status and save and remove it if is disconnected
	CPlayer *player = PlayerManager.getPlayer( PlayerManager.getPlayerId( id ) );
	if( player )
	{
		if( player->getPlayerConnection() == false )
		{
			// Save the player's characters
			PlayerManager.savePlayerActiveChar( PlayerManager.getPlayerId( id ) );

			// remove the player from all manager
			/*CEntityId id = */PlayerManager.disconnectPlayer( PlayerManager.getPlayerId( id ) );
		}
	}
} // cbEntityPos //



//-----------------------------------------------
// cbTpAcknowledge : callback for CLIENT:TP:ACK message
//-----------------------------------------------
void cbClientTpAck( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbTpAcknowledge);

	CEntityId Id;

	msgin.serial( Id );
	CCharacter *ch = PlayerManager.getChar( Id );
	if ( ch && ch->getEnterFlag() )
	{
		// if this is a ring shard then we need to make sure that the player gets put back into their AIInstance
		// before executing any code that can provoke a return ... otherwise the player can be stuck in '-1' indefinitely
		if (IsRingShard)
		{
			// we don't look at continent if we are in a ring shard
			ch->setRingShardInstanceNumber(ch->getStartupInstance());
		}

		ch->setAfkState(false);
		SGameCoordinate state = ch->getTpCoordinate();
		NLMISC::TGameCycle tick = CTickEventHandler::getGameCycle();

		if ( state.X <= 0 || state.Y >= 0 )
		{
			nlwarning("'%s' Invalid TP coords %d,%d,%d",ch->getId().toString().c_str(),state.X,state.Y,state.Z);
			return;
		}

		if (IsRingShard)
		{
			nlinfo("cbClientTpAck: Asking GPMS to TP player %s to final destination (%d,%d) at tick %u",
				   Id.toString().c_str(), (uint32)state.X, (uint32)state.Y, (uint32)tick );
		}
		CMessage msgout("ENTITY_TELEPORTATION");
		msgout.serial( Id );
		msgout.serial( state.X );
		msgout.serial( state.Y );
		msgout.serial( state.Z );
		msgout.serial( state.Heading );
		msgout.serial( tick );

		///if a cell is specified serial it
		if ( state.Cell != 0)
		{
			msgout.serial( state.Continent );
			sint32 cell = state.Cell;
			msgout.serial(cell);
		}

		sendMessageViaMirror("GPMS", msgout);
		ch->resetTpCoordinate();

//		ch->_PropertyDatabase.setProp( "TARGET:BARS:UID", CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
		CBankAccessor_PLR::getTARGET().getBARS().setUID(ch->_PropertyDatabase, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
//		ch->_PropertyDatabase.setProp( "TARGET:BARS:HP", 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setHP(ch->_PropertyDatabase, 0 );
//		ch->_PropertyDatabase.setProp( "TARGET:BARS:SAP", 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setSAP(ch->_PropertyDatabase, 0 );
//		ch->_PropertyDatabase.setProp( "TARGET:BARS:STA", 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setSTA(ch->_PropertyDatabase, 0 );
//		ch->_PropertyDatabase.setProp( "TARGET:BARS:FOCUS", 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setFOCUS(ch->_PropertyDatabase, 0 );
		ch->setTarget( CEntityId::Unknown );

		// player still intangible for a few seconds
		ch->setIntangibleEndDate( CTickEventHandler::getGameCycle() + IntangibleTimeAfterTP );

		if (ch->getMode()==MBEHAV::DEATH)
		{
			ch->setTimeOfDeath( CTickEventHandler::getGameTime() + 5.0 );
			return;
		}
		else
		{
			ch->setMode( MBEHAV::NORMAL );
			ch->setBehaviour( MBEHAV::IDLE );
		}

		// get the continent where the player will spawn
		if (!IsRingShard)
		{
			// lookup continent to see where we are
			CContinent * cont = CZoneManager::getInstance().getContinent(state.X,state.Y);
			uint32 in = INVALID_AI_INSTANCE;
			if ( cont )
			{
				const CONTINENT::TContinent contId = CONTINENT::TContinent(cont->getId());
				in = CUsedContinent::instance().getInstanceForContinent( contId );
				if ( in == INVALID_AI_INSTANCE )
				{
					nlwarning("<TP>%s will arrive in invalid continent %s", ch->getId().toString().c_str(), NLMISC::strlwr( cont->getName() ).c_str() );
				}

				// respawn the pets if needed
				ch->respawnPetAfterTp( state, in );

				nldebug("Setting player %s in instance #%u for continent '%s'",
					ch->getId().toString().c_str(),
					in,
					CONTINENT::toString(contId).c_str());
			}
			else
			{
				nlwarning("<TP>%s no valid continent at %d,%d", ch->getId().toString().c_str(),state.X,state.Y );
			}
			ch->setInstanceNumber( in );
		}

		CVector position;
		position.x = state.X * 0.001f;
		position.y = state.Y * 0.001f;
		position.z = state.Z * 0.001f;
		if( CPVPManager2::getInstance()->inSafeZone( position ) )
		{
			// character must be safe in PVP until he leave safe zone
			ch->setPvPSafeZoneActive();
		}

		// reset the who sees me property to previous value
		ch->setWhoSeesMe( ch->whoSeesMeBeforeTP() );

		if (IsRingShard)
		{
			// ask to character control interface to re-send Ring adventure position for check if it have been changed during the re-spawn
			ICharacterControl* characterControlModule = ICharacterControl::getInstance();
			nlassert( characterControlModule );
			characterControlModule->requestEntryPoint( ch->getId() );
		}

//////////////////////////////////////////////////////////////////////////
// antibug for invisibility problem
		if ( !R2_VISION::isEntityVisibleToPlayers(ch->getWhoSeesMe()) )
		{
			if (IsRingShard && ch->godMode() ) // DM or TESTER
			{
				// If a RING/DM is invisible he stays invisible.
			}
			else
			{
				CPlayer *player = PlayerManager.getPlayer(PlayerManager.getPlayerId(ch->getId()));
				if (player && player->getUserPriv().empty() )
				{
					nlwarning("INVISIBILITY (cbTpAcknowledge) Player %s (with no privilege) still invisible after a TP Ackowledge, set flag to visible", ch->getId().toString().c_str());
					ch->setWhoSeesMe( IsRingShard? R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_PLAYER,true): UINT64_CONSTANT(0xffffffffffffffff) );
				}
			}
		}


//////////////////////////////////////////////////////////////////////////
		ch->resetWhoSeesMeBeforeTP();

		// send message to AIS to enable aggro on this player
		ch->setAggroable(R2_VISION::isEntityVisibleToMobs(ch->getWhoSeesMe()), true);
	}
	else
	{
		nlwarning("<cbTpAcknowledge> Unknown character %s", Id.toString().c_str() );
	}
} // cbTpAcknowledge //

//---------------------------------------------------
// cbLeaveTeam: a player
//---------------------------------------------------
void cbLeaveTeam( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbLeaveTeam);

	CEntityId charId;
	msgin.serial( charId );
	TeamManager.quitTeam( charId );
} // cbLeaveTeam //

//---------------------------------------------------
// cbJoinTeam: join specified team
//---------------------------------------------------
void cbJoinTeam( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbJoinTeam);

	CEntityId charId;
	msgin.serial( charId );
	TeamManager.joinAccept( charId );
} // cbJoinTeam //

//---------------------------------------------------
// cbJoinTeamDecline: player decline a proposition
//---------------------------------------------------
void cbJoinTeamDecline( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbJoinTeamDecline);

	CEntityId charId;
	msgin.serial( charId );
	TeamManager.joinDecline( charId );
} // cbJoinTeamDecline //


//---------------------------------------------------
// cbJoinTeamProposal: propose a player (current target) to enter a team
//---------------------------------------------------
void cbJoinTeamProposal( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbJoinTeamProposal);

	CEntityId charId;
	msgin.serial( charId );

	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL || !character->getEnterFlag())
	{
		nlwarning("<cbJoinTeamProposal> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);
	TeamManager.joinProposal( character, character->getTarget() );
} // cbJoinTeamProposal //


//---------------------------------------------------
// cbJoinLeague: join specified League
//---------------------------------------------------
void cbJoinLeague( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbJoinLeague);
	
	CEntityId charId;
	msgin.serial( charId );
	TeamManager.joinLeagueAccept( charId );
} // cbJoinLeague //

//---------------------------------------------------
// cbJoinLeagueDecline: player decline the proposition
//---------------------------------------------------
void cbJoinLeagueDecline( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbJoinLeagueDecline);
	
	CEntityId charId;
	msgin.serial( charId );
	TeamManager.joinLeagueDecline( charId );
} // cbJoinLeagueDecline //


//---------------------------------------------------
// cbJoinLeagueProposal: propose a player (current target) to enter the League
//---------------------------------------------------
void cbJoinLeagueProposal( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbJoinLeagueProposal);
	
	CEntityId charId;
	msgin.serial( charId );

	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL || !character->getEnterFlag())
	{
		nlwarning("<cbJoinLeagueProposal> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);
	TeamManager.joinLeagueProposal( character, character->getTarget() );
} // cbJoinLeagueProposal //



//---------------------------------------------------
// cbKickTeammate: kick your target from your team
//---------------------------------------------------
void cbKickTeammate( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbKickTeammate);

	CEntityId charId;
	uint8 memberIndex;
	msgin.serial( charId );
	msgin.serial( memberIndex );


	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL || !character->getEnterFlag())
	{
		nlwarning("<cbJoinTeamProposal> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);
	TeamManager.kickCharacter( character, memberIndex);
} // cbKickTeammate //


//---------------------------------------------------
// cbHarvest:
//---------------------------------------------------
void cbHarvest( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbHarvest);

	CEntityId charId;
	msgin.serial( charId );

	uint16	mpIndex;
	msgin.serial( mpIndex );

	uint16	quantity;
	msgin.serial( quantity );

	// get harvester character
	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL || !character->getActionFlag())
	{
		nlwarning("<cbHarvest> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);
	character->harvestAsked( mpIndex, quantity );
} // cbHarvest //


//---------------------------------------------------
// cbHarvestClose:
//---------------------------------------------------
void cbHarvestClose( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbHarvestClose);

	CEntityId charId;
	msgin.serial( charId );

	// get harvester character
	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbHarvestClose> Invalid player Id %s", charId.toString().c_str() );
		return;
	}

	DEBUGLOG("<cbHarvestClose> Received Harvest Close for player %s, harvesting entity %s",charId.toString().c_str(), character->harvestedEntity().toString().c_str() );

	// end harvest
	character->endHarvest();
	character->setAfkState(false);
} // cbHarvestClose //


//---------------------------------------------------
// cbHarvestDeposit:
//---------------------------------------------------
void cbHarvestDeposit( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbHarvestDeposit);

	CEntityId charId;
	msgin.serial( charId );

	// the used skill
	SKILLS::ESkills skill;
	uint16 skillValue;
	msgin.serial( skillValue );
	skill = (SKILLS::ESkills) skillValue;

	// get harvester character
	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbHarvestDeposit> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	// check char is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbHarvestDeposit> player Id %s not yet ready", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);

	// check this harvester can use the given skill
/*	if ( skill >= SKILLS::NUM_SKILLS || character->getSkills()._Skills[skill].Base <= 0 )
	{
		nlwarning("<cbHarvestDeposit> player Id %s cannot use the skill (%s - %d) as his score is 0", charId.toString().c_str(), SKILLS::toString(skill).c_str(), skill );
		/// \todo Malkav:send a message to the client
		return;
	}
*/
	// end harvest but doesn't close the interface
	character->endHarvest(false);

	// begin harvest
	character->staticActionInProgress( true );
	nlerror( "character->harvestDeposit(true);" ); // TODO
	/*character->depositSearchSkill(skill);
	character->openHarvest();*/

//	character->sendMessageToClient( character->getId(), "WOS_HARVEST_SEARCHING" );
	nlerror( "CZoneManager::getInstance().harvestDeposit(character);" );

	// Changed : Wait new harvest rules ////////////////////////////////////////////////////
// execute phrase
	if (character->getHarvestInfos().Sheet != CSheetId::Unknown)
		character->harvestAsked(0);
	else
	{
		character->sendDynamicSystemMessage( character->getId(), "WOS_HARVEST_FOUND_NOTHING");
		character->sendCloseTempInventoryImpulsion();
	}
// Changed : Wait new harvest rules ////////////////////////////////////////////////////

//	character->setBehaviour( MBEHAV::HARVESTING );

} // cbHarvestDeposit //



//---------------------------------------------------
// cbHarvestMPDestroyed:
//---------------------------------------------------
void cbHarvestMPDestroyed( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbHarvestMPDestroyed);

	CEntityId charId;
	msgin.serial( charId );

	// get harvester character
	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbHarvestMPDestroyed> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	// check char is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbHarvestMPDestroyed> player Id %s not yet ready", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);

	// get harvested corpse
	const CEntityId &harvestedEntity = character->harvestedEntity();

	CCreature *creature = CreatureManager.getCreature( harvestedEntity );
	if (creature == NULL)
	{
		nlwarning("<cbHarvestMPDestroyed> Invalid creature Id %s", harvestedEntity.toString().c_str() );
		// reset harvest info
		character->resetHarvestInfos();
		character->endHarvest();
		return;
	}

	// remove the quantity of mp harvested from the ressource
	creature->removeMp( character->harvestedMpIndex(), character->harvestedMpQuantity() );

	// reset harvest info
	character->resetHarvestInfos();
} // cbHarvestMPDestroyed //

//---------------------------------------------------
// cbHarvestInterrupted:
//---------------------------------------------------
void cbHarvestInterrupted( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbHarvestInterrupted);

	CEntityId charId;
	msgin.serial( charId );

	// get harvester character
	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbHarvestFailed> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	// check char is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbHarvestInterrupted> player Id %s not yet ready", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);

	// reset harvest info
	character->resetHarvestInfos();

/*	string msgName = "OPS_HARVEST_INTERRUPTED";

	CMessage msg("STATIC_STRING");
	msg.serial( charId );
	set<CEntityId> excluded;
	msg.serialCont( excluded );
	msg.serial( msgName );
	sendMessageViaMirror ("IOS", msg);
*/
} // cbHarvestInterrupted //

//---------------------------------------------------
// cbHarvestDB:
//---------------------------------------------------
void cbHarvestDB( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbHarvestDB);

	CEntityId charId;
	CSheetId sheet;
	uint16 quantity;
	uint16 minQuality;
	uint16 maxQuality;

	msgin.serial( charId );
	msgin.serial( sheet );
	msgin.serial( quantity );
	msgin.serial( minQuality );
	msgin.serial( maxQuality );

	// get harvester character
	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbHarvestDB> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	// check char is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbHarvestDB> player Id %s not yet ready", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);

	// character is doing a static action
	character->staticActionInProgress(true);

	CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)character->getInventory(INVENTORIES::temporary);

	for (uint32 i = 0 ; i < invTemp->getSlotCount(); ++i )
	{
		if (i != 0 )
		{
			invTemp->setDispQuantity(i, 0);
			invTemp->setDispSheetId(i, CSheetId::Unknown);
		}
		else
		{
			invTemp->setDispQuantity(i, quantity);
			invTemp->setDispSheetId(i, sheet);
		}
		invTemp->setDispQuality(i, 0);
//trap		character->incSlotVersion( INVENTORIES::temporary,i );
	}

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
	params[0].SheetId = sheet;
	CCharacter::sendDynamicSystemMessage( charId, "WOS_HARVEST_FOUND_MP_S", params );
/*	const CStaticItem * staticItem = CSheets::getForm( sheet );
	if( staticItem )
	{
//		character->sendMessageToClient( charId, "WOS_HARVEST_FOUND_MP_S", staticItem->Name );
	}
	else
	{
		character->sendMessageToClient( charId, "WOS_HARVEST_FOUND_MP_S", "??????" );
	}
*/
	character->setBehaviour( MBEHAV::IDLE );

} // cbHarvestDB //

//---------------------------------------------------
// cbHarvestDBUpdateQty:
//---------------------------------------------------
void cbHarvestDBUpdateQty( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbHarvestDBUpdateQty);

	CEntityId charId;
	msgin.serial( charId );

	uint8 index;
	msgin.serial( index );

	uint16 quantity;
	msgin.serial( quantity );

	// get harvester character
	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbHarvestDBUpdateQty> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	// check char is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbHarvestDBUpdateQty> player Id %s not yet ready", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);

	CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)character->getInventory(INVENTORIES::temporary);
	invTemp->setDispQuantity(index, quantity);

} // cbHarvestDBUpdateQty //

//---------------------------------------------------
// cbClearHarvestDB:
//---------------------------------------------------
void cbClearHarvestDB( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClearHarvestDB);

	CEntityId charId;
	msgin.serial( charId );

	// get harvester character
	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbClearHarvestDB> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	// check char is ready
	if (!character->getEnterFlag())
	{
		nlwarning("<cbClearHarvestDB> player Id %s not yet ready", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);

	character->clearHarvestDB();
} // cbClearHarvestDB //

//-----------------------------------------------
// cbCreateItemInBag : create an object or a stack of objects in player bag
//-----------------------------------------------
/*void cbCreateItemInBag( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId charId;
	msgin.serial( charId );

	CSheetId itemSheetId;
	msgin.serial( itemSheetId );

	uint16 quantity;
	msgin.serial( quantity );

	uint16 quality;
	msgin.serial( quality );

	bool setCreator;
	msgin.serial( setCreator );

	// get character
	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbCreateItemInBag> Invalid player Id %s", charId.toString().c_str() );
		return;
	}

	if ( setCreator == true)
		character->createItemInBag( quality, quantity, itemSheetId, charId );
	else
		character->createItemInBag( quality, quantity, itemSheetId );

	character->_PropertyDatabase.setProp( "INVENTORY:COUNTER", character->interfaceCounter() );
} // cbCreateItemInBag //
*/

//-----------------------------------------------
// cbFightingTarget : callback for changing fighting target
//
//-----------------------------------------------
void cbFightingTarget( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbFightingTarget);

	CEntityId IdCharacter, Target;

	msgin.serial( IdCharacter );
	msgin.serial( Target );

	if( IdCharacter.getType() == RYZOMID::player )
	{
		CCharacter * c = PlayerManager.getChar( IdCharacter );

		if ( c && c->getEnterFlag())
		{
			c->setFightingTarget( Target );
			c->setAfkState(false);
//			egs_ecinfo("<cbFightingTarget> for character %s, set target to %s", IdCharacter.toString().c_str(), Target.toString().c_str() );
		}
		else
		{
			nlwarning( "<cbFightingTarget> Unknown character %s", IdCharacter.toString().c_str() );
		}
	}
}

//-----------------------------------------------
// cbPackAnimalCommand : callback for send command to pack animal
//
//-----------------------------------------------
void cbAnimalCommand( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbAnimalCommand);

	CEntityId entity;
	uint8 beastIndex;
	uint8 command;

	msgin.serial( entity );
	msgin.serial( beastIndex );
	msgin.serial( command );

	// make sure the entity id corresponds to a player
	BOMB_IF( entity.getType() != RYZOMID::player, "Ignoring attempt by non-player entity to send commands to pack animals: "<<entity.toString(), return );

	// get hold of a pointer to the player character and make sure it's valid
	CCharacter * c = PlayerManager.getChar( entity );
	BOMB_IF( !c || !c->getEnterFlag(),"<cbPackAnimalCommand> Unknown character "<<entity.toString(), return );

	// go ahead and perform the action
	c->setAfkState(false);
	c->sendAnimalCommand( beastIndex, command );
}

//-----------------------------------------------
// cbGodMode : callback for toggle god mode for player
//
//-----------------------------------------------
/*void cbGodMode( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId id;
	msgin.serial( id );

	CCharacter * c = PlayerManager.getChar( id );
	if( c && c->havePriv(PriviliegeGameMaster))
	{
		c->ToggleGodMode();
	}
}*/

//-----------------------------------------------
// cbTradeListReceived : // callback for send list traded by shopkeeper
//
//-----------------------------------------------
/* Deprecated
void cbTradeListReceived( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId entity,bot;
	RYZOM_TRADE::TTradeList TradeList;

	msgin.serial( entity );
	msgin.serial( bot );

	while( msgin.getPos() < (sint32) msgin.length() )
	{
		RYZOM_TRADE::CTradeBase *trade = new RYZOM_TRADE::CTradeBase();
		msgin.serial( *trade );
		TradeList.push_back( trade );
	}

	CCharacter * c = PlayerManager.getChar( entity );
	if( c )
	{
		c->tradeBegin( TradeList,bot );
	}
	else
	{
		nlwarning("<cbTradeListReceived> Unknown character %s", entity.toString().c_str() );
	}
}
*/

//-----------------------------------------------
// cbTradeBuySomething : callback player buy something in current trade page
//
//-----------------------------------------------
void cbTradeBuySomething( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbTradeBuySomething);

	CEntityId player;
	uint8 ItemNumber;
	uint16 Quantity;

	msgin.serial( player );
	msgin.serial( ItemNumber );
	msgin.serial( Quantity );

	CCharacter * c = PlayerManager.getChar( player );
	if( c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		c->buyItem( ItemNumber, Quantity );
	}
	else
	{
		nlwarning("<cbTradeBuySomething> Unknown character %s", player.toString().c_str() );
	}
}

/*
//-----------------------------------------------
// cbGiveSeed : callback add seed to player
//
//-----------------------------------------------
void cbGiveSeed( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId player;
	CSeeds seed;

	msgin.serial( player );
	msgin.serial( seed );

	CCharacter * c = PlayerManager.getChar( player );
	if( c )
	{
		c->giveSeed( seed );
	}
	else
	{
		nlwarning("<cbGiveSeed> Unknown character %s", player.toString().c_str() );
	}
}
*/
//-----------------------------------------------
// cbAddSurvivePact : callback for adding a survive pact
//
//-----------------------------------------------
void cbAddSurvivePact( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbAddSurvivePact);

	CEntityId player;
	uint8 PactNature, PactType;

	msgin.serial( player );
	msgin.serial( PactNature );
	msgin.serial( PactType );

	CCharacter * c = PlayerManager.getChar( player );
	if( c && c->getEnterFlag() )
	{
		c->setAfkState(false);
		c->addPact( PactNature, PactType );
	}
	else
	{
		nlwarning("<cbAddSurvivePact> Unknown character %s", player.toString().c_str() );
	}
}


//-----------------------------------------------
// cbFameChange:: Change one fame of character
//
//-----------------------------------------------
/*
void cbFameChange( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId id;
	uint8 fameChange;
	sint32 changeValue;

	msgin.serial( id );
	msgin.serial( fameChange );
	msgin.serial( changeValue );

	CCharacter * c = PlayerManager.getChar( id );
	if( c )
	{
		c->fameChange( fameChange, changeValue );
	}
	else
	{
		nlwarning("<cbFameChange> Unknown character %s", id.toString().c_str() );
	}
}
*/

///////////////// Callback for creatures /////////////////
///////////////// Callback for creatures /////////////////
///////////////// Callback for creatures /////////////////


//-----------------------------------------------
// cbAddCreature : <deprecated>
//
//-----------------------------------------------
/*void cbAddCreature( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	// read the creature id
	CEntityId Id;
	msgin.serial( Id );

	// read creature sheet id
	uint32 Sheet;
	msgin.serial( Sheet );

	CCreature * creature = new CCreature();

	creature->setId( Id );
	creature->addPropertiesToMirror();

	creature->loadSheetCreature( CSheetId(Sheet) );

	CreatureManager.addCreature( Id, creature );
}*/ // cbAddCreature //

//-----------------------------------------------
// cbRemoveCreature : <deprecated>
//
//-----------------------------------------------
/*void cbRemoveCreature( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId)
{
	CEntityId Id;
	msgin.serial( Id );

	CreatureManager.removeCreature( Id );
}*/ // cbRemoveCreature //


///////////////// Callback for both clients and creatures /////////////////
///////////////// Callback for both clients and creatures /////////////////
///////////////// Callback for both clients and creatures /////////////////

//-----------------------------------------------
// cbSetValue :
//
//-----------------------------------------------
void cbSetValue( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbSetValue);

	// read the entity id
	CEntityId Id;
	msgin.serial( Id );

	// read the var name
	string var;
	msgin.serial( var );

	// read the var value
	string value;
	msgin.serial( value );

//	egs_ecinfo("*** cbSetValue from service %s change value %s to %s", serviceName.c_str(), var.c_str(), value.c_str() );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( Id );

	if (entity == 0)
	{
		nlwarning("<EntityCallbacks::cbSetValue> Invalid entity Id %s", Id.toString().c_str() );
		return;
	}

	entity->setValue( var, value );
} // cbSetValue //

//-----------------------------------------------
// cbModifyCharValue :
//
//-----------------------------------------------
void cbModifyValue( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbModifyValue);

	// read the creature id
	CEntityId Id;
	msgin.serial( Id );

	// read the var name
	string var;
	msgin.serial( var );

	// read the modification value
	string modifValue;
	msgin.serial( modifValue );

//	egs_ecinfo("*** cbModifyValue from service %s modify value %s to %s", serviceName.c_str(), var.c_str(), modifValue.c_str() );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( Id );

	if (entity == 0)
	{
		nlwarning("<EntityCallbacks::cbModifyValue> Invalid entity Id %s", Id.toString().c_str() );
		return;
	}
	entity->modifyValue( var, modifValue );
} // cbModifyValue //

//-----------------------------------------------
// cbTarget :
//-----------------------------------------------
void cbTarget( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbTarget);

	if ( ! Mirror.mirrorIsReady() )
	{
		nlwarning("<cbTarget> Received from %s service but mirror not yet ready", serviceName.c_str() );
		return;
	}

	// Now all target info in in TDataSetRow

	// read the entity id
	TDataSetRow	Id;
	msgin.serial(Id);

	// read the target id
	TDataSetRow	targetId;
	msgin.serial(targetId);

	// Id should be valid because it's coming from a live player on the FS or on the AIS
	//nlassert( TheDataset.isDataSetRowStillValid( Id ) );
	// If targetId is not valid, make getEntityId return Unknown
	if ( ! TheDataset.isDataSetRowStillValid( targetId ) )
	{
		targetId = TDataSetRow::createFromRawIndex( INVALID_DATASET_INDEX );
	}

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr(Id);
	CEntityId	EentityId=TheDataset.getEntityId(Id);
	CEntityId	EtargetId=TheDataset.getEntityId(targetId);

	if (entity == 0)
	{
		nlwarning("<cbTarget> Invalid entity Id %s", EentityId.toString().c_str() );
		return;
	}
	if	(entity->getId().getType()==RYZOMID::player)
	{
		entity->setTarget(EtargetId);
		((CCharacter*)entity)->incInterfaceCounter();
	}

} // cbTarget //

//-----------------------------------------------
// cbChangeMode :
//-----------------------------------------------
void cbChangeMode( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbChangeMode);

	/*// read the character id
	CEntityId Id;
	msgin.serial( Id );
	*/
	// now use TDataSetRow
	TDataSetRow row;
	msgin.serial( row );

	// read the new Mode
	MBEHAV::TMode mode;
	msgin.serial( mode );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( row );

	if (entity == NULL)
	{
		nlwarning("<cbChangeMode> Invalid entity row %d", row.getIndex() );
		return;
	}

	//egs_ecinfo("<cbChangeMode> Set mode %d for entity %s from service %s", mode, Id.toString().c_str(), serviceName.c_str() );

	entity->setMode( mode );
} // cbChangeMode //

//-----------------------------------------------
// cbChangeBehaviour
//-----------------------------------------------
void cbChangeBehaviour( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbChangeBehaviour);

	// read the character id
	CEntityId Id;
	msgin.serial( Id );

	// read the new Mode
	MBEHAV::CBehaviour behaviour;
	msgin.serial( behaviour );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( Id );

	if (entity == 0)
	{
		nlwarning("<cbChangeBehaviour> Invalid entity Id %s", Id.toString().c_str() );
		return;
	}

//	egs_ecinfo("<cbChangeBehaviour> Set behaviour %s (value %d) for entity %s from service %s", behaviour.toString().c_str(), (uint32)( behaviour.Behaviour << 16 ) + behaviour.Data, Id.toString().c_str(), serviceName.c_str() );

	entity->setBehaviour( behaviour );
} // cbChangeBehaviour //

//---------------------------------------------------
// cbItemDrop :
//---------------------------------------------------
void cbItemDrop( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	nlwarning("cbItemDrop no more allowed");
//	H_AUTO(cbItemDrop);
//
//	CEntityId user;
//	uint16 inventory,slot,quantity;
//
//	msgin.serial(user);
//	msgin.serial(inventory);
//	msgin.serial(slot);
//	msgin.serial(quantity);
//
//	CCharacter *character = PlayerManager.getChar( user );
//	if (character == NULL)
//	{
//		nlwarning("<cbItemDrop> Invalid player Id %s", user.toString().c_str() );
//		return;
//	}
//	character->setAfkState(false);
//	character->dropItem( INVENTORIES::TInventory(inventory), slot, quantity );
}

//---------------------------------------------------
// cbItemPickup:
//---------------------------------------------------
void cbItemPickup( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbItemPickup);

	CEntityId charId;
	CEntityId entity;
	uint8	  type;

	msgin.serial( charId );
	msgin.serial( entity );
	msgin.serial( type );

//	egs_ecinfo("<cbItemPickup> Player Id %s pick up on entity %s, pickup type = %u", charId.toString().c_str(), entity.toString().c_str(), type );

	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbItemPickup> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);

	if (LHSTATE::TLHState(type) == LHSTATE::HARVESTABLE)
		character->itemPickup( entity, true );
	else
		character->itemPickup( entity, false );
}

//---------------------------------------------------
// cbItemClosePickup :
//---------------------------------------------------
void cbItemClosePickup( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbItemClosePickup);

	CEntityId charId;
	msgin.serial( charId );

	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbItemClosePickup> Invalid player Id %s", charId.toString().c_str() );
		return;
	}
	character->setAfkState(false);
	character->pickUpItemClose();
}

//---------------------------------------------------
// cbItemSwap :
//---------------------------------------------------
void cbItemSwap( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbItemSwap);

	CEntityId charId;
	uint16 slotSrc, slotDst, quantity;
	INVENTORIES::TInventory inventorySrc, inventoryDst;
	uint16 temp;

	msgin.serial( charId );
	msgin.serial( temp );
	inventorySrc = INVENTORIES::TInventory(temp);
	msgin.serial( slotSrc );
	msgin.serial( temp );
	inventoryDst = INVENTORIES::TInventory(temp);
	msgin.serial( slotDst );
	msgin.serial( quantity );

	CCharacter *character = PlayerManager.getChar( charId );
	if (character == NULL)
	{
		nlwarning("<cbItemSwap> Invalid player Id %s", charId.toString().c_str() );
		return;
	}

	TLogContext_Item_Swap logContext(character->getId());

	character->setAfkState(false);

	// increment inventory counter
	//character->incActionCounter();
	character->incInterfaceCounter();

	// Special case for guilds
	if (inventorySrc == INVENTORIES::guild || inventoryDst == INVENTORIES::guild)
	{
		uint16 nGuildSessionCounter;
		msgin.serial( nGuildSessionCounter );

		string sDebug = "<cbItemSwap> user:" + TheDataset.getEntityId(character->getEntityRowId()).toString() + " ";

		CGuild *pGuild = CGuildManager::getInstance()->getGuildFromId( character->getGuildId() );
		if (pGuild == NULL)
		{
			nlwarning("%s user is not in a guild.", sDebug.c_str());
			return;
		}

		if (inventorySrc == (uint16) INVENTORIES::guild)
		{
			if (inventoryDst != (uint16) INVENTORIES::bag)
			{
				nlwarning("%s user try to move an item from guild inventory to inventory %d : this is not allowed.", sDebug.c_str(), inventoryDst);
				return;
			}
			// Guild -> Bag
			pGuild->takeItem(character, slotSrc, quantity, nGuildSessionCounter);
		}
		else if (inventoryDst == (uint16) INVENTORIES::guild)
		{
			if (inventorySrc != (uint16) INVENTORIES::bag)
			{
				nlwarning("%s user try to move an item from inventory %d to guild inventory : this is not allowed.", sDebug.c_str(), inventorySrc);
				return;
			}
			// Bag -> Guild
			pGuild->putItem(character, slotSrc, quantity, nGuildSessionCounter);
		}

		return;
	}
	else if (inventorySrc == INVENTORIES::temporary || inventoryDst == INVENTORIES::temporary)
	{
		uint16 slot = slotSrc;
		if (inventoryDst == INVENTORIES::temporary)
			slot = slotDst;

		character->harvestAsked(slot, quantity);
	}
	else
	{
		// autostack by default (ignore destination slot furnished by client)
		character->moveItem(inventorySrc, slotSrc, inventoryDst, INVENTORIES::INSERT_IN_FIRST_FREE_SLOT, quantity);
	}
}

//---------------------------------------------------
// cbEngage: engage current target in combat
//---------------------------------------------------
void cbEngage( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbEngage);

	CEntityId charId;
	msgin.serial( charId );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( charId );

	if (entity == NULL)
	{
		nlwarning("<EntityCallbacks::cbEngage> Invalid entity Id %s", charId.toString().c_str() );
		return;
	}

	CEntityId targetId = entity->getTarget();

	vector<CSheetId>	bricks;
	bricks.push_back( CSheetId("engage.brick") );
	//bricks.push_back( CSheetId("engage_mode_1.brick_base") );

	vector<NLMISC::CSheetId> mpsSheet;
	vector<uint16> mpsQty;
	vector<uint16> mpsQuality;

	DEBUGLOG("<EntityCallbacks::cbEngage> entity %d engage entity %d", charId.toString().c_str(), targetId.toString().c_str() );

//	CBrickSentenceManager::executePhrase( charId, targetId, bricks, mpsSheet, mpsQty, mpsQuality);

} // cbEngage //




//---------------------------------------------------
// cbDefaultAttack: engage current target in combat
//---------------------------------------------------
void cbDefaultAttack( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbDefaultAttack);

	CEntityId entityId;
	msgin.serial( entityId );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( entityId );

	if (entity == NULL)
	{
		nlwarning("<EntityCallbacks::cbDefaultAttack> Invalid entity Id %s", entityId.toString().c_str() );
		return;
	}

	// can entity use actions ?
	if (!entity->canEntityUseAction())
		return;

	CEntityId targetId = entity->getTarget();
	CPhraseManager::getInstance().defaultAttackSabrina( entityId, targetId );

} // cbDefaultAttack //


//---------------------------------------------------
// cbStun: stun entity
//---------------------------------------------------
void cbStun( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbStun);

	CEntityId charId;
	msgin.serial( charId );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( charId );

	if (entity == NULL)
	{
		nlwarning("<EntityCallbacks::cbStun> Invalid entity Id %s", charId.toString().c_str() );
		return;
	}

	entity->stun();
} // cbStun //


//---------------------------------------------------
// cbWake: wake entity
//---------------------------------------------------
void cbWake( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbWake);

	CEntityId charId;
	msgin.serial( charId );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( charId );

	if (entity == NULL)
	{
		nlwarning("<EntityCallbacks::cbWake> Invalid entity Id %s", charId.toString().c_str() );
		return;
	}

	entity->wake();
} // cbWake //




//---------------------------------------------------
// cbExchangeProposal: invite an entity to exchange
//---------------------------------------------------
void cbExchangeProposal( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbExchangeProposal);

	CEntityId charId;
	msgin.serial( charId );
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		c->setAfkState(false);
		c->exchangeProposal();
	}
	else
		nlwarning("<cbExchangeProposal> Unknown character %s", charId.toString().c_str() );
}

//---------------------------------------------------
// accept exchange invitation
//---------------------------------------------------
void cbAcceptExchangeInvitation( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbAcceptExchangeInvitation);

	CEntityId charId;
	msgin.serial( charId );
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		c->setAfkState(false);
		c->acceptExchangeInvitation();
	}
	else
		nlwarning("<cbAcceptExchangeProposal> Unknown character %s", charId.toString().c_str() );
}

//---------------------------------------------------
// decline exchange invitation
//---------------------------------------------------
void cbDeclineExchangeInvitation( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbDeclineExchangeInvitation);

	CEntityId charId;
	msgin.serial( charId );
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		c->setAfkState(false);
		c->declineExchangeInvitation();
	}
	else
		nlwarning("<cbDeclineExchangeInvitation> Unknown character %s", charId.toString().c_str() );
}


//---------------------------------------------------
// accept exchange
//---------------------------------------------------
void cbAcceptExchange( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbAcceptExchange);

	CEntityId charId;
	uint8 exchangeId;
	msgin.serial( charId );
	msgin.serial( exchangeId );
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		c->acceptExchange(exchangeId);
		c->setAfkState(false);
	}
	else
		nlwarning("<cbAcceptExchange> Unknown character %s", charId.toString().c_str() );
}

//---------------------------------------------------
// end exchange
//---------------------------------------------------
void cbEndExchange( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbEndExchange);

	CEntityId charId;
	msgin.serial( charId );
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		c->setAfkState(false);
		c->incInterfaceCounter();
		c->abortExchange();
	}
	else
		nlwarning("<cbEndExchange> Unknown character %s", charId.toString().c_str() );
}

//---------------------------------------------------
// exchange seeds
//---------------------------------------------------
void cbExchangeSeeds( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbExchangeSeeds);

	///\todo : why not  an uint64?
	CEntityId charId;
	sint64 quantity;
	msgin.serial( charId );
	msgin.serial( quantity );
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		c->setAfkState(false);
		c->incInterfaceCounter();
		c->exchangeMoney(quantity);
	}
	else
		nlwarning("<cbExchangeSeeds> Unknown character %s", charId.toString().c_str() );

}

//---------------------------------------------------
// Entity want mounting
//---------------------------------------------------
void cbAnimalMount( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbAnimalMount);

	CEntityId id;
	msgin.serial( id );

	CCharacter * e = PlayerManager.getChar( id );
	if ( e )
	{
		if ( e->getRiderEntity().isNull() )
		{
			CEntityId target = e->getTarget();
			if( target.getType() == RYZOMID::creature || target.getType() == RYZOMID::npc )
			{
				CEntityBase * mount = CEntityBaseManager::getEntityBasePtr( target );
				if( mount )
				{
					const CStaticCreatures * form = mount->getForm();
					if( form )
					{
						if( form->getProperties().mountable() )
						{
							// test player isn't using a TP
							if (e->getTpTicketSlot() != -1)
							{
								PHRASE_UTILITIES::sendDynamicSystemMessage( e->getEntityRowId(), "MOUNT_CANT_WHILE_TP" );
							}
							else
							{
								//e->mount( e->getTargetDataSetRow() );
								// add mount phrase in manager
								static CSheetId mountBrick("bapa03.sbrick");
								vector<CSheetId> bricks;
								bricks.push_back(mountBrick);
								CPhraseManager::getInstance().executePhrase(e->getEntityRowId(), e->getTargetDataSetRow(), bricks);
							}
							return;
						}
						else
						{
							nlwarning("<cbAnimalMount> %d Target %s %s is not moutable !! sheeter or client bug ?", CTickEventHandler::getGameCycle(), target.toString().c_str(), mount->getType().toString().c_str() );
						}
					}
					else
					{
						nlwarning("<cbAnimalMount> %d Can't found static form sheet for entity %s %s !!", CTickEventHandler::getGameCycle(), target.toString().c_str(), mount->getType().toString().c_str() );
					}
				}
			}
		}
		else
		{
			egs_ecinfo("<cbAnimalMount> %d Target %s has a rider !!", CTickEventHandler::getGameCycle(), e->getTarget().toString().c_str() );
		}
	}

	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( id );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "ANIMALS:MOUNT_ABORT", bms) )
	{
		nlwarning("<cbAnimalMount> Msg name ANIMALS:MOUNT_ABORT not found");
		return;
	}
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(id.getDynamicId()), msgout );
}

//---------------------------------------------------
// Entity unseat
//---------------------------------------------------
void cbAnimalUnseat( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(cbAnimalUnseat);

	CEntityId id;
	msgin.serial( id );

	CCharacter * e = PlayerManager.getChar( id );
	if( e )
	{
		//e->unmount();
		static CSheetId unmountBrick("bapa04.sbrick");
		vector<CSheetId> bricks;
		bricks.push_back(unmountBrick);
		CPhraseManager::getInstance().executePhrase(e->getEntityRowId(), e->getEntityRowId(), bricks);
	}
}

//---------------------------------------------------
// Set player weather
//---------------------------------------------------
void cbSetPlayerWeather(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbSetPlayerWeather);
	uint32 charId;
	uint16 weatherValue;
	msgin.serial(charId);
	msgin.serial(weatherValue);

	CCharacter *chr = PlayerManager.getChar(charId>>4, charId&0xf);
	if (!chr)
	{
		nlwarning("cbSetPlayerWeather : no active character %u ", charId);
		return;
	}
	chr->setWeatherValue(weatherValue);
}

//---------------------------------------------------
// Set player season
//---------------------------------------------------
void cbSetPlayerSeason(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbSetPlayerWeather);
	uint32 charId;
	uint8 season;
	msgin.serial(charId);
	msgin.serial(season);

	CCharacter *chr = PlayerManager.getChar(charId>>4, charId&0xf);
	if (!chr)
	{
		nlwarning("cbSetPlayerSeason : no active character %u" , charId);
		return;
	}
	PlayerManager.sendImpulseToClient(chr->getId(), "SEASON:SET", season);
}


//---------------------------------------------------
// teleport the player
//---------------------------------------------------
void cbTeleportPlayer(NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbTeleportPlayer);

	CEntityId playerEid;
	sint32 x, y, z;
	float t;
	nlRead(msgin, serial, playerEid );
	nlRead(msgin, serial, x );
	nlRead(msgin, serial, y);
	nlRead(msgin, serial, z );
	nlRead(msgin, serial, t);


	CCharacter *chr = PlayerManager.getChar(playerEid);
	if (!chr)
	{
		return;
	}
	chr->teleportCharacter(x, y, z, true, true, t);
}


//---------------------------------------------------
/// Forage source position validation
//---------------------------------------------------
void cbRcvValidateSourceSpawnReply( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(RcvValidateSourceSpawnReply);

	// Read header
	TDataSetRow prospectorDataSetRow;
	uint32 nbSources, nbSpawnedSources = 0;
	msgin.fastRead( nbSources );

	for ( uint i=0; i!=nbSources; ++i )
	{
		// Access the source
		TDataSetRow sourceDataSetRow;
		msgin.serial( sourceDataSetRow );
		CHarvestSource *harvestSource = CHarvestSourceManager::getInstance()->getEntity( sourceDataSetRow );
		if ( ! harvestSource )
		{
			nlwarning( "Source E%u to spawnEnd not found", sourceDataSetRow.getIndex() );
			return;
		}
		bool canSpawn;
		msgin.serial( canSpawn );
		if ( canSpawn )
			++nbSpawnedSources;
		if ( i == 0 )
			prospectorDataSetRow = harvestSource->getProspectorDataSetRow();

		// End spawning (either leave it or destroy it)
		harvestSource->spawnEnd( canSpawn );
	}

	if ( ! prospectorDataSetRow.isNull() ) // is null for auto-spawned sources
	{
		// Report the sources found (or not found) to the user
		if ( nbSpawnedSources != 0 )
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = (sint32)nbSpawnedSources;
			PHRASE_UTILITIES::sendDynamicSystemMessage( prospectorDataSetRow, "FORAGE_FOUND_SOURCES", params );
		}
		else
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage( prospectorDataSetRow, "FORAGE_SOURCE_OBSTACLE" );
		}
	}
}

//---------------------------------------------------
// A creature can't reach a player
//---------------------------------------------------
void cbPlayerUnreachable( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbPlayerUnreachable);

	CEntityId creatureId;
	msgin.serial( creatureId );
	CCreature * creature = CreatureManager.getCreature( creatureId );
	if( !creature )
	{
		nlwarning("<cbPlayerUnreachable> Unknown creature %s", creatureId.toString().c_str() );
		return;
	}

	CEntityId charId;
	msgin.serial( charId );
	CCharacter * character = PlayerManager.getChar( charId );
	if( !character )
	{
		nlwarning("<cbPlayerUnreachable> Unknown character %s", charId.toString().c_str() );
		return;
	}

	// Do nothing for now, damage removal is done in aggro lost.
}

