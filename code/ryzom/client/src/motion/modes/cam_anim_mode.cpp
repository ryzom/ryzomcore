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
#include "camera_animation_manager/camera_animation_info.h"
#include "camera_animation_manager/camera_animation_player.h"


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
// camAnimModeStart :
// Manage interactions in interactive mode (start).
//-----------------------------------------------
void CUserControls::camAnimModeStart()
{
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
}// camAnimModeStart //

//-----------------------------------------------
// camAnimModeStop :
// Manage interactions in interactive mode (stop).
//-----------------------------------------------
void CUserControls::camAnimModeStop()
{
	UserEntity->frontVelocity(_CamAnimEntityFrontVelocity);
	UserEntity->lateralVelocity(_CamAnimEntityLateralVelocity);
}// camAnimModeStop //

//-----------------------------------------------
// camAnimMode :
// Manage interactions in free head mode.
//-----------------------------------------------
void CUserControls::camAnimMode()
{
	// Call the camera animation update function to update the view
	TCameraAnimationInfo newCamInfo = CCameraAnimationPlayer::getInstance()->update();

	// We normalize the look at direction
	newCamInfo.CamLookAtDir.normalize();

	// We udpate the view
	View.view(newCamInfo.CamLookAtDir);
	View.viewPos(newCamInfo.CamPos);
}// camAnimMode //
