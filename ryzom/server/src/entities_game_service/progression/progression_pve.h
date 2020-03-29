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



#ifndef RYZOM_PROGRESSION_PVE_H
#define RYZOM_PROGRESSION_PVE_H

#include "progression_common.h"
#include "server_share/msg_ai_service.h"
//
#include "egs_sheets/egs_static_success_table.h"
#include "egs_mirror.h"

// forward declaration
class CCharacter;
class CEntityBase;


//---------------------------------------------------
// Implementation of CAILostAggroMsgImp
//
//---------------------------------------------------
class CAILostAggroMsgImp : public CAILostAggroMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


namespace PROGRESSIONPVE
{


/**
 * CSkillProgress
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 * \contained skill and number of actions made using this skill
 */
class CSkillProgress
{
	NL_INSTANCE_COUNTER_DECL(CSkillProgress);
public:

	struct TSkillProgress
	{
		SKILLS::ESkills Skill;
		uint16			NbActions;
	};

	/// ctor
	CSkillProgress() {}

	/// dtor
	virtual ~CSkillProgress() { _SkillsProgress.clear(); }

	/// inc action counter
	void incNbAction( SKILLS::ESkills skill );
	
	/// apply xp to skill
	bool applyXp( CCharacter * c, float xpGainPerOpponent );
	
	/// return reference on __SkillsProgress
	std::vector< TSkillProgress >& getSkillsProgress() { return _SkillsProgress; }

private:
	std::vector< TSkillProgress > _SkillsProgress;
};

/**
 * CCharacterActions
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 * \contains actions performed per character on creature or other players for skills progress management
 */
class CCharacterActions
{
	NL_INSTANCE_COUNTER_DECL(CCharacterActions);
public:

	typedef std::map< TDataSetRow, CSkillProgress * > TSkillProgressPerOpponentContainer;
	
	/// ctor
	CCharacterActions() {}

	/// dtor+
	virtual ~CCharacterActions();

	// add player action
	void addAction( const TDataSetRow& target, SKILLS::ESkills skill, bool incActionCounter);

	// a creature dead and have offensive character for it, dispatch xp gain
	bool dispatchXpGain( TDataSetRow actor, TDataSetRow creature, float equivalentXpMembers, float xpFactor, const std::list<NLMISC::CEntityId> &allowedChar );
	
	// forget xp gain of one creature
	bool forgetXpGain( TDataSetRow creature );
	
private:
	// TOffensiveCharacter struct per creature
	TSkillProgressPerOpponentContainer _SkillProgressPerOpponent;
};


struct CTeamMember
{
	CTeamMember() { GainXp = false; }
	
	NLMISC::CEntityId	Id;
	bool				GainXp;
};

/**
 * CTeamDamage
 * \author David Fleury
 * \author Nevrax France
 * \date 2004
 * \manage damage inflicted by a team on a creature (a player alone is considered as a team of one)
 */
class CTeamDamage
{
public:
	inline CTeamDamage() : TeamId(CTEAM::InvalidTeamId), MaxSkillValue(0), TotalDamage(0.0f) {}
	explicit inline CTeamDamage(const NLMISC::CEntityId &playerId) : PlayerId(playerId), TeamId(CTEAM::InvalidTeamId), MaxSkillValue(0), TotalDamage(0.0f) {}
	explicit inline CTeamDamage(uint16 teamId) : TeamId(teamId), MaxSkillValue(0), TotalDamage(0.0f) {}

	/// single player Id if not a team
	NLMISC::CEntityId	PlayerId;
	/// team Id if not a single player
	uint16				TeamId;
	/// total damage
	float				TotalDamage;

	/// team members
	std::vector<CTeamMember> TeamMembers;

	/// min delta level between players and creature
	uint16				MaxSkillValue;

	/// enable xp for member
	void enableXP(const NLMISC::CEntityId &playerId, uint16 skillValue)
	{
		if (MaxSkillValue < skillValue)
			MaxSkillValue = skillValue;

		const uint size = (uint)TeamMembers.size();
		for (uint i = 0 ; i < size ; ++i)
		{
			if (TeamMembers[i].Id == playerId)
			{
				TeamMembers[i].GainXp = true;
				return;
			}
		}

		TeamMembers.resize(TeamMembers.size()+1);
		TeamMembers[TeamMembers.size()-1].Id = playerId;
		TeamMembers[TeamMembers.size()-1].GainXp = true;
	}

