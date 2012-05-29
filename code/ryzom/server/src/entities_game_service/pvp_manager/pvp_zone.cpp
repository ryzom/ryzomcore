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
#include "nel/misc/log.h"
#include "nel/misc/string_conversion.h"
#include "nel/ligo/primitive_utils.h"
#include "game_share/fame.h"

#include "pvp_zone.h"
#include "primitives_parser.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "guild_manager/fame_manager.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_manager/pvp_safe_zone.h"
#include "pvp_manager/pvp.h"
#include "game_event_manager.h"
#include "progression/progression_pvp.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace PVP_ZONE_TYPE
{

	NL_BEGIN_STRING_CONVERSION_TABLE (TPVPZoneType)
		NL_STRING_CONVERSION_TABLE_ENTRY(FreeZone)
		NL_STRING_CONVERSION_TABLE_ENTRY(VersusZone)
		NL_STRING_CONVERSION_TABLE_ENTRY(GuildZone)
		NL_STRING_CONVERSION_TABLE_ENTRY(OutpostZone)
		NL_STRING_CONVERSION_TABLE_ENTRY(Unknown)
	NL_END_STRING_CONVERSION_TABLE(TPVPZoneType, PVPZoneConversion, Unknown)

	//----------------------------------------------------------------------------
	TPVPZoneType fromString(const std::string & str)
	{
		return PVPZoneConversion.fromString(str);
	}

	//----------------------------------------------------------------------------
	const std::string & toString(TPVPZoneType type)
	{
		return PVPZoneConversion.toString(type);
	}

} // namespace PVP_ZONE_TYPE


//----------------------------------------------------------------------------
static inline bool inSameLeague(CCharacter * c1, CCharacter * c2)
{
	if (c1 == NULL || c2 == NULL)
		return false;

	return ( c1 == c2 ) || ( c1->getLeagueId() != DYN_CHAT_INVALID_CHAN && c1->getLeagueId() == c2->getLeagueId() );
}

//----------------------------------------------------------------------------
static inline bool inSameTeam(CCharacter * c1, CCharacter * c2)
{
	if (c1 == NULL || c2 == NULL)
		return false;

	return ( c1 == c2 ) || ( c1->getTeamId() != CTEAM::InvalidTeamId && c1->getTeamId() == c2->getTeamId() );
}

//----------------------------------------------------------------------------
static inline bool inSameGuild(CCharacter * c1, CCharacter * c2)
{
	if (c1 == NULL || c2 == NULL)
		return false;

	return ( c1 == c2 ) || ( c1->getGuildId() != 0 && c1->getGuildId() == c2->getGuildId() );
}

//----------------------------------------------------------------------------
IPVPZone::IPVPZone()
: _Alias(CAIAliasTranslator::Invalid), _CenterX(0), _CenterY(0), _PVPZoneType(PVP_ZONE_TYPE::Unknown), _DeathPenaltyFactor(0)
{
}

#define PRIM_ASSERT(exp) \
	nlassertex( exp, ("<IPVPZone::build> fatal error in primitive: '%s'", NLLIGO::buildPrimPath(zone).c_str() ) )

#define PRIM_VERIFY(exp) \
	nlverifyex( exp, ("<IPVPZone::build> fatal error in primitive: '%s'", NLLIGO::buildPrimPath(zone).c_str() ) )

//----------------------------------------------------------------------------
NLMISC::CSmartPtr<IPVPZone> IPVPZone::build(const NLLIGO::CPrimZone * zone)
{
	nlassert(zone);

	string value;
	PRIM_VERIFY( zone->getPropertyByName("type", value) );

	CSmartPtr<IPVPZone> pvpZone;

	PVP_ZONE_TYPE::TPVPZoneType pvpZoneType = PVP_ZONE_TYPE::fromString(value);
	switch (pvpZoneType)
	{
	case PVP_ZONE_TYPE::FreeZone:
		pvpZone = new CPVPFreeZone;
		break;

	case PVP_ZONE_TYPE::VersusZone:
		{
			string strClan1, strClan2;
			PRIM_VERIFY( zone->getPropertyByName("versus_clan1", strClan1) );
			PRIM_VERIFY( zone->getPropertyByName("versus_clan2", strClan2) );
			PRIM_VERIFY( zone->getPropertyByName("versus_give_faction_points", value) );
			bool giveFP = (value == "true");

			PVP_CLAN::TPVPClan clan1 = PVP_CLAN::fromString(strClan1);
			PVP_CLAN::TPVPClan clan2 = PVP_CLAN::fromString(strClan2);
			PRIM_ASSERT( clan1 >= PVP_CLAN::BeginClans && clan1 <= PVP_CLAN::EndClans );
			PRIM_ASSERT( clan2 >= PVP_CLAN::BeginClans && clan2 <= PVP_CLAN::EndClans );
			PRIM_ASSERT( clan1 != clan2 );

			pvpZone = new CPVPVersusZone(clan1, clan2, giveFP);
		}
		break;

	case PVP_ZONE_TYPE::GuildZone:
		pvpZone = new CPVPGuildZone;
		break;
	}
	PRIM_ASSERT( !pvpZone.isNull() );
	pvpZone->_PVPZoneType = pvpZoneType;

	PRIM_VERIFY( zone->getPropertyByName("disable_zone", value) );
	pvpZone->_Active = (value != "true");

	// Death penalty
	string deathPenaltyFactor;
	PRIM_VERIFY( zone->getPropertyByName("death_penalty_factor", deathPenaltyFactor) );
	pvpZone->_DeathPenaltyFactor = (float)atof( deathPenaltyFactor.c_str() );

	// zone must be defined
	PRIM_ASSERT( !zone->VPoints.empty() );

	// copy prim zone points
	pvpZone->VPoints = zone->VPoints;

	PRIM_VERIFY( zone->getPropertyByName("name", pvpZone->_Name) );

	PRIM_VERIFY( CPrimitivesParser::getAlias(zone, pvpZone->_Alias) );
	PRIM_ASSERT( pvpZone->_Alias != CAIAliasTranslator::Invalid );
 
	// get the bounding box
	float minX = zone->VPoints[0].x;
	float minY = zone->VPoints[0].y;
	float maxX = zone->VPoints[0].x;
	float maxY = zone->VPoints[0].y;
	for (uint i = 1; i < zone->VPoints.size(); i++)
	{
		if (zone->VPoints[i].x < minX) minX = zone->VPoints[i].x;
		if (zone->VPoints[i].y < minY) minY = zone->VPoints[i].y;
		if (zone->VPoints[i].x > maxX) maxX = zone->VPoints[i].x;
		if (zone->VPoints[i].y > maxY) maxY = zone->VPoints[i].y;
	}

	// compute the center of the box
	pvpZone->_CenterX = sint32 ( (minX + maxX) * 1000.f / 2.f );
	pvpZone->_CenterY = sint32 ( (minY + maxY) * 1000.f / 2.f );

	return pvpZone;
}

