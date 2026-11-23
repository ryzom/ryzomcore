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
#include "nel/misc/debug.h"
#include "nel/misc/time_nl.h"
#include "game_share/r2_basic_types.h"
#include "game_share/chat_group.h"
#include "game_share/utils.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

class CAIInstance;
class CGroup;
class CStateInstance;

/**
 * The lines of the log the sessions can cause in one minute. A session has its own number of lines
 * a minute, so that the lines of one session leave the lines of the others; all of them together
 * have a larger one. A line that is not written is counted for its session, and the count is given
 * with the next line of the session that is written.
 * It uses nothing of the service: the time is given by the caller, in milliseconds.
 */
class CRingLogBudget
{
public:
	/// A session whose count is no longer kept, and the lines of it that were not written
	typedef std::vector<std::pair<uint32, uint32> > TForgotten;

	/// maxSessionsCounted: over this number of sessions, the counts of the sessions that had no
	/// line for a minute are no longer kept
	CRingLogBudget(uint32 maxPerSessionMinute, uint32 maxPerMinute, uint32 maxSessionsCounted)
	    : m_MaxPerSessionMinute(maxPerSessionMinute)
	    , m_MaxPerMinute(maxPerMinute)
	    , m_MaxSessionsCounted(maxSessionsCounted)
	{
	}

	/// True when a line of the session can be written now. notLogged then receives the number of
	/// lines of the session that were not written since its last line that was, and that count
	/// begins again. False: the line is counted as not written.
	/// forgotten receives, after what it holds, the sessions whose count is no longer kept and that
	/// had lines that were not written.
	bool take(uint32 sessionId, NLMISC::TTime now, uint32 &notLogged, TForgotten &forgotten)
	{
		notLogged = 0;
		if (m_Sessions.size() > m_MaxSessionsCounted)
		{
			TSessions::iterator it = m_Sessions.begin();
			while (it != m_Sessions.end())
			{
				TSessions::iterator current = it++;
				if (current->first != sessionId && current->second.isOver(now))
				{
					if (current->second.NotLogged != 0)
						forgotten.push_back(std::make_pair(current->first, current->second.NotLogged));
					m_Sessions.erase(current);
				}
			}
		}

		CCount &session = m_Sessions[sessionId];
		if (!session.take(now, m_MaxPerSessionMinute))
			return false;
		if (!m_All.take(now, m_MaxPerMinute))
		{
			// the line of the session is not written: it counts as not logged for the session
			--session.Logged;
			++session.NotLogged;
			return false;
		}
		notLogged = session.NotLogged;
		session.NotLogged = 0;
		return true;
	}

	/// The number of sessions whose count is kept
	uint32 getSessionsCounted() const { return (uint32)m_Sessions.size(); }

private:
	/// The lines of one session in the minute that runs, or of all of them
	struct CCount
	{
		NLMISC::TTime WindowStart;
		uint32 Logged;
		uint32 NotLogged;

		CCount()
		    : WindowStart(0)
		    , Logged(0)
		    , NotLogged(0)
		{
		}

		/// True when the minute the count is of is over
		bool isOver(NLMISC::TTime now) const
		{
			return now < WindowStart || now - WindowStart >= 60 * 1000;
		}

		/// True when a line can be written now; it is counted, or counted as not logged
		bool take(NLMISC::TTime now, uint32 maxPerMinute)
		{
			if (isOver(now))
			{
				WindowStart = now;
				Logged = 0;
			}
			if (Logged >= maxPerMinute)
			{
				++NotLogged;
				return false;
			}
			++Logged;
			return true;
		}
	};

	typedef std::map<uint32, CCount> TSessions;

	uint32 m_MaxPerSessionMinute;
	uint32 m_MaxPerMinute;
	uint32 m_MaxSessionsCounted;
	TSessions m_Sessions;
	CCount m_All;
};

/**
 * The operations of the script machine the scripts of the scenario of one Ring session ran since
 * the update of the AI service began (CAIInstance holds one, CScenarioWorkScope counts in it).
 * The work one event starts has its own number of operations; this one is over all the events of
 * the session in one update, so that the number of events does not multiply what a session can
 * make the service do before the other sessions are updated. A native function a script calls is
 * counted by the operation that calls it, there is no second count: a call that sends a message
 * to another service or spawns a group is one operation, whatever it costs.
 * It uses nothing of the service.
 */
class CRingUpdateBudget
{
public:
	CRingUpdateBudget()
	    : m_Used(0)
	    , m_Reported(false)
	{
	}

	/// An update begins: nothing is used
	void reset()
	{
		m_Used = 0;
		m_Reported = false;
	}

