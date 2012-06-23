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
#include "nel/3d/u_camera.h"
// Client.
#include "../../input.h"
#include "../user_controls.h"
#include "../../actions_client.h"
#include "../../user_entity.h"
#include "../../view.h"
#include "../../interface_v3/interface_manager.h"
#include "../../misc.h"
#include "../../entities.h"
#include "../../time_client.h"


///////////
// Using //
///////////
using namespace std;
using namespace NLMISC;
using namespace NL3D;


/////////////
// Externs //
/////////////
extern sint					CompassMode;
extern UCamera				MainCam;
extern CEventsListener		EventsListener;				// Inputs Manager


////////////
// GLOBAL //
////////////

///////////////
// Functions //
///////////////

//-----------------------------------------------
// thirdModeStart :
// Manage the Third Person View Mode (start).
//-----------------------------------------------
void CUserControls::thirdModeStart()
{
	// Third person view, user is selectable.
	UserEntity->selectable(true);
	// External View
	_InternalView = false;

	// Initialize the camera
	UserEntity->dir(UserEntity->front());

	// Update the mount direction.
	CEntityCL *mount = EntitiesMngr.entity(UserEntity->parent());
	if(mount)
	{
		mount->front(UserEntity->front());
		mount->dir(UserEntity->dir());
	}

	// Show/hide all or parts of the user body (after _InternaView is set).
	UserEntity->updateVisualDisplay();
}// thirdModeStart //

//-----------------------------------------------
// thirdModeStop :
// Manage the Third Person View Mode (stop).
//-----------------------------------------------
void CUserControls::thirdModeStop()
{
}// thirdModeStop //

//-----------------------------------------------
// thirdMode :
// Manage the Third Person View Mode.
//-----------------------------------------------
void CUserControls::thirdMode()
{
	// Manage common moves.
	commonMove();

	// Set the view
	View.viewPos(UserEntity->pos());
	commonSetView();

}// thirdMode //
