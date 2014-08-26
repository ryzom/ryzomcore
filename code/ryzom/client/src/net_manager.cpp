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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Game Share
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/bot_chat_types.h"
#include "game_share/news_types.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/chat_group.h"
#include "game_share/character_summary.h"
#include "game_share/sphrase_com.h"
#include "game_share/msg_client_server.h"
#include "game_share/ryzom_database_banks.h"
#include "game_share/msg_encyclopedia.h"
#include "game_share/prerequisit_infos.h"
#include "game_share/permanent_ban_magic_number.h"
#include "game_share/item_special_effect.h"
#include "game_share/combat_flying_text.h"
#include "game_share/shard_names.h"
// Client.
#include "nel/gui/group_list.h"
#include "interface_v3/interface_manager.h"
#include "net_manager.h"
#include "client_cfg.h"
#include "entities.h"
#include "client_chat_manager.h"
#include "world_database_manager.h"
#include "continent_manager.h"
#include "motion/user_controls.h"
#include "interface_v3/bot_chat_manager.h"
#include "interface_v3/bot_chat_page_all.h"
#include "interface_v3/bot_chat_page_trade.h"
#include "interface_v3/bot_chat_page_create_guild.h"
#include "interface_v3/obs_huge_list.h"
#include "string_manager_client.h"
#include "interface_v3/people_interraction.h"
#include "interface_v3/bot_chat_manager.h"
#include "interface_v3/bot_chat_page_all.h"
#include "nel/gui/view_text_id.h"
#include "nel/gui/ctrl_text_button.h"
#include "interface_v3/input_handler_manager.h"
#include "interface_v3/guild_manager.h"
#include "interface_v3/skill_manager.h"
#include "misc.h"
#include "interface_v3/inventory_manager.h"
#include "interface_v3/sphrase_manager.h"
#include "outpost_manager.h"
#include "interface_v3/encyclopedia_manager.h"
#include "user_entity.h"
#include "init_main_loop.h"
#include "interface_v3/group_map.h"
#include "sound_manager.h"
#include "interface_v3/group_compas.h"
#include "interface_v3/group_html_webig.h"
#include "interface_v3/bar_manager.h"
#include "permanent_ban.h"
#include "global.h"
#include "connection.h"
#include "faction_war_manager.h"
#include "far_tp.h"
#include "input.h"
#include "r2/editor.h"
#include "game_share/r2_share_itf.h"
#include "game_share/r2_types.h"
#include "npc_icon.h"
#include "interface_v3/action_handler_base.h"

// Std.
#include <vector>


#define OLD_STRING_SYSTEM
#define BAR_STEP_TP 2

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace std;


extern bool	FirstFrame;
extern void selectTipsOfTheDay (uint tips);


////////////
// GLOBAL //
////////////
CGenericXmlMsgHeaderManager	GenericMsgHeaderMngr;	// Manage messages
#ifdef CLIENT_MULTI
CNetManagerMulti			NetMngr;				// Manage the connection.
#else
CNetManager					NetMngr;				// Manage the connection.
#endif

bool UseFemaleTitles = false;

bool serverReceivedReady = false;

static const std::string PLAYER_EXCHANGE_INVITATION_DIALOG = "ui:interface:accept_trade_invitation";

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Net_Mngr_Update )

////////////
// EXTERN //
////////////
extern bool								noUserChar;	// \todo GUIGUI : do this better.
extern bool								userChar;	// \todo GUIGUI : do this better.
extern std::vector<CCharacterSummary>	CharacterSummaries;
extern uint8 ServerPeopleActive;
extern uint8 ServerCareerActive;
extern vector<CMainlandSummary>			Mainlands;
extern bool								UserCharPosReceived;
extern CGenericXmlMsgHeaderManager		GenericMsgHeaderMngr;
extern CClientChatManager				ChatMngr;

extern bool CharNameValidArrived;
extern bool CharNameValid;
bool IsInRingSession = false;
TSessionId HighestMainlandSessionId; // highest in the position stack

extern const char *CDBBankNames[INVALID_CDB_BANK+1];

void cbImpulsionGatewayOpen(NLMISC::CBitMemStream &bms);
void cbImpulsionGatewayMessage(NLMISC::CBitMemStream &bms);
void cbImpulsionGatewayClose(NLMISC::CBitMemStream &bms);



///////////////
// FUNCTIONS //
///////////////

void impulseDatabaseInitPlayer(NLMISC::CBitMemStream &impulse)
{
	try
	{
		sint32 p = impulse.getPos();

		// get the egs tick of this change
		TGameCycle	serverTick;
		impulse.serial(serverTick);

		// read delta
		IngameDbMngr.readDelta( serverTick, impulse, CDBPlayer );
		IngameDbMngr.setInitPacketReceived();
		nlinfo( "DB_INIT:PLR done (%u bytes)", impulse.getPos()-p );
	}
	catch (const Exception &e)
	{
		BOMB( NLMISC::toString( "Problem while decoding a DB_INIT:PLR msg, skipped: %s", e.what() ), return );
	}
}

void impulseDatabaseUpdatePlayer(NLMISC::CBitMemStream &impulse)
{
	try
	{
		// get the egs tick of this change
		TGameCycle	serverTick;
		impulse.serial(serverTick);

		// read delta
		IngameDbMngr.readDelta( serverTick, impulse, CDBPlayer ); // unlike on the server, here there is only one unified CCDBSynchronized object
	}
	catch (const Exception &e)
	{

		BOMB( NLMISC::toString( "Problem while decoding a DB_UPDATE_PLR msg, skipped: %s", e.what() ), return );
	}
}

template <class CInventoryCategoryTemplate>
void updateInventoryFromStream(NLMISC::CBitMemStream &impulse, const CInventoryCategoryTemplate *templ, bool notifyItemSheetChanges);

void impulseDatabaseUpdateBank(NLMISC::CBitMemStream &impulse)
{
	uint32 bank = INVALID_CDB_BANK;
	try
	{
		// get the egs tick of this change
		TGameCycle	serverTick;
		impulse.serial(serverTick);

		// decode bank
		uint nbits;
		FILL_nbits_WITH_NB_BITS_FOR_CDBBANK
		impulse.serial( bank, nbits );

		// read delta
		IngameDbMngr.readDelta( serverTick, impulse, (TCDBBank)bank );

		// read guild inventory update
		if ( bank == CDBGuild )
		{
			updateInventoryFromStream( impulse, (INVENTORIES::CInventoryCategoryForGuild*)NULL, false );
		}
	}
	catch (const Exception &e)
	{
		BOMB( NLMISC::toString( "Problem while decoding a DB_GROUP:UPDATE_BANK %s msg, skipped: %s", CDBBankNames[bank], e.what() ), return );
	}
}

void impulseDatabaseInitBank(NLMISC::CBitMemStream &impulse)
{
	uint32 bank = INVALID_CDB_BANK;
	try
	{
		// get the egs tick of this change
		TGameCycle	serverTick;
		impulse.serial(serverTick);

		// decode bank
		uint nbits;
		FILL_nbits_WITH_NB_BITS_FOR_CDBBANK
		impulse.serial( bank, nbits );

		// read delta
		IngameDbMngr.readDelta( serverTick, impulse, (TCDBBank)bank );
		nldebug( "CDB: DB_GROUP:INIT_BANK %s", CDBBankNames[bank] );

		// read guild inventory update
		if ( bank == CDBGuild )
		{
			updateInventoryFromStream( impulse, (INVENTORIES::CInventoryCategoryForGuild*)NULL, false );
		}
	}
	catch (const Exception &e)
	{
		BOMB( NLMISC::toString( "Problem while decoding a DB_GROUP:INIT_BANK %s msg, skipped: %s", CDBBankNames[bank], e.what() ), return );
	}
}

void impulseDatabaseResetBank(NLMISC::CBitMemStream &impulse)
{
	uint32 bank = INVALID_CDB_BANK;
	try
	{
		// get the egs tick of this change
		TGameCycle	serverTick;
		impulse.serial(serverTick);

		// read the bank to reset
		uint nbits;
		FILL_nbits_WITH_NB_BITS_FOR_CDBBANK
		impulse.serial( bank, nbits );

		// reset the bank
		IngameDbMngr.resetBank( serverTick, bank );
		nldebug( "CDB: DB_GROUP:RESET_BANK %s", CDBBankNames[bank] );
	}
	catch (const Exception &e)
	{
		BOMB( NLMISC::toString( "Problem while decoding a DB_GROUP:RESET_BANK %s msg, skipped: %s", CDBBankNames[bank], e.what() ), return );
	}
}

static void readPrivileges(NLMISC::CBitMemStream &impulse)
{
	nlassert(impulse.isReading());
	// nico : temporarily uses a try block here to avoid prb with people having updated client and not the server
	try
	{
		impulse.serial(UserPrivileges);
	}
	catch(const EStreamOverflow &)
	{
		nlwarning("User privileges not serialised, assuming none");
		UserPrivileges = "";
	}
}

void impulseNoUserChar(NLMISC::CBitMemStream &impulse)
{
	// received NO_USER_CHAR
	//nlinfo("impulseCallBack : Received CONNECTION:NO_USER_CHAR");

	impulse.serial(ServerPeopleActive);
	impulse.serial(ServerCareerActive);
	readPrivileges(impulse);
	impulse.serialCont(Mainlands);
	CharacterSummaries.clear();
	noUserChar = true;

	LoginSM.pushEvent(CLoginStateMachine::ev_no_user_char);

	updatePatcherPriorityBasedOnCharacters();
}

void impulseFarTP(NLMISC::CBitMemStream &impulse)
{
	// received FAR_TP
	TSessionId sessionId;
	impulse.serial(sessionId);
	//nlinfo("impulseCallback : Received CONNECTION:FAR_TP %u", sessionId.asInt());
	bool bailOutIfSessionVanished;
	impulse.serial(bailOutIfSessionVanished);
	FarTP.requestFarTPToSession(sessionId, PlayerSelectedSlot, CFarTP::JoinSession, bailOutIfSessionVanished);
}


static std::string lookupSrcKeyFile(const std::string &src)
{
	if (CFile::isExists("save/" + src)) return "save/" + src;
	return CPath::lookup(src, false);
}

void copyKeySet(const std::string &srcPath, const std::string &destPath)
{
	// can't use CFile copyFile here, because src may be in a bnp file
	std::string srcStr;
	srcStr.resize(CFile::getFileSize(srcPath));
	if (srcStr.empty())
	{
		nlwarning("Can't copy keys from %s : file not found or empty");
		return;
	}
	try
	{
		CIFile ifile(srcPath);
		ifile.serialBuffer((uint8 *) &srcStr[0], (uint)srcStr.size());
		COFile ofile(destPath);
		ofile.serialBuffer((uint8 *) &srcStr[0], (uint)srcStr.size());
	}
	catch(const EStream &)
	{
		nlwarning("Couldn't copy %s to %s to create new character keyset", srcPath.c_str(), destPath.c_str());
	}
}

void impulseUserChars(NLMISC::CBitMemStream &impulse)
{
	// received USER_CHARS
	//nlinfo("impulseCallBack : Received CONNECTION:USER_CHARS");

	impulse.serial(ServerPeopleActive);
	impulse.serial(ServerCareerActive);
	// read characters summary
	CharacterSummaries.clear();
	impulse.serialCont (CharacterSummaries);
	// read shard name summaries
	std::vector<string>		shardNames;
	impulse.serialCont (shardNames);
	CShardNames::getInstance().loadShardNames(shardNames);
	// read privileges
	readPrivileges(impulse);
	impulse.serial(FreeTrial);
	FreeTrial = false;
	impulse.serialCont(Mainlands);
	userChar = true;

	LoginSM.pushEvent(CLoginStateMachine::ev_chars_received);

	// Create the message for the server to select the first character.
/*	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:SELECT_CHAR", out))
	{
		CSelectCharMsg	SelectCharMsg;
		SelectCharMsg.c = 0;	//TODO set here the character choosen by player
		out.serial( SelectCharMsg );
		NetMngr.push(out);
		NetMngr.send(NetMngr.getCurrentServerTick());
		// send CONNECTION:USER_CHARS
		nldebug("impulseCallBack : CONNECTION:SELECT_CHAR sent");
	}
	else
		nlwarning("impulseCallBack : unknown message name : 'CONNECTION:SELECT_CHAR'.");

	noUserChar = true;
	*/

	if (!NewKeysCharNameValidated.empty())
	{
		// if there's a new char for which a key set was wanted, create it now
		for (uint k = 0; k < CharacterSummaries.size(); ++k)
		{
			if (toLower(CharacterSummaries[k].Name) == toLower(NewKeysCharNameValidated))
			{
				// first, stripes server name
				copyKeySet(lookupSrcKeyFile(GameKeySet), "save/keys_" + buildPlayerNameForSaveFile(NewKeysCharNameValidated) + ".xml");
				copyKeySet(lookupSrcKeyFile(RingEditorKeySet), "save/keys_r2ed_" + buildPlayerNameForSaveFile(NewKeysCharNameValidated) + ".xml");
				break;
			}
		}
	}
	updatePatcherPriorityBasedOnCharacters();
}

void impulseUserChar(NLMISC::CBitMemStream &impulse)
{
	// received USER_CHAR
	//nlinfo("impulseCallBack : Received CONNECTION:USER_CHAR");

	// Serialize the message
	COfflineEntityState posState;
	extern uint8 ServerSeasonValue;
	extern bool ServerSeasonReceived;
	uint32 userRole;
	CUserCharMsg::read( impulse, posState, ServerSeasonValue, userRole, IsInRingSession, HighestMainlandSessionId, CharFirstConnectedTime, CharPlayedTime );
	ServerSeasonReceived = true; // set the season that will be used when selecting the continent from the position

	if (UserEntity)
	{
		UserEntity->pos(CVectorD((float)posState.X/1000.0f, (float)posState.Y/1000.0f, (float)posState.Z/1000.0f));
		UserEntity->front(CVector((float)cos(posState.Heading), (float)sin(posState.Heading), 0.f));
		UserEntity->dir(UserEntity->front());
		UserEntity->setHeadPitch(0);
		UserControls.resetCameraDeltaYaw();
		//nldebug("<impulseUserChar> pos : %f %f %f  heading : %f",UserEntity->pos().x,UserEntity->pos().y,UserEntity->pos().z,posState.Heading);

		// Update the position for the vision.
		NetMngr.setReferencePosition(UserEntity->pos());
	}
	else
	{
		UserEntityInitPos = CVectorD((float)posState.X/1000.0f, (float)posState.Y/1000.0f, (float)posState.Z/1000.0f);
		UserEntityInitFront = CVector((float)cos(posState.Heading), (float)sin(posState.Heading), 0.f);
		//nldebug("<impulseUserChar> pos : %f %f %f  heading : %f",UserEntityInitPos.x,UserEntityInitPos.y,UserEntityInitPos.z,posState.Heading);

		// Update the position for the vision.
		NetMngr.setReferencePosition(UserEntityInitPos);
	}

	UserCharPosReceived = true;

	// Configure the ring editor
	extern R2::TUserRole UserRoleInSession;
	UserRoleInSession = R2::TUserRole::TValues(userRole);
	ClientCfg.R2EDEnabled = IsInRingSession /*&& (UserRoleInSession.getValue() != R2::TUserRole::ur_player)*/;
	// !!!Do NOT uncomment the following line  do the  ClientCfg.R2EDEnabled = IsInRingSession && (UserRoleInSession != R2::TUserRole::ur_player);
	// even with UserRoleInSession R2::TUserRole::ur_player the ring features must be activated
	// because if the ring is not activated the dss do not know the existence of the player
	// So we can not kick him, tp to him, tp in to next act ....
	nldebug( "EnableR2Ed = %u, IsInRingSession = %u, UserRoleInSession = %u", (uint)ClientCfg.R2EDEnabled, (uint)IsInRingSession, userRole );

	updatePatcherPriorityBasedOnCharacters();
}

void impulseCharNameValid(NLMISC::CBitMemStream &impulse)
{
	//nlinfo("impulseCallBack : Received CONNECTION:VALID_NAME");
	uint8 nTmp;
	impulse.serial(nTmp);
	CharNameValid = ((nTmp != 0) ? true : false);
	CharNameValidArrived = true;
	if (CharNameValid) NewKeysCharNameValidated = NewKeysCharNameWanted;
}


void checkHandshake( NLMISC::CBitMemStream &impulse )
{
	// Decode handshake to check versions
	uint16 handshakeVersion;
	uint16 itemSlotVersion;
	impulse.serial( handshakeVersion );
	if ( handshakeVersion > 0 )
		nlerror( "Server handshake version is more recent than client one" );
	impulse.serial( itemSlotVersion );
	if ( itemSlotVersion != INVENTORIES::CItemSlot::getVersion() )
		nlerror( "Handshake: itemSlotVersion mismatch (S:%hu C:%hu)", itemSlotVersion, INVENTORIES::CItemSlot::getVersion() );
}


void impulseServerReady(NLMISC::CBitMemStream &impulse)
{
	// received CONNECTION:READY
	//nlinfo("impulseCallBack : Received CONNECTION:READY");

	serverReceivedReady = true;

	checkHandshake( impulse );

	LoginSM.pushEvent(CLoginStateMachine::ev_ready_received);
}

void impulseShardId(NLMISC::CBitMemStream &impulse)
{
	// received SHARD_ID

	uint32	shardId;
	impulse.serial(shardId);
	ShardId = shardId;

	string	webHost;
	impulse.serial(webHost);
	if (webHost != "")
	{
		WebServer = webHost;
	}

	nlinfo("WEB: Received SHARD_ID %d, web hosted at '%s', using '%s'", shardId, webHost.c_str(), WebServer.c_str());
}

void impulseServerQuitOk(NLMISC::CBitMemStream &impulse)
{
	// receive CONNECTION:SERVER_QUIT_OK
	if (FarTP.isFarTPInProgress())
	{
		FarTP.onServerQuitOk();
	}
	else
	{
		// ensure first a quit request is really asked
		if(game_exit_request)
		{
			// quit!
			game_exit= true;
			ryzom_exit= true;
		}
	}
}

void impulseServerQuitAbort(NLMISC::CBitMemStream &impulse)
{
	// receive CONNECTION:SERVER_QUIT_ABORT
	if (FarTP.isFarTPInProgress())
	{
		FarTP.onServerQuitAbort();
	}
	else
	{
		// abort any quit request
		game_exit_request= false;
		ryzom_exit_request= false;
	}
}

void impulseMailNotification(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	// receive CONNECTION:MAIL_AVAILABLE
	CInterfaceManager::getInstance()->notifyMailAvailable();
}

void impulseForumNotification(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	// receive CONNECTION:GUILD_MESSAGE_AVAILABLE
	CInterfaceManager::getInstance()->notifyForumUpdated();

}

