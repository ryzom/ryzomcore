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
#include "tool_select_move.h"
#include "entity_sorter.h"
//
#include "../interface_v3/interface_manager.h"
#include "../client_cfg.h"
#include "../entities.h"
#include "../global.h"
#include "displayer_visual.h"
#include "../sheet_manager.h"

#include "dmc/com_lua_module.h"
//
#include "nel/misc/matrix.h"


using namespace NLMISC;


namespace R2
{

// ***************************************************************
CToolSelectMove::CToolSelectMove()
{
	_MouseX = -1;
	_MouseY = -1;
	_DeltaX = 0;
	_DeltaY = 0;
	_ValidPos = false;
	_State = Idle;
	_Moved = false;
	_Duplicating = false;
	_InitiallyAccessible = false;
	_InitiallyValidShape = true;
	_DeltaAnchor.set(0, 0, 0);
	_StartPos.set(0, 0, 0);
	_LastValidPos.set(0, 0, 0);
}


// ***************************************************************
bool CToolSelectMove::checkAdditionnalRoomLeftFor(CInstance &instance)
{
	//H_AUTO(R2_CToolSelectMove_checkAdditionnalRoomLeftFor)
	CLuaObject &luaProj = instance.getLuaProjection();
	CLuaState &ls = getEditor().getLua();
	CLuaStackRestorer lsr(&ls, 0);
	// check ai & static cost : if they are too big, can't create the duplicate
	if (!luaProj.callMethodByNameNoThrow("getAiCost", 0, 1)
		|| !ls.isNumber(-1))
	{
		return false;
	}
	uint aiCost = (uint) ls.toNumber(-1);
	ls.pop();
	if (!luaProj.callMethodByNameNoThrow("getStaticObjectCost", 0, 1))
	{
		return false;
	}
	uint staticCost = (uint) ls.toNumber(-1);
	ls.pop();
	if (!getEditor().verifyRoomLeft(aiCost, staticCost))
	{
		return false;
	}
	return true;
}


// ***************************************************************
CInstance *CToolSelectMove::createGhost(CInstance &instance)
{
	//H_AUTO(R2_CToolSelectMove_createGhost)
	CLuaState &ls = getEditor().getLua();
	// copy then do a local paste
	CLuaStackRestorer lsr(&ls, 0);
	//
	CLuaObject &luaProj = instance.getLuaProjection();
	CLuaObject &classDef = instance.getClass();
	if (luaProj.callMethodByNameNoThrow("copy", 0, 1))
	{
		// now we got a table that is an exact (canonical) copy of the original object, with the
		// same instance ids..
		// prepare for new insertion by renaming these instance id's (which 'newCopy' does)
		if (classDef["newCopy"].callNoThrow(1, 1))
		{
			// now, insert the new copy as a ghost in the new scene
			if (classDef["pasteGhost"].callNoThrow(1, 1))
			{

				CLuaObject ghost(ls); // pop the ghost from stack
				CInstance *newInst = getEditor().getInstanceFromId(ghost["InstanceId"].toString());
				if (newInst)
				{
					if (!newInst->getGhost())
					{
						nlwarning("When duplicating an object using the 'select/move' tool, temporary duplicate should be inserted \
								   as a ghost in the scene, removing object...");
						getEditor().getDMC().requestEraseNode(newInst->getId(), "", -1);
					}
					// set the flag so that the cost of this object isn't taken in account in the displayed quotas
					newInst->getLuaProjection()["User"].setValue("GhostDuplicate", true);
					getEditor().setSelectedInstance(newInst);
					newInst->getDisplayerVisual()->setDisplayFlag(CDisplayerVisual::FlagHideActivities, true);
					nlwarning("CToolSelectMove: beginning duplicate with instance with id %s", newInst->getId().c_str());
					// show in "frozen" state
					{
						/*CObjectNumber *numberValue = new CObjectNumber(2); // 2 = frozen state
						getEditor().getDMC().requestSetNode(newInst->getId(), "DisplayMode", numberValue);
						delete numberValue;
						*/
						newInst->getDisplayerVisual()->setDisplayMode(CDisplayerVisual::DisplayModeFrozen);
						getEditor().getEntitySorter()->clipEntitiesByDist();
						return newInst;
					}
				}
			}
		}
	}
	return NULL;
}

// ***************************************************************
void CToolSelectMove::beginAction(CInstance &instance)
{
	_DeltaAnchor.set(0, 0, 0);
	//H_AUTO(R2_CToolSelectMove_beginAction)
	_StartTime = T1;
	CDisplayerVisual *dv = instance.getDisplayerVisual();
	if(!dv)
	{
		cancel();
		return;
	}
	_InitiallyAccessible = dv->isAccessible();
	_InitiallyValidShape = true;
	CDisplayerVisual *parentDV = dv->getParent();
	if (parentDV && parentDV->isGroup())
	{
		_InitiallyAccessible = _InitiallyAccessible && parentDV->isAccessible();
		_InitiallyValidShape = parentDV->isValidShape();
	}
	//
	getMousePos(_MouseX, _MouseY);
	// must work with eval link point because world pos may be realy far from link pos (for regions)
	ISelectableObject::TSelectionType selectionType = dv->getSelectionType();
	switch(selectionType)
	{
		case ISelectableObject::GroundProjected:
		{
			// because displayed instance is projected on scene (like roads, regions ...)
			// anchor point is scene / mouse ray intersection point
			CTool::CWorldViewRay worldViewRay;
			computeWorldViewRay(_MouseX, _MouseY, worldViewRay);
			CVector inter;     // intersection of view ray with landscape
			_ValidPos = false;
			switch(computeLandscapeRayIntersection(worldViewRay, inter))
			{
				case NoIntersection:
						return;
				break;
				case ValidPacsPos:
				case InvalidPacsPos:
					_StartPos = inter;
				break;
				default:
					nlassert(0);
				break;
			}
		}
		break;
		case ISelectableObject::LocalSelectBox:
		case ISelectableObject::WorldSelectBox:
			dv->snapToGround();
			_StartPos = dv->evalLinkPoint(false);
		break;
		default:
			nlassert(0);
		break;
	}
	_DeltaAnchor = _StartPos - dv->getWorldPos();
	// nico : tmp fix to have correct z until luaSnapToGround is fixed
	if (instance.getEntity())
	{
		instance.getEntity()->snapToGround();
		_StartPos.z = instance.getEntity()->pos().z;
	}
	//nlwarning("starting to move instance at pos %s", NLMISC::toString(_StartPos.asVector()).c_str());
	const NL3D::CFrustum &fru = MainCam.getFrustum();
	CVector posInCam = MainCam.getMatrix().inverted() * _StartPos;
	CVector projectedPos = fru.projectZ(posInCam);
	/** Compute delta in screen space between instance position and the click position
	  * We maintain that delta when the instance moves
	  */
	uint32 w, h;
	getScreenSize(w, h);
	if (!isMouseOnWorldMap())
	{
		_DeltaX = _MouseX - (sint32) (w * projectedPos.x);
		_DeltaY = _MouseY - (sint32) (h * projectedPos.y);
	}
	else
	{
		_DeltaX = 0;
		_DeltaY = 0;
	}
	///////////////////////
	// TMP TMP TMP
	_PosRefX = (sint32) (w * projectedPos.x);
	_PosRefY = (sint32) (h * projectedPos.y);

	//
	/*
	CVector debugPos;
	CVector rayOrigin;
	CVector rayDir;
	computeWorldViewRay(_MouseX - _DeltaX, _MouseY - _DeltaY, rayOrigin, rayDir);
	computeLandscapeRayIntersection(rayOrigin, rayOrigin + 1000.f * rayDir, debugPos);
	*/
	//nlwarning("ref point = %s", NLMISC::toString(debugPos).c_str());
	//
	_RefX = _MouseX - _DeltaX;
	_RefY = _MouseY - _DeltaY;
	//
	_MouseRefX = _MouseX;
	_MouseRefY = _MouseY;
	///////////////////////
	_ValidPos = false;
	_Moved = false;

	if (isShiftDown())
	{
		// see in advance if there will be room left to create a duplicate for this object
		if (!checkAdditionnalRoomLeftFor(instance))
		{
			_State = Idle;
			getEditor().setCurrentTool(NULL);
			return;
		}
		_Duplicating = true;
		_ValidPos = true; // valid pos for first frame
		_FinalPos = _StartPos;
		_GhostInstance = NULL;
	}
	_LastValidPos = _StartPos;
	dv->setMoveInProgress(true);
}

// ***************************************************************
void CToolSelectMove::cancelAction(CInstance &instance)
{
	//H_AUTO(R2_CToolSelectMove_cancelAction)
	CDisplayerVisual *dv = instance.getDisplayerVisual();
	nlassert(dv);
	dv->setMoveInProgress(false);
	if (_Moved)
	{
		std::string posInstanceId = instance.getPosInstanceId();
		if (posInstanceId.empty()) return;
		getEditor().requestRollbackLocalNode(posInstanceId, "");
	}
	if (_Duplicating)
	{
		// remove the ghost node
		if (_GhostInstance)
		{
			nlassert(_GhostInstance->getGhost()); // should have been inserted as a ghost in the scene
			//
			getEditor().getDMC().requestEraseNode(_GhostInstance->getId(), "", -1);
		}
	}
	_Duplicating = false;
}

// ***************************************************************
void CToolSelectMove::commitAction(CInstance &instance)
{
	//H_AUTO(R2_CToolSelectMove_commitAction)
	CDisplayerVisual *dv = instance.getDisplayerVisual();
	nlassert(dv);
	dv->setMoveInProgress(false);
	if (!_ValidPos)
	{
		cancelAction(instance);
	}
	else
	{
		if (_Duplicating && _Moved)
		{

			if (!_GhostInstance)
			{
				cancelAction(instance);
				return;
			}
			if (!_GhostInstance->getGhost())
			{
				cancelAction(instance);
				return;
			}

			CLuaState &ls = getEditor().getLua();
			CLuaObject luaProj = _GhostInstance->getLuaProjection();
			if (!checkAdditionnalRoomLeftFor(*_GhostInstance))
			{
				cancelAction(instance);
				return;
			}


			CLuaStackRestorer lsr(&ls, ls.getTop());
			// duplicate the ghost, and insert in the scene for real
			CLuaObject classDef = instance.getClass();
			std::string oldName = luaProj["Name"].toString(); // keep name, because the newCopy call
														   // will generate a newone, causing indices to increase 2 by 2

			// restore default value for display mode
			//getEditor().getDMC().requestEraseNode(_GhostInstance->getId(), "DisplayMode", -1);
			if (_GhostInstance->getDisplayerVisual())
			{
				_GhostInstance->getDisplayerVisual()->setDisplayMode(CDisplayerVisual::DisplayModeVisible);
			}

			if (_Moved)
			{
				if (luaProj.callMethodByNameNoThrow("copy", 0, 1))
				{
					{
						std::string posInstanceId = instance.getPosInstanceId();
						if (!posInstanceId.empty())
						{
							getEditor().requestRollbackLocalNode(posInstanceId, ""); // Locally modified coords have
																					 // (of a local object ...)
																					 // have been read during the copy,
																					 // get rid of them...
						}
					}
					getDMC().newAction(CI18N::get("uiR2EDCopyAction") + _GhostInstance->getDisplayName());
					getEditor().getDMC().requestEraseNode(_GhostInstance->getId(), "", -1);
					// now, instanciate cannonical copy
					if (classDef["newCopy"].callNoThrow(1, 1))
					{
						// set real pos for the copy & the name
						CLuaObject newCopy(ls);
						CLuaObject pos = newCopy["Position"];
						try
						{
							newCopy.setValue("Name", oldName);
							pos.setValue("x", _FinalPos.x);
							pos.setValue("y", _FinalPos.y);
							pos.setValue("z", _FinalPos.z);
						}
						catch(const ELuaNotATable &)
						{
							nlwarning("Error while setting position of copied object");
						}
						// ... and paste for real
						if (_AutoGroup.getGroupingCandidate())
						{
							newCopy.push();
							std::auto_ptr<CObject> desc(CComLuaModule::getObjectFromLua(ls.getStatePointer()));
							_AutoGroup.group(desc.get(), _FinalPos);
						}
						else
						{
							newCopy.push();
							ls.push(false); // second parameter is for "not a new place"
							ls.push(instance.getId()); // last param give the original instance id of the object being copied
							classDef["paste"].callNoThrow(3, 1);
							getEditor().getDMC().getActionHistoric().endAction();
							getEditor().getDMC().flushActions();
						}
					}
				}
				else
				{
					std::string posInstanceId = instance.getPosInstanceId();
					if (!posInstanceId.empty())
					{
						getEditor().requestRollbackLocalNode(posInstanceId, ""); // Locally modified coords
																				 // (of a local object ...)
																				 // have been read during the copy,
																				 // get rid of them...
					}
				}
			}
			_Duplicating = false;
		}
		else
		if (_Moved)
		{
			if (!_ValidPos)
			{
				cancelAction(instance);
			}
			else
			{
				ucstring instanceName = instance.getDisplayName();
				if(instanceName == CI18N::get("uiR2EDNoName"))
					instanceName = ucstring(instance.getClassName());

				//getDMC().newAction(CI18N::get("uiR2EDMoveAction") + instance.getDisplayName());
				getDMC().newAction(CI18N::get("uiR2EDMoveAction") + instanceName);
				std::string posInstanceId = instance.getPosInstanceId();
				if (posInstanceId.empty()) return;
				std::string instanceId = instance.getObjectTable()->getAttr("InstanceId")->toString();
				R2::getEditor().getLua().push(instanceId);
				R2::getEditor().callEnvFunc( "checkLeaderDistAndUngroup", 1, 0);
				getEditor().requestCommitLocalNode(posInstanceId, "");
			}
		}
		else
		{
			cancelAction(instance);
		}
	}
}

// ***************************************************************
const char *CToolSelectMove::getCursorForPossibleAction() const
{
	//H_AUTO(R2_CToolSelectMove_getCursorForPossibleAction)
	return isShiftDown() ? "curs_can_pan_dup.tga" : "curs_can_pan.tga";
}

// ***************************************************************
const char *CToolSelectMove::getDefaultCursor() const
{
	//H_AUTO(R2_CToolSelectMove_getDefaultCursor)
	if (isMouseOnUI() && !isMouseOnWorldMap()) return DEFAULT_CURSOR;
	return isShiftDown() ? "curs_dup.tga" : DEFAULT_CURSOR;
}

// ***************************************************************
const char *CToolSelectMove::getPickCursor() const
{
	//H_AUTO(R2_CToolSelectMove_getPickCursor)
	return isShiftDown() ? "curs_pick_dup.tga" : "curs_pick.tga";
}


// ***************************************************************
void CToolSelectMove::updateAction(CInstance &instance)
{
	//H_AUTO(R2_CToolSelectMove_updateAction)
	CDisplayerVisual *vd = _GhostInstance ? _GhostInstance->getDisplayerVisual() : instance.getDisplayerVisual();
	if(!vd) return;
	setMouseCursor(_Duplicating ? "curs_pan_dup.tga" : "curs_pan.tga");
	sint32 mx, my;
	getMousePos(mx, my);
	sint32 autoPanDx, autoPanDy;
	handleWorldMapAutoPan(autoPanDx, autoPanDy);
	CDisplayerVisual *vdParent = vd->getParent();
	if (mx != _MouseX || my != _MouseY || autoPanDx || autoPanDy)
	{
		if (!autoPanDx && !autoPanDy)
		{
			const sint32 moveThreshold = 2;
			const sint32 timeThreshold = 300; // if some time elapsed, then do the move
			if (!_Moved && abs(mx - _MouseRefX) < moveThreshold && abs(my - _MouseRefY) < moveThreshold &&
				 (T1 - _StartTime) < timeThreshold
			   )
			{
				// discard small displacement when clicking (move is unwanted most of the time)
				return;
			}
		}

		if (_Duplicating && !_GhostInstance)
		{
			_GhostInstance = createGhost(instance);
			if (!_GhostInstance)
			{
				_State = Idle;
				getEditor().setCurrentTool(NULL);
				return;
			}
			vd = _GhostInstance->getDisplayerVisual();
			if (!vd) return;
			vdParent = vd->getParent();
		}
		//
		_Moved = true;
		// mouse has moved
		// TODO : onMouseMove msg
		// update mouse pos
		_MouseX = mx;
		_MouseY = my;
		//
		CTool::CWorldViewRay worldViewRay;
		//
		_ValidPos = true;
		//
		if (!isMouseOnWorldMap())
		{
			if (isMouseOnUI())
			{
				_FinalPos = _LastValidPos;
				_ValidPos = false;
			}
			else
			{
				computeWorldViewRay(mx - _DeltaX, my - _DeltaY, worldViewRay);
			}
		}
		else
		{
			computeWorldViewRay(mx, my, worldViewRay);
			if (!worldViewRay.OnMiniMap)
			{
				_FinalPos = _LastValidPos;
				_ValidPos = false;
			}
		}
		if (_ValidPos)
		{
			CVector inter;     // intersection of view ray with landscape
			_ValidPos = false;
			switch(computeLandscapeRayIntersection(worldViewRay, inter))
			{
				case NoIntersection:
					// no collision, can't drop instance, so let it at its start position
					_FinalPos = _LastValidPos;
					_ValidPos = false;
				break;
				case ValidPacsPos:
					//nlwarning("moving instance at pos %s", NLMISC::toString(inter).c_str());
					_FinalPos = inter;
					_ValidPos = isValid2DPos(inter);
					if (_ValidPos)
					{
						_LastValidPos = _FinalPos;
					}

				break;
				case InvalidPacsPos:
					// If the object moved is a region or a group, invalid pacs pos
					// may still be acceptable as long as an intersection was found
					if (vd->isInvalidPacsPosAcceptable())
					{
						_FinalPos = inter;
						_ValidPos = true; // good pos to drop instance
					}
					else
					{
						_FinalPos = inter;
						_ValidPos = false;
					}
				break;
				default:
					nlassert(0);
				break;
			}
		}
		_FinalPos = _FinalPos - _DeltaAnchor;
		// make final pos relative to parent
		if (!_Duplicating && vd->inheritPos() && instance.getParent())
		{
			if (vdParent)
			{
				_FinalPos = _FinalPos - vdParent->getWorldPos();
			}
		}
		//
		setInstancePos(_FinalPos, instance);
	}
	if (_InitiallyAccessible)
	{
		// NB : do not do the test if initially inaccessible to allow user to go to an 'accessible' state more easily
		if (!vd->isAccessible())
		{
			_ValidPos = false;
		}
		if (vdParent && !vdParent->isAccessible())
		{
			_ValidPos = false;
		}
	}
	//
	if (_InitiallyValidShape)
	{
		// is started from an invalid shape, allow to use intermediate invalid shapes to correct it
		if (vdParent && vdParent->isGroup() && !vdParent->isValidShape())
		{
			_ValidPos = false;
		}
	}

	if (_Moved && _ValidPos)
	{
		std::string instanceId = instance.getObjectTable()->getAttr("InstanceId")->toString();
		R2::getEditor().getLua().push(instanceId);
		R2::getEditor().callEnvFunc( "checkGroupDistance", 1, 1);
		if (!R2::getEditor().getLua().isBoolean(-1))
		{
			nlassert(0 && "checkGroupDistance return wrong type");
		}
		bool positionOk = R2::getEditor().getLua().toBoolean(-1);
		R2::getEditor().getLua().pop();
		_ValidPos = positionOk;
	}


	//
	if (!_ValidPos && _Moved)
	{
		setMouseCursor("curs_stop.tga");
	}
	// TMP TMP
	/*
	{
		uint32 w, h;
		getScreenSize(w, h);
		Driver->setMatrixMode3D(MainCam);
		Driver->setModelMatrix(CMatrix::Identity);
		drawBox(_StartPos - CVector(0.02f, 0.02f, 0.02f), _StartPos + CVector(0.02f, 0.02f, 0.02f), CRGBA::Magenta);
		Driver->setMatrixMode2D11();
		Driver->drawLine((_RefX - 5) / (float) w, _RefY / (float) h, (_RefX + 5) / (float) w, _RefY / (float) h, CRGBA::Red);
		Driver->drawLine(_RefX / (float) w, (_RefY - 5) / (float) h, _RefX / (float) w, (_RefY + 5) / (float) h, CRGBA::Red);
		//
		Driver->drawLine((_PosRefX - 5) / (float) w, _PosRefY / (float) h, (_PosRefX + 5) / (float) w, _PosRefY / (float) h, CRGBA::Green);
		Driver->drawLine(_PosRefX / (float) w, (_PosRefY - 5) / (float) h, _PosRefX / (float) w, (_PosRefY + 5) / (float) h, CRGBA::Green);
		//
		Driver->drawLine((_MouseRefX - 5) / (float) w, _MouseRefY / (float) h, (_MouseRefX + 5) / (float) w, _MouseRefY / (float) h, CRGBA::Yellow);
		Driver->drawLine(_MouseRefX / (float) w, (_MouseRefY - 5) / (float) h, _MouseRefX / (float) w, (_MouseRefY + 5) / (float) h, CRGBA::Yellow);
		//
	}
	*/
}

// ***************************************************************
void CToolSelectMove::setInstancePos(const NLMISC::CVectorD &pos, CInstance &instance)
{
	//H_AUTO(R2_CToolSelectMove_setInstancePos)
	std::string posInstanceId = instance.getPosInstanceId();
	if (posInstanceId.empty()) return;
	CObject *newPos = buildVector(pos, posInstanceId);
	getEditor().requestSetLocalNode(_GhostInstance ? _GhostInstance->getId() : instance.getId(), "Position", newPos);
	delete newPos;
}

// ***************************************************************
bool CToolSelectMove::isActionPossibleOn(const CInstance &instance) const
{
	//H_AUTO(R2_CToolSelectMove_isActionPossibleOn)
	CInstance &mutableInstance = const_cast<CInstance &>(instance);
	CDisplayerVisual *dv = mutableInstance.getDisplayerVisual();
	if (dv && dv->getActualDisplayMode() != CDisplayerVisual::DisplayModeVisible)
	{
		return false;
	}
	if (isShiftDown())
	{
		CLuaStackRestorer lsr(&getEditor().getLua(), 0);
		mutableInstance.getLuaProjection().callMethodByNameNoThrow("isCopyable", 0, 1); // isCopyable should be 'const', so no worries here ...
		if (getEditor().getLua().toBoolean(1))
		{
			return true;
		}
		return false;
	}
	return true;
}

// ***************************************************************
void CToolSelectMove::onActivate()
{
	//H_AUTO(R2_CToolSelectMove_onActivate)
	setContextHelp(CI18N::get("uiR2EDToolSelectMove"));
}

// ***************************************************************
void CToolSelectMove::updateBeforeRender()
{
	//H_AUTO(R2_CToolSelectMove_updateBeforeRender)
	if (_Duplicating && _GhostInstance)
	{
		_AutoGroup.update(_FinalPos, _GhostInstance->getPaletteId(), _ValidPos && !isCtrlDown());
		_GhostInstance->getDisplayerVisual()->setDisplayFlag(CDisplayerVisual::FlagHideActivities, _AutoGroup.getGroupingCandidate() != NULL);
	}
	else
	{
		if (!_Duplicating && _GhostInstance) _GhostInstance->getDisplayerVisual()->setDisplayFlag(CDisplayerVisual::FlagHideActivities, false);
	}
}

/////////////////////
// ACTION HANDLERS //
/////////////////////


/**
  * Make the select/move tool current
  */
class CAHSelectMove : public IActionHandler
{
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		getEditor().setCurrentTool(new CToolSelectMove);
	}
};
REGISTER_ACTION_HANDLER(CAHSelectMove, "r2ed_select_move");



} // R2

