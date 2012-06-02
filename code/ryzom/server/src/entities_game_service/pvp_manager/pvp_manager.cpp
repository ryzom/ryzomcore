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

#include "nel/net/service.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/log.h"
#include "server_share/r2_variables.h"

#include "pvp_manager/pvp_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "team_manager/team.h"
#include "team_manager/team_manager.h"
#include "zone_manager.h"
#include "egs_globals.h"
#include "primitives_parser.h"
#include "pvp_manager/pvp_safe_zone.h"
#include "pvp_manager/pvp.h"
#include "outpost_manager/outpost.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLLIGO;

extern CGenericXmlMsgHeaderManager	GenericMsgManager;
CPVPManager * CPVPManager::_Instance = NULL;

CVariable<bool> DisablePVPChallenge("egs", "DisablePVPChallenge", "If true PVP challenge is disabled", false, 0, true );

//----------------------------------------------------------------------------
void CPVPManager::init()
{
	nlassert( _Instance == NULL );
	_Instance = new CPVPManager;
	_Instance->_FirstFreeIslandIdx = 0;
}

//----------------------------------------------------------------------------
void CPVPManager::initPVPIslands()
{
	CConfigFile::CVar& zones = IService::getInstance()->ConfigFile.getVar("ChallengeSpawnZones");
	for ( uint i = 0; (sint)i<zones.size(); ++i)
	{
		if (zones.asString(i) != "")
		{
			uint16 id = CZoneManager::getInstance().getTpSpawnZoneIdByName(zones.asString(i));
			const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(id);
			if ( zone == NULL )
				nlwarning("<PVP> Invalid spawn zone %s",zones.asString(i).c_str());
			else
				_Instance->_ChallengeZones.push_back(zone);
		}
	}
}

//----------------------------------------------------------------------------
void CPVPManager::release()
{
	nlassert( _Instance );
	delete _Instance;
	_Instance = NULL;
}

//----------------------------------------------------------------------------
void CPVPManager::tickUpdate()
{
	// remove expired PVP propositions
	while( !_PVPChallengesAsked.empty() &&  _PVPChallengesAsked.front().ExpirationDate <= CTickEventHandler::getGameCycle() )
	{		
		_PVPChallengesAsked.erase(_PVPChallengesAsked.begin() );
	}

	// remove expired PVP zone leave time buffer
	while( !_UsersLeavingPVPZone.empty() &&  _UsersLeavingPVPZone.front().first <= CTickEventHandler::getGameCycle() )
	{	
		// remove from PVP zone
		CCharacter * user = PlayerManager.getChar( _UsersLeavingPVPZone.front().second );

#ifdef PVP_DEBUG
		if (user)
		{
			IPVPZone * zone = getPVPZone( user->getCurrentPVPZone() );
			if (zone)
				egs_pvpinfo("PVP_DEBUG: player %s has left PVP zone '%s'", user->getName().toString().c_str(), zone->getName().c_str() );
		}
#endif // PVP_DEBUG

		if ( !user || !user->getPVPInterface().isValid() || !user->getPVPInterface().leavePVP( IPVP::LeavePVPZone ) )
		{
			// remove entry ( if user is found, interface manages to remove itself )
			_UsersLeavingPVPZone.erase( _UsersLeavingPVPZone.begin() );

#ifdef PVP_DEBUG
			if (user)
				egs_pvpinfo("PVP_DEBUG: player %s has been removed from _UsersLeavingPVPZone", user->getName().toString().c_str() );
#endif // PVP_DEBUG
		}
	}

	// remove expired PVP zone enter time buffer
	while( !_UsersEnteringPVPZone.empty() &&  _UsersEnteringPVPZone.front().first <= CTickEventHandler::getGameCycle() )
	{
		CCharacter * user = PlayerManager.getChar( _UsersEnteringPVPZone.front().second.RowId );
		if (user)
			doEnterPVPZone(user, _UsersEnteringPVPZone.front().second.ZoneAlias);

		_UsersEnteringPVPZone.erase( _UsersEnteringPVPZone.begin() );

#ifdef PVP_DEBUG
		if (user)
			egs_pvpinfo("PVP_DEBUG: player %s has been removed from _UsersEnteringPVPZone", user->getName().toString().c_str() );
#endif // PVP_DEBUG
	}

	// every seconds, check warning messages to send
	const TGameCycle time = CTickEventHandler::getGameCycle();
	if (time % 10 == 0)
	{
		if (PVPZoneLeaveBufferTime > PVPZoneWarningRepeatTime)
		{
			std::list<std::pair<NLMISC::TGameCycle,TDataSetRow> >::iterator it;
			for (it = _UsersLeavingPVPZone.begin(); it != _UsersLeavingPVPZone.end(); ++it)
			{
				const uint32 remainingTimeInSeconds = ((*it).first - time) / 10;
				if (remainingTimeInSeconds > (PVPZoneLeaveBufferTime / 10) - (PVPZoneWarningRepeatTimeL / 10) || remainingTimeInSeconds == 0)
					continue;
				if (remainingTimeInSeconds > (PVPZoneLeaveBufferTime / 10) - (PVPZoneWarningRepeatTime / 10) || remainingTimeInSeconds == 0)
					continue;

				// must do the test in seconds because this is called only once a second
				if ( (remainingTimeInSeconds % (PVPZoneWarningRepeatTime / 10) == 0 && remainingTimeInSeconds <= 60 )
				  || (remainingTimeInSeconds % (PVPZoneWarningRepeatTimeL / 10) == 0 && remainingTimeInSeconds > 60 ) )
				{
					CCharacter * user = PlayerManager.getChar( (*it).second );
					if (!user)
						continue;

					// send client message about time before player becomes attackable
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
					params[0].Int = remainingTimeInSeconds;
					CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_LEAVE_TIME_REPEAT", params );
				}
			}
		}

		if (PVPZoneEnterBufferTime > PVPZoneWarningRepeatTime)
		{
			std::list<std::pair<NLMISC::TGameCycle,CPVPZonePendingUser> >::iterator it;
			for (it = _UsersEnteringPVPZone.begin(); it != _UsersEnteringPVPZone.end(); ++it)
			{
				const uint32 remainingTimeInSeconds = ((*it).first - time) / 10;
				if (remainingTimeInSeconds > (PVPZoneEnterBufferTime / 10) - (PVPZoneWarningRepeatTime / 10) || remainingTimeInSeconds == 0)
					continue;

				// must do the test in seconds because this is called only once a second
				if ( remainingTimeInSeconds % (PVPZoneWarningRepeatTime / 10) == 0 )
				{
					CCharacter * user = PlayerManager.getChar( (*it).second.RowId );
					if (!user)
						continue;

					// send client message about time before player becomes attackable
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
					params[0].Int = remainingTimeInSeconds;
					CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_ENTER_TIME_REPEAT", params );
				}
			}
		}
	}
}



