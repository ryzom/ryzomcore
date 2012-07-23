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



#ifndef RY_TEAM_H
#define RY_TEAM_H

#include "game_share/chat_group.h"
#include "game_share/misc_const.h"
#include "game_share/string_manager_sender.h"
#include "game_share/scores.h"
#include "game_share/dyn_chat.h"
#include "mission_manager/mission_event.h"
#include "mission_manager/ai_alias_translator.h"
#include "team_manager/reward_sharing.h"
#include "mission_manager/mission_template.h"

class CCharacter;
class CMissionTeam;
/**
 * A team of players, that can accept bots under special circunstances ( mission, convoy,...)
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CTeam
{
public:
	/// constructor
	inline CTeam()
	:_TeamId(CTEAM::InvalidTeamId),_RewardSharing(NULL)
	{
		_ValidityFlags.Fake = false;
		_ValidityFlags.Valid = false;
	}

	/**
	 * initilization
	 * \param leader: pointer on the team leader	
	 * \param teamId: id of the team
	 */
	void init( CCharacter*  leader, uint16 teamId );

	/// release the team by seting it as an uninstanciated team
	void release();

	/// add all the team member to the specified chat group
	void addAllMembersToChatGroup( TGroupId idGroup );

	/**
	 * add a new character to this team
	 * \param newCharacter: the character to add
	 */
	void addCharacter(  CCharacter *newCharacter );

	/**
	 * remove the specified character from the team
	 * \return true if the char has been removed, false otherwise
	 */
	void removeCharacter( CCharacter * newChar );

	/**
	 * return the index of team leader successor
	 * 
	 */
	uint8 getSuccessorIndex(void);

	/**
	 * set the League
	 * \param leagueName is the name of League
	 */
	void setLeague(const std::string &leagueName);
	/**
	 * update the League (set LeagueId to all teammates)
	 */
	void updateLeague();
	
	/**
	 * set the team leader
	 * \param id is the Entity ID of the member in the team
	 * \param bMessage whether to show the message
	 */
	void setLeader(NLMISC::CEntityId id, bool bMessage = true);
	
	/**
	 * set the team leader
	 * \param memberIdx is the index of the member in the team
	 * \param bMessage whether to show the message
	 */
	void setLeader(uint8 memberIdx, bool bMessage = true);

	/**
	 * set the successor of the team leader
	 * \param memberIdx is the index of the member in the team
	 * \param bMessage whether to show the message
	 */
	void setSuccessor( uint8 memberIdx, bool bMessage = true );

	/**
	 * get the leader of the team
	 * \return team leader Id
	 */
	inline const NLMISC::CEntityId & getLeader() const { return _LeaderId; }

	/**
	 * get the successor of the team
	 * \return successor Id
	 */
	inline const NLMISC::CEntityId & getSuccessor() const { return _SuccessorId; }

	///\return number of team members
	inline uint8 getTeamSize() const { return _NbMembers; }

	///\return true if the team is valid
	inline bool isValid() const { return _ValidityFlags.Valid; }

	///\return true if the team is fake
	inline bool isFake() const { return _ValidityFlags.Fake; }

	/// set the team as a "fake" one
	inline void setAsFake(){ _ValidityFlags.Fake = _ValidityFlags.Valid = true; }

	///\return the next free team id
	uint16 getNextFreeId() const {return _NextFreeTeamId;}

	///\set the next free team id
	inline void setNextFreeId(uint16 id) { _NextFreeTeamId = id;}

	///\return the team members
	inline const std::list<NLMISC::CEntityId> & getTeamMembers() const { return _TeamMembers; }

	///\return the team id
	inline uint16 getTeamId() const { return _TeamId; }

	///\return the League id
	inline TChanID getLeagueId() const { return _LeagueId; }

	///\return the League id
	inline void setLeagueId(TChanID id) { _LeagueId = id; }

	/// send a message to the team
	void sendDynamicMessageToMembers(const std::string &msgName, const TVectorParamCheck &params, const std::set<NLMISC::CEntityId> &excluded) const;
	inline void sendDynamicMessageToMembers(const std::string &msgName, const TVectorParamCheck &params) const
	{
		static const std::set<NLMISC::CEntityId> excluded;
		sendDynamicMessageToMembers(msgName, params, excluded);
	}

	/// update given character score
//	void updateCharacterScore(const CCharacter *player, const std::string &scoreStr, uint8 value) const;
	void updateCharacterScore(const CCharacter *player, SCORES::TScores score, uint8 value) const;
	
	
	/// add a mission to team
	void addMission( CMissionTeam * mission );
	
	/// remove a mission to team
	inline void removeMission( CMissionTeam * mission, TMissionResult result)
	{
		for (uint i = 0; i < _Missions.size(); i++)
		{
			if ( _Missions[i] == mission )
			{
				removeMission(i, result);
			}
		}
	}
	void removeMission( uint idx, TMissionResult result);

	///\return the mission
	inline std::vector<CMissionTeam*> & getMissions()
	{
		return _Missions;
	}

	/// process mission events for the team mission.
	/// If a valid mission alias is given, only the specified mission is tested
	/// return true if the event is processed
	bool processTeamMissionEvent(std::list< CMissionEvent * > & eventList, TAIAlias missionAlias );

	/// process an event for a specific step
	bool processTeamMissionStepEvent(std::list< CMissionEvent* > & eventList, TAIAlias missionAlias, uint32 stepIndex);

	/// triggers a reward sharing session
	void rewardSharing(CRewardSharing * reward);

	inline CRewardSharing * getReward()
	{
		return _RewardSharing;
	}

	/// clear the team reward
	inline void clearReward()
	{
		delete _RewardSharing;
		_RewardSharing = NULL;
	}

	/// update team members positions
	void updateMembersPositions(bool forceUpdate = false);

	CMissionTeam* getMissionByAlias( TAIAlias missionAlias );
	
	void updateMembersDb();
/*
	bool processMissionStepEvent(std::list< const CMissionEvent* > & eventList, uint missionIndex, uint32 stepIndex );
	bool processMissionEvent( std::list< const CMissionEvent* > & eventList, uint missionIndex, uint32 stepIndex );
*/

private:
	/**
	 * send messages to indicate the player charId has left the team, also specify if he was the leader or not
	 */
	//void sendLeaveMessages(const  NLMISC::CEntityId &charId, bool leader );

	///\clear a player database entries that are related to teams
	void clearPlayerTeamDB( const NLMISC::CEntityId &charId );

	/// find character position in team
	sint16 findCharacterPosition( const NLMISC::CEntityId &charId ) const;

	///flags describing the validity of the team
	struct	SValidity
	{
		/// true if the team is fake
		bool Fake	:1;
		/// true uif the team is valid
		bool Valid  :1;
	}_ValidityFlags;

	
	///\the next free team id
	uint16								_NextFreeTeamId;

	///\id of the team
	uint16								_TeamId;
	
	///\id of the League (it's id of channel)
	TChanID							_LeagueId;

	/// Team Members. The index of an entity in the container is its position
	std::list<NLMISC::CEntityId>		_TeamMembers;

	///number of members
	uint8								_NbMembers;

	/// id of the team leader
	NLMISC::CEntityId					_LeaderId;

	/// successor id
	NLMISC::CEntityId					_SuccessorId;

	///the missions took by the group
	std::vector<CMissionTeam*>			_Missions;

	/// pointer on the current reward sharing structure, NULL if no reward sharing was initialized
	CRewardSharing*						_RewardSharing;
};

#endif // RY_TEAM_H

/* End of team.h */
