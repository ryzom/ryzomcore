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
#include "session_browser_impl.h"
#include "nel/gui/lua_object.h"
#include "nel/gui/lua_ihm.h"
#include "connection.h"
#include "net_manager.h"
#include "interface_v3/interface_manager.h"
#include "interface_v3/skill_manager.h"
#include "far_tp.h"
#include "r2/dmc/com_lua_module.h"
//
#include "nel/misc/time_nl.h"
#include "nel/net/service.h"
//
#include "r2/editor.h"
#include "r2/dmc/client_edition_module.h"
//
#include "game_share/shard_names.h"

using namespace std;
using namespace NLMISC;
using namespace RSMGR;
using namespace CHARSYNC;
using namespace NLNET;
using namespace R2;

CVariable<uint16> SBSPortOffset("client", "SBSPortOffset", "Offset of the SBS port from the FS port", 1000, 0, true);


// ****************************************************************************
void CSessionBrowserImpl::init(CLuaState *ls)
{
	if (ls != NULL)
	{
		nlassert(ls);
		_Lua = ls;
		_Lua->pushGlobalTable();
		CLuaObject game(*_Lua);
		game = game["game"];
		game.setValue("getRingSessionList", luaGetRingSessionList);
		game.setValue("getRingCharList", luaGetRingCharList);
		game.setValue("getRingStats", luaGetRingStats);
		game.setValue("getScenarioScores", luaGetScenarioScores);
		game.setValue("getSessionAverageScores", luaGetSessionAverageScores);
		game.setValue("getScenarioAverageScores", luaGetScenarioAverageScores);
		game.setValue("updateScenarioScores", luaUpdateScenarioScores);
		game.setValue("joinRingSession", luaJoinRingSession);
		game.setValue("checkRingAccess", luaCheckRingAccess);
		game.setValue("getFileHeader", luaGetFileHeader);
	}
	if (!ClientCfg.Local)
	{
		CSessionBrowserImpl::getInstance().setAuthInfo(getCookie());
		NLNET::CInetAddress address(getFrontEndAddress());
		address.setPort(address.port()+SBSPortOffset);
		CSessionBrowserImpl::getInstance().connectItf(address);
	}

	_LastAuthorRating = 0;
	_LastAMRating = 0;
	_LastMasterlessRating = 0;
	_LastRingPoints = string("");
	_LastMaxRingPoints = string("");
}


// ***************************************************************************
const NLNET::CLoginCookie &CSessionBrowserImpl::getCookie()
{
	/*if (CInterfaceManager::getInstance()->isInGame())
	{*/
		return NetMngr.getLoginCookie();
	/*}
	extern NLNET::CLoginCookie FakeCookie;
	return FakeCookie; // TMP TMP : for Nico's test*/
}

// ****************************************************************************
const std::string &CSessionBrowserImpl::getFrontEndAddress()
{
	if (!NetMngr.getFrontendAddress().empty())
	{
		return NetMngr.getFrontendAddress();
	}
	static std::string testAddress = "borisb";
	return testAddress; // TMP TMP : for Nico's test
}

// ****************************************************************************
uint32 CSessionBrowserImpl::getCharId()
{
	if (ClientCfg.Local)  return 0;
	return (getCookie().getUserId() << 4) + (uint32) PlayerSelectedSlot;
}

// ****************************************************************************
int CSessionBrowserImpl::luaGetRingSessionList(CLuaState &ls)
{
	nldebug("SB: luaGetRingSessionList");
	CLuaIHM::checkArgCount(ls, "getRingSessionList", 0);
	CSessionBrowserImpl::getInstance().getRingRatings(getCharId());
	CSessionBrowserImpl::getInstance().getSessionList(getCharId());
	return 0;
}

// ****************************************************************************
int CSessionBrowserImpl::luaGetRingCharList(CLuaState &ls)
{
	nldebug("SB: luaGetRingCharList");
	CLuaIHM::checkArgCount(ls, "getRingCharList", 0);
	if (R2::getEditor().getMode() != R2::CEditor::NotInitialized)
	{
		CSessionBrowserImpl::getInstance().getCharList(getCharId(), R2::getEditor().getDMC().getEditionModule().getCurrentAdventureId());
	}
	return 0;
}

// ****************************************************************************
int CSessionBrowserImpl::luaGetRingStats(CLuaState &ls)
{
	nldebug("SB: luaGetRingStats");
	CLuaIHM::checkArgCount(ls, "getRingStats", 0);
	CSessionBrowserImpl::getInstance().getRingRatings(getCharId());
	CSessionBrowserImpl::getInstance().getRingPoints(getCharId());
	return 0;
}