/*
//----------------------------------------------------------------------------
bool CPVPManager::testCurativeAction( CCharacter* actor, CEntityBase * target )
{
	nlassert(actor);
	nlassert(target);

	// if actor is not in PVP mode (outside a zone...)
	if ( ! actor->getPVPInterface().isValid() )
	{
		if ( target->getId().getType() == RYZOMID::player )
		{
			// if target is in PVP mode, actor cannot help target (anti-exploit)
			CCharacter *targetChar = static_cast<CCharacter*>(target);
			if ( targetChar->getPVPInterface().isValid() )
				return false;
		}
		else
		{
			// if target is a bot, can't help it
			return false;
		}

		return true;
	}

	return actor->getPVPInterface().canHelp( target );
}

//----------------------------------------------------------------------------
bool CPVPManager::testOffensiveAction( CCharacter* actor, CEntityBase * target )
{
	nlassert(actor);
	nlassert(target);

	if ( !actor->getPVPInterface().isValid() )
		return false;

	return actor->getPVPInterface().canHurt( target );
}

//----------------------------------------------------------------------------
bool CPVPManager::canApplyAreaEffect(CCharacter* actor, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const
{
	nlassert(actor);
	nlassert(areaTarget);

	if( offensive )
	{
		if( actor->getPVPInterface().isValid() )
			return false;
		return actor->getPVPInterface().canApplyAreaEffect( areaTarget, offensive, ignoreMainTarget );
	}
	else
	{
		if( ! actor->getPVPInterface().isValid() )
		{
			if( areaTarget->getId().getType() == RYZOMID::player )
			{
				CCharacter *targetChar = static_cast<CCharacter*>(areaTarget);
				if( targetChar->getPVPInterface().isValid() )
					return false;
			}
			else
			{
				return false;
			}
			return true;
		}
		return actor->getPVPInterface().canApplyAreaEffect( areaTarget, offensive, ignoreMainTarget );
	}
}
*/

//----------------------------------------------------------------------------
void CPVPManager::addPVPZone( NLMISC::CSmartPtr<IPVPZone> pvpZone )
{
	nlassert( !pvpZone.isNull() );

	// check that pvpZone is not in the manager yet and that it does not overlap another PVP zone (dirty test)
	for (uint i = 0; i < _PVPZones.size(); i++)
	{
		nlassert( _PVPZones[i]->getAlias() != pvpZone->getAlias() );

		if ( IPVPZone::overlap(pvpZone, _PVPZones[i]) )
		{
			nlwarning( "PRIM_ERROR: PVP zones '%s' and '%s' intersect", pvpZone->getName().c_str(), _PVPZones[i]->getName().c_str() );
			return;
		}
	}

	_PVPZones.push_back( pvpZone );
}

//----------------------------------------------------------------------------
void CPVPManager::addPVPSafeZone( NLMISC::CSmartPtr<CPVPSafeZone> safeZone )
{
	nlassert( !safeZone.isNull() );

	for (uint i = 0; i < _PVPZones.size(); i++)
	{
		if ( _PVPZones[i]->addSafeZone(safeZone) )
			break;
	}
}

//----------------------------------------------------------------------------
void CPVPManager::applyConfigToPVPZones()
{
	egs_pvpinfo("Apply config to PVP zones...");

	// set active zones
	CConfigFile::CVar * varPtr = IService::getInstance()->ConfigFile.getVarPtr("ActivePVPZones");
	if (varPtr)
	{
		for (uint i = 0; i < varPtr->size(); i++)
		{
			string zoneName = varPtr->asString(i);
			IPVPZone * pvpZone = CPVPManager::getInstance()->getPVPZone(zoneName);
			if (!pvpZone)
			{
				egs_pvpinfo("Unknown PVP zone: %s", zoneName.c_str());
				continue;
			}

			pvpZone->setActive(true);
			egs_pvpinfo("PVP zone '%s' is now active", zoneName.c_str());
		}
	}

	// set inactive zones
	varPtr = IService::getInstance()->ConfigFile.getVarPtr("InactivePVPZones");
	if (varPtr)
	{
		for (uint i = 0; i < varPtr->size(); i++)
		{
			string zoneName = varPtr->asString(i);
			IPVPZone * pvpZone = CPVPManager::getInstance()->getPVPZone(zoneName);
			if (!pvpZone)
			{
				egs_pvpinfo("Unknown PVP zone: %s", zoneName.c_str());
				continue;
			}

			pvpZone->setActive(false);
			egs_pvpinfo("PVP zone '%s' is now inactive", zoneName.c_str());
		}
	}

	// set zones giving faction points
	varPtr = IService::getInstance()->ConfigFile.getVarPtr("PVPZonesGiveFactionPoints");
	if (varPtr)
	{
		for (uint i = 0; i < varPtr->size(); i++)
		{
			string zoneName = varPtr->asString(i);
			CPVPVersusZone * versusZone = dynamic_cast<CPVPVersusZone *>(CPVPManager::getInstance()->getPVPZone(zoneName));
			if (!versusZone)
			{
				egs_pvpinfo("Unknown faction PVP zone: %s", zoneName.c_str());
				continue;
			}

			versusZone->giveFactionPoints(true);
			egs_pvpinfo("Faction PVP zone '%s' now gives faction points", zoneName.c_str());
		}
	}

	// set zones not giving faction points
	varPtr = IService::getInstance()->ConfigFile.getVarPtr("PVPZonesDoNotGiveFactionPoints");
	if (varPtr)
	{
		for (uint i = 0; i < varPtr->size(); i++)
		{
			string zoneName = varPtr->asString(i);
			CPVPVersusZone * versusZone = dynamic_cast<CPVPVersusZone *>(CPVPManager::getInstance()->getPVPZone(zoneName));
			if (!versusZone)
			{
				egs_pvpinfo("Unknown faction PVP zone: %s", zoneName.c_str());
				continue;
			}

			versusZone->giveFactionPoints(false);
			egs_pvpinfo("Faction PVP zone '%s' now does not give faction points", zoneName.c_str());
		}
	}
}

