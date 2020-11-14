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



#ifndef RY_PVP_INTERFACE_H
#define RY_PVP_INTERFACE_H

#include "player_manager/character_respawn_points.h"

#include "game_share/pvp_clan.h"
#include "server_share/pvp_relation.h"

class CEntityBase;
class CCharacter;
class CGameItemPtr;

/**
 * pvp interface 
 * \author Alain Saffray
 * \author Based on previous PVP interface made by Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class IPVPInterface : public NLMISC::CRefCount
{
public:
	virtual ~IPVPInterface() {}
	///\name PVP MODE FEATURES DEPENDANTS
	//@{
	/// return pvp relation between the two players
	virtual PVP_RELATION::TPVPRelation getPVPRelation( CCharacter * actor, CEntityBase * target, bool curative = false ) const = 0;
	/*
	/// return true if actor can use a curative action on target
	virtual bool isCurativeActionValid( CCharacter* actor, CEntityBase * target, bool checkMode = false ) const = 0;
	/// return true if actor can use an offensive action on target
	virtual bool isOffensiveActionValid( CCharacter* actor, CEntityBase * target, bool checkMode = false ) const = 0;
	/// returns true if owner area effect can be applied on areaTarget
	virtual bool canApplyAreaEffect(CCharacter* actor, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const = 0;
	*/
	/// return true if actor can use gived teleport point
	virtual bool isTPValid( CCharacter* actor, CGameItemPtr TeleportTicket ) const = 0;
	/// return true is respawn point is valid (use the same continent than character)
	virtual bool isRespawnValid( CCharacter* actor, CCharacterRespawnPoints::TRespawnPoint respawnPoint ) const = 0;
	/// final blower killer in pvp faction
	virtual void finalBlowerKillerInPvPFaction( CCharacter * killer, PVP_CLAN::TPVPClan finalBlowerFaction, CCharacter * victimChar ) const = 0;
	/// killer in PvP faction
	virtual void characterKillerInPvPFaction( CCharacter * character, PVP_CLAN::TPVPClan winnerFaction, sint32 factionPoint ) const = 0;
	/// killed character in PvP faction
	virtual void characterKilledInPvPFaction( CCharacter * character, PVP_CLAN::TPVPClan looserFaction, sint32 factionPoint ) const = 0;
	//@}
};

#endif // RY_PVP_INTERFACE_H


