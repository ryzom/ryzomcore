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



#ifndef RY_PVP_DUEL_H
#define RY_PVP_DUEL_H


#include "pvp_interface.h"

#include "game_share/pvp_clan.h"
#include "server_share/pvp_relation.h"

/**
 * A Duel between 2 players
 * \author Stephane coutelas
 * \author Nevrax France
 * \date 2005
 */
class CPVPDuel : public IPVPInterface
{
public:
	/// return pvp relation between the two players
	PVP_RELATION::TPVPRelation getPVPRelation( CCharacter * actor, CEntityBase * target, bool curative = false ) const;
	
	///\name PVP MODE FEATURES DEPENDANTS
	//@{
	/// return true if actor can use gived teleport point
	bool isTPValid( CCharacter* actor, CGameItemPtr TeleportTicket ) const;
	/// return true is respawn point is valid (use the same continent than character)
	bool isRespawnValid( CCharacter* actor, CCharacterRespawnPoints::TRespawnPoint respawnPoint ) const { return true; }
	/// final blower killer in pvp faction
	void finalBlowerKillerInPvPFaction( CCharacter * killer, PVP_CLAN::TPVPClan finalBlowerFaction, CCharacter * victimChar ) const {}
	/// killer in PvP faction
	void characterKillerInPvPFaction( CCharacter * character, PVP_CLAN::TPVPClan winnerFaction, sint32 factionPoint ) const {}
	/// killed character in PvP faction
	void characterKilledInPvPFaction( CCharacter * character, PVP_CLAN::TPVPClan looserFaction, sint32 factionPoint ) const {}
	//@}

};

#endif // RY_PVP_DUEL_H

