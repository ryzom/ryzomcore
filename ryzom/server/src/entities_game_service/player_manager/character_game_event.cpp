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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "nel/net/message.h"
#include "egs_sheets/egs_sheets.h"
#include "game_share/msg_encyclopedia.h"
#include "game_share/string_manager_sender.h"
#include "game_share/synchronised_message.h"
#include "player_manager/character_game_event.h"
#include "player_manager/character.h"
#include "game_event_manager.h"
#include "dyn_chat_egs.h"
#include "egs_pd.h"

//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CCharacterGameEvent);

//-----------------------------------------------------------------------------
// vars
//-----------------------------------------------------------------------------
CVariable<uint32> PlayerChannelHistoricSize("egs","PlayerChannelHistoricSize", "historic size of a player channel", 50, 0, true );
CVariable<uint32> GMChannelHistoricSize("egs","GMChannelHistoricSize", "historic size of a gm channel", 100, 0, true );


//-----------------------------------------------------------------------------
// methods CCharacterGameEvent
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CCharacterGameEvent::CCharacterGameEvent(CCharacter &c) : _Char(c)
{
	_Date = 0;
	_RegisterEventFaction = false;
}

//-----------------------------------------------------------------------------
void CCharacterGameEvent::subscribe()
{
	// if we are already in a game event do not reset
	if (isInGameEvent()) return;
	
	// Here is the code for the backup of data (last player position for ending correctly for instance)

	_Date = CTickEventHandler::getGameCycle();
}

//-----------------------------------------------------------------------------
void CCharacterGameEvent::reset()
{
	// if we are not in a game event do not reset
	if (! isInGameEvent()) return;

	// Remove game event mission
	map<TAIAlias, CMission*>::iterator it = _Char.getMissionsBegin();
	// remove missions
	while (it != _Char.getMissionsEnd())
	{
		if (CGameEventManager::getInstance().isGameEventMission(it->first))
		{
			TAIAlias mission = it->first;
			map<TAIAlias, CMission*>::iterator itToDel = it;
			++it;
			_Char._Missions->deleteFromMissions( mission );
		}
		else
		{
			++it;
		}
	}
	// remove mission history
	map<TAIAlias, TMissionHistory>::iterator itH = _Char._MissionHistories.begin();
	while (itH != _Char._MissionHistories.end())
	{
		if (CGameEventManager::getInstance().isGameEventMission(it->first))
		{
			TAIAlias mission = itH->first;
			map<TAIAlias, TMissionHistory>::iterator itHToDel = itH;
			++itH;
			_Char._MissionHistories.erase( itHToDel );
		}
		else
		{
			++itH;
		}
	}
	
	clearEventFaction();

	_Date = 0;
}

//-----------------------------------------------------------------------------
void CCharacterGameEvent::tickUpdate()
{
	if (_RegisterEventFaction && _Char.getEnterFlag())
	{
		registerEventFaction();
		setEventChannelSessions();
		_RegisterEventFaction = false;
	}
}

//-----------------------------------------------------------------------------
void CCharacterGameEvent::setEventFaction(const std::string & eventFaction)
{
	if( eventFaction != _EventFaction )
	{
		_PreviousEventFaction = _EventFaction;
		_EventFaction = eventFaction;
		_RegisterEventFaction = true;
	}
}

//-----------------------------------------------------------------------------
void CCharacterGameEvent::registerEventFaction()
{
	if (!_Char.getEnterFlag())
		return;

	TDataSetRow rowId = _Char.getEntityRowId();

	CMessage msg("CHARACTER_EVENT_FACTION");
	msg.serial(rowId);
	msg.serial(_EventFaction);

	sendMessageViaMirror("IOS", msg);
}

//-----------------------------------------------------------------------------
void CCharacterGameEvent::setEventChannelSessions()
{
	addEventSession( _EventFaction, true, PlayerChannelHistoricSize );
	removeEventSession( _PreviousEventFaction );

	addEventSession( _EventFaction + "_gm", false, GMChannelHistoricSize );
	removeEventSession( _PreviousEventFaction + "_gm" );
}


//-----------------------------------------------------------------------------
void CCharacterGameEvent::addEventSession( const string& channel, bool writeRight, uint32 historicSize )
{
	if( !channel.empty() )
	{
		TDataSetRow rowId = _Char.getEntityRowId();
		
		// add channel in dynchat
		TChanID chanId = DynChatEGS.addLocalizedChan( channel);
		
		// if channel didn't exist
		if( chanId != DYN_CHAT_INVALID_CHAN )
		{
			// set historic size of the newly created channel
			DynChatEGS.setHistoricSize( chanId, historicSize );
		}
		else
		{
			// get id of existing channel
			chanId = DynChatEGS.getChanIDFromName( channel );
		}

		// add a char session in his new channel
		if( chanId != DYN_CHAT_INVALID_CHAN )
		{
			if( !DynChatEGS.addSession( chanId, rowId, writeRight) )
			{
				nlwarning("<CCharacterGameEvent::addEventSession> Can't add session in channel %s for char %s",channel.c_str(), _Char.getId().toString().c_str());
			}
		}
	}
}


//-----------------------------------------------------------------------------
void CCharacterGameEvent::removeEventSession( const string& channel )
{
	if( !channel.empty() )
	{
		TChanID chanId = DynChatEGS.getChanIDFromName( channel );
		if( chanId != DYN_CHAT_INVALID_CHAN )
		{
			TDataSetRow rowId = _Char.getEntityRowId();

			// remove session of char from channel
			if( !DynChatEGS.removeSession( chanId, rowId ) )
			{
				nlwarning("<CCharacterGameEvent::removeEventSession> Can't remove session in channel %s for char %s",channel.c_str(),_Char.getId().toString().c_str());
			}
			
			// if channel is empty now, remove it
			if( DynChatEGS.getSessionCount( chanId ) == 0 )
			{
				if( !DynChatEGS.removeChan( chanId ) )
				{
					nlwarning("<CCharacterGameEvent::removeEventSession> Can't remove channel %s", chanId.toString().c_str());
				}
			}
		}
	}
}

