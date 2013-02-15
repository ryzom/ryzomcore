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
//
#include "tool_maintained_action.h"
#include "editor.h"
#include "../interface_v3/interface_manager.h"

namespace R2
{

// ***************************************************************
CToolMaintainedAction::CToolMaintainedAction()
{
	_State = Idle;
	_LeftButtonDown = false;
	_RightButtonDown = true;
	_PreviousToolClickEnd = false;
	_LostFocusRecover = false;
}

// ***************************************************************
CToolMaintainedAction::~CToolMaintainedAction()
{
//	getEditor().setFocusedInstance(NULL);
}

// ***************************************************************
void CToolMaintainedAction::cancel()
{
	//H_AUTO(R2_CToolMaintainedAction_cancel)
	if (_State == Action)
	{
		if (_TargetInstance)
		{
			cancelAction(*_TargetInstance);
		}
	}
	releaseMouse();
	_State = Idle;
}


// ***************************************************************
bool CToolMaintainedAction::onMouseLeftButtonDown()
{
	//H_AUTO(R2_CToolMaintainedAction_onMouseLeftButtonDown)
	CTool::TSmartPtr holdThis(this); // prevent deletion by son (if they set another tool)
									 // TODO nico : find a more general system than this, because
	if (_State == Action) return true; // may happen if focus was lost
	if (_State == Canceling)
	{
		releaseMouse();
		_State = Idle; // better for ergonomy (of if focus was lost, in which case we can't detect the button was up)
	}
	CInstance *instance = getEditor().getFocusedInstance();
	if (!instance) return false;
	_LeftButtonDown = true;
	//if (isValidInstanceForAction(*focusedInstance) return false;
	getEditor().setSelectedInstance(instance);
	if (!isActionPossibleOn(*instance))
	{
		_State = ActionNotPossible;
	}
	else
	{
		captureMouse();
		_TargetInstance = instance;
		beginAction(*instance);
		_State = Action;
	}
	return true;
}

// ***************************************************************
bool CToolMaintainedAction::onMouseLeftButtonUp()
{
	//H_AUTO(R2_CToolMaintainedAction_onMouseLeftButtonUp)
	bool mouseCaptured = isMouseCaptured();
	if (mouseCaptured)
	{
		releaseMouse();
	}
	if (_State == Idle)
	{
		// NB : we need to release the mouse here and signal the event as handled, because a previous tool such as
		// CToolDrawPrim terminates on a 'leftMouseDown' event then switch to the default 'SelectMove' tool
		// Not handling the event would trigger 'the mouseLeftClick' event which result in the default action 'unselect'
		return mouseCaptured;
	}
	if (_State == ActionNotPossible)
	{
		_State = Idle;
		return true;
	}
	_LeftButtonDown = false;
	if (_State == Canceling)
	{
		if (!_RightButtonDown) // both buttons must be released before mouse is released
		{
			releaseMouse();
			_State = Idle;
		}
		return true;
	}
	// from now the event is considered handled
	_State = Idle;
	if (!_TargetInstance) return true;
	commitAction(*_TargetInstance);
	return true;
}

// ***************************************************************
bool CToolMaintainedAction::onMouseRightButtonDown()
{
	//H_AUTO(R2_CToolMaintainedAction_onMouseRightButtonDown)
	if (_State == Canceling)
	{
		releaseMouse();

		_State = Idle; // better for ergonomy (if focus was lost, in which case we can't detect the button was up)
	}
	_LeftButtonDown = true;
	if (_State != Action)
	{
		return defaultRightButtonDownHandling();
	}
	if (_TargetInstance)
	{
		cancelAction(*_TargetInstance);
	}
	// Enter 'canceling' state : both button must be released before any other action can be done
	_State = Canceling;
	captureMouse();
	return true;
}

// ***************************************************************
bool CToolMaintainedAction::onMouseRightButtonUp()
{
	//H_AUTO(R2_CToolMaintainedAction_onMouseRightButtonUp)
	_LeftButtonDown = false;
	if (_State == Canceling)
	{
		if (!_LeftButtonDown) // both buttons must be released before before mouse is released
		{
			releaseMouse();
			_State = Idle;
		}
		return true;
	}
	return false;
}

// ***************************************************************
bool CToolMaintainedAction::onMouseLeftButtonClicked()
{
	//H_AUTO(R2_CToolMaintainedAction_onMouseLeftButtonClicked)
	// if this message reach us, then the user clicked somewhere in the view, so cancel the current
	// selection
	CGroupMap *gm = CTool::isMouseOnWorldMap();
	if (gm)
	{
		if (CWidgetManager::getInstance()->getCapturePointerLeft())
		{
			return false;
		}
	}
	if (!_PreviousToolClickEnd)
	{
		getEditor().setSelectedInstance(NULL);
	}
	_PreviousToolClickEnd = false;
	// special case for the minimap :
	// we say that the click has been handled only if mouse was already captured by the map
	return true;
}

// ***************************************************************
const char *CToolMaintainedAction::getDefaultCursor() const
{
	//H_AUTO(R2_CToolMaintainedAction_getDefaultCursor)
	return DEFAULT_CURSOR;
}

// ***************************************************************
const char *CToolMaintainedAction::getPickCursor() const
{
	//H_AUTO(R2_CToolMaintainedAction_getPickCursor)
	return "curs_pick.tga";
}

// ***************************************************************
void CToolMaintainedAction::updateBeforeRender()
{
	//H_AUTO(R2_CToolMaintainedAction_updateBeforeRender)
	// no-op
}


// ***************************************************************
void CToolMaintainedAction::onFocusGained()
{
	if (_State == Idle)
	{
		updateFocusedInstance(); // if alt-tab was used then udpdate focused instanceright now
		_LostFocusRecover = true;
	}
}

// ***************************************************************
bool CToolMaintainedAction::onMouseMove()
{
	if (_LostFocusRecover)
	{
		// If app focus was retrieved by a click in the scene, then
		// must update focused instance as a MouseMove is received, so that next 'mouse down' event can be on the right entity.
		// events sequence is :
		// -  focus gained
		// -  mouse move
		// - possibly 'mouse down' <- here focused instance needs to be good, hence the following code  !

		updateFocusedInstance();
		_LostFocusRecover = false;
	}
	return false;
}

// ***************************************************************
void CToolMaintainedAction::updateFocusedInstance()
{
	//handleMouseOverInstance(DEFAULT_CURSOR, , getCursorForPossibleAction());
	setMouseCursor(getDefaultCursor());
	CInstance *instanceUnder = checkInstanceUnderMouse();
	if (!instanceUnder)
	{
		getEditor().setFocusedInstance(NULL);
		return;
	}

	getEditor().setFocusedInstance(instanceUnder);
	if (instanceUnder != getEditor().getSelectedInstance())
	{
		setMouseCursor(isMouseOnWorldMap() ? "curs_default.tga" : getPickCursor()); // do not show the selection circle on the
																			   // map because it may cover other objects around
	}
	else
	{
		// if on the UI, just display the default cursor
		if (isMouseOnUI())
		{
			setMouseCursor("curs_default.tga");
		}
		else
		{
			if (isActionPossibleOn(*instanceUnder))
			{
				// indicate that the user can move the instance
				setMouseCursor(getCursorForPossibleAction());
			}
			else
			{
				setMouseCursor("curs_stop.tga");
			}
		}
	}
}

// ***************************************************************
void CToolMaintainedAction::updateAfterRender()
{
	//H_AUTO(R2_CToolMaintainedAction_updateAfterRender)
	switch(_State)
	{
		case Action:
		{
			if (_TargetInstance)
			{
				updateAction(*_TargetInstance);
			}
		}
		break;
		case ActionNotPossible:
		case Idle:
		{
			updateFocusedInstance();
		}
		break;
		case Canceling:
			setMouseCursor(DEFAULT_CURSOR); //\TODO nico Maybe a special cursor here would be more informative ?
		break;
	}
}

// ***************************************************************
bool CToolMaintainedAction::getPreviousToolClickEndFlag(bool clear /*=true*/)
{
	//H_AUTO(R2_CToolMaintainedAction_getPreviousToolClickEndFlag)
	bool flag = _PreviousToolClickEnd;
	if (clear)
	{
		_PreviousToolClickEnd = false;
	}
	return flag;
}



} // R2
