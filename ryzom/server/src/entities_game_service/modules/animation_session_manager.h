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

#ifndef ANIMATION_SESSION_MANAGER_H
#define ANIMATION_SESSION_MANAGER_H


//-----------------------------------------------------------------------------
// includes

#include "nel/misc/singleton.h"
#include "game_share/r2_share_itf.h"


//-----------------------------------------------------------------------------
// the object that tracks public info on animation sessions within the session
// manager singleton. All of the info here can be of use to the ring reward
// system which is why the structure is now in the public header and no longer
// directly in the cpp file

struct TAnimSessionInfo
{
	typedef uint32	TCharId;

	/// basic session info
	R2::TRunningScenarioInfo		ScenarioInfo;
	/// number of point gained
	uint32							RRPGained;
	/// the character that take participation in the session
	std::set<TCharId>				Participants;

	// a little structure to track activity info for players who are participating in the session
	struct TParticipantInfo
	{
		// number of ring points gained by the session participant in the current session
		uint32 PointsGainedThisSession;

		// number of times that the player has gained RRP so far this session
		uint32 NumRRPGains;

		// the best action level for the current time slice
		uint32 BestActionLevel;

		// the game cycle at which this character last gained points
		NLMISC::TGameCycle LastActivityTime;

		// ctor for initialising new records
		TParticipantInfo(): PointsGainedThisSession(0), NumRRPGains(0), BestActionLevel(0), LastActivityTime(0) {}
	};
	typedef std::map<TCharId,TParticipantInfo> TParticipantInfos;
	TParticipantInfos ParticipantInfos;
};


//-----------------------------------------------------------------------------
// the singleton interface

class IAnimSessionMgr 
	:	public NLMISC::CManualSingleton<IAnimSessionMgr>
{
public:

	/// An animation session is started, init the necessary storage for it.
	virtual void animSessionStarted(uint32 sessionId, const R2::TRunningScenarioInfo &scenarioInfo) =0;

	/** An animation session is ended, report the RRP gained to SU and release 
	 *	session resources.
	 */
	virtual void animSessionEnded(uint32 sessionId, uint32 scenarioScore, NLMISC::TTime timeTaken) =0;

	/** A character enter an animation session as player, start accumulating point
	 *	for it.
	 */
	virtual void characterEnterAnimSession(uint32 sessionId, uint32 charId) =0;

	/** A character leave an animation session as player, stop accumulating point
	 *	for it but keep the gained point for the report (or for later if the
	 *	character re-enter the session).
	 */
	virtual void characterLeaveAnimSession(uint32 sessionId, uint32 charId) =0;


	/** A character has done soething 'active' so update their last activity timestamp
	 */
	virtual void flagCharAsActive(uint32 charId, uint32 level) =0;

	/** A character has gained some ring points, add them to their curent anim session
	 */
	virtual void addRRPForChar(uint32 charId, uint32 nbRingPoints) =0;

	/** Retrieve the current session info for a character.
	 *	may return NULL if the character is found to be in a scenario
	 */
	virtual const TAnimSessionInfo *getAnimSessionForChar(uint32 charId) const =0;
};


#endif // ANIMATION_SESSION_MANAGER_H
