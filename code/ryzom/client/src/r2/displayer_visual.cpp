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
#include "displayer_visual.h"
// r2
#include "instance.h"
#include "editor.h"
#include "tool_select_move.h"
//
#include "nel/misc/algo.h"
#include "nel/misc/vector_2f.h"
//
#include "../time_client.h"
#include "nel/gui/lua_ihm.h"
#include "../global.h"
//
#include "../interface_v3/interface_manager.h"
#include "../interface_v3/group_in_scene.h"
//
#include "r2_config.h"

using namespace NLMISC;

extern uint	SkipFrame;

namespace R2
{


// ***************************************************************
CDisplayerVisual::CDisplayerVisual() : _DisplayFlags(FlagNone)
{
	_BlinkStartDate = 0;
	_Pos.set(0, 0, 0);
	_WorldPos = _Pos;
	_IconInScene = NULL;
	_IconInSceneCreationFailed = false;
	_DisplayFlags = 0;
	_RotateInProgress = false;
	_MoveInProgress = false;
	// TODO nico : optim here : only one decal used at a time
	_LastCamDist = -100.f;
	_DisplayMode = DisplayModeVisible;
	_InheritDisplayMode = false; // filled at init by this displayer lua parameter
	_LastParentOk = false;
	_LastParent = NULL;
}

// ***************************************************************
CDisplayerVisual::~CDisplayerVisual()
{
	if (_IconInScene)
	{
		CWidgetManager::getInstance()->unMakeWindow(_IconInScene);
		if (_IconInScene->getParent())
		{
			_IconInScene->getParent()->delGroup(_IconInScene);
		}
		else
		{
			delete _IconInScene;
		}
	}
}

// ***************************************************************
void CDisplayerVisual::setDisplayFlag(TDisplayFlags flag, bool on)
{
	//H_AUTO(R2_CDisplayerVisual_setDisplayFlag)
	nlctassert(FlagCount <= 32);
	nlassert((uint) flag < FlagCount);
	setFlags(_DisplayFlags, 1 << flag, on);
}

// ***************************************************************
bool CDisplayerVisual::getDisplayFlag(TDisplayFlags flag) const
{
	//H_AUTO(R2_CDisplayerVisual_getDisplayFlag)
	nlctassert(FlagCount <= 32);
	nlassert((uint) flag < FlagCount);
	return (_DisplayFlags & (1 << flag)) != 0;
}

// ***************************************************************
CDisplayerVisual *CDisplayerVisual::getParent()
{
	if (_LastParentOk) return _LastParent;
	CDisplayerVisual *result = NULL;
	//H_AUTO(R2_CDisplayerVisual_getParent)
	CInstance *inst = getDisplayedInstance();
	nlassert(inst);
	CInstance *parentInstance = inst->getParent();
	if (parentInstance)
	{
		result = parentInstance->getDisplayerVisual();
	}
	_LastParent = result;
	_LastParentOk = true;
	return result;
}

// ***************************************************************
const CDisplayerVisual *CDisplayerVisual::getParent() const
{
	//H_AUTO(R2_CDisplayerVisual_getParent)
	return (const_cast<CDisplayerVisual *>(this))->getParent();
	/*CInstance *inst = getDisplayedInstance();
	nlassert(inst);
	CInstance *parentInstance = inst->getParent();
	if (parentInstance)
	{
		return parentInstance->getDisplayerVisual();
	}
	return NULL;*/
}

// ***************************************************************
void CDisplayerVisual::onFocus(bool focused)
{
	//H_AUTO(R2_CDisplayerVisual_onFocus)
	setDisplayFlag(FlagHasFocus, focused);
}

// ***************************************************************
void CDisplayerVisual::onSelect(bool selected)
{
	//H_AUTO(R2_CDisplayerVisual_onSelect)
	setDisplayFlag(FlagSelected, selected);
}

// ***************************************************************
void CDisplayerVisual::onPostHrcMove()
{
	//H_AUTO(R2_CDisplayerVisual_onPostHrcMove)
	_LastParentOk = false; // must update parent
	updateWorldPosRecurse();
}


// ***************************************************************
void CDisplayerVisual::blink()
{
	//H_AUTO(R2_CDisplayerVisual_blink)
	_BlinkStartDate = T1;
}

// ***************************************************************
int CDisplayerVisual::luaBlink(CLuaState &ls)
{
	//H_AUTO(R2_CDisplayerVisual_luaBlink)
	CLuaIHM::checkArgCount(ls,   "luaBlink", 1);
	blink();
	return 0;
}

// ***************************************************************
NLMISC::CRGBA CDisplayerVisual::getBlinkColor(NLMISC::CRGBA defaultColor,  NLMISC::CRGBA blinkColor) const
{
	//H_AUTO(R2_NLMISC_CRGBA )
	const uint NUM_BLINKS = 3;
	const uint BLINK_LENGTH_IN_MS = 100;
	sint64 blink = (T1 - _BlinkStartDate) / BLINK_LENGTH_IN_MS;
	if (blink > NUM_BLINKS) return defaultColor;
	return !(blink & 1) ? blinkColor : defaultColor;
}

// *********************************************************************************************************
void CDisplayerVisual::onAttrModified(const std::string &attrName,  sint32 /* attrIndex */)
{
	//H_AUTO(R2_CDisplayerVisual_onAttrModified)
	if (attrName == "Position")
	{
		updatePos();
	}
	/*else if (attrName == "DisplayMode")
	{
		updateDisplayMode();
		uint numSons = getNumSons();
		for(uint k = 0; k < numSons; ++k)
		{
			CDisplayerVisual *dv = getSon(k);
			if (dv)
			{
				dv->onParentDisplayModeChanged();
			}
		}
	}*/
}


// *********************************************************************************************************
/*void CDisplayerVisual::updateDisplayMode()
{
	sint value = (sint) getNumber(&getProps(), "DisplayMode");
	if (value >= 0 && value < DisplayModeCount)
	{
		TDisplayMode newDisplayMode = (TDisplayMode) value;
		if (newDisplayMode != _DisplayMode)
		{
			//if (newDisplayMode == DisplayModeFrozen && getEditor().getSelectedInstance() == getDisplayedInstance())
			//{
			//	getEditor().setSelectedInstance(NULL);
			//}
			//_DisplayMode = newDisplayMode;
		}
	}
	else
	{
		nlwarning("Trying to set an invalid display mode : %d", (int) _DisplayMode);
	}
}*/

// *********************************************************************************************************
void CDisplayerVisual::updatePos()
{
	//H_AUTO(R2_CDisplayerVisual_updatePos)
	updateLocalPos();
	updateWorldPosRecurse();
}

// *********************************************************************************************************
void CDisplayerVisual::updateLocalPos()
{
	//H_AUTO(R2_CDisplayerVisual_updateLocalPos)
	if (getProps().findAttr("Position"))
	{
		_Pos = getVector(getObject(&getProps(), "Position"));
		static volatile bool wantAssert = true;
		if (!isValidDouble(_Pos.x) || !isValidDouble(_Pos.y) ||!isValidDouble(_Pos.z))
		{
//			nlassert(!wantAssert);
			BOMB_IF(wantAssert,"'wantAssert' was triggered",_Pos.set(0, 0, 0));
		}
	}
	else
	{
		_Pos.set(0, 0, 0);
	}
}

// *********************************************************************************************************
bool CDisplayerVisual::inheritPos() const
{
	//H_AUTO(R2_CDisplayerVisual_inheritPos)
	return getNumber(&getProps(), "InheritPos") != 0;
}

// *********************************************************************************************************
void CDisplayerVisual::updateWorldPos()
{
	//H_AUTO(R2_CDisplayerVisual_updateWorldPos)
	CDisplayerVisual *parent = getParent();
	if (parent && inheritPos())
	{
		_WorldPos = parent->getWorldPos() + _Pos;
	}
	else
	{
		_WorldPos = _Pos;
	}
	static volatile bool wantAssert = true;
	if (!isValidDouble(_WorldPos.x) || !isValidDouble(_WorldPos.y) ||!isValidDouble(_WorldPos.z))
	{
		nlassert(!wantAssert);
	}
	updateValidPosFlag();
}

// *********************************************************************************************************
void CDisplayerVisual::updateValidPosFlag()
{
	//H_AUTO(R2_CDisplayerVisual_updateValidPosFlag)
	// if I'm not a compound object, update my 'BadPos' flag
	if (!isCompound())
	{
		// If I'm outside of current map then don't display the flag
		CScenarioEntryPoints::CCompleteIsland *islandDesc = getEditor().getIslandCollision().getCurrIslandDesc();
		if (islandDesc)
		{
			if ((sint32) _WorldPos.x < islandDesc->XMin ||
				(sint32) _WorldPos.x > islandDesc->XMax ||
				(sint32) _WorldPos.y < islandDesc->YMin ||
				(sint32) _WorldPos.y > islandDesc->YMax)
			{
				setDisplayFlag(FlagBadPos, false);
				return;
			}
		}
		setDisplayFlag(FlagBadPos, !getEditor().getIslandCollision().isValidPos(NLMISC::CVector(_WorldPos)));
	}
}

// *********************************************************************************************************
void CDisplayerVisual::updateWorldPosRecurse()
{
	//H_AUTO(R2_CDisplayerVisual_updateWorldPosRecurse)
	struct CWorldPosUpdater : public IInstanceVisitor
	{
		virtual void visit(CInstance &inst)
		{
			if (inst.getDisplayerVisual())
			{
				inst.getDisplayerVisual()->updateWorldPos();
			}
		}
	};
	nlassert(getDisplayedInstance());
	CWorldPosUpdater worldPosUpdater;
	getDisplayedInstance()->visit(worldPosUpdater);
}

// *********************************************************************************************************
void CDisplayerVisual::onPostCreate()
{
	//H_AUTO(R2_CDisplayerVisual_onPostCreate)
	_LastParentOk = false;
	updateLocalPos();
	updateWorldPos(); // do not recurse here because sons pos have not been initialized yet
	//updateDisplayMode();
	if (isActiveInCurrentAct())
	{
		if (!getActive())
		{
			setActive(true);
		}
	}
}

// *********************************************************************************************************
void CDisplayerVisual::onErase()
{
	//H_AUTO(R2_CDisplayerVisual_onErase)
	if (getActive())
	{
		setActive(false);
	}
}

// *********************************************************************************************************
void CDisplayerVisual::onPreActChanged()
{
	//H_AUTO(R2_CDisplayerVisual_onPreActChanged)
	updatePos();
	if (!isActiveInCurrentAct())
	{
		if (getActive())
		{
			setActive(false);
		}
	}
}

// *********************************************************************************************************
void CDisplayerVisual::onActChanged()
{
	//H_AUTO(R2_CDisplayerVisual_onActChanged)
	updatePos();
	if (isActiveInCurrentAct())
	{
		if (!getActive())
		{
			setActive(true);
		}
	}
}

// *********************************************************************************************************
void CDisplayerVisual::onContinentChanged()
{
	//H_AUTO(R2_CDisplayerVisual_onContinentChanged)
	if (getActive())
	{
		setActive(false);
		setActive(true); // force a refresh of collision (EntitiesManager::changeContinent updates the primitive, but broken for an unknown reason ...)
	}
}


// *********************************************************************************************************
bool CDisplayerVisual::isActiveInCurrentAct() const
{
	//H_AUTO(R2_CDisplayerVisual_isActiveInCurrentAct)
	// parent act should be the base act (always exists), or the selected act
	CInstance *parentAct = getDisplayedInstance()->getParentAct();
	if (parentAct == getEditor().getBaseAct() || parentAct == getEditor().getCurrentAct())
	{
		return true;
	}
	return false;
}

// *********************************************************************************************************
void CDisplayerVisual::onPostRender()
{
	//H_AUTO(R2_CDisplayerVisual_onPostRender)
	if (!getActive()) return;

	// if this entity is currently moving then don't display the 'stop' icon
	// (it is already drawn by the mouse cursor)
	if (getDisplayedInstance() == getEditor().getSelectedInstance()
		&& dynamic_cast<CToolSelectMove *>(getEditor().getCurrentTool()))
	{
		if (_IconInScene)
		{
			_IconInScene->setActive(false);
		}
	}
	else
	{
		if (getDisplayFlag(FlagBadPos))
		{
			if (!_IconInScene && !_IconInSceneCreationFailed)
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				const char *iconTemplateName = "r2ed_bad_pos_icon";
				// if the in scene 'stop' window wasn't created, then create it now
				CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance (iconTemplateName , "ui:interface", NULL, 0);
				if (group)
				{
					_IconInScene = dynamic_cast<CGroupInScene *>(group);
					if (!_IconInScene)
					{
						nlwarning("Template %s has bad type : should be a derived group from CGroupInScene", iconTemplateName);
						delete group;
						_IconInSceneCreationFailed = true;
					}
					else
					{
						// Link to the interface
						CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", group);
						CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
						group->setParent(pRoot);
						if (pRoot)
							pRoot->addGroup (group);
					}
				}
				else
				{
					_IconInSceneCreationFailed = true;
				}
			}
			if (_IconInScene)
			{
				_IconInScene->setActive(true);
				// tmp set a position above head
				evalIconInScenePos(_IconInScene->Position);
			}
		}
		else
		{
			if (_IconInScene)
			{
				_IconInScene->setActive(false);
			}
		}
	}
}

