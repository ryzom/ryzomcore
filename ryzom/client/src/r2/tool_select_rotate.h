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

#ifndef R2_TOOL_SELECT_ROTATE_H
#define R2_TOOL_SELECT_ROTATE_H

#include "tool_maintained_action.h"

namespace R2
{

/**
  * Tool to select an element in the scene
  * If the left button is maintained down, then the selected element is moved
  */
class CToolSelectRotate : public CToolMaintainedAction
{
public:
	NLMISC_DECLARE_CLASS(R2::CToolSelectRotate);

	CToolSelectRotate();
	// from CTool
	virtual const char *getToolUIName() const { return "selectRotate"; }
	virtual bool  isCreationTool() const { return false; }
	virtual bool  isActionPossibleOn(const CInstance &instance) const;
protected:
	// from CToolMaintainedAction
	virtual bool onMouseLeftButtonDown();
	virtual void beginAction(CInstance &instance);
	virtual void cancelAction(CInstance &instance);
	virtual void commitAction(CInstance &instance);
	virtual void updateAction(CInstance &instance);
	virtual const char *getCursorForPossibleAction() const { return "r2ed_tool_can_rotate.tga"; }
private:
	float   _StartAngle;
	sint32  _MouseStartX;
private:
	void setRotateInProgress(bool rotateInProgress, CInstance &instance);
	// get orientation of an entity (in radians)
	static float getEntityAngle(CEntityCL &entity);
	// set orientation of an entity (in radians)
	static void  setEntityAngle(CEntityCL &entity, CInstance &instance, float angle);

};




} // R2

#endif
