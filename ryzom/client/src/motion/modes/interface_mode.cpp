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




//////////////
// Includes //
//////////////
#include "stdpch.h"
// 3D Interface.
#include "nel/3d/u_driver.h"
// Client.
#include "../../input.h"
#include "../user_controls.h"
#include "../../actions_client.h"
#include "../../view.h"
#include "../../time_client.h"
#include "../../entities.h"
#include "../../interface_v3/interface_manager.h"
#include "../../cursor_functions.h"
#include "../../entities.h"
#include "../../misc.h"
#include "../../client_cfg.h"
#include "../../user_entity.h"


///////////
// Using //
///////////
using namespace std;
using namespace NLMISC;
using namespace NL3D;


/////////////
// Externs //
/////////////
extern UDriver				*Driver;
extern CEventsListener		EventsListener;				// Inputs Manager

///////////////
// Functions //
///////////////
//-----------------------------------------------
// interfaceModeStart :
// Manage interactions in interactive mode (start).
//-----------------------------------------------
void CUserControls::interfaceModeStart()
{
	SetMouseCursor ();

	// First person view, user is not selectable.
	UserEntity->selectable(false);

	_InternalView = true;

	// Show/hide all or parts of the user body.
	UserEntity->updateVisualDisplay();
}// interfaceModeStart //

//-----------------------------------------------
// interfaceModeStop :
// Manage interactions in interactive mode (stop).
//-----------------------------------------------
void CUserControls::interfaceModeStop()
{
}// interfaceModeStop //

//-----------------------------------------------
// interfaceMode :
// Manage interactions in free head mode.
//-----------------------------------------------
void CUserControls::interfaceMode()
{
	// Manage common moves.
	commonMove();

	// Set the view.
	View.viewPos(UserEntity->pos() + CVector(0,0,_ZOscil));
	commonSetView();

}// interfaceMode //