// *********************************************************************************************************
void CDisplayerVisual::evalIconInScenePos(NLMISC::CVector &dest) const
{
	//H_AUTO(R2_CDisplayerVisual_evalIconInScenePos)
	NLMISC::CAABBox selectBox = getSelectBox();
	float radius = std::max(selectBox.getHalfSize().x, selectBox.getHalfSize().y);
	// use middle front of bbox for icon pos
	NLMISC::CVector result = getWorldPos().asVector() - radius * MainCam.getMatrix().getJ() + selectBox.getHalfSize().z * CVector::K;
	static volatile bool wantAssert = true;
	if (!isValidDouble(result.x) || !isValidDouble(result.y) ||!isValidDouble(result.z))
	{
		nlassert(!wantAssert);
		return;
	}
	dest = result;
}


// *********************************************************************************************************
bool CDisplayerVisual::evalEnterPoint(const NLMISC::CVector &/* startPoint */,  NLMISC::CVector &result)
{
	//H_AUTO(R2_CDisplayerVisual_evalEnterPoint)
	result = evalLinkPoint(false);
	return true;
}

// *********************************************************************************************************
NLMISC::CVector CDisplayerVisual::evalExitPoint()
{
	//H_AUTO(R2_NLMISC_CVector )
	return evalLinkPoint(false);
}

