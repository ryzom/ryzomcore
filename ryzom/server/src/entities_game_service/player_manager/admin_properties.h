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

#ifndef RY_ADMIN_PROPERTIES_H
#define RY_ADMIN_PROPERTIES_H

#include "mission_manager/ai_alias_translator.h"
#include "mission_manager/mission_types.h"

class CCharacter;

/**
 * properties for CSR characters. Every character has them: the privileges of the admin commands decide who uses them
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CAdminProperties
{
	NL_INSTANCE_COUNTER_DECL(CAdminProperties);
public:

	void setMissionMonitoredUser(const TDataSetRow & row)
	{
		_MissionUser = row;
	}

	TDataSetRow getMissionMonitoredUser() const
	{
		return _MissionUser;
	}

	/// The mission of the monitored character at an entry of its journal: the solo missions, which the CSR has at the
	/// same entries of its own journal, then the team and the guild missions, which it does not see
	CMission * getMission( uint indexInJournal ) const;

	/// The CSR monitors the missions of no character anymore: the one it monitored can be monitored again, and the
	/// journal of the CSR, which had its solo missions, is empty again. Nothing is done for a CSR that monitors none.
	static void stopMonitoringMissions( CCharacter & csr );

private:
	// the character whose missions are monitored: no row when there is none
	TDataSetRow _MissionUser;
};


#endif // RY_ADMIN_PROPERTIES_H

/* End of admin_properties.h */
