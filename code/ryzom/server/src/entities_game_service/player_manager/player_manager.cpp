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

#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/command.h"
#include "nel/misc/eid_translator.h"

#include "nel/net/service.h"
#include "nel/net/message.h"

#include "server_share/mail_forum_validator.h"
#include "game_share/send_chat.h"

//#include "../../pd_lib/pd_string_manager.h"

#include "team_manager/team_manager.h"
#include "player_manager/player_manager.h"
#include "mission_manager/mission_manager.h"
#include "mission_manager/mission_queue_manager.h"
#include "entity_manager/entity_callbacks.h"
#include "phrase_manager/phrase_manager.h"
#include "guild_manager/fame_manager.h"
#include "guild_manager/guild_manager.h"
#include "building_manager/building_manager.h"
#include "egs_pd.h"
#include "progression/progression_pve.h"
#include "progression/progression_pvp.h"
#include "pvp_manager/pvp_manager.h"
#include "world_instances.h"
#include "player_manager/character.h"
#include "player_manager/player.h"
#include "dyn_chat_egs.h"
#include "player_manager/admin_properties.h"
#include "player_manager/character_game_event.h"
#include "modules/shard_unifier_client.h"
#include "modules/r2_mission_item.h"
#include "modules/animation_session_manager.h"
#include "entities_game_service.h"


using namespace NLMISC;
using namespace NLNET;
using namespace std;

extern bool	IOSIsUp;

extern CTeamManager					TeamManager;
extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern CVariable<string>	NoForceDisconnectPriv;
extern CVariable<string>	NeverAggroPriv;
extern CVariable<string>	AlwaysInvisiblePriv;
extern CVariable<uint32>	MonkeyLoadEnable;

extern CPlayerService * PS;

static void	mailNotification(const std::string& to, const std::string& from);

typedef std::multiset<uint16>					TUInt16Multiset;


IPlayerManager *IPlayerManager::_Instance = NULL;


// filter for report BS down
bool ReportBSIsDown = true;

/**
 * This callback is called when a asynchronous character load is ready to be done.
 * The callback returns true if pdr loading and data extraction succeded, false otherwise.
 * If failed, the engine will try to load another file, until it succeed or there is no more file to load.
 */
typedef bool		(*TCharLoadCallback)(uint32 userId, uint charId, CPersistentDataRecord& pdr);
typedef bool		(*TUserReadyCallback)(uint32 userId);

class CPersistentPlayerLoader : public IBackupServiceConnection
{
public:

	CPersistentPlayerLoader()
	{
		_CharLoadCb = NULL;
		_UserReadyCb = NULL;
	}

	struct CPersistentPlayer;

//	virtual void	cbBSconnect(bool connecting);

	/// Called when BS answer the characters file list, and fills persistent player struct
//	void	buildPlayerList(const CFileDescriptionContainer& fileList);
	/// Start Asynchronous character loading
//	void	startCharLoading(CPersistentPlayer* player, uint charId);
	/// Called when asynchronous char loading is finished
//	void	loadPlayerChar(CPersistentPlayer* player, uint charId, const CFileDescription& fileDescription, NLMISC::IStream& dataStream);

	void	setCharReady(CPersistentPlayer* player, uint charId);
	void	checkPlayerReady(CPersistentPlayer* player);


	struct CPersistentChar
	{
		CPersistentChar() : Ready(true)	{}

		bool						Ready;
		std::vector<std::string>	FileList;
	};

	struct CPersistentPlayer
	{
		CPersistentPlayer() : Ready(false)	{}

		uint32							UserId;
		bool							Ready;
		std::vector<CPersistentChar>	Chars;
	};

//	class CClassCallback : public IBackupFileClassReceiveCallback
//	{
//	public:
//		CClassCallback(CPersistentPlayerLoader* load) : Load(load)	{}
//		CPersistentPlayerLoader*	Load;
//
//		void	callback(const CFileDescriptionContainer& fileList)
//		{
//			if (Load)
//			{
//				Load->buildPlayerList(fileList);
//				Load = NULL;
//			}
//		}
//	};

//	class CFileCallback : public IBackupFileReceiveCallback
//	{
//	public:
//		CFileCallback(CPersistentPlayerLoader* load, CPersistentPlayer* player, uint charId) : Load(load), Player(player), CharId(charId)	{}
//		CPersistentPlayer*			Player;
//		uint						CharId;
//		CPersistentPlayerLoader*	Load;
//		void				callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
//		{
//			if (Load)
//			{
//				Load->loadPlayerChar(Player, CharId, fileDescription, dataStream);
//				Load = NULL;
//			}
//		}
//	};

	TCharLoadCallback	_CharLoadCb;
	TUserReadyCallback	_UserReadyCb;

	typedef std::map<uint32, CPersistentPlayer>	TPPlayerMap;
	TPPlayerMap			_Players;
};


//void	CPersistentPlayerLoader::cbBSconnect(bool connecting)
//{
//	// load all character files if BS up
//	if (!connecting)
//		return;
//
//	std::vector<CBackupFileClass>	classes;
//
//	CBackupFileClass	classOk;
////	classOk.Patterns.push_back("account_*_*_pdr.xml");
//	classOk.Patterns.push_back("account_*_*_pdr.bin");
//	classes.push_back(classOk);
//
////	CBackupFileClass	classBackup;
////	classBackup.Patterns.push_back("account_*_*_pdr.xml_backup");
////	classBackup.Patterns.push_back("account_*_*_pdr.bin_backup");
////	classes.push_back(classBackup);
//
//	nlinfo("BSIF: requesting file class...");
//	BsiGlobal.requestFileClass("characters/", classes, new CClassCallback(this));
//}





//
//void	CPersistentPlayerLoader::buildPlayerList(const CFileDescriptionContainer& fileList)
//{
//	_Players.clear();
//
//	for (uint i=0; i<fileList.size(); ++i)
//	{
//		const CFileDescription&	fd = fileList[i];
//
//		uint	userId, charId;
//
//		if (sscanf(fd.FileName.c_str(), "account_%d_%d_", &userId, &charId) != 2)
//			continue;
//
//		CPersistentPlayer&	player = _Players[userId];
//
//		player.UserId = userId;
//		player.Ready = false;
//
//		if (player.Chars.size() <= charId)
//			player.Chars.resize(charId+1);
//
//		player.Chars[charId].Ready = false;
//		player.Chars[charId].FileList.push_back(fd.FileName);
//	}
//
//	TPPlayerMap::iterator	it;
//	for (it=_Players.begin(); it!=_Players.end(); ++it)
//	{
//		CPersistentPlayer&	player = (*it).second;
//
//		for (uint i=0; i<player.Chars.size(); ++i)
//		{
//			if (!player.Chars[i].Ready)
//			{
//				startCharLoading(&player, i);
//			}
//		}
//
//		checkPlayerReady(&player);
//	}
//}

//
//void	CPersistentPlayerLoader::startCharLoading(CPersistentPlayer* player, uint charId)
//{
//	// no file to load, assumes no character can be loaded
//	if (player->Chars[charId].FileList.empty())
//	{
//		setCharReady(player, charId);
//		return;
//	}
//
//	// pop next file to load and start loading with this file
//	nlinfo("BSIF: requesting file...");
//	BsiGlobal.requestFile(player->Chars[charId].FileList.front(), new CFileCallback(this, player, charId));
//	player->Chars[charId].FileList.erase(player->Chars[charId].FileList.begin());
//}

//
//void	CPersistentPlayerLoader::loadPlayerChar(CPersistentPlayer* player, uint charId, const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
//{
//	if (player->Chars.size() <= charId)
//		return;
//
//	/// \todo: try pdr parsing + char user loading
//	// if success, mark char as ready, eval if player is ready
//	// if failed, try to load next file for this char
//
//	NLMISC::CMemStream&	memStream = dynamic_cast<NLMISC::CMemStream&>(dataStream);
//	if (&memStream != NULL)
//	{
//		static CPersistentDataRecord	pdr;
//		pdr.clear();
//		bool					success = false;
//
//		try
//		{
//			if (pdr.fromBuffer(memStream))
//			{
//				success = (_CharLoadCb == NULL || _CharLoadCb(player->UserId, charId, pdr));
//			}
//		}
//		catch (const Exception& e)
//		{
//			nlwarning("Failed to load user '%d' character '%d' from BS pdr stream: %s", player->UserId, charId, e.what());
//		}
//		catch (...)
//		{
//			nlwarning("Failed to load user '%d' character '%d' from BS pdr stream: low level exception", player->UserId, charId);
//		}
//
//		if (!success)
//		{
//			startCharLoading(player, charId);
//			return;
//		}
//	}
//	else
//	{
//		// don't wipe here, internal issue
//		nlwarning("Failed to load Character '%d' from BS pdr stream: stream is not a CMemStream", charId);
//	}
//
//	setCharReady(player, charId);
//}

//
void	CPersistentPlayerLoader::setCharReady(CPersistentPlayer* player, uint charId)
{
	if (player->Chars.size() > charId)
		player->Chars[charId].Ready = true;

	checkPlayerReady(player);
}

//
void	CPersistentPlayerLoader::checkPlayerReady(CPersistentPlayer* player)
{
	if (player->Ready)
		return;

	// check all chars are ready
	for (uint i=0; i<player->Chars.size(); ++i)
		if (!player->Chars[i].Ready)
			return;

	player->Ready = true;

	// player is ready, call callback
	if (_UserReadyCb)
		_UserReadyCb(player->UserId);

	if( MonkeyLoadEnable > 0)
		PS->egsAddMonkeyPlayerCallback(player->UserId);
}



std::string	EGSAdditionalPlayerInfo(const NLMISC::CEntityId& eid)
{
	uint32	userId = uint32(eid.getShortId() >> 4);
	uint32	charId = uint32(eid.getShortId() & 0xf);

	std::string	addinfo = toString("SaveFile %saccount_%d_%d_pdr.bin", PlayerManager.getCharacterPath(userId, true).c_str(), userId, charId);

	// if the character is online, add some interesting info
	ICharacter *ic = ICharacter::getInterface(eid, true);
	if (ic != NULL)
	{
		// ok, this is an online character
		const CCharacter *character = ic->getCharacter();
		uint32 sessionId = 0;
		// retrieve the session id from the position stack
		if (!character->getPositionStack().empty())
		{
			sessionId = character->getPositionStack().top().SessionId;
			if (sessionId == SessionLockPositionStack)
				sessionId = character->currentSessionId();
		}

		addinfo += toString(" Pos: %i,%i,%i Session: %u", character->getX()/1000, character->getY()/1000, character->getZ()/1000, sessionId);
	}
	return addinfo;
}



