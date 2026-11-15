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

#ifndef SHARD_ACCESS_H
#define SHARD_ACCESS_H

#include "nel/misc/types_nl.h"

#include <string>
#include <vector>

namespace RSMGR {
/** The one place on the game side that decides what the permission rows of an account give it on the shards of the
 *	domain, and which mainland shard an account goes to for the character selection. The login script holds the
 *	same access rule for the web side (web/public_php/tools/shard_access.php).
 *
 *	The permission table of the nel database has one row per account, domain and ShardId. ShardId is -1 for a row
 *	that counts on all shards of the domain (and 0, which an earlier version of the account tool wrote for the same
 *	thing), or the id of one shard. AccessPrivilege is a SET of OPEN, DEV and RESTRICTED: the access level of a
 *	shard asks for the member of the same name (ds_open for OPEN and so on; ds_close for none, nobody enters).
 *
 *	The rule: the access of an account on a shard is what its row for all shards holds together with what its row
 *	for that shard holds. Either row can give the access; a row for one shard gives nothing on any other shard.
 *
 *	Nothing here reads a database: the functions work on rows already read, so they can be tested alone.
 */

/// One row of the permission table, of one account for one domain
struct CPermissionRow
{
	/// -1 (or 0) for all shards of the domain, or the id of one shard
	sint32 ShardId;
	/// The SET column as the database gives it: its members separated by commas
	std::string AccessPrivilege;

	CPermissionRow()
	    : ShardId(-1)
	{
	}

	CPermissionRow(sint32 shardId, const std::string &accessPrivilege)
	    : ShardId(shardId)
	    , AccessPrivilege(accessPrivilege)
	{
	}
};

typedef std::vector<CPermissionRow> TPermissionRows;

/// True for the ShardId of a row that counts on all shards of its domain
bool isAllShardsPermission(sint32 shardId);

/** The access privilege an access level asks for: "OPEN" for "ds_open", "DEV" for "ds_dev", "RESTRICTED" for
 *	"ds_restricted". Empty for "ds_close" and for anything else: no privilege gives access then.
 */
std::string accessPrivilegeOfLevel(const std::string &accessLevel);

/** The access of an account on one shard: what its rows for all shards hold together with what its row for that
 *	shard holds, each member once, in the order OPEN, DEV, RESTRICTED.
 */
void getShardAccess(const TPermissionRows &rows, uint32 shardId, std::vector<std::string> &access);

/// The same as one text for a log line or the reason of a refusal: "OPEN,RESTRICTED", or "no access"
std::string describeShardAccess(const TPermissionRows &rows, uint32 shardId);

/// True when the rows give the account, on that shard, the access the level ("ds_open", ...) asks for
bool hasShardAccess(const TPermissionRows &rows, uint32 shardId, const std::string &accessLevel);

/// A mainland shard an account may be sent to: it is online and the account has the access it asks for
struct CMainlandCandidate
{
	/// The mainland session of the shard
	uint32 SessionId;
	/// The players online on it and on their way to it
	uint32 Load;

	CMainlandCandidate()
	    : SessionId(0)
	    , Load(0)
	{
	}

	CMainlandCandidate(uint32 sessionId, uint32 load)
	    : SessionId(sessionId)
	    , Load(load)
	{
	}
};

/// Which rule chose the mainland shard
enum TMainlandChoice
{
	/// There was no candidate
	mc_none,
	/// The shard the operator promotes (ForceWelcomerShard)
	mc_forced,
	/// The home mainland of the character the account played last
	mc_last_played,
	/// The shard with the fewest players
	mc_lowest_load
};

/// The name of the rule for a log line
const char *mainlandChoiceName(TMainlandChoice choice);

/** Choose the mainland shard an account goes to for the character selection, among the shards it may use.
 *	In this order: the shard the operator promotes, when it is a candidate; the home mainland of the character the
 *	account played last, when it is a candidate; the candidate with the lowest load, the first of them in the list
 *	when several have it.
 *	\param candidates the shards that are online and that the account may use
 *	\param forcedSessionId the mainland session the operator promotes, 0 for none
 *	\param lastPlayedSessionId the home mainland session of the character played last, 0 when there is none
 *	\param choice the rule that chose
 *	\return the index of the chosen candidate, candidates.size() when there is none
 */
uint chooseMainland(const std::vector<CMainlandCandidate> &candidates, uint32 forcedSessionId, uint32 lastPlayedSessionId, TMainlandChoice &choice);

} // namespace RSMGR

#endif // SHARD_ACCESS_H