//----------------------------------------------------------------------------
TAIAlias CPVPManager::getPVPZoneFromUserPosition( CCharacter *user ) const
{
	H_AUTO(CPVPManager_getPVPZoneFromPosition);

	CVector vect( user->getState().X * 0.001f, user->getState().Y * 0.001f, 0.f );

	for (uint i = 0; i < _PVPZones.size(); i++)
	{
		IPVPZone * pvpZone = _PVPZones[i];
		if( pvpZone->contains(vect) )
		{
			if( pvpZone->isActive() )
			{
				if( pvpZone->isCharacterInConflict(user) )
				{
					return pvpZone->getAlias();
				}
				else
				{
					// verbose log
					//egs_pvpinfo("<CPVPManager::getPVPZoneFromUserPosition> user not in conflict");
				}
			}
			else
			{
				// verbose log
				//egs_pvpinfo("<CPVPManager::getPVPZoneFromUserPosition> zone inactive");
			}
		}
	}

	return CAIAliasTranslator::Invalid;
}

//----------------------------------------------------------------------------
void CPVPManager::leavePVPZone( CCharacter * user )
{
	H_AUTO(CPVPManager_leavePVPZone);

	nlassert(user);

	if( user->havePriv(":DEV:")==false && user->haveAnyPrivilege() && user->priviledgePVP() == false )
		return;

	// if user was entering the zone, just remove his pending entry
	if ( removeFromEnteringPVPZoneUsers(user->getEntityRowId()) )
	{
		CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_LEAVE_BEFORE_TIMEOUT" );
		return;
	}

	// check that user is not already leaving a PVP zone
	bool found = false;
	std::list<std::pair<NLMISC::TGameCycle,TDataSetRow> >::iterator it;
	for (it = _UsersLeavingPVPZone.begin(); it != _UsersLeavingPVPZone.end(); ++it)
	{
		if ( (*it).second == user->getEntityRowId() )
		{
			STOP("PVP: user should not be already leaving a PVP zone!");
			return;
		}
	}

	IPVPZone * zone = getPVPZone( user->getCurrentPVPZone() );
	if( zone )
	{
		if( zone->getPVPZoneType() != PVP_ZONE_TYPE::OutpostZone )
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = PVPZoneLeaveBufferTime / 10;
			CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_LEAVE_TIME", params );

			// add user to leaving PVP zone users
			NLMISC::TGameCycle endDate = CTickEventHandler::getGameCycle() + PVPZoneLeaveBufferTime;
			_UsersLeavingPVPZone.push_back( make_pair( endDate, user->getEntityRowId() ) );
			
#ifdef PVP_DEBUG
			egs_pvpinfo("PVP_DEBUG: player %s has been added to _UsersLeavingPVPZone", user->getName().toString().c_str() );
			BOMB_IF( zone == NULL, "PVP_DEBUG: user was not in a PVP zone!", return );
			egs_pvpinfo("PVP_DEBUG: player %s is leaving PVP zone '%s'", user->getName().toString().c_str(), zone->getName().c_str() );
#endif // PVP_DEBUG
		}
		else
		{
			if( user->getOutpostAlias() == zone->getAlias() )
			{
				user->startOutpostLeavingTimer();
			}
			else
			{
				COutpost * outpost = dynamic_cast<COutpost*>(zone);
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::outpost);
				params[0].SheetId = outpost->getSheet();
				CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "OUTPOST_LEAVE_ZONE", params );
			}
		}
	}


}