// ****************************************************************************
int CSessionBrowserImpl::luaGetScenarioScores(CLuaState &ls)
{
	nldebug("SB: luaGetScenarioScores");
	CLuaIHM::checkArgCount(ls, "getScenarioScores", 0);
	if (R2::getEditor().getMode() != R2::CEditor::NotInitialized)
	{
		CSessionBrowserImpl::getInstance().getMyRatings(getCharId(), R2::getEditor().getDMC().getEditionModule().getCurrentAdventureId());
	}
	return 0;
}

// ****************************************************************************
int CSessionBrowserImpl::luaGetSessionAverageScores(CLuaState &ls)
{
	nldebug("SB: luaGetSessionAverageScores");
	CLuaIHM::checkArgCount(ls, "getSessionAverageScores", 0);
	if (R2::getEditor().getMode() != R2::CEditor::NotInitialized)
	{
		CSessionBrowserImpl::getInstance().getSessionAverageScores(R2::getEditor().getDMC().getEditionModule().getCurrentAdventureId());
	}
	return 0;
}

// ****************************************************************************
int CSessionBrowserImpl::luaGetScenarioAverageScores(CLuaState &ls)
{
	nldebug("SB: luaGetScenarioAverageScores");
	CLuaIHM::checkArgCount(ls, "getScenarioAverageScores", 1);

	CLuaIHM::checkArgType(ls, "getScenarioAverageScores", 1, LUA_TSTRING);

	CSessionBrowserImpl::getInstance().getScenarioAverageScores(ls.toString(1));

	return 0;
}

// ****************************************************************************
int CSessionBrowserImpl::luaUpdateScenarioScores(CLuaState &ls)
{
	nldebug("SB: luaUpdateScenarioScores");
	const char *funcName = "updateScenarioScores";
	CLuaIHM::checkArgCount(ls, funcName, 5);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
	CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
	CLuaIHM::checkArgType(ls, funcName, 3, LUA_TNUMBER);
	CLuaIHM::checkArgType(ls, funcName, 4, LUA_TNUMBER);
	CLuaIHM::checkArgType(ls, funcName, 5, LUA_TNUMBER);

	if (R2::getEditor().getMode() != R2::CEditor::NotInitialized)
	{
		CSessionBrowserImpl::getInstance().setPlayerRating(getCharId(), R2::getEditor().getDMC().getEditionModule().getCurrentAdventureId(),
			(uint32) ls.toNumber(1), (uint32) ls.toNumber(2), (uint32) ls.toNumber(3), (uint32) ls.toNumber(4), (uint32) ls.toNumber(5));
	}

	return 0;
}

// ****************************************************************************
int CSessionBrowserImpl::luaJoinRingSession(CLuaState &ls)
{
	nldebug("SB: luaJoinRingSession");
	const char *funcName = "joinRingSession";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);

	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CSessionBrowserImpl & sessionBrowser = CSessionBrowserImpl::getInstance();
	sessionBrowser.joinSession(getCharId(), (TSessionId)(uint32) ls.toNumber(1), ClientCfg.ConfigFile.getVar("Application").asString(0));

	if(!sessionBrowser.waitOneMessage(sessionBrowser.getMessageName("on_joinSessionResult")))
	{
		nlwarning("joinSession callback return false");
	}

	if(sessionBrowser._LastJoinSessionResult == 20)
	{
		CViewText* pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:warning_free_trial:text"));
		if (pVT != NULL)
			pVT->setText(CI18N::get("uiRingWarningFreeTrial"));
		CAHManager::getInstance()->runActionHandler("enter_modal", NULL, "group=ui:interface:warning_free_trial");
	}

	return 0;
}

// *****************************************************************************
int CSessionBrowserImpl::luaCheckRingAccess(CLuaState &ls)
{
	lua_State* state = ls.getStatePointer();
	return R2::CComLuaModule::luaGetFileHeader(state);

}

// *****************************************************************************
int CSessionBrowserImpl::luaGetFileHeader(CLuaState &ls)
{
	lua_State* state = ls.getStatePointer();
	return R2::CComLuaModule::luaGetFileHeader(state);
}


// ****************************************************************************
void CSessionBrowserImpl::on_connectionFailed()
{
	nldebug("SB: on_connectionFailed");
	callRingAccessPointMethod("onConnectionFailed", 0, 0);
}