void impulsePermanentBan(NLMISC::CBitMemStream &impulse)
{
	uint64 magicNumber;
	impulse.serial(magicNumber);
	if (magicNumber != PermanentBanMSGMagicNumber) return; // bad msg
	setPermanentBanMarkers(true);
	applyPermanentBanPunishment();
	PermanentlyBanned = true;
}

void impulsePermanentUnban(NLMISC::CBitMemStream &impulse)
{
	uint64 magicNumber;
	impulse.serial(magicNumber);
	if (magicNumber != PermanentUnbanMSGMagicNumber) return; // bad msg
	setPermanentBanMarkers(false);
	PermanentlyBanned = false;
	if (UserEntity)
	{
		// allows to walk / run again
		UserEntity->walkVelocity(ClientCfg.Walk);
		UserEntity->runVelocity(ClientCfg.Run);
	}
}


// ***************************************************************************
class CInterfaceChatDisplayer : public CClientChatManager::IChatDisplayer
{
public:
	virtual void displayChat(TDataSetIndex compressedSenderIndex, const ucstring &ucstr, const ucstring &rawMessage, CChatGroup::TGroupType mode, NLMISC::CEntityId dynChatId, ucstring &senderName, uint bubbleTimer=0);
	virtual void displayTell(/*TDataSetIndex senderIndex, */const ucstring &ucstr, const ucstring &senderName);
	virtual void clearChannel(CChatGroup::TGroupType mode, uint32 dynChatDbIndex);

private:
	// Add colorization tag for sender name
	void colorizeSender(ucstring &text, const ucstring &senderName, CRGBA baseColor);

};
static CInterfaceChatDisplayer	InterfaceChatDisplayer;

void CInterfaceChatDisplayer::colorizeSender(ucstring &text, const ucstring &senderName, CRGBA baseColor)
{
	// find the sender/text separator to put color tags
	ucstring::size_type pos = senderName.length() - 1;
	if (pos != ucstring::npos)
	{
		ucstring str;

		CInterfaceProperty prop;
		prop.readRGBA("UI:SAVE:CHAT:COLORS:SPEAKER"," ");

		CChatWindow::encodeColorTag(prop.getRGBA(), str, false);

		str += text.substr(0, pos+1);

		CChatWindow::encodeColorTag(baseColor, str, true);

		str += text.substr(pos+1);

		text.swap(str);
	}
}

// display a chat from network to interface
void CInterfaceChatDisplayer::displayChat(TDataSetIndex compressedSenderIndex, const ucstring &ucstr, const ucstring &rawMessage, CChatGroup::TGroupType mode, NLMISC::CEntityId dynChatId, ucstring &senderName, uint bubbleTimer)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	ucstring finalString;
	string stringCategory = getStringCategory(ucstr, finalString);

	bool bubbleWanted = true;

	// Subtract rawMessage from ucstr so that the 'sender' part remains.
	ucstring senderPart = ucstr.luabind_substr(0, ucstr.length() - rawMessage.length());

	// search a "{no_bubble}" tag
	{
		ucstring::size_type index = finalString.find(ucstring("{no_bubble}"));
		const size_t tokenSize= 11; // length of "{no_bubble}"
		if (index != ucstring::npos)
		{
			bubbleWanted = false;
			finalString = finalString.luabind_substr(0, index) + finalString.substr(index+tokenSize,finalString.size());
		}
	}


	// **** get color
	CRGBA col;
	if (mode != CChatGroup::system)
	{
		// Remove all {break}
		for(;;)
		{
			ucstring::size_type index = finalString.find(ucstring("{break}"));
			if (index == ucstring::npos) break;
			finalString = finalString.luabind_substr(0, index) + finalString.luabind_substr(index+7,finalString.size());
		}

		// select DB
		sint32	dbIndex = ChatMngr.getDynamicChannelDbIndexFromId(dynChatId);
		clamp(dbIndex, (sint32)0 , (sint32)CChatGroup::MaxDynChanPerPlayer);
		string entry="UI:SAVE:CHAT:COLORS:";
		switch(mode)
		{
		case CChatGroup::dyn_chat:	entry+="DYN:" + NLMISC::toString(dbIndex);	break;
		case CChatGroup::say:	entry+="SAY";	break;
		case CChatGroup::shout:	entry+="SHOUT";	break;
		case CChatGroup::team:	entry+="GROUP";	break;
		case CChatGroup::guild:	entry+="CLADE";	break;
		case CChatGroup::civilization:	entry+="CIVILIZATION";	break;
		case CChatGroup::territory:	entry+="TERRITORY";	break;
		case CChatGroup::universe:	entry+="UNIVERSE_NEW";	break;
		case CChatGroup::region:	entry+="REGION";	break;
		case CChatGroup::tell:	entry+="TELL";	break;
		default:	nlwarning("unknown group type");	return;
		}

		// read DB
		CInterfaceProperty prop;
		prop.readRGBA(entry.c_str()," ");
		col = prop.getRGBA();

		// Override color if the string contains the color
		if (!stringCategory.empty() && stringCategory != "SYS")
		{
			map<string, CClientConfig::SSysInfoParam>::const_iterator it;
			it = ClientCfg.SystemInfoParams.find(strlwr(stringCategory));
			if (it != ClientCfg.SystemInfoParams.end())
			{
				col = it->second.Color;
			}
		}
	}

	if (stringCategory == "emt")
	{
		bubbleWanted = false;
	}

	if (mode != CChatGroup::system)
	{
		// find the sender/text separator to put color tags
		if (senderPart.empty() && stringCategory == "emt")
		{
			size_t pos = finalString.find(ucstring(": "), 0);
			if (pos != ucstring::npos)
			{
				senderPart = finalString.luabind_substr(0, pos + 2);
			}
		}
		colorizeSender(finalString, senderPart, col);
	}

	// play associated fx if any
	if( !stringCategory.empty() )
	{
		map<string, CClientConfig::SSysInfoParam>::const_iterator it;
		it = ClientCfg.SystemInfoParams.find( strlwr(stringCategory) );
		if( it != ClientCfg.SystemInfoParams.end() )
		{
			if( !(*it).second.SysInfoFxName.empty() )
			{
				NL3D::UParticleSystemInstance sysInfoFx = FXMngr.instantFX((*it).second.SysInfoFxName);
				if( !sysInfoFx.empty() )
				{
					sysInfoFx.setClusterSystem( UserEntity->getClusterSystem() );
					sysInfoFx.setPos( UserEntity->pos() );
				}
				else
				{
					nlwarning("<CInterfaceChatDisplayer::displayChat> Can't set chat fx %s",(*it).second.SysInfoFxName.c_str());
				}
			}
		}
	}

	// **** redirect to the correct interface output
	if( stringCategory != "bbl" )
	{
		bool windowVisible;
		if (mode == CChatGroup::system)
		{
			pIM->displaySystemInfo(finalString, stringCategory);
		}
		else if (mode == CChatGroup::guild)
		{
			PeopleInterraction.ChatInput.Guild.displayMessage(finalString, col, 2, &windowVisible);
		}
		else if (mode == CChatGroup::team)
		{
			PeopleInterraction.ChatInput.Team.displayMessage(finalString, col, 2, &windowVisible);
		}
		else if (mode == CChatGroup::region)
		{
			PeopleInterraction.ChatInput.Region.displayMessage(finalString, col, 2, &windowVisible);
		}
		else if (mode == CChatGroup::universe)
		{
			PeopleInterraction.ChatInput.Universe.displayMessage(finalString, col, 2, &windowVisible);
		}
		else if (mode == CChatGroup::dyn_chat)
		{
			// retrieve the DBIndex from the dynamic chat id
			sint32	dbIndex= ChatMngr.getDynamicChannelDbIndexFromId(dynChatId);
			// if found, display, else discarded
			if(dbIndex >= 0 && dbIndex < CChatGroup::MaxDynChanPerPlayer)
			{
				PeopleInterraction.ChatInput.DynamicChat[dbIndex].displayMessage(finalString, col, 2, &windowVisible);

				// Add dynchannel info before text so that the chat log will show the correct string.
				CCDBNodeLeaf* node = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:SHOW_DYN_CHANNEL_NAME_IN_CHAT_CB", false);
				if (pIM->getLogState())
				{
					// Add dyn chan number before string
					ucstring prefix("[" + NLMISC::toString(dbIndex) + "]");
					// Find position to put the new string
					// After timestamp?
					size_t pos = finalString.find(ucstring("]"));
					size_t colonpos = finalString.find(ucstring(": @{"));
					// If no ] found or if found but after the colon (so part of the user chat)
					if (pos == ucstring::npos || (colonpos < pos))
					{
						// No timestamp, so put it right after the color and add a space
						pos = finalString.find(ucstring("}"));;
						prefix += " ";
					}
					finalString = finalString.substr(0, pos + 1) + prefix + finalString.substr(pos + 1);

					if (node && node->getValueBool())
					{
						uint32 textId = ChatMngr.getDynamicChannelNameFromDbIndex(dbIndex);
						ucstring title;
						STRING_MANAGER::CStringManagerClient::instance()->getDynString(textId, title);
						prefix = title.empty() ? ucstring("") : ucstring(" ") + title;
						pos = finalString.find(ucstring("] "));
						finalString = finalString.substr(0, pos) + prefix + finalString.substr(pos);
					}
				}
			}
			else
			{
				nlwarning("Dynamic chat %s not found for message: %s", dynChatId.toString().c_str(), finalString.toString().c_str());
			}
		}
		else
		{
			ucstring::size_type index = finalString.find(ucstring("<BPFX>"));
			if (index != ucstring::npos)
			{
				bubbleWanted = false;
				finalString = finalString.substr(index+6,finalString.size());
				ucstring::size_type index2 = finalString.find(ucstring(" "));
				ucstring playerName;
				if (index2 < (finalString.size()-3))
				{
					playerName = finalString.substr(0,index2);
					finalString = finalString.substr(index2+1,finalString.size());
				}
				if (!senderName.empty())
				{
					CEntityCL *senderEntity = EntitiesMngr.getEntityByName (CEntityCL::removeTitleAndShardFromName(senderName), true, true);
					if (senderEntity)
					{
						if (senderEntity->Type != CEntityCL::Player)
						{
							if (playerName.empty())
							{
								senderEntity->removeStateFx();
								senderEntity->setStateFx(finalString.toString());
								nlinfo("empty");
							}
							else
							{
								CEntityCL *destEntity = EntitiesMngr.getEntityByName (CEntityCL::removeTitleAndShardFromName(playerName), false, true);
								if (destEntity)
								{
									destEntity->removeStateFx();
									destEntity->setStateFx(finalString.toString());
									nlinfo("no empty");
								}
							}
						}
					}
				}
				finalString = "";
			}
			else
			{
				PeopleInterraction.ChatInput.AroundMe.displayMessage(finalString, col, 2, &windowVisible);
			}
		}
		// if tell, bkup sendername
		if (mode == CChatGroup::tell && windowVisible && !senderName.empty())
		{
			PeopleInterraction.LastSenderName = CEntityCL::removeTitleAndShardFromName(senderName);
		}
	}

	// received CHAT
	//nldebug("<impulseChat> Received CHAT : %s with category %s",finalString.toString().c_str(),stringCategory.c_str());


	// **** Process chat entry for the bubbles
	// todo hulud : registering a chat callback would be better than calling this hardcoded action handler
	ucstring finalRawMessage;
	// remove color qualifier from raw string
	getStringCategory(rawMessage, finalRawMessage);
	if (bubbleWanted)
	{
		InSceneBubbleManager.chatOpen(compressedSenderIndex, finalRawMessage, bubbleTimer);
	}

	// Log
	pIM->log (finalString);

}


// display a tell from network to interface
void CInterfaceChatDisplayer::displayTell(/*TDataSetIndex senderIndex, */const ucstring &ucstr, const ucstring &senderName)
{

	ucstring finalString = ucstr;

	// for now, '&' are removed by server so use another format	until a special msg is made
	if (strFindReplace(finalString, ucstring("<R2_INVITE>"), ucstring()))
	{
		CLuaManager::getInstance().executeLuaScript("RingAccessPoint:forceRefresh()");
	}


	CInterfaceProperty prop;
	prop.readRGBA("UI:SAVE:CHAT:COLORS:TELL"," ");
	bool windowVisible;

	ucstring goodSenderName = CEntityCL::removeTitleAndShardFromName(senderName);
 
	// The sender part is up to and including the first ":" after the goodSenderName
	ucstring::size_type pos = finalString.find(goodSenderName);
	pos = finalString.find(':', pos);
	pos = finalString.find(' ', pos);
	ucstring senderPart = finalString.substr(0, pos+1);
 	colorizeSender(finalString, senderPart, prop.getRGBA());

	PeopleInterraction.ChatInput.Tell.displayTellMessage(/*senderIndex, */finalString, goodSenderName, prop.getRGBA(), 2, &windowVisible);
	CInterfaceManager::getInstance()->log(finalString);

	// Open the free teller window
	CChatGroupWindow *pCGW = PeopleInterraction.getChatGroupWindow();
	if (pCGW != NULL)
		pCGW->setActiveFreeTeller(goodSenderName);

	if (windowVisible && !goodSenderName.empty())
		PeopleInterraction.LastSenderName = goodSenderName;
}

// clear a channel
void CInterfaceChatDisplayer::clearChannel(CChatGroup::TGroupType mode, uint32 dynChatDbIndex)
{
	if (mode == CChatGroup::guild)			PeopleInterraction.ChatInput.Guild.clearMessages();
	else if (mode == CChatGroup::team)		PeopleInterraction.ChatInput.Team.clearMessages();
	else if (mode == CChatGroup::region)	PeopleInterraction.ChatInput.Region.clearMessages();
	else if (mode == CChatGroup::arround)	PeopleInterraction.ChatInput.AroundMe.clearMessages();
	else if (mode == CChatGroup::universe)	PeopleInterraction.ChatInput.Universe.clearMessages();
	else if (mode == CChatGroup::dyn_chat)
	{
		// if correct dbIndex, clear
		if(dynChatDbIndex<CChatGroup::MaxDynChanPerPlayer)
			PeopleInterraction.ChatInput.DynamicChat[dynChatDbIndex].clearMessages();
		else
			nlwarning("Dynamic chat %d not found for clearing", dynChatDbIndex);
	}

	// don't support other for now (NB: actually used only for dyn_chat)
}


// ***************************************************************************
void impulseChat(NLMISC::CBitMemStream &impulse)
{
	ChatMngr.processChatString(impulse, InterfaceChatDisplayer);
}

void impulseChat2(NLMISC::CBitMemStream &impulse)
{
	ChatMngr.processChatString2(impulse, InterfaceChatDisplayer);
}

void impulseTell(NLMISC::CBitMemStream &impulse)
{
	ChatMngr.processTellString(impulse, InterfaceChatDisplayer);
}

void impulseFarTell(NLMISC::CBitMemStream &impulse)
{
	ChatMngr.processFarTellString(impulse, InterfaceChatDisplayer);
}

void impulseTell2(NLMISC::CBitMemStream &impulse)
{
	ChatMngr.processTellString2(impulse, InterfaceChatDisplayer);
}

void impulseDynString(NLMISC::CBitMemStream &impulse)
{
	ChatMngr.processChatStringWithNoSender(impulse, CChatGroup::system, InterfaceChatDisplayer);
}

void inpulseDynStringInChatGroup(NLMISC::CBitMemStream &impulse)
{
	CChatGroup::TGroupType type = CChatGroup::say;
	impulse.serialEnum(type);
	ChatMngr.processChatStringWithNoSender(impulse, type, InterfaceChatDisplayer);
}