//-----------------------------------------------
// cbSetCharacterAIInstance: callback for SET_CHAR_AIINSTANCE message
//-----------------------------------------------
void cbSetCharacterAIInstance( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	H_AUTO(cbClientTpAck);
	
	CEntityId eid;
	TSessionId sessionId;
	
	msgin.serial( eid );
	msgin.serial( sessionId );
		
	uint32 playerId = PlayerManager.getPlayerId(eid);
	uint8 charIndex = PlayerManager.getCharIndex( eid );
	CCharacter *ch = PlayerManager.getChar( playerId, charIndex );
	if ( ch )
	{
		// if this is a ring shard then we need to make sure that the player gets put back into their AIInstance
		// before executing any code that can provoke a return ... otherwise the player can be stuck in '-1' indefinitely
		if (IsRingShard)
		{
			// we don't look at continent if we are in a ring shard
			ch->setRingShardInstanceNumber(sessionId.asInt());
			ch->setStartupInstance(sessionId.asInt());
		}		
	}
}


//---------------------------------------------------
// addClientCallback: Add callback for client & characters management
//
//---------------------------------------------------
void CPlayerManager::addClientCallback()
{
	//array of callback items
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "CL_CONNECT",				cbClientConnection		}, 
		{ "CL_DISCONNECT",			cbClientDisconnection	},
//		{ "CLIENT:CONNECTION:ENTER",cbClientEnter			},
		{ "RDY",					cbClientReady			},
		{ "SELECT_CHAR",			cbSelectChar			}, 
		{ "CHECK_NAME",				cbCheckName				},
		{ "CREATE_CHAR",			cbCreateChar			},
		{ "CLIENT:CONNECTION:DELETE_CHAR", cbDeleteChar		},

		{ "ITEM_DROP",				cbItemDrop				}, 
		{ "ITEM_PICK_UP",			cbItemPickup			}, 
		{ "ITEM_CLOSE_PICK_UP",		cbItemClosePickup		},
		{ "ITEM_SWAP",				cbItemSwap				}, 
		
		{ "PACK_ANIMAL_COMMAND",	cbAnimalCommand			},
		
//		{ "TRADE_LIST",				cbTradeListReceived		},//deprecated
//		{ "TRADE_CHANGE_PAGE",		cbTradePageSelect		},//deprecated
		{ "TRADE_BUY_SOMETHING",	cbTradeBuySomething		},
//		{ "GIVE_SEED",				cbGiveSeed				},

		{ "JOIN_TEAM",				cbJoinTeam				},
		{ "LEAVE_TEAM",				cbLeaveTeam				},
		{ "JOIN_TEAM_PROPOSAL",		cbJoinTeamProposal		},
		{ "JOIN_TEAM_DECLINE",		cbJoinTeamDecline		},
		{ "KICK_TEAMMATE",			cbKickTeammate			},
									
		{ "HARVEST",				cbHarvest				}, //use (from client)
		{ "HARVEST_CLOSE",			cbHarvestClose			}, //use (from client)
									
//		{ "HARVEST_RESULT",			cbHarvestResult			}, //deprecated

//		{ "HARVEST_MP_DESTOYED",	cbHarvestMPDestroyed	}, //deprecated
//		{ "HARVEST_INTERRUPTED",	cbHarvestInterrupted	}, //deprecated
									
//		{ "HARVEST_DB",				cbHarvestDB				}, //deprecated
//		{ "HARVEST_DB_UPDATE_QTY",	cbHarvestDBUpdateQty	}, //deprecated
//		{ "CLEAR_HARVEST_DB",		cbClearHarvestDB		}, //deprecated
				
//		{ "CREATE_ITEM_IN_BAG",		cbCreateItemInBag		},

		{ "ADD_SURVIVE_PACT",		cbAddSurvivePact		},
		
		{ "EXCHANGE_PROPOSAL",				cbExchangeProposal			},
		{ "EXCHANGE_ACCEPT_INVITATION",		cbAcceptExchangeInvitation	},
		{ "EXCHANGE_DECLINE_INVITATION",	cbDeclineExchangeInvitation	},
		{ "EXCHANGE_ACCEPT",				cbAcceptExchange			},
		{ "EXCHANGE_END",					cbEndExchange				},
		{ "EXCHANGE_SEEDS",					cbExchangeSeeds				},

		{ "SET_PLAYER_WEATHER",					cbSetPlayerWeather		}, // from DSS
		{ "SET_PLAYER_SEASON",					cbSetPlayerSeason		}, // from DSS

		{ "TELEPORT_PLAYER",					cbTeleportPlayer		}, // from AIS
		
		{ "SET_CHAR_AIINSTANCE",			cbSetCharacterAIInstance},

//		{ "FAME_CHANGE",			cbFameChange			}, // deprecated
	};
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
}

//-----------------------------------------------
// Callback of transport class CPetSpawnConfirmationMsg
// 
//-----------------------------------------------
void CPetSpawnConfirmationImp::callback(const string &serviceName, NLNET::TServiceId  sid)
{
	//if ( ! Mirror.mirrorIsReady() ) // can't occur because tc is a CMirrorTransportClass
	CCharacter * c = PlayerManager.getChar( CharacterMirrorRow );
	if( c )
	{
		c->onAnimalSpawned( (CPetSpawnConfirmationMsg::TSpawnError) SpawnError, PetIdx, PetMirrorRow );
	}
}


//-----------------------------------------------
// init the manager
// 
//-----------------------------------------------
void CPlayerManager::init()
{
	CFile::createDirectory(BsiGlobal.getLocalPath() + string("characters"));

	// init broadcast message
	_RepeatBroadcastMessage = 0;
	_NextBroadcastMessage = 0;

	_EIdTranslatorReady = true; // addAllCharForStringIdRequest() must be called first

	// register entityinfo callback (so admin tool can retrieve player save file..)
	CEntityIdTranslator::getInstance()->EntityInfoCallback = EGSAdditionalPlayerInfo;

	CMailForumValidator::setNotification(mailNotification);
} // init


//---------------------------------------------------
// createCharacterId :
//
//---------------------------------------------------
CEntityId CPlayerManager::createCharacterId( uint32 userId, uint32 index )
{
	TMapPlayerFrontEndId::iterator it = _PlayerFrontEndId.find( userId );
	if( it != _PlayerFrontEndId.end() )
	{
		// create character CEntityId
		uint64 id = (userId<<4)|index;

		CEntityId charId(RYZOMID::player,id);
		charId.setDynamicId( NLNET::TServiceId8(it->second).get());
		charId.setCreatorId( 0 );
		return charId;
	}
	else
	{
		nlwarning("(EGS)<createCharacterId> the front id for player %d is unknown",userId);
	}
	return CEntityId::Unknown;
} // createCharacterId //


//---------------------------------------------------
// setPlayerFrontEndId :
//
//---------------------------------------------------
void CPlayerManager::setPlayerFrontEndId( uint32 userId, NLNET::TServiceId serviceId )
{
	TMapPlayerFrontEndId::iterator it = _PlayerFrontEndId.find( userId );
	if( it == _PlayerFrontEndId.end() )
	{
		_PlayerFrontEndId.insert( make_pair(userId,serviceId) );
	}
	else
	{
		// remove the player from its previous front end if any
		TMapServiceIdToUint32Set::iterator	itFe = _MapFrontendToPlayers.find( (*it).second );
		if ( itFe != _MapFrontendToPlayers.end() )
		{
			(*itFe).second.erase( userId );
		}

		(*it).second = serviceId;

		nlwarning("(EGS)<setPlayerFrontEndId> the front id for player %d has already been set",userId);
	}

	TMapServiceIdToUint32Set::iterator	itFe = _MapFrontendToPlayers.find( serviceId );
	if ( itFe != _MapFrontendToPlayers.end() )
	{
		(*itFe).second.insert( userId );
	}
	else
	{
		TUint32Set set;
		set.insert( userId );

		_MapFrontendToPlayers.insert( make_pair(serviceId, set) );
	}

} // setPlayerFrontEndId //

//---------------------------------------------------
// getPlayerFrontEndId :
//
//---------------------------------------------------
TServiceId CPlayerManager::getPlayerFrontEndId( uint32 userId )
{
	TMapPlayerFrontEndId::iterator it = _PlayerFrontEndId.find( userId );
	if( it != _PlayerFrontEndId.end() )
	{
		return (*it).second;
	}
	else
	{
		throw EPlayer( userId );
	}

} // getPlayerFrontEndId //


// if returnRemotePath is true, returns the remote path to send to the Backup Service, otherwise returns the local path
std::string CPlayerManager::getCharacterPath(uint32 userId, bool returnRemotePath)
{
	// spread in 1000 folder
	return (returnRemotePath ? BsiGlobal.getRemotePath() : BsiGlobal.getLocalPath())+toString("characters/%03u/", userId%1000);
}

std::string CPlayerManager::getOfflineCommandPath(uint32 userId, bool returnRemotePath)
{
	// spread in 1000 folder
	return (returnRemotePath ? BsiGlobal.getRemotePath() : BsiGlobal.getLocalPath())+toString("characters_offline_commands/%03u/", userId%1000);
}


//---------------------------------------------------
// loadPlayer :
//
//---------------------------------------------------
void CPlayerManager::loadPlayer( CPlayer * player )
{
	H_AUTO(CPlayerManagerLoadPlayer);
	nlassert(player);
	
	if (PDRLoad)
	{
		player->loadAllCharactersPdr();
	}
	else
	{
		player->loadAllCharacters();
	}

}


/*
 * Async Load player infos
 */
void	CPlayerManager::asyncLoadPlayer( CPlayer* player, uint32 userId, const std::string& languageId, const NLNET::CLoginCookie& cookie, bool allAuthorized )
{
	TAsyncLoadMap::iterator	it = _LoadingPlayers.find(userId);

	if (it != _LoadingPlayers.end())
	{
		nlwarning("<CPlayerManager::asyncLoadPlayer>: player '%d' is already loading, disconnected before previous loading success?", userId);

		(*it).second->clear();
	}
	else
	{
		it = _LoadingPlayers.insert(TAsyncLoadMap::value_type(userId, new CAsyncPlayerLoad())).first;
	}

	CAsyncPlayerLoad*	load = (*it).second;

	load->Player = player;
	load->UserId = userId;
	load->LanguageId = languageId;
	load->LoginCookie = cookie;
	load->AllAuthorized = allAuthorized;

	load->startLoading();
}


/*
 * Cancel Async Loading of a player infos
 */
void	CPlayerManager::cancelAsyncLoadPlayer( uint32 userId )
{
	TAsyncLoadMap::iterator	it = _LoadingPlayers.find(userId);

	if (it == _LoadingPlayers.end())
		return;

	CAsyncPlayerLoad*	ptr = (*it).second;
	ptr->Player = NULL;

	(*it).second = NULL;

	_LoadingPlayers.erase(it);
}



