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

#include "pvp_manager/pvp_challenge.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "team_manager/team.h"
#include "team_manager/team_manager.h"
#include "zone_manager.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_manager/pvp.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;


//----------------------------------------------------------------------------
CPVPChallenge::CPVPChallenge(CCharacter *user1, CCharacter * user2)
{
	// allocate a PVP island
	_CellId = CPVPManager::getInstance()->allocatePVPIsland(this);
	/// add both user teams
	addUserTeam(user1);
	addUserTeam(user2);

	for ( uint i = 0; i < _Teams.size(); i++ )
		sendChallengeMessage(i, "CHALLENGE_STARTS" , TVectorParamCheck() );
}

//----------------------------------------------------------------------------
PVP_RELATION::TPVPRelation CPVPChallenge::getPVPRelation( CCharacter * user, CEntityBase * target ) const
{
	nlassert(user);
	nlassert(target);
	
	uint16 teamIdx1,teamIdx2,memberIdx;
	
	// here we should be in the user interface
	if ( !getMember( user->getEntityRowId(), teamIdx1, memberIdx ) )
	{
		nlwarning("<PVP>user %s is not in its PVP interface", user->getId().toString().c_str() );
		return PVP_RELATION::Unknown;
	}

	// check if target and user must are in the same challenge
	if ( !getMember( target->getEntityRowId(), teamIdx2, memberIdx ) )
	{
		return PVP_RELATION::Neutral;
	}
	
	// check if ennemy or ally
	if ( teamIdx1 != teamIdx2 )
	{	
		return PVP_RELATION::Ennemy;
	}
	else
	{	
		return PVP_RELATION::Ally;
	}
}


//----------------------------------------------------------------------------
void CPVPChallenge::addUserTeam(CCharacter *user)
{
	nlassert(user);
	const CTpSpawnZone * zone = CPVPManager::getInstance()->getChallengeSpawnZone( (uint)_Teams.size() );
	nlassert(zone);
	sint32 x,y,z;
	float heading;
	
	// add the user team
	CTeam * team = TeamManager.getRealTeam( user->getTeamId() );
	CTeamEntry entry;
	if ( team )
	{
		entry.TeamId = user->getTeamId();
		for ( list<CEntityId>::const_iterator it = team->getTeamMembers().begin(); it != team->getTeamMembers().end(); ++it )
		{
			CCharacter * c = PlayerManager.getChar( *it );
			if ( c )
			{
				// actually add the user to the team
				addUserToTeam( entry,c );
				// tp the user
				zone->getRandomPoint(x,y,z,heading);
				c->forbidNearPetTp();
				c->tpWanted(x,y,z,true,heading,0xFF,_CellId);
			}
		}
	}
	else
	{
		entry.TeamId = CTEAM::InvalidTeamId;
		// actually add the user to the team
		addUserToTeam( entry,user );
		// tp the user
		zone->getRandomPoint(x,y,z,heading);
		user->forbidNearPetTp();
		user->tpWanted(x,y,z,true,heading,0xFF,_CellId);
	}
	_Teams.push_back(entry);
}

//----------------------------------------------------------------------------
void CPVPChallenge::addUserToTeam( CTeamEntry & entry, CCharacter *user )
{
	// backup member properties
	nlassert(user);
	entry.Members.push_back(CMember());
	entry.Members.back().Row = user->getEntityRowId();
	user->getState().setCOfflineEntityState( entry.Members.back().OldCoords );// what a strange API!!! // AS: what a strange comment, if you not like this API, change it instead write useless comment or shut up ! (and where are your cojones for making critic anonymous comment ?) !
	//AS: for Noobs:
	// these lines are equivalents of the 3 previous you have difficult to understand it's seem, more readable for Noob you are:
	// CMember member;
	// member.Row = user->getEntityRowId();
	// COfflineEntityState& offlineEntityState = user->getState();
	// offlineEntityState.setCOfflineEntityState( member.OldCoord );
	// entry.Members.push_back( member );
	//
	// So if you found any strange API here, return to read your C/C++ language manual...
	// if you found setting var by reference as var parameter of function calling instead use a return value and = operator: (ex: COfflineEntityState::setCOfflineEntityState() method in case of you not reconize about what i'm speaking..)
	//		1: that allow return error code can be used eventually
	//		2: that is massively used in Nel even with void return value, and you never found that strange before...
	//		3: think a little before write stupid comments.
	entry.Members.back().OldCaracs.resize( SCORES::NUM_SCORES );
	for ( uint i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		CPhysicalScores & scores = user->getScores();
		nlassert( i < scores._PhysicalScores.size() );
		entry.Members.back().OldCaracs[i] = scores._PhysicalScores[i].Current;
	}
//	user->_PropertyDatabase.setProp("USER:IN_PVP_CHALLENGE",true );
	CBankAccessor_PLR::getUSER().setIN_PVP_CHALLENGE(user->_PropertyDatabase,true );
	user->updateTarget();
	user->getPVPInterface().init(this);
}