//----------------------------------------------------------------------------
void CPVPManager::enterPVPZone( CCharacter * user, TAIAlias pvpZoneAlias )
{
	H_AUTO(CPVPManager_enterPVPZone);

	nlassert(user);

	if( user->havePriv(":DEV:")==false && user->haveAnyPrivilege() && user->priviledgePVP() == false )
		return;

	IPVPZone * zone = getPVPZone( pvpZoneAlias );
	if ( !zone )
	{
		nlwarning("PVP: %s enters invalid PVP zone with alias %s", user->getId().toString().c_str(), CPrimitivesParser::aliasToString(pvpZoneAlias).c_str() );
		return;
	}
	

	IPVPZone * oldZone = dynamic_cast<IPVPZone *>( user->getPVPInterface().getPVPSession() );
	if (oldZone)
	{
		std::list<std::pair<NLMISC::TGameCycle,TDataSetRow> >::iterator it;
		for (it = _UsersLeavingPVPZone.begin(); it != _UsersLeavingPVPZone.end(); ++it)
		{
			// is user leaving a pvp zone ?
			if ( (*it).second == user->getEntityRowId() )
			{
				// is user entering the same zone he left ? if yes just let him enter back an reset timer
				if (oldZone->getAlias() == pvpZoneAlias)
				{
					_UsersLeavingPVPZone.erase(it);
					CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_ENTER_BACK" );
#ifdef PVP_DEBUG
					egs_pvpinfo("PVP_DEBUG: player %s has been removed from _UsersLeavingPVPZone", user->getName().toString().c_str() );
#endif // PVP_DEBUG
					return;
				}
				else
				{
					// if player goes from versus zone to another then he becomes pvp immediately in the new zone
					if( oldZone->getPVPZoneType() == PVP_ZONE_TYPE::VersusZone && zone->getPVPZoneType() == PVP_ZONE_TYPE::VersusZone )
					{
						CPVPVersusZone * oldVersusZone = dynamic_cast<CPVPVersusZone *>( oldZone );
						CPVPVersusZone * newVersusZone = dynamic_cast<CPVPVersusZone *>( zone );
						BOMB_IF( oldVersusZone == 0 || newVersusZone == 0, "Dynamic cast return 0, problem between object class type and zone type returned by object", break );

						if( (oldVersusZone->getClan( 1 ) == newVersusZone->getClan( 1 ) &&
							oldVersusZone->getClan( 2 ) == newVersusZone->getClan( 2 ) )
							||
							(oldVersusZone->getClan( 1 ) == newVersusZone->getClan( 2 ) &&
							oldVersusZone->getClan( 2 ) == newVersusZone->getClan( 1 ) ))
						{
							_UsersLeavingPVPZone.erase(it);
							doEnterPVPZone(user, pvpZoneAlias);
							return;
						}
					}

					// if user is changing of PVP zone, force him to leave the old zone right now
					// warning: all PVP zones should accept the leave event "EnterPVPZone"
					if ( !user->getPVPInterface().leavePVP( IPVP::EnterPVPZone ) )
						_UsersLeavingPVPZone.erase(it);
					
					break;
				}
			}
		}

		// (outpost use its own timer(doesn't use _UsersLeavingPVPZone) )
		if( oldZone->getPVPZoneType() == PVP_ZONE_TYPE::OutpostZone )
		{
			// check if he's re-entering into the same outpost
			if( pvpZoneAlias == oldZone->getAlias() )
			{
				egs_pvpinfo("<CPVPManager::enterPVPZone> re-entering outpost %s",CPrimitivesParser::aliasToString(pvpZoneAlias).c_str());
				user->stopOutpostLeavingTimer();
				CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_ENTER_BACK" );
			}
		}
	}
	else
	{
		// necessary if leaving a pvp zone whithout being a pvp mode
		removeFromLeavingPVPZoneUsers(user->getEntityRowId());
	}

	// ensure that user is not already entering a PVP zone
	removeFromEnteringPVPZoneUsers( user->getEntityRowId() );

	if( zone->getPVPZoneType() != PVP_ZONE_TYPE::OutpostZone )
	{
		// add user to entering PVP zone users
		NLMISC::TGameCycle endDate = CTickEventHandler::getGameCycle() + PVPZoneEnterBufferTime;
		CPVPZonePendingUser pendingUser;
		pendingUser.RowId = user->getEntityRowId();
		pendingUser.ZoneAlias = pvpZoneAlias;
		_UsersEnteringPVPZone.push_back( make_pair(endDate,pendingUser) );

#ifdef PVP_DEBUG
		egs_pvpinfo("PVP_DEBUG: player %s has been added to _UsersEnteringPVPZone", user->getName().toString().c_str() );
#endif // PVP_DEBUG

		// send client message about the zone user entered in
		user->enterPVPZone( zone->getPVPZoneType() );

		// send client message about time before player becomes attackable
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = PVPZoneEnterBufferTime / 10;
		CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_ENTER_TIME", params );

#ifdef PVP_DEBUG
		egs_pvpinfo("PVP_DEBUG: player %s is entering in PVP zone '%s'", user->getName().toString().c_str(), zone->getName().c_str());
#endif // PVP_DEBUG
	}
	else if( user->getEnterFlag() )
	{
		// entering into an outpost zone
		egs_pvpinfo("<CPVPManager::enterPVPZone> entering outpost %s",CPrimitivesParser::aliasToString(pvpZoneAlias).c_str());
		user->outpostOpenChooseSideDialog( pvpZoneAlias );
		user->setOutpostAliasBeforeUserValidation( pvpZoneAlias );
	}
}

//----------------------------------------------------------------------------
void CPVPManager::doEnterPVPZone(CCharacter * user, TAIAlias pvpZoneAlias)
{
	H_AUTO(CPVPManager_doEnterPVPZone);

	nlassert(user);

	// if user has another PVP session
	if ( user->getPVPInterface().isValid() )
	{
		// try to leave it
		user->getPVPInterface().leavePVP(IPVP::EnterPVPZone);
		if ( user->getPVPInterface().isValid() )
		{
			CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_CANNOT_ENTER");
			return;
		}
	}

	IPVPZone * zone = getPVPZone( pvpZoneAlias );
	if ( !zone )
	{
		nlwarning("PVP: %s enters invalid PVP zone with alias %s", user->getId().toString().c_str(), CPrimitivesParser::aliasToString(pvpZoneAlias).c_str() );
		return;
	}

	CCharacter::sendDynamicSystemMessage( user->getEntityRowId(), "PVP_ZONE_ENTER_TIMEOUT" );

	user->getPVPInterface().init( zone );
	zone->addPlayer( user );

#ifdef PVP_DEBUG
	egs_pvpinfo("PVP_DEBUG: player %s has entered in PVP zone '%s'", user->getName().toString().c_str(), zone->getName().c_str() );
#endif // PVP_DEBUG
}