// ***************************************************************************
//void impulseAddDynStr(NLMISC::CBitMemStream &impulse)
//{
//	bool huff = false;
//	impulse.serialBit(huff);
//
//	uint32 index;
//	ucstring ucstr;
//
//	impulse.serial( index );
//	impulse.serial( ucstr );
//
//	vector<bool> code;
//	if( huff )
//	{
//		impulse.serialCont( code );
//	}
//	if (PermanentlyBanned) return;
//	#ifdef OLD_STRING_SYSTEM
//		ChatMngr.getDynamicDB().add( index, ucstr, code );
//	#else
//		nlwarning( "// TRAP // WE MUST NEVER CALL THIS IMPULE ANYMORE : ALL IS HANDLED BY STRING_MANAGER NOW !!!" );
//	#endif
//
//	// received ADD_DYN_STR
//	nlinfo("impulseCallBack : Received ADD_DYN_STR : adding %s at index %d",ucstr.toString().c_str(),index);
//}
/*
string getInterfaceNameFromId (sint botType, sint interfaceId)
{
	string interfaceName = "ui:interface:bot_chat_";

	switch (botType)
	{
	case 0: interfaceName += "figurant_"; break;
	case 1: interfaceName += "figurant_presse_"; break;
	case 2: interfaceName += "chef_village_"; break;
	default: interfaceName += "figurant_"; break;
	}

	switch (interfaceId)
	{
	case BOTCHATTYPE::Intro: interfaceName += "intro"; break;
	case BOTCHATTYPE::FriendlyMainPage: interfaceName += "friendly_main"; break;
	case BOTCHATTYPE::NeutralMainPage: interfaceName += "neutral_main"; break;
	case BOTCHATTYPE::NastyMainPage: interfaceName += "nasty_main"; break;
	case BOTCHATTYPE::MoreNewsPage: interfaceName += "more_news"; break;
	case BOTCHATTYPE::Done: nlinfo ("end of bot chat"); interfaceName = ""; break;
	}
	return interfaceName;
}

static char *shortNews[] = {
	"The wind is sour and brings only bad tidings...", "Kitins have been sighted near the village!", "",
	"The tribe of the Black Circle has recently", "increased its activities in our region.", "",
	"The Black Circle has made an incursion", "into our territory!", "",
	"The Black Circle has been sighted near one", "of our forward posts, deep in dangerous territory.", "",
	"The tide has washed up evil news, friend.", "The Black Circle is active in our region.", "",
	"Our people suffer from a debilitating shortage.", "We are in sore need of KamiBast.", "",
	"The economy is slow and our reserve of", "Live Seed low.", "",
	"We are in sore need of Live Seed", "If there is a Goo epidemic, we shall all perish!", "",
	"Our master mages have gotten wind of", "the growing Kami discontentment", "",
};

static char *longNews[] = {
	"These powerful predators haven't come this near", "to the village since their devastating attack", "over 15 seasons ago!",
	"They are after more KamiBast", "for their occult practices.", "",
	"They have captured", "2 of our fortifications in the bush!", "",
	"They have taken over one of our richest sources", "of KamiBast, and are exploiting it", "for their own occult purposes.",
	"They now hold an important source", "of Live Seed hostage,", "close to one of our forward posts.",
	"We use the magical properties of KamiBast and", "its unusually rich fibers for all our crafts.", "",
	"If we don't harvest new Seed soon,", "we will have no way of purchasing goods", "and resources, beyond what we produce ourselves",
	"We use the rich Sap of Live Seed to produce", "an antidote that counters the disastrous", "effects of the Goo on all Atysian life forms.",
	"The Kamis are shaken by the Black Circle's", "presence. If the Circle continues it's occult", "practices, we will all suffer the Kamic anger.",
};
*/
/*
void setFakeNews ()
{
	char *table[] = { "figurant", "chef_village", "garde", "commercant" };

	sint rnd = rand ()%(sizeof(shortNews)/sizeof(shortNews[0])/3);
	rnd;

	for (uint i = 0; i < sizeof(table)/sizeof(table[0]); i++)
	{
		{ // set test for the friendly main
			string iname;
			iname = "ui:interface:bot_chat_";
			iname += table[i];
			iname += "_friendly_main";

			CInterfaceGroup *inter = CWidgetManager::getInstance()->getWindowFromId(iname);
			if (inter == NULL)
			{
				nlwarning ("cant find interface 's%'", iname.c_str());
				continue;
			}

			CViewText *inter2 = (CViewText *)inter->getView("title0");
			nlassert (inter2 != NULL);
			inter2->setText(ucstring(shortNews[rnd*3]));

			CViewText *inter3 = (CViewText *)inter->getView("title1");
			nlassert (inter3 != NULL);
			inter3->setText(ucstring(shortNews[rnd*3+1]));

			CViewText *inter4 = (CViewText *)inter->getView("title2");
			nlassert (inter4 != NULL);
			inter4->setText(ucstring(shortNews[rnd*3+2]));
		}
		{ // set test for the neutral main
			string iname;
			iname = "ui:interface:bot_chat_";
			iname += table[i];
			iname += "_neutral_main";

			CInterfaceGroup *inter = CWidgetManager::getInstance()->getWindowFromId(iname);
			if (inter == NULL)
			{
				nlwarning ("cant find interface 's%'", iname.c_str());
				continue;
			}

			CViewText *inter2 = (CViewText *)inter->getView("title0");
			nlassert (inter2 != NULL);
			inter2->setText(ucstring(shortNews[rnd*3]));

			CViewText *inter3 = (CViewText *)inter->getView("title1");
			nlassert (inter3 != NULL);
			inter3->setText(ucstring(shortNews[rnd*3+1]));
		}
		{ // set test for the more news
			string iname;
			iname = "ui:interface:bot_chat_";
			iname += table[i];
			iname += "_more_news";

			CInterfaceGroup *inter = CWidgetManager::getInstance()->getWindowFromId(iname);
			if (inter == NULL)
			{
				nlwarning ("cant find interface 's%'", iname.c_str());
				continue;
			}

			CViewText *inter2 = (CViewText *)inter->getView("title0");
			nlassert (inter2 != NULL);
			inter2->setText(ucstring(longNews[rnd*3]));

			CViewText *inter3 = (CViewText *)inter->getView("title1");
			nlassert (inter3 != NULL);
			inter3->setText(ucstring(longNews[rnd*3+1]));

			CViewText *inter4 = (CViewText *)inter->getView("title2");
			nlassert (inter4 != NULL);
			inter4->setText(ucstring(longNews[rnd*3+2]));
		}
	}
}
*/





//=========================================
/** Temp setup for choice list
  */
/*
static void setupBotChatChoiceList(CInterfaceGroup *botChatGroup)
{
	// Temp for test. Should then be read from server msg
	std::vector<ucstring> choices;
	for(uint k = 0; k < 90; ++k)
	{
		choices.push_back("Choice " + toString(k));
	}
	CBotChat::setChoiceList(botChatGroup, choices, false);
}
*/

//=========================================
/** Temp setup for description list
  */
/*
static void setupBotChatDescription(CInterfaceGroup *botChatGroup)
{
	ucstring desc;
	for(uint k = 0; k < 90; ++k)
	{
		desc += "This is a multi line description. ";
	}
	CBotChat::setDescription(botChatGroup, desc);
}
*/

//=========================================
/** Temp setup for bot chat gift
  */
/*
static void setupBotChatBotGift(CInterfaceGroup *botChatGroup)
{
	// create dummy item in the db
	CInterfaceManager *im = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INVENTORY:20:0:SHEET")->setValue32(CSheetId("ai_flesh_poisson.item").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INVENTORY:20:0:QUALITY")->setValue32(0);
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INVENTORY:20:1:SHEET")->setValue32(CSheetId("fyros_sword_lvl_01_05.item").asInt());
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:INVENTORY:20:1:QUALITY")->setValue32(2);
	CBotChat::setBotGift(botChatGroup, ucstring("Thanks to have succeeded the mission"), ucstring("Here's your reward"), ucstring("The bot has taken the object quest from your inventory"));
}
*/

//-----------------------------------------------
// impulseBotChatSetInterface :
//-----------------------------------------------
#if 0
void impulseBotChatSetInterface(NLMISC::CBitMemStream &impulse)
{
	// received ADD_DYN_STR

	CEntityId user;
	uint32 happyness;
	BOTCHATTYPE::TBotChatInterfaceId interfaceId;
	bool hasNews;

	impulse.serial (user);
	impulse.serial (happyness);

//	impulse.serialEnum (interfaceId);
	uint16	interfId;
	impulse.serial(interfId);
	interfaceId = (BOTCHATTYPE::TBotChatInterfaceId)(interfId&0xff);
	uint8 botType = (interfId>>8) & 0xff;

	impulse.serial (hasNews);

	nldebug("impulseCallBack : Received BOT_CHAT:SET_INTERFACE interface %d, have news %s, happy %d, bottype %hu", interfaceId, hasNews?"yes":"no", happyness,(uint16)botType);

	string stringId;
	vector<uint64> args;
	if (hasNews)
	{

/*		impulse.serial (stringId);
		impulse.serialCont (args);
		nlinfo ("receive the news '%s' with %d args", stringId.c_str(), args.size());
*/
		// TEMP FOR THE DEMO, DON'T USE THE NETWORK NEW BUT SELECT A NEWS HERE
/*
		CInterfaceGroup *inter = CWidgetManager::getInstance()->getWindowFromId("ui:interface:bot_chat_intro");
		nlassert (inter != NULL);
		inter->setActive(true);

		CViewText *inter2 = (CViewText *)inter->getView("hi");
		nlassert (inter2 != NULL);
		inter2->NetworkTextId.setString("IOS_NEWS_FOOTBALL_SHORT_EEII", &ChatMngr);
		inter2->NetworkTextId.Args.push_back(10);
		inter2->NetworkTextId.Args.push_back(20);
		inter2->NetworkTextId.Args.push_back(1);
		inter2->NetworkTextId.Args.push_back(2);
*/	}

	// FOR THE DEMO, find and set a fake news:
//	setFakeNews ();

	string interfaceName = getInterfaceNameFromId (botType, interfaceId);

	if(interfaceName.empty())
	{
		nlwarning ("Received an unknown bot chat interface %d", interfaceId);
	}
	else
	{
		CInterfaceGroup *inter = CWidgetManager::getInstance()->getWindowFromId(interfaceName);
		if (inter == NULL)
		{
			nlwarning ("Can't find interface name '%s' %d", interfaceName.c_str(), interfaceId);
		}
		else
		{
			CInterfaceManager::getInstance()->setBotChatWin(inter);
			if (inter->getActive())
			{
				nlwarning ("Interface %s is already active, not normal!", interfaceName.c_str());
			}
			else
			{
				nlinfo ("server want to me display the bot chat interface %s %d", interfaceName.c_str(), interfaceId);
				inter->setActive(true);
			}
		}
	}
}
#endif



//-----------------------------------------------
// impulseBeginTrade :
//-----------------------------------------------
void impulseBeginTrade(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	//open trade window
	CInterfaceGroup* win = CWidgetManager::getInstance()->getWindowFromId("ui:interface:trade");
	if (!win)
	{
		nlwarning("invalid interface ui:interface:trade");
		return;
	}
	win->setActive(true);
}

//-----------------------------------------------
// impulseBuyPrice :
//-----------------------------------------------
void impulseBuyPrice(NLMISC::CBitMemStream &impulse)
{
	uint16 botChatSession;
	uint32 sheetID;
	uint16 quality;
	uint64 price;
	impulse.serial(botChatSession);
	impulse.serial(price);
	impulse.serial(sheetID);
	impulse.serial(quality);
	// no more used
}

//-----------------------------------------------
// impulseDynChatOpen
//-----------------------------------------------
void impulseDynChatOpen(NLMISC::CBitMemStream &impulse)
{
	uint32 BotUID;			// Compressed Index
	uint32 BotName;			// Server string
	vector<uint32> DynStrs; // 0 - Desc, 1 - Option0, 2 - Option1, etc....
	impulse.serial(BotUID);
	impulse.serial(BotName);
	impulse.serialCont(DynStrs);

	if (PermanentlyBanned) return;

/*	string sTmp = "impulseCallback : Received BOTCHAT:DYNCHAT_OPEN BotUID:";
	sTmp += toString(BotUID) + " BotName:";
	sTmp += toString(BotName) + " DynStrs:";
	for (uint32 i = 0; i < DynStrs.size(); ++i)
	{
		sTmp += toString(DynStrs[i]);
		if (i != DynStrs.size()-1) sTmp += ",";
	}
	nlinfo(sTmp.c_str());*/

	InSceneBubbleManager.dynChatOpen(BotUID, BotName, DynStrs);
}

//-----------------------------------------------
// impulseDynChatClose
//-----------------------------------------------
void impulseDynChatClose(NLMISC::CBitMemStream &impulse)
{
	uint32 BotUID;			// Compressed Index
	impulse.serial(BotUID);
	if (PermanentlyBanned) return;
	//nlinfo("impulseCallback : Received BOTCHAT:DYNCHAT_CLOSE BotUID:"+toString(BotUID));
	InSceneBubbleManager.dynChatClose(BotUID);
}

//-----------------------------------------------
// impulseBeginCast:
//-----------------------------------------------
void impulseBeginCast(NLMISC::CBitMemStream &impulse)
{
	//open cast window
	uint32 begin,end;
	impulse.serial(begin);
	impulse.serial(end);
	if (PermanentlyBanned) return;
	CInterfaceManager* iMngr = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SPELL_CAST")->setValue32(1);
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CAST_BEGIN")->setValue32(begin);
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CAST_END")->setValue32(end);
}



//-----------------------------------------------
// impulseCorrectPos :
// Message from the server to correct the user position because he is not at the same position on the server..
//-----------------------------------------------
void impulseCorrectPos(NLMISC::CBitMemStream &impulse)
{
	// TP:CORRECT
	//nlinfo("impulseCallback : Received TP:CORRECT");
	sint32 x, y, z;
	impulse.serial(x);
	impulse.serial(y);
	impulse.serial(z);
	nlinfo("impulseCorrectPos: new user position %d %d %d", x, y, z);

	if(UserEntity->mode() != MBEHAV::COMBAT_FLOAT)
	{
		// Compute the destination.
		CVectorD dest = CVectorD((float)x/1000.0f, (float)y/1000.0f, (float)z/1000.0f);
		// Update the position for the vision.
		NetMngr.setReferencePosition(dest);
		// Change the user poisition.
		UserEntity->correctPos(dest);
	}
}// impulseCorrectPos //

class CDummyProgress : public IProgressCallback
{
	void progress (float /* value */) {}
};

//-----------------------------------------------
// impulseTP :
// Message from the server to teleport the user.
// \warning This function remove the current target. Do no use to correct a position.
//-----------------------------------------------
void impulseTPCommon(NLMISC::CBitMemStream &impulse, bool hasSeason);
void impulseTPCommon2(NLMISC::CBitMemStream &impulse, bool hasSeason);

void impulseTP(NLMISC::CBitMemStream &impulse)
{
	impulseTPCommon(impulse, false);
}

void impulseTPWithSeason(NLMISC::CBitMemStream &impulse)
{
	impulseTPCommon(impulse, true);
}

struct SQueuedTP
{
	NLMISC::CBitMemStream Impulse;
	bool HasSeason;

	SQueuedTP(const NLMISC::CBitMemStream &impulse, bool hasSeason)
		:Impulse(impulse), HasSeason(hasSeason)
	{
	}
};

// note - this method added by Sadge and Hamster to deal with unexplained recursive calls to impulseTPCommon
// these calls are provoked by the net manager update which is called during loading
void impulseTPCommon(NLMISC::CBitMemStream &impulse, bool hasSeason)
{
	CNiceInputAuto niceInputs;
	static std::list<SQueuedTP> queuedTPs;
	SQueuedTP thisTP(impulse,hasSeason);
	queuedTPs.push_back(thisTP);

	BOMB_IF(queuedTPs.size()!=1,NLMISC::toString("Queueing recursive TPs depth=%u",queuedTPs.size()),return);

	while(!queuedTPs.empty())
	{
		impulseTPCommon2(queuedTPs.front().Impulse,queuedTPs.front().HasSeason);
		queuedTPs.pop_front();
	};

}


void impulseTPCommon2(NLMISC::CBitMemStream &impulse, bool hasSeason)
{
	// choose a default screen if not setuped
	if( LoadingBackground!=ResurectKamiBackground && LoadingBackground!=ResurectKaravanBackground &&
		LoadingBackground!=TeleportKamiBackground && LoadingBackground!=TeleportKaravanBackground)
		LoadingBackground= TeleportKaravanBackground;
	// if resurect but user not dead, choose default. NB: this is a bug, the tp impulse should tell
	// which background to choose. \todo yoyo: this is a temp fix
	if( UserEntity && !UserEntity->isDead() &&
		(LoadingBackground==ResurectKamiBackground || LoadingBackground==ResurectKaravanBackground) )
		LoadingBackground= TeleportKaravanBackground;

	// Play music according to the background
	if(SoundMngr)
	{
		LoadingMusic.clear();
		if(LoadingBackground==TeleportKamiBackground)
			LoadingMusic= "Kami Teleport.ogg";
		else if(LoadingBackground==TeleportKaravanBackground)
			LoadingMusic= "Karavan Teleport.ogg";
		// if resurection, continue to play death music
		else if(LoadingBackground==ResurectKamiBackground || LoadingBackground==ResurectKaravanBackground)
		{
			// noop
		}
		// default: loading music
		else
		{
			LoadingMusic= "Loading Music Loop.ogg";
		}

		// start to play
		SoundMngr->playEventMusic(LoadingMusic, CSoundManager::LoadingMusicXFade, true);
	}

	// Create the loading texture.
	beginLoading (LoadingBackground);

	// No ESCAPE key
	UseEscapeDuringLoading = false;

	// Change the tips
	selectTipsOfTheDay (rand());

	// start progress bar and display background
	ProgressBar.reset (BAR_STEP_TP);
	ucstring nmsg("Loading...");
	ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );


	// received ADD_DYN_STR
	nlinfo("impulseTP: received a request for a TP.");
	sint32 x, y, z;
	impulse.serial(x);
	impulse.serial(y);
	impulse.serial(z);
	bool useHeading;
	impulse.serialBit( useHeading );
	// Is there an orientation too ?
	if( useHeading )
	{
		float angle;
		impulse.serial(angle);
		nlinfo("impulseTP: to %d %d %d %f", x, y, z, angle);
		CVector ori = CVector((float)cos(angle), (float)sin(angle), 0.0f);
		ori.normalize();
		UserEntity->dir(ori, false, false);
		UserEntity->front(ori, false, false);
		UserEntity->setHeadPitch(0);
		UserControls.resetCameraDeltaYaw();
	}
	else
		nlinfo("impulseTP: to %d %d %d", x, y, z);

	if (hasSeason)
	{
		extern uint8 ServerSeasonValue;
		extern bool ServerSeasonReceived;
		impulse.serial(ServerSeasonValue);
		ServerSeasonReceived = true;
	}

	if (ClientCfg.R2EDEnabled)
	{
		R2::getEditor().tpReceived();
	}

	// Compute the destination.
	CVectorD dest = CVectorD((float)x/1000.0f, (float)y/1000.0f, (float)z/1000.0f);
	// Update the position for the vision.
	NetMngr.setReferencePosition(dest);
	// Change the position of the entity and in Pacs.
	UserEntity->pos(dest);

	// Fade out the Game Sound
	if(SoundMngr)
		SoundMngr->fadeOutGameSound(ClientCfg.SoundTPFade);


	R2::TTeleportContext tpContext = R2::TPContext_Unknown;

	ucstring tpReason;
	ucstring tpCancelText;

	try
	{
		R2::TR2TpInfos tpInfos;
		impulse.serial(tpInfos);


		if ( tpInfos.UseTpMessage)
		{
			tpReason = CI18N::get(tpInfos.TpReasonId);

			uint32 size = (uint32)tpInfos.TpReasonParams.size();
			uint32 first = 0;
			CSString  str(tpReason.toString());
			for (;first != size ; ++first)
			{
				std::string value = tpInfos.TpReasonParams[first];
				std::string key = NLMISC::toString("%%%u", first +1);
				str = str.replace( key.c_str(), value.c_str());
			}
			tpReason = ucstring(str);
			tpCancelText = CI18N::get(tpInfos.TpCancelTextId);
			tpContext = tpInfos.TpContext;
		}

	}
	catch (const EStream &)
	{
		tpReason = ucstring("TP Reason");
		tpCancelText = ucstring("Cancel TP"); // for test
		// try to deduce tp context from current editor mode
		switch (R2::getEditor().getMode())
		{
			case R2::CEditor::EditionMode:
			case R2::CEditor::NotInitialized:
				tpContext = R2::TPContext_Unknown;
				tpReason = ucstring();
				tpCancelText = ucstring();
			break;
			case R2::CEditor::GoingToDMMode:
			case R2::CEditor::TestMode:
			case R2::CEditor::DMMode:
				tpContext = R2::TPContext_Edit;
			break;
			case R2::CEditor::AnimationModeLoading:
			case R2::CEditor::AnimationModeWaitingForLoading:
			case R2::CEditor::AnimationModeDm:
			case R2::CEditor::AnimationModeGoingToDm:
				tpContext = R2::TPContext_IslandOwner;
			break;
			case R2::CEditor::AnimationModePlay:
			case R2::CEditor::AnimationModeGoingToPlay:
			default:
				tpContext = R2::TPContext_Mainland;
			break;
		}
	}



	if (!tpReason.empty())
	{
		std::string tpIcon;
		switch(tpContext)
		{
			case R2::TPContext_Mainland:	 tpIcon = "cancel_tp_main_land.tga";		break;
			case R2::TPContext_Edit:		 tpIcon = "cancel_tp_edit.tga";			break;
			case R2::TPContext_IslandOwner:  tpIcon = "cancel_tp_island_owner.tga"; break;
			default: break;
		}
		ProgressBar.setTPMessages(tpReason, tpCancelText, tpIcon);
	}

	ProgressBar.progress(0);
	// enable hardware mouse to allow to click the buttons
	//bool oldHardwareCursor = IsMouseCursorHardware();
	//InitMouseWithCursor(true);
	// Select the closest continent from the new position.
	ContinentMngr.select(dest, ProgressBar);
	//
	//InitMouseWithCursor(oldHardwareCursor);

	// reset 'cancel' button
	ProgressBar.setTPMessages(ucstring(), ucstring(), "");


	// ProgressBar.enableQuitButton(false); // TMP TMP
	ProgressBar.progress(1.f); // do a last display without the buttons because first frame may take a while to draw, and the buttons have no more effect at this point.
	ProgressBar.finish();
	// ProgressBar.enableQuitButton(true); // TMP TMP

	// Teleport the User.
	UserEntity->tp(dest);

	// Msg Received, send an acknowledge after the landscape has been loaded.
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("TP:ACK", out))
	{
		NetMngr.push(out);
		nlinfo("impulseTP: teleport acknowledge 'TP:ACK' sent.");
	}
	else
		nlwarning("impulseTP: unknown message name : 'TP:ACK'.");


	// First frame
	FirstFrame = true;


	// if tp canceling was asked, act accordingly
	if (ProgressBar.getTPCancelFlag(true))
	{
		switch(tpContext)
		{
			case R2::TPContext_Mainland:
				CAHManager::getInstance()->runActionHandler("return_to_mainland", NULL);
			break;
			case R2::TPContext_Edit:
				CAHManager::getInstance()->runActionHandler("r2ed_stop_test", NULL);
			break;
			case R2::TPContext_IslandOwner:
				CAHManager::getInstance()->runActionHandler("r2_stop_live", NULL);
			break;
			default:
			break;
		}
	}

	initHardwareCursor(true);
}// impulseTP //