//----------------------------------------------------------------------------
NLMISC::CSmartPtr<IPVPZone> IPVPZone::buildOutpostZone(const NLLIGO::CPrimZone * zone, NLMISC::CSmartPtr<IPVPZone> pvpZone)
{
	PRIM_ASSERT( pvpZone );
	pvpZone->_PVPZoneType = PVP_ZONE_TYPE::OutpostZone;

	string value;
	//PRIM_VERIFY( zone->getPropertyByName("disable_zone", value) );
	pvpZone->_Active = true; //(value != "true");

	// Death penalty
	string deathPenaltyFactor;
	//PRIM_VERIFY( zone->getPropertyByName("death_penalty_factor", deathPenaltyFactor) );
	pvpZone->_DeathPenaltyFactor = 0; //(float)atof( deathPenaltyFactor.c_str() );

	// zone must be defined
	PRIM_ASSERT( !zone->VPoints.empty() );

	// copy prim zone points
	pvpZone->VPoints = zone->VPoints;

	PRIM_VERIFY( CPrimitivesParser::getAlias(zone, pvpZone->_Alias) );
	PRIM_ASSERT( pvpZone->_Alias != CAIAliasTranslator::Invalid );
 
	// get the bounding box
	float minX = zone->VPoints[0].x;
	float minY = zone->VPoints[0].y;
	float maxX = zone->VPoints[0].x;
	float maxY = zone->VPoints[0].y;
	for (uint i = 1; i < zone->VPoints.size(); i++)
	{
		if (zone->VPoints[i].x < minX) minX = zone->VPoints[i].x;
		if (zone->VPoints[i].y < minY) minY = zone->VPoints[i].y;
		if (zone->VPoints[i].x > maxX) maxX = zone->VPoints[i].x;
		if (zone->VPoints[i].y > maxY) maxY = zone->VPoints[i].y;
	}

	// compute the center of the box
	pvpZone->_CenterX = sint32 ( (minX + maxX) * 1000.f / 2.f );
	pvpZone->_CenterY = sint32 ( (minY + maxY) * 1000.f / 2.f );

	return pvpZone;
}


#undef PRIM_ASSERT
#undef PRIM_VERIFY