// *********************************************************************************************************
void CDisplayerVisual::getSonsWorldPos2f(std::vector<NLMISC::CVector2f> &result)
{
	//H_AUTO(R2_CDisplayerVisual_getSonsWorldPos2f)
	result.clear();
}

// *********************************************************************************************************
void CDisplayerVisual::getSons(std::vector<CDisplayerVisual *> &sons) const
{
	//H_AUTO(R2_CDisplayerVisual_getSons)
	sons.clear(); // no sons by default
}

// *********************************************************************************************************
void CDisplayerVisual::setRotateInProgress(bool rotateInProgress)
{
	//H_AUTO(R2_CDisplayerVisual_setRotateInProgress)
	_RotateInProgress = rotateInProgress;
}

// *********************************************************************************************************
void CDisplayerVisual::setMoveInProgress(bool moveInProgress)
{
	//H_AUTO(R2_CDisplayerVisual_setMoveInProgress)
	_MoveInProgress = moveInProgress;
}

// *********************************************************************************************************
bool CDisplayerVisual::testNeedZEval()
{
	//H_AUTO(R2_CDisplayerVisual_testNeedZEval)

	if (SkipFrame == 0) // don't update just after a tp because landscape hasn't been updated yet ...
	{
		float newCamDist = (MainCam.getMatrix().getPos() - _WorldPos).norm();
		if (fabsf(newCamDist - _LastCamDist) >= 5.f)
		{
			_LastCamDist = newCamDist;
			updateValidPosFlag();
			return true;
		}
	}
	else
	{
		_LastCamDist = -100.f;
		//nlwarning("Waiting skip frame : %d", (int) SkipFrame);
	}
	return false;
}