// ****************************************************************************
void CSessionBrowserImpl::on_CRingSessionManagerWebClient_Disconnection(NLNET::TSockId /* from */)
{
	nldebug("SB: on_CRingSessionManagerWebClient_Disconnection");
	callRingAccessPointMethod("onDisconnection", 0, 0);
}

// ****************************************************************************
void CSessionBrowserImpl::on_connectionClosed()
{
	nldebug("SB: on_connectionClosed");
	callRingAccessPointMethod("onConnectionClosed", 0, 0);
}

// ****************************************************************************
void CSessionBrowserImpl::on_invokeResult(NLNET::TSockId /* from */, uint32 /* userId */, uint32 resultCode, const std::string &resultString)
{
	nldebug("SB: on_invokeResult : result = %u, msg='%s'", resultCode, resultString.c_str());
	//
	_LastInvokeResult = resultCode;
	_LastInvokeResultMsg = resultString;
}

// ****************************************************************************
void CSessionBrowserImpl::on_scheduleSessionResult(NLNET::TSockId /* from */, uint32 charId, TSessionId sessionId, uint8 result, const std::string &resultString)
{
	nldebug("SB: on_scheduleSessionResult : result = %u, msg = '%s'", result, resultString.c_str());
	_LastScheduleSessionCharId = charId;
	_LastScheduleSessionResult = result;
	_LastScheduleSessionId = sessionId;
	_LastScheduleSessionResulMsg = resultString;
//	if (result == 0)
//	{
//		// attempt real far tp
//		if (!FarTP.requestFarTPToSession(sessionId, PlayerSelectedSlot, CFarTP::JoinSession, true))
//		{
//			callRingAccessPointMethod("onJoinFailed", 0, 0);
//		}
//	}
//	else
//	{
//		nlwarning(resultString.c_str());
//		callRingAccessPointMethod("onJoinFailed", 0, 0);
//	}
}

// ****************************************************************************
void CSessionBrowserImpl::on_sessionInfoResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const RSMGR::TRaceFilter &raceFilter, const RSMGR::TReligionFilter &religionFilter,
		const RSMGR::TGuildFilter &guildFilter, const RSMGR::TShardFilter &shardFilter, const RSMGR::TLevelFilter &levelFilter, bool subscriptionClosed, bool autoInvite, const std::string &language,
		const TSessionOrientation &/* orientation */, const std::string &description)
{
	nldebug("SB: on_sessionInfoResult");
	_LastRaceFilter = raceFilter;
	_LastReligionFilter = religionFilter;
	_LastGuildFilter = guildFilter;
	_LastShardFilter = shardFilter;
	_LastLevelFilter = levelFilter;
	_LastSubscriptionClosed = subscriptionClosed;
	_LastAutoInvite = autoInvite;
	_LastDescription = description;
}

// ****************************************************************************
void CSessionBrowserImpl::on_joinSessionResult(NLNET::TSockId /* from */, uint32 /* userId */, TSessionId sessionId, uint32 result, const std::string &shardAddr, const RSMGR::TSessionPartStatus &participantStatus)
{
	nldebug("SB: on_joinSessionResult : result = %u; msg = '%s'", result, shardAddr.c_str());
	_LastJoinSessionResult = result;
	_LastJoinSessionId = sessionId;
	_LastJoinSessionShardAddr = shardAddr;
	_LastJoinSessionPartStatus = participantStatus;

	// trigger the far tp action
	if (result == 0)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CAHManager::getInstance()->runActionHandler("on_connect_to_shard", NULL, string("cookie=")+NetMngr.getLoginCookie().toString()+"|fsAddr="+shardAddr);
	}
}

// ****************************************************************************
void CSessionBrowserImpl::on_joinSessionResultExt(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const RSMGR::TSessionPartStatus &participantStatus, const CSecurityCode& securityCode)
{
	nldebug("SB: on_joinSessionResultExt : result = %u, msg = '%s'", result, shardAddr.c_str());
	FarTP.setJoinSessionResult(sessionId, securityCode); // only joinSessionResultExt grants fast disconnection

	on_joinSessionResult(from, userId, sessionId, result, shardAddr, participantStatus);
}

// ****************************************************************************
void CSessionBrowserImpl::on_getShardsResult(NLNET::TSockId /* from */, uint32 /* userId */, const std::string &/* result */)
{
	nldebug("SB: on_getShardsResult");
}

