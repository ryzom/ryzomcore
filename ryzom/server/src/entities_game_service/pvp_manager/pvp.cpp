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

#include "nel/misc/hierarchical_timer.h"

#include "pvp_manager/pvp.h"
#include "team_manager/team.h"
#include "team_manager/team_manager.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "zone_manager.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_zone.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CPVPInterface);

//----------------------------------------------------------------------------
CPVPInterface::CPVPInterface(CCharacter * owner) : _Owner(owner)
{
	nlassert(owner);
}

//----------------------------------------------------------------------------
bool CPVPInterface::leavePVP(IPVP::TEndType type)
{
	H_AUTO(CPVPInterface_leavePVP);

	nlassert( isValid() );

#ifdef PVP_DEBUG
	IPVPZone * pvpZone = dynamic_cast<IPVPZone *>( &*_PVPSession );
#endif // PVP_DEBUG

	const bool result = _PVPSession->leavePVP(_Owner, type);
	if (result)
	{
#ifdef PVP_DEBUG
		if (pvpZone)
		{
			egs_pvpinfo("PVP_DEBUG: player %s has really left PVP zone '%s' %s",
				_Owner->getName().toString().c_str(), pvpZone->getName().c_str(), result ? "[ok]" : "[error]"
				);
		}
		else
		{
			egs_pvpinfo("PVP_DEBUG: player %s has really left PVP session %s",
				_Owner->getName().toString().c_str(), result ? "[ok]" : "[error]"
				);
		}
#endif // PVP_DEBUG
	}

	return result;
}

//----------------------------------------------------------------------------
bool CPVPInterface::doCancelRespawn()
{
	H_AUTO(CPVPInterface_doCancelRespawn);

	nlassert( isValid() );

	return _PVPSession->doCancelRespawn();
}

/*
//----------------------------------------------------------------------------
bool CPVPInterface::canHurt(CEntityBase * target) const
{
	H_AUTO(CPVPInterface_canHurt);

	nlassert(target);
	nlassert( isValid() );

	// player cannot attack himself
	if (_Owner == target)
		return false;

	// if the target is a character, it must be in pvp mode
	if ( target->getId().getType() == RYZOMID::player )
	{
		CCharacter *targetChar = static_cast<CCharacter*>(target);
		if ( ! targetChar->getPVPInterface().isValid() )
			return false;
	}

	// cannot hurt a dead entity
	if ( target->isDead() )
		return false;

	return _PVPSession->canUserHurtTarget(_Owner, target);
}

//----------------------------------------------------------------------------
bool CPVPInterface::canHelp(CEntityBase * target) const
{
	H_AUTO(CPVPInterface_canHelp);

	nlassert(target);
	nlassert( isValid() );

	// player can always help himself
	if (_Owner == target)
		return true;

	// if the target is a character, it must be in pvp mode
	if ( target->getId().getType() == RYZOMID::player )
	{
		CCharacter *targetChar = static_cast<CCharacter*>(target);
		if ( ! targetChar->getPVPInterface().isValid() )
			return false;
	}

	return _PVPSession->canUserHelpTarget(_Owner, target);
}

//----------------------------------------------------------------------------
bool CPVPInterface::canApplyAreaEffect(CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const
{
	H_AUTO(CPVPInterface_canApplyAreaEffect);

	nlassert(areaTarget);
	nlassert( isValid() );

	// if the target is a character, it must be in pvp mode
	if ( areaTarget->getId().getType() == RYZOMID::player )
	{
		CCharacter *areaTargetChar = static_cast<CCharacter*>(areaTarget);
		if ( ! areaTargetChar->getPVPInterface().isValid() )
			return false;
	}

	// cannot hurt a dead entity
	if ( offensive && areaTarget->isDead() )
		return false;

	const bool result = _PVPSession->canApplyAreaEffect(_Owner, areaTarget, offensive, ignoreMainTarget);

#ifdef PVP_DEBUG
	egs_pvpinfo("PVP_DEBUG: player %s %s apply his %s area effect on player %s",
		_Owner->getName().toString().c_str(),
		result ? "can" : "cannot",
		offensive ? "offensive" : "defensive",
		areaTarget->getName().toString().c_str()
		);
#endif // PVP_DEBUG

	return result;
}
*/

//----------------------------------------------------------------------------
void CPVPInterface::hurt(CCharacter * target)
{
	H_AUTO(CPVPInterface_hurt);

	nlassert(target);
	nlassert( isValid() );

	if ( !target->getPVPInterface().isValid() )
		return;

	_PVPSession->userHurtsTarget(_Owner, target);
}

//-----------------------------------------------------------------------------
bool CPVPInterface::killedBy(CEntityBase * killer)
{
	H_AUTO(CPVPInterface_killedBy);

	nlassert(killer);
	nlassert( isValid() );

	if ( getPVPSession() )
	{
		IPVPZone * pvpZone = dynamic_cast<IPVPZone *>( getPVPSession() );
		if ( pvpZone &&
			 pvpZone->isCharacterInConflict( _Owner ) && // only users engaged in war will benefit from the DP factor
			 pvpZone->hasDeathPenaltyFactorForVictimsOf( killer ) )
		{
			_Owner->setNextDeathPenaltyFactor( PVPZoneWithDeathPenalty.get() ? pvpZone->deathPenaltyFactor() : 0 );
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
bool CPVPInterface::getPvpClan( PVP_CLAN::TPVPClan& clan1, PVP_CLAN::TPVPClan& clan2 ) const
{
	H_AUTO(CPVPInterface_getPvpClan);

	nlassert( isValid() );

	CPVPVersusZone * versusZone = dynamic_cast<CPVPVersusZone *>(&*_PVPSession);
	if( versusZone != 0 )
	{
		clan1 = versusZone->getClan( 1 );
		clan2 = versusZone->getClan( 2 );
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void CPVPInterface::setPVPModeInMirror() const
{
	TYPE_PVP_MODE pvpMode;
	if ( !TheDataset.isAccessible(_Owner->getEntityRowId()) )
		return;

	CMirrorPropValue<TYPE_EVENT_FACTION_ID> propPvpMode( TheDataset, _Owner->getEntityRowId(), DSPropertyEVENT_FACTION_ID );

	if (_PVPSession)
	{
		pvpMode = _PVPSession->getPVPMode();
		pvpMode |= propPvpMode;
	}
	else
	{
		pvpMode = ~( PVP_MODE::PvpChallenge | PVP_MODE::PvpZoneFree | PVP_MODE::PvpZoneGuild | PVP_MODE::PvpZoneOutpost );
		pvpMode &= propPvpMode;
	}
	propPvpMode = pvpMode;
}