//---------------------------------------------------
// savePlayer :
//
//---------------------------------------------------
void CPlayerManager::savePlayerCharRecurs( uint32 userId, sint32 idx, std::set<CCharacter*> &charAlreadySaved, const std::string *filename)
{
	H_AUTO(CPlayerManagerSavePlayer);

	// *** Get the character to save
	// get the player
	TMapPlayers::iterator itPlayer = _Players.find( userId );	
	if( itPlayer == _Players.end() )
	{
		nlwarning("(EGS)<CPlayerManager::savePlayer>  :  Player with userId %u not exist in CPlayer", userId);
		return;
	}

	CPlayer * player = (*itPlayer).second.Player;
	nlassert( player );
	// dont save offline players
	if ( idx == -1 )
		return;
	CCharacter * c = player->getCharacter(idx);
	if ( !c )
	{
		nlwarning("<SAVE> Invalid active player for user %u",userId);
		return;
	}

	// if the character was already saved in the recurs call, abort
	if(charAlreadySaved.find(c)!=charAlreadySaved.end())
		return;


	// *** Save
	string fileNameBase;
	if(filename)
	{
		fileNameBase= *filename;
	}
	else
	{
		fileNameBase= NLMISC::toString( "account_%u_%d",userId, idx );
	}
	string savePath= _Stall? "": PlayerManager.getCharacterPath(userId, true);
	string fileExt= XMLSave? ".xml": ".bin";

	string serialFileName= savePath+ fileNameBase+ ".bin";
	string pdrFileName= savePath+ fileNameBase+ "_pdr"+ fileExt;

	if( UseBS && (_Stall == false) )
	{
		if (SerialSave || !PDRSave)
		{
			// perform a 'serial' save
			CMemStream f;
			try
			{
				{
					H_AUTO(SavePlayerSerialBS);
					(*itPlayer).second.Player->saveCharacter(f,idx);
				}
				CBackupMsgSaveFile msg( serialFileName, CBackupMsgSaveFile::SaveFile, BsiGlobal );
				{					
					H_AUTO(SavePlayerMakeMsgBS);
					msg.DataMsg.serialBuffer((uint8*)f.buffer(), f.length());
				}
				{
					H_AUTO(SavePlayerSendMessageBS);
					BsiGlobal.sendFile( msg );
				}
			}
			catch(const Exception &)
			{
				nlwarning("(EGS)<CPlayerManager::savePlayer>  :  Can't serial file %s (connection with BS service down ?)",serialFileName.c_str());
				return;
			}
		}

		if (PDRSave)
		{
			// perform a 'pdr' save
			CMemStream f;
			static CPersistentDataRecordRyzomStore	pdr;
			pdr.clear();
			
			try
			{
				{
					H_AUTO(SavePlayerPDRStore);
					(*itPlayer).second.Player->storeCharacter(pdr,idx);
				}
				CBackupMsgSaveFile msg( pdrFileName, CBackupMsgSaveFile::SaveFile, BsiGlobal );
				{					
					if (XMLSave)
					{
						H_AUTO(SavePlayerPDRMakeTxtMsgBS);
						std::string s;
						pdr.toString(s);
						msg.DataMsg.serialBuffer((uint8*)&s[0], (uint)s.size());
					}
					else
					{
						H_AUTO(SavePlayerPDRMakeBinMsgBS);					
						uint32 bufSize= pdr.totalDataSize();
						vector<char> buffer;
						buffer.resize(bufSize);
						pdr.toBuffer(&buffer[0],bufSize);
						msg.DataMsg.serialBuffer((uint8*)&buffer[0], bufSize);
					}
				}
				{
					H_AUTO(SavePlayerSendMessageBS);
					BsiGlobal.sendFile( msg );
				}
			}
			catch(const Exception &)
			{
				nlwarning("(EGS)<CPlayerManager::savePlayer>  :  Can't serial file %s (connection with BS service down ?)",pdrFileName.c_str());
				return;
			}
		}

		SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
		params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
		CCharacter::sendDynamicSystemMessage( c->getId(), "CHARACTER_SAVED",params );
	}
	else
	{
		if (SerialSave || !PDRSave)
		{
			COFile f;
			string serialPathFileName = BsiGlobal.getLocalPath() + serialFileName;
			if ( !f.open(serialPathFileName) )
			{
				nlwarning("(EGS)<CPlayerManager::savePlayer>  :  Can't open in write mode the file %s", serialPathFileName.c_str());
				return;
			}
			try
			{
				{
					// save player
					H_AUTO(SavePlayerSerial);
					(*itPlayer).second.Player->saveCharacter(f, idx );
				}
				{					
					// Close the File.
					H_AUTO(SavePlayerClose);
					f.close();
				}
			}
			catch(const Exception &e)
			{
				//f.close();
				nlwarning("(EGS)<CPlayerManager::savePlayer>  :  Can't write file %s : %s", serialPathFileName.c_str(), e.what());
				return;
			}
		}
		if (PDRSave)
		{
			// perform a 'pdr' save
			static CPersistentDataRecordRyzomStore	pdr;
			pdr.clear();
			string pdrPathFileName = BsiGlobal.getLocalPath() + pdrFileName;
			
			try
			{
				{
					H_AUTO(SavePlayerPDRStore);
					(*itPlayer).second.Player->storeCharacter(pdr,idx);
				}
				{
					H_AUTO(SavePlayerPDRSave);
					pdr.writeToFile(pdrPathFileName.c_str());
				}
			}
			catch(const Exception &)
			{
				nlwarning("(EGS)<CPlayerManager::savePlayer>  :  Can't serial file %s (connection with BS service down ?)", pdrPathFileName.c_str());
				return;
			}

			SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
			params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
			CCharacter::sendDynamicSystemMessage( c->getId(), "CHARACTER_SAVED",params );
		}
	}

	// the character have been saved, update the ring database
	if (IShardUnifierEvent::getInstance())
	{
		IShardUnifierEvent::getInstance()->onUpdateCharBestCombatLevel(c->getId(), max(c->getBestChildSkillValue(SKILLS::SF), c->getBestChildSkillValue(SKILLS::SMO)));
	}


	// *** Recurs save any player that must be save with me (typically any player that made an exchange with me since last save)
	// I am correctly saved, don't save me again
	charAlreadySaved.insert(c);

	// run entities that must be saved with me
	const std::set<NLMISC::CEntityId>	&entToSave= c->getEntitiesToSaveWithMe();
	std::set<NLMISC::CEntityId>::const_iterator		it= entToSave.begin();
	for(;it!=entToSave.end();++it)
	{
		// NB: we are sure that it won't be erased in the call (through clearEntitiesToSaveWithMe())
		// since savePlayerCharRecurs() will abort early if char already saved
		savePlayerCharRecurs(getPlayerId( *it ), getCharIndex(*it), charAlreadySaved);
	}

	// saved => clear the list
	c->clearEntitiesToSaveWithMe();
}


//-----------------------------------------------
// savePlayerChar
//-----------------------------------------------
void CPlayerManager::savePlayerChar( uint32 userId,sint32 idx, const std::string *filename)
{
	TTime start = NLMISC::CTime::getLocalTime();

	// save player and all players that must be saved with me (recurs)
	std::set<CCharacter*> charAlreadySaved;
	savePlayerCharRecurs( userId, idx, charAlreadySaved, filename);

	// big graph saved?
	if(charAlreadySaved.size()>=8)
	{
		TTime end = NLMISC::CTime::getLocalTime();
		sint32	dt= (sint32)(end-start);
		nlinfo( "<SAVE> Saved a big graph of %d connected player: %d ms", charAlreadySaved.size(), dt);
	}
}


//-----------------------------------------------
// savePlayerActiveChar
//-----------------------------------------------
void CPlayerManager::savePlayerActiveChar( uint32 userId, const std::string *filename )
{
	// get a ptr to the player record
	CPlayer * player = getPlayer(userId);
	if( player == 0 )
	{
		nlwarning("(EGS)<CPlayerManager::savePlayer>  :  Player with userId %u not exist in CPlayer", userId);
		return;
	}

	// save player to file
	savePlayerChar(userId, player->getActiveCharIndex(), filename);
}


//-----------------------------------------------
// Save all player
// 
//-----------------------------------------------
void CPlayerManager::saveAllPlayer()
{
	for ( TMapPlayers::iterator it =_Players.begin(); it != _Players.end(); ++it )
	{
		nlassert( (*it).second.Player );
		savePlayerActiveChar((*it).second.Player->getUserId());
/*		for ( uint i = 0; i < (*it).second.Player->getCharacterCount(); ++i )
		{
			savePlayerChar( (*it).first, (sint16)i );
		}
*/
	}
}


//-----------------------------------------------
// Save all player
// 
//-----------------------------------------------
void CPlayerManager::reloadPlayerActiveChar( uint32 userId, uint32 charIndex, const std::string *filename )
{
	if(filename)
	{
		CCharIdReplaced cir;
		cir.CharIndex = charIndex;
		cir.Filename = *filename;
		UserCharIdReplaced.insert( make_pair(userId, cir));
	}
	CCharacter * c = getActiveChar(userId);
	if(c)
	{
		c->requestFarTP( c->PositionStack.topToModify().SessionId, false );
	}
}


//---------------------------------------------------
// addPlayer :
//
//---------------------------------------------------
void CPlayerManager::addPlayer( uint32 userId, CPlayer * player )
{
	H_AUTO(CPlayerManagerAddPlayer);

	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer == _Players.end() )
	{
		SCPlayer Spl;
		Spl.Player = player;
		_Players.insert( make_pair( userId, Spl ) );

		// update for the unified entity locator
		if (IShardUnifierEvent::getInstance() != NULL)
		{
			IShardUnifierEvent::getInstance()->playerConnected(userId);
		}
	}
	else
	{
		nlwarning("(EGS)<CPlayerManager::addPlayer>  :  The player %d has already been added",userId );
	}

} // addPlayer //


/// A player entity have been removed from eid translator, check all guild member list
void CPlayerManager::checkContactLists()
{
	// for all connected character, check the contact list
	IPlayerManager::TMapPlayers::iterator first(_Players.begin()), last(_Players.end());
	for (; first != last; ++first)
	{
		IPlayerManager::SCPlayer &cp = first->second;

		CPlayer *player = cp.Player;
		if (player != NULL)
		{
			// ok, get it's active character (if any)
			CCharacter *character = player->getActiveCharacter();
			if (character != NULL)
			{
				character->validateContactList();
			}
		}
	}
}

/// A player entity have been removed from eid translator, check all contact list
void CPlayerManager::playerEntityRemoved(const NLMISC::CEntityId &eid)
{
	// for all connected character, check the contact list
	IPlayerManager::TMapPlayers::iterator first(_Players.begin()), last(_Players.end());
	for (; first != last; ++first)
	{
		IPlayerManager::SCPlayer &cp = first->second;

		CPlayer *player = cp.Player;
		if (player != NULL)
		{
			// ok, get it's active character (if any)
			CCharacter *character = player->getActiveCharacter();
			if (character != NULL)
			{
				// there is an active character, check the contact list
				character->removePlayerFromFriendListByEntityId(eid);
				character->removePlayerFromIgnoreListByEntityId(eid);
				if (character->hasInFriendList(eid))
					character->contactListRefChange( eid, CCharacter::RemoveFriend);
				if (character->hasInIgnoreList(eid))
					character->contactListRefChange( eid, CCharacter::RemoveIgnored);
			}
		}
	}
}