//----------------------------------------------------------------------------
bool CPVPManager::removeFromLeavingPVPZoneUsers(TDataSetRow rowId)
{
	std::list<std::pair<NLMISC::TGameCycle,TDataSetRow> >::iterator it;
	for (it = _UsersLeavingPVPZone.begin(); it != _UsersLeavingPVPZone.end(); ++it)
	{
		if ( (*it).second == rowId )
		{
			_UsersLeavingPVPZone.erase(it);

#ifdef PVP_DEBUG
			CCharacter * user = PlayerManager.getChar(rowId);
			if (user)
				egs_pvpinfo("PVP_DEBUG: player %s has been removed from _UsersLeavingPVPZone", user->getName().toString().c_str() );
#endif // PVP_DEBUG

			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
bool CPVPManager::removeFromEnteringPVPZoneUsers(TDataSetRow rowId)
{
	std::list<std::pair<NLMISC::TGameCycle,CPVPZonePendingUser> >::iterator it;
	for (it = _UsersEnteringPVPZone.begin(); it != _UsersEnteringPVPZone.end(); ++it)
	{
		if ( (*it).second.RowId == rowId )
		{
			_UsersEnteringPVPZone.erase(it);

#ifdef PVP_DEBUG
			CCharacter * user = PlayerManager.getChar(rowId);
			if (user)
				egs_pvpinfo("PVP_DEBUG: player %s has been removed from _UsersEnteringPVPZone", user->getName().toString().c_str() );
#endif // PVP_DEBUG

			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
void CPVPManager::askForPVPChallenge( const NLMISC::CEntityId & userId )
{
	if (DisablePVPChallenge.get())
	{
		CCharacter::sendDynamicSystemMessage(userId, "PVP_CHALLENGE_DISABLED");
		return;
	}

	// global chat parameters
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	
	// get protagonists
	CCharacter * user = PlayerManager.getChar( userId );
	if ( !user )
	{
		nlwarning("<PVP_CHALLENGE>'%s' is invalid",userId.toString().c_str() );
		return;
	}
	CCharacter * target = PlayerManager.getChar( user->getTarget() );
	if ( !target )
	{
		nlwarning("<PVP_CHALLENGE>'%s' has an invalid target",user->getTarget().toString().c_str() );
		return;
	}

	CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, user->getEntityRowId(), DSPropertyCELL );
	sint32 cell1 = mirrorCell;	
	CMirrorPropValueRO<TYPE_CELL> mirrorCell2( TheDataset, target->getEntityRowId(), DSPropertyCELL );
	sint32 cell2 = mirrorCell2;	

	if  (cell1 <= -2 || cell2 <= -2 )
	{
		CCharacter::sendDynamicSystemMessage( userId, "NO_CHALLENGE_HERE");
		return;
	}



	CPVPChallengeAsked entry;
	entry.InvitedTeam = CTEAM::InvalidTeamId;
	entry.Invitor = user->getEntityRowId();
	
	// user must not be in PVP
	if ( user->getPVPInterface().isValid() )
	{
		params[0].setEIdAIAlias( user->getTarget(), CAIAliasTranslator::getInstance()->getAIAlias(user->getTarget()) );
		CCharacter::sendDynamicSystemMessage( userId, "DUEL_YOU_ALREADY_IN_PVP",params);
		return;
	}
	// cant invite if aggro
	if ( user->getAggroCount() )
	{
		CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_YOU_AGGRO");
		return;
	}


	// nobody in the user team must be in PVP
	CTeam * teamUser = TeamManager.getRealTeam( user->getTeamId() );

	if ( teamUser )
	{
		// cant invite team mate
		if ( user->getTeamId() == target->getTeamId() )
		{
			params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()) );
			CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_SAME_TEAM",params);
			return;
		}		
		for ( list<CEntityId>::const_iterator it = teamUser->getTeamMembers().begin(); it != teamUser->getTeamMembers().end(); ++it )
		{
		 	CCharacter * c = PlayerManager.getChar( *it );
			if ( c )
			{
				if ( c->getPVPInterface().isValid() )
				{
					params[0].setEIdAIAlias( (*it), CAIAliasTranslator::getInstance()->getAIAlias((*it)) );
					CCharacter::sendDynamicSystemMessage( userId, "DUEL_TEAM_MEMBER_ALREADY_IN_PVP",params);
					return;
				}
				if ( c->getAggroCount() )
				{
					params[0].setEIdAIAlias( (*it), CAIAliasTranslator::getInstance()->getAIAlias((*it)) );
					CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_TEAM_AGGRO",params);
					return;
				}
			}
		}
	}

	// nobody in the target team must be in PVP
	CEntityId msgTargetEId;
	CTeam * teamTarget = TeamManager.getRealTeam( target->getTeamId() );
	if ( teamTarget )
	{
		for ( list<CEntityId>::const_iterator it = teamTarget->getTeamMembers().begin(); it != teamTarget->getTeamMembers().end(); ++it )
		{
			CCharacter * c = PlayerManager.getChar( *it );
			if ( c )
			{
				if ( c->getPVPInterface().isValid() )
				{
				params[0].setEIdAIAlias( (*it), CAIAliasTranslator::getInstance()->getAIAlias((*it)) );
					CCharacter::sendDynamicSystemMessage( userId, "DUEL_TARGET_TEAM_ALREADY_IN_PVP",params);
					return;
				}
				if ( c->getAggroCount() )
				{
					CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_OTHER_TEAM_AGGRO");
					return;
				}
			}
		}
		entry.InvitedTeam = teamTarget->getTeamId();
		msgTargetEId = teamTarget->getLeader();
	}
	else if ( target->getPVPInterface().isValid() )
	{
		params[0].setEIdAIAlias( user->getTarget(), CAIAliasTranslator::getInstance()->getAIAlias(user->getTarget()) );
		CCharacter::sendDynamicSystemMessage( userId, "DUEL_TARGET_ALREADY_IN_PVP",params);
		return;
	}
	else if ( target->getAggroCount() )
	{
		params[0].setEIdAIAlias( user->getTarget(), CAIAliasTranslator::getInstance()->getAIAlias(user->getTarget()) );
		CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_TARGET_AGGRO",params);
		return;
	}
	else
	{
		msgTargetEId = user->getTarget();
		entry.InvitedUser = target->getEntityRowId();
	}
	

	// remove previous invitation, and check that user and target are not invited
	bool problem = false;
	
	TDataSetRow userRow = TheDataset.getDataSetRow( userId );
	for ( std::list< CPVPChallengeAsked >::iterator it = _PVPChallengesAsked.begin(); it != _PVPChallengesAsked.end(); )
	{
		if ( (*it).Invitor == userRow )
		{
			// Here the user previously invited someone
			// ignore same invitation
			if ( (*it).InvitedUser == target->getEntityRowId() )
				return;
			
			// send cancel message to the previous invited
			CMessage msgout( "IMPULSION_ID" );
			CEntityId receiverId;
			if ( (*it).InvitedTeam != CTEAM::InvalidTeamId )
			{
				CTeam * team = TeamManager.getRealTeam( (*it).InvitedTeam );
				if ( !team )
				{
					nlwarning("<PVP_CHALLENGE> user %s previously invited team %u which is invalid. That could mean that the invitation was not propely removed when team was destroyed?",userId.toString().c_str(),(*it).InvitedTeam);
					_PVPChallengesAsked.erase(it);
					return;
				}
				receiverId = team->getLeader();
			}
			else
				receiverId = getEntityIdFromRow( (*it).InvitedUser );
			
			msgout.serial( receiverId );
			CBitMemStream bms;
			nlverify ( GenericMsgManager.pushNameToStream( "PVP_CHALLENGE:CANCEL_INVITATION", bms) );
			msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
			sendMessageViaMirror( NLNET::TServiceId(receiverId.getDynamicId()), msgout );
			
			//send chat infos
			params[0].setEIdAIAlias( receiverId, CAIAliasTranslator::getInstance()->getAIAlias(receiverId) );
			CCharacter::sendDynamicSystemMessage( userRow, "DUEL_YOU_CANCEL_INVITATION",params);
			
			// remove this proposition
			std::list< CPVPChallengeAsked >::iterator itErase = it;
			++it;
			_PVPChallengesAsked.erase(itErase);
			
		}
		else
		{
			if ( (*it).InvitedUser == user->getEntityRowId() || ( user->getTeamId() != CTEAM::InvalidTeamId && (*it).InvitedTeam == user->getTeamId() ) )
			{	
				// user is already invited : he has to accept or refuse first
				CCharacter::sendDynamicSystemMessage( userId, "DUEL_ALREADY_INVITED",params);
				// dont bail out as we can enter in case "if ( (*it).Invitor == userId )"
				problem = true;
			}
			if ( (*it).InvitedUser == target->getEntityRowId() || ( target->getTeamId() != CTEAM::InvalidTeamId && (*it).InvitedTeam == target->getTeamId() ) )
			{	
				// user is already invited : he has to accept or refuse first
				params[0].setEIdAIAlias( target->getId(), CAIAliasTranslator::getInstance()->getAIAlias(target->getId()) );
				CCharacter::sendDynamicSystemMessage( userId, "DUEL_TARGET_ALREADY_INVITED",params);
				// dont bail out as we can enter in case "if ( (*it).Invitor == userId )"
				problem = true;
			}
			++it;
		}
	}
	// problem occured : bail out
	if ( problem )
		return;
	
	entry.ExpirationDate = CTickEventHandler::getGameCycle() + DuelQueryDuration;
	_PVPChallengesAsked.push_front( entry );
	
	// tell invited player
	params[0].setEIdAIAlias( userId, CAIAliasTranslator::getInstance()->getAIAlias(userId) );
	uint32 txt = STRING_MANAGER::sendStringToClient( target->getEntityRowId(), "PVP_CHALLENGE_INVITATION", params );
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( const_cast<CEntityId&>(msgTargetEId) );
	CBitMemStream bms;
	nlverify ( GenericMsgManager.pushNameToStream( "PVP_CHALLENGE:INVITATION", bms) );
	bms.serial( txt );
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	sendMessageViaMirror( NLNET::TServiceId(msgTargetEId.getDynamicId()), msgout );
}

//----------------------------------------------------------------------------
void CPVPManager::acceptPVPChallenge( const NLMISC::CEntityId & userId )
{	
	BOMB_IF(IsRingShard,"acceptPVPChallenge() - not allowed on Ring shards because PVP Challenge requires non-ring GPMS",return);

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	CCharacter * invited = PlayerManager.getChar(userId ); 
	if ( !invited )
	{
		nlwarning("<PVP>invalid user %s", userId.toString().c_str() );
		return;
	}

	// the invited player must still be the leader of its team
	CTeam * team1 = TeamManager.getTeam( invited->getTeamId() );
	if ( team1 )
	{
		if ( team1->getLeader() != userId )
		{
			params[0].setEIdAIAlias( userId, CAIAliasTranslator::getInstance()->getAIAlias(userId) );
			CCharacter::sendDynamicSystemMessage( userId, "PVP_NO_MORE_LEADER",params);
			return;
		}
		bool stop = false;
		for ( list<CEntityId>::const_iterator it = team1->getTeamMembers().begin(); it != team1->getTeamMembers().end(); ++it )
		{
			//removeDuelInvitor( *it );
			CCharacter * c = PlayerManager.getChar( *it );
			if ( c )
			{
				if ( c->getAggroCount() )
				{
					CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_TEAM_AGGRO");
					return;
				}


				CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, c->getEntityRowId(), DSPropertyCELL );
				sint32 cell = mirrorCell;		
				if  (cell <= -2  )
				{
					CCharacter::sendDynamicSystemMessage( userId, "NO_CHALLENGE_HERE");
					return;
				}
			}
		}
	}
	else
	{
		if ( invited->getAggroCount() )
		{
			CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_YOU_AGGRO");
			return;
		}
		CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, invited->getEntityRowId(), DSPropertyCELL );
		sint32 cell = mirrorCell;		
		if  (cell <= -2  )
		{
			CCharacter::sendDynamicSystemMessage( userId, "NO_CHALLENGE_HERE");
			return;
		}
		//removeDuelInvitor( invited->getId() );
	}

	// do the same tests for the invitor
	CCharacter * invitor = NULL;
	for ( std::list< CPVPChallengeAsked >::iterator it = _PVPChallengesAsked.begin(); it != _PVPChallengesAsked.end(); )
	{
		// we have to find the user as the invited of a duel to start the duel
		if ( (*it).InvitedUser == invited->getEntityRowId() || ( team1 && invited->getTeamId() ==  (*it).InvitedTeam ) )
		{
			invitor = PlayerManager.getChar( (*it).Invitor );
			if ( !invitor )
			{
				nlwarning("<PVP>invalid invitor %s", getEntityIdFromRow( (*it).Invitor ).toString().c_str() );
				_PVPChallengesAsked.erase(it);
				return;
			}
			

			// ignore deprecated invitation
			if ( invited->getPVPInterface().isValid() || invitor->getPVPInterface().isValid() )
			{
				std::list< CPVPChallengeAsked >::iterator itErase = it;
				++it;
				_PVPChallengesAsked.erase(itErase);
				continue;
			}
			std::list< CPVPChallengeAsked >::iterator itErase = it;
			++it;
			_PVPChallengesAsked.erase(itErase);
		}
		// if the user proposed a duel, cancel the proposition
		else if ( (*it).Invitor == invited->getEntityRowId() )
		{
			params[0].setEIdAIAlias( userId, CAIAliasTranslator::getInstance()->getAIAlias(userId) );
			CCharacter::sendDynamicSystemMessage( (*it).Invitor, "DUEL_CANCEL_INVITATION",params);
			std::list< CPVPChallengeAsked >::iterator itErase = it;
			++it;
			_PVPChallengesAsked.erase(itErase);
		}
		else
			++it;
	}

	if ( invitor )
	{
		CTeam * team2 = TeamManager.getTeam( invitor->getTeamId() );
		if ( team2 )
		{
			for ( list<CEntityId>::const_iterator it = team2->getTeamMembers().begin(); it != team2->getTeamMembers().end(); ++it )
			{
				CCharacter * c = PlayerManager.getChar( *it );
				if ( c )
				{
					if ( c->getAggroCount() )
					{
						CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_OTHER_TEAM_AGGRO");
						return;
					}
				}
				CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, c->getEntityRowId(), DSPropertyCELL );
				sint32 cell = mirrorCell;		
				if  (cell <= -2  )
				{
					CCharacter::sendDynamicSystemMessage( userId, "NO_CHALLENGE_HERE");
					return;
				}
				//removeDuelInvitor( *it );
			}
		}
		else
		{
			//removeDuelInvitor( invited->getId() );
			if ( invitor->getAggroCount() )
			{
				CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_TARGET_AGGRO");
				return;
			}
			CMirrorPropValueRO<TYPE_CELL> mirrorCell( TheDataset, invitor->getEntityRowId(), DSPropertyCELL );
			sint32 cell = mirrorCell;		
			if  (cell <= -2  )
			{
				CCharacter::sendDynamicSystemMessage( userId, "NO_CHALLENGE_HERE");
				return;
			}
		}
		CPVPChallenge * challenge = new CPVPChallenge( invitor,invited );

		SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
		params[0].setEIdAIAlias( userId, CAIAliasTranslator::getInstance()->getAIAlias(userId) );
		CCharacter::sendDynamicSystemMessage( invitor->getId(), "CHALLENGE_ACCEPTED", params);
		params[0].setEIdAIAlias( invitor->getId(), CAIAliasTranslator::getInstance()->getAIAlias(invitor->getId()) );
		CCharacter::sendDynamicSystemMessage( userId, "CHALLENGE_ACCEPTED", params);
	}
}

