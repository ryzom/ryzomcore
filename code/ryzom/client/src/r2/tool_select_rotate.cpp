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
#include "editor.h"
#include "tool_select_rotate.h"
#include "tool_select_move.h"
//
#include "../net_manager.h"
#include "../motion/user_controls.h"
#include "../interface_v3/interface_manager.h"
#include "../entity_cl.h"
#include "../entities.h"
#include "displayer_visual.h"



using namespace NLMISC;

namespace R2
{

// ***************************************************************
CToolSelectRotate::CToolSelectRotate()
{
	_StartAngle = 0;
	_MouseStartX = -1;
	_State = Idle;
}


// ***************************************************************
void CToolSelectRotate::updateAction(CInstance &instance)
{
	//H_AUTO(R2_CToolSelectRotate_updateAction)
	CEntityCL *entity = instance.getEntity();
	if (entity)
	{
		setMouseCursor("r2ed_tool_rotating.tga");
		setEntityAngle(*entity, instance, (float) ((getMouseX() - _MouseStartX) * NLMISC::Pi / 180) + _StartAngle);
	}
}


// ***************************************************************
void CToolSelectRotate::setRotateInProgress(bool rotateInProgress, CInstance &instance)
{
	//H_AUTO(R2_CToolSelectRotate_setRotateInProgress)
	CDisplayerVisual *dv = instance.getDisplayerVisual();
	if (dv) dv->setRotateInProgress(rotateInProgress);
}

// ***************************************************************
void CToolSelectRotate::beginAction(CInstance &instance)
{
	//H_AUTO(R2_CToolSelectRotate_beginAction)
	_MouseStartX = getMouseX();
	_StartAngle = (float) instance.getObjectTable()->toNumber("Angle");
	setRotateInProgress(true, instance);
}

// ***************************************************************
void CToolSelectRotate::cancelAction(CInstance &instance)
{
	//H_AUTO(R2_CToolSelectRotate_cancelAction)
	CEntityCL *entity = instance.getEntity();
	nlassert(entity);
	getEditor().requestRollbackLocalNode(instance.getId(), "Angle");
	setRotateInProgress(false, instance);
}

// ***************************************************************
void CToolSelectRotate::commitAction(CInstance &instance)
{
	//H_AUTO(R2_CToolSelectRotate_commitAction)
	getDMC().newAction(CI18N::get("uiR2EDRotateAction") + instance.getDisplayName());
	// nothing to do, entity already has good angle
	getEditor().requestCommitLocalNode(instance.getId(), "Angle");
	setRotateInProgress(false, instance);
}

// ***************************************************************
void CToolSelectRotate::setEntityAngle(CEntityCL &/* entity */, CInstance &instance, float angle)
{
	//H_AUTO(R2_CToolSelectRotate_setEntityAngle)
	CObjectNumber *angleObject = new CObjectNumber(angle);
	getEditor().requestSetLocalNode(instance.getId(), "Angle", angleObject);
	delete angleObject;
}

// ***************************************************************
bool CToolSelectRotate::isActionPossibleOn(const CInstance &instance) const
{
	//H_AUTO(R2_CToolSelectRotate_isActionPossibleOn)
	CInstance &mutableInstance = const_cast<CInstance &>(instance);
	CDisplayerVisual *dv = mutableInstance.getDisplayerVisual();
	if (dv && dv->getActualDisplayMode() != CDisplayerVisual::DisplayModeVisible)
	{
		return false;
	}
	if (instance.getEntity() != NULL)
	{
		return !instance.getClass()["NameToProp"]["Angle"].isNil();
	}
	return false;
}

// ***************************************************************
bool CToolSelectRotate::onMouseLeftButtonDown()
{
	//H_AUTO(R2_CToolSelectRotate_onMouseLeftButtonDown)
	bool result = CToolMaintainedAction::onMouseLeftButtonDown();
	if (!result) return false;
	if (_State == ActionNotPossible)
	{
		CTool::TSmartPtr holder(this);
		cancel();
		// for ergonomy, switch to the 'move' tool
		getEditor().setCurrentTool(new CToolSelectMove);
		return getEditor().getCurrentTool()->onMouseLeftButtonDown();
	}
	return true;
}


/////////////////////
// ACTION HANDLERS //
/////////////////////

/**
  * Make the select/rotate tool current
  */
class CAHSelectRotate : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		getEditor().setCurrentTool(new CToolSelectRotate);
	}
};
REGISTER_ACTION_HANDLER(CAHSelectRotate, "r2ed_select_rotate");

} // R2
