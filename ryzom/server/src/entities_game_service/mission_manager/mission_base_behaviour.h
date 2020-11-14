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

#ifndef RY_MISSION_BASE_BEHAVIOUR_H
#define RY_MISSION_BASE_BEHAVIOUR_H

#include "game_share/string_manager_sender.h"
#include "mission_manager/mission_event.h"
#include "game_share/season.h"
#include "nel/misc/smart_ptr.h"

namespace EGSPD
{
	class CGuildMemberPD;
	class CGuildPD;
	class CGuildContainerPD;
	class CActiveStepStatePD;
	class CActiveStepPD;
	class CDoneStepPD;
	class CMissionCompassPD;
	class CMissionPD;
	class CMissionGuildPD;
	class CMissionTeamPD;
	class CMissionSoloPD;
	
	void init(uint32 overrideDbId);
};

class CMissionActionSetTeleport;
class CCharacter;
class CMissionTemplate;
class CGameItemPtr;

/**
 * Base gameplay implementations for missions
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CMissionBaseBehaviour : public NLMISC::CRefCount
{
public:
	/// struct representing a mission bot chat
	struct CBotChat
	{
		/// true if a gift is needed
		bool Gift;
		/// index of the involved step
		uint StepIndex;
	};
	
	/// the mission processing state
	enum TProcessingState
	{
		Normal = 0,
		InJump,			// jump instruction encountered : we must proceed with jumps special cleanup and stop action processing
		Complete,		// have to bail out from the current sequence and make the mission complete
		Failed,			// have to bail out from the current sequence and make the mission fail
		ActionFailed,   // have to bail out from the current sequence and make the mission step fail
		Init,			// Init state during instance initialization (activation of step disabled)
	};

	// constructor
	CMissionBaseBehaviour()
	{
		// will be inited
		_Mission = NULL;
	}

	/// init the mission members
	void onCreation(TAIAlias giver);
	/// init the mission members
	void onLoad();
	/// put the mission ingame
	void putInGame();

	///\name text/database related methods
	//@{
	/// send the mission current context menu. fill the textInfos vector. Each pair of this vector contains:
	/// - a bool set to true if the optiion needs a gift
	/// - the text Id of the menu option
	void sendContextTexts(const TDataSetRow& user, const TDataSetRow& interlocutor, std::vector< std::pair<bool,uint32> >& textInfos);
	/// fill a vector of bool. each entry is set to true if the option is a bot gift
	void getBotChatOptions(const TDataSetRow& interlocutor, std::vector<CBotChat> & botChats);
	/// override the mission description
	void overrideDesc( uint32 descIndex );
	/// send the mission description text
	uint32 sendDesc( const TDataSetRow & user );
	/// add a target to the missin compass. targetId is either the bot /place id
	void addCompassTarget( uint32 targetId, bool isBot );
	/// remove a bot from the compass
	void removeCompassBot( TAIAlias bot );
	/// remove a place from the compass
	void removeCompassPlace( uint16 placeId );
	//@}

	
	///\name event processing methods
	//@{
	/// retur the processing state of the mission
	TProcessingState getProcessingState(){ return _ProcessingState; }
	/// set the mission current processing state
	void setProcessingState(TProcessingState state){ _ProcessingState = state;}
	/// process a game event. It process the first event of  the list. All events triggered during the process are added to the list to be processed later
	/// we process in two passes because event processing can lead to the deletion of the mission, among other things
	CMissionEvent::TResult processEvent( const TDataSetRow & userRow, std::list< CMissionEvent* > & eventList,uint32 stepIndex );
	/// test if the generic contrainsts are met (time...)
	bool checkConstraints( bool logForProcessingEvent=true, const std::string dbgPrefix=std::string() ) const;
	/// jump to a specific point of the mission
	void jump( uint32 step, uint32 action, std::list< CMissionEvent * > & eventList);
	/// set the mission timer
	void setTimer( NLMISC::TGameCycle cycle );
	/// activate mission steps
	void activateInitialSteps(std::list< CMissionEvent * > & eventList);
	// flag the mission as finished
	void setSuccessFlag();
	void setFailureFlag();
	//@}
	
	/// setup an escort state
	virtual void setupEscort(const std::vector<TAIAlias> & aliases) = 0;
	/// fill a vector with all the entities that can achive the mission
	virtual void getEntities(std::vector<TDataSetRow>& entities) = 0;
	/// return the mission main character ( user, group leader,...)
	virtual CCharacter* getMainEntity() = 0;
	/// clear the users journal entry corresponding to this mission
	virtual void clearUsersJournalEntry() = 0;
	/// update the users journal entry corresponding to this mission
	virtual void updateUsersJournalEntry() = 0;
	/// stop all children missions that have the same taker
	virtual void stopChildren() = 0;
	/// On failure processing
	virtual void onFailure(bool ignoreJumps,bool sendMessage = true);

	///\name dynamic teleport methods methods
	//@{
	void addTeleport(uint32 idx);
	void removeTeleport(uint32 idx);
	void teleport(CCharacter * user,uint tpIndex);
	TAIAlias getTeleportBot(uint32 tpIndex);
	//@}

	/// force mission success
	virtual void forceSuccess() = 0;
	void checkEscortFailure(bool groupWiped);

	/// an gift to a bot was done
	bool itemGiftDone( CCharacter * user,const std::vector< CGameItemPtr > & itemsGiven,uint32 stepIndex, std::vector<uint32> & result );

	const std::vector<uint32>& getStepState(uint32 idx);
	uint8 getClientIndex(){ return _ClientIndex; }
	void setClientIndex( uint8 idx ){ _ClientIndex = idx; }

	/// update taker compass. return the number of entries
	uint updateCompass(CCharacter & user, const std::string & dbPrefix );

	/// apply the consequences of a crash to this mission
	void applyCrashHandler(bool EGSCrash, const std::string & AIInstanceName );
	
	/// apply the consequences of a player reconnection
	void applyPlayerReconnectHandler();

	/// return true if the mission is consistent with the template
	bool checkConsistencyWithTemplate();

	/// the mission ended successfully so we must check if it unlock some encyclopedia steps
	void updateEncyclopedia();
	
protected:
	void updateUserJournalEntry( CCharacter & user, const std::string & dbPrefix );
	template <class DBType>
	void _updateUserJournalEntry( CCharacter & user, DBType &missionDb);

	template <class DBType>
	uint _updateCompass(CCharacter & user, DBType &missionDb);
	
	CMissionEvent::TResult processEventForStep( const TDataSetRow & userRow,  EGSPD::CActiveStepPD & step, CMissionEvent & event );
	EGSPD::CMissionPD*	_Mission;
	TProcessingState	_ProcessingState;
	uint8				_ClientIndex;
};

#endif // RY_MISSION_BASE_BEHAVIOUR_H

/* End of mission_base_behaviour.h */

