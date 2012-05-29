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



#ifndef RY_PVP_MANAGER_H
#define RY_PVP_MANAGER_H

#include "mission_manager/ai_alias_translator.h"
#include "pvp_duel.h"
#include "pvp_manager/pvp_challenge.h"
#include "pvp_zone.h"

class CTpSpawnZone;

namespace NLMISC
{

	class CLog;

} // namespace NLMISC

class CPVPSafeZone;

/**
 * Singleton used manage PVP.
 *
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CPVPManager
{
public:
	///\name LOW LEVEL
	//@{
	/// singleton init
	static void init();
	/// singleton release
	static void release();
	/// instance accessor
	static CPVPManager* getInstance()
	{
		nlassert(_Instance);
		return _Instance;
	}
	/// init PVP island
	void initPVPIslands();
	/// callback called at each tick
	void tickUpdate();
	//@}

	///\name PVP CHALLENGE
	//@{
	/// player ask target in duel
	void askForPVPChallenge( const NLMISC::CEntityId & userId );
	/// player accept a duel
	void acceptPVPChallenge( const NLMISC::CEntityId & userId );
	/// player refuses a duel
	void refusePVPChallenge( const NLMISC::CEntityId & userId );
	/// remove a user from the duel invitations
	void removePVPChallengeInvitor( const NLMISC::CEntityId & userId );
	//@}
	
	///\name PVP ZONE
	//@{
	/// add a PVP zone in the manager
	void addPVPZone( NLMISC::CSmartPtr<IPVPZone> pvpZone );
	/// add a PVP safe zone in the manager
	void addPVPSafeZone( NLMISC::CSmartPtr<CPVPSafeZone> safeZone );
	/// apply configuration to PVP zones
	void applyConfigToPVPZones();
	/// get an active PVP zone alias from a position (if user is in conflict). Precondition: user not null
	TAIAlias getPVPZoneFromUserPosition( CCharacter *user ) const;
	/// user leaves a PVP zone
	void leavePVPZone( CCharacter * user );
	/// user enters a PVP zone
	void enterPVPZone( CCharacter * user, TAIAlias pvpZoneAlias );
	/// remove someone from leaving users
	bool removeFromLeavingPVPZoneUsers( TDataSetRow rowId );
	//@}
	
	///\name HELPERS
	//@{
	/*
	/// return true if actor can use a curative action on target
	bool testCurativeAction( CCharacter* actor, CEntityBase * target );
	/// return true if actor can use an offensive action on target
	bool testOffensiveAction( CCharacter* actor, CEntityBase * target );
	/// return true if entity in area effect must suffer the action
	bool canApplyAreaEffect(CCharacter* actor, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const;
	*/
	/// return an appropriate challenges spawn zone
	const CTpSpawnZone* getChallengeSpawnZone(uint idx)const
	{
		nlassert(idx < _ChallengeZones.size() );
		return _ChallengeZones[idx];
	}
	/// allocate a new PVP island and return its cellId
	sint32 allocatePVPIsland(CPVPChallenge* challenge);
	/// free a PVP island
	void freePVPIsland(sint32 cellId);
	/// handle player disconnection
	void playerDisconnects(CCharacter * user);

	/// get a PVP zone from alias
	IPVPZone * getPVPZone(TAIAlias alias);	
	/// get a PVP zone from name
	IPVPZone * getPVPZone(const std::string & name);	
	//@}

private:
	NLMISC_COMMAND_FRIEND(dumpPVPZone);
	NLMISC_COMMAND_FRIEND(dumpPVPZones);
	NLMISC_COMMAND_FRIEND(dumpUsersEnteringLeavingPVPZones);

	/// dump a PVP zone
	void dumpPVPZone(NLMISC::CLog * log, IPVPZone * pvpZone, bool dumpUsers);

	/// return an island index from its cell
	uint getIslandIdxFromCell(sint32 cell)
	{
		return uint(  ( -(cell+3) ) >> 1);
	}

	/// return an island cell from its index
	sint32 getIslandCellFromIdx(uint idx)
	{
		return ( -3 - sint32( idx << 1 ) );
	}

	/// remove someone from entering users
	bool removeFromEnteringPVPZoneUsers(TDataSetRow rowId);

	/// user really enters PVP zone, after enter buffer time is elapsed
	void doEnterPVPZone(CCharacter * user, TAIAlias pvpZoneAlias);

	/// ctor
	CPVPManager(){}
	/// dtor
	~CPVPManager(){}
	/// unique instance
	static CPVPManager * _Instance;

	/// PVP challenge propositions ( pair invitor-invited )
	struct CPVPChallengeAsked
	{
		/// invitor of the challenge
		TDataSetRow			Invitor;
		TDataSetRow			InvitedUser;
		uint16				InvitedTeam;
		NLMISC::TGameCycle	ExpirationDate;
	};

	/// PVP zones
	typedef std::vector<NLMISC::CSmartPtr<IPVPZone> >	TPVPZones;
	TPVPZones	_PVPZones;

	struct CPVPZonePendingUser
	{
		TDataSetRow	RowId;
		TAIAlias	ZoneAlias;
	};

	/// users entering a PVP zone
	std::list<std::pair<NLMISC::TGameCycle,CPVPZonePendingUser> >	_UsersEnteringPVPZone;

	/// users leaving a PVP zone
	std::list<std::pair<NLMISC::TGameCycle,TDataSetRow> >			_UsersLeavingPVPZone;

	/// pvp challenges invitations
	std::list< CPVPChallengeAsked >								_PVPChallengesAsked;

	/// challenge zones
	std::vector<const CTpSpawnZone*>							_ChallengeZones;

	/// pvp challenge islands, paired with an int storing the index of the next free id in the vector
	std::vector< std::pair< uint, CPVPChallenge* > >			_ChallengeIslands;
	uint														_FirstFreeIslandIdx;
};

#endif // RY_PVP_MANAGER_H