	/// remove a team member
	void removeMember(const NLMISC::CEntityId &playerId)
	{
		const uint size = (uint)TeamMembers.size();
		for (uint i = 0 ; i < size ; ++i)
		{
			if (TeamMembers[i].Id == playerId)
			{
				TeamMembers[i] = TeamMembers.back();
				TeamMembers.pop_back();
				return;
			}
		}
	}
};

/**
 * CCreatureInflictedDamage
 * \author David Fleury
 * \author Nevrax France
 * \date 2004
 * \manage damage inflicted by a creature on an another creature (for loot rights and progression management)
 */
class CCreatureInflictedDamage
{
public:
	inline CCreatureInflictedDamage() : TotalDamage(0.0f) {}
	explicit inline CCreatureInflictedDamage(const NLMISC::CEntityId &creatureId) : CreatureId(creatureId), TotalDamage(0.0f) {}

	/// creature id
	NLMISC::CEntityId	CreatureId;
	
	/// damage inflicted
	float				TotalDamage;
};

/**
 * CCreatureTakenDamage
 * \author David Fleury
 * \author Nevrax France
 * \date 2004
 * \manage damage inflicted to a creature
 */
struct CCreatureTakenDamage
{
	/// damage inflicted by players
	std::vector<CTeamDamage> PlayerInflictedDamage;
	
	/// damage inflicted by npcs (guards...) or other creatures
	std::vector<CCreatureInflictedDamage> CreatureInflictedDamage;

	/// total damage inflicted by npcs (guards...) or other creatures
	float					 TotalCreatureInflictedDamage;

	/// constructor
	inline CCreatureTakenDamage() : TotalCreatureInflictedDamage(0) {}

	sint16 getIndexForTeam(uint16 teamId)
	{
		const uint size = (uint)PlayerInflictedDamage.size();
		for ( uint i = 0 ; i < size ; ++i)
		{
			if ( PlayerInflictedDamage[i].TeamId == teamId )
			{
				return (sint16)i;
			}
		}
		return sint16(-1);
	}

	sint16 getIndexForPlayer(const NLMISC::CEntityId &id)
	{
		const uint size = (uint)PlayerInflictedDamage.size();
		for ( uint i = 0 ; i < size ; ++i)
		{
			if ( PlayerInflictedDamage[i].PlayerId == id )
			{
				return (sint16)i;
			}
		}
		return sint16(-1);
	}

	sint16 getIndexForCreature(const NLMISC::CEntityId &id)
	{
		const uint size = (uint)CreatureInflictedDamage.size();
		for ( uint i = 0 ; i < size ; ++i)
		{
			if ( CreatureInflictedDamage[i].CreatureId == id )
			{
				return (sint16)i;
			}
		}
		return sint16(-1);
	}

	/// return index of the team which has inflicted the most damage, returns -1 if most damage have been made by other creatures
	sint16 getMaxInflictedDamageTeamIndex()
	{
		float maxDmg = 0;//TotalCreatureInflictedDamage;
		sint16 index = -1;
		const uint size = (uint)PlayerInflictedDamage.size();
		for ( uint i = 0 ; i < size ; ++i)
		{
			if ( PlayerInflictedDamage[i].TotalDamage > maxDmg )
			{
				maxDmg = PlayerInflictedDamage[i].TotalDamage;
				index = (sint16)i;
			}
		}
		
		return index;
	}
	
	/// get all the players that have contributed to creature death
	std::set<NLMISC::CEntityId> getAllPlayers()
	{
		std::set<NLMISC::CEntityId> players;
		std::vector<CTeamDamage>::const_iterator itPlayer, itPlayerEnd=PlayerInflictedDamage.end();
		for (itPlayer=PlayerInflictedDamage.begin(); itPlayer!=itPlayerEnd; ++itPlayer)
		{
			if (itPlayer->PlayerId!=NLMISC::CEntityId::Unknown)
				players.insert(itPlayer->PlayerId);
			if (itPlayer->TeamId!=CTEAM::InvalidTeamId)
			{
				std::vector<CTeamMember> const& members = itPlayer->TeamMembers;
				std::vector<CTeamMember>::const_iterator itMember, itMemberEnd = members.end();
				for (itMember=members.begin(); itMember!=itMemberEnd; ++itMember)
					if (itMember->Id!=NLMISC::CEntityId::Unknown)
						players.insert(itMember->Id);
			}
		}
		return players;
	}
	
