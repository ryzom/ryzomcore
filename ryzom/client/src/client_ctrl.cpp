// Ryzom - MMORPG Framework <https://wiki.ryzom.dev/>
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

#include "client_ctrl.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/misc/algo.h"
#include "nel/misc/cmd_args.h"
#include "nel/misc/ctrl_endpoint.h"
#include "nel/misc/json.h"
#include "nel/misc/path.h"

#include "client_ctrl_methods.h"
#include "connection.h"
#include "interface_v3/interface_manager.h"
#include "game_share/character_summary.h"
#include "game_share/people_pd.h"
#include "far_tp.h"
#include "login.h"
#include "init.h"
#include "misc.h"
#include "user_agent.h"
#include "r2/editor.h"

#include "config.h"

#include <cstdlib>

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

extern std::string ClientInstanceSuffix;

namespace ClientCtrl {

namespace {

CCtrlEndpoint *s_Endpoint = nullptr;
std::string s_Phase = "starting";
std::string s_ExitReason;
// the reason of an exit asked for, which the client may not carry out
std::string s_RequestedExitReason;
uint64 s_Frame = 0;
bool s_InterfaceAvailable = false;
// in update(): an update that comes then is one of a call that pumps the events
bool s_Updating = false;
// the loading screens being drawn, innermost last
uint s_LoadingDepth = 0;
std::string s_LoadingMessage;
float s_LoadingValue = 0.f;
// the text of a dialog of the system that holds the main thread (CDialogScope)
std::string s_Dialog;
// the system messages of the method being run (CShownScope)
std::vector<std::string> *s_Shown = nullptr;

const char *loginStateName(CLoginStateMachine::TState state)
{
	switch (state)
	{
	case CLoginStateMachine::st_start:
	case CLoginStateMachine::st_login:
	case CLoginStateMachine::st_auto_login:
	case CLoginStateMachine::st_alt_login:
		return "login";
	case CLoginStateMachine::st_shard_list:
		return "shard_list";
	case CLoginStateMachine::st_start_config:
		return "configurator";
	case CLoginStateMachine::st_scan_data:
	case CLoginStateMachine::st_check_patch:
	case CLoginStateMachine::st_display_cat:
	case CLoginStateMachine::st_patch:
	case CLoginStateMachine::st_reboot_screen:
	case CLoginStateMachine::st_restart_client:
		return "patching";
	case CLoginStateMachine::st_display_eula:
		return "eula";
	case CLoginStateMachine::st_browser_screen:
		return "browser";
	case CLoginStateMachine::st_create_account:
		return "create_account";
	case CLoginStateMachine::st_rate_session:
		return "rate_session";
	case CLoginStateMachine::st_close_client:
	case CLoginStateMachine::st_end:
		return "quitting";
	default:
		return nullptr;
	}
}

const char *ringModeName(R2::CEditor::TMode mode)
{
	switch (mode)
	{
	case R2::CEditor::NotInitialized: return nullptr;
	case R2::CEditor::EditionMode: return "edition";
	case R2::CEditor::GoingToDMMode: return "going_to_test";
	case R2::CEditor::GoingToEditionMode: return "going_to_edition";
	case R2::CEditor::TestMode: return "test";
	case R2::CEditor::DMMode: return "dm";
	case R2::CEditor::AnimationModeLoading: return "animation_loading";
	case R2::CEditor::AnimationModeWaitingForLoading: return "animation_waiting_for_loading";
	case R2::CEditor::AnimationModeDm: return "animation_dm";
	case R2::CEditor::AnimationModePlay: return "animation_play";
	case R2::CEditor::AnimationModeGoingToDm: return "animation_going_to_dm";
	case R2::CEditor::AnimationModeGoingToPlay: return "animation_going_to_play";
	default: return "unknown";
	}
}

/// The state of the client as a tool sees it, from the phase the loops set and what the client knows
std::string computeState(CJsonValue &detail)
{
	detail = CJsonValue::object();
	detail.set("phase", s_Phase);
	CLoginStateMachine::TState loginState = LoginSM.getCurrentState();
	detail.set("login_state", CLoginStateMachine::toString(loginState));
	detail.set("interface", s_InterfaceAvailable);

	std::string state = s_Phase;
	if (s_Phase == "login")
	{
		const char *name = loginStateName(loginState);
		if (name != nullptr)
			state = name;
	}
	else if (s_Phase == "character_selection")
	{
		// the screen comes before the list of the characters: chars.list is empty until they arrived
		uint characters = 0;
		for (uint i = 0; i < CharacterSummaries.size(); ++i)
			if (CharacterSummaries[i].People != EGSPD::CPeople::Unknown)
				++characters;
		detail.set("characters_received", !CharacterSummaries.empty());
		detail.set("characters", characters);
	}
	else if (s_Phase == "in_game")
	{
		if (FarTP.isFarTPInProgress())
			state = "far_tp";
		if (R2::getEditor().isInitialized())
		{
			const char *mode = ringModeName(R2::getEditor().getMode());
			if (mode != nullptr)
				detail.set("ring_mode", mode);
		}
	}
	if (!s_Dialog.empty())
		detail.set("dialog", s_Dialog);
	if (!s_ExitReason.empty())
	{
		state = "quitting";
		detail.set("exit_reason", s_ExitReason);
	}
	else if (!s_Dialog.empty())
	{
		state = "dialog";
	}
	else if (s_LoadingDepth > 0)
	{
		detail.set("loading", s_LoadingMessage);
		detail.set("progress", (double)((sint)(s_LoadingValue * 1000.f)) / 1000.0);
		state = "loading";
	}
	return state;
}

/// Whether the client stays in a state until the player or a tool acts: a wait for another state that ends up there
/// is over. The login screens that the client passes through on its own (an automatic login, a patch check, the
/// connection) are not, as are the loadings and the far TP. A dialog of the system waits for the user, wherever it is.
bool isSettled(const std::string &state)
{
	if (state == "dialog")
		return true;
	if (state == "starting" || state == "loading" || state == "connecting" || state == "entering_game" || state == "leaving_game" || state == "far_tp"
	    || state == "quitting")
		return false;
	if (s_Phase == "login")
	{
		switch (LoginSM.getCurrentState())
		{
		case CLoginStateMachine::st_start:
		case CLoginStateMachine::st_auto_login:
		case CLoginStateMachine::st_scan_data:
		case CLoginStateMachine::st_check_patch:
		case CLoginStateMachine::st_patch:
		case CLoginStateMachine::st_restart_client:
		case CLoginStateMachine::st_close_client:
		case CLoginStateMachine::st_connect:
		case CLoginStateMachine::st_end:
			return false;
		default:
			break;
		}
	}
	return true;
}

/// Gives the endpoint the state of the client now: a tool reads it from the channel thread, also while the main thread
/// does not come back to an update
void publishState()
{
	CJsonValue detail;
	std::string state = computeState(detail);
	s_Endpoint->setState(state, detail, isSettled(state));
}

void atExitRelease()
{
	// exit() from somewhere in the client: the reason set last, or none
	release(s_ExitReason.empty() ? std::string("exit() without a reason given") : s_ExitReason);
}

/// The endpoint was asked for on the command line, and whoever asked waits for it: the client does not run without
/// it. The log has the error for a tool, the dialog of the error for the user (the log, on another virtual desktop).
void stopForError(const std::string &message)
{
	nlwarning("CTRL: %s", message.c_str());
	ExitClientError("%s", message.c_str());
}

} // anonymous namespace

void addArgs(CCmdArgs &args)
{
	// the name is given with '=' only: the arguments after the option are the login and the password, which would be
	// taken for the name otherwise
	args.addLongArgWithEquals("ctrl-listen", "name",
	    "Listen on a local pipe for a control tool on this machine (nel_mcp), under this name; a '#' in it stands for the instance of the client in its directory (nothing for the "
	    "first one, then b, c... as in clientb.log). Whoever opens it acts as you in this client, and with --ctrl-trusted it runs Lua and commands in it too");
	// only the command line says it, as for --ctrl-listen: what the client shows cannot vouch for itself
	args.addArg("", "ctrl-trusted", "",
	    "Everything this client will show comes from trusted authors (a local shard with your own accounts), so a control tool may run Lua, action handlers, commands and database "
	    "writes in it; without it the client's content counts as untrusted and those methods are refused");
}

void start(const CCmdArgs &args)
{
	// a flag: --ctrl-trusted=0 would still be the flag, which is not what it says
	if (!args.getLongArg("ctrl-trusted").empty())
	{
		stopForError("--ctrl-trusted takes no value: give it alone for a client whose content is trusted, or leave it out");
		// ExitClientError() calls exit(), the code after it is never reached
		return;
	}
	if (!args.haveLongArg("ctrl-listen"))
	{
		if (args.haveLongArg("ctrl-trusted"))
			nlwarning("CTRL: --ctrl-trusted does nothing without the control endpoint: give --ctrl-listen=<name> too");
		return;
	}
	std::vector<std::string> values = args.getLongArg("ctrl-listen");
	if (values.empty())
	{
		stopForError("--ctrl-listen needs a name: --ctrl-listen=<name>");
		// ExitClientError() calls exit(), the code after it is never reached
		return;
	}
	// a launcher that may start several clients from one directory names each after its instance: client_dev#
	// gives client_dev, then client_devb... as their logs are client.log, clientb.log
	std::string name = strFindReplaceAll(values.front(), std::string("#"), ClientInstanceSuffix);

	CJsonValue identity = CJsonValue::object();
	identity.set("program", CPath::standardizePath(args.getProgramPath(), true) + args.getProgramName());
	identity.set("cwd", CPath::getCurrentPath());
	identity.set("version", getDebugVersion());
#ifdef REVISION
	identity.set("build", REVISION);
#endif
	identity.set("log", CPath::getFullPath(getLogDirectory(), true) + "client" + ClientInstanceSuffix + ".log");
	identity.set("instance", "client" + ClientInstanceSuffix);
	identity.set("kind", "ryzom_client");
	// the account of a login from the command line; the character follows once one is played
	if (!LoginLogin.empty())
		identity.set("account", LoginLogin);

	s_Endpoint = new CCtrlEndpoint();
	// what the client shows is written by the players and web pages of its shard unless the command line vouches for
	// them: the endpoint refuses its privileged methods then
	s_Endpoint->setTrusted(args.haveLongArg("ctrl-trusted"));
	std::string error;
	if (!s_Endpoint->start(name, identity, error))
	{
		delete s_Endpoint;
		s_Endpoint = nullptr;
		stopForError(toString("The control endpoint '%s' (--ctrl-listen) could not start: %s. The client does not run without the endpoint it was asked for: start it "
		                      "with another name (--ctrl-listen=<name>) or without the option",
		    name.c_str(), error.c_str()));
		// ExitClientError() calls exit(), the code after it is never reached
		return;
	}
	registerMethods(*s_Endpoint);
	atexit(atExitRelease);
	s_Endpoint->setState(s_Phase);
}

bool isActive()
{
	return s_Endpoint != nullptr;
}

bool isDriven()
{
	return s_Endpoint != nullptr && s_Endpoint->isLeaseHeld();
}

bool isDrivenUntrusted()
{
	return s_Endpoint != nullptr && !s_Endpoint->isTrusted() && s_Endpoint->isLeaseHeld();
}

bool refuseTypedCommand(const std::string &commandLine)
{
	if (!isDrivenUntrusted())
		return false;
	// the name as the chat reads it: up to the first space, or between quotes
	std::string name = commandLine.substr(0, commandLine.find(' '));
	if (!commandLine.empty() && commandLine[0] == '"')
	{
		std::string::size_type end = commandLine.find('"', 1);
		if (end != std::string::npos)
			name = commandLine.substr(1, end - 1);
	}
	nlwarning("CTRL: the command '%s' typed into the interface is not run: a control tool holds the lease of this client, whose content is not trusted (started without "
	          "--ctrl-trusted)",
	    name.c_str());
	CInterfaceManager::getInstance()->displaySystemInfo(name + ": not run while a control tool drives this client, whose content is not trusted");
	return true;
}

CCtrlEndpoint *getEndpoint()
{
	return s_Endpoint;
}

void update(bool interfaceAvailable)
{
	if (s_Endpoint == nullptr)
		return;
	// a call that runs into a loading is pumped again from inside it: once that pump is over, the call goes on with the
	// interface of its own update
	bool nested = s_Updating;
	bool interfaceBefore = s_InterfaceAvailable;
	s_Updating = true;
	s_InterfaceAvailable = interfaceAvailable;
	publishState();
	if (interfaceAvailable)
		trackWindows();
	// a loading screen pumps deep in other code: only what reads runs there, the rest waits for the loop
	s_Endpoint->update(interfaceAvailable);
	s_Updating = nested;
	if (nested)
		s_InterfaceAvailable = interfaceBefore;
}

bool isInterfaceAvailable()
{
	return s_InterfaceAvailable;
}

void beforeSwap()
{
	++s_Frame;
	if (s_Endpoint == nullptr)
		return;
	s_Endpoint->setFrame(s_Frame);
	frameShown();
}

void sceneRendered()
{
	if (s_Endpoint != nullptr)
		screenSceneRendered();
}

void setPhase(const char *phase)
{
	// the main loop sets its phase at every frame: only a change counts, and only with the endpoint
	if (s_Endpoint == nullptr || s_Phase == phase)
		return;
	s_Phase = phase;
	// the state follows at once, as the next update can be far (a loading that does not pump)
	publishState();
}

const std::string &getPhase()
{
	return s_Phase;
}

void setExitReason(const std::string &reason)
{
	s_ExitReason = reason;
	if (s_Endpoint != nullptr)
	{
		s_Endpoint->setExitReason(reason);
		s_Endpoint->addEvent("exit", CJsonValue::object().set("reason", reason));
		// the client is about to exit, most often without another update
		publishState();
	}
}

void setRequestedExitReason(const std::string &reason)
{
	s_RequestedExitReason = reason;
}

void release(const std::string &reason, bool generic)
{
	if (s_Endpoint == nullptr)
		return;
	releaseMethods();
	CCtrlEndpoint *endpoint = s_Endpoint;
	s_Endpoint = nullptr;
	// the reason given where the exit was decided says more than the place it ends at; a quit asked for by a tool
	// explains only the ordinary end, not a window closed or an error that came after it
	endpoint->stop(!s_ExitReason.empty() ? s_ExitReason : ((generic && !s_RequestedExitReason.empty()) ? s_RequestedExitReason : reason));
	delete endpoint;
}

void showSystemInfo(const std::string &category, const std::string &text)
{
	if (s_Endpoint == nullptr)
		return;
	s_Endpoint->captureOutput(text);
	if (s_Shown != nullptr)
		s_Shown->push_back(text);
	s_Endpoint->addEvent("system_info", CJsonValue::object().set("category", category).set("text", text));
}

CShownScope::CShownScope(std::vector<std::string> &shown)
    : m_Previous(s_Shown)
{
	s_Shown = &shown;
}

CShownScope::~CShownScope()
{
	s_Shown = m_Previous;
}

void addEvent(const std::string &kind, const CJsonValue &data)
{
	if (s_Endpoint != nullptr)
		s_Endpoint->addEvent(kind, data);
}

void setIdentity(const std::string &key, const std::string &value)
{
	if (s_Endpoint != nullptr)
		s_Endpoint->setIdentity(key, value);
}

CLoadingScope::CLoadingScope(const std::string &message, float value)
    : m_Counted(s_Endpoint != nullptr)
{
	// nobody reads it without the endpoint: the message is not copied at every progress call
	if (!m_Counted)
		return;
	++s_LoadingDepth;
	s_LoadingMessage = message;
	s_LoadingValue = value;
}

CLoadingScope::~CLoadingScope()
{
	if (m_Counted)
		--s_LoadingDepth;
}

CDialogScope::CDialogScope(const std::string &text)
    : m_Shown(s_Endpoint != nullptr)
{
	if (!m_Shown)
		return;
	m_Previous = s_Dialog;
	s_Dialog = text;
	s_Endpoint->addEvent("dialog", CJsonValue::object().set("text", text));
	// no update comes until the user closes it
	publishState();
}

CDialogScope::~CDialogScope()
{
	if (!m_Shown || s_Endpoint == nullptr)
		return;
	s_Dialog = m_Previous;
	publishState();
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl.cpp */
