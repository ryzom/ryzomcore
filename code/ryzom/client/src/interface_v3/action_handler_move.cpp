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

using namespace std;
using namespace NLMISC;

#include "nel/gui/action_handler.h"
#include "../motion/user_controls.h"
#include "../view.h"
#include "../misc.h"
#include "../input.h"
#include "../client_cfg.h"
#include "../actions_client.h"
#include "../entities.h"
#include "interface_manager.h"
#include "action_handler_tools.h"
#include "nel/gui/ctrl_base_button.h"

////////////
// GLOBAL //
////////////
extern sint					CompassMode;
extern class CView			View;
extern CUserControls		UserControls;
extern bool					ShowInterface;


/**********************************************************************************************************
*																										  *
*										debug		handlers  actions									  *
*																										  *
***********************************************************************************************************/

// ------------------------------------------------------------------------------------------------
class CAHChangeCompassMode : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CompassMode = CompassMode ? 0:1;
	}
};
REGISTER_ACTION_HANDLER (CAHChangeCompassMode, "change_compass_mode");

// ------------------------------------------------------------------------------------------------
class CAHSetPos : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		UserEntity->pacsPos(View.viewPos());
		UserEntity->front(View.view());
		UserEntity->dir(View.view());
	}
};
REGISTER_ACTION_HANDLER (CAHSetPos, "set_pos");


// ----------------------------------------------
class CAHFrontSelection : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		if(!UserEntity)
			return;

		// select Enemy or enemy? Never select user
		uint	flags;
		if(Params=="friend")
			// for friend: select both dead and alive (for heal). But Select only Players (not bot)
			flags= CEntityFilterFlag::NotUser | CEntityFilterFlag::Player | CEntityFilterFlag::Friend;
		else
			// for enemy: interesting to select only Alive entities (to chain-nuke)
			// select both NonPlayer and Player (for PVP)
			flags= CEntityFilterFlag::NotUser | CEntityFilterFlag::Enemy | CEntityFilterFlag::Alive;

		// If there is an entity selected -> Set as the Target. Cycle with last target
		CEntityCL *entity = EntitiesMngr.getEntityInCamera(flags, ClientCfg.SpaceSelectionDist, UserEntity->targetSlot());
		if(entity)
		{
			// Select this entity.
			UserEntity->selection(entity->slot());
			// Yoyo: not interesting: commonly Fight (use shortcut instead)
			// Launch Context Menu.
			/*CInterfaceManager *IM = CInterfaceManager::getInstance();
			IM->launchContextMenuInGame("ui:interface:game_context_menu");*/
		}
	}
};
REGISTER_ACTION_HANDLER (CAHFrontSelection, "front_selection");

// ------------------------------------------------------------------------------------------------
class CAHMove : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Moving Break the Follow Mode
		UserEntity->disableFollow();
		UserEntity->moveTo(CLFECOMMON::INVALID_SLOT, 0.0, CUserEntity::None);
	}
};

// ------------------------------------------------------------------------------------------------
class CAHTurnLeft    : public CAHMove
{
};
REGISTER_ACTION_HANDLER (CAHTurnLeft, "turn_left");

// ------------------------------------------------------------------------------------------------
class CAHTurnRight   : public CAHMove
{
};
REGISTER_ACTION_HANDLER (CAHTurnRight, "turn_right");

// ------------------------------------------------------------------------------------------------
class CAHStrafeLeft  : public CAHMove
{
};
REGISTER_ACTION_HANDLER (CAHStrafeLeft, "strafe_left");

// ------------------------------------------------------------------------------------------------
class CAHStrafeRight : public CAHMove
{
};
REGISTER_ACTION_HANDLER (CAHStrafeRight, "strafe_right");

// ------------------------------------------------------------------------------------------------
class CAHForward     : public CAHMove
{
};
REGISTER_ACTION_HANDLER (CAHForward, "forward");

// ------------------------------------------------------------------------------------------------
class CAHBackward    : public CAHMove
{
};
REGISTER_ACTION_HANDLER (CAHBackward, "backward");

// ------------------------------------------------------------------------------------------------
class CAHToggleAutoWalk: public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
	}
};
REGISTER_ACTION_HANDLER (CAHToggleAutoWalk, "toggle_auto_walk");


// ------------------------------------------------------------------------------------------------
class CAHToggleLight: public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		UserEntity->light();
	}
};
REGISTER_ACTION_HANDLER (CAHToggleLight, "toggle_light");

// ------------------------------------------------------------------------------------------------
class CAHFreeMouse : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		InitMouseWithCursor (!IsMouseCursorHardware ());
		ClientCfg.HardwareCursor = IsMouseCursorHardware();
		// if the game config window is opened, keep in sync
		bool written = false;
		CInterfaceManager *im = CInterfaceManager::getInstance();
		if (im)
		{
			CInterfaceGroup *ig = dynamic_cast<CInterfaceGroup *>(CWidgetManager::getInstance()->getElementFromId("ui:interface:game_config"));
			if (ig && ig->getActive())
			{
				CInterfaceGroup *igHard = dynamic_cast<CInterfaceGroup *>(ig->getGroup("hard"));
				if (igHard)
				{
					CCtrlBaseButton *cbb = dynamic_cast<CCtrlBaseButton *>(igHard->getCtrl("c"));
					if (cbb)
					{
						if(cbb->getPushed() != IsMouseCursorHardware())
						{
							cbb->setPushed(IsMouseCursorHardware());
							cbb->runLeftClickAction();
							written = true;
						}
					}
				}
			}
		}
		if (!written)
		{
			ClientCfg.writeBool("HardwareCursor", IsMouseCursorHardware());
		}
	}
};
REGISTER_ACTION_HANDLER (CAHFreeMouse, "free_mouse");

