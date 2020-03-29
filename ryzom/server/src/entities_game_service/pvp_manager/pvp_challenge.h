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



#ifndef RY_PVP_CHALLENGE_H
#define RY_PVP_CHALLENGE_H

#include "pvp_base.h"
#include "game_share/base_types.h"
#include "game_share/string_manager_sender.h"
#include "server_share/entity_state.h"

class CCharacter;

/**
 * A PVP challenge : team vs team, teleportation on a part of the world
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CPVPChallenge : public IPVP
{
public:
	/// a member of a team in the challenge the challenge
	struct CMember
	{
		/// row id of the user
		TDataSetRow				Row;
		/// previous coords of the player
		COfflineEntityState	OldCoords;
		std::vector<sint32>					OldCaracs;
	};
	/// a team of the challenge
	struct CTeamEntry
	{
		/// id of the corresponding ingame team ( CTeam::InvalidTeamId if there are none, e.g. : alone player )
		uint16					TeamId;
		/// members of the team that are actually in the challenge
		std::vector<CMember>	Members;
	};
	
	CPVPChallenge(CCharacter *user1, CCharacter * user2);
	/// get a member of the challenge
	const CMember * getMember( const TDataSetRow & userRow, uint16 &teamIdx, uint16 &memberIdx )const;

	PVP_MODE::TPVPMode getPVPMode() const { return PVP_MODE::PvpChallenge; }

	/// return pvp relation between the two players
	PVP_RELATION::TPVPRelation getPVPRelation( CCharacter * user, CEntityBase * target ) const;
	
private:
	bool leavePVP( CCharacter * user, IPVP::TEndType type );

	/// Return true for players in the challenge, from a different team, false for anyone else (including non-players) ('attackable' will be used instead)
	bool canUserHurtTarget(CCharacter * user, CEntityBase * target) const;

	/// Return true for players in the challenge, from the same team, false for anyone else (including non-players)
	bool canUserHelpTarget(CCharacter * user, CEntityBase * target) const;

	/// Return true for ennemy players and for non-players (if offensive)
	bool canApplyAreaEffect(CCharacter * caster, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const;

	virtual bool doCancelRespawn() const { return true; }

	/// add a user team
	void addUserTeam(CCharacter *user);
	/// backup a member propeties
	inline void addUserToTeam( CTeamEntry & entry, CCharacter *user );
	/// send a message to the specified team members
	inline void sendChallengeMessage(uint16 teamIdx, const std::string & msg, const TVectorParamCheck & params = TVectorParamCheck() )const;
	/// put back the former user caracs
	inline void restoreScores(CCharacter * user, const CMember& member );
	
	/// teams in challenge
	std::vector<CTeamEntry> _Teams;
	/// cellid of the challenge island
	sint32					_CellId;
};

#endif // RY_PVP_CHALLENGE_H