//---------------------------------------------------
// getPlayer :
//
//---------------------------------------------------
CPlayer* CPlayerManager::getPlayer( uint32 userId )
{
	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		return (*itPlayer).second.Player;
	}
	return 0;
}


//---------------------------------------------------
// addChar :
//
//---------------------------------------------------
void CPlayerManager::addChar( uint32 userId, CEntityId charId, CCharacter * ch, uint32 index )
{
	H_AUTO(CPlayerManagerAddChar);

	// add char in the player's char list
	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		(*itPlayer).second.Player->addCharacter( ch, index );
	}
	else
	{
		nlwarning("(OS)<CPlayerManager::addChar>  :  The player %d doesn't exist", userId );
	}
} // addChar //


//---------------------------------------------------
// deleteCharacter :
//
//---------------------------------------------------
void CPlayerManager::deleteCharacter( uint32 userId, uint32 characterIndex )
{
	H_AUTO(CPlayerManagerDeleteCharacter);
	
	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		(*itPlayer).second.Player->deleteCharacter( characterIndex );
	}
	else
	{
		nlwarning("(EGS)<CPlayerManager::deleteCharacter>  :  The player %d doesn't exist", userId );
	}
}


//---------------------------------------------------
// getLastCharacterIndex :
//
//---------------------------------------------------
sint32 CPlayerManager::getFirstFreeCharacterIndex( uint32 userId )
{
	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		return (*itPlayer).second.Player->getFirstFreeCharacterIndex();
	}
	else
	{
		throw CPlayerManager::EPlayer(userId);
	}

} // getLastCharacterIndex //


//---------------------------------------------------
// getChar :
//
//---------------------------------------------------
CCharacter * CPlayerManager::getChar( uint32 userId, uint32 index )
{
	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		return (*itPlayer).second.Player->getCharacter( index );
	}
	return NULL;
} // getChar //

//---------------------------------------------------
// getChar :
//---------------------------------------------------
CCharacter * CPlayerManager::getChar( const CEntityId &charId )
{
	CPlayer * player = PlayerManager.getPlayer( PlayerManager.getPlayerId( charId ) );
	if( player )
	{
		return player->getCharacter( charId );
	}
	return NULL;
} // getChar //

//---------------------------------------------------
//---------------------------------------------------
CCharacter *CPlayerManager::getOnlineChar( const NLMISC::CEntityId &charId )
{
	CCharacter * c = getChar( charId );
	if( c != 0 )
	{
		if (TheDataset.isAccessible( c->getEntityRowId() ) )
		{
			return c;
		}
	}
	return 0;
}

CCharacter * CPlayerManager::getChar( const TDataSetRow &rowId )
{
	if (TheDataset.isAccessible(rowId))
	{
		CCharacter * c = getChar( TheDataset.getEntityId(rowId) );
		if( c )
		{
			if( TheDataset.isAccessible( c->getEntityRowId() ) )
			{
				if( c->getEntityRowId() == rowId )
				{
					return c;
				}
				else
				{
					nlwarning("<CPlayerManager::getChar> Character %s accessed by it's TDataSetRow in mirror (E%u_%hu) have a different TDataSetRow initialised in character class (E%u_%hu)",
						c->getId().toString().c_str(), rowId.getIndex(), rowId.counter(), c->getEntityRowId().getIndex(), c->getEntityRowId().counter() );
				}
			}
			else
			{
				nlwarning("<CPlayerManager::getChar> Character %s accessed by it's TDataSetRow in mirror (E%u_%hu) have not it's TDataSetRow initialised in character class",
					c->getId().toString().c_str(), rowId.getIndex(), rowId.counter() );
			}
		}
	}
	return NULL;
}


//---------------------------------------------------
// getActiveChar :
//
//---------------------------------------------------
CCharacter * CPlayerManager::getActiveChar( uint32 userId )
{
	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		return (*itPlayer).second.Player->getActiveCharacter();
	}
	else
	{
		return NULL;
	}

} // getActiveChar //


//---------------------------------------------------
// setActiveCharForPlayer :
//
//---------------------------------------------------
void CPlayerManager::setActiveCharForPlayer( uint32 userId, uint32 index, CEntityId charId )
{
	H_AUTO(CPlayerManagerSetActiveCharForPlayer);
	
	TMapPlayers::iterator itPlayer = _Players.find( userId );
	
	if( itPlayer == _Players.end() )
	{
		nlwarning("(EGS)<CPlayerManager::setActiveCharForPlayer> player %d unknown", userId );
		return;
	}

	nlinfo("set active char %d for player %d", index, userId);

	// set char as active char
	(*itPlayer).second.Player->setActiveCharIndex( index, charId );

	CEntityIdTranslator::getInstance()->setEntityOnline (charId, true);
	
} // setActiveCharForPlayer //


//---------------------------------------------------
// disconnectPlayer :
//
//---------------------------------------------------
void CPlayerManager::disconnectPlayer( uint32 userId )
{
	H_AUTO(CPlayerManagerDisconnectPlayer);
	
	// save and remove active character
	CCharacter *c = PlayerManager.getActiveChar( userId );
	CEntityId id;
	if( c )
	{
		// inform CMissionItem singleton about character disconnection
		CR2MissionItem::getInstance().characterDisconnection(c->getId());

		// tell mission queues manager player is disconnected
		CMissionQueueManager::getInstance()->disconnectPlayer(c);

		// send a message to player to force it's client to quit
		{
			CMessage msgout( "IMPULSION_ID" );
			msgout.serial( const_cast<CEntityId&> (c->getId()) );
			CBitMemStream bms;
			if ( ! GenericMsgManager.pushNameToStream( "CONNECTION:SERVER_QUIT_OK", bms) )
			{
				nlwarning("<disconnectPlayer> Msg name CONNECTION:SERVER_QUIT_OK not found");
				return;
			}
			msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
			CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(c->getId().getDynamicId()), msgout );
		}

// The following commented out by Sadge because the player will be removed from the mirror by the time the message arrives at
// the GPMS and the GPMS code relies on a mirror lookup to get the mirror row...
//		// if we're not in the ring then send msg to GPMS to force no vision processing till player is actually removed
//		if (!IsRingShard)
//		{
//			CMessage	msgout("DISABLE_VISION_PROC");
//			msgout.serial( const_cast<CEntityId&> (c->getId()) );
//			sendMessageViaMirror( "GPMS", msgout );
//		}

		// ADDED : bugfix : some network card/router/software do not send the
		// message of connection close, thus leaving some player 'connected'
		// the an FS leading to false disconnection or connection failure later.
		// Now, send a msg to FS to remove the player.
		//REMOVED : because it make faile the discconnect/reconnect when starting 
		//a ring scenario
//		{
//			CMessage	msgout("DISCONNECT_CLIENT");
//			nlWrite(msgout, serial, uint32(c->getId().getShortId()>>4));
//			sendMessageViaMirror( "FS", msgout );
//		}

		// Save the player's characters
		if( c->getEnterFlag() )
		{
			// Callback to disconnect a character
			c->onDisconnection(false);
			//set disconnection time
			c->setDisconnexionTime();
			
			PlayerManager.savePlayerActiveChar( userId );

			c->removeAllSpells();
		}

		// Remove user language from IOS (in case the player quitted before passing cbClientEnter())
		CMessage msgout("REMOVE_USER_LANGUAGE");
		msgout.serial(userId);
		CUnifiedNetwork::getInstance()->send("IOS", msgout);

		///\todo nico mission
		/*
		// force the failure of group missions take by this character only
		for ( uint i = 0; i < c->getPickedMissions().size(); i++)
		{
			if ( c->getPickedMissions()[i]->getTemplate()->isGroupMission() )
			{
				c->getPickedMissions()[i]->failure();
			}
		}
		*/

		// set player to disconnect mode to disconnected
		CPlayer *player = PlayerManager.getPlayer( userId );
		if( player )
		{
			player->setPlayerConnection( false );
		}
		CBuildingManager::getInstance()->playerDisconnects(c);
	}

	// remove the player from its previous front end if any
	TMapPlayerFrontEndId::iterator itFeid = _PlayerFrontEndId.find( userId );
	if (itFeid != _PlayerFrontEndId.end() )
	{
		TMapServiceIdToUint32Set::iterator	itFe = _MapFrontendToPlayers.find( (*itFeid).second );
		if ( itFe != _MapFrontendToPlayers.end() )
		{
			(*itFe).second.erase( userId );
		}

		_PlayerFrontEndId.erase( itFeid );
	}

	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		CEntityId charId;
		if( (*itPlayer).second.Player->getActiveCharacter() != 0 )
		{
			charId = (*itPlayer).second.Player->getActiveCharacter()->getId();
		}

		if ( ! charId.isUnknownId() )
		{
			CCharacter * character = itPlayer->second.Player->getActiveCharacter();
			if ( character )
			{
				// remove a special user
				for  (uint i = 0; i < _SpecialUsers.size(); i++ )
				{
					std::vector<CEntityId>::iterator it = std::find( _SpecialUsers[i].second.begin(), _SpecialUsers[i].second.end(), character->getId() );
					if ( it != _SpecialUsers[i].second.end() )
					{
						(*it) = _SpecialUsers[i].second.back();
						_SpecialUsers[i].second.pop_back();
						break;
					}
				}
				
				// remove the char from its team
				if ( character->getTeamId() != CTEAM::InvalidTeamId )
					TeamManager.removeCharacter( charId );
				// unvalidate player web account
				// \todo this is unsafe, because name is an ucstring which might be fucked up when casted into string
//				CMailForumValidator::unvalidateUserEntry( character->getName().toString() );
				CMailForumValidator::unvalidateUserEntry( character->getHomeMainlandSessionId(), character->getName().toUtf8() );

				nlinfo("(EGS)<CPlayerManager::disconnectPlayer> player %d (Row %u) removed", userId, character->getEntityRowId().getIndex() );
				// free the current interlocutor of the player
				CCharacter *interlocutor = getChar(character->getCurrentInterlocutor());
				if (interlocutor)
					interlocutor->removeFromExchange();

				if ( character->getEnterFlag() ) // avoid players not entered yet
				{
					// remove entity from PhraseManager
					CPhraseManager::getInstance().removeEntity( character->getEntityRowId(), true );
				}

				// remove fame row for the player (if 'entered')
				if (character->getEnterFlag())
					CFameManager::getInstance().removePlayer( charId );

			}

			CEntityIdTranslator::getInstance()->setEntityOnline (charId, false);
		}

		// erase the player, MUST be done first, as character destructor sometimes call some methods
		// which call CPlayerManager::getChar()
		delete (*itPlayer).second.Player;

		// remove from map
		_Players.erase( itPlayer );

		// update for the unified entity locator
		if (IShardUnifierEvent::getInstance() != NULL)
		{
			IShardUnifierEvent::getInstance()->playerDisconnected(userId);
		}


		// Remove the player from the mirror (only if already in mirror)
		if ( (!charId.isUnknownId()) && FeTempDataset->getDataSetRow( charId ).isValid() )
			Mirror.removeEntity( charId );

		nlinfo( "<cbClientDisconnection> player %u is disconnected", userId );
	}
	else
	{
		nlwarning("(EGS)<CPlayerManager::disconnectPlayer> : The user 0x%x doesn't exist",userId);
	}
} // disconnectPlayer //