	/// The operations a work that begins now is given: maxPerWork, or what the session has left of
	/// maxPerUpdate when that is less. maxPerUpdate 0: the update has no limit.
	uint32 grant(uint32 maxPerWork, uint32 maxPerUpdate) const
	{
		if (maxPerUpdate == 0)
			return maxPerWork;
		const uint32 left = m_Used < maxPerUpdate ? maxPerUpdate - m_Used : 0;
		return left < maxPerWork ? left : maxPerWork;
	}

	/// Counts the operations a work used. The count stays at the largest number it can hold.
	void use(uint32 opcodes)
	{
		m_Used = opcodes > 0xffffffffu - m_Used ? 0xffffffffu : m_Used + opcodes;
	}

	/// The operations used since the update began
	uint32 getUsed() const { return m_Used; }

	/// True the first time it is called in an update: the caller then writes the one line that says
	/// the session has no operation left
	bool reportOnce()
	{
		if (m_Reported)
			return false;
		m_Reported = true;
		return true;
	}

private:
	uint32 m_Used;
	bool m_Reported;
};

/// A text a scenario gives, as it is written in a line of the log: at most maxSize bytes of it, and
/// a '?' for each control character, so that it stays one line
inline std::string ringLogText(const std::string &text, size_t maxSize)
{
	const size_t size = text.size() < maxSize ? text.size() : maxSize;
	std::string out(text, 0, size);
	for (size_t i = 0; i < size; ++i)
	{
		if ((uint8)out[i] < 0x20 || (uint8)out[i] == 0x7f)
			out[i] = '?';
	}
	if (text.size() > size)
		out += "...";
	return out;
}

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

	/// Logs a refusal for the session whose scenario is loaded (isLoadingScenario)
	static void logLoadRefusal(const std::string &what);

	/// Logs a line the scenario of the session of this group causes while it runs: what its
	/// script prints, or what the service says about a value its script gave. False when the
	/// group is of no Ring session, and nothing is logged: the caller logs the line as the line
	/// of a shard it is. True for a group of a Ring session: the line is written with the session
	/// and the group, unless too many were in the last minute for this session, or for all of
	/// them; those are counted as the refusals are, in a count of their own, so that the lines
	/// of a scenario do not take the place of its refusals.
	static bool logForSession(const CStateInstance *caller, const std::string &text);
	/// The same for a line a group causes where its state instance is not at hand
	static bool logForGroup(const CGroup *group, const std::string &text);

	/// A bot of the group says a text of its session: sends the DSS what forwardToDss of
	/// game_share/send_chat.h sends it. That function writes a line at each call; here the line
	/// is a line of the session, in a count of its own, as a scenario that does nothing wrong has
	/// one for each sentence. False when the group is of no Ring session, and nothing is sent:
	/// the caller does what it does for a shard.
	static bool forwardToDss(const CGroup *group, const TDataSetRow &senderId, CChatGroup::TGroupType groupType, const std::string &sentenceId, uint32 scenarioId);

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

/// A warning of the service about what the script of a group gave a function, or did. For a group
/// of a Ring session it is a line of the session (CRingScenarioGuard::logForSession); for a group
/// of a shard it is the warning it was. caller is the state instance of the group, text a string.
#define RYAI_SCRIPT_WARNING(caller, text) \
	do \
	{ \
		const std::string &scriptWarningText_ = (text); \
		if (!CRingScenarioGuard::logForSession((caller), scriptWarningText_)) \
		{ \
			nlwarning("%s", scriptWarningText_.c_str()); \
		} \
	} while (false)

/// The same where the warning of a shard is followed by DEBUG_STOP, which stops a debug build of
/// the service and writes the call stack in the others: the group of a Ring session has the line
/// of its session, and neither of the two.
#define RYAI_SCRIPT_WARNING_STOP(caller, text) \
	do \
	{ \
		const std::string &scriptWarningText_ = (text); \
		if (!CRingScenarioGuard::logForSession((caller), scriptWarningText_)) \
		{ \
			nlwarning("%s", scriptWarningText_.c_str()); \
			DEBUG_STOP; \
		} \
	} while (false)

/// The same as RYAI_SCRIPT_WARNING where the state instance is not at hand: group is the CGroup
/// the warning is about.
#define RYAI_GROUP_WARNING(group, text) \
	do \
	{ \
		const std::string &scriptWarningText_ = (text); \
		if (!CRingScenarioGuard::logForGroup((group), scriptWarningText_)) \
		{ \
			nlwarning("%s", scriptWarningText_.c_str()); \
		} \
	} while (false)

#endif