// ------------------------------------------------------------------------------------------------
class CAHToggleCamera : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Change the camera view
		UserEntity->toggleCamera();
	}
};
REGISTER_ACTION_HANDLER (CAHToggleCamera, "toggle_camera");

// ------------------------------------------------------------------------------------------------
class CAHToggleNames : public IActionHandler
{
public:
	CAHToggleNames()
	{
		_Count = 0;
	}

	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CActionsManager *pAM = &Actions;
		// Key Up
		if(!pAM->valide(CAction::CName("toggle_names", Params.c_str())))
		{
			// Toggles Names back.
			if(_Count > 1)
				ClientCfg.Names = !ClientCfg.Names;
			_Count = 0;
		}
		// Key Down
		else
		{
			// First Time
			if(_Count == 0)
				ClientCfg.Names = !ClientCfg.Names;
			_Count++;
		}
	}
private:
	uint32 _Count;
};
REGISTER_ACTION_HANDLER (CAHToggleNames, "toggle_names");

// ------------------------------------------------------------------------------------------------
class CAHRearView : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		static bool PreviousShowInterface = true;

		CActionsManager *pAM = &Actions;
		// Key Down
		if(pAM->valide(CAction::CName("rear_view", Params.c_str())))
		{
			PreviousShowInterface = ShowInterface;		// save previous show interface value
			ShowInterface = false;
			View.rearView(true);
		}
		// Key Up
		else
		{
			ShowInterface = PreviousShowInterface;		// restore previous show interface value
			View.rearView(false);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHRearView, "rear_view");

// ------------------------------------------------------------------------------------------------
class CAHCameraUp : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
	}
};
REGISTER_ACTION_HANDLER (CAHCameraUp, "camera_up");
class CAHCameraDown : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
	}
};
REGISTER_ACTION_HANDLER (CAHCameraDown, "camera_down");
// ------------------------------------------------------------------------------------------------
class CAHCameraTurnLeft : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
	}
};
REGISTER_ACTION_HANDLER (CAHCameraTurnLeft, "camera_turn_left");

class CAHCameraTurnRight : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
	}
};
REGISTER_ACTION_HANDLER (CAHCameraTurnRight, "camera_turn_right");

class CAHCameraTurnCenter : public IActionHandler
{
public:
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		UserControls.resetSmoothCameraDeltaYaw();
	}
};
REGISTER_ACTION_HANDLER (CAHCameraTurnCenter, "camera_turn_center");


// ------------------------------------------------------------------------------------------------
// Toggle Sit / Stand, but don't change speed
class CAHToggleSitStand: public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		UserEntity->sit(!UserEntity->isSit());
	}
};
REGISTER_ACTION_HANDLER (CAHToggleSitStand, "toggle_sit_stand");

// Force sit, but don't change speed
class CAHForceSit: public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if(!UserEntity->isSit())
		{
			// disable afk mode
			UserEntity->setAFK(false);

			UserEntity->sit(true);
		}
	}
};
REGISTER_ACTION_HANDLER (CAHForceSit, "force_sit");

// Force stand, but don't change speed
class CAHForceStand: public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		if(UserEntity->isSit())
			UserEntity->sit(false);
	}
};
REGISTER_ACTION_HANDLER (CAHForceStand, "force_stand");


// ------------------------------------------------------------------------------------------------
// Toggle run/walk, but don't unsit
class CAHToggleRunWalk : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		UserEntity->switchVelocity();
	}
};
REGISTER_ACTION_HANDLER (CAHToggleRunWalk, "toggle_run_walk");

// force walk mode, and leave sit() mode if any
class CAHForceWalk : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// swith velocity?
		if(UserEntity->running())
			UserEntity->switchVelocity();

		// unsit?
		if(UserEntity->isSit())
			UserEntity->sit(false);

		// leave afk mode?
		if(UserEntity->isAFK())
			UserEntity->setAFK(false);
	}
};
REGISTER_ACTION_HANDLER (CAHForceWalk, "force_walk");

// force run mode, and leave sit() mode if any
class CAHForceRun : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// swith velocity?
		if(!UserEntity->running())
			UserEntity->switchVelocity();

		// unsit?
		if(UserEntity->isSit())
			UserEntity->sit(false);

		// leave afk mode?
		if(UserEntity->isAFK())
			UserEntity->setAFK(false);
	}
};
REGISTER_ACTION_HANDLER (CAHForceRun, "force_run");

// ------------------------------------------------------------------------------------------------
class CAHToggleDodgeParry : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		ucstring msg;
		// 0 - dodge mode
		// 1 - parry mode
		if (NLGUI::CDBManager::getInstance()->getDbProp("SERVER:DEFENSE:DEFENSE_MODE")->getValue32() == 0)
		{
			sendMsgToServer("COMBAT:PARRY");
			msg = CI18N::get("msgUserModeParry");
		}
		else
		{
			sendMsgToServer("COMBAT:DODGE");
			msg = CI18N::get("msgUserModeDodge");
		}
		// display dodge/parry mode message
		string cat = getStringCategory(msg, msg);
		pIM->displaySystemInfo(msg, cat);
	}
};
REGISTER_ACTION_HANDLER (CAHToggleDodgeParry, "toggle_dodge_parry");

