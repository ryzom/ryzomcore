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



#ifndef EGS_GAME_EVENT_MANAGER_H
#define EGS_GAME_EVENT_MANAGER_H

// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "nel/misc/common.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/eid_translator.h"

#include "game_share/persistent_data.h"

#include "mission_manager/ai_alias_translator.h"

#include "dyn_chat_egs.h"

class CCharacter;

/**
 * CGameEventManager
 *
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date December 2004
 */
class CGameEventManager : public NLMISC::CSingleton<CGameEventManager>
{
public:
	CGameEventManager();

	// --------------------------
	// State Modification Methods
	// --------------------------
	
	// Read game event file
	void init();

	// No more than one GameEvent at a time
	void resetGameEvent(const std::string &sEventName, const std::string &sEventFaction1, const std::string &sEventFaction2, const std::string &sEventFaction1ChannelName, const std::string &sEventFaction2ChannelName, bool factionChanelInZoneOnly );

	// called when a new character is loaded
	void characterLoadedCallback(CCharacter *c);

	// add a new loged character to event channel s
	void addCharacterToChannelEvent( CCharacter *c );

	// add character to it's faction event channels
	void addCharacterToChannelFactionEvent( CCharacter *c, uint8 clan );
		
	// remove character to it's faction event channels
	void removeCharacterToChannelFactionEvent( CCharacter *c, uint8 clan );

	// --------------
	// Access Methods
	// --------------
	
	// Does an event is running ?
	bool isGameEventRunning() { return (_Date != 0); }

	// Get the date where the event started
	uint32 getStartingDate() { return _Date; }

	const std::string &getGameEventName() { return _Name; }

	bool isGameEventMission(TAIAlias missionAlias);

	TChanID getChannelEventId() const { return _ChannelEventId; }

	std::string	getFaction1() { return _EventFaction1Name; }
	std::string	getFaction2() { return _EventFaction2Name; }

	bool isFactionChanelInZoneOnly() { return _FactionChanelInZoneOnly;	}

	inline bool isInit() { return _InitOk; }

	DECLARE_PERSISTENCE_METHODS
		
private:
	void saveGameEventFile();

	void createEventChannel();
		
private:
	
	// The Singleton instance
	NLMISC::TGameCycle	_Date;

	std::string			_Name;
	std::string			_EventFaction1Name;
	std::string			_EventFaction2Name;
	std::string			_EventFaction1ChannelName;
	std::string			_EventFaction2ChannelName;
	
	TChanID				_ChannelEventId;
	TChanID				_ChannelEventFaction1Id;
	TChanID				_ChannelEventFaction2Id;
	TChanID				_ChannelGMEventFaction1Id;
	TChanID				_ChannelGMEventFaction2Id;

	bool				_FactionChanelInZoneOnly;

	bool				_InitOk;
};

#endif // EGS_GAME_EVENT_MANAGER_H