//---------------------------------------------------
// disconnectFrontend :
//---------------------------------------------------
void CPlayerManager::disconnectFrontend( NLNET::TServiceId serviceId )
{
	TMapServiceIdToUint32Set::iterator	itFe = _MapFrontendToPlayers.find( serviceId );
	if ( itFe != _MapFrontendToPlayers.end() )
	{
		TUint32Set backupSet = (*itFe).second;
		TUint32Set::iterator it;
		for ( it  = backupSet.begin() ; it != backupSet.end() ; ++it)
		{
			disconnectPlayer( *it );
		}
		_MapFrontendToPlayers.erase( itFe );
	}
} // disconnectFrontend //


//---------------------------------------------------
// GPMS connexion
//---------------------------------------------------
void CPlayerManager::gpmsConnexion()
{
}


//---------------------------------------------------
// getState :
//
//---------------------------------------------------
const CEntityState& CPlayerManager::getState( const CEntityId& charId )
{
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		return c->getState();
	}
	else
	{
		throw EChar(charId);
	}

} // getState //


//---------------------------------------------------
// getType :
//
//---------------------------------------------------
NLMISC::CSheetId CPlayerManager::getType( const CEntityId& charId )
{
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		return c->getType();
	}
	else
	{
		throw EChar(charId);
	}
	
} // getState //


//---------------------------------------------------
// setEnterFlag :
//
//---------------------------------------------------
void CPlayerManager::setEnterFlag( const CEntityId& charId, bool b )
{
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		c->setEnterFlag( b );
		// synchronize the online state
		c->online(b);
	}
	else
	{
		nlwarning("(EGS)<CPlayerManager::setEnterFlag> : The player %s doesn't exist",charId.toString().c_str());
	}

} // setEnterFlag //


//---------------------------------------------------
// setValue :
//
//---------------------------------------------------
void CPlayerManager::setValue( CEntityId charId, string var, string value )
{
	H_AUTO(CPlayerManagerSetValue);
	
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		c->setValue( var, value );
	}
	else
	{
		nlwarning("(EGS)<CPlayerManager::setValue> : The character %s doesn't exist",charId.toString().c_str());
	}

} // setValue //


//---------------------------------------------------
// modifyValue :
//
//---------------------------------------------------
void CPlayerManager::modifyValue( CEntityId charId, string var, string value )
{
	H_AUTO(CPlayerManagerModifyValue);
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		c->modifyValue( var, value );
	}
	else
	{
		nlwarning("(EGS)<CPlayerManager::modifyValue> : The character %s doesn't exist",charId.toString().c_str());
	}

} // setValue //


//---------------------------------------------------
// getValue :
//
//---------------------------------------------------
string CPlayerManager::getValue( CEntityId charId, string var )
{
	H_AUTO(CPlayerManagerGetValue);
	CCharacter * c = PlayerManager.getChar( charId );
	if( c )
	{
		string value;
		c->getValue( var, value );
		return value;
	}
	else
	{
		throw EChar( charId );
	}

} // setValue //


//---------------------------------------------------
// Register all character name to IOS when service is up
//
//---------------------------------------------------
void CPlayerManager::registerCharacterName()
{
	for ( TMapPlayers::iterator itPlayer =_Players.begin() ; itPlayer != _Players.end() ; ++itPlayer )
	{
		CCharacter * c = (*itPlayer).second.Player->getActiveCharacter();
		if( c != 0 )
		{
			c->registerName();

			// by default IOS considers that players do not have an event faction
			// we don't need to register event faction in this case
			if ( !c->getGameEvent().getEventFaction().empty() )
				c->getGameEvent().registerEventFaction();
		}
	}
}

//---------------------------------------------------
// onNPCIconTimerChanged
//---------------------------------------------------
void CPlayerManager::onNPCIconTimerChanged(NLMISC::IVariable &var)
{
	// Send new timer to all players online.
	const CPlayerManager::TMapPlayers& player = PlayerManager.getPlayers();
	for( CPlayerManager::TMapPlayers::const_iterator it = player.begin(); it != player.end(); ++it )
	{
		CCharacter* character = it->second.Player->getActiveCharacter();
		if( character )
		{
			character->sendNpcMissionGiverTimer( true ); // force sending if the value returns to default
		}
	}
}

//---------------------------------------------------
// tickUpdate :
//---------------------------------------------------
void CPlayerManager::tickUpdate()
{
	H_AUTO(CPlayerManagerTickUpdate);

	// process entities waiting name registration if IOS is up
//	if ( !_EntitiesWaitingNameRegistration.empty() && IOSIsUp)
//	{
//		std::list<CEntityId>::iterator it = _EntitiesWaitingNameRegistration.begin();
//		std::list<CEntityId>::iterator itEnd = _EntitiesWaitingNameRegistration.end();
//		uint count = 0;
//		// process only 100 players per tick
//		for ( ; it != itEnd && count < 100 ; ++it )
//		{
//			ucstring name = CEntityIdTranslator::getInstance()->getByEntity(*it);		
//			if ( name.empty() )
//			{
//				nlwarning("empty entity name for %s", (*it).toString().c_str());
//			}
//			else
//			{
//				/* We must use this method to store string in IOS because CPDStringManager counts STORE_STRING/STORE_STRING_RESULT messages,
//				 * then we can use RY_PDS::CPDStringManager::isWaitingIOSStoreStringResult()
//				 */
//				RY_PDS::CPDStringManager::storeStringInIOS( name );
//			}
//
//			++count;
//		}
//
//		// erase processed entities
//		_EntitiesWaitingNameRegistration.erase(_EntitiesWaitingNameRegistration.begin(), it);
//	}

	// update  GM root status
	while ( !_UsersRootedByGM.empty() )
	{
		if ( _UsersRootedByGM.front().EndDate <= CTickEventHandler::getGameCycle() )
		{
			CCharacter::sendDynamicSystemMessage( _UsersRootedByGM.front().Id,"CSR_ROOT_ENDS" );
			_UsersRootedByGM.pop_front();
		}
		else
			break;
	}
	// update  GM mute status
	while ( !_UsersMutedByGM.empty() )
	{
		if ( _UsersMutedByGM.front().EndDate <= CTickEventHandler::getGameCycle() )
		{
			CCharacter::sendDynamicSystemMessage( _UsersMutedByGM.front().Id,"CSR_MUTE_ENDS" );
			CMessage msgOut("MUTE_PLAYER");
			bool mute = false;
			msgOut.serial( const_cast<CEntityId&>(_UsersMutedByGM.front().Id) );
			msgOut.serial( mute );
			CUnifiedNetwork::getInstance()->send( "IOS", msgOut );
			_UsersMutedByGM.pop_front();
		}
		else
			break;
	}

	// update mute universe status
	while ( ! _UsersUniversChatMutedByGM.empty() )
	{
		if ( _UsersUniversChatMutedByGM.front().EndDate <= CTickEventHandler::getGameCycle() )
		{
			CCharacter::sendDynamicSystemMessage( _UsersUniversChatMutedByGM.front().Id,"CSR_UNIVERSE_MUTE_ENDS" );
			CMessage msgOut("MUTE_UNIVERSE");
			bool mute = false;
			msgOut.serial( const_cast<CEntityId&>(_UsersUniversChatMutedByGM.front().Id) );
			msgOut.serial( mute );
			CUnifiedNetwork::getInstance()->send( "IOS", msgOut );
			_UsersUniversChatMutedByGM.pop_front();
		}
		else
			break;
	}

	for( vector< uint32 >::iterator it = _PlayerMustBeDisconnected.begin(); it != _PlayerMustBeDisconnected.end(); ++it )
	{
		TMapPlayers::iterator itPlayer = _Players.find( *it );
		if( itPlayer != _Players.end() )
		{
			CEntityId charId = createCharacterId( *it, (uint8)(*itPlayer).second.Player->getActiveCharIndex() );
			TDataSetRow CharRowId = TheDataset.getDataSetRow( charId );

//			// remove from dyn chat
//			DynChatEGS.removeClient(CharRowId);
		
			CCharacter * c = (*itPlayer).second.Player->getActiveCharacter();
			if( c != 0 )
			{
				if( TheDataset.isAccessible( CharRowId ) )
				{
					// restore character CentityId and TDataSetRow
					c->setId( charId );
					c->setEntityRowId( CharRowId );

					// inform CMissionItem singleton about character disconnection
					CR2MissionItem::getInstance().characterDisconnection(c->getId());

					// remove dyn chats before saving
					CMissionManager::getInstance()->removeAllUserDynChat(c);
					
					// remove processing actions
					CPhraseManager::getInstance().cancelTopPhrase(CharRowId, true);

					// eventually remove character from instanciated zone
					CBuildingManager::getInstance()->removePlayerFromRoom( c );
					
					// remove character from current lift
					CBuildingManager::getInstance()->playerDisconnects(c);
					
					// remove a special user
					for  (uint i = 0; i < _SpecialUsers.size(); i++ )
					{
						std::vector<CEntityId>::iterator it = std::find( _SpecialUsers[i].second.begin(), _SpecialUsers[i].second.end(), charId );
						if ( it != _SpecialUsers[i].second.end() )
						{
							(*it) = _SpecialUsers[i].second.back();
							_SpecialUsers[i].second.pop_back();
							break;
						}
					}

					// remove the char from its team
					TeamManager.removeCharacter( charId );
					
					// remove fame row for the player
					CFameManager::getInstance().removePlayer( charId );
				}	
			}

			// remove the player from its previous front end if any
			TMapPlayerFrontEndId::iterator itFeid = _PlayerFrontEndId.find( *it );
			if (itFeid != _PlayerFrontEndId.end() )
			{
				TMapServiceIdToUint32Set::iterator	itFe = _MapFrontendToPlayers.find( (*itFeid).second );
				if ( itFe != _MapFrontendToPlayers.end() )
				{
					(*itFe).second.erase( *it );
				}
				_PlayerFrontEndId.erase( itFeid );
			}
			CEntityIdTranslator::getInstance()->setEntityOnline (charId, false);
			// Remove the player from the mirror (harmless if not in mirror yet, except warnings)
			Mirror.removeEntity( charId );
			(*itPlayer).second.Player->clearActivePlayerPointer();
			delete (*itPlayer).second.Player;
			_Players.erase( itPlayer );

			// update for the unified entity locator
			if (IShardUnifierEvent::getInstance() != NULL)
			{
				IShardUnifierEvent::getInstance()->playerDisconnected(*it);
			}

			
		}
	}
	_PlayerMustBeDisconnected.clear();

	for( uint32 du = 0; du < _DisconnectedUser.size(); )
	{
		TMapPlayers::iterator itPlayer = _Players.find( _DisconnectedUser[ du ] );
		if( itPlayer != _Players.end() )
		{
			if( (*itPlayer).second.Player->getDisconnectionTime() + TimeBeforeDisconnection < CTickEventHandler::getGameCycle() )
			{
				_DisconnectedUser[ du ] = _DisconnectedUser.back();
				_DisconnectedUser.pop_back();
				disconnectUser( (*itPlayer).first );
			}
			else
			{
				++du;
			}
		}
		else
		{
			_DisconnectedUser[ du ] = _DisconnectedUser.back();
			_DisconnectedUser.pop_back();
		}
	}

	// broadcast message if needed
	broadcastMessageUpdate();
} // tickUpdate //