// ****************************************************************************
void CSessionBrowserImpl::on_CSessionBrowserServerWebClient_Disconnection(NLNET::TSockId /* from */)
{
	nldebug("SB: on_CSessionBrowserServerWebClient_Disconnection");
	callRingAccessPointMethod("onDisconnection", 0, 0);
}

// ****************************************************************************
void CSessionBrowserImpl::on_sessionList(NLNET::TSockId /* from */, uint32 /* charId */, const std::vector <RSMGR::TSessionDesc > &sessions)
{
	nldebug("SB: on_sessionList");
	fill(sessions);
}

// ****************************************************************************
void CSessionBrowserImpl::fill(const std::vector <RSMGR::TSessionDesc > &sessions)
{
	// build datas & send to lua
	nlassert(_Lua);
	try
	{
		CLuaStackRestorer lsr(_Lua, _Lua->getTop());
		_Lua->newTable();
		for (uint k = 0; k < sessions.size(); ++k)
		{
			const RSMGR::TSessionDesc &sd = sessions[k];
			_Lua->newTable();
			CLuaObject session(*_Lua);
			session.setValue("Id", (double) sd.getSessionId().asInt());
			session.setValue("Owner", sd.getOwnerName());
			session.setValue("Title", sd.getTitle());
			session.setValue("Desc", sd.getDescription());
			session.setValue("Level", (double) sd.getSessionLevel().getValue());
			session.setValue("Language", sd.getLanguage());
			uint flags = (sd.getAnimMode().getValue() == RSMGR::TAnimMode::am_dm ? (uint) 1 : 0) |
						 (sd.getRequesterCharInvited()                           ? (uint) 2 : 0);
			if(sd.getRequesterCharKicked())
				flags = (uint) 4;
			session.setValue("Flags", (double)  flags);
			session.setValue("PlayerCount", (double)  sd.getNbConnectedPlayer());
			session.setValue("AllowFreeTrial", (double)  sd.getAllowFreeTrial());

			session.setValue("NbRating",			(double) sd.getNbRating());
			session.setValue("RateFun",				(double) sd.getRateFun());
			session.setValue("RateDifficulty",		(double) sd.getRateDifficulty());
			session.setValue("RateAccessibility",	(double) sd.getRateAccessibility());
			session.setValue("RateOriginality",		(double) sd.getRateOriginality());
			session.setValue("RateDirection",		(double) sd.getRateDirection());

			session.setValue("ScenarioRRPTotal",	(double) sd.getScenarioRRPTotal());

			session.setValue("AuthorRating",		(double) _LastAuthorRating);
			if(sd.getAnimMode().getValue() == RSMGR::TAnimMode::am_dm)
				session.setValue("OwnerRating",			(double) _LastAMRating);
			else
				session.setValue("OwnerRating",	(double) _LastMasterlessRating);

			// calculate the difference between local time and gmt
			time_t rawtime;
			struct tm * timeinfo;

			rawtime= sd.getLaunchDate();
			timeinfo = localtime ( &rawtime );
			time_t localTime = mktime( timeinfo );
			//nldebug("local time %02d:%02d",timeinfo->tm_hour,timeinfo->tm_min);

			rawtime= sd.getLaunchDate();
			timeinfo = gmtime ( &rawtime );
			time_t gmtTime = mktime( timeinfo );
			//nldebug("gm time %02d:%02d",timeinfo->tm_hour,timeinfo->tm_min);

			// convert GMT time value from server to local time
			time_t adjustedTime= sd.getLaunchDate() + localTime - gmtTime;
			session.setValue("LaunchDate", (double)  adjustedTime);

			session.setValue("ScenarioType", (double)  sd.getOrientation().getValue());
			session.push();
			_Lua->rawSetI(-2, k +1); // set in session list
		}
		// call into lua
		callRingAccessPointMethod("onSessionListReceived", 1, 0);
	}
	catch(const ELuaError &)
	{
		// no-op (error msg already printed at exception launch)
	}
}

// ****************************************************************************
void CSessionBrowserImpl::playerRatingFill(bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection)
{
	nlassert(_Lua);
	try
	{
		_Lua->newTable();
		CLuaObject scores(*_Lua);

		scores.setValue("ScenarioRated",			(double) scenarioRated);
		scores.setValue("RateFun",					(double) rateFun);
		scores.setValue("RateDifficulty",			(double) rateDifficulty);
		scores.setValue("RateAccessibility",		(double) rateAccessibility);
		scores.setValue("RateOriginality",			(double) rateOriginality);
		scores.setValue("RateDirection",			(double) rateDirection);

		scores.push();

		// call into lua
		callScenarioScoresMethod("onScenarioScoresReceived", 1, 0);
	}
	catch(const ELuaError &)
	{
		// no-op (error msg already printed at exception launch)
	}
}

