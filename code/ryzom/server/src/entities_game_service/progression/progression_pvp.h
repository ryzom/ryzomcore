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

#ifndef RYZOM_PROGRESSION_PVP_H
#define RYZOM_PROGRESSION_PVP_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "progression_common.h"
#include "game_share/pvp_clan.h"

//-----------------------------------------------------------------------------

class CEntityBase;
class CCreature;
class CCharacter;


namespace PROGRESSIONPVP
{


/// convenience typedef for team id
typedef uint16 TTeamId;


/**
 * A damage score table keeps all damages done by a player/team/creature on a player
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005
 */
class CDamageScoreTable
{
public:
	class IReferenceTracker
	{
	protected:
		friend class CDamageScoreTable;

		virtual ~IReferenceTracker() {}

		virtual void cbTeamReferenceAdded(TDataSetRow ownerRowId, TTeamId teamId) =0;
		virtual void cbTeamReferenceRemoved(TDataSetRow ownerRowId, TTeamId teamId) =0;

		virtual void cbPlayerReferenceAdded(TDataSetRow ownerRowId, TDataSetRow playerRowId) =0;
		virtual void cbPlayerReferenceRemoved(TDataSetRow ownerRowId, TDataSetRow playerRowId) =0;

		virtual void cbCreatureReferenceAdded(TDataSetRow ownerRowId, TDataSetRow creatureRowId) =0;
		virtual void cbCreatureReferenceRemoved(TDataSetRow ownerRowId, TDataSetRow creatureRowId) =0;
	};

	struct CWinner
	{
		std::vector<TDataSetRow> Players;	///< 1 or more players for team
		uint32 MaxSkillValue;				///< max skill value
		double TotalDamageRatio;			///< total damage ratio (0 < ratio <= 1)
	};

public:
	/// set the reference tracker at the class level: all CDamageScoreTable objects will notify this tracker
	static void setReferenceTracker(IReferenceTracker * refTracker);

public:
	/// ctor
	CDamageScoreTable(TDataSetRow ownerRowId);
	/// dtor
	~CDamageScoreTable();

	/// return true if the score table is empty
	bool isEmpty() const;

	/// add damage points to the score of the given team
	void addTeamDamage(TTeamId teamId, uint32 damage);
	/// add damage points to the score of the given single player (without team)
	void addPlayerDamage(TDataSetRow playerRowId, uint32 damage);
	/// add damage points to the score of the given creature
	void addCreatureDamage(TDataSetRow creatureRowId, uint32 damage);

	/// change damage score of all entities equitably (add damageDelta/nbEntities to each entity damage score)
	void changeAllDamageEquitably(sint32 damageDelta);

	/// called when a member of a team does a combat action
	void teamUsedSkillWithValue(TTeamId teamId, uint32 skillValue);
	/// called when a single player does a combat action
	void playerUsedSkillWithValue(TDataSetRow playerRowId, uint32 skillValue);

	/// a player joins the given team
	void playerJoinsTeam(TDataSetRow playerRowId, TTeamId teamId);
	/// a player leaves the given team
	void playerLeavesTeam(TDataSetRow playerRowId, TTeamId teamId);

	/// add a player to the team beneficiaries (players which will gain points from the team)
	void addTeamBeneficiary(TTeamId teamId, TDataSetRow playerRowId);
	/// remove a player from the team beneficiaries (players which will gain points from the team)
	void removeTeamBeneficiary(TTeamId teamId, TDataSetRow playerRowId);

	/// remove a team from the score table
	/// \param transferDamages : if true team damages will be transferred to a fake creature
	void removeTeam(TTeamId teamId, bool transferDamages = true);
	/// remove a player from the score table
	/// \param transferDamages : if true player damages will be transferred to a fake creature
	void removePlayer(TDataSetRow playerRowId, bool transferDamages = true);
	/// remove a creature from the score table
	/// \param transferDamages : if true creature damages will be transferred to a fake creature
	void removeCreature(TDataSetRow creatureRowId, bool transferDamages = true);

