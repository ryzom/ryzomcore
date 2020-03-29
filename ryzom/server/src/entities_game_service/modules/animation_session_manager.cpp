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
#include "nel/net/module_builder_parts.h"
#include "game_share/ring_session_manager_itf.h"
#include "server_share/log_ring_gen.h"
#include "animation_session_manager.h"
#include "player_manager/character_interface.h"
#include "player_manager/ring_reward_points.h"
#include "player_manager/player_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

class CAnimSessionMgr 
	:	public IAnimSessionMgr,
		public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
		public IModuleTrackerCb
{
	typedef uint32	TCharId;	typedef uint32	TSessionId;

	typedef CModuleTracker<TModuleClassPred>	TMyModuleTracker;
	/// A tracker to track the SU module 'RingSessionManager'.
	TMyModuleTracker							_RingSessionManagerTracker;
	/// A tracker to track the DSS module 'ServerAnimationModule'
	TMyModuleTracker							_ServerAnimationModuleTracker;


	/// Struct to hold info about active animation session
	typedef map<TSessionId, TAnimSessionInfo>	TAnimSessionInfos;
	/// Storage for all active animation sessions
	TAnimSessionInfos							_AnimSessionInfos;

	typedef map<TCharId, TSessionId>			TCharCurrentAnimSession;
	/// a map that store the current anim session for each character
	TCharCurrentAnimSession						_CharCurrentAnimSession;


public:
	CAnimSessionMgr()
		:	_RingSessionManagerTracker(TModuleClassPred("RingSessionManager")),
			_ServerAnimationModuleTracker(TModuleClassPred("ServerAnimationModule"))
	{
		// init the tracker without callback
		_RingSessionManagerTracker.init(this);
		// init the tracker with a callback
		_ServerAnimationModuleTracker.init(this, this);
	}


	///////////////////////////////////////////////////////////////////////////
	// implementation if IModuleTrackerCb
	///////////////////////////////////////////////////////////////////////////
	virtual void onTrackedModuleUp(IModuleProxy *moduleProxy)
	{
		// nothing
	}
	virtual void onTrackedModuleDown(IModuleProxy *moduleProxy)
	{
		// as we attached only to the server animation module tracker callbacks,
		// we can remove all session data we stored here
		_AnimSessionInfos.clear();
		_CharCurrentAnimSession.clear();
	}
	///////////////////////////////////////////////////////////////////////////
	// implementation if IAnimSessionMgr
	///////////////////////////////////////////////////////////////////////////
	/// An animation session is started, init the necessary storage for it.
	void animSessionStarted(uint32 sessionId, const R2::TRunningScenarioInfo &scenarioInfo)
	{
		nldebug("animSessionStarted : session %u started with scenario '%s', desc '%s', md5 '%s', owner %u, author '%s'",
			sessionId,
			scenarioInfo.getScenarioTitle().c_str(),
			scenarioInfo.getScenarioDesc().c_str(),
			scenarioInfo.getScenarioKey().toString().c_str(),
			scenarioInfo.getSessionAnimatorCharId(),
			scenarioInfo.getScenarioAuthorName().c_str());
		// create a record for this session
		_AnimSessionInfos[sessionId].ScenarioInfo = scenarioInfo;
		_AnimSessionInfos[sessionId].Participants.clear();
		_AnimSessionInfos[sessionId].RRPGained = 0;
	}

	/** An animation session is ended, report the RRP gained to SU and release 
	 *	session resources.
	 */
	void animSessionEnded(uint32 sessionId, uint32 scenarioScore, NLMISC::TTime timeTaken)
	{
		nldebug("animSessionEnded : session %u ended", sessionId);

		TAnimSessionInfos::iterator it(_AnimSessionInfos.find(sessionId));
		BOMB_IF(it == _AnimSessionInfos.end(), "No info for session "<<sessionId<<" in the anim session manager", return);

		TAnimSessionInfo &asi = it->second;

		const TMyModuleTracker::TTrackedModules &modules = _RingSessionManagerTracker.getTrackedModules();

		// should have only one module there
		if (modules.empty())
			return;

		vector<TCharId> charIds(asi.Participants.begin(), asi.Participants.end());
		// send the end session report to the SU
		RSMGR::CRingSessionManagerProxy rsm(*modules.begin());
		rsm.scenarioEnded(this, sessionId, asi.ScenarioInfo, asi.RRPGained, scenarioScore, uint32(timeTaken), charIds);

		// release resources
		_AnimSessionInfos.erase(it);
	}

	/** A character enter an animation session as player, start accumulating point
	 *	for it.
	 */
	void characterEnterAnimSession(uint32 sessionId, uint32 charId)
	{
		nldebug("characterEnterAnimSession : character %u enter anim session %u", charId, sessionId);
		// remember the current session for this char (in order to cumulate points in the right scenario)
		_CharCurrentAnimSession[charId] = sessionId;
		TAnimSessionInfos::iterator it(_AnimSessionInfos.find(sessionId));
		BOMB_IF(it == _AnimSessionInfos.end(), "No info for session "<<sessionId<<" in the anim session manager", return);

		TAnimSessionInfo &asi = it->second;

		// remember this player has participated in the session at one time
		asi.Participants.insert(charId);

		// set the session level in the character RRP object (if character in game)
		ICharacter *ichar = ICharacter::getInterface(charId, true);
		BOMB_IF(ichar==0, "characterEnterAnimSession: ICharacter::getInterface("<<charId<<", true) return NULL", return);
		BOMB_IF(!ichar->getEnterFlag(), "characterEnterAnimSession: character "<<charId<<" has enterFlag false", return);
		
		CRingRewardPoints &rrp = ichar->getRingRewardPoints();
		rrp.setScenarioLevel(asi.ScenarioInfo.getSessionLevel());

		log_Ring_EnterSession(CEntityId(RYZOMID::player, uint64(charId)), sessionId);
	}

	/** A character leave an animation session as player, stop accumulating point
	 *	for it but keep the gained point for the report (or for later if the
	 *	character re-enter the session).
	 */
	void characterLeaveAnimSession(uint32 sessionId, uint32 charId)
	{
		nldebug("characterLeaveAnimSession : character %u leave anim session %u", charId, sessionId);
		// remove the current session (if already stored with this session)
		TCharCurrentAnimSession::iterator it(_CharCurrentAnimSession.find(charId));
		if (it != _CharCurrentAnimSession.end() && it->second == sessionId)
			_CharCurrentAnimSession.erase(it);
		else
			nlwarning("characterLeaveAnimSession : the character %u is currently in session %u, not %u",
				charId, it->second, sessionId);

		log_Ring_LeaveSession(CEntityId(RYZOMID::player, uint64(charId)), sessionId);
	}

	/** A character has done something 'active' so update their last activity timestamp
	 */
	virtual void flagCharAsActive(uint32 charId,uint32 level)
	{
		/// retrieve the session this character belong to
		TAnimSessionInfo *sessionInfo = getAnimSessionForChar(charId);
		BOMB_IF(sessionInfo == NULL, "Failed to retrieve the current anim session of char "<<charId, return);
		/// set last activity timestamp and best action level
		TAnimSessionInfo::TParticipantInfo& theParticipantInfo= sessionInfo->ParticipantInfos[charId];
		theParticipantInfo.LastActivityTime= CTickEventHandler::getGameCycle();
		theParticipantInfo.BestActionLevel= std::max(theParticipantInfo.BestActionLevel,level);
	}

	/** A character has gained some ring points, add them to their curent anim session
	 */
	virtual void addRRPForChar(uint32 charId, uint32 nbRingPoints)
	{
		/// retrieve the session this character belong to
		TAnimSessionInfo *sessionInfo = getAnimSessionForChar(charId);
		BOMB_IF(sessionInfo == NULL, "Failed to retrieve the current anim session of char "<<charId, return);
		
		/// add ring points to global accumulator for entire scenario
		sessionInfo->RRPGained += nbRingPoints;

		/// get a refference to the participant info for the character, create new record if required
		TAnimSessionInfo::TParticipantInfo& theParticipantInfo= sessionInfo->ParticipantInfos[charId];

		/// add ring points to given player's personal accumulator
		theParticipantInfo.PointsGainedThisSession+= nbRingPoints;

		/// incremenet the counter of number of times RRPs have been gained this session
		theParticipantInfo.NumRRPGains ++;

		/// reset the best action level for the character
		theParticipantInfo.BestActionLevel= 0;
	}

	/** Retrieve the current anim session info for a char
	 *	may return NULL if the character is not found to be in a scenario
	 */
	TAnimSessionInfo *getAnimSessionForChar(uint32 charId)
	{
		/// retrieve the session this character belong to
		TCharCurrentAnimSession::iterator it = _CharCurrentAnimSession.find(charId);
		if (it == _CharCurrentAnimSession.end())
			return NULL;

		TSessionId &sessionId = it->second;
		TAnimSessionInfos::iterator it2(_AnimSessionInfos.find(sessionId));
		if (it2 == _AnimSessionInfos.end())
			return NULL;
		
		return &(it2->second);
	}

	virtual const TAnimSessionInfo *getAnimSessionForChar(uint32 charId) const
	{
		return const_cast<CAnimSessionMgr*>(this)->getAnimSessionForChar(charId);
	}

	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CAnimSessionMgr, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CAnimSessionMgr, dump, "dump the session manager internal state", "no param");
		NLMISC_COMMAND_HANDLER_ADD(CAnimSessionMgr, listAnimSession, "list all know anim session", "no param");
		NLMISC_COMMAND_HANDLER_ADD(CAnimSessionMgr, dumpAnimSession, "dump the state of an anim session", "<sessionId>");
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dumpAnimSession)
	{
		if (args.size() != 1)
			return false;

		TSessionId sessionId;
		NLMISC::fromString(args[0], sessionId);
		if (sessionId == 0)
		{
			log.displayNL("Invalid session id '%s'", args[0].c_str());
			return true;
		}

		TAnimSessionInfos::iterator it(_AnimSessionInfos.find(sessionId));
		if (it == _AnimSessionInfos.end())
		{
			log.displayNL("No session found with id %u", sessionId);
			return true;
		}
		TAnimSessionInfo &asi = it->second;

		log.displayNL("Dumping session %u :", sessionId);
		log.displayNL("  md5       = %s",		asi.ScenarioInfo.getScenarioKey().toString().c_str());
		log.displayNL("  title     = '%s'",		asi.ScenarioInfo.getScenarioTitle().c_str());
		log.displayNL("  desc      = '%s'",		asi.ScenarioInfo.getScenarioDesc().c_str());
		log.displayNL("  level     = %s",		asi.ScenarioInfo.getSessionLevel().toString().c_str());
		log.displayNL("  author    = '%s'",		asi.ScenarioInfo.getScenarioAuthorName().c_str());
		log.displayNL("  animatorId= %u",		asi.ScenarioInfo.getSessionAnimatorCharId());
		log.displayNL("  mode      = %s",		asi.ScenarioInfo.getDMLess() ? "dmless" : "mastered");
		log.displayNL("  RRP fained= %u",		asi.RRPGained);

		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(listAnimSession)
	{
		if (args.size() != 0)
			return false;

		log.displayNL("Listing %u anim sessions:", _AnimSessionInfos.size());

		TAnimSessionInfos::const_iterator first(_AnimSessionInfos.begin()), last(_AnimSessionInfos.end());
		for (; first != last; ++first)
		{
			const TAnimSessionInfo &asi = first->second;
			log.displayNL("  Session %u use scenario '%s' (md5 %s) and has seen %u player up to now and has gained %u RRP.",
				first->first,
				asi.ScenarioInfo.getScenarioTitle().c_str(),
				asi.ScenarioInfo.getScenarioKey().toString().c_str(),
				asi.Participants.size(),
				asi.RRPGained);
		}

		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(dump)
	{
		// recall base class dump
		NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

		log.displayNL("--------------------------------");
		log.displayNL("Dumping AnimSessionManager state");
		log.displayNL("--------------------------------");

		// dump some general information
		log.displayNL(" This module know %u ring session manager module, %u server animation module", 
			_RingSessionManagerTracker.getTrackedModules().size(),
			_ServerAnimationModuleTracker.getTrackedModules().size());
		log.displayNL(" There are %u running anim session and %u character in anim session",
			_AnimSessionInfos.size(),
			_CharCurrentAnimSession.size());

		return true;
	}
};

NLNET_REGISTER_MODULE_FACTORY(CAnimSessionMgr, "AnimSessionManager");