//-----------------------------------------------
// impulseCombatEngageFailed :
//-----------------------------------------------
void impulseCombatEngageFailed(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	nlinfo("impulseCombatEngageFailed: Combat Engage Failed.");

	// Unlock the motion.
	UserControls.locked(false);
}// impulseCombatEngageFailed //

//-----------------------------------------------
// impulseTeamInvitation :
//-----------------------------------------------
void impulseTeamInvitation(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseTeamInvitation: received an invitation");

	uint32 textID;
	impulse.serial(textID);
	if (PermanentlyBanned) return;

	CLuaManager::getInstance().executeLuaScript("game:onTeamInvation("+toString(textID)+")", 0);
}// impulseTeamInvitation //

//-----------------------------------------------
// impulseTeamShareOpen
// The server request that the client opens the team sharing system
//-----------------------------------------------
void impulseTeamShareOpen(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer*>( CWidgetManager::getInstance()->getElementFromId("ui:interface:team_share"));
	if (!gc) return;
	gc->setActive(true);
	CWidgetManager::getInstance()->setTopWindow(gc);
	gc->updateCoords();
	gc->center();
}// impulseTeamShareOpen //

//-----------------------------------------------
// impulseTeamShareInvalid
// invalidate the player validation. If someone has choosen an item/phrase after the player has validated
// the player receive this message to let him know that the chance percentage to obtain a specific item has
// changed and so the player can update its own settings to fit better to what he wants.
// On the client side we have just to show the valid button. All the resets are done on the server side.
//-----------------------------------------------
void impulseTeamShareInvalid(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCtrlTextButton *pTB = dynamic_cast<CCtrlTextButton*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:team_share:content:ok"));
	if (pTB != NULL)
		pTB->setActive(true);
}// impulseTeamShareInvalid //

//-----------------------------------------------
// impulseTeamShareClose
// The server wants to close the team sharing interface (if the sharing has been validated or other reasons)
//-----------------------------------------------
void impulseTeamShareClose(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer
	*pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:team_share"));
	if (pGC != NULL)
		pGC->setActive(false);
	CCtrlTextButton *pTB = dynamic_cast<CCtrlTextButton*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:team_share:content:ok"));
	if (pTB != NULL)
		pTB->setActive(true);
}// impulseTeamShareClose //

//-----------------------------------------------
// impulseTeamContactInit
// initialize friend list and ignore list from the contact list
//-----------------------------------------------
void impulseTeamContactInit(NLMISC::CBitMemStream &impulse)
{
	vector<uint32> vFriendListName;
	vector<TCharConnectionState> vFriendListOnline;
	vector<ucstring> vIgnoreListName;

	impulse.serialCont(vFriendListName);
	uint32 nbState;
	impulse.serial(nbState);
	vFriendListOnline.resize(nbState);
	for (uint i=0; i<nbState; ++i)
	{
		impulse.serialShortEnum(vFriendListOnline[i]);
	}
//	impulse.serialCont(vFriendListOnline);
	impulse.serialCont(vIgnoreListName);

	if (PermanentlyBanned) return;

	//nlinfo("impulseCallback : Received TEAM:CONTACT_INIT nbfriend:%d nbignore:%d", vFriendListName.size(), vIgnoreListName.size());

	PeopleInterraction.initContactLists(vFriendListName, vFriendListOnline, vIgnoreListName);
}// impulseTeamContactInit //

//-----------------------------------------------
// impulseTeamContactCreate
// create one character from the friend or ignore list
//-----------------------------------------------
void impulseTeamContactCreate(NLMISC::CBitMemStream &impulse)
{
	uint32	contactId;
	uint32	nameId;
	TCharConnectionState	online = ccs_offline;
	uint8	nList;

	impulse.serial(contactId);
	impulse.serial(nameId);
	impulse.serialShortEnum(online);
	impulse.serial(nList);

	// client patch to resolve bad server response when requesting ignore list contact creation
	if (nList == 1)		// ignore list
	{
		// prevent adding an empty player to ignore list
		if (nameId == 0) return;
	}

	if (PermanentlyBanned) return;

	//nlinfo("impulseCallback : Received TEAM:CONTACT_CREATE %d %d %s %d", contactId, nameId, online?"true":"false", nList);

	PeopleInterraction.addContactInList(contactId, nameId, online, nList);

}// impulseTeamContactStatus //

//-----------------------------------------------
// impulseTeamContactStatus
// update one of the character from the friend list
//-----------------------------------------------
void impulseTeamContactStatus(NLMISC::CBitMemStream &impulse)
{
	uint32	contactId;
	TCharConnectionState	online = ccs_offline;

	impulse.serial(contactId);
	impulse.serialShortEnum(online);

	if (PermanentlyBanned) return;

	//nlinfo("impulseCallback : Received TEAM:CONTACT_STATUS %d %s", contactId, online == ccs_online ?"online": online==ccs_offline?"offline" : "foreign_online");

	// 0<=FriendList (actually ignore list does not show online state)
	PeopleInterraction.updateContactInList(contactId, online, 0);

	// Resort the contact list if needed
	CInterfaceManager* pIM= CInterfaceManager::getInstance();
	CPeopleList::TSortOrder order = (CPeopleList::TSortOrder)(NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CONTACT_LIST:SORT_ORDER")->getValue32());

	if (order == CPeopleList::sort_online)
	{
		PeopleInterraction.FriendList.sortEx(order);
	}
}// impulseTeamContactStatus //


//-----------------------------------------------
// impulseTeamContactRemove
// Remove a contact by the server
//-----------------------------------------------
void impulseTeamContactRemove(NLMISC::CBitMemStream &impulse)
{
	uint32	contactId;
	uint8	nList;

	impulse.serial(contactId);
	impulse.serial(nList);

	if (PermanentlyBanned) return;

	//nlinfo("impulseCallback : Received TEAM:CONTACT_REMOVE %d %d", contactId, nList);

	PeopleInterraction.removeContactFromList(contactId, nList);

}// impulseTeamContactRemove //


//-----------------------------------------------
// servers sets information of a guild member:
//	u16 ( member index ) u32 (player name ), u8 ( player grade + last bit set if player online ).
//-----------------------------------------------
/*void impulseGuildSetMemberInfo(NLMISC::CBitMemStream &impulse)
{
	uint16 index;
	impulse.serial(index);
	uint32 guildMemberName;
	impulse.serial(guildMemberName);
	uint8 grade;
	impulse.serial(grade);
	bool online = ((grade&0x80) != 0);
	grade = (grade & 0x7F);
	CGuildManager::getInstance()->set(index, guildMemberName, grade, online);
}*/

//-----------------------------------------------
// vector of  pair( u32 (player name ), u8 ( player grade + last bit set if player online ) )
//-----------------------------------------------
/*void impulseGuildInitMemberInfo(NLMISC::CBitMemStream &impulse)
{
	vector < pair < uint32, uint8 > > AllMembers;
	uint16 nbEntries;
	impulse.serial(nbEntries);
	AllMembers.resize(nbEntries);
	for (uint32 i = 0; i < nbEntries; ++i)
	{
		uint32 name;
		impulse.serial(name);
		uint8 gradeNonline;
		impulse.serial(gradeNonline);
		AllMembers[i].first = name;
		AllMembers[i].second = gradeNonline;
	}

	CGuildManager::getInstance()->init(AllMembers);
}*/


//-----------------------------------------------
// impulseGuildInvitation
//-----------------------------------------------
/*void impulseGuildInvitation(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseGuildInvitation");

}*/

//-----------------------------------------------
// impulseGuildJoinProposal
// server sent to client invitation (uint32 invitorNameId, uint32 guildNameId
//-----------------------------------------------
void impulseGuildJoinProposal(NLMISC::CBitMemStream &impulse)
{

	uint32 phraseID;
	impulse.serial(phraseID);

	if (PermanentlyBanned) return;

	//nlinfo("impulseCallback : Received GUILD:JOIN_PROPOSAL %d", phraseID);

	CGuildManager::getInstance()->launchJoinProposal(phraseID);
	/*//activate the pop up window
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>( CWidgetManager::getInstance()->getElementFromId("ui:interface:join_guild_proposal"));
	if (!gc) return;
	CViewText *vt = dynamic_cast<CViewText*>(gc->getView("invitor_name"));
	if (vt == NULL) return;
	vt->setText(invitor);
	gc->setActive(true);
	CWidgetManager::getInstance()->setTopWindow(gc);
	gc->updateCoords();
	gc->center();
	gc->enableBlink(2);*/
}// impulseGuildJoinProposal //


//-----------------------------------------------
// impulseCloseTempInv
//-----------------------------------------------
void impulseCloseTempInv(NLMISC::CBitMemStream &impulse)
{
	CTempInvManager::getInstance()->close();
}

//-----------------------------------------------
// impulseAscencorTeleport
//-----------------------------------------------
void impulseAscencorTeleport(NLMISC::CBitMemStream &impulse)
{

} // impulseAscencorTeleport //

//-----------------------------------------------
// impulseEnterCrZoneProposal
// server sent to client invitation (uint32 invitorNameId, uint32 guildNameId
//-----------------------------------------------
void impulseEnterCrZoneProposal(NLMISC::CBitMemStream &impulse)
{
	uint32 phraseID;
	impulse.serial(phraseID);
	if (PermanentlyBanned) return;

	//nlinfo("impulseCallback : Received MISSION:ASK_ENTER_CRITICAL %d", phraseID);

	//activate the pop up window
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>( CWidgetManager::getInstance()->getElementFromId("ui:interface:enter_crzone_proposal"));
	if (!gc) return;
	CViewTextID *vti = dynamic_cast<CViewTextID *>(gc->getView("phrase"));
	if (!vti) return;
	vti->setTextId(phraseID);
	gc->setActive(true);
	CWidgetManager::getInstance()->setTopWindow(gc);
	gc->updateCoords();
	gc->center();
	gc->enableBlink(2);
}// impulseEnterCrZoneProposal //

//-----------------------------------------------
// impulseCloseEnterCrZoneProposal
// server close proposal interface
//-----------------------------------------------
void impulseCloseEnterCrZoneProposal(NLMISC::CBitMemStream &impulse)
{
	// hide interface
	CInterfaceManager* pIM = CInterfaceManager::getInstance();
	CInterfaceGroup *pIG = (CInterfaceGroup*)CWidgetManager::getInstance()->getElementFromId ("ui:interface:enter_crzone_proposal");
	if(pIG)
		pIG->setActive(false);
}// impulseCloseEnterCrZoneProposal //


//-----------------------------------------------
// impulseExchangeInvitation :
//-----------------------------------------------
void impulseExchangeInvitation(NLMISC::CBitMemStream &impulse)
{
	uint32 textID;
	impulse.serial(textID);
	if (PermanentlyBanned) return;
	CInterfaceManager* iMngr = CInterfaceManager::getInstance();

	// show the modal window that allow the player to accept / decline the invitation
	CGroupContainer *wnd = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(PLAYER_EXCHANGE_INVITATION_DIALOG));
	if (wnd)
	{
		wnd->setActive(true);
		wnd->updateCoords();
		wnd->center();
		wnd->enableBlink(2);
		CWidgetManager::getInstance()->setTopWindow(wnd);
	}

	CViewTextID *vti = dynamic_cast<CViewTextID *>(wnd->getView("invite_phrase"));
	if (vti)
	{
		vti->setTextId(textID);
	}

}// impulseExchangeInvitation //

//-----------------------------------------------
// impulseExchangeCloseInvitation :
//-----------------------------------------------
void impulseExchangeCloseInvitation(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	CInterfaceManager* iMngr = CInterfaceManager::getInstance();
	// hide the modal window that allow the player to accept / decline the invitation
	CInterfaceGroup *wnd = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId(PLAYER_EXCHANGE_INVITATION_DIALOG));
	if (wnd) wnd->setActive(false);
}

//-----------------------------------------------
// impulseMountAbort :
//-----------------------------------------------
void impulseMountAbort(NLMISC::CBitMemStream &impulse)
{
	nlwarning("impulseMountAbort: Received ANIMALS:MOUNT_ABORT => no more used");
}// impulseMountAbort //

//-----------------------------------------------
// impulseRyzomTime :
// Synchronize the ryzom time with the server.
//-----------------------------------------------
/*
void impulseRyzomTime(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseRyzomTime: Ryzom Time Received");
	uint32 serverTick;
	float ryzomTime;
	uint32 ryzomDay;
	impulse.serial(serverTick);
	impulse.serial(ryzomTime);
	impulse.serial(ryzomDay);
	nlinfo("impulseRyzomTime: Day '%d' Time '%f'.", ryzomDay, ryzomTime);

	// Initialize
	RT.setOrigin( serverTick, ryzomDay, ryzomTime );
}// impulseRyzomTime //
*/
//-----------------------------------------------
// impulseWhere :
// Display server position
//-----------------------------------------------
void impulseWhere(NLMISC::CBitMemStream &impulse)
{
	//nlinfo("impulseCallback : Received DEBUG:REPLY_WHERE");

	sint32 x,y,z;
	impulse.serial(x);
	impulse.serial(y);
	impulse.serial(z);
	if (PermanentlyBanned) return;
	char buf[128];

	double xf = ((double)x)/1000.0f;
	double yf = ((double)y)/1000.0f;
	double zf = ((double)z)/1000.0f;

	sprintf(buf,"Your server position is : X= %g   Y= %g   Z= %g",xf,yf,zf);
	nlinfo(buf);
	CInterfaceManager::getInstance()->displaySystemInfo(ucstring(buf));
}// impulseWhere //

//-----------------------------------------------
// impulseWho :
// Display server position
//-----------------------------------------------
/*
void impulseWho(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseWho Received");
	CInterfaceManager::getInstance()->displaySystemInfo(ucstring("Players currently in the game :"));

	ucstring name;
	uint32 loginId;
	uint16 dist;
	uint8 dirshort;
	string str;
	while( impulse.getPos() < (sint32)impulse.length() )
	{
		impulse.serial(name);
		impulse.serial(loginId);
		impulse.serial(dist);
		impulse.serial(dirshort);

		double angle = dirshort * 2.0 * NLMISC::Pi / 255.0;
		angle -= NLMISC::Pi;
		nlinfo ("name %s uid %u dist %hu dirshort %hu angle %f", name.toString().c_str(),loginId, dist, (uint16)dirshort, angle);
		sint direction =(sint) floor( 0.5 + ( 8.0 * (angle + NLMISC::Pi)/(NLMISC::Pi) ) );
		direction = ((direction%16)+16)%16;
		static const string txts[]=
		{
			"uiW",
			"uiWSW",
			"uiSW",
			"uiSSW",
			"uiS",
			"uiSSE",
			"uiSE",
			"uiESE",
			"uiE",
			"uiENE",
			"uiNE",
			"uiNNE",
			"uiN",
			"uiNNW",
			"uiNW",
			"uiWNW",
		};

		str = toString (" - uid %d - distance %hu meters - direction ", loginId, dist);
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring(name + ucstring(str) + CI18N::get(txts[direction])));
	}
}// impulseWho //
*/

/*
void impulseWhoGM(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulseWhoGM Received");
	CInterfaceManager::getInstance()->displaySystemInfo(ucstring("Players currently in the game :"));

	ucstring name;
	uint32 loginId;
	uint16 dist;
	uint8 dirshort;
	string str;
	while( impulse.getPos() < (sint32)impulse.length() )
	{
		impulse.serial(name);
		impulse.serial(loginId);
		impulse.serial(dist);
		impulse.serial(dirshort);

		double angle = dirshort * 2.0 * NLMISC::Pi / 255.0;
		angle -= NLMISC::Pi;
		nlinfo ("name %s uid %u dist %hu dirshort %hu angle %f", name.toString().c_str(),loginId, dist, (uint16)dirshort, angle);
		sint direction =(sint) floor( 0.5 + ( 8.0 * (angle + NLMISC::Pi)/(NLMISC::Pi) ) );
		direction = ((direction%16)+16)%16;
		static const string txts[]=
		{
			"uiW",
				"uiWSW",
				"uiSW",
				"uiSSW",
				"uiS",
				"uiSSE",
				"uiSE",
				"uiESE",
				"uiE",
				"uiENE",
				"uiNE",
				"uiNNE",
				"uiN",
				"uiNNW",
				"uiNW",
				"uiWNW",
		};

		str = toString (" - uid %d - distance %hu meters - direction ", loginId, dist);
		CInterfaceManager::getInstance()->displaySystemInfo(ucstring(name + ucstring(str) + CI18N::get(txts[direction])));
	}
}// impulseWho //
*/
//-----------------------------------------------
// impulseCounter :
// check UDP validity
//-----------------------------------------------
void impulseCounter(NLMISC::CBitMemStream &impulse)
{
	//nlinfo("impulseCallBack : Received DEBUG:COUNTER");
	try
	{
		uint32	counter;
		impulse.serial(counter);

		static uint			queueTop = 0;
		static deque<bool>	queue;

		if (counter > queueTop)
		{
			queue.resize(queue.size()+counter-queueTop, false);
			queueTop = counter;
		}

		if (queueTop-counter+1 > queue.size())
		{
			nlinfo("COUNTER: counter %d arrived too late...", counter);
		}
		else
		{
			if (queue[queue.size()-1-(queueTop-counter)])
			{
				nlwarning("COUNTER: Received counter %d more than once !", counter);
			}
			else
			{
				nldebug("COUNTER: set counter %d", counter);
				queue[queue.size()-1-(queueTop-counter)] = true;
			}

			while (queue.size() > 128)
			{
				if (!queue.front())
				{
					nlwarning("COUNTER: counter %d not received !", queueTop-queue.size()-1);
				}

				queue.pop_front();
			}
		}
	}
	catch (const Exception &e)
	{
		nlwarning ("Problem while decoding a COUTNER msg, skipped: %s", e.what());
	}
}

