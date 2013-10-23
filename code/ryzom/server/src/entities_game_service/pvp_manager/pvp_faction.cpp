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

#include "pvp_faction.h"
#include "player_manager/character.h"
#include "pvp_manager/pvp_manager_2.h"
#include "zone_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "stat_db.h"
#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"
#include "pvp_manager/pvp_faction_hof.h"
#include "nel/misc/variable.h"

using namespace std;
using namespace NLMISC;
using namespace	PHRASE_UTILITIES;

CVariable<bool> ResPawnPVPInSameRegionForbiden("egs","ResPawnPVPInSameRegionForbiden", "When character dead in PvP Faction, it can't respawn in same same region of it's death", true, 0, true );

//----------------------------------------------------------------------------
PVP_RELATION::TPVPRelation CPVPFaction::getPVPRelation( CCharacter * actor, CEntityBase * target, bool curative ) const
{
	// Init relation reminders
	CPVPManager2::getInstance()->setPVPFactionAllyReminder( false );
	CPVPManager2::getInstance()->setPVPFactionEnemyReminder( false );

	bool isAlly = false;

	bool targetSafe = false;
	bool actorSafe = false;
	bool targetInSafeZone = false;
	bool actorInSafeZone = false;

	// Check actor and target validity
	if (actor == 0 || target == 0)
	{
		nlwarning("<CPVPFaction::getPVPRelation> actor: %p  target: %p", actor, target);
		return PVP_RELATION::Unknown;
	}

	CCharacter * pTarget = dynamic_cast<CCharacter*>(target);
	if (pTarget == 0)
		return PVP_RELATION::Unknown;

	if (CPVPManager2::getInstance()->inSafeZone(pTarget->getPosition()))
	{
		targetInSafeZone = true;
		if (pTarget->getSafeInPvPSafeZone())
			targetSafe = true;
	}

	if( CPVPManager2::getInstance()->inSafeZone(actor->getPosition()))
	{
		actorInSafeZone = true;
		if( actor->getSafeInPvPSafeZone())
			actorSafe = true;
	}
	
	// Target is not tagged => Neutral
	if (!pTarget->getPVPFlag() && !pTarget->getPvPRecentActionFlag())
		return PVP_RELATION::Neutral;

	// Actor is not tagged => Check target
	if (!actor->getPVPFlag() && !actor->getPvPRecentActionFlag())
	{
		// Target flag => NeutralPVP
		if (pTarget->getPvPRecentActionFlag())
			return PVP_RELATION::NeutralPVP;
		else
			return PVP_RELATION::Neutral;
	}


	// In same Team => Ally
	if ((pTarget->getTeamId() != CTEAM::InvalidTeamId) && (actor->getTeamId() != CTEAM::InvalidTeamId) && (actor->getTeamId() == pTarget->getTeamId()))
	{
		isAlly = true;
	}

	// In Same League => Ally
	if ((pTarget->getLeagueId() != DYN_CHAT_INVALID_CHAN) && (actor->getLeagueId() != DYN_CHAT_INVALID_CHAN) && (actor->getLeagueId() == pTarget->getLeagueId()))
	{
		isAlly = true;
	}

	// In same Guild and not Leagued => Ally
	if ((pTarget->getGuildId() != 0) && (actor->getGuildId() != 0) && (actor->getGuildId() == pTarget->getGuildId()) && (pTarget->getLeagueId() == DYN_CHAT_INVALID_CHAN) && (actor->getLeagueId() == DYN_CHAT_INVALID_CHAN))
	{
		isAlly = true;
	}

	if (isAlly)
	{
		// One is in safe zone but not other => NeutralPVP
		if ((targetInSafeZone && !actorInSafeZone) || (actorInSafeZone && !targetInSafeZone))
		{
			return PVP_RELATION::NeutralPVP;
		}
		
		// One is safe but not other => NeutralPVP
		if (targetSafe != actorSafe)
		{
			return PVP_RELATION::NeutralPVP;
		}

		CPVPManager2::getInstance()->setPVPFactionAllyReminder(true);
		return PVP_RELATION::Ally;
	}
	
	// If one is safe => NeutralPVP
	if (targetSafe || actorSafe)
	{
		return PVP_RELATION::NeutralPVP;
	}
	
	// 
	if ((targetInSafeZone && !pTarget->getPvPRecentActionFlag()) || (actorInSafeZone && !actor->getPvPRecentActionFlag()))
	{
		return PVP_RELATION::NeutralPVP;
	}

	CPVPManager2::getInstance()->setPVPFactionEnemyReminder(true);
	return PVP_RELATION::Ennemy;
}


