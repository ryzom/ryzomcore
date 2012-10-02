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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "progression_pvp.h"

#include "player_manager/character.h"
#include "player_manager/player.h"
#include "player_manager/player_manager.h"
#include "entity_manager/entity_manager.h"
#include "creature_manager/creature.h"
#include "creature_manager/creature_manager.h"
#include "team_manager/team_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "pvp_manager/pvp_manager_2.h"
#include "pvp_manager/pvp.h"
#include "pvp_manager/pvp_zone.h"
#include "stat_db.h"
#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"

//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


namespace PROGRESSIONPVP
{


NL_INSTANCE_COUNTER_IMPL(CCharacterProgressionPVP);

CVariable<double>	MaxDistanceForPVPPointsGain("egs", "MaxDistanceForPVPPointsGain", "max distance from PvP combat to gain PvP points (faction and HoF points) from team PvP kills (in meters)", 50.0, 0, true);
CVariable<sint32>	MinPVPDeltaLevel("egs", "MinPVPDeltaLevel", "minimum delta level used to compute the faction points gain", -50, 0, true);
CVariable<sint32>	MaxPVPDeltaLevel("egs", "MaxPVPDeltaLevel", "maximum delta level used to compute the faction points gain", 50, 0, true);
CVariable<double>	PVPTeamMemberDivisorValue("egs", "PVPTeamMemberDivisorValue", "for team PvP progression add this value to the faction points divisor for each team member above one", 1.0, 0, true);
CVariable<double>	PVPFactionPointBase("egs", "PVPFactionPointBase", "it is the base used in faction point gain formula", 5.0, 0, true);
CVariable<double>	PVPHoFPointBase("egs", "PVPHoFPointBase", "it is the base used in HoF point gain formula", 5.0, 0, true);
CVariable<double>	PVPFactionPointLossFactor("egs", "PVPFactionPointLossFactor", "in faction PvP the killed players loses the faction points gained per killer multiplied by this factor", 0.1, 0, true);
CVariable<double>	PVPHoFPointLossFactor("egs", "PVPHoFPointLossFactor", "in faction PvP the killed players loses the HoF points gained per killer multiplied by this factor", 0.5, 0, true);
CVariable<uint32>	TimeWithoutPointForSamePVPKill("egs", "TimeWithoutPointForSamePVPKill", "players will not get any point for the same PvP kill for this time in seconds", 300, 0, true);


const TDataSetRow						CDamageScoreTable::_FakeCreatureRowId; // INVALID_DATASET_ROW
CDamageScoreTable::IReferenceTracker *	CDamageScoreTable::_ReferenceTracker = NULL;
CCharacterProgressionPVP *				CCharacterProgressionPVP::_Instance = NULL;

//-----------------------------------------------------------------------------
// helpers
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// remove an element from a vector (only the first matching element is removed and the vector order is changed)
// \return true if element was found and removed
template <typename TElem>
static inline bool removeElementFromVector(TElem elem, std::vector<TElem> & vec)
{
	for (uint i = 0; i < vec.size(); i++)
	{
		if (vec[i] == elem)
		{
			vec[i] = vec.back();
			vec.pop_back();
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
static inline CRewardedKills::CRewardedKiller * getRewardedKiller(NLMISC::CEntityId playerId, vector<CRewardedKills::CRewardedKiller> & rewardedKillers)
{
	for (uint i = 0; i < rewardedKillers.size(); i++)
	{
		if (playerId == rewardedKillers[i].KillerId)
			return &rewardedKillers[i];
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// methods CDamageScoreTable
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CDamageScoreTable::setReferenceTracker(IReferenceTracker * refTracker)
{
	_ReferenceTracker = refTracker;
}

//-----------------------------------------------------------------------------
CDamageScoreTable::CDamageScoreTable(TDataSetRow ownerRowId) : _OwnerRowId(ownerRowId)
{
}

//-----------------------------------------------------------------------------
CDamageScoreTable::~CDamageScoreTable()
{
	if (_ReferenceTracker != NULL)
	{
		for (uint i = 0; i < _TeamDamageScores.size(); i++)
		{
			_ReferenceTracker->cbTeamReferenceRemoved(_OwnerRowId, _TeamDamageScores[i].TeamId);
		}
		for (uint i = 0; i < _PlayerDamageScores.size(); i++)
		{
			_ReferenceTracker->cbPlayerReferenceRemoved(_OwnerRowId, _PlayerDamageScores[i].PlayerRowId);
		}
		for (uint i = 0; i < _CreatureDamageScores.size(); i++)
		{
			if (_CreatureDamageScores[i].CreatureRowId != _FakeCreatureRowId)
				_ReferenceTracker->cbCreatureReferenceRemoved(_OwnerRowId, _CreatureDamageScores[i].CreatureRowId);
		}
	}
}

//-----------------------------------------------------------------------------
bool CDamageScoreTable::isEmpty() const
{
	return (_TeamDamageScores.empty() && _PlayerDamageScores.empty() && _CreatureDamageScores.empty());
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::addTeamDamage(TTeamId teamId, uint32 damage)
{
	nlassert(damage > 0);

	CTeamDamageScore * teamScore = getTeamDamageScore(teamId);
	if (teamScore == NULL)
	{
		_TeamDamageScores.push_back(CTeamDamageScore(teamId, damage, 0));
		if (_ReferenceTracker != NULL)
			_ReferenceTracker->cbTeamReferenceAdded(_OwnerRowId, teamId);
	}
	else
	{
		teamScore->TotalDamage += damage;
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::addPlayerDamage(TDataSetRow playerRowId, uint32 damage)
{
	nlassert(damage > 0);

	CPlayerDamageScore * playerScore = getPlayerDamageScore(playerRowId);
	if (playerScore == NULL)
	{
		_PlayerDamageScores.push_back(CPlayerDamageScore(playerRowId, damage, 0));
		if (_ReferenceTracker != NULL)
			_ReferenceTracker->cbPlayerReferenceAdded(_OwnerRowId, playerRowId);
	}
	else
	{
		playerScore->TotalDamage += damage;
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::addCreatureDamage(TDataSetRow creatureRowId, uint32 damage)
{
	nlassert(damage > 0);

	CCreatureDamageScore * creatureScore = getCreatureDamageScore(creatureRowId);
	if (creatureScore == NULL)
	{
		_CreatureDamageScores.push_back(CCreatureDamageScore(creatureRowId, damage));
		if (_ReferenceTracker != NULL)
			_ReferenceTracker->cbCreatureReferenceAdded(_OwnerRowId, creatureRowId);
	}
	else
	{
		creatureScore->TotalDamage += damage;
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::changeAllDamageEquitably(sint32 damageDelta)
{
	if (damageDelta == 0)
		return;

	const uint32 nbEntities = (uint32)(_TeamDamageScores.size() + _PlayerDamageScores.size() + _CreatureDamageScores.size());
	if (nbEntities == 0)
		return;

	const double damageDeltaPerEntity = double(damageDelta) / nbEntities;

	// apply delta to each team/player/creature score
	uint i = 0;
	while (i < _TeamDamageScores.size())
	{
		_TeamDamageScores[i].TotalDamage += damageDeltaPerEntity;

		// remove the score if no damage points anymore
		if (_TeamDamageScores[i].TotalDamage <= 0.0)
		{
			TTeamId teamId = _TeamDamageScores[i].TeamId;

			_TeamDamageScores[i] = _TeamDamageScores.back();
			_TeamDamageScores.pop_back();
			if (_ReferenceTracker != NULL)
				_ReferenceTracker->cbTeamReferenceRemoved(_OwnerRowId, teamId);
			continue;
		}
		i++;
	}

	i = 0;
	while (i < _PlayerDamageScores.size())
	{
		_PlayerDamageScores[i].TotalDamage += damageDeltaPerEntity;

		// remove the score if no damage points anymore
		if (_PlayerDamageScores[i].TotalDamage <= 0.0)
		{
			TDataSetRow playerRowId = _PlayerDamageScores[i].PlayerRowId;

			_PlayerDamageScores[i] = _PlayerDamageScores.back();
			_PlayerDamageScores.pop_back();
			if (_ReferenceTracker != NULL)
				_ReferenceTracker->cbPlayerReferenceRemoved(_OwnerRowId, playerRowId);
			continue;
		}
		i++;
	}

	i = 0;
	while (i < _CreatureDamageScores.size())
	{
		_CreatureDamageScores[i].TotalDamage += damageDeltaPerEntity;

		// remove the score if no damage points anymore
		if (_CreatureDamageScores[i].TotalDamage <= 0.0)
		{
			TDataSetRow creatureRowId = _CreatureDamageScores[i].CreatureRowId;

			_CreatureDamageScores[i] = _CreatureDamageScores.back();
			_CreatureDamageScores.pop_back();
			if (_ReferenceTracker != NULL && creatureRowId != _FakeCreatureRowId)
				_ReferenceTracker->cbCreatureReferenceRemoved(_OwnerRowId, creatureRowId);
			continue;
		}
		i++;
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::teamUsedSkillWithValue(TTeamId teamId, uint32 skillValue)
{
	CTeamDamageScore * teamScore = getTeamDamageScore(teamId);
	if (teamScore == NULL)
		return;

	if (skillValue > teamScore->MaxSkillValue)
		teamScore->MaxSkillValue = skillValue;
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::playerUsedSkillWithValue(TDataSetRow playerRowId, uint32 skillValue)
{
	CPlayerDamageScore * playerScore = getPlayerDamageScore(playerRowId);
	if (playerScore == NULL)
		return;

	if (skillValue > playerScore->MaxSkillValue)
		playerScore->MaxSkillValue = skillValue;
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::playerJoinsTeam(TDataSetRow playerRowId, TTeamId teamId)
{
	CTeamDamageScore * teamScore = getTeamDamageScore(teamId);
	CPlayerDamageScore * playerScore = getPlayerDamageScore(playerRowId);

	// nothing to do if the player has no score
	if (playerScore == NULL)
		return;

	// create the team score if necessary
	if (teamScore == NULL)
	{
		_TeamDamageScores.push_back(CTeamDamageScore(teamId, 0.0, 0));
		teamScore = &_TeamDamageScores.back();
		if (_ReferenceTracker != NULL)
			_ReferenceTracker->cbTeamReferenceAdded(_OwnerRowId, teamId);
	}

	// add the player score to the team score
	teamScore->TotalDamage += playerScore->TotalDamage;
	if (playerScore->MaxSkillValue > teamScore->MaxSkillValue)
		teamScore->MaxSkillValue = playerScore->MaxSkillValue;

	// if player brings damage points to his team, add him as beneficiary
	if (playerScore->TotalDamage > 0.0)
	{
		addTeamBeneficiary(teamId, playerRowId);
	}

	// delete the player score
	*playerScore = _PlayerDamageScores.back();
	_PlayerDamageScores.pop_back();
	if (_ReferenceTracker != NULL)
		_ReferenceTracker->cbPlayerReferenceRemoved(_OwnerRowId, playerRowId);
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::playerLeavesTeam(TDataSetRow playerRowId, TTeamId teamId)
{
	CTeamDamageScore * teamScore = getTeamDamageScore(teamId);
	CPlayerDamageScore * playerScore = getPlayerDamageScore(playerRowId);

	// nothing to do if the team has no score
	if (teamScore == NULL)
		return;

	vector<TDataSetRow> & beneficiaries = teamScore->Beneficiaries;
	const uint32 nbBeneficiaries = (uint32)beneficiaries.size();

	// remove player from beneficiaries
	const bool isBeneficiary = removeElementFromVector(playerRowId, beneficiaries);

	// nothing to do if the player was not beneficiary
	if (!isBeneficiary)
		return;

	// create a score for the new single player
	if (playerScore == NULL)
	{
		_PlayerDamageScores.push_back(CPlayerDamageScore(playerRowId, 0.0, 0));
		playerScore = &_PlayerDamageScores.back();
		if (_ReferenceTracker != NULL)
			_ReferenceTracker->cbPlayerReferenceAdded(_OwnerRowId, playerRowId);
	}
	else
	{
		STOP("a player leaving a team should not have its own player score!");
	}

	// move damage points for the player from team score to player score
	const double damagePerBeneficiary = teamScore->TotalDamage / nbBeneficiaries;
	teamScore->TotalDamage -= damagePerBeneficiary;
	playerScore->TotalDamage += damagePerBeneficiary;
	playerScore->MaxSkillValue = teamScore->MaxSkillValue;
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::addTeamBeneficiary(TTeamId teamId, TDataSetRow playerRowId)
{
	CTeamDamageScore * teamScore = getTeamDamageScore(teamId);
	if (teamScore == NULL)
		return;

	vector<TDataSetRow> & beneficiaries = teamScore->Beneficiaries;
	for (uint i = 0; i < beneficiaries.size(); i++)
	{
		if (beneficiaries[i] == playerRowId)
			return;
	}
	beneficiaries.push_back(playerRowId);
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::removeTeamBeneficiary(TTeamId teamId, TDataSetRow playerRowId)
{
	CTeamDamageScore * teamScore = getTeamDamageScore(teamId);
	if (teamScore == NULL)
		return;

	removeElementFromVector(playerRowId, teamScore->Beneficiaries);
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::removeTeam(TTeamId teamId, bool transferDamages)
{
	CTeamDamageScore * teamScore = getTeamDamageScore(teamId);
	if (teamScore == NULL)
		return;

	if (transferDamages)
	{
		addFakeCreature(teamScore->TotalDamage);
	}

	// delete the team score
	*teamScore = _TeamDamageScores.back();
	_TeamDamageScores.pop_back();
	if (_ReferenceTracker != NULL)
		_ReferenceTracker->cbTeamReferenceRemoved(_OwnerRowId, teamId);
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::removePlayer(TDataSetRow playerRowId, bool transferDamages)
{
	CPlayerDamageScore * playerScore = getPlayerDamageScore(playerRowId);
	if (playerScore == NULL)
		return;

	if (transferDamages)
	{
		addFakeCreature(playerScore->TotalDamage);
	}

	// delete the player score
	*playerScore = _PlayerDamageScores.back();
	_PlayerDamageScores.pop_back();
	if (_ReferenceTracker != NULL)
		_ReferenceTracker->cbPlayerReferenceRemoved(_OwnerRowId, playerRowId);
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::removeCreature(TDataSetRow creatureRowId, bool transferDamages)
{
	CCreatureDamageScore * creatureScore = getCreatureDamageScore(creatureRowId);
	if (creatureScore == NULL)
		return;

	if (transferDamages)
	{
		addFakeCreature(creatureScore->TotalDamage);
	}

	// delete the creature score
	*creatureScore = _CreatureDamageScores.back();
	_CreatureDamageScores.pop_back();
	if (_ReferenceTracker != NULL && creatureRowId != _FakeCreatureRowId)
		_ReferenceTracker->cbCreatureReferenceRemoved(_OwnerRowId, creatureRowId);
}

//-----------------------------------------------------------------------------
bool CDamageScoreTable::getWinners(std::vector<CWinner> & winners)
{
	winners.clear();

	double creaturesDamage = 0.0;
	for (uint i = 0; i < _CreatureDamageScores.size(); i++)
	{
		creaturesDamage += _CreatureDamageScores[i].TotalDamage;
	}

	double playersDamage = 0.0;
	for (uint i = 0; i < _TeamDamageScores.size(); i++)
	{
		if (_TeamDamageScores[i].TotalDamage <= 0.0)
			continue;

		if (_TeamDamageScores[i].Beneficiaries.size() > 0)
		{
			CWinner winner;
			winner.Players			= _TeamDamageScores[i].Beneficiaries;
			winner.MaxSkillValue	= _TeamDamageScores[i].MaxSkillValue;
			winner.TotalDamageRatio	= _TeamDamageScores[i].TotalDamage;
			winners.push_back(winner);
		}
		playersDamage += _TeamDamageScores[i].TotalDamage;
	}
	for (uint i = 0; i < _PlayerDamageScores.size(); i++)
	{
		if (_PlayerDamageScores[i].TotalDamage <= 0.0)
			continue;

		CWinner winner;
		winner.Players.push_back(_PlayerDamageScores[i].PlayerRowId);
		winner.MaxSkillValue	= _PlayerDamageScores[i].MaxSkillValue;
		winner.TotalDamageRatio	= _PlayerDamageScores[i].TotalDamage;
		winners.push_back(winner);

		playersDamage += _PlayerDamageScores[i].TotalDamage;
	}

	if (winners.empty())
		return false;

	// if creatures made more damage than players there is no winner (creatures won!)
	if (creaturesDamage > playersDamage)
	{
		winners.clear();
		return false;
	}

	const double totalDamage = creaturesDamage + playersDamage;

	// compute total damage ratio for each winner
	for (uint i = 0; i < winners.size(); i++)
	{
		winners[i].TotalDamageRatio /= totalDamage;
	}

	return true;
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::dumpDamageScoreTable(NLMISC::CLog & log) const
{
	// dump team scores
	for (uint i = 0; i < _TeamDamageScores.size(); i++)
	{
		const CTeamDamageScore & teamScore = _TeamDamageScores[i];

		CEntityId leaderId;
		string leaderName;

		CTeam * team = TeamManager.getTeam(teamScore.TeamId);
		if (team != NULL)
		{
			leaderId = team->getLeader();
			CCharacter * leaderChar = PlayerManager.getChar(leaderId);
			if (leaderChar != NULL)
			{
				leaderName = leaderChar->getName().toUtf8();
			}
		}

		string beneficiaries;
		for (uint k = 0; k < teamScore.Beneficiaries.size(); k++)
		{
			CCharacter * beneficiaryChar = PlayerManager.getChar(teamScore.Beneficiaries[k]);
			if (beneficiaryChar != NULL)
				beneficiaries += "'" + beneficiaryChar->getName().toUtf8() + "'";
			else
				beneficiaries += teamScore.Beneficiaries[k].toString();

			if (k != teamScore.Beneficiaries.size() - 1)
				beneficiaries += ", ";
		}

		log.displayNL("Team %u, leader %s '%s', damage = %g, max skill value = %u, beneficiaries = (%s)",
			teamScore.TeamId,
			leaderId.toString().c_str(),
			leaderName.c_str(),
			teamScore.TotalDamage,
			teamScore.MaxSkillValue,
			beneficiaries.c_str()
			);
	}

	// dump single player scores
	for (uint i = 0; i < _PlayerDamageScores.size(); i++)
	{
		const CPlayerDamageScore & playerScore = _PlayerDamageScores[i];

		CEntityId playerId;
		string playerName;
		CCharacter * playerChar = PlayerManager.getChar(playerScore.PlayerRowId);
		if (playerChar != NULL)
		{
			playerId = playerChar->getId();
			playerName = playerChar->getName().toUtf8();
		}

		log.displayNL("Single player %s '%s', damage = %g, max skill value = %u",
			playerId.toString().c_str(),
			playerName.c_str(),
			playerScore.TotalDamage,
			playerScore.MaxSkillValue
			);
	}

	// dump creature scores
	for (uint i = 0; i < _CreatureDamageScores.size(); i++)
	{
		const CCreatureDamageScore & creatureScore = _CreatureDamageScores[i];

		if (creatureScore.CreatureRowId == _FakeCreatureRowId)
		{
			log.displayNL("Fake creature, damage = %g", creatureScore.TotalDamage);
			continue;
		}

		CEntityId creatureId;
		string creatureName;
		CCreature * creature = CreatureManager.getCreature(creatureScore.CreatureRowId);
		if (creature != NULL)
		{
			creatureId = creature->getId();
			if (creature->getForm() != NULL)
			{
				creatureName = EGSPD::CPeople::toString(creature->getForm()->getRace());
			}
		}

		log.displayNL("Creature %s '%s', damage = %g",
			creatureId.toString().c_str(),
			creatureName.c_str(),
			creatureScore.TotalDamage
			);
	}
}

//-----------------------------------------------------------------------------
CDamageScoreTable::CTeamDamageScore * CDamageScoreTable::getTeamDamageScore(TTeamId teamId)
{
	for (uint i = 0; i < _TeamDamageScores.size(); i++)
	{
		if (_TeamDamageScores[i].TeamId == teamId)
			return &_TeamDamageScores[i];
	}
	return NULL;
}

//-----------------------------------------------------------------------------
CDamageScoreTable::CPlayerDamageScore * CDamageScoreTable::getPlayerDamageScore(TDataSetRow playerRowId)
{
	for (uint i = 0; i < _PlayerDamageScores.size(); i++)
	{
		if (_PlayerDamageScores[i].PlayerRowId == playerRowId)
			return &_PlayerDamageScores[i];
	}
	return NULL;
}

//-----------------------------------------------------------------------------
CDamageScoreTable::CCreatureDamageScore * CDamageScoreTable::getCreatureDamageScore(TDataSetRow creatureRowId)
{
	// cannot get fake creatures index (they all have the same row id)
	if (creatureRowId == _FakeCreatureRowId)
		return NULL;

	for (uint i = 0; i < _CreatureDamageScores.size(); i++)
	{
		if (_CreatureDamageScores[i].CreatureRowId == creatureRowId)
			return &_CreatureDamageScores[i];
	}
	return NULL;
}

//-----------------------------------------------------------------------------
void CDamageScoreTable::addFakeCreature(double damage)
{
	_CreatureDamageScores.push_back(CCreatureDamageScore(_FakeCreatureRowId, damage));
}

//-----------------------------------------------------------------------------
// methods CWoundedPlayers
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CWoundedPlayers::getPlayersWoundedByTeam(TTeamId teamId, std::vector<TDataSetRow> & woundedPlayers) const
{
	TPlayersWoundedByTeam::const_iterator it = _PlayersWoundedByTeam.find(teamId);
	if (it == _PlayersWoundedByTeam.end())
		woundedPlayers.clear();
	else
		woundedPlayers = (*it).second;
}

//-----------------------------------------------------------------------------
void CWoundedPlayers::getPlayersWoundedByPlayer(TDataSetRow playerRowId, std::vector<TDataSetRow> & woundedPlayers) const
{
	TPlayersWoundedByEntity::const_iterator it = _PlayersWoundedByPlayer.find(playerRowId);
	if (it == _PlayersWoundedByPlayer.end())
		woundedPlayers.clear();
	else
		woundedPlayers = (*it).second;
}

//-----------------------------------------------------------------------------
void CWoundedPlayers::getPlayersWoundedByCreature(TDataSetRow creatureRowId, std::vector<TDataSetRow> & woundedPlayers) const
{
	TPlayersWoundedByEntity::const_iterator it = _PlayersWoundedByCreature.find(creatureRowId);
	if (it == _PlayersWoundedByCreature.end())
		woundedPlayers.clear();
	else
		woundedPlayers = (*it).second;
}

//-----------------------------------------------------------------------------
void CWoundedPlayers::cbTeamReferenceAdded(TDataSetRow ownerRowId, TTeamId teamId)
{
	vector<TDataSetRow> & targets = _PlayersWoundedByTeam[teamId];

#ifdef NL_DEBUG
	nlassert(!removeElementFromVector(ownerRowId, targets));
#endif // NL_DEBUG

	targets.push_back(ownerRowId);
}

//-----------------------------------------------------------------------------
void CWoundedPlayers::cbTeamReferenceRemoved(TDataSetRow ownerRowId, TTeamId teamId)
{
	TPlayersWoundedByTeam::iterator it = _PlayersWoundedByTeam.find(teamId);
	BOMB_IF(it == _PlayersWoundedByTeam.end(), "wounded player not found!", return);

	vector<TDataSetRow> & targets = (*it).second;
	if (removeElementFromVector(ownerRowId, targets))
	{
		if (targets.empty())
			_PlayersWoundedByTeam.erase(it);
		return;
	}
	STOP("reference not found!");
}

//-----------------------------------------------------------------------------
void CWoundedPlayers::cbPlayerReferenceAdded(TDataSetRow ownerRowId, TDataSetRow playerRowId)
{
	vector<TDataSetRow> & targets = _PlayersWoundedByPlayer[playerRowId];

#ifdef NL_DEBUG
	nlassert(!removeElementFromVector(ownerRowId, targets));
#endif // NL_DEBUG

	targets.push_back(ownerRowId);
}

//-----------------------------------------------------------------------------
void CWoundedPlayers::cbPlayerReferenceRemoved(TDataSetRow ownerRowId, TDataSetRow playerRowId)
{
	TPlayersWoundedByEntity::iterator it = _PlayersWoundedByPlayer.find(playerRowId);
	BOMB_IF(it == _PlayersWoundedByPlayer.end(), "wounded player not found!", return);

	vector<TDataSetRow> & targets = (*it).second;
	if (removeElementFromVector(ownerRowId, targets))
	{
		if (targets.empty())
			_PlayersWoundedByPlayer.erase(it);
		return;
	}
	STOP("reference not found!");
}

//-----------------------------------------------------------------------------
void CWoundedPlayers::cbCreatureReferenceAdded(TDataSetRow ownerRowId, TDataSetRow creatureRowId)
{
	vector<TDataSetRow> & targets = _PlayersWoundedByCreature[creatureRowId];

#ifdef NL_DEBUG
	nlassert(!removeElementFromVector(ownerRowId, targets));
#endif // NL_DEBUG

	targets.push_back(ownerRowId);
}

//-----------------------------------------------------------------------------
void CWoundedPlayers::cbCreatureReferenceRemoved(TDataSetRow ownerRowId, TDataSetRow creatureRowId)
{
	TPlayersWoundedByEntity::iterator it = _PlayersWoundedByCreature.find(creatureRowId);
	BOMB_IF(it == _PlayersWoundedByCreature.end(), "wounded player not found!", return);

	vector<TDataSetRow> & targets = (*it).second;
	if (removeElementFromVector(ownerRowId, targets))
	{
		if (targets.empty())
			_PlayersWoundedByCreature.erase(it);
		return;
	}
	STOP("reference not found!");
}

//-----------------------------------------------------------------------------
// methods CRewardedKills
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CRewardedKills::tickUpdate()
{
	// only update every seconds
	if (CTickEventHandler::getGameCycle() % 10 != 0)
		return;

	H_AUTO(CRewardedKills_tickUpdate);

	const TGameCycle currentDate = CTickEventHandler::getGameCycle();
	const TGameCycle ticksWithoutPoint = TimeWithoutPointForSamePVPKill.get() * 10;

	// remove old enough kills
	TRewardedKillersByVictim::iterator it = _RewardedKillersByVictim.begin();
	while (it != _RewardedKillersByVictim.end())
	{
		vector<CRewardedKiller> & rewardedKillers = (*it).second;

		uint i = 0;
		while (i < rewardedKillers.size())
		{
			if (rewardedKillers[i].KillDate + ticksWithoutPoint <= currentDate)
			{
				rewardedKillers[i] = rewardedKillers.back();
				rewardedKillers.pop_back();
				continue;
			}
			i++;
		}

		if (rewardedKillers.empty())
		{
			TRewardedKillersByVictim::iterator itDel = it;
			++it;
			_RewardedKillersByVictim.erase(itDel);
		}
		else
		{
			++it;
		}
	}
}

//-----------------------------------------------------------------------------
void CRewardedKills::addRewardedKill(NLMISC::CEntityId victimId, const std::vector<NLMISC::CEntityId> & killers)
{
	if (killers.empty())
		return;

	const TGameCycle currentDate = CTickEventHandler::getGameCycle();
	vector<CRewardedKiller> & rewardedKillers = _RewardedKillersByVictim[victimId];
	for (uint i = 0; i < killers.size(); i++)
	{
		addRewardedKiller(CRewardedKiller(killers[i], currentDate), rewardedKillers);
		CCharacter *killer = PlayerManager.getChar(killers[i]);
		CMissionEventKillPlayer killevent( TheDataset.getDataSetRow(victimId) );
		killer->processMissionEvent( killevent );
	}
}

//-----------------------------------------------------------------------------
void CRewardedKills::getRewardedKillers(NLMISC::CEntityId victimId, std::vector<CRewardedKiller> & rewardedKillers) const
{
	rewardedKillers.clear();

	TRewardedKillersByVictim::const_iterator it = _RewardedKillersByVictim.find(victimId);
	if (it == _RewardedKillersByVictim.end())
		return;

	rewardedKillers = (*it).second;
}

//-----------------------------------------------------------------------------
void CRewardedKills::addRewardedKiller(const CRewardedKiller & rewardedKiller, std::vector<CRewardedKiller> & rewardedKillers) const
{
	for (uint i = 0; i < rewardedKillers.size(); i++)
	{
		if (rewardedKiller.KillerId == rewardedKillers[i].KillerId)
		{
			rewardedKillers[i].KillDate = rewardedKiller.KillDate;
			return;
		}
	}

	rewardedKillers.push_back(rewardedKiller);
}

//-----------------------------------------------------------------------------
// methods CDamageScoreManager
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CDamageScoreManager::CDamageScoreManager()
{
	CDamageScoreTable::setReferenceTracker(&_WoundedPlayers);
}

//-----------------------------------------------------------------------------
CDamageScoreManager::~CDamageScoreManager()
{
	CDamageScoreTable::setReferenceTracker(NULL);
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::tickUpdate()
{
	_RewardedKills.tickUpdate();
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::reportAction(const TReportAction & action)
{
	H_AUTO(CDamageScoreManager_reportAction);

	CEntityBase * actor = CEntityBaseManager::getEntityBasePtr(action.ActorRowId);
	if (actor == NULL)
	{
		nlwarning("received report action with unknown actor entity");
		return;
	}

	CEntityBase * target = CEntityBaseManager::getEntityBasePtr(action.TargetRowId);

	const bool isOffensive = isOffensiveAction(action);
	const bool isCurative = isCurativeAction(action);

	// add damage to the score if any
	if (target != NULL && isOffensive && action.Hp > 0)
	{
		addDamage(actor, target, action.Hp);
	}

	// for a creature actor we only need to add damage
	if (actor->getId().getType() != RYZOMID::player)
		return;

	// from here we consider offensive and curative player actions only (ie 'combat' actions)
	if (!isOffensive && !isCurative)
		return;

	CCharacter * actorChar = dynamic_cast<CCharacter *>(actor);
	BOMB_IF(actorChar == NULL, "not a character!", return);

	CCharacter * targetChar = NULL;
	if (target && target->getId().getType() == RYZOMID::player)
		targetChar = dynamic_cast<CCharacter *>(target);

	// if the target is a player, he must be in 'faction point PvP'
	if (targetChar != NULL && !playerInFactionPvP(targetChar))
		return;

	if (isCurative)
	{
		if (targetChar != NULL)
		{
			// apply HP heal as HP regen
			if (action.Hp > 0)
				changeAllDamageEquitably(targetChar->getEntityRowId(), -sint32(action.Hp));

			// discard curative actions on a non team member
			if (actorChar->getTeamId() != CTEAM::InvalidTeamId && actorChar->getTeamId() != targetChar->getTeamId())
				return;
		}
	}

	// get the skill value of the action
	const uint32 skillValue = uint32(actorChar->getSkillBaseValue(action.Skill));

	if (actorChar->getTeamId() != CTEAM::InvalidTeamId)
	{
		// player is in a team
		// register the action on current target and other team targets which are not too far (if any)
		vector<TDataSetRow> teamTargets;
		_WoundedPlayers.getPlayersWoundedByTeam(actorChar->getTeamId(), teamTargets);
		for (uint i = 0; i < teamTargets.size(); i++)
		{
			// check distance between player and team targets (but the current player target)
			if (target != NULL && teamTargets[i] == target->getEntityRowId())
			{
				const double distance = PHRASE_UTILITIES::getDistance(actorChar->getEntityRowId(), teamTargets[i]);
				if (distance > MaxDistanceForPVPPointsGain)
					continue;
				// TODO: kxu: remove entities that are too far (avoid mem leak)
			}

			TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(teamTargets[i]);
			BOMB_IF(it == _DamageScoreTablesByPlayer.end(), "player damage score table not found!", continue);

			CDamageScoreTable & scoreTable = (*it).second;
			scoreTable.teamUsedSkillWithValue(actorChar->getTeamId(), skillValue);
			scoreTable.addTeamBeneficiary(actorChar->getTeamId(), actorChar->getEntityRowId());
		}
	}
	else // single player (without team)
	{
		// register the action on current target and other player targets which are not too far (if any)
		vector<TDataSetRow> playerTargets;
		_WoundedPlayers.getPlayersWoundedByPlayer(actorChar->getEntityRowId(), playerTargets);
		for (uint i = 0; i < playerTargets.size(); i++)
		{
			// check distance between player and targets (but the current player target)
			if (target == NULL || playerTargets[i] != target->getEntityRowId())
			{
				const double distance = PHRASE_UTILITIES::getDistance(actorChar->getEntityRowId(), playerTargets[i]);
				if (distance > MaxDistanceForPVPPointsGain)
					continue;
				// TODO: kxu: remove entities that are too far (avoid mem leak)
			}

			TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(playerTargets[i]);
			BOMB_IF(it == _DamageScoreTablesByPlayer.end(), "player damage score table not found!", continue);

			CDamageScoreTable & scoreTable = (*it).second;
			scoreTable.playerUsedSkillWithValue(actorChar->getEntityRowId(), skillValue);
		}
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::playerJoinsTeam(const CCharacter * playerChar, TTeamId teamId)
{
	H_AUTO(CDamageScoreManager_playerJoinsTeam);

	nlassert(playerChar != NULL);

	if (!playerInFactionPvP(playerChar))
		return;

	vector<TDataSetRow> targets;
	_WoundedPlayers.getPlayersWoundedByPlayer(playerChar->getEntityRowId(), targets);
	for (uint i = 0; i < targets.size(); i++)
	{
		TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(targets[i]);
		BOMB_IF(it == _DamageScoreTablesByPlayer.end(), "found no table!", continue);

		CDamageScoreTable & scoreTable = (*it).second;
		scoreTable.playerJoinsTeam(playerChar->getEntityRowId(), teamId);
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::playerLeavesTeam(const CCharacter * playerChar, TTeamId teamId)
{
	H_AUTO(CDamageScoreManager_playerLeavesTeam);

	nlassert(playerChar != NULL);

	if (!playerInFactionPvP(playerChar))
		return;

	vector<TDataSetRow> targets;
	_WoundedPlayers.getPlayersWoundedByTeam(teamId, targets);
	for (uint i = 0; i < targets.size(); i++)
	{
		TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(targets[i]);
		BOMB_IF(it == _DamageScoreTablesByPlayer.end(), "found no table!", continue);

		CDamageScoreTable & scoreTable = (*it).second;
		scoreTable.playerLeavesTeam(playerChar->getEntityRowId(), teamId);
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::disbandTeam(TTeamId teamId, const std::list<NLMISC::CEntityId> & members)
{
	H_AUTO(CDamageScoreManager_disbandTeam);

	vector<TDataSetRow> memberRowIds;
	for (list<CEntityId>::const_iterator itMember = members.begin(); itMember != members.end(); ++itMember)
	{
		const CCharacter * memberChar = PlayerManager.getChar(*itMember);
		if (memberChar == NULL || !playerInFactionPvP(memberChar))
			continue;

		memberRowIds.push_back(memberChar->getEntityRowId());
	}

	vector<TDataSetRow> targets;
	_WoundedPlayers.getPlayersWoundedByTeam(teamId, targets);
	for (uint i = 0; i < targets.size(); i++)
	{
		TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(targets[i]);
		BOMB_IF(it == _DamageScoreTablesByPlayer.end(), "found no table!", continue);

		CDamageScoreTable & scoreTable = (*it).second;
		for (uint k = 0; k < memberRowIds.size(); k++)
		{
			scoreTable.playerLeavesTeam(memberRowIds[k], teamId);
		}

		// delete the team score (but do not transfer it to a fake creature as it was transferred to players)
		scoreTable.removeTeam(teamId, false);

		// delete the score table if it is empty
		if (scoreTable.isEmpty())
			_DamageScoreTablesByPlayer.erase(it);
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::playerRegenHP(const CCharacter * playerChar, sint32 regenHP)
{
	H_AUTO(CDamageScoreManager_playerRegenHP);

	nlassert(playerChar != NULL);

	if (regenHP <= 0)
		return;

	if (!playerInFactionPvP(playerChar))
		return;

	changeAllDamageEquitably(playerChar->getEntityRowId(), -regenHP);
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::playerDeath(CCharacter * victimChar, const CCharacter * finalBlower)
{
	H_AUTO(CDamageScoreManager_playerDeath);

	nlassert(victimChar != NULL);

//	PVP_CLAN::TPVPClan victimFaction;
	bool victimLosesFactionPoints = false;
//	if (!playerInFactionPvP(victimChar, &victimFaction, &victimLosesFactionPoints))
//		return;

//	PVP_CLAN::TPVPClan finalBlowerFaction;
//	if (!playerInFactionPvP(finalBlower, &finalBlowerFaction))
//		return;

	// check if victim and final blower are in PvP Faction (by tag or by a pvp versus zone)
	/* if(!CPVPManager2::getInstance()->factionWarOccurs(victimFaction, finalBlowerFaction))
	{
		CPVPVersusZone * zone = dynamic_cast<CPVPVersusZone *>(const_cast<CPVPInterface &>(victimChar->getPVPInterface()).getPVPSession());
		if( zone == 0 )
			return;
		PVP_RELATION::TPVPRelation pvpRelation = zone->getPVPRelation(victimChar, const_cast<CCharacter*>(finalBlower));
		if( pvpRelation != PVP_RELATION::Ennemy )
			return;
	}*/

	// a dead player loses his damage points
	clearDamages(victimChar, true);

	TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(victimChar->getEntityRowId());
	if (it == _DamageScoreTablesByPlayer.end())
		return;

	CDamageScoreTable & scoreTable = (*it).second;

	vector<CDamageScoreTable::CWinner> winners;
	if (!scoreTable.getWinners(winners))
	{
		// debug info
		if (VerboseFactionPoint.get())
		{
			nlinfo("Faction PvP kill: no winner for killed player %s '%s' (creatures won!)",
				victimChar->getId().toString().c_str(),
				victimChar->getName().toUtf8().c_str()
				);
			nlinfo("Damage score table:\n");
			scoreTable.dumpDamageScoreTable(*InfoLog);
			nlinfo("----------------------------------------------------------------------------");
		}
		return;
	}

	BOMB_IF(winners.empty(), "winners should not be empty!", return);

	const uint32 victimSkillValue = uint32(victimChar->getSkillBaseValue(victimChar->getBestSkill()));

	// debug info
	if (VerboseFactionPoint.get())
	{
		nlinfo("Faction PvP kill: killed player %s '%s' (best skill value = %u)",
			victimChar->getId().toString().c_str(),
			victimChar->getName().toUtf8().c_str(),
			victimSkillValue
			);
		nlinfo("Damage score table:\n");
		scoreTable.dumpDamageScoreTable(*InfoLog);
		nlinfo("----------------------------------------------------------------------------");
	}

	// get players who cannot win any point for this kill
	vector<CRewardedKills::CRewardedKiller> noPointPlayers;
	_RewardedKills.getRewardedKillers(victimChar->getId(), noPointPlayers);

	// player who gets points for this kill
	vector<CEntityId> rewardedKillers;

	// compute and distribute faction points and HoF points to each winner
	uint32 fpLoss = 0;
	uint32 hofpLoss = 0;
	for (uint i = 0; i < winners.size(); i++)
	{
		uint32 fpForWinner = computeFactionPointsForWinner(winners[i], victimSkillValue);
		uint32 hofpForWinner = computeHoFPointsForWinner(winners[i], victimSkillValue);

		uint32 fpPerPlayer = fpForWinner;
		uint32 hofpPerPlayer = hofpForWinner;
		if (winners[i].Players.size() > 1)
		{
			double teamDivisor = 1.0 + (double(winners[i].Players.size() - 1) * PVPTeamMemberDivisorValue.get());
			fpPerPlayer = uint32(double(fpForWinner) / teamDivisor + 0.5);
			hofpPerPlayer = uint32(double(hofpForWinner) / teamDivisor + 0.5);
		}

		// debug info
		if (VerboseFactionPoint.get())
		{
			nlinfo("Winner %u (%u players): damage percent = %.1f%%, faction points = %u (%u per player), HoF points = %u (%u per player)",
				i,
				winners[i].Players.size(),
				winners[i].TotalDamageRatio * 100.0,
				fpForWinner,
				fpPerPlayer,
				hofpForWinner,
				hofpPerPlayer
				);
		}

		uint nbRewardedMembers = 0; // nb of rewarded members in the team
		const vector<TDataSetRow> & players = winners[i].Players;
		for (uint k = 0; k < players.size(); k++)
		{
			CCharacter * winnerChar = PlayerManager.getChar(players[k]);
			BOMB_IF(winnerChar == NULL, "invalid winner!", continue);

			PVP_CLAN::TPVPClan winnerFaction = PVP_CLAN::None;
			bool winnerGainFactionPoints = true;

			if (!canPlayerWinPoints(winnerChar, victimChar))
				continue;

			//if(!playerInFactionPvP(winnerChar, &winnerFaction, &winnerGainFactionPoints))
			//	continue; // can be in Duel or in other pvp mode.

			CRewardedKills::CRewardedKiller * noPointPlayer = getRewardedKiller(winnerChar->getId(), noPointPlayers);
			if (noPointPlayer != NULL)
			{
				// player cannot gain point for this kill until endDate
				const TGameCycle currentDate = CTickEventHandler::getGameCycle();
				const TGameCycle endDate = noPointPlayer->KillDate + (TimeWithoutPointForSamePVPKill.get() * 10);
				uint32 remainingTime; // in ticks
				if (endDate > currentDate)
					remainingTime = endDate - currentDate;
				else
					remainingTime = 0;

				if (winnerGainFactionPoints)
					winnerChar->sendFactionPointCannotGainYetMessage(victimChar->getId(), remainingTime / 10);
				continue;
			}

			if (winnerGainFactionPoints)
			{
				// Compute Fames delta
				sint32 fameFactor = 0;
				for (uint8 fameIdx = 0; fameIdx < 7; fameIdx++)
				{
					sint32 victimFame = CFameInterface::getInstance().getFameIndexed(victimChar->getId(), fameIdx);
					sint32 winnerFame = CFameInterface::getInstance().getFameIndexed(winnerChar->getId(), fameIdx);

					if ( (victimFame >= 25*6000 && winnerFame <= -25*6000) || 
						 (winnerFame >= 25*6000 && victimFame <= -25*6000) )
						fameFactor++;

					if ( (victimFame >= 25*6000 && winnerFame >= 25*6000) || 
						 (victimFame <= -25*6000 && winnerFame <= -25*6000) )
						fameFactor--;						
				}
				clamp(fameFactor, 0, 3);
				nlinfo("points = %d * %d", fpPerPlayer, fameFactor);

				// player gains faction points
				changePlayerPvpPoints(winnerChar, sint32(fpPerPlayer) * fameFactor);
				winnerChar->sendFactionPointGainKillMessage(winnerFaction, fpPerPlayer * fameFactor, victimChar->getId());
			}

			// player gains HoF points
			// Episode 2 is finished, they are no reason for win HoF point again, we need to state about if we made future episodes (3, 4..) 
			// and a way for known if an episode occurs (and specs for known if other episode pemrti to win HoF point...)
			//changePlayerHoFPoints(winnerChar, sint32(hofpPerPlayer));

			/*
			// PvP faction winner HOF reward
			CPVPManager2::getInstance()->characterKillerInPvPFaction( winnerChar, winnerFaction, (sint32)fpPerPlayer );
			if( finalBlower == winnerChar )
			{
				CPVPManager2::getInstance()->finalBlowerKillerInPvPFaction( winnerChar, winnerFaction, victimChar );
			}
			*/
			rewardedKillers.push_back(winnerChar->getId());
			nbRewardedMembers++;
		}

		// if at least one member in the team has gain points for this kill
		// add point loss for the victim
		if (nbRewardedMembers != 0)
		{
			if (victimLosesFactionPoints)
				fpLoss += fpPerPlayer;
			hofpLoss += hofpPerPlayer;
		}

		// debug info
		if (VerboseFactionPoint.get())
		{
			nlinfo("----------------------------------------------------------------------------");
		}
	}

	// add the newly rewarded killers
	_RewardedKills.addRewardedKill(victimChar->getId(), rewardedKillers);

	if (victimLosesFactionPoints)
		fpLoss = uint32(double(fpLoss) * PVPFactionPointLossFactor.get() + 0.5);

	hofpLoss = uint32(double(hofpLoss) * PVPHoFPointLossFactor.get() + 0.5);

	// keep the point loss to apply if the victim respawn
	/*CPointLoss pointLoss;
	pointLoss.PlayerLosesFactionPoints	= victimLosesFactionPoints;
	pointLoss.PlayerFaction				= victimFaction;
	pointLoss.FactionPointLoss			= fpLoss;
	pointLoss.HoFPointLoss				= hofpLoss;

	_PointLossByPlayer[victimChar->getId()] = pointLoss;

	// PvP faction looser lost
	//CPVPManager2::getInstance()->characterKilledInPvPFaction( victimChar, victimFaction, -(sint32) fpLoss );
*/
	// delete the score table of the victim
	removeDamageScoreTable(victimChar->getEntityRowId());
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::spireDestroyed(CCreature * spire, const CCharacter * finalBlower)
{
	nlassert( spire->isSpire() );
	TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(spire->getEntityRowId());
	if (it == _DamageScoreTablesByPlayer.end())
		return;

	CDamageScoreTable & scoreTable = (*it).second;

	vector<CDamageScoreTable::CWinner> winners;
	if (!scoreTable.getWinners(winners))
	{
		// debug info
		if (VerboseFactionPoint.get())
		{
			nlinfo("Faction Spire kill: spire is destroyed by creature %s '%s'",
				spire->getId().toString().c_str(),
				spire->getName().toUtf8().c_str()
				);
			nlinfo("Damage score table:\n");
			scoreTable.dumpDamageScoreTable(*InfoLog);
			nlinfo("----------------------------------------------------------------------------");
		}
		return;
	}

	BOMB_IF(winners.empty(), "winners should not be empty!", return);

	// debug info
	if (VerboseFactionPoint.get())
	{
		nlinfo("Faction Spire kill: killed player %s '%s'",
			spire->getId().toString().c_str(),
			spire->getName().toUtf8().c_str()
			);
		nlinfo("Damage score table:\n");
		scoreTable.dumpDamageScoreTable(*InfoLog);
		nlinfo("----------------------------------------------------------------------------");
	}

	// compute and distribute faction points and HoF points to each winner
	uint32 fpLoss = 0;
	uint32 hofpLoss = 0;
	for (uint i = 0; i < winners.size(); i++)
	{
		uint32 fpForWinner = computeFactionPointsForWinner(winners[i], winners[i].MaxSkillValue);
		uint32 hofpForWinner = computeHoFPointsForWinner(winners[i], winners[i].MaxSkillValue);

		uint32 fpPerPlayer = fpForWinner;
		uint32 hofpPerPlayer = hofpForWinner;
		if (winners[i].Players.size() > 1)
		{
			double teamDivisor = 1.0 + (double(winners[i].Players.size() - 1) * PVPTeamMemberDivisorValue.get());
			fpPerPlayer = uint32(double(fpForWinner) / teamDivisor + 0.5);
			hofpPerPlayer = uint32(double(hofpForWinner) / teamDivisor + 0.5);
		}

		// debug info
		if (VerboseFactionPoint.get())
		{
			nlinfo("Winner %u (%u players): damage percent = %.1f%%, faction points = %u (%u per player), HoF points = %u (%u per player)",
				i,
				winners[i].Players.size(),
				winners[i].TotalDamageRatio * 100.0,
				fpForWinner,
				fpPerPlayer,
				hofpForWinner,
				hofpPerPlayer
				);
		}

		uint nbRewardedMembers = 0; // nb of rewarded members in the team
		const vector<TDataSetRow> & players = winners[i].Players;
		for (uint k = 0; k < players.size(); k++)
		{
			CCharacter * winnerChar = PlayerManager.getChar(players[k]);
			BOMB_IF(winnerChar == NULL, "invalid winner!", continue);

			PVP_CLAN::TPVPClan winnerFaction;
			bool winnerGainFactionPoints;
			BOMB_IF(!playerInFactionPvP(winnerChar, &winnerFaction, &winnerGainFactionPoints), "winner not in faction PvP!", continue);

			if (winnerGainFactionPoints)
			{
				// player gains faction points
				changePlayerFactionPoints(winnerChar, winnerFaction, sint32(fpPerPlayer));
				//winnerChar->sendFactionPointGainKillMessage(winnerFaction, fpPerPlayer, victimChar->getId());
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
				params[0].Int = sint32(fpPerPlayer);
				PHRASE_UTILITIES::sendDynamicSystemMessage(winnerChar->getEntityRowId(),"PVP_SPIRE_FACTION_POINT",params);
			}

			CPVPFactionRewardManager::getInstance().updateFactionPointPool(winnerFaction, sint32(fpPerPlayer));

			// player gains HoF points
			//changePlayerHoFPoints(winnerChar, sint32(hofpPerPlayer));

			// PvP faction winner HOF reward
			CPVPManager2::getInstance()->characterKillerInPvPFaction( winnerChar, winnerFaction, (sint32)fpPerPlayer );
		}

		// debug info
		if (VerboseFactionPoint.get())
		{
			nlinfo("----------------------------------------------------------------------------");
		}
	}

	// delete the score table of the victim
	removeDamageScoreTable(spire->getEntityRowId());
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::playerResurrected(const CCharacter * victimChar)
{
	H_AUTO(CDamageScoreManager_playerResurrected);

	nlassert(victimChar != NULL);

	// when a player is resurrected he does not lose any point
	TPointLossByPlayer::iterator it = _PointLossByPlayer.find(victimChar->getId());
	if (it != _PointLossByPlayer.end())
	{
		_PointLossByPlayer.erase(it);
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::playerRespawn(CCharacter * victimChar)
{
	H_AUTO(CDamageScoreManager_playerRespawn);

	nlassert(victimChar != NULL);

	TPointLossByPlayer::iterator it = _PointLossByPlayer.find(victimChar->getId());
	if (it == _PointLossByPlayer.end())
		return;

	const CPointLoss & pointLoss = (*it).second;

	/*
	if (pointLoss.PlayerLosesFactionPoints)
	{
		// victim loses faction points
		uint32 fpLoss = uint32(abs( changePlayerFactionPoints(victimChar, pointLoss.PlayerFaction, -sint32(pointLoss.FactionPointLoss)) ));
		victimChar->sendFactionPointLoseMessage(pointLoss.PlayerFaction, fpLoss);
	}

	// victim loses HoF points
	changePlayerHoFPoints(victimChar, -sint32(pointLoss.HoFPointLoss));
	*/

	// remove the applied entry
	_PointLossByPlayer.erase(it);
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::removePlayer(const CCharacter * playerChar)
{
	H_AUTO(CDamageScoreManager_removePlayer);

	nlassert(playerChar != NULL);

	// damage points are transferred to a fake creature
	clearDamages(playerChar, true);

	// delete the score table of the player if any
	removeDamageScoreTable(playerChar->getEntityRowId());
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::removeCreature(const CCreature * creature)
{
	H_AUTO(CDamageScoreManager_removeCreature);

	nlassert(creature != NULL);

	// damage points are lost (not transferred)
	clearDamages(creature, false);
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::dumpPlayerDamageScoreTable(const CCharacter * playerChar, NLMISC::CLog & log) const
{
	nlassert(playerChar != NULL);

	TDamageScoreTablesByPlayer::const_iterator it = _DamageScoreTablesByPlayer.find(playerChar->getEntityRowId());
	if (it == _DamageScoreTablesByPlayer.end())
		return;

	const CDamageScoreTable & scoreTable = (*it).second;
	scoreTable.dumpDamageScoreTable(log);
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::addDamage(const CEntityBase * actor, const CEntityBase * target, uint32 damage)
{
	H_AUTO(CDamageScoreManager_addDamage);

	nlassert(actor != NULL);
	nlassert(target != NULL);
	nlassert(damage > 0);

	// only keep damage if the target is a player in 'faction point PvP'
	NLMISC::CSString sheetIdName = target->getType().toString();

	if (target->getId().getType() == RYZOMID::player)
	{
		const CCharacter * targetChar = dynamic_cast<const CCharacter *>(target);
		BOMB_IF(targetChar == NULL, "not a character!", return);
		if (!playerInFactionPvP(targetChar))
			return;
	}
	else if( target->isSpire() == false)
		return;

	// get the damage score table associated to the target (create it if necessary)
	TDamageScoreTablesByPlayer::iterator itTable = _DamageScoreTablesByPlayer.find(target->getEntityRowId());
	if (itTable == _DamageScoreTablesByPlayer.end())
	{
		pair<TDamageScoreTablesByPlayer::iterator, bool> ret = _DamageScoreTablesByPlayer.insert( make_pair(target->getEntityRowId(), CDamageScoreTable(target->getEntityRowId())) );
		BOMB_IF(!ret.second, "failed to insert damage score table!", return);
		itTable = ret.first;
	}

	CDamageScoreTable & scoreTable = (*itTable).second;

	// add damage to the score associated to the actor (team/player/creature)
	// and add the target in the list of players wounded by the actor
	if (actor->getId().getType() == RYZOMID::player)
	{
		const CCharacter * actorChar = dynamic_cast<const CCharacter *>(actor);
		BOMB_IF(actorChar == NULL, "not a character!", return);

		if (actorChar->getTeamId() != CTEAM::InvalidTeamId)
			scoreTable.addTeamDamage(actorChar->getTeamId(), damage);
		else
			scoreTable.addPlayerDamage(actorChar->getEntityRowId(), damage);
	}
	else
	{
		scoreTable.addCreatureDamage(actor->getEntityRowId(), damage);
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::clearDamages(const CCharacter * playerChar, bool transferDamages)
{
	H_AUTO(CDamageScoreManager_clearDamages1);

	nlassert(playerChar != NULL);

	if (playerChar->getTeamId() != CTEAM::InvalidTeamId)
	{
		// player is in a team
		vector<TDataSetRow> targets;
		_WoundedPlayers.getPlayersWoundedByTeam(playerChar->getTeamId(), targets);
		for (uint i = 0; i < targets.size(); i++)
		{
			TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(targets[i]);
			BOMB_IF(it == _DamageScoreTablesByPlayer.end(), "found no table!", continue);

			CDamageScoreTable & scoreTable = (*it).second;

			// player is not a beneficiary anymore
			scoreTable.removeTeamBeneficiary(playerChar->getTeamId(), playerChar->getEntityRowId());
		}
	}
	else // single player (without team)
	{
		vector<TDataSetRow> targets;
		_WoundedPlayers.getPlayersWoundedByPlayer(playerChar->getEntityRowId(), targets);
		for (uint i = 0; i < targets.size(); i++)
		{
			TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(targets[i]);
			BOMB_IF(it == _DamageScoreTablesByPlayer.end(), "found no table!", continue);

			CDamageScoreTable & scoreTable = (*it).second;

			// remove the player score
			scoreTable.removePlayer(playerChar->getEntityRowId(), transferDamages);

			// delete the score table if it is empty
			if (scoreTable.isEmpty())
				_DamageScoreTablesByPlayer.erase(it);
		}
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::clearDamages(const CCreature * creature, bool transferDamages)
{
	H_AUTO(CDamageScoreManager_clearDamages2);

	nlassert(creature != NULL);

	vector<TDataSetRow> targets;
	_WoundedPlayers.getPlayersWoundedByCreature(creature->getEntityRowId(), targets);
	for (uint i = 0; i < targets.size(); i++)
	{
		TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(targets[i]);
		BOMB_IF(it == _DamageScoreTablesByPlayer.end(), "found no table!", continue);

		CDamageScoreTable & scoreTable = (*it).second;

		// remove the creature score
		scoreTable.removeCreature(creature->getEntityRowId(), transferDamages);

		// delete the score table if it is empty
		if (scoreTable.isEmpty())
			_DamageScoreTablesByPlayer.erase(it);
	}
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::changeAllDamageEquitably(TDataSetRow playerRowId, sint32 damageDelta)
{
	H_AUTO(CDamageScoreManager_changeAllDamageEquitably);

	TDamageScoreTablesByPlayer::iterator itTable = _DamageScoreTablesByPlayer.find(playerRowId);
	if (itTable == _DamageScoreTablesByPlayer.end())
		return;

	CDamageScoreTable & scoreTable = (*itTable).second;

	scoreTable.changeAllDamageEquitably(damageDelta);

	// delete the score table if it is empty
	if (scoreTable.isEmpty())
		_DamageScoreTablesByPlayer.erase(itTable);
}

//-----------------------------------------------------------------------------
void CDamageScoreManager::removeDamageScoreTable(TDataSetRow playerRowId)
{
	H_AUTO(CDamageScoreManager_removeDamageScoreTable);

	TDamageScoreTablesByPlayer::iterator it = _DamageScoreTablesByPlayer.find(playerRowId);
	if (it == _DamageScoreTablesByPlayer.end())
		return;

	_DamageScoreTablesByPlayer.erase(it);
}

//-----------------------------------------------------------------------------
bool CDamageScoreManager::isOffensiveAction(const TReportAction & action) const
{
	switch (action.ActionNature)
	{
	case ACTNATURE::FIGHT:
		// discard fight actions without any damage
		if (action.Hp == 0)
			return false;

	case ACTNATURE::OFFENSIVE_MAGIC:
	case ACTNATURE::NEUTRAL:
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CDamageScoreManager::isCurativeAction(const TReportAction & action) const
{
	switch (action.ActionNature)
	{
	case ACTNATURE::CURATIVE_MAGIC:
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool CDamageScoreManager::playerInFactionPvP(const CCharacter * playerChar, PVP_CLAN::TPVPClan * faction, bool * withFactionPoints) const
{
	nlassert(playerChar != NULL);

	if (playerChar->getPVPInterface().isValid())
	{
		CPVPVersusZone * zone = dynamic_cast<CPVPVersusZone *>(const_cast<CPVPInterface &>(playerChar->getPVPInterface()).getPVPSession());
		if (zone != NULL)
		{
			PVP_CLAN::TPVPClan factionInZone = zone->getCharacterClan(playerChar->getId());
			if (factionInZone == PVP_CLAN::Neutral)
				return false;
			
			BOMB_IF(factionInZone < PVP_CLAN::BeginClans || factionInZone > PVP_CLAN::EndClans, "invalid faction!", return false);
			
			if (faction)
				*faction = factionInZone;
			
			if (withFactionPoints)
				*withFactionPoints = zone->giveFactionPoints();
			
			return true;
		}
	}
	else if( playerChar->getPVPFlag(false) )
	{ 
		/*pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance = playerChar->getAllegiance();
		if( (allegiance.first != PVP_CLAN::Neutral) && (allegiance.first != PVP_CLAN::None) )
		{
			if (faction)
				*faction = allegiance.first;
			if (withFactionPoints)
				*withFactionPoints = true;
			return true;
		}
		if ( allegiance.second != PVP_CLAN::Neutral)
		{
			if (faction)
				*faction = allegiance.second;
			if (withFactionPoints)
				*withFactionPoints = true;
			return true;
		}*/
		return true;
	}
	return false;
}

bool CDamageScoreManager::canPlayerWinPoints(const CCharacter * winnerChar,  CCharacter * victimeChar) const
{
	// Don't win points if in duel
	if ( winnerChar->getDuelOpponent() && (winnerChar->getDuelOpponent()->getId() == victimeChar->getId()) )
		return false;

	if (victimeChar->getPVPRecentActionFlag())
		nlinfo("PVP recent flag");
	// Only win points if victime is Flag
	if( winnerChar->getPVPFlag(false))
	{
		 if ( true/*victimeChar->getPVPRecentActionFlag()*/ )
		 {
			nlinfo("Teams : %d, %d", winnerChar->getTeamId(), victimeChar->getTeamId());
			// Don't win points if in same Team
			if ( winnerChar->getTeamId() != CTEAM::InvalidTeamId && victimeChar->getTeamId() != CTEAM::InvalidTeamId && (winnerChar->getTeamId() == victimeChar->getTeamId()) )
				return false;

			// Don't win points if in same Guild
			nlinfo("Guild : %d, %d", winnerChar->getGuildId(), victimeChar->getGuildId());
			if ( winnerChar->getGuildId() != 0 && victimeChar->getGuildId() != 0 && (winnerChar->getGuildId() == victimeChar->getGuildId()) )
				return false;

			// Don't win points if in same League
			if ( winnerChar->getLeagueId() != DYN_CHAT_INVALID_CHAN && victimeChar->getLeagueId() != DYN_CHAT_INVALID_CHAN && (winnerChar->getLeagueId() == victimeChar->getLeagueId()) )
				return false;

			return true;
		 }
	}

	return false;
}

//-----------------------------------------------------------------------------
uint32 CDamageScoreManager::computeFactionPointsForWinner(const CDamageScoreTable::CWinner & winner, uint32 victimSkillValue)
{
	double factionPoints;
	
	// compute base number of faction points for the victim
	factionPoints = PVPFactionPointBase.get() * pow(2.0, double(victimSkillValue) / 50.0);

	// get the part of this winner (proportional to the made damages)
	factionPoints *= winner.TotalDamageRatio;

	// apply the delta level between winner and victim
	sint32 deltaLevel = victimSkillValue - winner.MaxSkillValue;
	clamp(deltaLevel, MinPVPDeltaLevel.get(), MaxPVPDeltaLevel.get());

	if (deltaLevel < 0)
	{
		double factor = (1.0 / double(-MinPVPDeltaLevel.get())) * double(deltaLevel) + 1.0;
		factionPoints *= factor;
	}
	else if (deltaLevel > 0)
	{
		double factor = (1.0 / double(MaxPVPDeltaLevel.get())) * double(deltaLevel) + 1.0;
		factionPoints *= factor;
	}

	// ensure faction points will not be negative
	if (factionPoints < 0.0)
		factionPoints = 0.0;

	return uint32(factionPoints + 0.5);
}

//-----------------------------------------------------------------------------
uint32 CDamageScoreManager::computeHoFPointsForWinner(const CDamageScoreTable::CWinner & winner, uint32 victimSkillValue)
{
	double hofPoints;
	
	// base number of HoF points
	hofPoints = PVPHoFPointBase.get();

	// get the part of this winner (proportional to the made damages)
	hofPoints *= winner.TotalDamageRatio;

	// apply the delta level between winner and victim
	sint32 deltaLevel = victimSkillValue - winner.MaxSkillValue;
	clamp(deltaLevel, MinPVPDeltaLevel.get(), MaxPVPDeltaLevel.get());

	if (deltaLevel < 0)
	{
		double factor = (1.0 / double(-MinPVPDeltaLevel.get())) * double(deltaLevel) + 1.0;
		hofPoints *= factor;
	}
	else if (deltaLevel > 0)
	{
		double factor = (1.0 / double(MaxPVPDeltaLevel.get())) * double(deltaLevel) + 1.0;
		hofPoints *= factor;
	}

	// ensure HoF points will not be negative
	if (hofPoints < 0.0)
		hofPoints = 0.0;

	return uint32(hofPoints + 0.5);
}

//-----------------------------------------------------------------------------
sint32 CDamageScoreManager::changePlayerFactionPoints(CCharacter * playerChar, PVP_CLAN::TPVPClan faction, sint32 fpDelta)
{
	nlassert(playerChar != NULL);
	BOMB_IF(faction < PVP_CLAN::BeginClans || faction > PVP_CLAN::EndClans, "invalid PvP faction!", return 0);

	if (fpDelta == 0)
		return 0;

	uint32 factionPoints = playerChar->getFactionPoint(faction);

	// player cannot have negative faction points
	fpDelta = max(fpDelta, -sint32(factionPoints));

	factionPoints += fpDelta;

	// set the new faction points
	playerChar->setFactionPoint(faction, factionPoints, true);

	return fpDelta;
}

//-----------------------------------------------------------------------------
sint32 CDamageScoreManager::changePlayerPvpPoints(CCharacter * playerChar, sint32 fpDelta)
{
	nlassert(playerChar != NULL);

	if (fpDelta == 0)
		return 0;

	uint32 points = playerChar->getPvpPoint();

	// player cannot have negative pvp points
	fpDelta = max(fpDelta, -sint32(points));

	points += fpDelta;

	// set the new pvp points
	playerChar->setPvpPoint(points);

	return fpDelta;
}


//-----------------------------------------------------------------------------
void CDamageScoreManager::changePlayerHoFPoints(CCharacter * playerChar, sint32 hofpDelta)
{
	nlassert(playerChar != NULL);
	if (hofpDelta == 0)
		return;

	string sdbPvPPath;
	if (playerChar->getSDBPvPPath(sdbPvPPath))
	{
		bool res = CStatDB::getInstance()->tablePlayerAdd(sdbPvPPath, playerChar->getId(), hofpDelta);
		if (playerChar->getGuildId() != 0)
		{
			res &= CStatDB::getInstance()->tableGuildAdd(sdbPvPPath, playerChar->getGuildId(), hofpDelta);
		}

		if (res)
		{
			// debug info
			if (VerboseFactionPoint.get())
			{
				nlinfo("player %s '%s' gets %d HoF points at path '%s'",
					playerChar->getId().toString().c_str(),
					playerChar->getName().toUtf8().c_str(),
					hofpDelta,
					sdbPvPPath.c_str());
			}
		}
		else
		{
			nlwarning("SDB: invalid path : '%s'", sdbPvPPath.c_str());
		}
	}
}


} // namespace PROGRESSIONPVP