//---------------------------------------------------
// a user are disconnected
//---------------------------------------------------
void CPlayerManager::userDisconnected( uint32 userId )
{ 
	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		// In case of server crash before delay is elapsed, save character now
		savePlayerActiveChar( userId );
		
		// If the player is in game (has an active character), setup the disconnection timer,
		// otherwise (at character selection) disconnect immediately.
		if ((*itPlayer).second.Player->getActiveCharacter() != 0)
		{
			_DisconnectedUser.push_back( userId );
			(*itPlayer).second.Player->setDisconnectionTime();
		}
		else
		{
			disconnectUser( userId );
		}
	}
}

//---------------------------------------------------
// Name Unifier has renamed a character
//---------------------------------------------------
void CPlayerManager::characterRenamed(uint32 charId, const std::string &newName)
{
	// if the player is in the create character screen, send it a new character description
	uint32 userId = charId >> 4;
	
	TMapPlayers::iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		SCPlayer &scp = itPlayer->second;

		if (scp.Player->getActiveCharacter() == NULL)
		{
			// no active character, send the new character summary
			sendCharactersSummary(scp.Player);
		}
	}	
}


//---------------------------------------------------
// Synchronize client ryzom time and day:
//---------------------------------------------------
void CPlayerManager::synchronizeClientRyzomTime( uint32 ticks, float time, uint32 day )
{
	H_AUTO(CPlayerManagerSynchronizeClientRyzomTime);
	
	for( TMapPlayers::const_iterator it = _Players.begin(); it != _Players.end(); ++it )
	{
		CCharacter * c = (*it).second.Player->getActiveCharacter();
		if( c )
		{
			CMessage msgout( "IMPULSION_ID" );
			CEntityId entityId = static_cast<CEntityId>(c->getId());
			msgout.serial( entityId );
			CBitMemStream bms;
			if ( ! GenericMsgManager.pushNameToStream( "CONNECTION:TIME_DATE_SYNCHRO", bms) )
			{
				nlwarning("<cbClientTimeSync> Msg name CONNECTION:TIME_DATE_SYNCHRO not found");
				return;
			}
			bms.serial( ticks );
			bms.serial( time );
			bms.serial( day );
			msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
			sendMessageViaMirror( NLNET::TServiceId(c->getId().getDynamicId()), msgout );
		}
	}
}


//---------------------------------------------------
// return character corresponding to name, use only for tests TIME consuming !
//---------------------------------------------------
CCharacter * CPlayerManager::getCharacterByName( const std::string& name )
{
	for( TMapPlayers::const_iterator it = _Players.begin(); it != _Players.end(); ++it )
	{
		if( (*it).second.Player->getActiveCharacter() != 0 )
		{
			string name1, name2;
			name1 = NLMISC::strlwr( (*it).second.Player->getActiveCharacter()->getName().toString() );
			name2 = NLMISC::strlwr( name );
			if( name1 == name2 )
			{
				return (*it).second.Player->getActiveCharacter();
			}
		}
	}
	return 0;
}


//---------------------------------------------------
// return character corresponding to name, use only for tests TIME consuming !
//---------------------------------------------------
void CPlayerManager::updateRegen()
{
	H_AUTO(CPlayerManagerUpdateRegen);
	
	for( TMapPlayers::const_iterator it = _Players.begin(); it != _Players.end(); ++it )
	{
		CCharacter * c = (*it).second.Player->getActiveCharacter();
		if( c )
		{
			c->updateRegen();
		}
	}
}


//---------------------------------------------------
// return true if character have privilege
//---------------------------------------------------
bool CPlayerManager::havePriv (uint32 userId, const std::string &priv) const
{
	TMapPlayers::const_iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		return (*itPlayer).second.Player->havePriv(priv);
	}
	return false;
}

//---------------------------------------------------
// return true if character have any privilege
//---------------------------------------------------
bool CPlayerManager::haveAnyPriv(uint32 userId) const
{
	TMapPlayers::const_iterator itPlayer = _Players.find( userId );
	if( itPlayer != _Players.end() )
	{
		return (*itPlayer).second.Player->haveAnyPriv();
	}
	return false;
}

//---------------------------------------------------
// addGMRoot
//---------------------------------------------------
void CPlayerManager::addGMRoot( const NLMISC::CEntityId & gmId , const NLMISC::CEntityId & targetId, NLMISC::TGameCycle endDateInTicks )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);

	// remove previous root
	for ( std::list<CUserCursedByGM>::iterator it = _UsersRootedByGM.begin(); it != _UsersRootedByGM.end(); ++it )
	{
		if ( (*it).Id == targetId )
		{
			_UsersRootedByGM.erase( it );
			break;
		}
	}
	params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
	CCharacter::sendDynamicSystemMessage( gmId,"CSR_ROOT_OK",params );
	params[0].setEIdAIAlias( gmId, CAIAliasTranslator::getInstance()->getAIAlias(gmId) );
	CCharacter::sendDynamicSystemMessage( targetId,"CSR_IS_ROOTED_BY",params );

	// put the new root in the sorted list
	std::list<CUserCursedByGM>::iterator it = _UsersRootedByGM.begin();
	for (; it != _UsersRootedByGM.end(); ++it )
	{
		if ( (*it).EndDate > endDateInTicks )
			break;
	}
	_UsersRootedByGM.insert( it , CUserCursedByGM( targetId, endDateInTicks ) );
}

//---------------------------------------------------
// removeGMRoot
//---------------------------------------------------
void CPlayerManager::removeGMRoot( const NLMISC::CEntityId & gmId , const NLMISC::CEntityId & targetId )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	for ( std::list<CUserCursedByGM>::iterator it = _UsersRootedByGM.begin(); it != _UsersRootedByGM.end(); ++it )
	{
		if ( (*it).Id == targetId )
		{
			params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
			CCharacter::sendDynamicSystemMessage( gmId,"CSR_UNROOT_OK",params );
			params[0].setEIdAIAlias( gmId, CAIAliasTranslator::getInstance()->getAIAlias(gmId) );
			CCharacter::sendDynamicSystemMessage( targetId,"CSR_IS_UNROOTED_BY",params );
			_UsersRootedByGM.erase(it);
			return;
		}
	}
	params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
	CCharacter::sendDynamicSystemMessage( gmId,"CSR_NOT_ROOTED",params );
}

//---------------------------------------------------
// addGMMute
//---------------------------------------------------
void CPlayerManager::addGMMute( const NLMISC::CEntityId & gmId , const NLMISC::CEntityId & targetId, NLMISC::TGameCycle endDateInTicks )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	
	// remove previous root
	for ( std::list<CUserCursedByGM>::iterator it = _UsersMutedByGM.begin(); it != _UsersMutedByGM.end(); ++it )
	{
		if ( (*it).Id == targetId )
		{
			_UsersMutedByGM.erase( it );
			break;
		}
	}
	params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
	CCharacter::sendDynamicSystemMessage( gmId,"CSR_MUTE_OK",params );
	params[0].setEIdAIAlias( gmId, CAIAliasTranslator::getInstance()->getAIAlias(gmId) );
	CCharacter::sendDynamicSystemMessage( targetId,"CSR_IS_MUTED_BY",params );
	
	// put the new root in the sorted list
	std::list<CUserCursedByGM>::iterator it = _UsersMutedByGM.begin();
	for (; it != _UsersMutedByGM.end(); ++it )
	{
		if ( (*it).EndDate > endDateInTicks )
			break;
	}
	_UsersMutedByGM.insert( it , CUserCursedByGM( targetId, endDateInTicks ) );
	CMessage msgOut("MUTE_PLAYER");
	bool mute = true;
	msgOut.serial( const_cast<CEntityId&>(targetId) );
	msgOut.serial( mute );
	CUnifiedNetwork::getInstance()->send( "IOS", msgOut );
}

//---------------------------------------------------
// removeGMMute
//---------------------------------------------------
void CPlayerManager::removeGMMute( const NLMISC::CEntityId & gmId , const NLMISC::CEntityId & targetId )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	for ( std::list<CUserCursedByGM>::iterator it = _UsersMutedByGM.begin(); it != _UsersMutedByGM.end(); ++it )
	{
		if ( (*it).Id == targetId )
		{
			params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
			CCharacter::sendDynamicSystemMessage( gmId,"CSR_UNMUTE_OK",params );
			params[0].setEIdAIAlias( gmId, CAIAliasTranslator::getInstance()->getAIAlias(gmId) );
			CCharacter::sendDynamicSystemMessage( targetId,"CSR_IS_UNMUTED_BY",params );
			_UsersMutedByGM.erase(it);
			break;
		}
	}
	params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
	CCharacter::sendDynamicSystemMessage( gmId,"CSR_NOT_MUTED",params );

	CMessage msgOut("MUTE_PLAYER");
	bool mute = false;
	msgOut.serial( const_cast<CEntityId&>(targetId) );
	msgOut.serial( mute );
	CUnifiedNetwork::getInstance()->send( "IOS", msgOut );
}