//----------------------------------------------------------------------------
void CPVPManager::refusePVPChallenge( const NLMISC::CEntityId & userId )
{
	CCharacter * user = PlayerManager.getChar( userId );
	if ( !user )
	{
		nlwarning("<PVP>Invalid user %s",userId.toString().c_str());
		return;
	}

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	params[0].setEIdAIAlias( userId, CAIAliasTranslator::getInstance()->getAIAlias(userId) );

	CTeam * team = TeamManager.getTeam( user->getTeamId() );
	
	// find the proposition
	for ( std::list< CPVPChallengeAsked >::iterator it = _PVPChallengesAsked.begin(); it != _PVPChallengesAsked.end();++it )
	{
		if ( (*it).InvitedUser == user->getEntityRowId() || ( team && team->getLeader() == userId && (*it).InvitedTeam == user->getTeamId()  ) )
		{
			CCharacter::sendDynamicSystemMessage( (*it).Invitor, "DUEL_REFUSE_INVITATION", params);
			_PVPChallengesAsked.erase(it);
			return;
		}
	}
}

//----------------------------------------------------------------------------
void CPVPManager::removePVPChallengeInvitor( const NLMISC::CEntityId & userId )
{
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	params[0].setEIdAIAlias( userId, CAIAliasTranslator::getInstance()->getAIAlias(userId) );
	TDataSetRow row = TheDataset.getDataSetRow( userId );
	// find invitation, send messages and remove it
	for ( std::list< CPVPChallengeAsked >::iterator it = _PVPChallengesAsked.begin(); it != _PVPChallengesAsked.end();++it )
	{
		if ( (*it).Invitor == row )
		{	
			CEntityId msgId;
			if ( (*it).InvitedTeam != CTEAM::InvalidTeamId )
			{
				CTeam * team = TeamManager.getTeam( (*it).InvitedTeam );
				if (team )
					msgId = team->getLeader();
			}
			else
				msgId = getEntityIdFromRow ( (*it).InvitedUser );
			if (msgId != CEntityId::Unknown )
			{
				CMessage msgout( "IMPULSION_ID" );
				msgout.serial( msgId );
				CBitMemStream bms;
				nlverify ( GenericMsgManager.pushNameToStream( "PVP_CHALLENGE:CANCEL_INVITATION", bms) );
				msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
				sendMessageViaMirror( NLNET::TServiceId(msgId.getDynamicId()), msgout );
				CCharacter::sendDynamicSystemMessage( msgId, "DUEL_CANCEL_INVITATION",params);
			}
			_PVPChallengesAsked.erase(it);
			return;
		}
	}
}