//-----------------------------------------------
// impulsePhraseSend :
// A dyn string (or phrase) is send (so, we receive it)
//-----------------------------------------------
void impulsePhraseSend(NLMISC::CBitMemStream &impulse)
{
	STRING_MANAGER::CStringManagerClient::instance()->receiveDynString(impulse);
}

//-----------------------------------------------
// impulseStringResp :
// Update the local string set
//-----------------------------------------------
void impulseStringResp(NLMISC::CBitMemStream &impulse)
{
	uint32 stringId;
	string	strUtf8;
	impulse.serial(stringId);
	impulse.serial(strUtf8);
	ucstring str;
	str.fromUtf8(strUtf8);

	if (PermanentlyBanned) return;

	STRING_MANAGER::CStringManagerClient::instance()->receiveString(stringId, str);
}

//-----------------------------------------------
// impulseReloadCache :
// reload the string cache
//-----------------------------------------------
void impulseReloadCache(NLMISC::CBitMemStream &impulse)
{
	uint32 timestamp;;
	impulse.serial(timestamp);
	if (PermanentlyBanned) return;
	STRING_MANAGER::CStringManagerClient::instance()->loadCache(timestamp);
}

//-----------------------------------------------
// impulseBotChatEnd
// ForceThe end of the bot chat
//-----------------------------------------------
void impulseBotChatForceEnd(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	CBotChatManager::getInstance()->setCurrPage(NULL);
}


//-----------------------------------------------
// MISSION COMPLETED JOURNAL
//-----------------------------------------------
/*
#define MC_M_CONTAINER "ui:interface:info_player_journal"
#define MC_S_CONTAINER "ui:interface:ipj_com_missions"
#define MC_TEMPLATE "tipj_mission_complete"
//-----------------------------------------------
CGroupContainer *getMissionCompletedContainer()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(MC_M_CONTAINER);
	CGroupContainer *pGCM = dynamic_cast<CGroupContainer*>(pIE);
	if (pGCM == NULL) return NULL;

	CGroupList *pList = pGCM->getList();
	CGroupContainer *pGCS = dynamic_cast<CGroupContainer*>(pList->getGroup(MC_S_CONTAINER));
	return pGCS;
}

//-----------------------------------------------
void clearMissions()
{
	CGroupContainer *pGCMC = getMissionCompletedContainer();
	CInterfaceGroup *pContent = pGCMC->getGroup("content");
	pContent->clearGroups();
}
//-----------------------------------------------
void addMission(uint32 titleID)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGCMC = getMissionCompletedContainer();
	if (pGCMC == NULL)
	{
		nlwarning("cannot get container for missions completed");
		return;
	}
	CInterfaceGroup *pContent = pGCMC->getGroup("content");

	uint32 nNbMission = pContent->getGroups().size();
	vector<pair<string, string> > vArgs;

	vArgs.push_back(pair<string,string>("id", "mc"+NLMISC::toString(nNbMission)));
	vArgs.push_back(pair<string,string>("mcid", NLMISC::toString(titleID)));

 	if (nNbMission == 0)
	{
		vArgs.push_back(pair<string,string>("posref", "TL TL"));
		vArgs.push_back(pair<string,string>("posparent", "parent"));
		vArgs.push_back(pair<string,string>("y", "0"));
	}
	else
	{
		vArgs.push_back(pair<string,string>("posref", "BL TL"));
	}

	CInterfaceGroup *pIG = pIM->createGroupInstance(MC_TEMPLATE, pContent->getId(), vArgs);
	if (pIG == NULL)
	{
		nlwarning("cannot create a mission completed");
		return;
	}
	pIG->setParent(pContent);
 	if (nNbMission == 0)
		pIG->setParentPos(pContent);
	else
		pIG->setParentPos(pContent->getGroups()[nNbMission-1]);
	pContent->addGroup(pIG);
}
*/
//-----------------------------------------------
// impulseJournalInitCompletedMissions :
// initialize the player journal missions for completed missions
//-----------------------------------------------
void impulseJournalInitCompletedMissions (NLMISC::CBitMemStream &impulse)
{
/*
	vector<uint32> vMissionCompleted;
	impulse.serialCont(vMissionCompleted);

	clearMissions();

	for (uint32 i = 0; i < vMissionCompleted.size(); ++i)
		addMission (vMissionCompleted[i]);
*/
}

//-----------------------------------------------
// impulseJournalInitCompletedMissions :
// initialize the player journal missions for completed missions
//-----------------------------------------------
void impulseJournalUpdateCompletedMissions (NLMISC::CBitMemStream &impulse)
{
/*
	uint32 nNewCompletedMission;
	impulse.serial(nNewCompletedMission);

	addMission (nNewCompletedMission);
*/
}


//-----------------------------------------------
// impulseJournalCantAbandon :
// server refuses mission abandon
//-----------------------------------------------
void impulseJournalCantAbandon (NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	/// reactivate abandon button
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:MISSION_ABANDON_BUTTON",false);
	if (pNL != NULL)
		pNL->setValue64(1);
}



//-----------------------------------------------
// server add a compass target
//-----------------------------------------------
void impulseJournalAddCompass(NLMISC::CBitMemStream &impulse)
{
	sint32 x;
	sint32 y;
	uint32 text;
	impulse.serial(x);
	impulse.serial(y);
	impulse.serial(text);
	if (PermanentlyBanned) return;
	//nlinfo("impulseCallback : Received JOURNAL:ADD_COMPASS %d %d %d", x, y, text);
	CCompassDialogsManager::getInstance().addEntry( x,y,text );
}

//-----------------------------------------------
// server removes a compass target
//-----------------------------------------------
void impulseJournalRemoveCompass(NLMISC::CBitMemStream &impulse)
{
	uint32 text;
	impulse.serial(text);
	if (PermanentlyBanned) return;
	//nlinfo("impulseCallback : Received JOURNAL:REMOVE_COMPASS %d", text);
	CCompassDialogsManager::getInstance().removeEntry( text );
}



//
// the server ask me to execute a command
//
void impulseRemoteAdmin (NLMISC::CBitMemStream &impulse)
{
	CLog logDisplayVars;
	CLightMemDisplayer mdDisplayVars;
	logDisplayVars.addDisplayer (&mdDisplayVars);
	mdDisplayVars.setParam (10);

	uint32 rid;
	impulse.serial (rid);
	string cmd;
	impulse.serial (cmd);

	// remove the 2 first rc character if exists, only there to say to the EGS that is a remote command
	if (cmd.size()>2 && tolower(cmd[0])=='r' && tolower(cmd[1])=='c')
		cmd = cmd.substr(2);

	mdDisplayVars.clear ();
	ICommand::execute(cmd, logDisplayVars, !ICommand::isCommand(cmd));
	const std::deque<std::string>	&strs = mdDisplayVars.lockStrings();

	string str;
	if (ICommand::isCommand(cmd))
	{
		for (uint k = 0; k < strs.size(); k++)
		{
			str += strs[k];
		}
	}
	else
	{
		if (strs.size()>0)
		{
			str = strs[0].substr(0,strs[0].size()-1);
			// replace all spaces into underscore because space is a reserved char
			for (uint i = 0; i < str.size(); i++) if (str[i] == ' ') str[i] = '_';
		}
		else
		{
			str = "???";
		}
	}
	mdDisplayVars.unlockStrings();

	//nlinfo("impulseCallback : Received COMMAND:REMOTE_ADMIN : Server asked me to execute '%s', result is '%s'", cmd.c_str(), str.c_str());

	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("COMMAND:REMOTE_ADMIN_ANSWER", out))
	{
		out.serial (rid);
		out.serial (cmd);
		out.serial (str);
		NetMngr.push (out);
		//nlinfo("impulseCallback : COMMAND:REMOTE_ADMIN_ANSWER %d %s %s sent", rid, cmd.c_str(), str.c_str());
	}
}


//-----------------------------------------------
// impulseGuildAscensor :
// server request that the client launch the ascensor interface
//-----------------------------------------------

void impulseGuildAscensor (NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	//nlinfo("impulseCallback : Received GUILD:ASCENSOR");
	CGuildManager::getInstance()->launchAscensor();
}

//-----------------------------------------------
//impulseGuildLeaveAscensor
//-----------------------------------------------
void impulseGuildLeaveAscensor (NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	//nlinfo("impulseCallback : Received GUILD:LEAVE_ASCENSOR");
	CGuildManager::getInstance()->quitAscensor();
}

//-----------------------------------------------
//impulseGuildAbortCreation
//-----------------------------------------------
void impulseGuildAbortCreation (NLMISC::CBitMemStream &impulse)
{
	CBotChatPage *pPage = CBotChatManager::getInstance()->getCurrPage();
	CBotChatPageCreateGuild *pPageCG = dynamic_cast<CBotChatPageCreateGuild*>(pPage);
	if (pPageCG == BotChatPageAll->CreateGuild)
		CBotChatManager::getInstance()->setCurrPage(NULL);
}

void impulseGuildOpenGuildWindow(NLMISC::CBitMemStream &impulse)
{
	CGuildManager::getInstance()->openGuildWindow();
}


//-----------------------------------------------
// impulseGuildOpenInventory
//-----------------------------------------------
void impulseGuildOpenInventory (NLMISC::CBitMemStream &impulse)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:INVENTORY_GUILD_OPENED")->setValue32(1);
}

//-----------------------------------------------
// impulseGuildCloseInventory
//-----------------------------------------------
void impulseGuildCloseInventory (NLMISC::CBitMemStream &impulse)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:INVENTORY_GUILD_OPENED")->setValue32(0);
}

//-----------------------------------------------
// impulseGuildUpdatePlayerTitle
// server block/unblock some reserved titles
//-----------------------------------------------
void impulseGuildUpdatePlayerTitle(NLMISC::CBitMemStream &impulse)
{
	CSkillManager *pSM = CSkillManager::getInstance();
	bool bUnblock;
	impulse.serial(bUnblock);
	vector<uint16> vTitles;
	impulse.serialCont(vTitles);
	if (PermanentlyBanned) return;
	if (bUnblock)
	{
		for (uint32 i = 0; i < vTitles.size(); ++i)
			pSM->unblockTitleFromServer((CHARACTER_TITLE::ECharacterTitle)vTitles[i]);
	}
	else
	{
		for (uint32 i = 0; i < vTitles.size(); ++i)
			pSM->blockTitleFromServer((CHARACTER_TITLE::ECharacterTitle)vTitles[i]);
	}
}

//-----------------------------------------------
// impulseGuildUseFemaleTitles
// server activates/deactivates use of female titles
//-----------------------------------------------
void impulseGuildUseFemaleTitles(NLMISC::CBitMemStream &impulse)
{
	impulse.serial( UseFemaleTitles );
}

//-----------------------------------------------
// impulsePhraseDownLoad
// server upload the phrases.
//-----------------------------------------------
void impulsePhraseDownLoad (NLMISC::CBitMemStream &impulse)
{
	std::vector<CSPhraseSlot>	phrases;

	// Read Known Phrases
	impulse.serialCont(phrases);
	CSPhraseManager	*pPM= CSPhraseManager::getInstance();
	for(uint i=0;i<phrases.size();i++)
	{
		if(phrases[i].PhraseSheetId != CSheetId::Unknown)
		{
		   CSPhraseCom          phraseCom;
		   pPM->buildPhraseFromSheet(phraseCom, phrases[i].PhraseSheetId.asInt());
		   pPM->setPhraseNoUpdateDB(phrases[i].KnownSlot, phraseCom);
		}
		else
		{
		   pPM->setPhraseNoUpdateDB(phrases[i].KnownSlot, phrases[i].Phrase);
		}
	}
	// must update the DB (NB: if initInGameDone) after all phrase set.
	pPM->updateBookDB();

	// Then Read Memorized Phrases
	std::vector<CSPhraseMemorySlot>		memorizedPhrases;
	impulse.serialCont(memorizedPhrases);
	if (PermanentlyBanned) return;
	for(uint i=0;i<memorizedPhrases.size();i++)
	{
		pPM->memorizePhrase(
			memorizedPhrases[i].MemoryLineId,
			memorizedPhrases[i].MemorySlotId,
			memorizedPhrases[i].PhraseId);
	}

	// OK.
	extern bool	SabrinaPhraseBookLoaded;
	SabrinaPhraseBookLoaded= true;

	// update gray state, if game inited.
	pPM->updateMemoryBar();
}

//-----------------------------------------------
// impulsePhraseConfirmBuy
// server confirm/infirm the buy of botchat phrase.
//-----------------------------------------------
void impulsePhraseConfirmBuy (NLMISC::CBitMemStream &impulse)
{
	uint16	phraseId;
	bool	confirm;

	impulse.serial(phraseId);
	impulse.serial(confirm);

	if (PermanentlyBanned) return;

	CSPhraseManager		*pSM= CSPhraseManager::getInstance();
	pSM->receiveBotChatConfirmBuy(phraseId, confirm);
}


//-----------------------------------------------
// impulsePhraseAckExecuteCyclic
// server confirm/infirm the cyclic execution of a phrase
//-----------------------------------------------
void impulsePhraseAckExecuteCyclic (NLMISC::CBitMemStream &impulse)
{
	uint8	counter;
	bool	ok;

	impulse.serial(ok);
	impulse.serial(counter);

	if (PermanentlyBanned) return;


	CSPhraseManager		*pSM= CSPhraseManager::getInstance();
	pSM->receiveAckExecuteFromServer(true, counter, ok);
}


//-----------------------------------------------
// impulsePhraseAckExecuteCyclic
// server confirm/infirm the execution of a phrase
//-----------------------------------------------
void impulsePhraseAckExecuteNext (NLMISC::CBitMemStream &impulse)
{
	uint8	counter;
	bool	ok;

	impulse.serial(ok);
	impulse.serial(counter);

	if (PermanentlyBanned) return;

	CSPhraseManager		*pSM= CSPhraseManager::getInstance();
	pSM->receiveAckExecuteFromServer(false, counter, ok);
}

// Same params as in BOMB_IF
#ifdef FINAL_VERSION
#define SKIP_IF(condition,msg,skipAction) if (!(condition)); else skipAction;
#else
#define SKIP_IF(condition,msg,skipAction) if (!(condition)) WARN(msg); else skipAction;
#endif

template <class CInventoryCategoryTemplate>
void updateInventoryFromStream (NLMISC::CBitMemStream &impulse, const CInventoryCategoryTemplate *templ, bool notifyItemSheetChanges)
{
	try
	{
		// get the egs tick of this change
		TGameCycle	serverTick;
		impulse.serial(serverTick);

		// For All inventories
		for ( uint invId=0; invId!=CInventoryCategoryTemplate::NbInventoryIds; ++invId )
		{
			// Presence bit
			bool hasContent;
			impulse.serialBit( hasContent );
			if ( ! hasContent )
				continue;

			// Number field
			uint32 nbChanges;
			impulse.serial( nbChanges, INVENTORIES::LowNumberBits );
			if ( nbChanges == INVENTORIES::LowNumberBound )
				impulse.serial( nbChanges, 32 );

			const string invBranchStr = CInventoryCategoryTemplate::getDbStr( (typename CInventoryCategoryTemplate::TInventoryId)invId );
			ICDBNode::CTextId textId( invBranchStr );
			ICDBNode *inventoryNode = IngameDbMngr.getNodePtr()->getNode( textId, false );
			BOMB_IF( !inventoryNode, "Inventory missing in database", return )

			// List of updates
			for ( uint c=0; c!=nbChanges; ++c )
			{
				// Unpack (the bitmemstream is written from high-order to low-order)
				uint32 iuInfoVersion;
				impulse.serial( iuInfoVersion, 1 );
				if ( iuInfoVersion == 1 )
				{
					uint32 slotIndex;
					impulse.serial( slotIndex, CInventoryCategoryTemplate::SlotBitSize );

					// Access the database leaf
					CCDBNodeBranch *slotNode = safe_cast<CCDBNodeBranch*>(inventoryNode->getNode( (uint16)slotIndex ));
					CCDBNodeLeaf *leafNode = type_cast<CCDBNodeLeaf*>(slotNode->find( INVENTORIES::InfoVersionStr ));
					BOMB_IF( !leafNode, "Inventory slot property missing in database", continue );

					// Apply or increment Info Version in database
					if ( CInventoryCategoryTemplate::needPlainInfoVersionTransfer() )
					{
						uint32 infoVersion;
						impulse.serial( infoVersion, INVENTORIES::InfoVersionBitSize );
						leafNode->setPropCheckGC( serverTick, infoVersion );
					}
					else
					{
						// NB: don't need to check GC on a info version upgrade, since this is always a delta of +1
						// the order of received of this impulse is not important
						leafNode->setValue64( leafNode->getValue64() + 1 );
					}

				}
				else
				{
					uint32 iuAll;
					impulse.serial( iuAll, 1 );
					if ( iuAll == 1 )
					{
						INVENTORIES::CItemSlot itemSlot;
						itemSlot.serialAll( impulse, templ );
						//nldebug( "Inv %s Update %u", CInventoryCategoryTemplate::InventoryStr[invId], itemSlot.getSlotIndex() );

						// Apply all properties to database
						CCDBNodeBranch *slotNode = safe_cast<CCDBNodeBranch*>(inventoryNode->getNode( (uint16)itemSlot.getSlotIndex() ));
						for ( uint i=0; i!=INVENTORIES::NbItemPropId; ++i )
						{
							CCDBNodeLeaf *leafNode = type_cast<CCDBNodeLeaf*>(slotNode->find( string(INVENTORIES::CItemSlot::ItemPropStr[i]) ));
							SKIP_IF( !leafNode, "Inventory slot property missing in database", continue );
							leafNode->setPropCheckGC( serverTick, (sint64)itemSlot.getItemProp( ( INVENTORIES::TItemPropId)i ) );
						}
					}
					else
					{
						uint32 iuOneProp;
						impulse.serial( iuOneProp, 1 );
						if ( iuOneProp == 1 )
						{
							INVENTORIES::CItemSlot itemSlot;
							itemSlot.serialOneProp( impulse, templ );
							//nldebug( "Inv %s Prop %u %s", CInventoryCategoryTemplate::InventoryStr[invId], itemSlot.getSlotIndex(), INVENTORIES::CItemSlot::ItemPropStr[itemSlot.getOneProp().ItemPropId] );

							// Apply property to database
							CCDBNodeBranch *slotNode = safe_cast<CCDBNodeBranch*>(inventoryNode->getNode( (uint16)itemSlot.getSlotIndex() ));
							CCDBNodeLeaf *leafNode = type_cast<CCDBNodeLeaf*>(slotNode->find( string(INVENTORIES::CItemSlot::ItemPropStr[itemSlot.getOneProp().ItemPropId]) ));
							if (!leafNode) nlwarning("BUG: Inventory slot property missing in database (iuOneProp) (%s, %i, %s)", slotNode->getFullName().c_str(), (sint)itemSlot.getOneProp().ItemPropId, INVENTORIES::CItemSlot::ItemPropStr[itemSlot.getOneProp().ItemPropId]);
							SKIP_IF( !leafNode, "Inventory slot property missing in database", continue );
							leafNode->setPropCheckGC( serverTick, (sint64)itemSlot.getOneProp().ItemPropValue );

						}
						else // iuReset
						{
							uint32 slotIndex;
							impulse.serial( slotIndex, CInventoryCategoryTemplate::SlotBitSize );
							//nldebug( "Inv %s Reset %u", CInventoryCategoryTemplate::InventoryStr[invId], slotIndex );

							// Reset all properties in database
							CCDBNodeBranch *slotNode = safe_cast<CCDBNodeBranch*>(inventoryNode->getNode( (uint16)slotIndex ));
							for ( uint i=0; i!=INVENTORIES::NbItemPropId; ++i )
							{
								// Instead of clearing all leaves (by index), we must find and clear only the
								// properties in TItemPropId, because the actual database leaves may have
								// less properties, and because we must not clear the leaf INFO_VERSION.
								CCDBNodeLeaf *leafNode = type_cast<CCDBNodeLeaf*>(slotNode->find( string(INVENTORIES::CItemSlot::ItemPropStr[i]) ));
								if (!leafNode) nlwarning("BUG: Inventory slot property missing in database (iuReset) (%s, %i, %s)", slotNode->getFullName().c_str(), (sint)i, INVENTORIES::CItemSlot::ItemPropStr[i]);
								SKIP_IF( !leafNode, "Inventory slot property missing in database", continue );
								leafNode->setPropCheckGC( serverTick, 0 );
							}
						}
					}
				}
			}
		}

		CInventoryManager::getInstance()->sortBag();
	}
	catch (const Exception &e)
	{
		nlwarning ("Problem while decoding a DB_UPD_INV msg, skipped: %s", e.what());
	}
}

