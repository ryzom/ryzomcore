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

// What the permission rows of an account give it on the shards of a domain, and the choice of the mainland shard
// for the character selection: the rules of the shard unifier, which need no database (shard_access.h).

#include "nel/misc/types_nl.h"
#include "shard_unifier_service/shard_access.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace RSMGR;

namespace {

const uint32 ShardA = 301;
const uint32 ShardB = 302;

TPermissionRows makeRows(sint32 shardId1, const char *access1)
{
	TPermissionRows rows;
	rows.push_back(CPermissionRow(shardId1, access1));
	return rows;
}

TPermissionRows makeRows(sint32 shardId1, const char *access1, sint32 shardId2, const char *access2)
{
	TPermissionRows rows = makeRows(shardId1, access1);
	rows.push_back(CPermissionRow(shardId2, access2));
	return rows;
}

std::vector<CMainlandCandidate> makeCandidates(uint32 loadA, uint32 loadB)
{
	std::vector<CMainlandCandidate> candidates;
	candidates.push_back(CMainlandCandidate(ShardA, loadA));
	candidates.push_back(CMainlandCandidate(ShardB, loadB));
	return candidates;
}

} // namespace

TEST(ShardAccess, AccessPrivilegeOfLevel)
{
	EXPECT_EQ("OPEN", accessPrivilegeOfLevel("ds_open"));
	EXPECT_EQ("DEV", accessPrivilegeOfLevel("ds_dev"));
	EXPECT_EQ("RESTRICTED", accessPrivilegeOfLevel("ds_restricted"));
	// a closed shard asks for no privilege: nobody has what it takes
	EXPECT_EQ("", accessPrivilegeOfLevel("ds_close"));
	EXPECT_EQ("", accessPrivilegeOfLevel("open"));
	EXPECT_EQ("", accessPrivilegeOfLevel("ds_"));
	EXPECT_EQ("", accessPrivilegeOfLevel(""));
	EXPECT_EQ("", accessPrivilegeOfLevel("invalid"));
}

TEST(ShardAccess, AllShardsRowIds)
{
	EXPECT_TRUE(isAllShardsPermission(-1));
	EXPECT_TRUE(isAllShardsPermission(0));
	EXPECT_FALSE(isAllShardsPermission(302));
	EXPECT_FALSE(isAllShardsPermission(-2));
}

TEST(ShardAccess, OnlyAnAllShardsRow)
{
	TPermissionRows rows = makeRows(-1, "OPEN");
	EXPECT_EQ("OPEN", describeShardAccess(rows, ShardA));
	EXPECT_EQ("OPEN", describeShardAccess(rows, ShardB));
	EXPECT_TRUE(hasShardAccess(rows, ShardA, "ds_open"));
	EXPECT_TRUE(hasShardAccess(rows, ShardB, "ds_open"));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_dev"));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_restricted"));
}

TEST(ShardAccess, OnlyARowForOneShard)
{
	TPermissionRows rows = makeRows(ShardA, "OPEN");
	EXPECT_TRUE(hasShardAccess(rows, ShardA, "ds_open"));
	// a row for one shard gives nothing on another shard
	EXPECT_FALSE(hasShardAccess(rows, ShardB, "ds_open"));
	EXPECT_EQ("no access", describeShardAccess(rows, ShardB));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_restricted"));
}

TEST(ShardAccess, BothRowsWithDifferentSets)
{
	TPermissionRows rows = makeRows(-1, "OPEN", ShardB, "RESTRICTED");
	EXPECT_EQ("OPEN", describeShardAccess(rows, ShardA));
	// the two rows together, in the order of the SET
	EXPECT_EQ("OPEN,RESTRICTED", describeShardAccess(rows, ShardB));
	EXPECT_TRUE(hasShardAccess(rows, ShardA, "ds_open"));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_restricted"));
	EXPECT_TRUE(hasShardAccess(rows, ShardB, "ds_open"));
	EXPECT_TRUE(hasShardAccess(rows, ShardB, "ds_restricted"));
	EXPECT_FALSE(hasShardAccess(rows, ShardB, "ds_dev"));

	// the order the rows are read in does not matter
	TPermissionRows reversed = makeRows(ShardB, "RESTRICTED", -1, "OPEN");
	EXPECT_EQ("OPEN", describeShardAccess(reversed, ShardA));
	EXPECT_EQ("OPEN,RESTRICTED", describeShardAccess(reversed, ShardB));

	// a row for one shard takes nothing away from what the row for all shards gives
	TPermissionRows withEmpty = makeRows(-1, "OPEN,DEV", ShardA, "");
	EXPECT_EQ("OPEN,DEV", describeShardAccess(withEmpty, ShardA));
}

