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
#include "game_share/send_chat.h"

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
/// Lines the scenario of one session causes while it runs that are logged in one minute
const uint32 s_MaxLinesLoggedPerSessionMinute = 10;
/// Lines the scenarios of all sessions cause while they run that are logged in one minute
const uint32 s_MaxLinesLoggedPerMinute = 100;
/// Bytes of a text of a scenario, and of the name of its group, that are written in such a line
const size_t s_MaxLineTextSize = 512;
const size_t s_MaxLineNameSize = 64;

/// The refusals, the lines the scenarios cause while they run, and the lines of the sentences
/// their bots say: each has its own count
CRingLogBudget s_Refusals(s_MaxRefusalsLoggedPerSessionMinute, s_MaxRefusalsLoggedPerMinute, s_MaxSessionsCounted);
CRingLogBudget s_Lines(s_MaxLinesLoggedPerSessionMinute, s_MaxLinesLoggedPerMinute, s_MaxSessionsCounted);
CRingLogBudget s_Sentences(s_MaxLinesLoggedPerSessionMinute, s_MaxLinesLoggedPerMinute, s_MaxSessionsCounted);

/// Writes a line the scenario of the session of the group causes while it runs, when budget has
/// one for it. False when the group is of no Ring session: nothing is written or counted.
bool logLine(CRingLogBudget &budget, const CGroup *group, const std::string &text)
{
	const CAIInstance *instance = group ? group->getAIInstance() : NULL;
	const uint32 session = instance ? instance->getRingSession().asInt() : 0;
	if (session == 0)
		return false;

	uint32 notLogged;
	CRingLogBudget::TForgotten forgotten;
	const bool logged = budget.take(session, CTime::getLocalTime(), notLogged, forgotten);

	for (size_t i = 0; i < forgotten.size(); ++i)
		nlinfo("RING: session %u: %u more lines of its scenario were not logged", forgotten[i].first, forgotten[i].second);
	if (!logged)
		return true;

	if (notLogged != 0)
		nlinfo("RING: session %u: %u more lines of its scenario were not logged", session, notLogged);
	// The text and the name are given by the scenario: they are written as one line of a size
	nlinfo("RING: session %u (AI instance %u) group '%s': %s", session, instance->getInstanceNumber(),
	    ringLogText(group->getName(), s_MaxLineNameSize).c_str(), ringLogText(text, s_MaxLineTextSize).c_str());
	return true;
}

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
	// A session has its own number of lines a minute, so that the refusals of one session leave the
	// lines of the others; all of them together have a larger one (CRingLogBudget)
	uint32 notLogged;
	CRingLogBudget::TForgotten forgotten;
	const bool logged = s_Refusals.take(sessionId.asInt(), CTime::getLocalTime(), notLogged, forgotten);

	// The counts of the sessions that had no refusal for a minute are not kept for ever
	for (size_t i = 0; i < forgotten.size(); ++i)
		nlwarning("RING: session %u: %u more refusals were not logged", forgotten[i].first, forgotten[i].second);
	if (!logged)
		return;

	if (notLogged != 0)
		nlwarning("RING: session %u: %u more refusals were not logged", sessionId.asInt(), notLogged);
	nlwarning("RING: session %u (AI instance %u) refused: %s", sessionId.asInt(), instanceNumber, what.c_str());
}

// ****************************************************************************
void CRingScenarioGuard::logLoadRefusal(const std::string &what)
{
	logRefusal(s_LoadSession, s_LoadInstance, what);
}

// ****************************************************************************
bool CRingScenarioGuard::logForSession(const CStateInstance *caller, const std::string &text)
{
	// These lines are what a scenario says or does wrong for itself, not what it was refused: they
	// are lines for who looks at a session, and have a count of their own
	return caller && logLine(s_Lines, const_cast<CStateInstance *>(caller)->getGroup(), text);
}

// ****************************************************************************
bool CRingScenarioGuard::logForGroup(const CGroup *group, const std::string &text)
{
	return logLine(s_Lines, group, text);
}

// ****************************************************************************
bool CRingScenarioGuard::forwardToDss(const CGroup *group, const TDataSetRow &senderId, CChatGroup::TGroupType groupType, const std::string &sentenceId, uint32 scenarioId)
{
	if (!logLine(s_Sentences, group, "forwarding to DSS : id: " + sentenceId))
		return false;

	// the line of the sentence was written or counted above
	std::string sentence = sentenceId;
	::forwardToDss(senderId, groupType, sentence, scenarioId, false);
	return true;
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