	/// get players which had the best damage score
	/// \param winners : return winners
	/// \return false if there is no winner (creatures won)
	bool getWinners(std::vector<CWinner> & winners);

	/// dump the damage score table in a log
	void dumpDamageScoreTable(NLMISC::CLog & log) const;

private:
	/// damage score of a team
	struct CTeamDamageScore
	{
		CTeamDamageScore() : TeamId(0), TotalDamage(0.0), MaxSkillValue(0) {}
		CTeamDamageScore(TTeamId teamId, double totalDamage, uint32 maxSkillValue) : TeamId(teamId), TotalDamage(totalDamage), MaxSkillValue(maxSkillValue) {}

		TTeamId	TeamId;
		double	TotalDamage;
		uint32	MaxSkillValue;

		/// players which will get points for this team kill
		std::vector<TDataSetRow>	Beneficiaries;
	};

	/// damage score of a single player
	struct CPlayerDamageScore
	{
		CPlayerDamageScore() : TotalDamage(0.0), MaxSkillValue(0) {}
		CPlayerDamageScore(TDataSetRow playerRowId, double totalDamage, uint32 maxSKillValue) : PlayerRowId(playerRowId), TotalDamage(totalDamage), MaxSkillValue(maxSKillValue) {}

		TDataSetRow	PlayerRowId;
		double		TotalDamage;
		uint32		MaxSkillValue;
	};

	/// damage score of a creature
	struct CCreatureDamageScore
	{
		CCreatureDamageScore() : TotalDamage(0.0) {}
		CCreatureDamageScore(TDataSetRow creatureRowId, double totalDamage) : CreatureRowId(creatureRowId), TotalDamage(totalDamage) {}

		TDataSetRow	CreatureRowId;
		double		TotalDamage;
	};

private:
	/// return the team damage score or NULL if not found
	CTeamDamageScore * getTeamDamageScore(TTeamId teamId);
	/// return the player damage score or NULL if not found
	CPlayerDamageScore * getPlayerDamageScore(TDataSetRow playerRowId);
	/// return the creature damage score or NULL if not found (also return NULL for a fake creature)
	CCreatureDamageScore * getCreatureDamageScore(TDataSetRow creatureRowId);

	/// add a new fake creature with the given damage points
	void addFakeCreature(double damage);

private:
	/// row id of fake creatures
	static const TDataSetRow			_FakeCreatureRowId;
	/// reference tracker of the class
	static IReferenceTracker *			_ReferenceTracker;

	const TDataSetRow					_OwnerRowId;

	std::vector<CTeamDamageScore>		_TeamDamageScores;
	std::vector<CPlayerDamageScore>		_PlayerDamageScores;
	std::vector<CCreatureDamageScore>	_CreatureDamageScores;
};

/**
 * It permits to find all players wounded by an entity (player/team/creature)
 * ie to find all players on which the entity has a damage score.
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005
 */
class CWoundedPlayers : public CDamageScoreTable::IReferenceTracker
{
public:
	void getPlayersWoundedByTeam(TTeamId teamId, std::vector<TDataSetRow> & woundedPlayers) const;
	void getPlayersWoundedByPlayer(TDataSetRow playerRowId, std::vector<TDataSetRow> & woundedPlayers) const;
	void getPlayersWoundedByCreature(TDataSetRow creatureRowId, std::vector<TDataSetRow> & woundedPlayers) const;

protected:
	void cbTeamReferenceAdded(TDataSetRow ownerRowId, TTeamId teamId);
	void cbTeamReferenceRemoved(TDataSetRow ownerRowId, TTeamId teamId);

	void cbPlayerReferenceAdded(TDataSetRow ownerRowId, TDataSetRow playerRowId);
	void cbPlayerReferenceRemoved(TDataSetRow ownerRowId, TDataSetRow playerRowId);

