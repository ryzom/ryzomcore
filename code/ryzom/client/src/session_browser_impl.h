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

#ifndef SESSION_BROWSER_IMPL_H
#define SESSION_BROWSER_IMPL_H


#include "session_browser.h"
#include "game_share/ring_session_manager_itf.h"
#include "nel/gui/lua_helper.h"
#include "far_tp.h"

class CSessionBrowserImpl : public CSessionBrowser,
							public NLMISC::CSingleton<CSessionBrowserImpl>
{
public:
	/** Register access to session browser from lua (from the 'game' table)
	  *
      * game.getSessionList() -> send a request to server to get the list of session
	  * As a result :
	  * On success, RingAccessPoint:fill() will be called with a table containing the result of the request
	  * On failure : - RingAccessPoint:onDisconnection()
	  *              - RingAccessPoint:onConnectionClosed()
	  *              - RingAccessPoint:onConnectionFailed()
	  */
	void init(CLuaState *ls);
	// from CSessionBrowser
	virtual void on_connectionFailed();
	virtual void on_connectionClosed();
	virtual void on_CRingSessionManagerWebClient_Disconnection(NLNET::TSockId from);
	virtual void on_invokeResult(NLNET::TSockId from, uint32 userId, uint32 resultCode, const std::string &resultString);
	virtual void on_scheduleSessionResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint8 result, const std::string &resultString);
	virtual void on_sessionInfoResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const RSMGR::TRaceFilter &raceFilter, const RSMGR::TReligionFilter &religionFilter,
		const RSMGR::TGuildFilter &guildFilter, const RSMGR::TShardFilter &shardFilter, const RSMGR::TLevelFilter &levelFilter, bool subscriptionClosed, bool autoInvite, const std::string &language, const RSMGR::TSessionOrientation &orientation, const std::string &description);
	virtual void on_joinSessionResult(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const RSMGR::TSessionPartStatus &participantStatus);
	virtual void on_joinSessionResultExt(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const RSMGR::TSessionPartStatus &participantStatus, const CSecurityCode& securityCode);
	virtual void on_getShardsResult(NLNET::TSockId from, uint32 userId, const std::string &result);
	virtual void on_CSessionBrowserServerWebClient_Disconnection(NLNET::TSockId from);
	virtual void on_sessionList(NLNET::TSockId from, uint32 charId, const std::vector < RSMGR::TSessionDesc > &sessions);
	virtual void on_charList(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::vector < RSMGR::TCharDesc > &charDescs);
	virtual void on_playerRatings(NLNET::TSockId from, uint32 charId, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection);
	virtual void on_ringRatings(NLNET::TSockId from, uint32 charId, uint32 authorRating, uint32 AMRating, uint32 masterlessRating);
	virtual void on_ringPoints(NLNET::TSockId from, uint32 charId, const std::string &ringPoints, const std::string &maxRingPoints);
	// Return average scores of a session
	virtual void on_sessionAverageScores(NLNET::TSockId from, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal);
	virtual void on_scenarioAverageScores(NLNET::TSockId from, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal);

	static uint32 getCharId();

	// tmp, for local test
	void testFill();
	void testCharsFill();


	// Result value
	uint32		_LastInvokeResult;
	std::string	_LastInvokeResultMsg;

	uint32		_LastJoinSessionResult;
	TSessionId	_LastJoinSessionId;
	std::string	_LastJoinSessionShardAddr;
	RSMGR::TSessionPartStatus	_LastJoinSessionPartStatus;

	uint32		_LastScheduleSessionCharId;
	uint32		_LastScheduleSessionResult;
	TSessionId	_LastScheduleSessionId;
	std::string	_LastScheduleSessionResulMsg;

	CFarTP::TJoinMode	CurrentJoinMode;

	RSMGR::TRaceFilter		_LastRaceFilter;
	RSMGR::TReligionFilter	_LastReligionFilter;
	RSMGR::TGuildFilter		_LastGuildFilter;
	RSMGR::TShardFilter		_LastShardFilter;
	RSMGR::TLevelFilter		_LastLevelFilter;
	bool					_LastSubscriptionClosed;
	bool					_LastAutoInvite;
	std::string				_LastDescription;

	uint32 _LastAuthorRating;
	uint32 _LastAMRating;
	uint32 _LastMasterlessRating;
	std::string _LastRingPoints;
	std::string _LastMaxRingPoints;

	static const std::string &getFrontEndAddress();
private:
	CLuaState::TRefPtr _Lua;
	static int luaGetRingSessionList(CLuaState &ls);
	static int luaGetRingCharList(CLuaState &ls);
	static int luaJoinRingSession(CLuaState &ls);
	static int luaCheckRingAccess(CLuaState &ls);
	static int luaGetFileHeader(CLuaState &ls);
	static int luaGetRingStats(CLuaState &ls);
	static int luaGetScenarioScores(CLuaState &ls);
	static int luaUpdateScenarioScores(CLuaState &ls);
	static int luaGetSessionAverageScores(CLuaState &ls);
	static int luaGetScenarioAverageScores(CLuaState &ls);
	// Call a method inside the 'RingAccessPoint' lua table
	void callRingAccessPointMethod(const char *name, int numArg, int numResult);
	void callRingCharTrackingMethod(const char *name, int numArg, int numResult);
	void callRingPlayerInfoMethod(const char *name, int numArg, int numResult);
	void callScenarioScoresMethod(const char *name, int numArg, int numResult);

	void fill(const std::vector <RSMGR::TSessionDesc > &sessions);
	void charsFill(const std::vector <RSMGR::TCharDesc > &chars);
	void ringStatsFill();
	void playerRatingFill(bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection);
	void averageScoresFill(bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal);

	static const NLNET::CLoginCookie &getCookie();
};


#endif //SESSION_BROWSER_H
