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

// The login and character methods of the control endpoint: login.login, chars.list, chars.play,
// client.reselect, client.quit. Each does what the screen's own button does, through its action
// handler, and returns at once: the state follows, and wait_for_state waits for it.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/gui/action_handler.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/widget_manager.h"

#include "connection.h"
#include "far_tp.h"
#include "game_share/character_summary.h"
#include "game_share/people_pd.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace NLGUI;

namespace ClientCtrl {

namespace {

const char *const s_LoginEditBox = "ui:login:checkpass:content:eb_login:eb";
const char *const s_PasswordEditBox = "ui:login:checkpass:content:eb_password:eb";

void methodLogin(CCtrlCall &call)
{
	if (!requireInterface(call) || !requireNoModal(call))
		return;
	CLoginStateMachine::TState state = LoginSM.getCurrentState();
	if (state != CLoginStateMachine::st_login)
	{
		call.fail("the login screen is not shown: the login state is " + CLoginStateMachine::toString(state));
		return;
	}
	const std::string &login = call.getArgs().get("login").getString();
	const std::string &password = call.getArgs().get("password").getString();
	if (login.empty())
	{
		call.fail("login is required");
		return;
	}
	CGroupEditBox *loginBox = dynamic_cast<CGroupEditBox *>(CWidgetManager::getInstance()->getElementFromId(s_LoginEditBox));
	CGroupEditBox *passwordBox = dynamic_cast<CGroupEditBox *>(CWidgetManager::getInstance()->getElementFromId(s_PasswordEditBox));
	if (loginBox == nullptr || passwordBox == nullptr)
	{
		call.fail(toString("the login screen has no %s or %s", s_LoginEditBox, s_PasswordEditBox));
		return;
	}
	// the fields and the button of the screen: what a player types and clicks
	loginBox->setInputString(login);
	passwordBox->setInputString(password);
	std::vector<std::string> shown;
	{
		CShownScope scope(shown);
		CAHManager::getInstance()->runActionHandler("on_login", nullptr, std::string());
	}
	setIdentity("account", login);
	CJsonValue r = CJsonValue::object();
	r.set("login", login);
	r.set("submitted", true);
	r.set("next", "wait for the state: shard_list or connecting on success; a message_box event and the login state on a refusal");
	CJsonValue shownList = CJsonValue::array();
	for (uint i = 0; i < shown.size(); ++i)
		shownList.push(CJsonValue(shown[i]));
	r.set("shown", shownList);
	call.reply(r);
}

CJsonValue characterToJson(uint slot, const CCharacterSummary &cs)
{
	CJsonValue c = CJsonValue::object();
	c.set("slot", slot);
	c.set("name", cs.Name.toUtf8());
	c.set("people", EGSPD::CPeople::toString(cs.People));
	c.set("mainland", cs.Mainland.asInt());
	c.set("location", cs.Location);
	c.set("in_newbieland", cs.InNewbieland);
	c.set("in_ring_session", cs.InRingSession);
	c.set("has_edit_session", cs.HasEditSession);
	return c;
}

void methodCharsList(CCtrlCall &call)
{
	CJsonValue list = CJsonValue::array();
	for (uint i = 0; i < CharacterSummaries.size(); ++i)
	{
		if (CharacterSummaries[i].People == EGSPD::CPeople::Unknown)
			continue;
		list.push(characterToJson(i, CharacterSummaries[i]));
	}
	CJsonValue r = CJsonValue::object();
	r.set("characters", list);
	r.set("slots", (uint32)CharacterSummaries.size());
	r.set("selected_slot", (uint32)PlayerSelectedSlot);
	call.reply(r);
}

/// The slot of a character by its slot number or its name; fails the call when there is none
bool findCharacter(CCtrlCall &call, uint &slot)
{
	const CJsonValue &args = call.getArgs();
	if (args.get("slot").isNumber())
	{
		slot = (uint)args.get("slot").getInt(0);
		if (slot < CharacterSummaries.size() && CharacterSummaries[slot].People != EGSPD::CPeople::Unknown)
			return true;
		call.fail(toString("there is no character in slot %u; chars.list lists them", slot));
		return false;
	}
	const std::string &name = args.get("name").getString();
	if (name.empty())
	{
		call.fail("slot or name is required");
		return false;
	}
	for (uint i = 0; i < CharacterSummaries.size(); ++i)
	{
		// the name with or without the shard it is shown with, Name(Mainland)
		std::string full = toLowerAscii(CharacterSummaries[i].Name.toUtf8());
		std::string bare = full.substr(0, full.find('('));
		if (CharacterSummaries[i].People != EGSPD::CPeople::Unknown && (full == toLowerAscii(name) || bare == toLowerAscii(name)))
		{
			slot = i;
			return true;
		}
	}
	call.fail(toString("there is no character named '%s'; chars.list lists them", name.c_str()));
	return false;
}

void methodCharsPlay(CCtrlCall &call)
{
	if (!requireInterface(call) || !requireNoModal(call))
		return;
	// the screen itself, waiting for the player: not a reconnection that picks the character again by itself
	if (getPhase() != "character_selection" || FarTP.isIngame() || CharacterSummaries.empty())
	{
		call.fail("the character selection is not shown (the state is " + getPhase() + ")");
		return;
	}
	uint slot;
	if (!findCharacter(call, slot))
		return;
	std::string mode = call.getArgs().get("mode").getString("play");
	// edit_mode of launch_game: 0 plays, 1 enters the edit session of the character, 2 a new scenario
	int editMode = mode == "play" ? 0 : (mode == "edit" ? 1 : (mode == "new_scenario" ? 2 : -1));
	if (editMode < 0)
	{
		call.fail("mode is play, edit or new_scenario");
		return;
	}
	std::vector<std::string> shown;
	{
		CShownScope scope(shown);
		CAHManager::getInstance()->runActionHandler("launch_game", nullptr, toString("slot=%u|edit_mode=%d", slot, editMode));
	}
	CJsonValue r = characterToJson(slot, CharacterSummaries[slot]);
	r.set("mode", mode);
	r.set("next", mode == "play" ? "wait for the state in_game" : "wait for the state in_game with the ring_mode of the detail");
	CJsonValue shownList = CJsonValue::array();
	for (uint i = 0; i < shown.size(); ++i)
		shownList.push(CJsonValue(shown[i]));
	r.set("shown", shownList);
	call.reply(r);
}

void methodReselect(CCtrlCall &call)
{
	if (!requireInterface(call) || !requireNoModal(call))
		return;
	if (!FarTP.isIngame() || FarTP.isFarTPInProgress())
	{
		call.fail("the character is not in the game");
		return;
	}
	bool limbo = call.getArgs().get("limbo").getBool(false);
	CAHManager::getInstance()->runActionHandler("reselect_character", nullptr, limbo ? "limbo=1" : std::string());
	CJsonValue r = CJsonValue::object();
	r.set("limbo", limbo);
	r.set("next", "wait for the state character_selection");
	call.reply(r);
}

void methodQuit(CCtrlCall &call)
{
	// what the quit button of the screen runs: on the login screens the login's own, elsewhere the immediate quit
	// every screen before the connection is the login's: its main loop does not read the game's exit flags
	bool atLogin = LoginSM.getCurrentState() != CLoginStateMachine::st_ingame;
	// the reason the exit gives, if it comes: nothing says the handler quits (a free trial asks first)
	setRequestedExitReason("a control tool asked the client to quit");
	CAHManager::getInstance()->runActionHandler(atLogin ? "login_quit" : "quit_ryzom_now", nullptr, std::string());
	CJsonValue r = CJsonValue::object();
	r.set("handler", atLogin ? "login_quit" : "quit_ryzom_now");
	r.set("next", "the endpoint closes when the client exits; list_instances gives the reason. A screen that asks first (a free trial) leaves it running");
	call.reply(r);
}

} // anonymous namespace

void registerLoginMethods(CCtrlEndpoint &endpoint)
{
	endpoint.addMethod(new CCtrlFunctionMethod("login.login",
	    "Logs in on the login screen with a login and password, as typing them and pressing Login does. Returns at once: the state goes on to shard_list or connecting, or a refusal "
	    "shows as a message_box event (2102 when the account is still online: wait about 30 s after its last session and try again). The nel_mcp tool login takes an account of its profile instead.",
	    CCtrlSchema().str("login", "The account login.", true).str("password", "Its password."), false, methodLogin));
	endpoint.addMethod(new CCtrlFunctionMethod("chars.list",
	    "The characters of the account, as the character selection has them: slot, name, people, home mainland (session id), location, newbieland, ring session, edit session.",
	    CCtrlSchema(), true, methodCharsList));
	endpoint.addMethod(new CCtrlFunctionMethod("chars.play",
	    "Plays a character from the character selection, as its PLAY button does, or enters the scenario editor with it (mode edit resumes its edit session, new_scenario starts a new one). Returns at once; wait for in_game.",
	    CCtrlSchema()
	        .integer("slot", "The slot of the character.")
	        .str("name", "Or its name.")
	        .choice("mode", "play|edit|new_scenario", "play (the default), edit, or new_scenario."),
	    false, methodCharsPlay));
	endpoint.addMethod(new CCtrlFunctionMethod("client.reselect",
	    "Returns from the game to the character selection, as the menu does; limbo keeps the same connection (a single shard). Returns at once; wait for character_selection.",
	    CCtrlSchema().boolean("limbo", "Stay on the same connection, without the shard unifier."), false, methodReselect));
	endpoint.addMethod(new CCtrlFunctionMethod("client.quit",
	    "Quits the client at once, as the quit buttons do (login_quit on the login screens, quit_ryzom_now elsewhere). The record of the instance keeps the reason.", CCtrlSchema(), false,
	    methodQuit));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_login.cpp */