// ****************************************************************************
void CSessionBrowserImpl::averageScoresFill(bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal)
{
	nlassert(_Lua);
	try
	{
		_Lua->newTable();
		CLuaObject scores(*_Lua);

		scores.setValue("ScenarioRated",			(double) scenarioRated);
		scores.setValue("RateFun",					(double) rateFun);
		scores.setValue("RateDifficulty",			(double) rateDifficulty);
		scores.setValue("RateAccessibility",		(double) rateAccessibility);
		scores.setValue("RateOriginality",			(double) rateOriginality);
		scores.setValue("RateDirection",			(double) rateDirection);
		scores.setValue("RRPTotal",					(double) rrpTotal);

		scores.push();

		// call into lua
		callScenarioScoresMethod("onAverageScoresReceived", 1, 0);
	}
	catch(const ELuaError &)
	{
		// no-op (error msg already printed at exception launch)
	}
}


// ****************************************************************************
static RSMGR::TSessionDesc buildSession(uint32 id, const std::string &owner, const std::string &title, const std::string &description, uint32 level, uint32 playerCount, const std::string &/* language */, uint32 launchTime, bool dm, bool invited)
{
	RSMGR::TSessionDesc result;
	result.setSessionId((TSessionId)id);
	result.setRequesterCharInvited(invited);
	result.setOwnerName(owner);
	result.setTitle(title);
	result.setDescription(description);
	result.setSessionLevel(R2::TSessionLevel((R2::TSessionLevel::TValues) level));
	result.setLaunchDate(launchTime);
	result.setNbConnectedPlayer(playerCount);
	result.setAnimMode(RSMGR::TAnimMode(dm ? RSMGR::TAnimMode::am_dm : RSMGR::TAnimMode::am_autonomous));
	return result;
}

// ****************************************************************************
void CSessionBrowserImpl::testFill()
{
	std::vector <RSMGR::TSessionDesc > sessions;
	uint32 refTime = NLMISC::CTime::getSecondsSince1970();
	sessions.push_back(buildSession(50, "_toto", "Scenar de toto", "Fight scenario", 0, 0, "en", refTime - 1000, true, true));
	sessions.push_back(buildSession(51, "_titi", "Titi's scenario", "Un peu de RP", 0, 4,  "en", refTime - 2000, false, true));
	sessions.push_back(buildSession(52, "_bob", "Yubo's back", "Chasse aux yubos", 0, 10, "en", refTime - 3000, true, false));
	sessions.push_back(buildSession(54, "_nico", "Nico test", "Scenario de test de nico", 1, 3, "fr", refTime - 10000, false, false));
	sessions.push_back(buildSession(55, "_toto2", "Scenar de toto", "Fight scenario", 1, 0, "de", refTime - 20000, true, true));
	sessions.push_back(buildSession(56, "_titi2", "Titi's scenario", "Un peu de RP", 1, 4, "it", refTime - 40000, true, true));
	sessions.push_back(buildSession(57, "_bob2", "Yubo's back", "Chasse aux yubos", 2, 10, "cz", refTime - 60000, true, true));
	sessions.push_back(buildSession(59, "_nico2", "Nico test", "Scenario de test de nico", 3, 3, "fr", refTime - 100000, true, true));
	sessions.push_back(buildSession(510, "_toto3", "Scenar de toto", "Fight scenario", 3, 0, "de", refTime - 200000, true, true));
	sessions.push_back(buildSession(511, "_titi3", "Titi's scenario", "Un peu de RP", 4, 4, "en", refTime - 300000, true, true));
	sessions.push_back(buildSession(512, "_bob3", "Yubo's back", "Chasse aux yubos", 4, 10, "fr", refTime - 500000, true, true));
	sessions.push_back(buildSession(514, "_nico3", "Nico test", "Scenario de test de nico", 5, 3, "en", refTime - 800000, true, true));
	sessions.push_back(buildSession(515, "_toto4", "Scenar de toto", "Fight scenario", 5, 0, "fr", refTime - 1000000, true, true));
	sessions.push_back(buildSession(516, "_titi'", "Titi's scenario", "Un peu de RP", 5, 4, "en", refTime - 2000000, true, true));
	sessions.push_back(buildSession(517, "_bob4", "Yubo's back", "Chasse aux yubos", 5, 10, "fr", refTime - 3000000, true, true));
	sessions.push_back(buildSession(519, "_nico4", "Nico test", "Scenario de test de nico", 5, 3, "en", refTime - 5000000, true, true));
	fill(sessions);
}

