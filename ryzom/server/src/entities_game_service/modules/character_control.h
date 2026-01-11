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

#ifndef NL_CHARACTER_CONTROL_H
#define NL_CHARACTER_CONTROL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"

#include "game_share/far_position.h"
#include "game_share/scenario.h"

#include "modules/character_control.h"
#include "modules/r2_mission_item.h"

class ICharacterControl
:	public NLMISC::CManualSingleton<ICharacterControl>
{
public:

	virtual void requestStartParams( const NLMISC::CEntityId& entityId, TSessionId lastStoredSessionId ) = 0;
	
	virtual void requestEntryPoint( const NLMISC::CEntityId& entityId ) = 0;

	virtual void sendItemDescription( TSessionId scenarioId, const std::vector<R2::TMissionItem> &missionItem ) = 0;

	virtual void scenarioEnded( TSessionId scenarioId ) = 0;

	virtual void stealMissionItem( const NLMISC::CEntityId &eid, const std::vector<R2::TItemAndQuantity> &items ) = 0;
	
	virtual void getMissionItemOwnedByCharacter(const NLMISC::CEntityId & eid) = 0;

	virtual void activateEasterEgg(uint32 easterEggId, TSessionId scenarioId, uint32 aiInstanceId, const std::vector< R2::TItemAndQuantity > &items, const CFarPosition &pos, const std::string& name, const std::string & look) =0;
	
	virtual void deactivateEasterEgg(uint32 easterEggId, TSessionId scenarioId) =0;

	virtual void getEasterEggDropped(TSessionId scenarioId, std::vector<R2::TEasterEggInfo> &easterEgg ) =0;

	virtual void lootEasterEggEvent( uint32 externalEasterEggId, TSessionId scenarioId ) =0;

	virtual TSessionId getSessionId(TSessionId sessionId) const = 0;

	virtual void characterReady(const NLMISC::CEntityId &entityId) =0;
};


#endif // NL_CHARACTER_CONTROL_H

/* End of character_control.h */