// *********************************************************************************************************
bool CDisplayerVisual::isSelectable() const
{
	//H_AUTO(R2_CDisplayerVisual_isSelectable)
	static volatile bool bypass = false;
	if (bypass) return false;
	TDisplayMode dm = getActualDisplayMode();
	return getDisplayedInstance()->getSelectableFromRoot() && (dm == DisplayModeVisible || dm == DisplayModeLocked);
}

// *********************************************************************************************************
CDisplayerVisual::TDisplayMode CDisplayerVisual::getActualDisplayMode() const
{
	//H_AUTO(R2_CDisplayerVisual_getActualDisplayMode)
	static volatile bool bypass = false;
	if (bypass)
	{
		return CDisplayerVisual::DisplayModeVisible;
	}

	TDisplayMode dm;
	const CDisplayerVisual *parent = getParent();
	if (_InheritDisplayMode && parent)
	{
		TDisplayMode parentDM = parent->getActualDisplayMode();
		dm = parentDM == DisplayModeVisible ? _DisplayMode : parentDM;
	}
	else
	{
		dm= _DisplayMode;
	}

	static volatile bool bypass2 = false;
	if (bypass2)
	{
		return CDisplayerVisual::DisplayModeVisible;
	}

	if (!getDisplayedInstance()->getSelectableFromRoot() && dm != DisplayModeHidden)
	{
		dm = DisplayModeFrozen;
	}

	static volatile bool bypass3 = false;
	if (bypass3)
	{
		return CDisplayerVisual::DisplayModeVisible;
	}

	// If it is not an animation but a scenario started from ring access do not display things
	if ( CEditor::getIsStartingScenario() )
	{
		return DisplayModeHidden;
	}
	return dm;
}