//---------------------------------------------------
// mute the universe channel for a duration
//---------------------------------------------------
void CPlayerManager::muteUniverse( const NLMISC::CEntityId & gmId, NLMISC::TGameCycle endDateInTicks, const NLMISC::CEntityId & targetId )
{

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	
	// remove previous mute
	for ( std::list<CUserCursedByGM>::iterator it = _UsersUniversChatMutedByGM.begin(); it != _UsersUniversChatMutedByGM.end(); ++it )
	{
		if ( (*it).Id == targetId )
		{
			_UsersUniversChatMutedByGM.erase( it );
			break;
		}
	}
	params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
	CCharacter::sendDynamicSystemMessage( gmId,"CSR_UNIVERSE_MUTE_OK",params );
	params[0].setEIdAIAlias( gmId, CAIAliasTranslator::getInstance()->getAIAlias(gmId) );
	CCharacter::sendDynamicSystemMessage( targetId,"CSR_IS_UNIVERSE_MUTED_BY",params );
	
	// put the new mute in the sorted list
	std::list<CUserCursedByGM>::iterator it = _UsersUniversChatMutedByGM.begin();
	for (; it != _UsersUniversChatMutedByGM.end(); ++it )
	{
		if ( (*it).EndDate > endDateInTicks )
			break;
	}
	_UsersUniversChatMutedByGM.insert( it, CUserCursedByGM( targetId, endDateInTicks ) );
	CMessage msgOut("MUTE_UNIVERSE");
	bool mute = true;
	msgOut.serial( const_cast<CEntityId&>(targetId) );
	msgOut.serial( mute );
	CUnifiedNetwork::getInstance()->send( "IOS", msgOut );
}

//---------------------------------------------------
// un-mute the universe channel
//---------------------------------------------------
void CPlayerManager::unmuteUniverse( const NLMISC::CEntityId & gmId, const NLMISC::CEntityId & targetId )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	for ( std::list<CUserCursedByGM>::iterator it = _UsersUniversChatMutedByGM.begin(); it != _UsersUniversChatMutedByGM.end(); ++it )
	{
		if ( (*it).Id == targetId )
		{
			params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
			CCharacter::sendDynamicSystemMessage( gmId,"CSR_UNIVERSE_UNMUTE_OK",params );
			params[0].setEIdAIAlias( gmId, CAIAliasTranslator::getInstance()->getAIAlias(gmId) );
			CCharacter::sendDynamicSystemMessage( targetId,"CSR_IS_UNIVERSE_UNMUTED_BY",params );
			_UsersUniversChatMutedByGM.erase(it);
			break;
		}
	}
	params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
	CCharacter::sendDynamicSystemMessage( gmId,"CSR_NOT_UNIVERSE_MUTED",params );
	
	CMessage msgOut("MUTE_UNIVERSE");
	bool mute = false;
	msgOut.serial( const_cast<CEntityId&>(targetId) );
	msgOut.serial( mute );
	CUnifiedNetwork::getInstance()->send( "IOS", msgOut );
}

//---------------------------------------------------
// addAllCharForStringIdRequest
//---------------------------------------------------
//void CPlayerManager::addAllCharForStringIdRequest()
//{
//	// reset eid translator state
//	_EIdTranslatorReady = false;
//
//	const map<NLMISC::CEntityId, CEntityIdTranslator::CEntity> &entities = NLMISC::CEntityIdTranslator::getInstance()->getRegisteredEntities();
//	
//	map<NLMISC::CEntityId, CEntityIdTranslator::CEntity>::const_iterator it;
//	const map<NLMISC::CEntityId, CEntityIdTranslator::CEntity>::const_iterator itEnd = entities.end();
//
//	for ( it = entities.begin() ; it != itEnd ; ++it)
//	{
//		if ( (*it).first.getType() == RYZOMID::player)
//		{
//			PlayerManager.addEntityForStringIdRequest((*it).first);
//		}
//	}
//} // addAllCharForStringIdRequest //

//---------------------------------------------------
// addSpecialUser
//---------------------------------------------------
void CPlayerManager::addSpecialUser( const std::string & priv, const NLMISC::CEntityId & user )
{
	uint i = 0;
	for ( ; i < _SpecialUsers.size(); i++ )
	{
		if ( _SpecialUsers[i].first == priv )
			break;
	}
	if ( i == _SpecialUsers.size() )
		_SpecialUsers.push_back( std::make_pair( priv, std::vector<NLMISC::CEntityId>() ) );
	_SpecialUsers[i].second.push_back( user );
}

//---------------------------------------------------
// getSpecialUsers
//---------------------------------------------------
const vector<CEntityId> * CPlayerManager::getSpecialUsers(const std::string& priv)
{
	uint i = 0;
	for ( ; i < _SpecialUsers.size(); i++ )
	{
		if ( _SpecialUsers[i].first == priv )
			return &_SpecialUsers[i].second;
	}
	return NULL;
}

//---------------------------------------------------
// getDismissCoords
//---------------------------------------------------
bool CPlayerManager::getDismissCoords( const TDataSetRow & userRow, COfflineEntityState & state )
{
	std::map<TDataSetRow, COfflineEntityState>::iterator it = _SummonedUsers.find( userRow );
	if ( it == _SummonedUsers.end() )
		return false;
	state = (*it).second;
	return true;
}

//---------------------------------------------------
// isRootedByGM
//---------------------------------------------------
bool CPlayerManager::isRootedByGM( const NLMISC::CEntityId & userId )
{
	for ( std::list<CUserCursedByGM>::iterator it = _UsersRootedByGM.begin(); it != _UsersRootedByGM.end(); ++it )
	{
		if ( (*it).Id == userId )
			return true;
	}
	return false;
}

//---------------------------------------------------
// hasBetterCSRGrade
//---------------------------------------------------
bool CPlayerManager::hasBetterCSRGrade( const NLMISC::CEntityId& entityId1, const NLMISC::CEntityId& entityId2, bool devIsNormalPlayer )
{
	CPlayer * p1 = PlayerManager.getPlayer( PlayerManager.getPlayerId(entityId1) );
	if (p1 == NULL)
	{
		nlwarning ("ADMIN: Can't find player with UserId %d for checking privilege", PlayerManager.getPlayerId(entityId1));
		return false;
	}
	CPlayer * p2 = PlayerManager.getPlayer( PlayerManager.getPlayerId(entityId2) );
	if (p2 == NULL)
	{
		nlwarning ("ADMIN: Can't find player with UserId %d for checking privilege", PlayerManager.getPlayerId(entityId2));
		return false;
	}

	return hasBetterCSRGrade( p1, p2, devIsNormalPlayer );
}

//---------------------------------------------------
// hasBetterCSRGrade
//---------------------------------------------------
bool CPlayerManager::hasBetterCSRGrade( CPlayer* p1, CPlayer *p2, bool devIsNormalPlayer )
{
	if (p1 == NULL || p2 == NULL)
		return false;

	// grades order: DEV > SGM > EM > GM > EG > VG > SG > G > OBSERVER > PR

	if ( (!devIsNormalPlayer) && p1->havePriv(":DEV:") )
		return true;

	// devs can only be affected by other devs
	if ( (!devIsNormalPlayer) && p2->havePriv(":DEV:") )
		return false;

	if ( p2->havePriv(":SGM:") )
		return ( p1->havePriv(":SGM:") );
	if ( p2->havePriv(":EM:") )
		return ( p1->havePriv(":SGM:EM:GM:") );
	if ( p2->havePriv(":GM:") )
		return ( p1->havePriv(":SGM:EM:GM:") );
	if ( p2->havePriv(":EG:") )
		return ( p1->havePriv(":SGM:EM:GM:EG:") );
	if ( p2->havePriv(":VG:") )
		return ( p1->havePriv(":SGM:EM:GM:EG:VG:") );
	if ( p2->havePriv(":SG:") )
		return ( p1->havePriv(":SGM:EM:GM:EG:VG:SG:") );
	if ( p2->havePriv(":G:") )
		return ( p1->havePriv(":SGM:EM:GM:EG:VG:SG:G:") );
	if ( p2->havePriv(":OBSERVER:") )
		return ( p1->havePriv(":SGM:EM:GM:EG:VG:SG:G:OBSERVER:") );
	if ( p2->havePriv(":PR:") )
		return ( p1->havePriv(":SGM:EM:GM:EG:VG:SG:G:OBSERVER:PR:") );

	// observers cannot affect other players
	if ( p1->havePriv(":OBSERVER:") )
		return false;

	return p1->havePriv(":SGM:EM:GM:EG:VG:SG:G:OBSERVER:PR:");
}

//---------------------------------------------------
// hasBetterCSRGrade
//---------------------------------------------------
bool CPlayerManager::hasBetterCSRGrade( CCharacter * user1, CCharacter * user2, bool devIsNormalPlayer )
{ 
	if (user1 == NULL || user2 == NULL)
		return false;
	return hasBetterCSRGrade( user1->getId(), user2->getId(), devIsNormalPlayer );
}


//-----------------------------------------------------------------------------
void CPlayerManager::respawnPetForInstance( uint32 InstanceNumber, const std::string& InstanceContinent )
{
	for( CPlayerManager::TMapPlayers::const_iterator it = _Players.begin(); it != _Players.end(); ++it )
	{
		CCharacter * c = (*it).second.Player->getActiveCharacter();
		if( c )
		{
			if( c->getEnterFlag() )
				c->respawnPetForInstance( InstanceNumber, InstanceContinent );
		}
	}
}


//-----------------------------------------------------------------------------
void CPlayerManager::respawnHandledAIGroupForInstance( uint32 InstanceNumber )
{
	for( TMapPlayers::const_iterator it = _Players.begin(); it != _Players.end(); ++it )
	{
		CCharacter * c = (*it).second.Player->getActiveCharacter();
		// If the character exists, is in game and in the same instance (same "ai-continent")
		if ( c != NULL )
		{
			if (c->getEnterFlag() && (c->getInstanceNumber() == InstanceNumber))
			{
				// Respawn the bot on ais
				c->spawnAllHandledAIGroup();
			}
		}
	}
}


//--------------------------------------------------------------
//	message broadcasted to character
//--------------------------------------------------------------
void CPlayerManager::broadcastMessage( uint32 repeat, uint32 during, uint32 every, const std::string& message )
{
	if( repeat == 1 && during > 0 )
	{
		_RepeatBroadcastMessage = during / every;
	}
	else
	{
		_RepeatBroadcastMessage = repeat;
	}
	
	_EveryBroadcastMessage = every * 10;
	_NextBroadcastMessage = CTickEventHandler::getGameCycle();
	_BroadcastMessage = message;
	
	broadcastMessageUpdate();
}


