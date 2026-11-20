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
// handler, and returns at once with what the handler did: a refusal or a stop shows as the message
// box it opened, and the state follows otherwise, for wait_for_state to wait for.

#include "stdpch.h"

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/gui/action_handler.h"
#include "nel/gui/db_manager.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_modal.h"
#include "nel/gui/parser.h"
#include "nel/gui/view_text.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/cdb_leaf.h"

#include "connection.h"
#include "far_tp.h"
#include "game_share/character_summary.h"
#include "game_share/people_pd.h"
#include "interface_v3/action_handler_base.h"
#include "interface_v3/interface_manager.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace NLGUI;

// connection.cpp: a character was chosen, and the character selection waits for the server
extern bool WaitServerAnswer;

namespace ClientCtrl {

namespace {

const char *const s_LoginEditBox = "ui:login:checkpass:content:eb_login:eb";
const char *const s_PasswordEditBox = "ui:login:checkpass:content:eb_password:eb";

CJsonValue shownToJson(const std::vector<std::string> &shown)
{
	CJsonValue list = CJsonValue::array();
	for (uint i = 0; i < shown.size(); ++i)
		list.push(CJsonValue(shown[i]));
	return list;
}

/// The text of a message box (the view text of the box, :text)
std::string messageBoxText(CInterfaceGroup *box)
{
	CViewText *text = dynamic_cast<CViewText *>(findElement(box->getId() + ":text"));
	return text != nullptr ? text->getText() : std::string();
}

/// The number of a login error: the "(error code 61)" of the client, or the "(2102)" the login service ends its text
/// with; -1 for none
sint32 loginErrorCode(const std::string &text)
{
	sint32 code = -1;
	for (std::string::size_type open = text.find('('); open != std::string::npos; open = text.find('(', open + 1))
	{
		std::string::size_type start = open + 1;
		if (text.compare(start, 11, "error code ") == 0)
			start += 11;
		std::string::size_type end = start;
		while (end < text.size() && text[end] >= '0' && text[end] <= '9')
			++end;
		sint32 value;
		if (end > start && end < text.size() && text[end] == ')' && fromString(text.substr(start, end - start), value))
			code = value;
	}
	return code;
}

/// Closes the modal window when a click outside closes it (exit_click_out), as the click on the screen's button
/// would: its click out handler runs, then it goes. The id of what was closed, or empty.
std::string closeClickOutModal()
{
	CWidgetManager *wm = CWidgetManager::getInstance();
	if (!wm->hasModal())
		return std::string();
	// a copy: the pop removes it
	CWidgetManager::SModalWndInfo modal = wm->getModal();
	CInterfaceGroup *window = modal.ModalWindow;
	if (window == nullptr || !modal.ModalExitClickOut)
		return std::string();
	std::string id = window->getId();
	if (!modal.ModalHandlerClickOut.empty())
		CAHManager::getInstance()->runActionHandler(modal.ModalHandlerClickOut, nullptr, modal.ModalClickOutParams);
	CGroupModal *groupModal = dynamic_cast<CGroupModal *>(window);
	std::string postClickOut = groupModal != nullptr ? groupModal->OnPostClickOut : std::string();
	std::string postClickOutParams = groupModal != nullptr ? groupModal->OnPostClickOutParams : std::string();
	wm->popModalWindow();
	if (!postClickOut.empty())
		CAHManager::getInstance()->runActionHandler(postClickOut, nullptr, postClickOutParams);
	return id;
}

void methodLogin(CCtrlCall &call)
{
	if (!requireInterface(call))
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
	// the box of a refused login closes when the player clicks the Login button again
	std::string closed = closeClickOutModal();
	if (!requireNoModal(call))
		return;
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
	CJsonValue r = CJsonValue::object();
	r.set("login", login);
	if (!closed.empty())
		r.set("closed", closed);
	r.set("shown", shownToJson(shown));
	// the check of the login is done in the handler: a refusal opens a message box with the reason (tryLogin)
	CInterfaceGroup *box = CWidgetManager::getInstance()->getModalWindow();
	if (box != nullptr)
	{
		std::string text = messageBoxText(box);
		r.set("ok", false);
		r.set("error", text);
		sint32 code = loginErrorCode(text);
		r.set("code", code >= 0 ? CJsonValue(code) : CJsonValue());
		r.set("message_box", box->getId());
		r.set("next", code == 2102 ? "the account is still online: wait about 30 s after its last session and log in again" : "fix what the error says and log in again");
		call.reply(r);
		return;
	}
	setIdentity("account", login);
	r.set("ok", true);
	r.set("next", "wait for the state: shard_list or connecting");
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
		sint64 value = args.get("slot").getInt();
		if (value >= 0 && value < (sint64)CharacterSummaries.size() && CharacterSummaries[(size_t)value].People != EGSPD::CPeople::Unknown)
		{
			slot = (uint)value;
			return true;
		}
		call.fail(toString("there is no character in slot %d; chars.list lists them", (sint)value));
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

/// Fails the call and returns false when the screen shown is not the character selection, of the screens of the
/// character loop (UI:CURRENT_SCREEN: selection, creation, appearance...)
bool requireSelectionScreen(CCtrlCall &call)
{
	sint32 selectScreen;
	if (!fromString(CWidgetManager::getInstance()->getParser()->getDefine("screen_select"), selectScreen))
	{
		call.fail("the interface has no define screen_select (out_v2_config.xml): the screen shown cannot be told");
		return false;
	}
	CCDBNodeLeaf *screen = NLGUI::CDBManager::getInstance()->getDbProp("UI:CURRENT_SCREEN", false);
	if (screen == nullptr || screen->getValue32() != selectScreen)
	{
		call.fail(toString("the character selection screen is not shown (UI:CURRENT_SCREEN is %d, the selection is %d)", screen != nullptr ? screen->getValue32() : -1, selectScreen));
		return false;
	}
	return true;
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
	if (!requireSelectionScreen(call))
		return;
	// the screen's buttons are off from the first press to the answer of the server
	if (WaitServerAnswer)
	{
		call.fail("a character was chosen already and the client waits for the answer of the server; wait for the state");
		return;
	}
	uint slot;
	if (!findCharacter(call, slot))
		return;
	std::string mode = call.getArgs().get("mode").getString("play");
	// edit_mode of launch_game: 0 plays, 1 enters the edit session of the character, 2 a new scenario
	int editMode = mode == "play" ? 0 : (mode == "edit" ? 1 : 2);
	// the screen warns that a new scenario closes the edit session of the character before it starts one
	if (editMode == 2 && !call.getArgs().get("confirm").getBool(false))
	{
		call.fail("new_scenario closes the edit session of the character, which the screen asks to confirm first: confirm true");
		return;
	}
	std::vector<std::string> shown;
	{
		CShownScope scope(shown);
		// as the PLAY button: the buttons go off, then the launch (proc_charsel_play2)
		CAHManager::getInstance()->runActionHandler("proc", nullptr, "charsel_disable_buttons");
		CAHManager::getInstance()->runActionHandler("launch_game", nullptr, toString("slot=%u|edit_mode=%d", slot, editMode));
	}
	CJsonValue r = characterToJson(slot, CharacterSummaries[slot]);
	r.set("mode", mode);
	r.set("shown", shownToJson(shown));
	// a launch that stops says why in a message box (a patch the editor needs, an edit session that would not close)
	CInterfaceGroup *box = CWidgetManager::getInstance()->getModalWindow();
	if (box != nullptr)
	{
		r.set("ok", false);
		r.set("error", messageBoxText(box));
		r.set("message_box", box->getId());
		call.reply(r);
		return;
	}
	if (!WaitServerAnswer)
	{
		r.set("ok", false);
		r.set("error", "launch_game did not start the launch, and showed nothing: the log says why");
		call.reply(r);
		return;
	}
	r.set("ok", true);
	r.set("next", mode == "play" ? "wait for the state in_game" : "wait for the state in_game with the ring_mode of the detail");
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
	if (!requireInterface(call))
		return;
	// what the quit button of the screen shown runs: the login screens have their own, as their loop does not read the
	// game's exit flags; the character selection and creation screens quit_ryzom; the game the quit now of its quit
	// dialog. Another phase (a connection, a loading) has no quit button.
	const char *handler = nullptr;
	CInterfaceGroup *outgame = CWidgetManager::getInstance()->getMasterGroupFromId("ui:outgame");
	if (getPhase() == "login")
		handler = "login_quit";
	else if (CInterfaceManager::getInstance()->isInGame())
		handler = "quit_ryzom_now";
	else if (getPhase() == "character_selection" && outgame != nullptr && outgame->getActive())
		handler = "quit_ryzom";
	if (handler == nullptr)
	{
		call.fail("no screen of the client offers to quit in the phase " + getPhase() + ": wait for the state to settle");
		return;
	}
	// the reason the exit gives, if it comes
	setRequestedExitReason("a control tool asked the client to quit");
	std::vector<std::string> shown;
	{
		CShownScope scope(shown);
		CAHManager::getInstance()->runActionHandler(handler, nullptr, std::string());
	}
	// login_quit ends the login; the others set the exit flag, or ask the server first when connected (quit_ryzom),
	// or ask about the account first (a free trial)
	bool quitting = std::string(handler) == "login_quit" || game_exit;
	bool asked = !quitting && game_exit_request;
	CJsonValue r = CJsonValue::object();
	r.set("handler", handler);
	r.set("shown", shownToJson(shown));
	if (quitting)
	{
		r.set("quitting", true);
		r.set("next", "the endpoint closes when the client exits; list_instances gives the reason");
	}
	else if (asked)
	{
		r.set("quitting", true);
		r.set("next", "the client asked the server and exits once it agrees; the endpoint closes then");
	}
	else
	{
		// no exit follows: a later one is not this one
		setRequestedExitReason(std::string());
		r.set("quitting", false);
		r.set("next", "the client asks first (a free trial): ui.tree shows what it shows");
	}
	call.reply(r);
}

} // anonymous namespace

void registerLoginMethods(CCtrlEndpoint &endpoint)
{
	endpoint.addMethod(newMethod("login.login",
	    "Logs in on the login screen with a login and password, as typing them and pressing Login does; the box of an earlier refusal, which a click closes, closes first. "
	    "ok says whether the login service took them: on a refusal, error and code give the reason from its message box (2102 when the account is still online: wait about 30 s "
	    "after its last session and try again). The state goes on to shard_list or connecting otherwise. The nel_mcp tool login takes an account of its profile instead.",
	    CCtrlSchema().str("login", "The account login.", true).str("password", "Its password."), false, methodLogin, nullptr, nullptr));
	endpoint.addMethod(newMethod("chars.list",
	    "The characters of the account, as the character selection has them: slot, name, people, home mainland (session id), location, newbieland, ring session, edit session.",
	    CCtrlSchema(), true, methodCharsList, nullptr, nullptr));
	endpoint.addMethod(newMethod("chars.play",
	    "Plays a character from the character selection screen, as its PLAY button does, or enters the scenario editor with it (mode edit resumes its edit session, new_scenario "
	    "closes it and starts a new one, which takes confirm true as the screen's warning does). It fails while the client waits for the server's answer to a choice. ok false with "
	    "error when the launch stopped with a message box; otherwise wait for in_game.",
	    CCtrlSchema()
	        .integer("slot", "The slot of the character.")
	        .str("name", "Or its name.")
	        .choice("mode", "play|edit|new_scenario", "play (the default), edit, or new_scenario.")
	        .boolean("confirm", "For new_scenario: the edit session of the character is closed."),
	    false, methodCharsPlay, nullptr, nullptr));
	endpoint.addMethod(newMethod("client.reselect",
	    "Returns from the game to the character selection, as the menu does; limbo keeps the same connection (a single shard). Returns at once; wait for character_selection.",
	    CCtrlSchema().boolean("limbo", "Stay on the same connection, without the shard unifier."), false, methodReselect, nullptr, nullptr));
	endpoint.addMethod(newMethod("client.quit",
	    "Quits the client as the quit button of the screen shown does: login_quit on the login screens, quit_ryzom on the character screens (which asks the server first when "
	    "connected), the quit now of the game's quit dialog in the game. quitting says whether an exit follows; the record of the instance keeps the reason.",
	    CCtrlSchema(), false, methodQuit, nullptr, nullptr));
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_login.cpp */