	void cbCreatureReferenceAdded(TDataSetRow ownerRowId, TDataSetRow creatureRowId);
	void cbCreatureReferenceRemoved(TDataSetRow ownerRowId, TDataSetRow creatureRowId);

private:
	typedef CHashMap<TDataSetRow, std::vector<TDataSetRow>, TDataSetRow::CHashCode>	TPlayersWoundedByEntity;
	typedef std::map<TTeamId, std::vector<TDataSetRow> >									TPlayersWoundedByTeam;

	/// players wounded by team
	TPlayersWoundedByTeam		_PlayersWoundedByTeam;
	/// players wounded by other player
	TPlayersWoundedByEntity		_PlayersWoundedByPlayer;
	/// players wounded by creature
	TPlayersWoundedByEntity		_PlayersWoundedByCreature;
};

/**
 * It keeps kills that have been rewarded and which cannot be rewarded again for a lapse of time.
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005
 */
class CRewardedKills
{
public:
	struct CRewardedKiller
	{
		CRewardedKiller(NLMISC::CEntityId killerId, NLMISC::TGameCycle killDate)
			: KillerId(killerId), KillDate(killDate)
		{
		}

		NLMISC::CEntityId	KillerId;
		NLMISC::TGameCycle	KillDate;
	};

public:
	/// called at each tick
	void tickUpdate();

	/// add a rewarded kill (at the current date)
	void addRewardedKill(NLMISC::CEntityId victimId, const std::vector<NLMISC::CEntityId> & killers);

	/// get rewarded killers for the given victim
	/// they cannot be rewarded again for the moment
	void getRewardedKillers(NLMISC::CEntityId victimId, std::vector<CRewardedKiller> & rewardedKillers) const;

private:
	void addRewardedKiller(const CRewardedKiller & rewardedKiller, std::vector<CRewardedKiller> & rewardedKillers) const;

private:
	typedef std::map<NLMISC::CEntityId, std::vector<CRewardedKiller> > TRewardedKillersByVictim;

	/// rewarded killers by victim
	TRewardedKillersByVictim	_RewardedKillersByVictim;
};

/**
 * This manager collects actions of players
 * and keeps their damage scores on each attacked player.
 * It also gives faction and HoF points to players.
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005
 */
class CDamageScoreManager
{
public:
	/// ctor
	CDamageScoreManager();

	/// virtual dtor
	virtual ~CDamageScoreManager();

	/// called at each tick
	void tickUpdate();

	/// report an action
	void reportAction(const TReportAction & action);

	/// a player joins the given team
	void playerJoinsTeam(const CCharacter * playerChar, TTeamId teamId);
	/// a player leaves the given team
	void playerLeavesTeam(const CCharacter * playerChar, TTeamId teamId);
	/// a team is separated
	/// \param members : members of the team before it is separated
	void disbandTeam(TTeamId teamId, const std::list<NLMISC::CEntityId> & members);

	/// a player regenerates HP
	void playerRegenHP(const CCharacter * playerChar, sint32 regenHP);
	/// report a player death
	void playerDeath(CCharacter * playerChar, const CCharacter * finalBlower);
	/// report a spire destroyed
	void spireDestroyed(CCreature * spire, const CCharacter * finalBlower);
	/// report a player resurrection
	void playerResurrected(const CCharacter * playerChar);
	/// report a player respawn
	void playerRespawn(CCharacter * playerChar);

	/// remove a player (player leaves PvP or disconnects, ...)
	void removePlayer(const CCharacter * playerChar);
	/// remove a creature (creature dies, ...)
	void removeCreature(const CCreature * creature);

	/// dump the damage score table of the given player in a log
	void dumpPlayerDamageScoreTable(const CCharacter * playerChar, NLMISC::CLog & log) const;

private:
	/// add damage to the score of the given actor (or his team if any) for the given target
	void addDamage(const CEntityBase * actor, const CEntityBase * target, uint32 damage);
	/// clear damage points of the given player
	/// \param transferDamages : if true player damages will be transferred to a fake creature
	/// NOTE: if player is in a team, the team keeps all damage points
	void clearDamages(const CCharacter * playerChar, bool transferDamages);
	/// clear damage points of the given creature
	/// \param transferDamages : if true creature damages will be transferred to a fake creature
	void clearDamages(const CCreature * creature, bool transferDamages);