//----------------------------------------------------------------------------
sint32 CPVPManager:: allocatePVPIsland(CPVPChallenge* challenge)
{
#ifdef NL_DEBUG
	static const uint allocStep = 1;
#else
	static const uint allocStep = 128;
#endif
	// reallocate vector if necessary
	const uint oldSize = (uint)_ChallengeIslands.size();
	if ( _FirstFreeIslandIdx >= oldSize )
	{
		const uint newSize = oldSize + allocStep;
		_ChallengeIslands.resize( newSize );
		for ( uint i = oldSize; i < newSize; i++ )
		{
			_ChallengeIslands[i].first = i + 1;
			_ChallengeIslands[i].second = NULL;
			//allocate the cell in GPMS ( cells are >0 here )
			NLNET::CMessage msgout("CREATE_INDOOR_UNIT");

			sint32 cellId = - getIslandCellFromIdx(i);
			msgout.serial(cellId);
			sendMessageViaMirror ("GPMS", msgout);
		}
	}
	sint32 ret = getIslandCellFromIdx(_FirstFreeIslandIdx);
	_ChallengeIslands[_FirstFreeIslandIdx].second = challenge;
	_FirstFreeIslandIdx = _ChallengeIslands[_FirstFreeIslandIdx].first;
	return ret;
}

//----------------------------------------------------------------------------
void CPVPManager::freePVPIsland(sint32 cellId)
{
	uint idx = getIslandIdxFromCell( cellId );
	if ( idx < _ChallengeIslands.size() )
	{
		_ChallengeIslands[idx].second = NULL;
		_ChallengeIslands[idx].first = _FirstFreeIslandIdx;
		_FirstFreeIslandIdx = idx;
	}
	else
		nlwarning("invalid PVP cell %d. idx = %u. size is %u", cellId, idx, _ChallengeIslands.size()  );
}

//----------------------------------------------------------------------------
void CPVPManager::playerDisconnects(CCharacter * user)
{
	nlassert(user);
	CPVPManager::getInstance()->removePVPChallengeInvitor(user->getId());

	if ( user->getPVPInterface().isValid() )
		user->getPVPInterface().leavePVP(IPVP::Disconnect);
}

//----------------------------------------------------------------------------
IPVPZone * CPVPManager::getPVPZone(TAIAlias alias)
{
	for (uint i = 0; i < _PVPZones.size(); i++)
	{
		if ( _PVPZones[i]->getAlias() == alias )
			return _PVPZones[i];
	}

	return NULL;
}

