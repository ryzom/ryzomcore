// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef RYAI_RING_SCENARIO_GUARD_H
#define RYAI_RING_SCENARIO_GUARD_H

#include "nel/misc/types_nl.h"
#include "game_share/r2_basic_types.h"

#include <string>

class CAIInstance;
class CStateInstance;

/**
 * What a scenario of a Ring session can reach in this service.
 *
 * The scenario of a session is made by a client: its groups, its events and the text of its scripts
 * are given to this service as they were uploaded. It is loaded in the AI instance of its session,
 * where the names its scripts use are looked up, so a script names the groups of its own session
 * only. Two things are not kept apart by the instance, and are kept apart here:
 * - the native functions. A script of a scenario calls the functions the editor writes scripts
 *   with. The others are for the scripts of the shard (commands of the service, files, players by
 *   their entity id, the data of other services) and are refused when a scenario is compiled.
 * - the session a function is called for. The functions that take the id of a session pass it on
 *   to the services that keep the sessions: it must be the session of the instance of the caller.
 *
 * A refusal is logged with the session and what was asked, a limited number of times per minute,
 * and what was asked is not done.
 */
class CRingScenarioGuard
{
public:
	/// The session whose scenario runs in the instance of the group of this state instance, 0 for none
	static TSessionId getSession(const CStateInstance *caller);

	/// True when the session a script names, a number of the script, is the session of the instance
	/// of the caller: session then receives that session, which is what is passed on (a number of a
	/// script does not hold every session id exactly). If not, the refusal is logged: 'what' is the
	/// function that was called, and it must do nothing.
	static bool checkScriptSession(const CStateInstance *caller, float sessionArg, const char *what, TSessionId &session);
	/// True when sessionId is the session of the instance. If not, the refusal is logged.
	static bool checkSession(const CAIInstance *instance, TSessionId sessionId, const char *what);

	/// True for the signature (name_in_out) of a native function a script of a scenario calls
	static bool isScenarioFunction(const std::string &signature);

	/// True while the scenario of a session is loaded: the scripts that are compiled are its scripts
	static bool isLoadingScenario() { return s_LoadDepth != 0; }

	/// Called by the compiler for a native function a script of the scenario that is loaded may
	/// not call
	static void refuseFunction(const std::string &signature);

	/// Called by the compiler for a script of the scenario that is loaded that is not what a script
	/// of a scenario is (R2::checkRtScenarioScript); why says what it has
	static void refuseScript(const std::string &name, const std::string &why);

	/// Logs a refusal, unless too many were logged in the last minute for this session, or for all
	/// of them: those are counted for the session, and the count is logged with its next refusal
	/// that is
	static void logRefusal(TSessionId sessionId, uint32 instanceNumber, const std::string &what);

	/// Warns about the functions of the list that are not native functions of the service
	static void checkFunctionList();

	/// Exists while the scenario of a session is loaded in its instance
	class CLoadScope
	{
	public:
		CLoadScope(TSessionId sessionId, uint32 instanceNumber);
		~CLoadScope();

	private:
		TSessionId m_PreviousSession;
		uint32 m_PreviousInstance;
		uint32 m_PreviousRefused;
	};

private:
	static uint32 s_LoadDepth;
	static TSessionId s_LoadSession;
	static uint32 s_LoadInstance;
	/// Functions refused since the load of the scenario began
	static uint32 s_LoadRefused;
};

#endif