//-----------------------------------------------
// impulseUpdateInventory:
//-----------------------------------------------
void impulseUpdateInventory (NLMISC::CBitMemStream &impulse)
{
	updateInventoryFromStream( impulse, (INVENTORIES::CInventoryCategoryForCharacter*)NULL, true );
};

//-----------------------------------------------
// impulseInitInventory:
//-----------------------------------------------
void impulseInitInventory (NLMISC::CBitMemStream &impulse)
{
	sint32 p = impulse.getPos();
	impulseUpdateInventory( impulse );
	IngameDbMngr.setInitPacketReceived();
	nlinfo( "DB_INIT:INV done (%u bytes)", impulse.getPos()-p );
}

//-----------------------------------------------
// impulseItemInfoSet:
//-----------------------------------------------
void impulseItemInfoSet (NLMISC::CBitMemStream &impulse)
{
	CItemInfos	itemInfos;
	impulse.serial(itemInfos);

	getInventory().onReceiveItemInfo(itemInfos);
}

//-----------------------------------------------
// impulseItemInfoRefreshVersion:
//-----------------------------------------------
void impulseItemInfoRefreshVersion (NLMISC::CBitMemStream &impulse)
{
	uint16 slotId;
	uint8 infoVersion;
	impulse.serial(slotId);
	impulse.serial(infoVersion);

	getInventory().onRefreshItemInfoVersion(slotId, infoVersion);
}

//-----------------------------------------------
// impulsePrereqInfoSet:
//-----------------------------------------------
void impulsePrereqInfoSet (NLMISC::CBitMemStream &impulse)
{
	CPrerequisitInfos prereqInfos;
	uint8 index;
	impulse.serial(prereqInfos);
	impulse.serial(index);

	//write infos in interface
	CBotChatManager::getInstance()->onReceiveMissionInfo(index, prereqInfos);
}

//-----------------------------------------------
//-----------------------------------------------
void impulseDeathRespawnPoint (NLMISC::CBitMemStream &impulse)
{
	CRespawnPointsMsg msg;
	impulse.serial(msg);
	if (PermanentlyBanned) return;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupMap *pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:respawn_map:content:map_content:actual_map"));
	if (pMap == NULL)
	{
		nlwarning("problem cannot find ui:interface:respawn_map:content:map_content:actual_map");
		return;
	}
	pMap->addRespawnPoints(msg);


	pMap = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:actual_map"));
	if (pMap == NULL)
	{
		nlwarning("problem cannot find ui:interface:map:content:map_content:actual_map");
		return;
	}
	pMap->addRespawnPoints(msg);
}

//-----------------------------------------------
//-----------------------------------------------
void impulseDeathRespawn (NLMISC::CBitMemStream &impulse)
{
	// TODO : Bring me to life !!!
}

//-----------------------------------------------
// impulseDuelInvitation :
//-----------------------------------------------
void impulseDuelInvitation(NLMISC::CBitMemStream &impulse)
{
	uint32 textID;
	impulse.serial(textID);

	//nlinfo("impulseCallback : Received DUEL:INVITATION %d", textID);

	if (PermanentlyBanned) return;

	//activate the pop up window
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:join_duel_proposal"));
	if (pGC == NULL) return;
	CViewTextID *pVTID = dynamic_cast<CViewTextID *>(pGC->getView("invitor_name"));
	if (pVTID == NULL) return;
	pVTID->setTextId(textID);
	pGC->setActive(true);
	CWidgetManager::getInstance()->setTopWindow(pGC);
	pGC->updateCoords();
	pGC->center();
	pGC->enableBlink(2);

}// impulseDuelInvitation //

//-----------------------------------------------
// impulseDuelCancelInvitation:
//-----------------------------------------------
void impulseDuelCancelInvitation(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	//nlinfo("impulseCallback : Received DUEL:CANCEL_INVITATION");

	//activate the pop up window
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:join_duel_proposal"));
	if (pGC == NULL) return;
	pGC->setActive(false);

}// impulseDuelCancelInvitation //

//-----------------------------------------------
// impulsePVPChallengeInvitation :
//-----------------------------------------------
void impulsePVPChallengeInvitation(NLMISC::CBitMemStream &impulse)
{
	uint32 textID;
	impulse.serial(textID);

	if (PermanentlyBanned) return;

	//nlinfo("impulseCallback : Received PVP_CHALLENGE:INVITATION %d", textID);

	//activate the pop up window
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:join_pvp_challenge_proposal"));
	if (pGC == NULL) return;
	CViewTextID *pVTID = dynamic_cast<CViewTextID *>(pGC->getView("invitor_name"));
	if (pVTID == NULL) return;
	pVTID->setTextId(textID);
	pGC->setActive(true);
	CWidgetManager::getInstance()->setTopWindow(pGC);
	pGC->updateCoords();
	pGC->center();
	pGC->enableBlink(2);

}// impulsePVPChallengeInvitation //

//-----------------------------------------------
// impulsePVPChallengeCancelInvitation:
//-----------------------------------------------
void impulsePVPChallengeCancelInvitation(NLMISC::CBitMemStream &impulse)
{
	//nlinfo("impulseCallback : Received PVP_CHALLENGE:CANCEL_INVITATION");

	//activate the pop up window
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:join_pvp_challenge_proposal"));
	if (pGC == NULL) return;
	pGC->setActive(false);

}// impulsePVPChallengeCancelInvitation //



//-----------------------------------------------
// impulsePVPFactionPushFactionWar:
//-----------------------------------------------
void impulsePVPFactionPushFactionWar(NLMISC::CBitMemStream &impulse)
{
	//nlinfo("impulseCallback : Received PVP_FACTION:PUSH_FACTION_WAR");

	PVP_CLAN::CFactionWar factionWar;
	impulse.serialEnum(factionWar.Clan1);
	impulse.serialEnum(factionWar.Clan2);

	CFactionWarManager::getInstance()->addFactionWar(factionWar);
}


//-----------------------------------------------
// impulsePVPFactionPopFactionWar:
//-----------------------------------------------
void impulsePVPFactionPopFactionWar(NLMISC::CBitMemStream &impulse)
{
	//nlinfo("impulseCallback : Received PVP_FACTION:POP_FACTION_WAR");

	PVP_CLAN::CFactionWar factionWar;
	impulse.serialEnum(factionWar.Clan1);
	impulse.serialEnum(factionWar.Clan2);

	CFactionWarManager::getInstance()->stopFactionWar(factionWar);
}


//-----------------------------------------------
// impulsePVPFactionFactionWars:
//-----------------------------------------------
void impulsePVPFactionFactionWars(NLMISC::CBitMemStream &impulse)
{
	//nlinfo("impulseCallback : Received PVP_FACTION:FACTION_WARS");

	CFactionWarsMsg factionWars;
	impulse.serial(factionWars);

	for( uint i=0; i<factionWars.FactionWarOccurs.size(); ++i )
	{
		CFactionWarManager::getInstance()->addFactionWar(factionWars.FactionWarOccurs[i]);
	}
}



//-----------------------------------------------
// impulsePVPChooseClan
//-----------------------------------------------
/*
void impulsePVPChooseClan(NLMISC::CBitMemStream &impulse)
{
	nlinfo("impulsePVPChooseClan : Received PVP_CLAN:CHOOSE_CLAN");

	EGSPD::CPeople::TPeople clan1= EGSPD::CPeople::Unknown, clan2= EGSPD::CPeople::Unknown;
	impulse.serialEnum( clan1 );
	impulse.serialEnum( clan2 );

	if (PermanentlyBanned) return;

	//activate the pop up window
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:join_pvp_clan_proposal"));
	if (pGC == NULL) return;
	pGC->setActive(true);

	CCtrlTextButton * butClan1 = dynamic_cast<CCtrlTextButton*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:join_pvp_clan_proposal:content:clan1"));
	if( butClan1 == NULL )
		return;
	butClan1->setText( ucstring(EGSPD::CPeople::toString( clan1 )) );

	CCtrlTextButton * butClan2 = dynamic_cast<CCtrlTextButton*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:join_pvp_clan_proposal:content:clan2"));
	if( butClan2 == NULL )
		return;
	butClan2->setText( ucstring(EGSPD::CPeople::toString( clan2 )) );
}
*/

//-----------------------------------------------
// impulseEncyclopediaUpdate
//-----------------------------------------------
void impulseEncyclopediaUpdate(NLMISC::CBitMemStream &impulse)
{
	//nlinfo("impulseCallback : Received ENCYCLOPEDIA:UPDATE");

	CEncyclopediaUpdateMsg msg;
	impulse.serial(msg);
	if (PermanentlyBanned) return;
	CEncyclopediaManager::getInstance()->update(msg);
}// impulseEncyclopediaUpdate //

//-----------------------------------------------
// impulseEncyclopediaInit
//-----------------------------------------------
void impulseEncyclopediaInit(NLMISC::CBitMemStream &impulse)
{
	//nlinfo("impulseCallback : Received ENCYCLOPEDIA:INIT");

	CEncyclopediaUpdateMsg msg;
	impulse.serial(msg);
	if (PermanentlyBanned) return;
	CEncyclopediaManager::getInstance()->update(msg);
}// impulseEncyclopediaInit //

//-----------------------------------------------
//-----------------------------------------------
void impulseItemOpenRoomInventory(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	// This is a message because we may do other things there
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:INVENTORY_ROOM_OPENED")->setValue32(1);
}

//-----------------------------------------------
//-----------------------------------------------
void impulseItemCloseRoomInventory(NLMISC::CBitMemStream &impulse)
{
	if (PermanentlyBanned) return;
	// This is a message because we may do other things there
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:INVENTORY_ROOM_OPENED")->setValue32(0);

	// deactivate the pop up window
	CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:inv_room"));
	if (pGC == NULL) return;
	pGC->setActive(false);
}

//-----------------------------------------------
//-----------------------------------------------
void impulseUserBars(NLMISC::CBitMemStream &impulse)
{
	uint8	msgNumber;
	sint32	hp, sap, sta, focus;
	impulse.serial(msgNumber);
	impulse.serial(hp);
	impulse.serial(sap);
	impulse.serial(sta);
	impulse.serial(focus);

	if (PermanentlyBanned) return;

	// Setup the user Bars
	CBarManager::CBarInfo	bi;
	CBarManager::getInstance()->setupUserBarInfo(msgNumber, hp, sap, sta, focus);
}

//-----------------------------------------------
//-----------------------------------------------
void impulseOutpostChooseSide(NLMISC::CBitMemStream &impulse)
{
	// read message
	bool playerGuildInConflict;
	bool playerGuildIsAttacker;
	impulse.serial(playerGuildInConflict);
	impulse.serial(playerGuildIsAttacker);
	uint32 ownerGuildNameId;
	impulse.serial( ownerGuildNameId );
	uint32 attackerGuildNameId;
	impulse.serial( attackerGuildNameId );
	uint32 declTimer;
	impulse.serial( declTimer );

	// start
	OutpostManager.startPvpJoinProposal(playerGuildInConflict, playerGuildIsAttacker,
		ownerGuildNameId, attackerGuildNameId, declTimer);
}

//-----------------------------------------------
//-----------------------------------------------
void impulseOutpostDeclareWarAck(NLMISC::CBitMemStream &impulse)
{
	bool	canValidate;
	uint32	docTextId;
	uint32	timeStartAttack;

	impulse.serial(canValidate);
	impulse.serial(docTextId);
	impulse.serial(timeStartAttack);

	// write result in Local DB.
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	// ack reception
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ACK_RECEIVED");
	if(node)
		node->setValueBool(true);
	// set result of ACK
	node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ACK_OK");
	if(node)
		node->setValueBool(canValidate);
	node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ACK_TEXTID");
	if(node)
		node->setValue32(docTextId);
	node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ACK_TIME_RANGE_ATT");
	if(node)
		node->setValue32(timeStartAttack);
}

extern void addWebIGParams(string &url, bool trustedDomain);

//-----------------------------------------------
//-----------------------------------------------
class CServerMessageBoxOnReceiveTextId : public STRING_MANAGER::IStringWaitCallback
{
private:
	enum TTextType {TitleType= 0, ContentType, NumTextType};
	uint32		_TextId[NumTextType];
	bool		_TextReceived[NumTextType];
	bool		_AlreadyDisplayed;

	// show the window
	void	activateMsgBoxWindow()
	{
		STRING_MANAGER::CStringManagerClient		*pSMC= STRING_MANAGER::CStringManagerClient::instance();

		// get the content string (should have been received!)
		ucstring	contentStr;
		ucstring	titleStr;
		if(!pSMC->getDynString(_TextId[ContentType], contentStr))
			return;

		if(!pSMC->getDynString(_TextId[TitleType], titleStr))
			return;

		// if the string start with a @{Wxxxx} code, remove it and get the wanted window size
		sint	w = 256;		// default size to 256 !!
		bool	is_webig = false;

		if(contentStr.size()>=6 && contentStr[0]=='W' && contentStr[1]=='E' && contentStr[2]=='B'
			&& contentStr[3]==' ' && contentStr[4]==':' && contentStr[5]==' ' )
		{
			uint i;
			const uint digitStart= 6;
			const uint digitMaxEnd= (uint)contentStr.size();

			is_webig = true;

			for(i = digitStart; i < digitMaxEnd; i++)
			{
				if(contentStr[i] == ' ')
					break;
			}
			if(i != digitMaxEnd)
			{
				ucstring web_app = contentStr.substr(digitStart, i-digitStart);
				contentStr = ucstring("http://"+ClientCfg.WebIgMainDomain+"/")+web_app+ucstring("/index.php?")+contentStr.substr(i+1);
			}
			else
			{
				contentStr = "";
				i = digitStart;
			}
		} 
		else if(contentStr.size()>=5 && contentStr[0]=='@' && contentStr[1]=='{' && contentStr[2]=='W')
		{
			uint	i;
			const	uint digitStart= 3;
			const	uint digitMaxEnd= 8;
			for(i=digitStart;i<contentStr.size() && i<digitMaxEnd;i++)
			{
				if(contentStr[i]=='}')
					break;
			}
			if(i!=digitMaxEnd)
			{
				// get the width
				ucstring	digitStr= contentStr.substr(digitStart, i-digitStart);
				fromString(digitStr.toString(), w);
				// remove the first tag
				contentStr= contentStr.substr(i+1);
			}
		}

		// open the message box window or web ig
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();

		if (is_webig)
		{
			CGroupHTML *groupHtml;
			string group = titleStr.toString();
			// <missing:XXX>
			group = group.substr(9, group.size()-10);
			groupHtml = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:"+group+":content:html"));
			if (!groupHtml)
			{
				groupHtml = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:webig:content:html"));
				group = "webig";
			}

			if (groupHtml)
			{
				CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:"+group));
				if (pGC)
				{
					if (contentStr.empty())
					{
						pGC->setActive(false);
					}
					else
					{
						if (group == "webig")
							pGC->setActive(true);
						string url = contentStr.toString();
						addWebIGParams(url, true);
						groupHtml->browse(url.c_str());
						CWidgetManager::getInstance()->setTopWindow(pGC);
					}
				}
			}
		}
		else
		{
			CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:server_message_box"));
			if (pGC)
			{
				// show the window with correct width
				pGC->setW(w);
				pGC->setActive(true);

				// must set the text by hand
				CViewText	*vt= dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(CWidgetManager::getInstance()->getParser()->getDefine("server_message_box_content_view_text")));
				if(vt)
					vt->setTextFormatTaged(contentStr);

				// open
				CWidgetManager::getInstance()->setTopWindow(pGC);
				pGC->invalidateCoords();
				// Yoyo: because of buggued group container, I found that 6 times is a good number....
				for(uint i=0;i<6;i++)
					pGC->updateCoords();
				pGC->center();
				pGC->enableBlink(2);
			}
		}
	}

