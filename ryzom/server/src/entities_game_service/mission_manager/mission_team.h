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

#ifndef RY_MISSION_TEAM_H
#define RY_MISSION_TEAM_H

#include "egs_pd.h"

/**
 * <Class description>
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CMissionTeam : public EGSPD::CMissionTeamPD
{
	NL_INSTANCE_COUNTER_DECL(CMissionTeam);
public:

	CMissionTeam() : _Chained(false) { }

	inline void setTeam( uint16 teamId );
	/// override
	void updateUsersJournalEntry();
	/// override
	void clearUsersJournalEntry();
	/// override
	void setupEscort(const std::vector<TAIAlias> & aliases);
	/// override
	void getEntities(std::vector<TDataSetRow>& entities);
	/// override
	void stopChildren();
	/// override
	void onFailure(bool ignoreJumps,bool sendMessage = true);
	/// force mission success
	void forceSuccess();
	/// return the mission main character ( user, group leader,...)
	CCharacter* getMainEntity();
	/// during the execution of the mission does the compiler encounter a 'chain_mission' ?
	bool isChained() { return _Chained; }
	/// when executing a 'chain_mission' set this flag to let know that we have chained
	void setChained(bool b=true) { _Chained = b; }

private:
	uint16 _TeamId;

	bool _Chained;
};

inline void CMissionTeam::setTeam( uint16 teamId )
{
	_TeamId = teamId;
}

#endif // RY_MISSION_TEAM_H

/* End of mission_team.h */
