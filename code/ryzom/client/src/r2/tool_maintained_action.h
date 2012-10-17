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

#ifndef R2_TOOL_MAINTAINED_ACTION_H
#define R2_TOOL_MAINTAINED_ACTION_H


#include "tool.h"
#include "instance.h"

namespace R2
{

/** This class contained common behaviour for tools that follow the following mechanism :
  * - when the left bouton if maintained, an action if performerd  on highlighted instance
  *   if the instance wasn't the selection then it becomes the selection
  * - when the right button is pushed down while an action is performed then the action
  *   is canceled. The mouse is then captured until both right and left buttons are released
  * - when right button is pushed down outside of action then the context menu for current selected
  *   instance is shown. If the mouse was over an highlighted instance that is not the selection then
  *   it become the selection
  * - when right or left button are pushed then maintained and the mouse wasn't on an entity then
  *   the camera handling begins.
  */
class CToolMaintainedAction : public CTool
{
public:
	public:
	enum TState { Idle, Action, ActionNotPossible, Canceling };
	CToolMaintainedAction();
	virtual ~CToolMaintainedAction();

	// when another tool finishes with a (double) click, it may switch to the default select move tool
	// so the last click may be interpreted as a click "in scene", which does an unselect.
	// Call this after switching to prevent this behavior
	void markPreviousToolClickEnd() { _PreviousToolClickEnd = true; }

	virtual bool getPreviousToolClickEndFlag(bool clear = true);

protected:
	TState _State;
protected:
	///////////////////////////
	// INTERFACE TO DERIVERS //
	///////////////////////////

	virtual void beginAction(CInstance &instance) = 0;
	virtual void cancelAction(CInstance &instance) = 0;
	virtual void commitAction(CInstance &instance) = 0;
	virtual void updateAction(CInstance &instance) = 0;
	virtual const char *getCursorForPossibleAction() const = 0;
	virtual const char *getDefaultCursor() const;
	virtual const char *getPickCursor() const;
	virtual bool isActionPossibleOn(const CInstance &instance) const = 0;

	////////////////
	// from CTool //
	////////////////

	virtual void onFocusGained();
	virtual bool onMouseMove();
	virtual bool onMouseLeftButtonDown();
	virtual bool onMouseRightButtonDown();
	virtual bool onMouseLeftButtonUp();
	virtual bool onMouseRightButtonUp();
	virtual bool onMouseLeftButtonClicked();
	virtual void updateBeforeRender();
	virtual void updateAfterRender();
	virtual void cancel();



private:
	CInstance::TRefPtr _TargetInstance;
	bool _LeftButtonDown;
	bool _RightButtonDown;
	bool _PreviousToolClickEnd;
	bool _LostFocusRecover;
private:
	void updateFocusedInstance();
};






} // R2

#endif