//----------------------------------------------------------------------------
IPVPZone * CPVPManager::getPVPZone(const std::string & name)
{
	for (uint i = 0; i < _PVPZones.size(); i++)
	{
		if ( _PVPZones[i]->getName() == name )
			return _PVPZones[i];
	}

	return NULL;
}

//----------------------------------------------------------------------------
void CPVPManager::dumpPVPZone(NLMISC::CLog * log, IPVPZone * pvpZone, bool dumpUsers)
{
	if (!log || !pvpZone)
		return;

	pvpZone->dumpZone(log, dumpUsers);
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(dumpPVPZone, "dump a PVP zone", "<zone_name> [<dump_users=true/false>]")
{
	if (args.size() != 1 && args.size() != 2)
		return false;

	const string & zoneName = args[0];
	bool dumpUsers = true;
	if (args.size() == 2)
	{
		if (args[1] == "false" || args[1] == "0")
			dumpUsers = false;
	}

	IPVPZone * pvpZone = CPVPManager::getInstance()->getPVPZone(zoneName);
	if (!pvpZone)
	{
		log.displayNL("Unknown PVP zone: %s", zoneName.c_str());
		return true;
	}

	log.displayNL("----------------------------------------------------------------------------");
	CPVPManager::getInstance()->dumpPVPZone(&log, pvpZone, dumpUsers);

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(dumpPVPZones, "dump all PVP zones", "[<dump_users=true/false>]")
{
	if (args.size() > 1)
		return false;

	bool dumpUsers = true;
	if (args.size() == 1)
	{
		if (args[0] == "false" || args[0] == "0")
			dumpUsers = false;
	}

	for (uint i = 0; i < CPVPManager::getInstance()->_PVPZones.size(); i++)
	{
		IPVPZone * pvpZone = CPVPManager::getInstance()->_PVPZones[i];
		if (!pvpZone)
			continue;

		log.displayNL("----------------------------------------------------------------------------");
		CPVPManager::getInstance()->dumpPVPZone(&log, pvpZone, dumpUsers);
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(dumpUsersEnteringLeavingPVPZones, "dump users entering or leaving PVP zones", "")
{
	if (args.size() != 0)
		return false;

	const TGameCycle time = CTickEventHandler::getGameCycle();

	// dump entering users
	{
		log.displayNL("----------------------------------------------------------------------------");
		log.displayNL("Users entering in PVP zones:");

		std::list<std::pair<NLMISC::TGameCycle,CPVPManager::CPVPZonePendingUser> >::iterator it;
		for (it = CPVPManager::getInstance()->_UsersEnteringPVPZone.begin(); it != CPVPManager::getInstance()->_UsersEnteringPVPZone.end(); ++it)
		{
			const uint32 remainingTimeInSeconds = ((*it).first - time) / 10;

			CCharacter * user = PlayerManager.getChar( (*it).second.RowId );
			if (!user)
			{
				log.displayNL("unknown player row id: %s", (*it).second.RowId.toString().c_str());
				continue;
			}

			string zoneName;
			IPVPZone * pvpZone = CPVPManager::getInstance()->getPVPZone( (*it).second.ZoneAlias );
			if (pvpZone)
				zoneName = pvpZone->getName();
			else
				zoneName = "not found!";

			log.displayNL("\t%s %s [%s PVP interface] is entering in PVP zone '%s' [remaining time: %u seconds]",
				user->getName().toString().c_str(),
				user->getId().toString().c_str(),
				user->getPVPInterface().isValid() ? "valid" : "invalid",
				zoneName.c_str(),
				remainingTimeInSeconds
				);
		}
	}

	// dump leaving users
	{
		log.displayNL("----------------------------------------------------------------------------");
		log.displayNL("Users leaving PVP zones:");

		std::list<std::pair<NLMISC::TGameCycle,TDataSetRow> >::iterator it;
		for (it = CPVPManager::getInstance()->_UsersLeavingPVPZone.begin(); it != CPVPManager::getInstance()->_UsersLeavingPVPZone.end(); ++it)
		{
			const uint32 remainingTimeInSeconds = ((*it).first - time) / 10;

			CCharacter * user = PlayerManager.getChar( (*it).second );
			if (!user)
			{
				log.displayNL("unknown player row id: %s", (*it).second.toString().c_str());
				continue;
			}

			log.displayNL("\t%s %s [%s PVP interface] [remaining time: %u seconds]",
				user->getName().toString().c_str(),
				user->getId().toString().c_str(),
				user->getPVPInterface().isValid() ? "valid" : "invalid",
				remainingTimeInSeconds
				);
		}
	}

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setPVPZoneActive, "set a PVP zone active or not", "<zone_name> <1/true/0/false>")
{
	if (args.size() != 2)
		return false;

	const string & zoneName = args[0];
	bool active;
	if (args[1] == "false" || args[1] == "0")
		active = false;
	else if (args[1] == "true" || args[1] == "1")
		active = true;
	else
		return false;

	IPVPZone * pvpZone = CPVPManager::getInstance()->getPVPZone(zoneName);
	if (!pvpZone)
	{
		log.displayNL("Unknown PVP zone: %s", zoneName.c_str());
		return true;
	}

	pvpZone->setActive(active);
	log.displayNL("PVP zone '%s' is now %s", zoneName.c_str(), (active ? "active" : "inactive"));

	return true;
}

//----------------------------------------------------------------------------
NLMISC_COMMAND(setPVPZoneGiveFactionPoints, "set a PVP zone active or not", "<zone_name> <1/true/0/false>")
{
	if (args.size() != 2)
		return false;

	const string & zoneName = args[0];
	bool giveFP;
	if (args[1] == "false" || args[1] == "0")
		giveFP = false;
	else if (args[1] == "true" || args[1] == "1")
		giveFP = true;
	else
		return false;

	CPVPVersusZone * versusZone = dynamic_cast<CPVPVersusZone *>(CPVPManager::getInstance()->getPVPZone(zoneName));
	if (!versusZone)
	{
		log.displayNL("Unknown faction PVP zone: %s", zoneName.c_str());
		return true;
	}

	versusZone->giveFactionPoints(giveFP);
	log.displayNL("Faction PVP zone '%s' now %s faction points", zoneName.c_str(), (giveFP ? "gives" : "does not give"));

	return true;
}
