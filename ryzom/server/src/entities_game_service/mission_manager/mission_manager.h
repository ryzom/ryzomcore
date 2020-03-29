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



#ifndef RY_MISSION_MANAGER_H
#define RY_MISSION_MANAGER_H

#include "nel/ligo/primitive.h"

#include "server_share/mission_messages.h"
#include "server_share/msg_ai_service.h"

#include "mission_manager/mission_template.h"



class CCreature;
class CGuild;

/// transport class received from AIS when it answers a dynamic mission request
/*class CDynMissionDescMsgImp : public CDynMissionDescMsg
{
	void callback (const std::string &name, uint8 id);
};
*/

/// Implementation of the CCAisActionMsg transport class
class CCAisActionMsgImp : public CCAisActionMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

/// Implementation (called when a Handled AI Group is spawned)
class CHandledAIGroupSpawnedMsgImp : public CHandledAIGroupSpawnedMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

/// Implementation (called when a Handled AI Group is despawned)
class CHandledAIGroupDespawnedMsgImp : public CHandledAIGroupDespawnedMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


/**
 * CSteps
 * Set of mission step identifiers.
 * The steps are iterated to test and process them.
 * In the processEvent callbacks, some steps may be added (using addStep())
 * or removed (using setCancelled). Either adding or removing does not break the iteration,
 * and a removed step will be skipped in the current iteration (using isCancelled())
 */
class CSteps
{
	NL_INSTANCE_COUNTER_DECL(CSteps);
public:

	/// Identify a mission step template
	struct CMissionStepId
	{
		TAIAlias	MissionAlias;
		uint32		StepIndex; // starts at 0

		bool operator<( const struct CMissionStepId& s2 ) const
		{
			if ( MissionAlias == s2.MissionAlias )
				return (StepIndex < s2.StepIndex);
			else
				return (MissionAlias < s2.MissionAlias);
		}

		bool operator==( const struct CMissionStepId& s2 ) const
		{
			return (MissionAlias == s2.MissionAlias) &&
				   (StepIndex == s2.StepIndex);
		}
	};

	typedef std::set<CMissionStepId> CMissionStepIds; // does not invalidate iterators when erasing an item
	typedef CMissionStepIds::iterator iterator;
	typedef CMissionStepIds::const_iterator const_iterator;
	
	CSteps() {}

	/// Add an active step (stepIndex starts at 0)
	void		addStep( TAIAlias missionId, uint32 stepIndex )
	{
		CMissionStepId msi;
		msi.MissionAlias = missionId;
		msi.StepIndex = stepIndex;
		_Steps.insert( msi );
	}

	/// Remove a step
	void		removeStep( TAIAlias missionId, uint32 stepIndex )
	{
		CMissionStepId msi;
		msi.MissionAlias = missionId;
		msi.StepIndex = stepIndex;
		_CancelledSteps.push_back( msi );
	}

private:

	friend class CMissionManager;

	/// Return true if there is no step left
	bool		empty() const { return _Steps.empty(); }

	/// Iterate on the steps
	const_iterator	const_begin() const { return _Steps.begin(); }
	/// Iterate on the steps
	const_iterator	const_end() const { return _Steps.end(); }

	/// Iterate on the steps
	iterator	begin() { return _Steps.begin(); }
	/// Iterate on the steps
	iterator	end() { return _Steps.end(); }

	/// Return true if the step was cancelled and should not be processed
	bool		isCancelled( const_iterator itStep ) const
	{
		return (std::find( _CancelledSteps.begin(), _CancelledSteps.end(), *itStep ) != _CancelledSteps.end());
	}

	/// Remove the cancelled steps from the active steps (and from the cancelled steps as well)
	void		removeAllCancelledSteps();

	/// Remove and return true if empty() (does not invalidate other iterators). Precondition: it in [begin(),end()[
	void		removeIteratedStep( iterator it ) { _Steps.erase( it ); }