// ****************************************************************************
void CSessionBrowserImpl::on_charList(NLNET::TSockId /* from */, uint32 /* charId */, TSessionId /* sessionId */, const std::vector <RSMGR::TCharDesc > &chars)
{
	nldebug("SB: on_charList");
	charsFill(chars);
}

// ****************************************************************************
void CSessionBrowserImpl::on_playerRatings(NLNET::TSockId /* from */, uint32 /* charId */, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection)
{
	playerRatingFill(scenarioRated, rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection);
}

// ****************************************************************************
// Return average scores of a session
void CSessionBrowserImpl::on_sessionAverageScores(NLNET::TSockId /* from */, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal)
{
	averageScoresFill(scenarioRated, rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection, rrpTotal);
}

// ****************************************************************************
// Return average scores of a scenario
void CSessionBrowserImpl::on_scenarioAverageScores(NLNET::TSockId /* from */, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal)
{
	//averageScoresFill(scenarioRated, rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection, rrpTotal);
	nlassert(_Lua);
	try
	{
		_Lua->newTable();
		CLuaObject scores(*_Lua);

		scores.setValue("ScenarioRated",			(double) scenarioRated);
		scores.setValue("RateFun",					(double) rateFun);
		scores.setValue("RateDifficulty",			(double) rateDifficulty);
		scores.setValue("RateAccessibility",		(double) rateAccessibility);
		scores.setValue("RateOriginality",			(double) rateOriginality);
		scores.setValue("RateDirection",			(double) rateDirection);
		scores.setValue("RRPTotal",					(double) rrpTotal);

		scores.push();

		// call into lua
		callScenarioScoresMethod("onScenarioAverageScoresReceived", 1, 0);
	}
	catch(const ELuaError &)
	{
		// no-op (error msg already printed at exception launch)
	}
}

// ****************************************************************************
void CSessionBrowserImpl::on_ringRatings(NLNET::TSockId /* from */, uint32 /* charId */, uint32 authorRating, uint32 AMRating, uint32 masterlessRating)
{
	_LastAuthorRating = authorRating;
	_LastAMRating = AMRating;
	_LastMasterlessRating = masterlessRating;
	ringStatsFill();
}

// ****************************************************************************
void CSessionBrowserImpl::on_ringPoints(NLNET::TSockId /* from */, uint32 /* charId */, const std::string &ringPoints, const std::string &maxRingPoints)
{
	_LastRingPoints = ringPoints;
	_LastMaxRingPoints = maxRingPoints;
	ringStatsFill();
}

// ****************************************************************************
void CSessionBrowserImpl::charsFill(const std::vector <RSMGR::TCharDesc > &chars)
{
	// build datas & send to lua
	nlassert(_Lua);
	try
	{
		CLuaStackRestorer lsr(_Lua, _Lua->getTop());
		_Lua->newTable();
		for (uint k = 0; k < chars.size(); ++k)
		{
			const RSMGR::TCharDesc &cd = chars[k];
			if(cd.getCharId()!=getCharId())
			{
				_Lua->newTable();
				CLuaObject luaChar(*_Lua);
				luaChar.setValue("Id",			(double) cd.getCharId());
				luaChar.setValue("Char",		cd.getCharName());
				luaChar.setValue("Guild",		cd.getGuildName());
				luaChar.setValue("Race",		(double) cd.getRace().getValue());
				luaChar.setValue("Religion",	(double) cd.getCult().getValue());

				string shardName = toString(cd.getShardId());
				for(uint l = 0; l < Mainlands.size(); ++l)
				{
					if(Mainlands[l].Id.asInt() == cd.getShardId())
					{
						shardName = toString(Mainlands[l].Name);
						break;
					}
				}

				luaChar.setValue("Shard",		shardName);
				// note: we do 'level-1' because the TSessionLevel enum starts at 1
				luaChar.setValue("Level",		(double) (cd.getLevel().getValue()-1));
				/*
				uint32 flags = 0;
				if (cd.getConnected()) { flags += 1; }
				if (cd.getKicked()){ flags += 2; }
				*/
				uint32 flags = cd.getConnected();
				if (cd.getKicked()){ flags = 2; }

				luaChar.setValue("Flags",		(double) flags);
				luaChar.push();
 				_Lua->rawSetI(-2, k +1); // set in chars list
			}
		}
		// call into lua
		callRingCharTrackingMethod("onCharsListReceived", 1, 0);
	}
	catch(const ELuaError &)
	{
		// no-op (error msg already printed at exception launch)
	}
}