//----------------------------------------------------------------------------
bool CPVPFaction::isTPValid( CCharacter* actor, CGameItemPtr TeleportTicket ) const
{
	bool result = ! actor->getPvPRecentActionFlag();
	if( TeleportTicket != 0 && TeleportTicket->getStaticForm() != 0 && result )
	{
		// if character have is not PvP faction flagged (Tag PvP off), only recent PvP action can forbid TP usage
		if( actor->getPVPFlag() == false )
			return true;

		// check if war occurs between player character allegiances and region destination clan owner  
		uint16 idx = CZoneManager::getInstance().getTpSpawnZoneIdByName( TeleportTicket->getStaticForm()->Destination );
		const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone( idx );
		if( zone != 0 )
		{
			PVP_CLAN::TPVPClan regionFactionOwner = CPVPFactionRewardManager::getInstance().getRegionOwner( zone->getRegion() );
			pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> actorAllegiance = actor->getAllegiance();

			if( regionFactionOwner == PVP_CLAN::Neutral )
				return true;
			else
			{
				if( CPVPManager2::getInstance()->factionWarOccurs(actorAllegiance.first, regionFactionOwner) == false 
				&& CPVPManager2::getInstance()->factionWarOccurs(actorAllegiance.second, regionFactionOwner) == false )
					return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------------
bool CPVPFaction::isRespawnValid( CCharacter* actor, CCharacterRespawnPoints::TRespawnPoint respawnPoint ) const
{
	if( actor->getPVPFlag() == false )
		return true;

	const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(respawnPoint);
	if (zone == NULL)
		return false;

	// validate default respawn point
	if( zone->getPlaceType() == PLACE_TYPE::Capital || zone->getName() == "place_aegus" || zone->getName() == "place_stalli" || zone->getName() == "place_qai_lo" || zone->getName() == "place_aubermouth" )
		return true;

	// check if the respawn point is in the region where player character is killed in PvP
	if( ResPawnPVPInSameRegionForbiden )
		if ( (zone->getRegion() == actor->getKilledPvPRegion() ) && actor->getPvPRecentActionFlag() )
			return false;

	// check if respawn point is in enemy region
	PVP_CLAN::TPVPClan regionClan = CPVPFactionRewardManager::getInstance().getRegionOwner(zone->getRegion());
	pair< PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan > allegiance = actor->getAllegiance();

	if( regionClan != PVP_CLAN::Neutral )
	{
		if( CPVPManager2::getInstance()->factionWarOccurs( regionClan, allegiance.first ) || 
			CPVPManager2::getInstance()->factionWarOccurs( regionClan, allegiance.second ) )
			return false;
	}
	return true;
}

//----------------------------------------------------------------------------
void CPVPFaction::finalBlowerKillerInPvPFaction( CCharacter * killer, PVP_CLAN::TPVPClan finalBlowerFaction, CCharacter * victimChar ) const
{
	CPVPFactionHOF::getInstance()->writeStatInHOFDatabase( killer, finalBlowerFaction, CPVPFactionHOF::final_blow, 1 );

	BOMB_IF(victimChar == 0, "<CPVPFaction::finalBlowerKillerInPvPFaction> victimChar pointer is null !", return );

	SM_STATIC_PARAMS_1(fbMsgParam, STRING_MANAGER::player);
	fbMsgParam[0].setEId( victimChar->getId() );
	CCharacter::sendDynamicSystemMessage( killer->getId(), "CHARACTER_MADE_THE_FINAL_BLOW", fbMsgParam );
}

//----------------------------------------------------------------------------
void CPVPFaction::characterKillerInPvPFaction( CCharacter * character, PVP_CLAN::TPVPClan winnerFaction, sint32 factionPoint ) const
{
	CPVPFactionHOF::getInstance()->writeStatInHOFDatabase( character, winnerFaction, CPVPFactionHOF::faction_point, factionPoint );
	CPVPFactionHOF::getInstance()->writeStatInHOFDatabase( character, winnerFaction, CPVPFactionHOF::kill, 1 );
}

//----------------------------------------------------------------------------
void CPVPFaction::characterKilledInPvPFaction( CCharacter * character, PVP_CLAN::TPVPClan looserFaction, sint32 factionPoint ) const
{
	CPVPFactionHOF::getInstance()->writeStatInHOFDatabase( character, looserFaction, CPVPFactionHOF::faction_point, factionPoint );
	CPVPFactionHOF::getInstance()->writeStatInHOFDatabase( character, looserFaction, CPVPFactionHOF::lost, 1 );

	character->killedInPVP();
}

