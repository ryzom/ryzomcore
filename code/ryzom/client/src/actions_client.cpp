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


/////////////
// INCLUDE //
/////////////
// 3D Interface.
#include "nel/3d/u_driver.h"
// Client.
#include "actions_client.h"
#include "interface_v3/input_handler_manager.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;


////////////
// EXTERN //
////////////
extern UDriver *Driver;


////////////
// GLOBAL //
////////////
CActionsContext	ActionsContext;			// Register action managers
CActionsManager Actions;				// Default actions Manager
CEventsListener			EventsListener;	// Inputs Manager

//-----------------------------------------------
// initActions :
// Initialize Actions.
//-----------------------------------------------
void initActions()
{
	/* Add the Actions listener to the Interface Manager Events Server, NOT TO THE DRIVER
		This is because the interfaceManager filter the events
	*/
	CInputHandlerManager	*pIH= CInputHandlerManager::getInstance();
	EventsListener.addToServer(pIH->FilteredEventServer);
}// initActions //

void uninitActions()
{
	/* Add the Actions listener to the Interface Manager Events Server, NOT TO THE DRIVER
		This is because the interfaceManager filter the events
	*/
	CInputHandlerManager	*pIH= CInputHandlerManager::getInstance();
	EventsListener.removeFromServer(pIH->FilteredEventServer);
	Actions.clear();
	EditActions.clear();
}// initActions //