// ****************************************************************************

inline double ecoRingPoints(const std::string & ecoPoints, const char * c)
{
	std::string::size_type cPlace = ecoPoints.find(c);
	if(cPlace==string::npos)
		return 0;
	std::string::size_type sepPlace = ecoPoints.find(":", cPlace);
	std::string points = ecoPoints.substr(cPlace+1, sepPlace);
	double ret;
	fromString(points, ret);
	return ret;
}

void CSessionBrowserImpl::ringStatsFill()
{
	// build datas & send to lua
	nlassert(_Lua);
	try
	{
		CLuaStackRestorer lsr(_Lua, _Lua->getTop());
		_Lua->newTable();

		CLuaObject luaRingPoints(*_Lua);

		luaRingPoints.setValue("AuthorRating",		(double) _LastAuthorRating);
		luaRingPoints.setValue("AMRating",			(double) _LastAMRating);
		luaRingPoints.setValue("MasterlessRating",	(double) _LastMasterlessRating);

		luaRingPoints.setValue("MaxBasicRingPoints",	ecoRingPoints(_LastMaxRingPoints, "A"));
		luaRingPoints.setValue("BasicRingPoints",	    ecoRingPoints(_LastRingPoints, "A"));
		luaRingPoints.setValue("MaxDesertRingPoints",	ecoRingPoints(_LastMaxRingPoints, "D"));
		luaRingPoints.setValue("DesertRingPoints",		ecoRingPoints(_LastRingPoints, "D"));
		luaRingPoints.setValue("MaxForestRingPoints",	ecoRingPoints(_LastMaxRingPoints, "F"));
		luaRingPoints.setValue("ForestRingPoints",		ecoRingPoints(_LastRingPoints, "F"));
		luaRingPoints.setValue("MaxJungleRingPoints",	ecoRingPoints(_LastMaxRingPoints, "J"));
		luaRingPoints.setValue("JungleRingPoints",		ecoRingPoints(_LastRingPoints, "J"));
		luaRingPoints.setValue("MaxPrimeRootRingPoints",ecoRingPoints(_LastMaxRingPoints, "R"));
		luaRingPoints.setValue("PrimeRootRingPoints",	ecoRingPoints(_LastRingPoints, "P"));
		luaRingPoints.setValue("MaxSubtropicRingPoints",ecoRingPoints(_LastMaxRingPoints, "L"));
		luaRingPoints.setValue("SubtropicRingPoints",	ecoRingPoints(_LastRingPoints, "L"));

		luaRingPoints.push();

		// call into lua
		callRingPlayerInfoMethod("onRingStatsPlayerReceving", 1, 0);

		CSkillManager *pSM = CSkillManager::getInstance();
		if( pSM )
		{
			pSM->tryToUnblockTitleFromRingRatings( _LastAuthorRating, _LastAMRating, _LastMasterlessRating );
		}
	}
	catch(const ELuaError &)
	{
		// no-op (error msg already printed at exception launch)
	}
}

// ****************************************************************************
static RSMGR::TCharDesc buildChar(uint32 id, const std::string &charName, const std::string &guild,
									TRace race, TCult cult, uint32 shardId, TSessionLevel level, bool connected)
{
	RSMGR::TCharDesc result;
	result.setCharId(id);
	result.setShardId(shardId);
	result.setCharName(charName);
	result.setGuildName(guild);
	result.setRace(race);
	result.setCult(cult);
	result.setLevel(level);
	result.setConnected(connected);
	return result;
}