TEST(ShardAccess, LegacyZeroRow)
{
	TPermissionRows rows = makeRows(0, "DEV");
	EXPECT_TRUE(hasShardAccess(rows, ShardA, "ds_dev"));
	EXPECT_TRUE(hasShardAccess(rows, ShardB, "ds_dev"));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_open"));

	TPermissionRows both = makeRows(-1, "OPEN", 0, "RESTRICTED");
	EXPECT_EQ("OPEN,RESTRICTED", describeShardAccess(both, ShardA));

	// shard ids 0 and -1 name no shard: asking for them gives what the rows for all shards hold
	TPermissionRows mixed = makeRows(-1, "OPEN", ShardA, "DEV");
	EXPECT_EQ("OPEN", describeShardAccess(mixed, 0));
	EXPECT_EQ("OPEN", describeShardAccess(mixed, uint32(-1)));
}

TEST(ShardAccess, NoRow)
{
	TPermissionRows rows;
	EXPECT_EQ("no access", describeShardAccess(rows, ShardA));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_open"));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_dev"));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_restricted"));
}

TEST(ShardAccess, EmptySetAndUnknownMembers)
{
	TPermissionRows rows = makeRows(-1, "");
	EXPECT_EQ("no access", describeShardAccess(rows, ShardA));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_open"));

	// members are taken as the database gives them; anything else in the value gives nothing
	TPermissionRows odd = makeRows(-1, "open,CLOSE,,RESTRICTED,RESTRICTED");
	EXPECT_EQ("RESTRICTED", describeShardAccess(odd, ShardA));
}

TEST(ShardAccess, ClosedShard)
{
	// whatever the rows hold, a closed shard takes nobody
	TPermissionRows rows = makeRows(-1, "OPEN,DEV,RESTRICTED", ShardA, "OPEN,DEV,RESTRICTED");
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "ds_close"));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, "invalid"));
	EXPECT_FALSE(hasShardAccess(rows, ShardA, ""));
}

TEST(ShardAccess, ShardLevelDiffersFromDomainStatus)
{
	// An open domain with an open shard A and a restricted shard B for a support team.
	// A player: OPEN on all shards. Shard B asks for RESTRICTED, which the player does not have.
	TPermissionRows player = makeRows(-1, "OPEN");
	EXPECT_TRUE(hasShardAccess(player, ShardA, "ds_open"));
	EXPECT_FALSE(hasShardAccess(player, ShardB, "ds_restricted"));

	// A member of the support team who also plays: OPEN on all shards and RESTRICTED on B.
	TPermissionRows support = makeRows(-1, "OPEN", ShardB, "RESTRICTED");
	EXPECT_TRUE(hasShardAccess(support, ShardA, "ds_open"));
	EXPECT_TRUE(hasShardAccess(support, ShardB, "ds_restricted"));

	// An account for the support shard only: it never has access to shard A, whatever level A asks for
	TPermissionRows supportOnly = makeRows(ShardB, "OPEN,RESTRICTED");
	EXPECT_TRUE(hasShardAccess(supportOnly, ShardB, "ds_restricted"));
	EXPECT_FALSE(hasShardAccess(supportOnly, ShardA, "ds_open"));
	EXPECT_FALSE(hasShardAccess(supportOnly, ShardA, "ds_restricted"));
	EXPECT_FALSE(hasShardAccess(supportOnly, ShardA, "ds_dev"));

	// Shard A put in restricted for a maintenance: the player stays out, an account with RESTRICTED gets in
	EXPECT_FALSE(hasShardAccess(player, ShardA, "ds_restricted"));
	TPermissionRows staff = makeRows(-1, "OPEN,DEV,RESTRICTED");
	EXPECT_TRUE(hasShardAccess(staff, ShardA, "ds_restricted"));
	EXPECT_TRUE(hasShardAccess(staff, ShardA, "ds_dev"));
}

TEST(ShardAccess, PrivilegeOfTheAccountIsNoInput)
{
	// The privilege of an account (":DEV:") is no input of the rule: only its permission rows are. A developer
	// account with an OPEN row has no access to a shard in development, one with the three members has.
	TPermissionRows openOnly = makeRows(-1, "OPEN");
	EXPECT_FALSE(hasShardAccess(openOnly, ShardA, "ds_dev"));
	TPermissionRows asSetupMakesIt = makeRows(-1, "OPEN,DEV,RESTRICTED");
	EXPECT_TRUE(hasShardAccess(asSetupMakesIt, ShardA, "ds_dev"));
}

TEST(MainlandChoice, NoCandidate)
{
	std::vector<CMainlandCandidate> candidates;
	TMainlandChoice choice = mc_lowest_load;
	EXPECT_EQ(0u, chooseMainland(candidates, ShardA, ShardB, choice));
	EXPECT_EQ(mc_none, choice);
}