public:
	// called when the string is available
	virtual void onDynStringAvailable(uint stringId, const ucstring &value)
	{
		// don't care if already displayed
		if(_AlreadyDisplayed)
			return;

		// check if one of waited text
		for(uint i=0;i<NumTextType;i++)
		{
			if(stringId==_TextId[i])
			{
				_TextReceived[i]= true;
			}
		}

		// all received?
		for(uint i=0;i<NumTextType;i++)
		{
			if(!_TextReceived[i])
				return;
		}
		// Yes => display window
		_AlreadyDisplayed= true;
		activateMsgBoxWindow();
	}

	// start the waiter
	void		startWaitTexts(uint32 titleTextId, uint32 docTextId)
	{
		// reset
		_TextId[TitleType]= titleTextId;
		_TextId[ContentType]= docTextId;
		_TextReceived[TitleType]= false;
		_TextReceived[ContentType]= false;
		_AlreadyDisplayed= false;

		// start to wait receive of those string (NB: they may be already here, but waitDynStrings calls directly the callback in this case)
		STRING_MANAGER::CStringManagerClient		*pSMC= STRING_MANAGER::CStringManagerClient::instance();
		pSMC->waitDynString(titleTextId, this);
		pSMC->waitDynString(docTextId, this);
	}
};
CServerMessageBoxOnReceiveTextId	ServerMessageBoxOnReceiveTextId;


void	impulseUserPopup(NLMISC::CBitMemStream &impulse)
{
	uint32	titleTextId;
	uint32	docTextId;
	impulse.serial(titleTextId);
	impulse.serial(docTextId);

	// setup TEMP DB for title
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:TEMP:SERVER_POPUP:TITLE");
	if(node)		node->setValue32(titleTextId);

	// Open the Popup only when the 2 dyn strings are available
	ServerMessageBoxOnReceiveTextId.startWaitTexts(titleTextId, docTextId);
}

//-----------------------------------------------
//-----------------------------------------------
//extern void impulseCombatFlyingHpDelta(NLMISC::CBitMemStream &impulse);
void impulseCombatFlyingHpDelta(NLMISC::CBitMemStream &impulse)
{
	uint32 entityID;
	uint32 rgba;
	sint16 hpDelta;
	impulse.serial(entityID);
	impulse.serial(rgba);
	impulse.serial(hpDelta);
	CRGBA color((uint8)(rgba>>24&255), (uint8)(rgba>>16&255), (uint8)(rgba>>8&255), (uint8)(rgba&255));
	CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex(entityID);
	if (entity)
		entity->addHPOutput(ucstring(toString("%d", hpDelta)), color);
}

void impulseCombatFlyingTextItemSpecialEffectProc(NLMISC::CBitMemStream &impulse)
{
	uint32 entityID;
	uint32 rgba;
	uint8 effect;
	sint32 param;
	impulse.serial(entityID);
	impulse.serial(rgba);
	impulse.serial(effect);
	impulse.serial(param);
	CRGBA color((uint8)(rgba>>24&255), (uint8)(rgba>>16&255), (uint8)(rgba>>8&255), (uint8)(rgba&255));
	ucstring text = CI18N::get(toString("uiItemSpecialEffectFlyingText%s", ITEM_SPECIAL_EFFECT::toString((ITEM_SPECIAL_EFFECT::TItemSpecialEffect)effect).c_str()));
	strFindReplace(text, "%param", toString("%d", param));
	CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex(entityID);
	if (entity)
		entity->addHPOutput(text, color);
}

void impulseCombatFlyingText(NLMISC::CBitMemStream &impulse)
{
	uint32 entityID;
	uint8 tmp;
	impulse.serial(entityID);
	impulse.serial(tmp);
	COMBAT_FLYING_TEXT::TCombatFlyingText type = (COMBAT_FLYING_TEXT::TCombatFlyingText)tmp;

	CRGBA color(255, 255, 255);
	ucstring text("");
	float dt = 0.0f;

	switch (type)
	{
	case COMBAT_FLYING_TEXT::TargetDodge:		// The target dodged
		color = CRGBA(255, 128, 64);
		text = CI18N::get("uiDodge");
		break;

	case COMBAT_FLYING_TEXT::TargetParry:		// The target parried
		color = CRGBA(255, 128, 64);
		text = CI18N::get("uiParry");
		break;

	case COMBAT_FLYING_TEXT::TargetEvade:		// Actually the user miss his hit
		color = CRGBA(255, 128, 64);
		text = CI18N::get("uiEvade");
		break;

	case COMBAT_FLYING_TEXT::SelfEvade:			// Actually the target miss his hit
		color = CRGBA(255, 255, 0);
		text = CI18N::get("uiEvade");
		break;

	case COMBAT_FLYING_TEXT::TargetResist:		// The target resisted magic
		color = CRGBA(255, 128, 64);
		text = CI18N::get("uiResist");
		break;

	case COMBAT_FLYING_TEXT::SelfResist:		// The user resisted magic
		color = CRGBA(255, 255, 0);
		text = CI18N::get("uiResist");
		break;

	case COMBAT_FLYING_TEXT::SelfInterrupt:		// the user cast was interupted
		color = CRGBA(200, 0, 0);
		text = CI18N::get("uiInterrupt");
		dt = 0.4f;
		break;

	case COMBAT_FLYING_TEXT::SelfFailure:		// The user failed to cast
		color = CRGBA(200, 0, 0);
		text = CI18N::get("uiFailure");
		break;

	default: // bad type
		nlwarning("Bad type for COMBAT_FLYING_TEXT:TCombatFlyingText enum");
		break;
	}

	CEntityCL *entity = EntitiesMngr.getEntityByCompressedIndex(entityID);
	if (entity)
		entity->addHPOutput(text, color, dt);
}

void  impulseSetSeason(NLMISC::CBitMemStream &impulse)
{
	extern uint8 ServerSeasonValue;
	extern bool ServerSeasonReceived;
	impulse.serial(ServerSeasonValue);
	ServerSeasonReceived = true;
}

void impulseDssDown(NLMISC::CBitMemStream &impulse)
{
	FarTP.onDssDown();
}

void impulseSetNpcIconDesc(NLMISC::CBitMemStream &impulse)
{
	uint8 nb8;
	impulse.serial(nb8);
	bool hasChanged = false;
	for (uint i=0; i!=(uint)nb8; ++i)
	{
		TNPCIconCacheKey npcIconCacheKey;
		impulse.serial(npcIconCacheKey);
		uint32 state;
		impulse.serial(state);
		hasChanged = CNPCIconCache::getInstance().onReceiveMissionAvailabilityForThisChar(npcIconCacheKey, (NPC_ICON::TNPCMissionGiverState)state) || hasChanged; // mind the order to avoid partial evaluation
	}
	if (hasChanged)
		CNPCIconCache::getInstance().refreshIconsOfScene();
}

void impulseServerEventForMissionAvailability(NLMISC::CBitMemStream &impulse)
{
	CNPCIconCache::getInstance().onEventForMissionAvailabilityForThisChar();
}

void impulseSetNpcIconTimer(NLMISC::CBitMemStream &impulse)
{
	NLMISC::TGameCycle delay;
	impulse.serial(delay);
	CNPCIconCache::getInstance().setMissionGiverTimer(delay);
}

//-----------------------------------------------
// initializeNetwork :
//-----------------------------------------------
void initializeNetwork()
{
	GenericMsgHeaderMngr.setCallback("DB_UPD_PLR",						impulseDatabaseUpdatePlayer);
	GenericMsgHeaderMngr.setCallback("DB_INIT:PLR",						impulseDatabaseInitPlayer);
	GenericMsgHeaderMngr.setCallback("DB_UPD_INV",						impulseUpdateInventory);
	GenericMsgHeaderMngr.setCallback("DB_INIT:INV",						impulseInitInventory);
	GenericMsgHeaderMngr.setCallback("DB_GROUP:UPDATE_BANK",			impulseDatabaseUpdateBank);
	GenericMsgHeaderMngr.setCallback("DB_GROUP:INIT_BANK",				impulseDatabaseInitBank);
	GenericMsgHeaderMngr.setCallback("DB_GROUP:RESET_BANK",				impulseDatabaseResetBank);
	GenericMsgHeaderMngr.setCallback("CONNECTION:NO_USER_CHAR",			impulseNoUserChar);
	GenericMsgHeaderMngr.setCallback("CONNECTION:USER_CHARS",			impulseUserChars);
	GenericMsgHeaderMngr.setCallback("CONNECTION:USER_CHAR",			impulseUserChar);
	GenericMsgHeaderMngr.setCallback("CONNECTION:FAR_TP",				impulseFarTP);
	GenericMsgHeaderMngr.setCallback("CONNECTION:READY",				impulseServerReady);
	GenericMsgHeaderMngr.setCallback("CONNECTION:VALID_NAME",			impulseCharNameValid);
	GenericMsgHeaderMngr.setCallback("CONNECTION:SHARD_ID",				impulseShardId);
	GenericMsgHeaderMngr.setCallback("CONNECTION:SERVER_QUIT_OK",		impulseServerQuitOk);
	GenericMsgHeaderMngr.setCallback("CONNECTION:SERVER_QUIT_ABORT",	impulseServerQuitAbort);
	GenericMsgHeaderMngr.setCallback("CONNECTION:MAIL_AVAILABLE",		impulseMailNotification);
	GenericMsgHeaderMngr.setCallback("CONNECTION:GUILD_MESSAGE_AVAILABLE",	impulseForumNotification);
	GenericMsgHeaderMngr.setCallback("CONNECTION:PERMANENT_BAN",		impulsePermanentBan);
	GenericMsgHeaderMngr.setCallback("CONNECTION:UNBAN",		        impulsePermanentUnban);

	GenericMsgHeaderMngr.setCallback("STRING:CHAT",				impulseChat);
	GenericMsgHeaderMngr.setCallback("STRING:TELL",				impulseTell);
	GenericMsgHeaderMngr.setCallback("STRING:FAR_TELL",			impulseFarTell);
	GenericMsgHeaderMngr.setCallback("STRING:CHAT2",			impulseChat2);
	GenericMsgHeaderMngr.setCallback("STRING:DYN_STRING",		impulseDynString);
	GenericMsgHeaderMngr.setCallback("STRING:DYN_STRING_GROUP",	inpulseDynStringInChatGroup);
	GenericMsgHeaderMngr.setCallback("STRING:TELL2",			impulseTell2);
//	GenericMsgHeaderMngr.setCallback("STRING:ADD_DYN_STR",		impulseAddDynStr);
	GenericMsgHeaderMngr.setCallback("TP:DEST",					impulseTP);
	GenericMsgHeaderMngr.setCallback("TP:DEST_WITH_SEASON",		impulseTPWithSeason);
	GenericMsgHeaderMngr.setCallback("TP:CORRECT",				impulseCorrectPos);
	GenericMsgHeaderMngr.setCallback("COMBAT:ENGAGE_FAILED",	impulseCombatEngageFailed);
	GenericMsgHeaderMngr.setCallback("BOTCHAT:DYNCHAT_OPEN",	impulseDynChatOpen);
	GenericMsgHeaderMngr.setCallback("BOTCHAT:DYNCHAT_CLOSE",	impulseDynChatClose);

	GenericMsgHeaderMngr.setCallback("CASTING:BEGIN",			impulseBeginCast);
	GenericMsgHeaderMngr.setCallback("TEAM:INVITATION",			impulseTeamInvitation);
	GenericMsgHeaderMngr.setCallback("TEAM:SHARE_OPEN",			impulseTeamShareOpen);
	GenericMsgHeaderMngr.setCallback("TEAM:SHARE_INVALID",		impulseTeamShareInvalid);
	GenericMsgHeaderMngr.setCallback("TEAM:SHARE_CLOSE",		impulseTeamShareClose);
	GenericMsgHeaderMngr.setCallback("TEAM:CONTACT_INIT",		impulseTeamContactInit);
	GenericMsgHeaderMngr.setCallback("TEAM:CONTACT_CREATE",		impulseTeamContactCreate);
	GenericMsgHeaderMngr.setCallback("TEAM:CONTACT_STATUS",		impulseTeamContactStatus);
	GenericMsgHeaderMngr.setCallback("TEAM:CONTACT_REMOVE",		impulseTeamContactRemove);

	GenericMsgHeaderMngr.setCallback("EXCHANGE:INVITATION",		impulseExchangeInvitation);
	GenericMsgHeaderMngr.setCallback("EXCHANGE:CLOSE_INVITATION", impulseExchangeCloseInvitation);
	GenericMsgHeaderMngr.setCallback("ANIMALS:MOUNT_ABORT",		impulseMountAbort);

	GenericMsgHeaderMngr.setCallback("DEBUG:REPLY_WHERE",		impulseWhere);
	GenericMsgHeaderMngr.setCallback("DEBUG:COUNTER",			impulseCounter);

	//
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:PHRASE_SEND",		impulsePhraseSend);
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:STRING_RESP",		impulseStringResp);
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:RELOAD_CACHE",		impulseReloadCache);
	//
	GenericMsgHeaderMngr.setCallback("BOTCHAT:FORCE_END",		impulseBotChatForceEnd);

	GenericMsgHeaderMngr.setCallback("JOURNAL:INIT_COMPLETED_MISSIONS",		impulseJournalInitCompletedMissions);
	GenericMsgHeaderMngr.setCallback("JOURNAL:UPDATE_COMPLETED_MISSIONS",	impulseJournalUpdateCompletedMissions);
//	GenericMsgHeaderMngr.setCallback("JOURNAL:CANT_ABANDON",				impulseJournalCantAbandon);

	GenericMsgHeaderMngr.setCallback("JOURNAL:ADD_COMPASS",					impulseJournalAddCompass);
	GenericMsgHeaderMngr.setCallback("JOURNAL:REMOVE_COMPASS",				impulseJournalRemoveCompass);


	//GenericMsgHeaderMngr.setCallback("GUILD:SET_MEMBER_INFO",	impulseGuildSetMemberInfo);
	//GenericMsgHeaderMngr.setCallback("GUILD:INIT_MEMBER_INFO",	impulseGuildInitMemberInfo);

	GenericMsgHeaderMngr.setCallback("GUILD:JOIN_PROPOSAL", impulseGuildJoinProposal);

	GenericMsgHeaderMngr.setCallback("GUILD:ASCENSOR", impulseGuildAscensor);
	GenericMsgHeaderMngr.setCallback("GUILD:LEAVE_ASCENSOR", impulseGuildLeaveAscensor);
	GenericMsgHeaderMngr.setCallback("GUILD:ABORT_CREATION", impulseGuildAbortCreation);
	GenericMsgHeaderMngr.setCallback("GUILD:OPEN_GUILD_WINDOW", impulseGuildOpenGuildWindow);

	GenericMsgHeaderMngr.setCallback("GUILD:OPEN_INVENTORY", impulseGuildOpenInventory);
	GenericMsgHeaderMngr.setCallback("GUILD:CLOSE_INVENTORY", impulseGuildCloseInventory);

	GenericMsgHeaderMngr.setCallback("GUILD:UPDATE_PLAYER_TITLE", impulseGuildUpdatePlayerTitle);
	GenericMsgHeaderMngr.setCallback("GUILD:USE_FEMALE_TITLES", impulseGuildUseFemaleTitles);
	//GenericMsgHeaderMngr.setCallback("GUILD:INVITATION", impulseGuildInvitation);

	GenericMsgHeaderMngr.setCallback("HARVEST:CLOSE_TEMP_INVENTORY", impulseCloseTempInv);

	GenericMsgHeaderMngr.setCallback("COMMAND:REMOTE_ADMIN", impulseRemoteAdmin);

	GenericMsgHeaderMngr.setCallback("PHRASE:DOWNLOAD", impulsePhraseDownLoad);
	GenericMsgHeaderMngr.setCallback("PHRASE:CONFIRM_BUY", impulsePhraseConfirmBuy);
	GenericMsgHeaderMngr.setCallback("PHRASE:EXEC_CYCLIC_ACK", impulsePhraseAckExecuteCyclic);
	GenericMsgHeaderMngr.setCallback("PHRASE:EXEC_NEXT_ACK", impulsePhraseAckExecuteNext);

	GenericMsgHeaderMngr.setCallback("ITEM_INFO:SET", impulseItemInfoSet);
	GenericMsgHeaderMngr.setCallback("ITEM_INFO:REFRESH_VERSION", impulseItemInfoRefreshVersion);
	GenericMsgHeaderMngr.setCallback("MISSION_PREREQ:SET", impulsePrereqInfoSet);
	GenericMsgHeaderMngr.setCallback("ITEM:OPEN_ROOM_INVENTORY", impulseItemOpenRoomInventory);
	GenericMsgHeaderMngr.setCallback("ITEM:CLOSE_ROOM_INVENTORY", impulseItemCloseRoomInventory);

	GenericMsgHeaderMngr.setCallback("DEATH:RESPAWN_POINT", impulseDeathRespawnPoint);
	GenericMsgHeaderMngr.setCallback("DEATH:RESPAWN",		impulseDeathRespawn);

	GenericMsgHeaderMngr.setCallback("DUEL:INVITATION",			impulseDuelInvitation);
	GenericMsgHeaderMngr.setCallback("DUEL:CANCEL_INVITATION",	impulseDuelCancelInvitation);

	GenericMsgHeaderMngr.setCallback("PVP_CHALLENGE:INVITATION",		impulsePVPChallengeInvitation);
	GenericMsgHeaderMngr.setCallback("PVP_CHALLENGE:CANCEL_INVITATION",	impulsePVPChallengeCancelInvitation);

	GenericMsgHeaderMngr.setCallback("PVP_FACTION:PUSH_FACTION_WAR",	impulsePVPFactionPushFactionWar);
	GenericMsgHeaderMngr.setCallback("PVP_FACTION:POP_FACTION_WAR",		impulsePVPFactionPopFactionWar);
	GenericMsgHeaderMngr.setCallback("PVP_FACTION:FACTION_WARS",		impulsePVPFactionFactionWars);


//	GenericMsgHeaderMngr.setCallback("PVP_VERSUS:CHOOSE_CLAN",	impulsePVPChooseClan);

	GenericMsgHeaderMngr.setCallback("ENCYCLOPEDIA:UPDATE", impulseEncyclopediaUpdate);
	GenericMsgHeaderMngr.setCallback("ENCYCLOPEDIA:INIT", impulseEncyclopediaInit);

	GenericMsgHeaderMngr.setCallback("USER:BARS", impulseUserBars);
	GenericMsgHeaderMngr.setCallback("USER:POPUP", impulseUserPopup);


	GenericMsgHeaderMngr.setCallback("MISSION:ASK_ENTER_CRITICAL", impulseEnterCrZoneProposal);
	GenericMsgHeaderMngr.setCallback("MISSION:CLOSE_ENTER_CRITICAL", impulseCloseEnterCrZoneProposal);

	// Module gateway message
	GenericMsgHeaderMngr.setCallback( "MODULE_GATEWAY:FEOPEN",		cbImpulsionGatewayOpen);
	GenericMsgHeaderMngr.setCallback( "MODULE_GATEWAY:GATEWAY_MSG", cbImpulsionGatewayMessage );
	GenericMsgHeaderMngr.setCallback( "MODULE_GATEWAY:FECLOSE",		cbImpulsionGatewayClose );

	GenericMsgHeaderMngr.setCallback( "OUTPOST:CHOOSE_SIDE",		impulseOutpostChooseSide );
	GenericMsgHeaderMngr.setCallback( "OUTPOST:DECLARE_WAR_ACK",	impulseOutpostDeclareWarAck );

	GenericMsgHeaderMngr.setCallback( "COMBAT:FLYING_HP_DELTA",		impulseCombatFlyingHpDelta );
	GenericMsgHeaderMngr.setCallback( "COMBAT:FLYING_TEXT_ISE",		impulseCombatFlyingTextItemSpecialEffectProc );
	GenericMsgHeaderMngr.setCallback( "COMBAT:FLYING_TEXT",			impulseCombatFlyingText );

	GenericMsgHeaderMngr.setCallback( "SEASON:SET",					impulseSetSeason );
	GenericMsgHeaderMngr.setCallback( "RING_MISSION:DSS_DOWN",		impulseDssDown );

	GenericMsgHeaderMngr.setCallback( "NPC_ICON:SET_DESC",			impulseSetNpcIconDesc );
	GenericMsgHeaderMngr.setCallback( "NPC_ICON:SVR_EVENT_MIS_AVL",	impulseServerEventForMissionAvailability );
	GenericMsgHeaderMngr.setCallback( "NPC_ICON:SET_TIMER",			impulseSetNpcIconTimer );
}