// ****************************************************************************
void CSessionBrowserImpl::testCharsFill()
{
	std::vector <RSMGR::TCharDesc > chars;
	chars.push_back(buildChar(50, "_toto", "Guild de toto",		TRace::r_fyros, TCult::c_kami,		242, TSessionLevel::sl_a, false));
	chars.push_back(buildChar(51, "_titi", "Titi's guild",		TRace::r_matis, TCult::c_karavan,	243, TSessionLevel::sl_b, true));
	chars.push_back(buildChar(52, "_bob",	"Yubo guild",		TRace::r_matis, TCult::c_karavan,	102, TSessionLevel::sl_c, false));
	chars.push_back(buildChar(54, "_nico",	"Nico guild",		TRace::r_matis, TCult::c_kami,		103, TSessionLevel::sl_d, false));
	chars.push_back(buildChar(55, "_toto2", "Guild de toto",	TRace::r_tryker, TCult::c_kami,		103, TSessionLevel::sl_e, true));
	chars.push_back(buildChar(56, "_titi2", "Titi's guild",		TRace::r_matis, TCult::c_karavan,	102, TSessionLevel::sl_f, true));
	chars.push_back(buildChar(57, "_bob2", "Yubo guild",		TRace::r_fyros, TCult::c_neutral,	101, TSessionLevel::sl_a, true));
	chars.push_back(buildChar(59, "_nico2", "Nico guild",		TRace::r_fyros, TCult::c_neutral,	101, TSessionLevel::sl_b, true));
	chars.push_back(buildChar(510, "_toto3", "Guild de toto",	TRace::r_tryker, TCult::c_karavan,	103, TSessionLevel::sl_c, false));
	chars.push_back(buildChar(511, "_titi3", "Titi's guild",	TRace::r_tryker, TCult::c_karavan,	102, TSessionLevel::sl_d, true));
	chars.push_back(buildChar(512, "_bob3", "Yubo guild",		TRace::r_zorai, TCult::c_karavan,	103, TSessionLevel::sl_e, false));
	chars.push_back(buildChar(514, "_nico3", "Nico guild",		TRace::r_zorai, TCult::c_kami,		102, TSessionLevel::sl_f, true));
	chars.push_back(buildChar(515, "_toto4", "Guild de toto",	TRace::r_zorai, TCult::c_karavan,	101, TSessionLevel::sl_a, true));
	chars.push_back(buildChar(516, "_titi'", "Titi's guild",	TRace::r_tryker, TCult::c_karavan,	102, TSessionLevel::sl_b, false));
	chars.push_back(buildChar(517, "_bob4", "Yubo guild",		TRace::r_fyros, TCult::c_neutral,	102, TSessionLevel::sl_c, false));
	chars.push_back(buildChar(519, "_nico4", "Nico guild",		TRace::r_fyros, TCult::c_kami,		103, TSessionLevel::sl_d, true));
	charsFill(chars);
}

// ****************************************************************************
void CSessionBrowserImpl::callRingAccessPointMethod(const char *name, int numArg, int numResult)
{
	// when you load an animation, lua state isn't initialized for a short time
	if(!_Lua) return;
	nlassert(name);
	{
		CLuaStackRestorer lsr(_Lua, _Lua->getTop() + numResult);
		_Lua->pushGlobalTable();
		CLuaObject rap(*_Lua);
		rap = rap["RingAccessPoint"];
		rap.callMethodByNameNoThrow(name, numArg, numResult);
	}
}

// ****************************************************************************
void CSessionBrowserImpl::callRingCharTrackingMethod(const char *name, int numArg, int numResult)
{
	// when you load an animation, lua state isn't initialized for a short time
	if(!_Lua) return;
	nlassert(name);
	{
		CLuaStackRestorer lsr(_Lua, _Lua->getTop() + numResult);
		_Lua->pushGlobalTable();
		CLuaObject rap(*_Lua);
		rap = rap["CharTracking"];
		rap.callMethodByNameNoThrow(name, numArg, numResult);
	}
}

// ****************************************************************************
void CSessionBrowserImpl::callRingPlayerInfoMethod(const char *name, int numArg, int numResult)
{
	// when you load an animation, lua state isn't initialized for a short time
	if(!_Lua) return;
	nlassert(name);
	{
		CLuaStackRestorer lsr(_Lua, _Lua->getTop() + numResult);
		_Lua->pushGlobalTable();
		CLuaObject rap(*_Lua);
		rap = rap["RingPlayerInfo"];
		rap.callMethodByNameNoThrow(name, numArg, numResult);
	}
}

// ****************************************************************************
void CSessionBrowserImpl::callScenarioScoresMethod(const char *name, int numArg, int numResult)
{
	// when you load an animation, lua state isn't initialized for a short time
	if(!_Lua) return;
	nlassert(name);
	{
		CLuaStackRestorer lsr(_Lua, _Lua->getTop() + numResult);
		_Lua->pushGlobalTable();
		CLuaObject rap(*_Lua);
		rap = rap["ScenarioScores"];
		rap.callMethodByNameNoThrow(name, numArg, numResult);
	}
}