	/// return true if the max damages were inflicted by the fictitious creature
	bool isMaxCreatureInflictedDamageTransfered()
	{
		float maxDmg = 0;
		sint16 index = -1;
		NLMISC::CEntityId maxCreatureId = NLMISC::CEntityId::Unknown;
		float maxDamage = 0;
		std::vector<CCreatureInflictedDamage>::const_iterator it, itEnd=CreatureInflictedDamage.end();
		for (it=CreatureInflictedDamage.begin(); it!=itEnd; ++it)
		{
			if ( it->TotalDamage > maxDamage )
			{
				maxDamage = it->TotalDamage;
				maxCreatureId = it->CreatureId;
			}
		}
		return maxCreatureId==NLMISC::CEntityId::Unknown;
	}

	/// apply regen, remove an equal part of damage on each registered entity
	void applyRegenHP(sint32 regenHP)
	{
		if (regenHP == 0 || (PlayerInflictedDamage.size() + CreatureInflictedDamage.size() == 0) )
			return;
		
		const float damageLoss = (float)regenHP / float(PlayerInflictedDamage.size() + CreatureInflictedDamage.size());

		// remove damage for teams
		for ( uint i = 0 ; i < PlayerInflictedDamage.size() ; ++i)
		{
			if ( PlayerInflictedDamage[i].TotalDamage > damageLoss)
			{
				PlayerInflictedDamage[i].TotalDamage -= damageLoss;
			}
			else
			{
				PlayerInflictedDamage[i].TotalDamage = 0.0f;
//				PlayerInflictedDamage[i] = PlayerInflictedDamage.back();
//				PlayerInflictedDamage.pop_back();
			}
		}

		// remove damage for creatures
		for ( uint i = 0 ; i < CreatureInflictedDamage.size() ; ++i)
		{
			if ( CreatureInflictedDamage[i].TotalDamage > damageLoss)
			{
				CreatureInflictedDamage[i].TotalDamage -= damageLoss;
				TotalCreatureInflictedDamage -= damageLoss;
			}
			else //if (CreatureInflictedDamage[i].TotalDamage > 0.0f)
			{
				TotalCreatureInflictedDamage -= CreatureInflictedDamage[i].TotalDamage;
				//CreatureInflictedDamage[i] = CreatureInflictedDamage.back();
				//CreatureInflictedDamage.pop_back();
				CreatureInflictedDamage[i].TotalDamage = 0.0f;
			}
		}		
	}

	/// tranfer player damage on a fictious creature (when player enters water), return true if entry hve been erased
	bool transferDamageOnFictitiousCreature(const NLMISC::CEntityId &playerId, uint16 teamId = CTEAM::InvalidTeamId);

	/// attribute kills for mission system
	void attributeKillsForMission(TDataSetRow victimRowId);

private:
	/// attribute kill to an entity for mission system
	static void attributeKill( TDataSetRow killerRowId, TDataSetRow victimRowId);
	
	/// attribute kill to all team members for mission system
	static void attributeKill( uint16 teamId, TDataSetRow victimRowId);
};

/**
 * CCharacterProgressionPVE
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2003
 * \manage PvE progression of player characters
 */
class CCharacterProgressionPVE
{
	NL_INSTANCE_COUNTER_DECL(CCharacterProgressionPVE);
public:
	
	friend class CSkillProgress;

	//typedef std::map< TDataSetRow, COffensiveCharacter * > TOffensiveCharacterContainer;
	typedef std::map< TDataSetRow, CCharacterActions* > TCharacterActionsContainer;

	typedef std::map< TDataSetRow, CCreatureTakenDamage > TCreatureTakenDamageContainer;
	typedef std::map< uint16, std::vector<TDataSetRow> >	TTeamsAttackedCreature;
	typedef std::map< NLMISC::CEntityId, std::vector<TDataSetRow> >	TEntityAttackedCreature;
	
	// initialize 
	static CCharacterProgressionPVE* getInstance() 
	{
		if(_Instance == NULL)
		{
			_Instance = new CCharacterProgressionPVE();
		}
		return _Instance;
	}

	/// constructor
	CCharacterProgressionPVE() {}

	/// destructor
	virtual ~CCharacterProgressionPVE();

	// game system report an action
	void actionReport( TReportAction& reportAction, bool incActionCounter = true, bool scaleForNewbies = true );