// *********************************************************************************************************
CRGBA CDisplayerVisual::getDisplayModeColorInScene() const
{
	//H_AUTO(R2_CDisplayerVisual_getDisplayModeColorInScene)
	switch(getActualDisplayMode())
	{
		case DisplayModeVisible: return CV_UnselectedInstanceColor.get();
		case DisplayModeFrozen: return CV_FrozenInstanceColor.get();
		case DisplayModeLocked: return CV_LockedInstanceColor.get();
		case DisplayModeArray: return CV_ArrayInstanceColor.get();
		default: return CRGBA(0, 0, 0 ,0);
	}
}

// *********************************************************************************************************
CRGBA CDisplayerVisual::getDisplayModeColorInMap() const
{
	//H_AUTO(R2_CDisplayerVisual_getDisplayModeColorInMap)
	switch(getActualDisplayMode())
	{
		case DisplayModeVisible: return CRGBA::White;
		case DisplayModeFrozen: return CV_MapEntityFrozenColor.get();
		case DisplayModeLocked: return CV_MapEntityLockedColor.get();
		case DisplayModeArray: return CV_ArrayInstanceColor.get();
		default: return CRGBA(0, 0, 0 ,0);
	}
}

// *********************************************************************************************************
bool CDisplayerVisual::init(const CLuaObject &parameters)
{
	//H_AUTO(R2_CDisplayerVisual_init)
	if (parameters["InheritDisplayMode"].isBoolean())
	{
		_InheritDisplayMode = parameters["InheritDisplayMode"].toBoolean();
	}
	return CDisplayerBase::init(parameters);
}

// *********************************************************************************************************
void CDisplayerVisual::setDisplayMode(sint32 mode)
{
	//H_AUTO(R2_CDisplayerVisual_setDisplayMode)
	if (mode == _DisplayMode) return;
	if (mode < 0 || mode >= DisplayModeCount)
	{
		nlwarning("Trying to set invalid display mode : %d", (int) mode);
		return;
	}
	_DisplayMode = (TDisplayMode) mode;
	uint numSons = getNumSons();
	for(uint k = 0; k < numSons; ++k)
	{
		CDisplayerVisual *dv = getSon(k);
		if (dv)
		{
			dv->onParentDisplayModeChanged();
		}
	}
}


} // R2