//--------------------------------------------------------------
//	message broadcasted update
//--------------------------------------------------------------
void CPlayerManager::broadcastMessageUpdate()
{
	if( _NextBroadcastMessage == CTickEventHandler::getGameCycle() )
	{
		// manage special tokens in broadcast messages
		NLMISC::CSString msg= _BroadcastMessage;
		uint32 timeRemaining= (_RepeatBroadcastMessage-1)* _EveryBroadcastMessage/ 10;
		NLMISC::CSString secondsStr= NLMISC::toString("%u",timeRemaining);
		NLMISC::CSString minutesStr= NLMISC::toString("%u:%02u",timeRemaining/60,timeRemaining%60);
		NLMISC::CSString hoursStr= NLMISC::toString("%u:%02u:%02u",timeRemaining/3600,(timeRemaining/60)%60,timeRemaining%60);
		msg= msg.replace("$seconds$",secondsStr.c_str());
		msg= msg.replace("$minutes$",minutesStr.c_str());
		msg= msg.replace("$hours$",hoursStr.c_str());
		nlinfo("broadcasting message: %s",msg.c_str());

		SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
		params[0].Literal.fromUtf8(msg);
		
		for( TMapPlayers::const_iterator it = _Players.begin(); it != _Players.end(); ++it )
		{
			CCharacter * c = (*it).second.Player->getActiveCharacter();
			if( c )
			{
				c->sendDynamicSystemMessage( c->getId(), "BROADCAST", params );
			}
		}
		
		_RepeatBroadcastMessage--;
		
		if( _RepeatBroadcastMessage > 0 )
		{
			_NextBroadcastMessage = CTickEventHandler::getGameCycle() + _EveryBroadcastMessage;
		}
		else if( _Stall == true )
		{
			forceDisconnectUserWithoutPrivileges();
			broadcastMessage( 2, 0, 5, "Technical problem occured on the server,");
			broadcastMessage( 2, 0, 5, "All non administrator accounts are disconnected immediately.");
			broadcastMessage( 2, 0, 5, "Customer Support is already working on it.");
			broadcastMessage( 2, 0, 5, "Sorry for any inconveniences.");
			broadcastMessage( 2, 0, 15, "...");
		}
	}
}


//--------------------------------------------------------------
// disconnect all users without privileges due to technical pb on server
//--------------------------------------------------------------
void CPlayerManager::forceDisconnectUserWithoutPrivileges()
{
	for( TMapPlayers::const_iterator it = _Players.begin(); it != _Players.end(); )
	{
		CPlayer * player = (*it).second.Player;
		++it;
		if( player )
		{
//			bool havePrivilege = false;
//			havePrivilege = havePrivilege || player->havePriv( ":DEV:" );
//			havePrivilege = havePrivilege || player->havePriv( ":SGM:" );
//			havePrivilege = havePrivilege || player->havePriv( ":GM:" );

			if(!player->havePriv(NoForceDisconnectPriv))
			{
				// ask to front-end for disconnect client
				CMessage msgOut("DISCONNECT_CLIENT");
				uint32 userId = player->getUserId(); 
				msgOut.serial( userId );
				CUnifiedNetwork::getInstance()->send( getPlayerFrontEndId( player->getUserId() ), msgOut );
				
				// disconnect user now for prevent to save it again
				disconnectPlayer( player->getUserId() );
			}
		}
	}		
}

//--------------------------------------------------------------
//	Set name/stringId association, returns true if association has been succesfully set
//--------------------------------------------------------------
//bool CPlayerManager::setStringId( const ucstring &str, uint32 stringId)
//{
//	bool result = NLMISC::CEntityIdTranslator::getInstance()->setEntityNameStringId(str, stringId);
//
//	if ( !_EIdTranslatorReady && _EntitiesWaitingNameRegistration.empty() && !RY_PDS::CPDStringManager::isWaitingIOSStoreStringResult() )
//	{
//		initWhenEIdTranslatorReady();
//		_EIdTranslatorReady = true;
//	}
//
//	return result;
//}

//--------------------------------------------------------------
//	init called after addAllCharForStringIdRequest(), when eid translator is ready (ie all char names have been registered)
//--------------------------------------------------------------
void CPlayerManager::initWhenEIdTranslatorReady()
{
	nlinfo("<CPlayerManager::initWhenEIdTranslatorReady> all character names are now registered in the eid translator, init begins...");

	nlinfo("<CPlayerManager::initWhenEIdTranslatorReady> updating guild members string ids...");
	CGuildManager::getInstance()->updateGuildMembersStringIds();

	nlinfo("<CPlayerManager::initWhenEIdTranslatorReady> init done.");
}

//--------------------------------------------------------------
//	send message of the day to all connected players
//--------------------------------------------------------------
void broadcastMessageOfTheDay(IVariable &var)
{
	string message = var.toString();
	
	if( message.empty() ) return;

	for( CPlayerManager::TMapPlayers::const_iterator it = PlayerManager.getPlayers().begin(); it != PlayerManager.getPlayers().end(); ++it )
	{
		CCharacter * c = (*it).second.Player->getActiveCharacter();
		if( c )
		{
			if( c->getEnterFlag() )
				c->sendMessageOfTheDay();
		}
	}
}



//--------------------------------------------------------------
//  Mail notification, called by mail validator when a player receives a mail
//--------------------------------------------------------------
static void	mailNotification(const std::string& to, const std::string& from)
{
	// 'to' user received a mail

	nldebug("MAIL: received mail notification for '%s' from '%s'", to.c_str(), from.c_str());

	// 'to' is empty, can't do anything
	if (to.empty())
		return;

	// first, build a valid character (upper case first, then lower case);
	ucstring	ucCharname = toLower(to);	// lower case
	ucCharname[0] = toUpper(ucCharname[0]);	// first upper case

	// second, get char id that matches the name
	CEntityId	charId = NLMISC::CEntityIdTranslator::getInstance()->getByEntity(ucCharname);
	// valid name?
	if (charId == CEntityId::Unknown)
		return;

	// get character (only if exists and is online)
	CCharacter*	pChar = PlayerManager.getOnlineChar(charId);
	if (pChar == NULL)
		return;

	// and send impulse to client...
	PlayerManager.sendImpulseToClient(pChar->getId(), "CONNECTION:MAIL_AVAILABLE");
}



//-----------------------------------------------
// Report an action
//-----------------------------------------------
NLMISC_COMMAND(actionReport,"report action for progression testing","<character id(id:type:crea:dyn)> <delta level> <Skill>")
{
	if( args.size() >= 3 && args.size() < 4 )
	{
		CEntityId id;
		id.fromString( args[0].c_str() );
		
		sint32 deltaLevel;
		NLMISC::fromString(args[1], deltaLevel);
		string Skill = args[2];
		SKILLS::ESkills s = SKILLS::toSkill( Skill );
		if( s == SKILLS::unknown )
		{
			log.displayNL("<actionReport> command, unknown Skill '%s'", Skill.c_str() );
			return false;
		}

		CCharacter *c = PlayerManager.getChar(id);
		if( c == 0 )
		{
			log.displayNL("<actionReport> command, unknown character '%s'", id.toString().c_str() );
			return false;
		}

		TReportAction report;
		report.ActorRowId = c->getEntityRowId();
		report.ActionNature = ACTNATURE::CRAFT;
		report.DeltaLvl = deltaLevel;
		report.Skill = s;
		PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( report );
		PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(report);
		return true;
	}
	return false;
}


NLMISC_COMMAND(setPriv,"set a privilege to a user using his user id, must be in form :priv1:priv2:priv3:","<uid> <priv>")
{
	if (args.size() != 2)
		return false;

	if ( !( args[1] == ":SGM:" || args[1] == ":GM:" || args[1] == ":VG:" || args[1] == ":SG:" || args[1] == ":G:" || args[1] == ":DEV:" || 
			args[1] == ":CM:" || args[1] == ":EM:" || args[1] == ":EG:" || args[1] == ":OBSERVER:" || args[1].empty() ) )
	{
		log.displayNL("invalid GM title '%s' valid are : ':SGM:' ':GM:' ':VG:' ':SG:' ':G:' ':DEV:' ':CM:' ':EM:' ':EG:' ':OBSERVER:' ''",args[1].c_str() );
		return true;
	}

	uint32 uid;
	NLMISC::fromString(args[0], uid);
	CPlayer *p = PlayerManager.getPlayer(uid);
	if (p == NULL)
	{
		log.displayNL ("Can't find player with uid %d", uid);
		return false;
	}
	p->setUserPriv (args[1]);
	log.displayNL ("New priv for player %d is '%s'", uid, args[1].c_str());

	if ( !args[1].empty() && !p->havePriv(":DEV:") )
	{
		CCharacter *c = p->getActiveCharacter();
		if (c)
		{
			c->getAdminProperties().init();
			if (p->havePriv(NeverAggroPriv))
			{
				c->setGodMode( true );
				c->setAggroableOverride(0);
			}
			
			if (IsRingShard)
			{
				uint64 whoSeesMe= R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_PLAYER,false);
				if( p->havePriv(":SG:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_SG,false);
				if( p->havePriv(":VG:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_VG,false);
				if( p->havePriv(":EG:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_EG,false);
				if( p->havePriv(":EM:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_EM,false);
				if( p->havePriv(":GM:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_GM,false);
				if( p->havePriv(":SGM:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_SGM,false);
				if( p->havePriv(":DEV:") )				whoSeesMe = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_DEV,false);
				if (p->havePriv(AlwaysInvisiblePriv))	whoSeesMe = R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_PLAYER,false);
				c->setWhoSeesMe(whoSeesMe);
			}
			else if (p->havePriv(AlwaysInvisiblePriv))
			{
				c->setWhoSeesMe(uint64(0));
				c->setInvisibility(true);
			}
		}
	}

	return true;
}


NLMISC_COMMAND(setPvPTag,"set player character PvP TAG to true or false","<eid> <0/1>")
{
	if (args.size() != 2)
		return false;

	CEntityId id;
	id.fromString(args[0].c_str());
	uint tagValue;
	NLMISC::fromString(args[1], tagValue);
	CCharacter *c = PlayerManager.getChar(id);
	if (c == NULL)
	{
		log.displayNL ("Can't find user with id '%s'", args[0].c_str());
		return false;
	}

	if( tagValue == 1 )
	{
		c->setPVPFlag( true );
	}
	else
	{
		c->setPVPFlag( false );
	}
	return true;
}


//NLMISC_COMMAND(checkWeightAndBulk,"check a player bulk","<player ID>")
//{
//	if (args.size() != 1)
//		return false;
//	
//	CEntityId id;
//	id.fromString(args[0].c_str());
//	CCharacter *c = PlayerManager.getChar(id);
//	if (c == NULL)
//	{
//		log.displayNL ("Can't find user with id '%s'", args[0].c_str());
//		return false;
//	}
//	uint32 weight1 = c->getCarriedWeight();
////	uint32 bulk1 = c->getBulk();
//	uint32 bulk1 = c->getInventory(INVENTORIES::bag).getInventoryBulk();
////	c->computeWeightAndBulk();
//	uint32 weight2 = c->getCarriedWeight();
//	uint32 bulk2 = c->getBulk();
//
//	if ( weight1 == weight2 )
//		log.displayNL("weight ok : %d",weight1);
//	else
//		log.displayNL("error : current weight = %d, real weight = %d",weight1,weight2);
//	if ( bulk1 == bulk2 )
//		log.displayNL("bulk ok : %d",bulk2);
//	else
//		log.displayNL("error : current bulk = %d, real bulk = %d",bulk1,bulk2);
//	return true;
//}