	// get progression factor
	double getProgressionFactor( CEntityBase * actor, sint32 deltaLvl, SKILLS::ESkills skill, SUCCESS_TABLE_TYPE::TSuccessTableType tableType, bool scaleForNewbies = true );

	// forget xp gain (after creature lost agro on character)
	void forgetXpGain( TDataSetRow creature, TDataSetRow offensiveCharacter );

	// report creature death with character perform offensive action against
	void creatureDeath( TDataSetRow creature );
		
	// character disconnect
	void clearAllXpForPlayer( TDataSetRow character, uint16 teamId, bool removeDamageFromTeam );

	/// a player joins given team
	void playerJoinsTeam(const NLMISC::CEntityId &playerId, uint16 teamId);
	/// a player leaves given team
	void playerLeavesTeam(const NLMISC::CEntityId &playerId, uint16 teamId);
	/// disband given team
	void disbandTeam(uint16 teamId, const std::list<NLMISC::CEntityId> &teamMembers);
	/// clearPlayerDamage (enters water, disconnect...)
	void clearPlayerDamage(const NLMISC::CEntityId &playerId, uint16 teamId, bool removeDamageFromTeam);
	/// clear creature damage
	void clearCreatureInflictedDamage(TDataSetRow creature);
	/// player lost aggro, if not in teams, clears Xp, if in team check at least one team member has still aggro on creature
	void lostAggro(TDataSetRow creature, TDataSetRow player);
	/// add damage from given entityId on given entity
	void addDamage(const NLMISC::CEntityId &actorId, const NLMISC::CEntityId &targetId, uint32 damage);
	/// enable Xp Gain for a player, set a list with all creatures on which he now has possible Xp Gain
	void enableXpGainForPlayer(const NLMISC::CEntityId &playerId, uint16 skillValue, std::list<TDataSetRow> &enabledCreatures);
	/// remove xp from a creature (so clear all references on it)
	void removeXpCreature(TDataSetRow creature);
	/// remove a creature (so clear all references on it)
	void removeCreature(TDataSetRow creature);
	/// apply regen of a creature
	void applyRegenHP(TDataSetRow creature, sint32 regenHP);
	///  attribute player inflicted damage to a fictious creature (when player enters water)
	void transferDamageOnFictitiousCreature(TDataSetRow playerRowId, uint16 teamId);
	///  attribute player inflicted damage on a specific creature to a fictious creature (to avoid exploits)
	void transferDamageOnFictitiousCreature(TDataSetRow playerRowId, uint16 teamId, TDataSetRow creatureRowId);

private:
	// return xp gain depend on skill and delta level * factor * SkillProgressionFactor
	double getXpGain( CEntityBase * actor, sint32 deltaLvl, SKILLS::ESkills skill, float factor, SUCCESS_TABLE_TYPE::TSuccessTableType tableType, bool scaleForNewbies = true );
		
	// process report for a fight action (melee and range combat, all spells makes damage, all spells makes disease and curse)
	void offensiveActionReported( const TReportAction& reportAction, CEntityBase * actor, CEntityBase * target, bool incActionCounter );

	// process report for a curative magic action (all spells make benefic effects has heal, remove curse or disease, restore any energy)
	void curativeActionReported( const TReportAction& reportAction, CEntityBase * actor, CEntityBase * target, bool incActionCounter );

	// process report for a simple action (ie action with immediat xp gain directly depends on delta level reported)
	void simpleActionReported( const TReportAction& reportAction, CEntityBase * actor, bool scaleForNewbies = true );

	/// check at least a team member has aggro with given creature, return true if aggro exists
	bool checkAggroForTeam( uint16 teamId, TDataSetRow creatureRowId );

private:	
	// singleton instance
	static CCharacterProgressionPVE *	_Instance;

	// COffensiveCharacter instance per character actor
	TCharacterActionsContainer		_CharacterActions;

	/// creatures wounded by teams
	TTeamsAttackedCreature			_TeamsWoundedCreatures;	
	
	/// creatures wounded by single players
	TEntityAttackedCreature			_PlayersWoundedCreatures;

	/// creatures wounded by other creatures
	TEntityAttackedCreature			_CreaturesWoundedCreatures;

	/// damage inflicted on creatures
	TCreatureTakenDamageContainer	_CreatureTakenDamage;
};


} // namespace PROGRESSIONPVE


#endif // RYZOM_PROGRESSION_PVE_H

/* End of file progression_pve.h */
