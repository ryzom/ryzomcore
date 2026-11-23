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

#include "client_ctrl_methods.h"

#ifdef RYZOM_HAS_CLIENT_CTRL

#include "nel/misc/common.h"

#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

#include "far_tp.h"
#include "user_entity.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace ClientCtrl {

void registerMethods(CCtrlEndpoint &endpoint)
{
	registerScriptMethods(endpoint);
	registerInputMethods(endpoint);
	registerLoginMethods(endpoint);
	registerScreenMethods(endpoint);
	registerUiMethods(endpoint);
	registerChatMethods(endpoint);
	registerWorldMethods(endpoint);
	registerDataMethods(endpoint);
	registerGameMethods(endpoint);
	registerNetMethods(endpoint);
}

void releaseMethods()
{
	releaseScreenMethods();
	releaseDataMethods();
}

void frameShown()
{
	screenFrameShown();
}

bool requireInterface(CCtrlCall &call)
{
	if (isInterfaceAvailable())
		return true;
	call.fail("the interface is not available in this state (a loading screen); wait for the state to change");
	return false;
}

bool requireNoModal(CCtrlCall &call)
{
	NLGUI::CInterfaceGroup *modal = NLGUI::CWidgetManager::getInstance()->getModalWindow();
	if (modal == nullptr)
		return true;
	call.fail("a modal window takes the input: " + modal->getId() + "; close it first (ui.tree shows its buttons, ui.click presses one)",
	    CJsonValue::object().set("modal", modal->getId()));
	return false;
}

bool requireInGame(CCtrlCall &call)
{
	if (UserEntity == nullptr || !FarTP.isIngame() || FarTP.isFarTPInProgress())
	{
		call.fail("the character is not in the game");
		return false;
	}
	// a loading in the game (a teleport) pumps from the middle of changing the world: nothing of it is read there
	if (!isInterfaceAvailable())
	{
		call.fail("the character is in a loading (a teleport, a wait for the server); wait for the state to change");
		return false;
	}
	return true;
}

} // ClientCtrl

#endif // RYZOM_HAS_CLIENT_CTRL

/* End of client_ctrl_methods.cpp */