	// Set of mission steps ids. As this is a step, some elements can be inserted while iterating.
	// However, even though erase an element does not invalidate other iterators, ++ can't be done on
	// an iterator to an erased element
	CMissionStepIds					_Steps;

	std::vector<CMissionStepId>		_CancelledSteps;
};


/**
 * Main manager for the mission system (singleton)
 * stores all the static mission data and is responsible of mission instanciation
 * Mission templates store the mission description
 * Mission instances store the mission dynamic data
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class CMissionManager
{
public:
	///\name Low level/basic features management, interaction with the shard
	//@{
	/// get the singleton instance
	static CMissionManager* getInstance(){return _Instance;}
	/// init the mission manager
	static void init();
	/// release the manager
	static void release();
	/// update called each tick
	void tickUpdate();
	//@}

	///\name accessors
	//@{
	/// get a template from its alias
	CMissionTemplate* getTemplate(TAIAlias  alias);
	static bool getEmoteId(const std::string & str, uint32& emoteId);
	//@}

	/// instanciate a charge mission
	void instanciateChargeMission(TAIAlias  alias, TAIAlias giver, CGuild * guild );
	/// instanciate a mission for a specific player 
	void instanciateMission(CCharacter* user,TAIAlias alias, TAIAlias giver, std::list< CMissionEvent * > & eventList, TAIAlias mainMission = CAIAliasTranslator::Invalid);
	/// deinstanciate a mission From this manager. It doesnt delete it, nor does it remove it from a character
	void deInstanciateMission(CMission * mission);
	/// tell the system that a mission was done for the first time
	void missionDoneOnce( CMissionTemplate* templ );
	/// add a timed mission
	void addTimedMission( CMission * mission );
	/// add a timed mission
	void removeTimedMission( CMission * mission );
	/// send the content of the journal to the player
	void sendDebugJournal( CCharacter * user);
	/// register an escort. If the user is set, it means that the mission is linked to this user rather than a team
	void registerEscort( TAIAlias group, TAIAlias mission, const NLMISC::CEntityId & user = NLMISC::CEntityId::Unknown );
	/// unregister an escort step. If the user is set, it means that the mission is linked to this user rather than a team
	void unregisterEscort( TAIAlias group, const NLMISC::CEntityId & user = NLMISC::CEntityId::Unknown );
	/// update a character escort team if needed.
	void updateEscortTeam( const NLMISC::CEntityId & user );
	/// check if an escort mission fails because of an NPC death
	void checkEscortFailure( TAIAlias group, bool groupWiped );
	/// remove a mono mission
	void removeMonoMission( CMission * mission );
	
	/// add a dyn chat to the manager
	void addDynChat( CMission * instance, CMissionStepDynChat * step, uint32 stepIndex );
	/// switch the speaker of a dyn chat with the successor
	void switchDynChatSpeaker(CCharacter * user, const NLMISC::CEntityId & successorId);
	/// remove all the dynamic chats of a user
	void removeAllUserDynChat(CCharacter * user);
	/// remove a dynamic chat
	void removeMissionDynChat(CCharacter * user, CMission * instance);
	/// player selected a dyn chat answer
	void dynChatChoice( CCharacter * user, const TDataSetRow & botRow,uint8 choice );	
	/// const access to the template pool
	const CHashMap< uint,CMissionTemplate* > &getMissionTemplates()	{ return _MissionTemplates;}

	/// check the place constraints of a mission
	void checkPlaceConstraints ( CMission* mission);
	/// user enters a new place ( check inside / outside constraints 
	void enterPlace( CMission* mission,TAIAlias placeAlias, uint16 placeId);
	/// user leaves a place ( check inside / outside constraints )
	void leavePlace( CMission* mission,TAIAlias placeAlias, uint16 placeId );
	/// cleanup the constraint when it was removed from the mission
	void cleanPlaceConstraint( CMission* mission,TAIAlias placeAlias );
	/// remove a mission from constraints
	void removeFromPlaceConstraints( CMission* mission)
	{
		_PlaceDependantMissions.erase( mission );
	}

	/// remove a mission mith a 'player reconnect' handler
	void removePlayerReconnectHandlingMissions( CMission & mission );
	/// add a mission with a 'player reconnect' handler
	void addPlayerReconnectHandlingMissions( CMission & mission );
	
	/// remove a mission mith a crash handler
	void removeCrashHandlingMissions( CMission & mission );
	/// add a mission with a crash handler
	void addCrashHandlingMissions( CMission & mission );
	/// apply the consequences of an AI crash
	void applyAICrashConsequences( NLNET::TServiceId aiServiceId );

	/// Add a mission step into the list of places to check for 'visit place' mission step. This method can be called from a processed event.
	void insertMissionStepForPlaceCheck( const TDataSetRow& characterRowId, TAIAlias missionId, uint32 stepIndex0 );

	/// Remove a mission step from the list of places to check for 'visit place' mission step. This method can be called from a processed event.
	void removeMissionStepForPlaceCheck( const TDataSetRow& characterRowId, TAIAlias missionId, uint32 stepIndex0 );

	static RY_PDS::IPDBaseData* missionFactoryPD(); 
	static RY_PDS::IPDBaseData* missionFactoryPDSolo();
	static RY_PDS::IPDBaseData* missionFactoryPDTeam();
	static RY_PDS::IPDBaseData* missionFactoryPDGuild();
	
	/// @name Mission validation
	//@{
public:
	bool isMissionValid(std::string const& missionName, std::string const& hashKey);
private:
	void loadMissionValidationFile(std::string const& filename);
	//@}
		
private:

	void addMonoMission( CMission * mission );
	void closeDynChat( CCharacter * user, const TDataSetRow & botRow );
	void openDynChat( CCharacter * user,const CCreature * bot,CMissionStepDynChat * dynChat, const NLMISC::CEntityId & giverId );
	void initInstanciatedMission(CMission * inst, std::list< CMissionEvent * > & eventList);

	/// ctor
	CMissionManager();	
	/// dtor
	~CMissionManager();
	/// parse a primitive to build mission. Return true on success
	bool parsePrimForMissions(const NLLIGO::IPrimitive* prim,const std::string &filename, CMissionGlobalParsingData & globalData, uint &badMissionCount, TAIAlias npcGiverAlias);
	/// Check the place of characters who have VisitPlace missions
	void checkVisitPlaceMissions();
	/// Display statistical data for a mission
//	bool dumpMissionStat(NLMISC::CLog &log, TAIAlias missionAlias);


	/// singleton's instance
	static CMissionManager*							_Instance;
	/// The mission templates, mapped by alias
	CHashMap< uint,CMissionTemplate* >			_MissionTemplates;
	/// the mission timers
	std::list< CMission* >							_TimedMissions;
	/// the mono missions
	std::list< CMission* >							_MonoMissions;
	/// the missions with a crash handler
	std::vector< CMission* >						_CrashHandlingMissions;
	/// the missions with a 'player reconnect' handler
	std::vector< CMission* >						_PlayerReconnectHandlingMissions;
	
	/// emote map used in mission
	static std::map< std::string, uint32 >			_Emotes;

	/// map linking a referenced mission aliases to the real mission alias
	CHashMap<uint,TAIAlias>					_RefMissions;

	/// npc groups being escorted. The key is the group alias. The data is the Alias of the mission linked to the group
	CHashMap<uint,TAIAlias>					_EscortGroups;

	CHashMultiMap<NLMISC::CEntityId, TAIAlias,NLMISC::CEntityIdHashMapTraits> _SoloEscorts;

	typedef std::map<TDataSetRow,CSteps> CStepsByCharacter;

	/// List of (non-persistent) characters ids (see insertMissionStepForPlaceCheck()) and the number of related missions
	CStepsByCharacter								_CharactersForPlaceCheck;

	/// dynamic chats struct
	struct CDynChat
	{
		// mission concerned
		CMission*	Mission;
		// step concerned ( index in the template mission )
		uint32				StepIndex;
		// talking bot
		TDataSetRow			Bot;
	};
	/// dyn chats store by players
	CHashMultiMap<TDataSetRow, CDynChat,TDataSetRow::CHashCode>	_DynChats;

	/// structures used to check place constraints in missions
	struct CPlaceChecker
	{
		TAIAlias			PlaceAlias;
		NLMISC::TGameCycle	EndDate;
	};
	std::map< CMission*, std::vector<CPlaceChecker> > _PlaceDependantMissions;
	
	struct CMissionState
	{
		std::string name;
		std::string state;
		std::string hashKey;
		CMissionState(std::string _name, std::string _state, std::string _hashKey)
		: name(_name), state(_state), hashKey(_hashKey) { }
	};
	typedef std::map<std::string, CMissionState> TMissionStatesContainer;
	typedef std::set<std::string> TValidMissionStatesContainer;
	TMissionStatesContainer			_MissionStates;
	TValidMissionStatesContainer	_ValidMissionStates;
	
	NLMISC_COMMAND_FRIEND(displayMissionStats);
	NLMISC_COMMAND_FRIEND(displayDynChats);
};

inline CMissionTemplate* CMissionManager::getTemplate(TAIAlias  alias)
{
	//first check if it is a reference
	CHashMap<uint,TAIAlias>::iterator itRef = _RefMissions.find( alias );
	if ( itRef != _RefMissions.end() )
		alias = (*itRef).second;

	CHashMap<uint,CMissionTemplate*>::iterator it = _MissionTemplates.find( alias );
	if ( it != _MissionTemplates.end() )
		return ( (*it).second );
	return NULL;
}// CMissionManager::getTemplateId

inline bool CMissionManager::getEmoteId(const std::string & str, uint32&  emoteId)
{
	std::map< std::string, uint32 >::iterator it = _Emotes.find( str );
	if ( it == _Emotes.end() )
		return false;

	emoteId = (*it).second;
	return true;
}// CMissionManager::getEmoteId

inline void CMissionManager::addTimedMission( CMission * mission )
{
	std::list< CMission* >::iterator it = _TimedMissions.begin();
	for (; it != _TimedMissions.end(); ++it )
	{
		if ( (*it)->getEndDate() >= mission->getEndDate() )
		{
			_TimedMissions.insert( it,mission );
			return;
		}
	}
	_TimedMissions.push_back(mission);
}

inline void CMissionManager::removeTimedMission( CMission * mission )
{
	std::list< CMission* >::iterator it = _TimedMissions.begin();
	for (; it != _TimedMissions.end(); ++it )
	{
		if ( (*it) == mission )
		{
			_TimedMissions.erase(it);
			return;
		}
	}

}

inline void CMissionManager::addMonoMission( CMission * mission )
{
	std::list< CMission* >::iterator it = _MonoMissions.begin();
	for (; it != _MonoMissions.end(); ++it )
	{
		if ( (*it)->getMonoEndDate() >= mission->getMonoEndDate() )
		{
			_MonoMissions.insert( it,mission );
			return;
		}
	}
	_MonoMissions.push_back(mission);
}

inline void CMissionManager::removeMonoMission( CMission * mission )
{
	std::list< CMission* >::iterator it = _MonoMissions.begin();
	for (; it != _MonoMissions.end(); ++it )
	{
		if ( (*it) == mission )
		{
			_MonoMissions.erase(it);
			return;
		}
	}
	nlwarning("<CMissionManager removeMonoMission> Unregistered mission %u",mission->getTemplateId());
}

#endif // RY_MISSION_MANAGER_H

/* End of mission_manager.h */