//----------------------------------------------------------------------------
bool CPVPChallenge::leavePVP( CCharacter * user, IPVP::TEndType type )
{
	//remettre l'abandon en place pour leader uniquement. Dans ce cas tp tous les gus
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);

		
	nlassert(user);
	uint16 teamIdx,memberIdx;
	const CMember* member =getMember(user->getEntityRowId(),teamIdx, memberIdx );
	if ( !member )
	{
		
		return true;
	}
	switch ( type )
	{
	case IPVP::AbandonChallenge:
		{
			params[0].setEIdAIAlias( user->getId(), CAIAliasTranslator::getInstance()->getAIAlias(user->getId()) );
			for ( uint i = 0; i < _Teams.size(); i++ )
			{
				if ( i == teamIdx )
				{
					sendChallengeMessage( i, "CHALLENGE_YOU_ABANDON",params);
					sendChallengeMessage( i, "CHALLENGE_LOST");
				}
				else
					sendChallengeMessage( i, "CHALLENGE_HE_ABANDON",params);
			}

			const uint size = (uint)_Teams[ teamIdx ].Members.size();
			for ( uint i = 0; i < size; i++ )
			{
				CCharacter * c = PlayerManager.getChar( _Teams[ teamIdx ].Members[i].Row );
				if ( c )
				{
					restoreScores(c,_Teams[ teamIdx ].Members[i]);
					// copy former coords, as this object will be de delete through smart pointer deallocation
					COfflineEntityState oldState = member->OldCoords;
					// reset the PVP interface of the user
					c->getPVPInterface().reset();
					// tp the user to the former coords
//					c->_PropertyDatabase.setProp("USER:IN_PVP_CHALLENGE",false );
					CBankAccessor_PLR::getUSER().setIN_PVP_CHALLENGE(c->_PropertyDatabase,false );
					c->updateTarget();
					// tell team
					c->forbidNearPetTp();
					c->tpWanted( member->OldCoords.X, member->OldCoords.Y, member->OldCoords.Z,true,member->OldCoords.Heading);
				}
			}
			_Teams[ teamIdx ].Members.clear();
		}
		break;
	case IPVP::Disconnect:
	case IPVP::Death:
	case IPVP::QuitTeam:
		{
			restoreScores(user, *member );
			if ( _Teams[teamIdx].Members.size() == 1 )
				sendChallengeMessage( teamIdx, "CHALLENGE_LOST");
			// reset the PVP interface of the user
			user->getPVPInterface().reset();
			user->forbidNearPetTp();
			user->tpWanted( member->OldCoords.X, member->OldCoords.Y, member->OldCoords.Z,true,member->OldCoords.Heading);
			_Teams[teamIdx].Members[memberIdx] = _Teams[teamIdx].Members.back();
			_Teams[teamIdx].Members.pop_back();
			break;
		}
		break;
	case IPVP::Teleport:
		restoreScores(user, *member );
		// only reset the user interface
		if ( _Teams[teamIdx].Members.size() == 1 )
			sendChallengeMessage( teamIdx, "CHALLENGE_LOST");
		_Teams[teamIdx].Members[memberIdx] = _Teams[teamIdx].Members.back();
		_Teams[teamIdx].Members.pop_back();
		user->getPVPInterface().reset();
		break;
	default:
		return true;
	}
	
