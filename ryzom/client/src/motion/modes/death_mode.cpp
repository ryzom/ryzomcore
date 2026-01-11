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
// INCLUDES //
//////////////
#include "stdpch.h"
// Client.
#include "../../input.h"
#include "../user_controls.h"
#include "../../user_entity.h"
#include "../../view.h"
#include "../../interface_v3/interface_manager.h"
#include "../../entities.h"


///////////
// USING //
///////////


////////////
// EXTERN //
////////////


///////////////
// Functions //
///////////////
//-----------------------------------------------
// deathModeStart :
// Manage interactions in interactive mode (start).
//-----------------------------------------------
void CUserControls::deathModeStart()
{
	// Mouse free look
	SetMouseCursor();
	// No more Velocity.
	UserEntity->frontVelocity(0);
	UserEntity->lateralVelocity(0);
	// No more autowalk
	_DirectionMove		= none;
	// Third person view but player dead, user is not selectable.
	UserEntity->selectable(false);
	_InternalView = false;
	// Show/hide all or parts of the user body (after _InternaView is set).
	UserEntity->updateVisualDisplay();
}// deathModeStart //

//-----------------------------------------------
// deathModeStop :
// Manage interactions in interactive mode (stop).
//-----------------------------------------------
void CUserControls::deathModeStop()
{
}// deathModeStop //

//-----------------------------------------------
// deathMode :
// Manage interactions in free head mode.
//-----------------------------------------------
void CUserControls::deathMode()
{
	// Set the view.
	View.view(CVector(0,0.01f,-1));
	View.viewPos(UserEntity->pos() + CVector(0,0,4));
}// deathMode //
