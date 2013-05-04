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

#include "nel/misc/variable.h"

#include "nel/net/service.h"

#include "game_event_manager.h"

#include "primitives_parser.h"

#include "player_manager/player_manager.h"
#include "player_manager/character_game_event.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "egs_variables.h"
#include "pvp_manager/pvp_zone.h"



// ----------------------------------------------------------------------------

using namespace NLMISC;
using namespace NLNET;
using namespace std;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


// ----------------------------------------------------------------------------

CVariable<std::string> GameEventFile("egs", "GameEventFile", "game event file holding all g.e. stuff", std::string("game_event.txt"), 0, true );

// ----------------------------------------------------------------------------
CGameEventManager::CGameEventManager()
{
	_Date = 0;
	_ChannelEventId = TChanID::Unknown;
	_FactionChanelInZoneOnly = false;
	_InitOk = false;
}

// ----------------------------------------------------------------------------
void CGameEventManager::init()
{
	string sFilename = GameEventFile;
	sFilename = CPath::standardizePath(IService::getInstance()->WriteFilesDirectory) + sFilename;

	if (CFile::fileExists(sFilename))
	{
		static CPersistentDataRecord	pdr;
		pdr.clear();
		pdr.readFromTxtFile(sFilename.c_str());
		apply(pdr);
		createEventChannel();
	}
	_InitOk = true;
}

// ----------------------------------------------------------------------------
void CGameEventManager::resetGameEvent(const string &sEventName, const string &sEventFaction1, const string &sEventFaction2, const string &sEventFaction1ChannelName, const string &sEventFaction2ChannelName, bool factionChanelInZoneOnly )
{
	_Name = sEventName;
	_EventFaction1Name = sEventFaction1;
	_EventFaction2Name = sEventFaction2;

	_EventFaction1ChannelName = sEventFaction1ChannelName;
	_EventFaction2ChannelName = sEventFaction2ChannelName;

	_FactionChanelInZoneOnly = factionChanelInZoneOnly;
	
	// Get the starting date
	if( !sEventName.empty() )
		_Date = CTickEventHandler::getGameCycle();
	else
		_Date = 0; // event stoped

	// Save game event file
	saveGameEventFile();

	// Start a new event channel
	if( _ChannelEventId != TChanID::Unknown )
	{
		DynChatEGS.removeChan( _ChannelEventId );
		_ChannelEventId = TChanID::Unknown;
	}
	if( _ChannelEventFaction1Id != TChanID::Unknown )
	{
		DynChatEGS.removeChan( _ChannelEventFaction1Id );
		_ChannelEventFaction1Id = TChanID::Unknown;
	}
	if( _ChannelEventFaction2Id != TChanID::Unknown )
	{
		DynChatEGS.removeChan( _ChannelEventFaction2Id );
		_ChannelEventFaction2Id = TChanID::Unknown;
	}
	if( _ChannelGMEventFaction1Id != TChanID::Unknown )
	{
		DynChatEGS.removeChan( _ChannelGMEventFaction1Id );
		_ChannelGMEventFaction1Id = TChanID::Unknown;
	}
	if( _ChannelGMEventFaction2Id != TChanID::Unknown )
	{
		DynChatEGS.removeChan( _ChannelGMEventFaction2Id );
		_ChannelGMEventFaction2Id = TChanID::Unknown;
	}
		
	createEventChannel();

	// For all character online reset their game event part
	const CPlayerManager::TMapPlayers &playerMap = PlayerManager.getPlayers();
	CPlayerManager::TMapPlayers::const_iterator it = playerMap.begin();
	while (it != playerMap.end())
	{
		CPlayer *p = it->second.Player;		
		CCharacter *c = p->getActiveCharacter();
		if (c != NULL)
		{
			c->getGameEvent().reset();
			addCharacterToChannelEvent( c );

			if( !_FactionChanelInZoneOnly )
			{
				// find the clan
				PVP_CLAN::TPVPClan clan1 = PVP_CLAN::fromString(sEventFaction1);
				PVP_CLAN::TPVPClan clan2 = PVP_CLAN::fromString(sEventFaction2);
				PVP_CLAN::TPVPClan clan = CPVPVersusZone::getPlayerClan( c, clan1, clan2 );
				// add the good chanel
				if( clan == clan1 )
				{
					addCharacterToChannelFactionEvent( c, 1 );
				}
				else if( clan == clan2 )
				{
					addCharacterToChannelFactionEvent( c, 2 );
				}
			}
		}
		++it;
	}
}

// ----------------------------------------------------------------------------
void CGameEventManager::createEventChannel()
{
	if( !_Name.empty() )
	{
		ucstring title;
		title.fromUtf8(_Name);
		_ChannelEventId = DynChatEGS.addChan(_Name, title );
		// set historic size of the newly created channel
		DynChatEGS.setHistoricSize( _ChannelEventId, EventChannelHistoricSize );
	}

	if( !_EventFaction1Name.empty() )
	{
		ucstring title;
		title.fromUtf8(_EventFaction1Name);
		_ChannelEventFaction1Id = DynChatEGS.addChan(_EventFaction1Name, title);
		if( _EventFaction1ChannelName.empty() )
		{
			title.fromUtf8(_EventFaction1Name+"_event");
		}
		else
		{
			title.fromUtf8( _EventFaction1ChannelName );
		}
		_ChannelGMEventFaction1Id = DynChatEGS.addChan(_EventFaction1Name+"_GM", title);
		// set historic size of the newly GM created channel
		DynChatEGS.setHistoricSize( _ChannelGMEventFaction1Id, EventChannelHistoricSize );
	}

	if( !_EventFaction2Name.empty() )
	{
		ucstring title;
		title.fromUtf8(_EventFaction2Name);
		_ChannelEventFaction2Id = DynChatEGS.addChan(_EventFaction2Name, title );
		if( _EventFaction2ChannelName.empty() )
		{
			title.fromUtf8(_EventFaction2Name+"_event");
		}
		else
		{
			title.fromUtf8( _EventFaction2ChannelName );
		}
		_ChannelGMEventFaction2Id = DynChatEGS.addChan(_EventFaction2Name+"_GM", title);
		// set historic size of the newly GM created channel
		DynChatEGS.setHistoricSize( _ChannelGMEventFaction2Id, EventChannelHistoricSize );
	}
}

