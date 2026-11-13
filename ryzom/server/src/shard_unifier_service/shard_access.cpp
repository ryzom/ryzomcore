// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "shard_access.h"

#include "nel/misc/common.h"
#include "nel/misc/string_common.h"

#include <algorithm>

using namespace std;
using namespace NLMISC;

namespace RSMGR {

namespace {

// The members of the SET column permission.AccessPrivilege, in its order
const char *const s_AccessPrivilegeMembers[] = { "OPEN", "DEV", "RESTRICTED" };
const uint s_NbAccessPrivilegeMembers = sizeof(s_AccessPrivilegeMembers) / sizeof(s_AccessPrivilegeMembers[0]);

// Mark the members a stored AccessPrivilege value holds
void addHeldMembers(const std::string &accessPrivilege, bool held[s_NbAccessPrivilegeMembers])
{
	vector<string> stored;
	explode(accessPrivilege, string(","), stored, true);
	for (uint i = 0; i < stored.size(); ++i)
	{
		for (uint m = 0; m < s_NbAccessPrivilegeMembers; ++m)
		{
			if (stored[i] == s_AccessPrivilegeMembers[m])
				held[m] = true;
		}
	}
}

} // anonymous namespace

bool isAllShardsPermission(sint32 shardId)
{
	return shardId == -1 || shardId == 0;
}

std::string accessPrivilegeOfLevel(const std::string &accessLevel)
{
	if (accessLevel.size() <= 3 || accessLevel.compare(0, 3, "ds_") != 0)
		return std::string();
	string accessPrivilege = toUpperAscii(accessLevel.substr(3));
	for (uint m = 0; m < s_NbAccessPrivilegeMembers; ++m)
	{
		if (accessPrivilege == s_AccessPrivilegeMembers[m])
			return accessPrivilege;
	}
	return std::string();
}

void getShardAccess(const TPermissionRows &rows, uint32 shardId, std::vector<std::string> &access)
{
	access.clear();
	bool held[s_NbAccessPrivilegeMembers];
	std::fill(held, held + s_NbAccessPrivilegeMembers, false);
	for (uint i = 0; i < rows.size(); ++i)
	{
		const CPermissionRow &row = rows[i];
		// the id of a shard is positive: -1 and 0 are no shard, they name all of them
		if (isAllShardsPermission(row.ShardId) || (row.ShardId > 0 && uint32(row.ShardId) == shardId))
			addHeldMembers(row.AccessPrivilege, held);
	}
	for (uint m = 0; m < s_NbAccessPrivilegeMembers; ++m)
	{
		if (held[m])
			access.push_back(s_AccessPrivilegeMembers[m]);
	}
}

std::string describeShardAccess(const TPermissionRows &rows, uint32 shardId)
{
	vector<string> access;
	getShardAccess(rows, shardId, access);
	if (access.empty())
		return "no access";
	string text;
	for (uint i = 0; i < access.size(); ++i)
	{
		if (i != 0)
			text += ",";
		text += access[i];
	}
	return text;
}

bool hasShardAccess(const TPermissionRows &rows, uint32 shardId, const std::string &accessLevel)
{
	string accessPrivilege = accessPrivilegeOfLevel(accessLevel);
	if (accessPrivilege.empty())
		return false;
	vector<string> access;
	getShardAccess(rows, shardId, access);
	return std::find(access.begin(), access.end(), accessPrivilege) != access.end();
}

const char *mainlandChoiceName(TMainlandChoice choice)
{
	switch (choice)
	{
	case mc_forced:
		return "ForceWelcomerShard";
	case mc_last_played:
		return "home mainland of the last played character";
	case mc_lowest_load:
		return "lowest load";
	default:
		return "none";
	}
}

uint chooseMainland(const std::vector<CMainlandCandidate> &candidates, uint32 forcedSessionId, uint32 lastPlayedSessionId, TMainlandChoice &choice)
{
	choice = mc_none;
	if (candidates.empty())
		return 0;

	if (forcedSessionId != 0)
	{
		for (uint i = 0; i < candidates.size(); ++i)
		{
			if (candidates[i].SessionId == forcedSessionId)
			{
				choice = mc_forced;
				return i;
			}
		}
	}

	if (lastPlayedSessionId != 0)
	{
		for (uint i = 0; i < candidates.size(); ++i)
		{
			if (candidates[i].SessionId == lastPlayedSessionId)
			{
				choice = mc_last_played;
				return i;
			}
		}
	}

	uint best = 0;
	for (uint i = 1; i < candidates.size(); ++i)
	{
		if (candidates[i].Load < candidates[best].Load)
			best = i;
	}
	choice = mc_lowest_load;
	return best;
}

} // namespace RSMGR
