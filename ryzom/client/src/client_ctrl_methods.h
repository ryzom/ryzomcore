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

#ifndef CL_CLIENT_CTRL_METHODS_H
#define CL_CLIENT_CTRL_METHODS_H

#include "client_ctrl.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/misc/ctrl_endpoint.h"
#include "nel/misc/json.h"

namespace ClientCtrl {

/// The methods of the client, by group; each group adds its own
void registerMethods(NLMISC::CCtrlEndpoint &endpoint);
/// Before the endpoint goes
void releaseMethods();
/// A frame is about to be shown (the back buffer holds it)
void frameShown();

void registerScriptMethods(NLMISC::CCtrlEndpoint &endpoint);
void registerInputMethods(NLMISC::CCtrlEndpoint &endpoint);
void registerLoginMethods(NLMISC::CCtrlEndpoint &endpoint);
void registerScreenMethods(NLMISC::CCtrlEndpoint &endpoint);
void registerUiMethods(NLMISC::CCtrlEndpoint &endpoint);
void registerChatMethods(NLMISC::CCtrlEndpoint &endpoint);
void registerWorldMethods(NLMISC::CCtrlEndpoint &endpoint);
void registerDataMethods(NLMISC::CCtrlEndpoint &endpoint);
void registerGameMethods(NLMISC::CCtrlEndpoint &endpoint);
void registerNetMethods(NLMISC::CCtrlEndpoint &endpoint);

/// Runs a Lua chunk in the interface's Lua state with the items of args as its ..., and gives what it returns as
/// JSON (tables followed to maxDepth, maxItems in all), or the error with a traceback
bool runLua(const std::string &code, const NLMISC::CJsonValue &args, uint maxDepth, uint maxItems, NLMISC::CJsonValue &results, std::string &error,
    bool *truncated = nullptr);

/// The frame hooks of the screen methods: before the swap, and before the interface is drawn
void screenFrameShown();
void screenSceneRendered();
void releaseScreenMethods();
/// Reports the windows that opened or closed, and the modal window that changed, as window events
void trackWindows();
void releaseDataMethods();

// *** input, shared by the methods that act through the mouse and keyboard ***

/// Posts mouse events at a window pixel: move, move_held (a move with the button held), down, up, click,
/// double_click, wheel (steps). The modifiers come from ctrl, shift, alt of the arguments.
bool postMouse(const std::string &action, double px, double py, const std::string &button, const NLMISC::CJsonValue &modifierArgs, sint steps, std::string &error);
/// Posts a key press
bool postKey(const std::string &key, std::string &error);
/// Answers the call after the next pump of the events, once the client has handled what was posted; the
/// method's poll function is pollAfterPump
void finishAfterPump(NLMISC::CCtrlCall &call, const NLMISC::CJsonValue &result);
void pollAfterPump(NLMISC::CCtrlCall &call);

// *** helpers for the methods ***

/// Fails the call and returns false when the interface does not handle events in this phase
bool requireInterface(NLMISC::CCtrlCall &call);
/// Fails the call and returns false when the character is not in the game
bool requireInGame(NLMISC::CCtrlCall &call);
/// Fails the call and returns false while a modal window takes the input, which a button behind it would not get
bool requireNoModal(NLMISC::CCtrlCall &call);

/// The system messages shown while it lives go to the list too, so a method returns what it showed
class CShownScope
{
public:
	CShownScope(std::vector<std::string> &shown);
	~CShownScope();

private:
	std::vector<std::string> *m_Previous;
};

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

#endif // CL_CLIENT_CTRL_METHODS_H

/* End of client_ctrl_methods.h */