//-----------------------------------------------
// impulseCallBack :
// The impulse callback to receive all msg from the frontend.
//-----------------------------------------------
void impulseCallBack(NLMISC::CBitMemStream &impulse, sint32 packet, void *arg)
{
	GenericMsgHeaderMngr.execute(impulse);
}


////////////
// METHOD //
////////////
//-----------------------------------------------
// CNetManager :
// Constructor.
//-----------------------------------------------
CNetManager::CNetManager() : CNetworkConnection()
{
#ifdef ENABLE_INCOMING_MSG_RECORDER
	_IsReplayStarting = false;
#endif
}// CNetManager //

//-----------------------------------------------
// update :
// Updates the whole connection with the frontend.
// Call this method evently.
// \return bool : 'true' if data were sent/received.
//-----------------------------------------------
bool CNetManager::update()
{
	H_AUTO_USE ( RZ_Client_Net_Mngr_Update )

#ifdef ENABLE_INCOMING_MSG_RECORDER
	if(_IsReplayStarting)
		return;
#endif

	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
	{
		// Init
		if(_CurrentServerTick == 0)
		{
			if(T1 >= _LCT)
			{
				_MachineTimeAtTick = T1;
				_CurrentClientTime = _MachineTimeAtTick - _LCT;
				_CurrentClientTick = 0;
				_CurrentServerTick = 10;
			}

			return false;
		}

		if((T1 - _MachineTimeAtTick) >= _MsPerTick)
		{
			NLMISC::TGameCycle nbTick = (NLMISC::TGameCycle)((T1 - _MachineTimeAtTick)/_MsPerTick);
			_CurrentClientTick += nbTick;
			_CurrentServerTick += nbTick;
			_MachineTimeAtTick += nbTick*_MsPerTick;
		}

		// update the smooth server tick for debug
		CNetworkConnection::updateSmoothServerTick();

		// emulation done
#ifdef ENABLE_INCOMING_MSG_RECORDER
		return false;
#endif
	}

	// Update the base class.
	bool result = CNetworkConnection::update();
	// Get changes with the update.
	const vector<CChange> &changes = NetMngr.getChanges();

	// Manage changes
	vector<CChange>::const_iterator it;
	for(it = changes.begin(); it < changes.end(); ++it)
	{
		const CChange &change = *it;
		// Update a property.
		if(change.Property < AddNewEntity)
		{
			if (!IgnoreEntityDbUpdates  || change.ShortId == 0)
			{
				// Update the visual property for the slot.
				EntitiesMngr.updateVisualProperty(change.GameCycle, change.ShortId, change.Property, change.PositionInfo.PredictedInterval);
			}
			else
			{
				nlwarning("CNetManager::update : Skipping EntitiesMngr.updateVisualProperty() because IgnoreEntityDbUpdates=%s and change.ShortId=%d", (IgnoreEntityDbUpdates?"true":"false"), change.ShortId);
			}
		}
		// Add New Entity (and remove the old one in the slot).
		else if(change.Property == AddNewEntity)
		{
			if (!IgnoreEntityDbUpdates  || change.ShortId == 0)
			{
				// Remove the old entity.
				EntitiesMngr.remove(change.ShortId, false);
				// Create the new entity.
				if(EntitiesMngr.create(change.ShortId, get(change.ShortId), change.NewEntityInfo) == 0)
					nlwarning("CNetManager::update : entity in the slot '%u' has not been created.", change.ShortId);
			}
			else
			{
				nlwarning("CNetManager::update : Skipping EntitiesMngr.create() because IgnoreEntityDbUpdates=%s and change.ShortId=%d", (IgnoreEntityDbUpdates?"true":"false"), change.ShortId);
			}
		}
		// Delete an entity
		else if(change.Property == RemoveOldEntity)
		{
			if (!IgnoreEntityDbUpdates || change.ShortId == 0)
			{
				// Remove the old entity.
				EntitiesMngr.remove(change.ShortId, true);
			}
			else
			{
				nlwarning("CNetManager::update : Skipping EntitiesMngr.remove() because IgnoreEntityDbUpdates=%s and change.ShortId=%d", (IgnoreEntityDbUpdates?"true":"false"), change.ShortId);
			}
		}
		// Lag detected.
		else if(change.Property == LagDetected)
		{
			nldebug("CNetManager::update : Lag detected.");
		}
		// Probe received.
		else if(change.Property == ProbeReceived)
		{
			nldebug("CNetManager::update : Probe Received.");
		}
		// Connection ready.
		else if(change.Property == ConnectionReady)
		{
			nldebug("CNetManager::update : Connection Ready.");
		}
		// Property unknown.
		else
			nlwarning("CNetManager::update : The property '%d' is unknown.", change.Property);
	}
	ChatMngr.flushBuffer(InterfaceChatDisplayer);
	// Clear all changes.
	clearChanges();

	// Update data base server state
	if (IngameDbMngr.getNodePtr())
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		if (im)
		{
			CCDBNodeLeaf *node = m_PingLeaf ? &*m_PingLeaf
				: &*(m_PingLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:PING", false));
			if (node)
				node->setValue32(getPing());
			node = m_UploadLeaf ? &*m_UploadLeaf
				: &*(m_UploadLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:UPLOAD", false));
			if (node)
				node->setValue32((sint32)(getMeanUpload()*1024.f/8.f));
			node = m_DownloadLeaf ? &*m_DownloadLeaf
				: &*(m_DownloadLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:DOWNLOAD", false));
			if (node)
				node->setValue32((sint32)(getMeanDownload()*1024.f/8.f));
			node = m_PacketLostLeaf ? &* m_PacketLostLeaf
				: &*(m_PacketLostLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:PACKETLOST", false));
			if (node)
				node->setValue32((sint32)getMeanPacketLoss());
			node = m_ServerStateLeaf ? &*m_ServerStateLeaf
				: &*(m_ServerStateLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:SERVERSTATE", false));
			if (node)
				node->setValue32((sint32)getConnectionState());
			node = m_ConnectionQualityLeaf ? &*m_ConnectionQualityLeaf
				: &*(m_ConnectionQualityLeaf = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CONNECTION_QUALITY", false));
			if (node)
				node->setValue32((sint32)getConnectionQuality());
		}
	}

	// Return 'true' if data were sent/received.
	return result;


}// update //

//-----------------------------------------------
// getConnectionQuality :
//-----------------------------------------------
bool CNetManager::getConnectionQuality()
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
		return true;

	return CNetworkConnection::getConnectionQuality();
}// getConnectionQuality //


/**
 * Buffers a bitmemstream, that will be converted into a generic action, to be sent later to the server (at next update).
 */
void CNetManager::push(NLMISC::CBitMemStream &msg)
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
		return;

	if (PermanentlyBanned) return;

	CNetworkConnection::push(msg);
}

/**
 * Buffers a target action
 */
void CNetManager::pushTarget(CLFECOMMON::TCLEntityId slot)
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
	{
		if(UserEntity->mode() != MBEHAV::COMBAT
		&& UserEntity->mode() != MBEHAV::COMBAT_FLOAT)
		{
			UserEntity->targetSlot(slot);
		}
		return;
	}

	CNetworkConnection::pushTarget(slot, LHSTATE::NONE);
}


/**
 * Buffers a pick-up action
 */
void CNetManager::pushPickup(CLFECOMMON::TCLEntityId slot, LHSTATE::TLHState lootOrHarvest)
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
	{
		return;
	}

	CNetworkConnection::pushTarget(slot, lootOrHarvest);
}


/**
 * Send
 */
void CNetManager::send(NLMISC::TGameCycle gameCycle)
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
		return;

	// wait till next server is received
	if (_LastSentCycle >= gameCycle)
	{
		//nlinfo ("Try to CNetManager::send(%d) _LastSentCycle=%d more than one time with the same game cycle, so we wait new game cycle to send", gameCycle, _LastSentCycle);
		while (_LastSentCycle >= gameCycle)
		{
			// Update network.
			update();
			// Send dummy info
			send();
			// Do not take all the CPU.
			nlSleep(100);

			gameCycle = getCurrentServerTick();
		}
	}

	CNetworkConnection::send(gameCycle);
}

/**
 * Send
 */
void CNetManager::send()
{
	// If the client is in Local Mode -> no network.
	if(ClientCfg.Local)
		return;

	CNetworkConnection::send();
}

/**
 * Disconnects the current connection
 */
void CNetManager::disconnect()
{
	// If the client is in Local Mode -> no need to disconnect.
	if(ClientCfg.Local)
		return;

	CNetworkConnection::disconnect();
}// disconnect //


/**
 * Reset data and init the socket
 */
void CNetManager::reinit()
{
	if(ClientCfg.Local)
		return;

	IngameDbMngr.resetInitState();
	CNetworkConnection::reinit();
}

void CNetManager::waitForServer()
{
	sint	LastGameCycle = getCurrentServerTick();

	for(;;)
	{
		// Event server get events
		CInputHandlerManager::getInstance()->pumpEventsNoIM();
		// Update Network.
		update();

		if (LastGameCycle != (sint) getCurrentServerTick())
			break;

		nlSleep(100);
		send();
	}

}// waitForServer //


#ifdef ENABLE_INCOMING_MSG_RECORDER
//-----------------------------------------------
// setReplayingMode :
//-----------------------------------------------
void CNetManager::setReplayingMode( bool onOff, const std::string& filename )
{
	CNetworkConnection::setReplayingMode(onOff, filename);
	_IsReplayStarting = onOff;
}// setReplayingMode //

//-----------------------------------------------
// startReplay :
//-----------------------------------------------
void CNetManager::startReplay()
{
	// Init Replay
	_MachineTimeAtTick = T1;
	if(_MachineTimeAtTick >= _LCT)
		_CurrentClientTime = _MachineTimeAtTick - _LCT;
	else
		_CurrentClientTime = 0;
	// Replay now in progress.
	_IsReplayStarting = false;
}// startReplay //
#endif


/*
 * Create the net managers in CLIENT_MULTI mode
 */
void	CNetManagerMulti::init( const std::string& cookie, const std::string& addr )
{
	uint nb, baseCookie;
	NLMISC::CConfigFile::CVar *var = ClientCfg.ConfigFile.getVarPtr( "NbConnections" );
	if ( var )
		nb = var->asInt();
	else
		nb = 1;
	var = ClientCfg.ConfigFile.getVarPtr( "UserId" );
	if ( var )
		baseCookie = var->asInt();
	else
		baseCookie = 0;
	std::vector<std::string> fsAddrs;
	fsAddrs.push_back( addr );
	string portString = addr.substr( addr.find( ':' ) );
	var = ClientCfg.ConfigFile.getVarPtr( "AdditionalFSList" );
	if ( var )
	{
		for ( uint i=0; i!=var->size(); ++i )
			fsAddrs.push_back( var->asString( i ) + portString );
	}
	nlinfo( "CNetManagerMulti: Creating %u connections to %u front-ends, baseCookie=%u...", nb, fsAddrs.size(), baseCookie );

	for ( uint i=0; i!=nb; ++i )
	{
		CNetManager *nm = new CNetManager();
		string multicook = NLMISC::toString( "%8x|%8x|%8x", 0, 0, baseCookie + i );
		nm->init( multicook, fsAddrs[i % fsAddrs.size()] );
		_NetManagers.push_back( nm );
	}
}

//
uint32					ShardId = 0;
std::string				WebServer = "";




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////// COMMANDS after should NOT appear IN the FINAL VERSION ///////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


#if !FINAL_VERSION

// temp : simulate a team msg in local mode
NLMISC_COMMAND(localTellTeam, "Temp : simulate a tell in local mode", "<people_name> <msg>")
{
	if (args.empty()) return false;
	ucstring player = args[0];
	std::string msg;
	if (args.size() >= 2)
	{
		msg = args[1];
		for(uint k = 2; k < args.size(); ++k)
		{
			msg += " " + args[k];
		}
	}
	TDataSetIndex dsi = INVALID_DATASET_INDEX;
	InterfaceChatDisplayer.displayChat(dsi, ucstring(msg), ucstring(msg), CChatGroup::team, NLMISC::CEntityId::Unknown, player);
	return true;
}

// temp : simulate a tell in local mode
NLMISC_COMMAND(localTell, "Temp : simulate a tell in local mode", "<people_name> <msg>")
{
	if (args.empty()) return false;
	ucstring player = args[0];
	std::string msg;
	if (args.size() >= 2)
	{
		msg = args[1];
		for(uint k = 2; k < args.size(); ++k)
		{
			msg += " " + args[k];
		}
	}
//	TDataSetIndex dsi = INVALID_DATASET_ROW;
	InterfaceChatDisplayer.displayTell(/*dsi, */ucstring(msg), player);
	return true;
}

NLMISC_COMMAND(testDynChatOpen, "", "")
{
	NLMISC::CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	uint32 BotUID = 22;			// Compressed Index
	uint32 BotName = 654;			// Server string
	vector<uint32> DynStrs; // 0 - Desc, 1 - Option0, 2 - Option1, etc....
	DynStrs.push_back(16540);
	DynStrs.push_back(11465);
	DynStrs.push_back(12654);
	bm.serial(BotUID);
	bm.serial(BotName);
	bm.serialCont(DynStrs);
	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	impulseDynChatOpen(bm);
	return true;
}

NLMISC_COMMAND(testDynChatClose, "", "")
{
	NLMISC::CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	uint32 BotUID = 22;			// Compressed Index
	bm.serial(BotUID);
	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	impulseDynChatClose(bm);
	return true;
}


NLMISC_COMMAND(testCloseTempInv, "","")
{
	NLMISC::CBitMemStream bm;
	impulseCloseTempInv(bm);
	return true;
}

NLMISC_COMMAND(testTeamInvite, "","")
{
	NLMISC::CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	uint32 index = 10;
	bm.serial(index);
	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	impulseTeamInvitation(bm);
	return true;
}
NLMISC_COMMAND(testGuildInvite, "","")
{
	NLMISC::CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	uint32 index = 10;
	bm.serial(index);
	bm.serial(index);
	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	impulseGuildJoinProposal(bm);
	return true;
}

NLMISC_COMMAND( testExchangeInvitation, "Test the modal window for invitation exchange", "" )
{
	CBitMemStream impulse;
	uint32 nameIndex = 0;
	impulse.serial(nameIndex);
	impulse.invert();
	impulseExchangeInvitation(impulse);
	return true;
}


NLMISC_COMMAND(testAscensor, "Temp : Simulate a GUILD:ASCENSOR message coming from server","")
{
	NLMISC::CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	uint32 index = 10;
	bm.serial(index);
	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	impulseGuildAscensor(bm);
	return true;
}

NLMISC_COMMAND(testDuelInvite, "","")
{
	NLMISC::CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	uint32 index = 10;
	bm.serial(index);
	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	impulseDuelInvitation(bm);
	return true;
}

//NLMISC_COMMAND(receiveId, "","<num> <name>")
//{
//	uint32 index;
//	fromString(args[0], index);
//	ucstring ucstr = args[1];
//
//	vector<bool> code;
//
//#ifdef OLD_STRING_SYSTEM
//	ChatMngr.getDynamicDB().add( index, ucstr, code );
//#else
//	// TRAP // WE MUST NEVER CALL THIS COMMAND ANYMORE : ALL IS HANDLED BY STRING_MANAGER NOW !!!
//	nlstop;
//#endif
//
//	return true;
//}

NLMISC_COMMAND(testOutpostChooseSide, "","b b u32 u32")
{
	if(args.size()<4)
		return false;
	NLMISC::CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	bool playerGuildInConflict;
	fromString(args[0], playerGuildInConflict);
	bool playerGuildIsAttacker;
	fromString(args[1], playerGuildIsAttacker);
	bm.serial(playerGuildInConflict);
	bm.serial(playerGuildIsAttacker);
	uint32 ownerGuildNameId;
	fromString(args[2], ownerGuildNameId);
	bm.serial( ownerGuildNameId );
	uint32 attackerGuildNameId;
	fromString(args[3], attackerGuildNameId);
	bm.serial( attackerGuildNameId );
	uint32 declTimer= 100;
	bm.serial( declTimer );

	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	impulseOutpostChooseSide(bm);
	return true;
}

NLMISC_COMMAND(testUserPopup, "","u32 u32")
{
	if(args.size()<2)
		return false;
	NLMISC::CBitMemStream bm;
	if (bm.isReading()) bm.invert();
	uint32 titleId;
	fromString(args[0], titleId);
	bm.serial( titleId );
	uint32 textId;
	fromString(args[1], textId);
	bm.serial( textId );

	bm.invert();
	bm.seek(0, NLMISC::IStream::begin);
	impulseUserPopup(bm);
	return true;
}


#endif