TEST(MainlandChoice, LowestLoad)
{
	TMainlandChoice choice;
	// an account without characters: no last played shard
	std::vector<CMainlandCandidate> candidates = makeCandidates(50, 10);
	EXPECT_EQ(1u, chooseMainland(candidates, 0, 0, choice));
	EXPECT_EQ(mc_lowest_load, choice);

	candidates = makeCandidates(10, 50);
	EXPECT_EQ(0u, chooseMainland(candidates, 0, 0, choice));
	EXPECT_EQ(mc_lowest_load, choice);

	// the same load: the first in the list
	candidates = makeCandidates(20, 20);
	EXPECT_EQ(0u, chooseMainland(candidates, 0, 0, choice));
	EXPECT_EQ(mc_lowest_load, choice);

	// one candidate
	candidates.clear();
	candidates.push_back(CMainlandCandidate(ShardB, 4000));
	EXPECT_EQ(0u, chooseMainland(candidates, 0, 0, choice));
	EXPECT_EQ(mc_lowest_load, choice);
}

TEST(MainlandChoice, LastPlayed)
{
	TMainlandChoice choice;
	// the home mainland of the last played character, although it has the higher load
	std::vector<CMainlandCandidate> candidates = makeCandidates(50, 10);
	EXPECT_EQ(0u, chooseMainland(candidates, 0, ShardA, choice));
	EXPECT_EQ(mc_last_played, choice);

	candidates = makeCandidates(10, 50);
	EXPECT_EQ(1u, chooseMainland(candidates, 0, ShardB, choice));
	EXPECT_EQ(mc_last_played, choice);
}

TEST(MainlandChoice, LastPlayedShardIsNoCandidate)
{
	TMainlandChoice choice;
	// The shard of the last played character is offline, or the account may not use it: it is not in the list
	// (the caller only lists the shards that are online and that the account may use), the load decides
	std::vector<CMainlandCandidate> candidates = makeCandidates(50, 10);
	EXPECT_EQ(1u, chooseMainland(candidates, 0, 303, choice));
	EXPECT_EQ(mc_lowest_load, choice);

	candidates.clear();
	candidates.push_back(CMainlandCandidate(ShardA, 4000));
	EXPECT_EQ(0u, chooseMainland(candidates, 0, ShardB, choice));
	EXPECT_EQ(mc_lowest_load, choice);
}

TEST(MainlandChoice, ForcedComesFirst)
{
	TMainlandChoice choice;
	// the shard the operator promotes wins over the last played character and over the load
	std::vector<CMainlandCandidate> candidates = makeCandidates(10, 50);
	EXPECT_EQ(1u, chooseMainland(candidates, ShardB, ShardA, choice));
	EXPECT_EQ(mc_forced, choice);

	// when the promoted shard is no candidate (offline, or not for this account), the other rules decide
	EXPECT_EQ(0u, chooseMainland(candidates, 303, ShardA, choice));
	EXPECT_EQ(mc_last_played, choice);
	EXPECT_EQ(0u, chooseMainland(candidates, 303, 0, choice));
	EXPECT_EQ(mc_lowest_load, choice);
	EXPECT_EQ(0u, chooseMainland(candidates, 303, 304, choice));
	EXPECT_EQ(mc_lowest_load, choice);
}

TEST(MainlandChoice, WithoutTheLastPlayedShard)
{
	// What on_joinMainland asks when the preference is switched off (PreferLastPlayedShard), or when the last played
	// character could not be looked up: no last played shard is given, the promoted shard and then the load decide
	TMainlandChoice choice;
	std::vector<CMainlandCandidate> candidates = makeCandidates(50, 10);
	EXPECT_EQ(1u, chooseMainland(candidates, 0, 0, choice));
	EXPECT_EQ(mc_lowest_load, choice);
	EXPECT_EQ(0u, chooseMainland(candidates, ShardA, 0, choice));
	EXPECT_EQ(mc_forced, choice);
}

TEST(MainlandChoice, FallbackBehindTheLastPlayedShard)
{
	// What on_joinMainland does when the last played rule chose: the chosen shard is taken out of the list and the
	// load decides among the rest, for the case that the chosen shard refuses the entry (full, closed)
	std::vector<CMainlandCandidate> candidates;
	candidates.push_back(CMainlandCandidate(ShardA, 900));
	candidates.push_back(CMainlandCandidate(ShardB, 300));
	candidates.push_back(CMainlandCandidate(303, 100));
	TMainlandChoice choice;
	uint chosen = chooseMainland(candidates, 0, ShardA, choice);
	ASSERT_EQ(0u, chosen);
	ASSERT_EQ(mc_last_played, choice);
	candidates.erase(candidates.begin() + chosen);
	uint fallback = chooseMainland(candidates, 0, 0, choice);
	EXPECT_EQ(303u, candidates[fallback].SessionId);
	EXPECT_EQ(mc_lowest_load, choice);
}

TEST(MainlandChoice, Names)
{
	EXPECT_STREQ("ForceWelcomerShard", mainlandChoiceName(mc_forced));
	EXPECT_STREQ("home mainland of the last played character", mainlandChoiceName(mc_last_played));
	EXPECT_STREQ("lowest load", mainlandChoiceName(mc_lowest_load));
	EXPECT_STREQ("none", mainlandChoiceName(mc_none));
}