	/// change damage score of all entities equitably for the given player
	/// NOTE: it may delete the score table if it remains empty
	void changeAllDamageEquitably(TDataSetRow playerRowId, sint32 damageDelta);

	/// remove damage score table of the given player and all dependencies
	void removeDamageScoreTable(TDataSetRow playerRowId);

	/// return true if the given action is offensive
	bool isOffensiveAction(const TReportAction & action) const;
	/// return true if the given action is curative
	bool isCurativeAction(const TReportAction & action) const;

	bool canPlayerWinPoints(const CCharacter * winnerChar, CCharacter * victimeChar) const;

	/// return true if the given player is in 'faction PvP' (in a faction PvP zone)
	/// \param faction : if not NULL return the faction of the player in the PvP zone where he is
	/// \param gainFactionPoints : if not NULL return true if the zone gives faction points or not
	bool playerInFactionPvP(const CCharacter * playerChar, PVP_CLAN::TPVPClan * faction = NULL, bool * withFactionPoints = NULL) const;

	/// return the number of faction points that the winner will receive
	uint32 computeFactionPointsForWinner(const CDamageScoreTable::CWinner & winner, uint32 victimSkillValue);

	/// return the number of HoF points that the winner will receive
	uint32 computeHoFPointsForWinner(const CDamageScoreTable::CWinner & winner, uint32 victimSkillValue);

	/// add/remove faction points to a player
	/// \return the applied delta (a negative delta may have been modified up to zero to keep faction points positive)
	sint32 changePlayerFactionPoints(CCharacter * playerChar, PVP_CLAN::TPVPClan faction, sint32 fpDelta);

	/// add/remove pvp points to a player
	/// \return the applied delta (a negative delta may have been modified up to zero to keep pvp points positive)
	sint32 changePlayerPvpPoints(CCharacter * playerChar, sint32 fpDelta);

	/// add/remove HoF points to a player and his guild if his SDB PvP path is defined
	void changePlayerHoFPoints(CCharacter * playerChar, sint32 hofpDelta);

private:
	struct CPointLoss
	{
		bool				PlayerLosesFactionPoints;
		PVP_CLAN::TPVPClan	PlayerFaction;
		uint32				FactionPointLoss;
		uint32				HoFPointLoss;
	};

	typedef CHashMap<TDataSetRow, CDamageScoreTable, TDataSetRow::CHashCode>	TDamageScoreTablesByPlayer;
	typedef std::map<NLMISC::CEntityId, CPointLoss>									TPointLossByPlayer;

	/// damage score tables by player
	TDamageScoreTablesByPlayer	_DamageScoreTablesByPlayer;

	/// players wounded by team/player/creature
	CWoundedPlayers				_WoundedPlayers;

	/// rewarded kills which cannot be rewarded again
	CRewardedKills				_RewardedKills;

	/// point loss by player (it is applied if the player respawn)
	TPointLossByPlayer			_PointLossByPlayer;
};

/**
 * This is the interface for the character progression in PvP
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005
 */
class CCharacterProgressionPVP : public CDamageScoreManager
{
	NL_INSTANCE_COUNTER_DECL(CCharacterProgressionPVP);

public:
	/// get the singleton instance
	static CCharacterProgressionPVP * getInstance()
	{
		if (_Instance == NULL)
		{
			_Instance = new CCharacterProgressionPVP;
		}
		return _Instance;
	}

private:
	/// private ctor
	CCharacterProgressionPVP() {}

	/// singleton instance
	static CCharacterProgressionPVP * _Instance;
};


} // namespace PROGRESSIONPVP


#endif // RYZOM_PROGRESSION_PVP_H

/* End of file progression_pvp.h */