//	user->_PropertyDatabase.setProp("USER:IN_PVP_CHALLENGE",false );
	CBankAccessor_PLR::getUSER().setIN_PVP_CHALLENGE(user->_PropertyDatabase,false );
	user->updateTarget();
	
	/// check if there remains only 1 team
	uint lastTeamIdx = ~0;
	const uint size = (uint)_Teams.size();
	for ( uint i = 0; i < size; i++ )
	{
		if ( !_Teams[i].Members.empty() )
		{
			if ( lastTeamIdx == ~0 )
				lastTeamIdx = i;
			else
			{
				lastTeamIdx = ~0;
				break;
			}
		}
	}

	if ( lastTeamIdx != ~0 )
	{
		CPVPManager::getInstance()->freePVPIsland(_CellId);
		sendChallengeMessage( lastTeamIdx, "CHALLENGE_WON");
		// free the island
		nlassert( lastTeamIdx < _Teams.size() );
		const uint size = (uint)_Teams[ lastTeamIdx ].Members.size();
		for ( uint i = 0; i < size; i++ )
		{
			CCharacter * c = PlayerManager.getChar( _Teams[ lastTeamIdx ].Members[i].Row );
			if ( c )
			{
				restoreScores( c,_Teams[ lastTeamIdx ].Members[i] );
				// copy former coords, as this object will be de delete through smart pointer deallocation
				COfflineEntityState oldState = member->OldCoords;
				// tp the user to the former coords
				// we have to reset the PVP interface before teleporteing as PVP has effects on teleports.
				// So let's backup member former coords as smart pointer on ' could be deleted because of teleports
				sint32 x = member->OldCoords.X;
				sint32 y = member->OldCoords.Y;
				sint32 z = member->OldCoords.Z;
				float heading = member->OldCoords.Heading;
				c->getPVPInterface().reset();
				c->forbidNearPetTp();
				c->tpWanted( x, y, z,true,heading);
//				c->_PropertyDatabase.setProp("USER:IN_PVP_CHALLENGE",false );
				CBankAccessor_PLR::getUSER().setIN_PVP_CHALLENGE(c->_PropertyDatabase, false );
				c->updateTarget();
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------------
bool CPVPChallenge::canUserHurtTarget(CCharacter * user, CEntityBase * target) const
{
	nlassert(user);
	nlassert(target);

	uint16 teamIdx1,teamIdx2,memberIdx;
	// here we should be in the user interface
	if ( !getMember( user->getEntityRowId(), teamIdx1, memberIdx ) )
	{
		nlwarning("<PVP>user %s is not in its PVP interface", user->getId().toString().c_str() );
		return false;
	}
	// target and user must be in the same challenge
	if ( !getMember( target->getEntityRowId(), teamIdx2, memberIdx ) )
		return false;
	// target and user must not be in the same team
	if ( teamIdx1 == teamIdx2 )
		return false;
	return true;
}

//----------------------------------------------------------------------------
bool CPVPChallenge::canUserHelpTarget(CCharacter * user, CEntityBase * target) const
{
	nlassert(user);
	nlassert(target);
	uint16 teamIdx1,teamIdx2,memberIdx;
	// here we should be in the user interface
	if ( !getMember( user->getEntityRowId(), teamIdx1, memberIdx ) )
	{
		nlwarning("<PVP>user %s is not in its PVP interface", user->getId().toString().c_str() );
		return false;
	}
	// target and user must be in the same challenge
	if ( !getMember( target->getEntityRowId(), teamIdx2, memberIdx ) )
		return false;
	// target and user must be in the same team
	if ( teamIdx1 != teamIdx2 )
		return false;
	return true;
}

//----------------------------------------------------------------------------
bool CPVPChallenge::canApplyAreaEffect(CCharacter * caster, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const
{
	nlassert(caster);
	nlassert(areaTarget);

	// Allow hitting bots
	if ( offensive && areaTarget->getId().getType() != RYZOMID::player )
		return true;

	if (offensive)
		return canUserHurtTarget(caster, areaTarget);

	return canUserHelpTarget(caster, areaTarget);
}

//----------------------------------------------------------------------------
const CPVPChallenge::CMember * CPVPChallenge::getMember( const TDataSetRow & userRow, uint16& teamIdx, uint16 & memberIdx )const
{
	const uint size = (uint)_Teams.size();
	for  ( uint i = 0; i < size ; i++ )
	{
		const uint size2 = (uint)_Teams[i].Members.size();
		for  ( uint j = 0; j < size2 ; j++ )
		{
			if ( userRow == _Teams[i].Members[j].Row )
			{
				teamIdx = i;
				memberIdx = j;
				return &_Teams[i].Members[j];
			}
		}
	}
	return NULL;
}

//----------------------------------------------------------------------------
void CPVPChallenge::sendChallengeMessage(uint16 teamIdx, const std::string & msg, const TVectorParamCheck & params)const
{
	nlassert( teamIdx < _Teams.size() );
	if ( _Teams[teamIdx].TeamId != CTEAM::InvalidTeamId )
	{
		CTeam * team = TeamManager.getTeam(_Teams[teamIdx].TeamId);
		if ( !team )
		{
			nlwarning("<PVP>team id %u is invamlid",_Teams[teamIdx].TeamId );
			return;
		}
		team->sendDynamicMessageToMembers(msg,params);
	}
	else
	{
		if ( !_Teams[teamIdx].Members.empty() )
		{
			CCharacter::sendDynamicSystemMessage( _Teams[teamIdx].Members[0].Row,msg,params );
		}
	}
}

//----------------------------------------------------------------------------
void CPVPChallenge::restoreScores(CCharacter * user, const CMember& member )
{
	nlassert(user);
	for ( uint i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		CPhysicalScores & scores = user->getScores();
		nlassert( i < scores._PhysicalScores.size() );
		nlassert( i < member.OldCaracs.size() );
		user->getScores()._PhysicalScores[i].Current = member.OldCaracs[i];
	}
	user->removeAllSpells();
}

