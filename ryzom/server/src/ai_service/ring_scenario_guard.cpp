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

#include "stdpch.h"
#include "ring_scenario_guard.h"

#include "ai_instance.h"
#include "ai_grp.h"
#include "state_instance.h"
#include "script_compiler.h"

#include "game_share/rt_scenario_check.h"

using namespace NLMISC;

namespace {

/// Refusals of one session that are logged in one minute; the others are counted for the session
const uint32 s_MaxRefusalsLoggedPerSessionMinute = 10;
/// Refusals of all sessions that are logged in one minute
const uint32 s_MaxRefusalsLoggedPerMinute = 100;
/// Refused scripts that are logged for one load of a scenario; the others are counted
const uint32 s_MaxScriptsLoggedPerLoad = 4;
/// Sessions whose count of refusals is kept; over this, the counts of the minutes that are over go
const uint32 s_MaxSessionsCounted = 256;

/// The refusals of one session in the minute that runs, or of all of them
struct CRefusalCount
{
	TTime WindowStart;
	uint32 Logged;
	uint32 NotLogged;

	CRefusalCount()
	    : WindowStart(0)
	    , Logged(0)
	    , NotLogged(0)
	{
	}

	/// True when a line can be logged now; it is counted, or counted as not logged
	bool take(TTime now, uint32 maxPerMinute)
	{
		if (now < WindowStart || now - WindowStart >= 60 * 1000)
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

typedef std::map<uint32, CRefusalCount> TSessionRefusals;
TSessionRefusals s_SessionRefusals;
CRefusalCount s_AllRefusals;

} // namespace

uint32 CRingScenarioGuard::s_LoadDepth = 0;
TSessionId CRingScenarioGuard::s_LoadSession;
uint32 CRingScenarioGuard::s_LoadInstance = 0;
uint32 CRingScenarioGuard::s_LoadRefused = 0;

// ****************************************************************************
TSessionId CRingScenarioGuard::getSession(const CStateInstance *caller)
{
	if (!caller)
		return TSessionId(0);
	CGroup *group = const_cast<CStateInstance *>(caller)->getGroup();
	if (!group)
		return TSessionId(0);
	const CAIInstance *instance = group->getAIInstance();
	return instance ? instance->getRingSession() : TSessionId(0);
}

// ****************************************************************************
bool CRingScenarioGuard::checkSession(const CAIInstance *instance, TSessionId sessionId, const char *what)
{
	TSessionId own = instance ? instance->getRingSession() : TSessionId(0);
	if (own.asInt() != 0 && own == sessionId)
		return true;

	logRefusal(own, instance ? instance->getInstanceNumber() : 0,
	    toString("%s was called for session %u, the caller is of %s", what, sessionId.asInt(),
	        own.asInt() != 0 ? "another session" : "an instance without a session"));
	return false;
}

// ****************************************************************************
bool CRingScenarioGuard::checkScriptSession(const CStateInstance *caller, float sessionArg, const char *what, TSessionId &session)
{
	CGroup *group = caller ? const_cast<CStateInstance *>(caller)->getGroup() : NULL;
	const CAIInstance *instance = group ? group->getAIInstance() : NULL;
	TSessionId own = instance ? instance->getRingSession() : TSessionId(0);

	// The number of the script is compared as the script holds it. What is passed on is the
	// session of the instance, never a number made from the one of the script.
	if (own.asInt() != 0 && (float)own.asInt() == sessionArg)
	{
		session = own;
		return true;
	}

	session = TSessionId(0);
	logRefusal(own, instance ? instance->getInstanceNumber() : 0,
	    toString("the group '%s' called %s for session %.0f, it is of %s", group ? group->getName().c_str() : "", what, (double)sessionArg,
	        own.asInt() != 0 ? "another session" : "an instance without a session"));
	return false;
}

// ****************************************************************************
bool CRingScenarioGuard::isScenarioFunction(const std::string &signature)
{
	// the list is the one the scenario service checks an upload with
	return R2::isRtScenarioScriptFunction(signature);
}

// ****************************************************************************
void CRingScenarioGuard::refuseFunction(const std::string &signature)
{
	++s_LoadRefused;
	if (s_LoadRefused <= s_MaxScriptsLoggedPerLoad)
	{
		logRefusal(s_LoadSession, s_LoadInstance,
		    toString("a script of the scenario calls %s, which is not a function of a scenario: the action that holds the script is left out", signature.c_str()));
	}
}

// ****************************************************************************
void CRingScenarioGuard::refuseScript(const std::string &name, const std::string &why)
{
	++s_LoadRefused;
	if (s_LoadRefused <= s_MaxScriptsLoggedPerLoad)
	{
		logRefusal(s_LoadSession, s_LoadInstance,
		    "the script '" + name + "' of the scenario " + why + ": the action that holds the script is left out");
	}
}

// ****************************************************************************
void CRingScenarioGuard::logRefusal(TSessionId sessionId, uint32 instanceNumber, const std::string &what)
{
	TTime now = CTime::getLocalTime();

	// The counts of the sessions that had no refusal for a minute are not kept for ever
	if (s_SessionRefusals.size() > s_MaxSessionsCounted)
	{
		TSessionRefusals::iterator it = s_SessionRefusals.begin();
		while (it != s_SessionRefusals.end())
		{
			TSessionRefusals::iterator current = it++;
			if (current->first != sessionId.asInt() && (now < current->second.WindowStart || now - current->second.WindowStart >= 60 * 1000))
			{
				if (current->second.NotLogged != 0)
					nlwarning("RING: session %u: %u more refusals were not logged", current->first, current->second.NotLogged);
				s_SessionRefusals.erase(current);
			}
		}
	}

	// A session has its own number of lines a minute, so that the refusals of one session leave the
	// lines of the others; all of them together have a larger one
	CRefusalCount &session = s_SessionRefusals[sessionId.asInt()];
	if (!session.take(now, s_MaxRefusalsLoggedPerSessionMinute))
		return;
	if (!s_AllRefusals.take(now, s_MaxRefusalsLoggedPerMinute))
	{
		// the line of the session is not written: it counts as not logged for the session
		--session.Logged;
		++session.NotLogged;
		return;
	}

	if (session.NotLogged != 0)
	{
		nlwarning("RING: session %u: %u more refusals were not logged", sessionId.asInt(), session.NotLogged);
		session.NotLogged = 0;
	}
	nlwarning("RING: session %u (AI instance %u) refused: %s", sessionId.asInt(), instanceNumber, what.c_str());
}

// ****************************************************************************
void CRingScenarioGuard::checkFunctionList()
{
	uint32 count;
	const char *const *functions = R2::getRtScenarioScriptFunctions(count);
	for (uint32 i = 0; i < count; ++i)
	{
		if (AICOMP::CCompiler::getFunctionList().find(CStringMapper::map(functions[i])) == AICOMP::CCompiler::getFunctionList().end())
		{
			nlwarning("RING: '%s' is in the list of the functions of a scenario and is not a native function of this service: the scripts of the editor that call it do not compile, correct the list in game_share/rt_scenario_check.cpp",
			    functions[i]);
		}
	}
}

// ****************************************************************************
CRingScenarioGuard::CLoadScope::CLoadScope(TSessionId sessionId, uint32 instanceNumber)
    : m_PreviousSession(s_LoadSession)
    , m_PreviousInstance(s_LoadInstance)
    , m_PreviousRefused(s_LoadRefused)
{
	++s_LoadDepth;
	s_LoadSession = sessionId;
	s_LoadInstance = instanceNumber;
	s_LoadRefused = 0;
}

// ****************************************************************************
CRingScenarioGuard::CLoadScope::~CLoadScope()
{
	if (s_LoadRefused > s_MaxScriptsLoggedPerLoad)
	{
		logRefusal(s_LoadSession, s_LoadInstance,
		    toString("%u more scripts that are not scripts of a scenario were left out in the same load", s_LoadRefused - s_MaxScriptsLoggedPerLoad));
	}
	--s_LoadDepth;
	s_LoadSession = m_PreviousSession;
	s_LoadInstance = m_PreviousInstance;
	s_LoadRefused = m_PreviousRefused;
}
