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
#include "player_manager/admin_properties.h"
#include "mission_manager/mission_manager.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "team_manager/team_manager.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild.h"
#include "mission_manager/mission_solo.h"
#include "mission_manager/mission_team.h"
#include "mission_manager/mission_guild.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CAdminProperties);

void CAdminProperties::stopMonitoringMissions( CCharacter & csr )
{
	CAdminProperties & properties = csr.getAdminProperties();
	const TDataSetRow monitored = properties._MissionUser;
	if ( monitored.isNull() )
		return;
	properties._MissionUser = TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW );

	CCharacter * user = PlayerManager.getChar( monitored );
	if ( user && user->getMonitoringCSR() == csr.getEntityRowId() )
		user->setMonitoringCSR( TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW ) );

	// The CSR has no entry of its own: monitorMissions refuses a CSR that has missions, and CCharacter::addMission
	// stops the monitoring before the entry of a new mission is written. All the entries were the user's.
	for ( uint8 i = 0; i < MaxSoloMissionCount; i++ )
		CMissionSolo::clearJournalEntry( csr, i );
}

CMission* CAdminProperties::getMission(uint indexInJournal) const
{
	CCharacter *user = PlayerManager.getChar(_MissionUser);
	if (!user)
		return NULL;

	// The solo missions come first, by their entry in the journal. The entries of the group journal follow: those of
	// the team, then those of the guild (the journal entry of a guild mission counts from the end of the team's).
	if (indexInJournal < MaxSoloMissionCount)
	{
		for (map<TAIAlias, CMission *>::iterator it = user->getMissionsBegin(); it != user->getMissionsEnd(); ++it)
		{
			if ((*it).second->getClientIndex() == indexInJournal)
				return (*it).second;
		}
		return NULL;
	}

	indexInJournal -= MaxSoloMissionCount;
	if (indexInJournal < MaxGroupMissionCount)
	{
		CTeam *team = TeamManager.getRealTeam(user->getTeamId());
		if (team)
		{
			for (uint i = 0; i < team->getMissions().size(); i++)
			{
				if (team->getMissions()[i]->getClientIndex() == indexInJournal)
					return team->getMissions()[i];
			}
		}
		return NULL;
	}

	CGuild *guild = CGuildManager::getInstance()->getGuildFromId(user->getGuildId());
	if (guild)
	{
		for (uint i = 0; i < guild->getMissions().size(); i++)
		{
			if (guild->getMissions()[i]->getClientIndex() == indexInJournal)
				return guild->getMissions()[i];
		}
	}
	return NULL;
}