// ----------------------------------------------------------------------------
void CGameEventManager::characterLoadedCallback(CCharacter *c)
{
	if (c->getGameEvent().getDate() < _Date)
	{
		c->getGameEvent().reset();
	}
}

// ----------------------------------------------------------------------------
void CGameEventManager::addCharacterToChannelEvent( CCharacter *c )
{
	if( _ChannelEventId != TChanID::Unknown )
	{
		if( c->haveAnyPrivilege() )
		{
			DynChatEGS.addSession(_ChannelEventId, c->getEntityRowId(), true);
			if( _ChannelEventFaction1Id != TChanID::Unknown )
				DynChatEGS.addSession(_ChannelEventFaction1Id, c->getEntityRowId(), true);
			if( _ChannelEventFaction2Id != TChanID::Unknown )
				DynChatEGS.addSession(_ChannelEventFaction2Id, c->getEntityRowId(), true);
			if( _ChannelGMEventFaction1Id != TChanID::Unknown )
				DynChatEGS.addSession(_ChannelGMEventFaction1Id, c->getEntityRowId(), true);
			if( _ChannelGMEventFaction2Id != TChanID::Unknown )
				DynChatEGS.addSession(_ChannelGMEventFaction2Id, c->getEntityRowId(), true);
		}
		else
		{
			DynChatEGS.addSession(_ChannelEventId, c->getEntityRowId(), false);
		}
	}
}

// ----------------------------------------------------------------------------
void CGameEventManager::addCharacterToChannelFactionEvent( CCharacter *c, uint8 clan )
{
	if( c != 0 && c->haveAnyPrivilege() == false )
	{
		if( clan == 1 )
		{
			if( _ChannelGMEventFaction1Id != TChanID::Unknown )
				DynChatEGS.addSession(_ChannelGMEventFaction1Id, c->getEntityRowId(), false);
			if( _ChannelEventFaction1Id != TChanID::Unknown )
				DynChatEGS.addSession(_ChannelEventFaction1Id, c->getEntityRowId(), true);
		}
		else
		{
			if( _ChannelGMEventFaction2Id != TChanID::Unknown )
				DynChatEGS.addSession(_ChannelGMEventFaction2Id, c->getEntityRowId(), false);
			if( _ChannelEventFaction2Id != TChanID::Unknown )
				DynChatEGS.addSession(_ChannelEventFaction2Id, c->getEntityRowId(), true);
		}
		c->channelAdded(true);
	}
}

// ----------------------------------------------------------------------------
void CGameEventManager::removeCharacterToChannelFactionEvent( CCharacter *c, uint8 clan )
{
	if( c != 0 )
	{
		if( clan == 1 )
		{
			if( _ChannelGMEventFaction1Id != TChanID::Unknown )
				DynChatEGS.removeSession(_ChannelGMEventFaction1Id, c->getEntityRowId());
			if( _ChannelEventFaction1Id != TChanID::Unknown )
				DynChatEGS.removeSession(_ChannelEventFaction1Id, c->getEntityRowId());
		}
		else
		{
			if( _ChannelGMEventFaction1Id != TChanID::Unknown )
				DynChatEGS.removeSession(_ChannelGMEventFaction2Id, c->getEntityRowId());
			if( _ChannelEventFaction2Id != TChanID::Unknown )
				DynChatEGS.removeSession(_ChannelEventFaction2Id, c->getEntityRowId());
		}
		c->channelAdded(false);
	}
}

// ----------------------------------------------------------------------------
bool CGameEventManager::isGameEventMission(TAIAlias missionAlias)
{
	// Get the alias static part (given by the file r:\code\ryzom\data_leveldesign\primitives\file_index.cfg)
	uint32 maStaticPart = CPrimitivesParser::getInstance().aliasGetStaticPart(missionAlias);

	// The last 1024 aliases are the GameEvent aliases !!!
	if ((maStaticPart >= (4096-1024)) && (maStaticPart < 4096) )
		return true;

	return false;
}

// ----------------------------------------------------------------------------
void CGameEventManager::saveGameEventFile()
{
	if( _InitOk )
	{
		string sFilename = GameEventFile;
		sFilename = CPath::standardizePath(IService::getInstance()->WriteFilesDirectory) + sFilename;
		
		static CPersistentDataRecordRyzomStore	pdr;
		pdr.clear();
		store(pdr);
		pdr.writeToTxtFile(sFilename.c_str());
	}
}

// ----------------------------------------------------------------------------
// PERSISTENCE METHODS
// ----------------------------------------------------------------------------

#define PERSISTENT_CLASS CGameEventManager

#define PERSISTENT_PRE_STORE\
	H_AUTO(CGameEventManagerStore);\

#define PERSISTENT_PRE_APPLY\
	H_AUTO(CGameEventManagerApply);\

#define PERSISTENT_DATA\
	PROP_GAME_CYCLE_COMP(_Date)\
	PROP(string, _Name)\
	PROP(string, _EventFaction1Name)\
	PROP(string, _EventFaction2Name)\
	PROP(string, _EventFaction1ChannelName)\
	PROP(string, _EventFaction2ChannelName)\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"