//----------------------------------------------------------------------------
bool IPVPZone::overlap(NLMISC::CSmartPtr<IPVPZone> pvpZone1, NLMISC::CSmartPtr<IPVPZone> pvpZone2)
{
	// well.. it is an ugly test
	// TODO : segment intersection checking

	for (uint i = 0; i < pvpZone1->VPoints.size(); i++)
	{
		if ( pvpZone2->contains(pvpZone1->VPoints[i]) )
			return true;
	}

	for (uint i = 0; i < pvpZone2->VPoints.size(); i++)
	{
		if ( pvpZone1->contains(pvpZone2->VPoints[i]) )
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------
IPVPZone::~IPVPZone()
{
}

//----------------------------------------------------------------------------
void IPVPZone::dumpZone(NLMISC::CLog * log, bool dumpUsers) const
{
	if (!log)
		return;

	log->displayNL("Name: '%s'", _Name.c_str());
	log->displayNL("Active: %s", (_Active ? "yes" : "no"));
	log->displayNL("Alias: %s", CPrimitivesParser::aliasToString(_Alias).c_str());
	log->displayNL("Center: x=%d, y=%d", _CenterX / 1000, _CenterY / 1000);
	log->displayNL("Type: %s", PVP_ZONE_TYPE::toString(_PVPZoneType).c_str());
	log->displayNL("Death penalty factor: %g", _DeathPenaltyFactor);

	// TODO : dump safe zones

	if (dumpUsers)
	{
		log->displayNL("Users:");

		std::set<TDataSetRow>::const_iterator it;
		for (it = _Users.begin(); it != _Users.end(); ++it)
		{
			CCharacter * user = PlayerManager.getChar(*it);
			if (!user)
			{
				log->displayNL("\tunknown player row id: %s", (*it).toString().c_str());
				continue;
			}

			log->displayNL("\t%s %s [%s PVP interface]",
				user->getName().toString().c_str(),
				user->getId().toString().c_str(),
				user->getPVPInterface().isValid() ? "valid" : "invalid"
				);
		}
	}
}

//----------------------------------------------------------------------------
bool IPVPZone::contains(const NLMISC::CVector & v, bool excludeSafeZones) const
{
	if ( !CPrimZone::contains(v) )
		return false;
	
	// safe zones are excluded from the PVP zone
	if (excludeSafeZones)
	{
		for (uint i = 0; i < _SafeZones.size(); i++)
		{
			if ( _SafeZones[i]->contains(v) )
				return false;
		}
	}
	
	return true;
}

//----------------------------------------------------------------------------
bool IPVPZone::contains(CCharacter* user, bool excludeSafeZones) const
{
	NLMISC::CVector vect( float(user->getState().X)/1000.f, float(user->getState().Y)/1000.f, 0.f );
	return contains(vect, excludeSafeZones);
}

//----------------------------------------------------------------------------
bool IPVPZone::addSafeZone(NLMISC::CSmartPtr<CPVPSafeZone> safeZone)
{
	nlassert( !safeZone.isNull() );

	if ( !CPrimZone::contains(safeZone->getCenter()) )
		return false;

	for (uint i = 0; i < _SafeZones.size(); i++)
	{
		if ( safeZone->getAlias() == _SafeZones[i]->getAlias() )
			return false;
	}

	_SafeZones.push_back( safeZone );

	return true;
}

//----------------------------------------------------------------------------
void IPVPZone::setActive(bool active)
{
	if (active == _Active)
		return;

	_Active = active;
	egs_pvpinfo("PVP: zone '%s' has been set %s", _Name.c_str(), (_Active ? "active" : "inactive"));

	if (!active)
	{
		// make a copy of _Users because leavePVP() will erase elements of _Users
		std::set<TDataSetRow> users = _Users;
		for (std::set<TDataSetRow>::iterator it = users.begin(); it != users.end(); ++it)
		{
			CCharacter * user = PlayerManager.getChar(*it);
			if (user)
				user->getPVPInterface().leavePVP( IPVP::LeavePVPZone );
		}
		_Users.clear();

#ifdef PVP_DEBUG
		egs_pvpinfo("PVP_DEBUG: cleared _Users");
#endif // PVP_DEBUG
	}
}

//----------------------------------------------------------------------------
void CPVPFreeZone::addPlayer(CCharacter * user)
{
	nlassert(user);

	_Users.insert( user->getEntityRowId() );
}

//----------------------------------------------------------------------------
PVP_RELATION::TPVPRelation CPVPFreeZone::getPVPRelation( CCharacter * user, CEntityBase * target ) const
{
	nlassert(user);
	nlassert(target);

	CCharacter * pTarget = dynamic_cast<CCharacter*>(target);

	bool targetSafe = false;
	bool actorSafe = false;

	if (target->getId().getType() != RYZOMID::player)
	{
		return PVP_RELATION::Unknown;
	}

	if (CPVPManager2::getInstance()->inSafeZone(pTarget->getPosition()))
	{
		if (pTarget->getSafeInPvPSafeZone())
			targetSafe = true;
	}

	if (CPVPManager2::getInstance()->inSafeZone(user->getPosition()))
	{
		if( user->getSafeInPvPSafeZone())
			actorSafe = true;
	}

	if ((targetSafe && !actorSafe) || (actorSafe && !targetSafe)) {
		return PVP_RELATION::NeutralPVP;
	}

	// In same Zone
	if (_Users.find( target->getEntityRowId() ) != _Users.end())
	{
		// In Same Team or League => Ally
		if (inSameTeam( user, pTarget ) || inSameLeague( user, pTarget ))
		{
			return PVP_RELATION::Ally;
		}

		// If both not in safe zone => Ennemy
		if (!targetSafe && !actorSafe)
			return PVP_RELATION::Ennemy;
	}

	return PVP_RELATION::NeutralPVP;
}

//----------------------------------------------------------------------------
bool CPVPFreeZone::leavePVP( CCharacter * user, IPVP::TEndType type )
{
	nlassert(user);

	switch (type)
	{
	case IPVP::LeavePVPZone:
	case IPVP::EnterPVPZone:
		CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_LEAVE_TIMEOUT" );
		break;

	case IPVP::Disconnect:
		break;

	default:
		// ignore all other events
		return false;
	}

	CPVPManager::getInstance()->removeFromLeavingPVPZoneUsers(user->getEntityRowId());
	user->getPVPInterface().reset();

	_Users.erase( user->getEntityRowId() );

	return true;
}

/*
//----------------------------------------------------------------------------
bool CPVPFreeZone::canUserHurtTarget(CCharacter * user, CEntityBase * target) const
{
	nlassert(user);
	nlassert(target);

	// check that target is in the same zone than user (discards bots)
	if ( _Users.find( target->getEntityRowId() ) == _Users.end() )
		return false;

	return true;
}

//----------------------------------------------------------------------------
bool CPVPFreeZone::canUserHelpTarget(CCharacter * user, CEntityBase * target) const
{
	nlassert(user);
	nlassert(target);

	// check that target is in the same zone than user (discards bots)
	if ( _Users.find( target->getEntityRowId() ) == _Users.end() )
		return false;

	return true;
}

//----------------------------------------------------------------------------
bool CPVPFreeZone::canApplyAreaEffect(CCharacter * caster, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const
{
	nlassert(caster);
	nlassert(areaTarget);

	// Allow hitting bots
	if ( offensive && areaTarget->getId().getType() != RYZOMID::player )
		return true;

	// check that areaTarget is in the same zone than user (discards bots)
	if ( _Users.find( areaTarget->getEntityRowId() ) == _Users.end() )
		return false;

	bool apply = false;
	CCharacter * targetChar = PlayerManager.getChar( caster->getTarget() );
	CCharacter * areaTargetChar = static_cast<CCharacter*>(areaTarget);

	if (ignoreMainTarget)
		targetChar = areaTargetChar;

	if ( inSameTeam(targetChar,caster) )
	{
		if (offensive)
			apply = true;
		else
			apply = inSameTeam(areaTargetChar,caster);
	}
	else
	{
		if (offensive)
			apply = !inSameTeam(areaTargetChar,caster);
		else
		{
			apply = true;
		}
	}

	return apply;
}
*/

//----------------------------------------------------------------------------
CPVPVersusZone::CPVPVersusZone(PVP_CLAN::TPVPClan clan1, PVP_CLAN::TPVPClan clan2, bool giveFP)
	: _Clan1(clan1), _Clan2(clan2), _GiveFactionPoints(giveFP)
{
	nlassert( clan1 >= PVP_CLAN::BeginClans && clan1 <= PVP_CLAN::EndClans );
	nlassert( clan2 >= PVP_CLAN::BeginClans && clan2 <= PVP_CLAN::EndClans );
	nlassert( clan1 != clan2 );
}

//----------------------------------------------------------------------------
void CPVPVersusZone::addPlayer(CCharacter * user)
{
	nlassert(user);

	map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::iterator clanIt = _UsersClan.find( user->getId() );
	if ( clanIt == _UsersClan.end() )
	{
		// set player neutral by default
		_UsersClan[user->getId()] = PVP_CLAN::Neutral;

		// update mirror
		setPlayerClanInMirror(user, PVP_CLAN::Neutral);

		// ask player to choose his clan
		//user->openPVPVersusDialog();
		
		// set player clan depending on it's fames
		setPlayerClan( user );
	}
	else
	{
		// set player clan depending on it's fames
		setPlayerClan( user );
		// update mirror
		setPlayerClanInMirror(user, (*clanIt).second);
	}

	_Users.insert( user->getEntityRowId() );
}

//----------------------------------------------------------------------------
void CPVPVersusZone::setActive(bool active)
{
	if (active == _Active)
		return;

	IPVPZone::setActive(active);

	if (!active)
	{
		_AggressedNeutralUsers.clear();
		_UsersClan.clear();

#ifdef PVP_DEBUG
		egs_pvpinfo("PVP_DEBUG: cleared _AggressedNeutralUsers and _UsersClan");
#endif // PVP_DEBUG
	}
}

//----------------------------------------------------------------------------
void CPVPVersusZone::giveFactionPoints(bool giveFP)
{
	if (giveFP == _GiveFactionPoints)
		return;

	_GiveFactionPoints = giveFP;
	egs_pvpinfo("PVP: zone '%s' now %s faction points", getName().c_str(), (_GiveFactionPoints ? "gives" : "does not give"));

	if (!giveFP)
	{
		// if zone does not give faction points anymore, remove all players in the zone from PvP progression system
		for (std::set<TDataSetRow>::iterator it = _Users.begin(); it != _Users.end(); ++it)
		{
			CCharacter * user = PlayerManager.getChar(*it);
			if (user != NULL)
				PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->removePlayer(user);
		}
	}
}

//----------------------------------------------------------------------------
static inline sint32 fameToPercent(sint32 fame)
{
	return sint32( float(fame) / FameAbsoluteMax * 100 );
}

//----------------------------------------------------------------------------
static inline sint32 fameFromPercent(sint32 percent)
{
	return percent * (FameAbsoluteMax / 100);
}


//----------------------------------------------------------------------------
PVP_CLAN::TPVPClan CPVPVersusZone::getPlayerClan(CCharacter * user, PVP_CLAN::TPVPClan clan1, PVP_CLAN::TPVPClan clan2 )
{
	nlassert(user);

	if( clan1 == PVP_CLAN::Unknown || clan2 == PVP_CLAN::Unknown )
	{
		return PVP_CLAN::Unknown;
	}

	// get fame of player in each clan
	const sint32 fame1 = CFameInterface::getInstance().getFameIndexed( user->getId(), PVP_CLAN::getFactionIndex(clan1), false, true );
	sint32 fame2 = CFameInterface::getInstance().getFameIndexed( user->getId(), PVP_CLAN::getFactionIndex(clan2), false, true );
	
	// find his clan
	PVP_CLAN::TPVPClan clan = determinatePlayerClan( user, clan1, fame1, clan2, fame2 );

	return clan;
}

//----------------------------------------------------------------------------
bool CPVPVersusZone::isOverridedByARunningEvent( CCharacter * user )
{
	
	bool event = CGameEventManager::getInstance().isGameEventRunning();
	PVP_CLAN::TPVPClan eventClan1 = PVP_CLAN::fromString( CGameEventManager::getInstance().getFaction1() );
	PVP_CLAN::TPVPClan eventClan2 = PVP_CLAN::fromString( CGameEventManager::getInstance().getFaction2() );
	bool zoneOnly = CGameEventManager::getInstance().isFactionChanelInZoneOnly();
	
	if( event && eventClan1==_Clan1 && eventClan2==_Clan2 && !zoneOnly )
	{
		if( user->isChannelAdded() )
			return true;
	}
	return false;
}

//----------------------------------------------------------------------------
bool CPVPVersusZone::setPlayerClan(CCharacter * user/*, PVP_CLAN::TPVPClan clan*/)
{
	nlassert(user);
/*
	BOMB_IF( (clan == PVP_CLAN::None), "clan 'None' is not allowed", return false );
	BOMB_IF( (clan != PVP_CLAN::Neutral && clan != _Clan1 && clan != _Clan2), "invalid clan for this pvp zone", return false );
*/
	// find the clan
	PVP_CLAN::TPVPClan clan = getPlayerClan(user, _Clan1, _Clan2 );
	//nldebug("<CPVPVersusZone::setPlayerClan> clan=%s",PVP_CLAN::toString(clan).c_str());
	if( clan == PVP_CLAN::Unknown )
	{
		return false;
	}

	// channel are added only if there is not an event with same faction flaged no zone only
	if( ! isOverridedByARunningEvent( user ) )
	{
		// add channel faction
		if( clan == _Clan1 )
		{
			CGameEventManager::getInstance().addCharacterToChannelFactionEvent( user, 1 );
		}
		else if( clan == _Clan2 )
		{
			CGameEventManager::getInstance().addCharacterToChannelFactionEvent( user, 2 );
		}
	}
	else
	{
		//nldebug("<CPVPVersusZone::setPlayerClan> overrided by an event");
	}

	map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::iterator clanIt = _UsersClan.find( user->getId() );

	// player is not in this zone
	if ( clanIt == _UsersClan.end() )
		return false;

	// player already is in this clan
	if ( (*clanIt).second == clan )
		return true;

	// set clan
	(*clanIt).second = clan;

	// update mirror
	setPlayerClanInMirror(user, clan);

#ifdef PVP_DEBUG
	egs_pvpinfo("PVP_DEBUG: player %s now is in clan %s", user->getName().toString().c_str(), PVP_CLAN::toString( clan ).c_str() );
#endif // PVP_DEBUG

	if (clan != PVP_CLAN::Neutral)
	{
		const PVP_CLAN::TPVPClan rivalClan = (clan == _Clan1) ? _Clan2 : _Clan1;
		
		// if player is not neutral anymore, clear his aggressors
		_AggressedNeutralUsers.erase( user->getEntityRowId() );

/*
		// update fame if necessary
		const uint32 faction		= PVP_CLAN::getFactionIndex(clan);
		const uint32 rivalFaction	= PVP_CLAN::getFactionIndex(rivalClan);
		const sint32 fame			= CFameInterface::getInstance().getFameIndexed( user->getId(), faction, false, true );
		const sint32 rivalFame		= CFameInterface::getInstance().getFameIndexed( user->getId(), rivalFaction, false, true );

		if (fame != NO_FAME && rivalFame != NO_FAME)
		{
			const sint32 famePercent = fameToPercent(fame);
			const sint32 rivalFamePercent = fameToPercent(rivalFame);

			// rival faction fame must be negative and lower than faction fame on client side (percentage of fame)
			if (rivalFamePercent >= famePercent || rivalFamePercent >= 0)
			{
				const sint32 newRivalFamePercent = min( -1, famePercent - 1 );
				const sint32 newRivalFame = fameFromPercent(newRivalFamePercent);

				CFameManager::getInstance().setPlayerFame(user->getId(), rivalFaction, newRivalFame);

#ifdef PVP_DEBUG
				egs_pvpinfo("PVP_DEBUG: fame of player %s for faction %s has been changed from %d (%d) to %d (%d)",
							user->getName().toString().c_str(),
							PVP_CLAN::toString( rivalClan ).c_str(),
							rivalFame, rivalFamePercent,
							newRivalFame, newRivalFamePercent
							);
#endif // PVP_DEBUG
			}
		}
*/	}

	return true;
}

//----------------------------------------------------------------------------
PVP_CLAN::TPVPClan CPVPVersusZone::determinatePlayerClan( CCharacter *user, PVP_CLAN::TPVPClan clan1, sint32 fame1, PVP_CLAN::TPVPClan clan2, sint32 fame2 )
{
	if( fame1 > fame2 )
	{
		return clan1;
	}
	else if( fame1 != fame2 )
	{
		return clan2;
	}
	else
	{
		return PVP_CLAN::Neutral;
	}
}

//----------------------------------------------------------------------------
void CPVPVersusZone::setPlayerClanInMirror(CCharacter * user, PVP_CLAN::TPVPClan clan) const
{
	BOMB_IF( clan >= PVP_CLAN::NbClans, "invalid clan!", return );
	
	if (!user)
		return;
	
	const TDataSetRow & rowId = user->getEntityRowId();
	
	if ( !TheDataset.isAccessible(rowId) )
		return;
	
	CMirrorPropValue<TYPE_PVP_CLAN> propPvpClan( TheDataset, rowId, DSPropertyPVP_CLAN );
	propPvpClan = (uint8) clan;
	user->updateGuildFlag();
}

//----------------------------------------------------------------------------
PVP_CLAN::TPVPClan CPVPVersusZone::getCharacterClan( const NLMISC::CEntityId& character ) const
{
	map<CEntityId,PVP_CLAN::TPVPClan>::const_iterator it = _UsersClan.find( character );
	if( it != _UsersClan.end() )
	{
		return (*it).second;
	}
	return PVP_CLAN::Neutral;		
}

//----------------------------------------------------------------------------
bool CPVPVersusZone::leavePVP(CCharacter * user, IPVP::TEndType type)
{
	nlassert(user);

	switch (type)
	{
	case IPVP::LeavePVPZone:
	case IPVP::EnterPVPZone:
		{
			// reset user clan only if he leaves the zone
			// disconnecting in the zone will not work (anti-exploit)
			map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::iterator it = _UsersClan.find( user->getId() );
			if( it != _UsersClan.end() )
			{
				if( user->haveAnyPrivilege() == false )
				{
					if( ! isOverridedByARunningEvent( user ) )
					{	
						if( (*it).second == _Clan1 )
						{
							CGameEventManager::getInstance().removeCharacterToChannelFactionEvent( user, 1 );
						}
						else if( (*it).second == _Clan2 )
						{
							CGameEventManager::getInstance().removeCharacterToChannelFactionEvent( user, 2 );
						}
					}
				}
			}
			
			_UsersClan.erase( it );

			// update mirror
			setPlayerClanInMirror(user, PVP_CLAN::None);

			if( type != EnterPVPZone )
			CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_LEAVE_TIMEOUT" );
		}
		break;

	case IPVP::Disconnect:
		break;

	default:
		// ignore all other events
		return false;
	}

	_Users.erase( user->getEntityRowId() );
	_AggressedNeutralUsers.erase( user->getEntityRowId() );

	CPVPManager::getInstance()->removeFromLeavingPVPZoneUsers(user->getEntityRowId());
	user->getPVPInterface().reset();

	// reset PvP progression data of a player when he leaves a 'Faction PvP' zone
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->removePlayer(user);

	return true;
}

//----------------------------------------------------------------------------
void CPVPVersusZone::userHurtsTarget(CCharacter * user, CCharacter * target)
{
	nlassert(user);
	nlassert(target);

	// check that target is in the same zone than user
	if ( _Users.find( target->getEntityRowId() ) == _Users.end() )
		return;

	map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::const_iterator targetClanIt = _UsersClan.find( target->getId() );
	BOMB_IF( targetClanIt == _UsersClan.end(), "PVP: player must have a clan!", return );

	const PVP_CLAN::TPVPClan targetClan = (*targetClanIt).second;
	if (targetClan != PVP_CLAN::Neutral)
		return;

	// flag for known if aggressor are already aggressed the target
	bool aggressorAlreadyKnown = false;

	map<TDataSetRow,TAggressors>::const_iterator it = _AggressedNeutralUsers.find( target->getEntityRowId() );
	if( it != _AggressedNeutralUsers.end() )
	{
		if( (*it).second.find( user->getEntityRowId() ) == (*it).second.end() )
		{
			aggressorAlreadyKnown = true;
		}
	}

	if( aggressorAlreadyKnown == false )
	{
		// add aggressor of neutral target
		_AggressedNeutralUsers[target->getEntityRowId()].insert( user->getEntityRowId() );
	}
	else if( target->getTarget() == user->getId() )
	{
		// Update neutral bot chat programme
		target->setTargetBotchatProgramm( user, user->getId() );
	}
	
#ifdef PVP_DEBUG
	map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::const_iterator userClanIt = _UsersClan.find( user->getId() );
	BOMB_IF( userClanIt == _UsersClan.end(), "PVP: player must have a clan!", return );

	const PVP_CLAN::TPVPClan userClan = (*userClanIt).second;

	egs_pvpinfo("PVP_DEBUG: player %s (%s) hurts neutral player %s",
		user->getName().toString().c_str(),
		PVP_CLAN::toString( userClan ).c_str(),
		target->getName().toString().c_str()
		);
#endif // PVP_DEBUG
}

//----------------------------------------------------------------------------
PVP_RELATION::TPVPRelation CPVPVersusZone::getPVPRelation( CCharacter * user, CEntityBase * target ) const
{
	nlassert(user);
	nlassert(target);

	if( target->getId().getType() != RYZOMID::player )
	{
		return PVP_RELATION::Unknown;
	}

	map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::const_iterator userClanIt = _UsersClan.find( user->getId() );
	BOMB_IF( userClanIt == _UsersClan.end(), "PVP: player must have a clan!", return PVP_RELATION::Neutral );
	
	// check that target is in the same zone than user (discards bots)
	if ( _Users.find( target->getEntityRowId() ) != _Users.end() )
	{
		map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::const_iterator targetClanIt = _UsersClan.find( target->getId() );
		BOMB_IF( targetClanIt == _UsersClan.end(), "PVP: player must have a clan!", return PVP_RELATION::Neutral );

		const PVP_CLAN::TPVPClan userClan = (*userClanIt).second;
		const PVP_CLAN::TPVPClan targetClan = (*targetClanIt).second;
	
		if (userClan == PVP_CLAN::Neutral)
		{
			// neutral players can only attack their aggressors
			map<TDataSetRow,TAggressors>::const_iterator it = _AggressedNeutralUsers.find( user->getEntityRowId() );
			if (it != _AggressedNeutralUsers.end() )
			{
				const TAggressors & aggressors = (*it).second;
				if ( aggressors.find( target->getEntityRowId() ) != aggressors.end() )
					return PVP_RELATION::Ennemy;
			}
		}
		else
		{
			if (userClan != targetClan)
			{
				return PVP_RELATION::Ennemy;
			}
			if (userClan == targetClan)
			{
				return PVP_RELATION::Ally;
			}
		}
	}

	// if target is in versus pvp then he's neutral pvp
	const CCharacter * targetChar = static_cast<const CCharacter*>(target);
	if( targetChar->getPVPInterface().isValid() )
	{
		if( targetChar->getPVPInterface().getPVPSession()->getPVPMode() == PVP_MODE::PvpZoneFaction );
		{
			return PVP_RELATION::NeutralPVP;
		}
	}

	return PVP_RELATION::Neutral;
}
/*
//----------------------------------------------------------------------------
bool CPVPVersusZone::canUserHurtTarget(CCharacter * user, CEntityBase * target) const
{
	nlassert(user);
	nlassert(target);

	// check that target is in the same zone than user (discards bots)
	if ( _Users.find( target->getEntityRowId() ) == _Users.end() )
		return false;

	map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::const_iterator userClanIt = _UsersClan.find( user->getId() );
	BOMB_IF( userClanIt == _UsersClan.end(), "PVP: player must have a clan!", return false );

	map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::const_iterator targetClanIt = _UsersClan.find( target->getId() );
	BOMB_IF( targetClanIt == _UsersClan.end(), "PVP: player must have a clan!", return false );

	const PVP_CLAN::TPVPClan userClan = (*userClanIt).second;
	const PVP_CLAN::TPVPClan targetClan = (*targetClanIt).second;

	if (userClan == PVP_CLAN::Neutral)
	{
		// neutral players can only attack their aggressors
		map<TDataSetRow,TAggressors>::const_iterator it = _AggressedNeutralUsers.find( user->getEntityRowId() );
		if (it != _AggressedNeutralUsers.end() )
		{
			const TAggressors & aggressors = (*it).second;
			if ( aggressors.find( target->getEntityRowId() ) != aggressors.end() )
				return true;
		}
	}
	else if (userClan != targetClan)
	{
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
bool CPVPVersusZone::canUserHelpTarget(CCharacter * user, CEntityBase * target) const
{
	nlassert(user);
	nlassert(target);

	// check that target is in the same zone than user (discards bots)
	if ( _Users.find( target->getEntityRowId() ) == _Users.end() )
		return false;

	map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::const_iterator userClanIt = _UsersClan.find( user->getId() );
	BOMB_IF( userClanIt == _UsersClan.end(), "PVP: player must have a clan!", return false );

	map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::const_iterator targetClanIt = _UsersClan.find( target->getId() );
	BOMB_IF( targetClanIt == _UsersClan.end(), "PVP: player must have a clan!", return false );

	const PVP_CLAN::TPVPClan userClan = (*userClanIt).second;
	const PVP_CLAN::TPVPClan targetClan = (*targetClanIt).second;

	// everyone can help neutral players
	if (targetClan == PVP_CLAN::Neutral)
		return true;

	return (userClan == targetClan);
}

//----------------------------------------------------------------------------
bool CPVPVersusZone::canApplyAreaEffect(CCharacter * caster, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const
{
	nlassert(caster);
	nlassert(areaTarget);

	if (caster == areaTarget)
		return !offensive;

	// Allow hitting bots
	if ( offensive && areaTarget->getId().getType() != RYZOMID::player )
		return true;

	if (offensive)
	{
		return canUserHurtTarget(caster, areaTarget);
	}

	return canUserHelpTarget(caster, areaTarget);
}
*/

//----------------------------------------------------------------------------
void CPVPVersusZone::dumpZone(NLMISC::CLog * log, bool dumpUsers) const
{
	if (!log)
		return;

	IPVPZone::dumpZone(log, false);
	log->displayNL("Clans: '%s' VS '%s'", PVP_CLAN::toString(_Clan1).c_str(), PVP_CLAN::toString(_Clan2).c_str());
	log->displayNL("Give faction points: %s", _GiveFactionPoints ? "true" : "false");

	if (dumpUsers)
	{
		log->displayNL("Users:");

		std::set<TDataSetRow>::const_iterator it;
		for (it = _Users.begin(); it != _Users.end(); ++it)
		{
			CCharacter * user = PlayerManager.getChar(*it);
			if (!user)
			{
				log->displayNL("\tunknown player row id: %s", (*it).toString().c_str());
				continue;
			}

			string clan;
			map<NLMISC::CEntityId,PVP_CLAN::TPVPClan>::const_iterator userClanIt = _UsersClan.find( user->getId() );
			if ( userClanIt != _UsersClan.end() )
				clan = PVP_CLAN::toString( (*userClanIt).second );
			else
				clan = "not found!";

			log->displayNL("\t%s %s [%s PVP interface] [clan=%s]",
				user->getName().toString().c_str(),
				user->getId().toString().c_str(),
				user->getPVPInterface().isValid() ? "valid" : "invalid",
				clan.c_str()
				);

			map<TDataSetRow,TAggressors>::const_iterator aggressedUserIt = _AggressedNeutralUsers.find( user->getEntityRowId() );
			if ( aggressedUserIt != _AggressedNeutralUsers.end() )
			{
				log->displayNL("\tAggressors:");

				TAggressors::const_iterator aggressorIt;
				for (aggressorIt = (*aggressedUserIt).second.begin(); aggressorIt != (*aggressedUserIt).second.end(); ++aggressorIt)
				{
					CCharacter * aggressor = PlayerManager.getChar(*aggressorIt);
					if (!aggressor)
					{
						log->displayNL("\t\t| unknown player row id: %s", (*aggressorIt).toString().c_str());
						continue;
					}
					log->displayNL("\t\t| %s", aggressor->getName().toString().c_str());
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
void CPVPGuildZone::addPlayer(CCharacter * user)
{
	nlassert(user);

	_Users.insert( user->getEntityRowId() );
}

//----------------------------------------------------------------------------
bool CPVPGuildZone::leavePVP( CCharacter * user, IPVP::TEndType type )
{
	nlassert(user);

	switch (type)
	{
	case IPVP::LeavePVPZone:
	case IPVP::EnterPVPZone:
		CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_LEAVE_TIMEOUT" );
		break;

	case IPVP::Disconnect:
		break;

	default:
		// ignore all other events
		return false;
	}

	CPVPManager::getInstance()->removeFromLeavingPVPZoneUsers(user->getEntityRowId());
	user->getPVPInterface().reset();

	_Users.erase( user->getEntityRowId() );

	return true;
}


//----------------------------------------------------------------------------
PVP_RELATION::TPVPRelation CPVPGuildZone::getPVPRelation( CCharacter * user, CEntityBase * target ) const
{
	nlassert(user);
	nlassert(target);

	CCharacter * pTarget = dynamic_cast<CCharacter*>(target);

	bool targetSafe = false;
	bool actorSafe = false;

	if( target->getId().getType() != RYZOMID::player )
	{
		return PVP_RELATION::Unknown;
	}

	if (CPVPManager2::getInstance()->inSafeZone(pTarget->getPosition()))
	{
		if (pTarget->getSafeInPvPSafeZone())
			targetSafe = true;
	}

	if( CPVPManager2::getInstance()->inSafeZone(user->getPosition()))
	{
		if( user->getSafeInPvPSafeZone())
			actorSafe = true;
	}

	if ((targetSafe && !actorSafe) || (actorSafe && !targetSafe)) {
		return PVP_RELATION::NeutralPVP;
	}

	// if target is in same zone then he's an ennemy or ally 
	if ( _Users.find( target->getEntityRowId() ) != _Users.end() )
	{
		// In Same Team or League => Ally
		if (inSameTeam( user, pTarget ) || inSameLeague( user, pTarget ))
		{
			return PVP_RELATION::Ally;
		}
		
		// in same Guild and not in Leagues => Ally 
		if (inSameGuild( user, pTarget ) && user->getLeagueId() == DYN_CHAT_INVALID_CHAN && pTarget->getLeagueId() == DYN_CHAT_INVALID_CHAN)
		{
			return PVP_RELATION::Ally;
		}

		// If both not in safe zone => Ennemy
		if (!targetSafe && !actorSafe)
			return PVP_RELATION::Ennemy;
	}

	return PVP_RELATION::NeutralPVP;
}
	
/*
//----------------------------------------------------------------------------
bool CPVPGuildZone::canUserHurtTarget(CCharacter * user, CEntityBase * target) const
{
	nlassert(user);
	nlassert(target);

	// check that target is in the same zone than user (discards bots)
	if ( _Users.find( target->getEntityRowId() ) == _Users.end() )
		return false;

	return true;
}

//----------------------------------------------------------------------------
bool CPVPGuildZone::canUserHelpTarget(CCharacter * user, CEntityBase * target) const
{
	nlassert(user);
	nlassert(target);

	// check that target is in the same zone than user (discards bots)
	if ( _Users.find( target->getEntityRowId() ) == _Users.end() )
		return false;

	CCharacter *targetChar = dynamic_cast<CCharacter*>(target);
	if ( targetChar )
	{
		if ( inSameTeam(targetChar,user) || inSameGuild(targetChar,user) )
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------
bool CPVPGuildZone::canApplyAreaEffect(CCharacter * caster, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const
{
	nlassert(caster);
	nlassert(areaTarget);

	// Allow hitting bots
	if ( offensive && areaTarget->getId().getType() != RYZOMID::player )
		return true;

	// check that areaTarget is in the same zone than user (discards bots)
	if ( _Users.find( areaTarget->getEntityRowId() ) == _Users.end() )
		return false;

	bool apply = false;
	CCharacter *targetChar, *areaTargetChar = static_cast<CCharacter*>(areaTarget);
	if (ignoreMainTarget)
		targetChar = areaTargetChar; // only use area target if ignoreMainTarget is true
	else
		targetChar = PlayerManager.getChar( caster->getTarget() );

	if ( inSameTeam(targetChar,caster) )
	{
		if ( inSameGuild(targetChar,caster) )
		{
			if (offensive)
				apply = true;
			else
				apply = inSameTeam(areaTargetChar,caster) || inSameGuild(areaTargetChar,caster);
		}
		else
		{
			if (offensive)
				apply = inSameTeam(areaTargetChar,caster) || !inSameGuild(areaTargetChar,caster);
			else
				apply = inSameTeam(areaTargetChar,caster) || inSameGuild(areaTargetChar,caster);
		}
	}
	else
	{
		if ( inSameGuild(targetChar,caster) )
		{
			if (offensive)
				apply = !inSameTeam(areaTargetChar,caster);
			else
				apply = inSameGuild(areaTargetChar,caster);
		}
		else
		{
			if (offensive)
				apply = !( inSameTeam(areaTargetChar,caster) || ( !inSameTeam(areaTargetChar,targetChar) && inSameGuild(areaTargetChar,caster) ) );
			else
			{
				BOMB_IF (!ignoreMainTarget, "PVP: you should not be able to cast a defensive spell on an enemy!", return false);
			}
		}
	}

	return apply;
}
*/

//----------------------------------------------------------------------------
bool IPVPZone::hasDeathPenaltyFactorForVictimsOf( CEntityBase *killer ) const
{
	return (killer->getId().getType() == RYZOMID::player);
}

//----------------------------------------------------------------------------
bool CPVPOutpostZone::hasDeathPenaltyFactorForVictimsOf( CEntityBase *killer ) const
{
	// Note: this quick test does not use the outpost rules (inside zone, opponent...)
	// (kills by creatures not from the outpost are included)
	uint entityType = (uint)killer->getId().getType();
	return (entityType == RYZOMID::player) || (entityType == RYZOMID::npc) || (entityType == RYZOMID::creature);
}
