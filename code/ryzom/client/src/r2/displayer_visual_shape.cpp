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
#include "displayer_visual_shape.h"
#include "instance.h"
#include "editor.h"
#include "r2_config.h"
#include "tool.h"
//
#include "../entity_cl.h"
#include "../global.h"
#include "../misc.h"
#include "../pacs_client.h"
#include "verbose_clock.h"
//
#include "nel/3d/u_instance_material.h"
#include "nel/3d/u_shape_bank.h"
#include "nel/3d/u_visual_collision_manager.h"
#include "nel/3d/u_visual_collision_entity.h"


using namespace NLMISC;

extern uint	SkipFrame;

namespace R2
{

// *********************************************************************************************************
CDisplayerVisualShape::CDisplayerVisualShape(const std::string &shapeName, float scale, bool worldMapDisplay)
{
	_ShapeName = shapeName;
	_Scale = scale;
	_Touched = true;
	_BadShapeName = false;
	_Active = false;
	_VisualSnapToGroundDone = false;
	_VisualCollisionEntity = NULL;
	_WorldMapDisplay = worldMapDisplay;
}

// *********************************************************************************************************
CDisplayerVisualShape::~CDisplayerVisualShape()
{
	deleteShape();
	deleteVisualCollisionEntity();
}

// *********************************************************************************************************
void CDisplayerVisualShape::deleteVisualCollisionEntity()
{
	//H_AUTO(R2_CDisplayerVisualShape_deleteVisualCollisionEntity)
	if (CollisionManager)
	{
		if (_VisualCollisionEntity)
		{
			CollisionManager->deleteEntity(_VisualCollisionEntity);
			_VisualCollisionEntity = NULL;
		}
	}
}

// *********************************************************************************************************
void CDisplayerVisualShape::deleteShape()
{
	//H_AUTO(R2_CDisplayerVisualShape_deleteShape)
	if (!_Instance.empty())
	{
		Scene->deleteInstance(_Instance);
	}
	if (_MapDeco.isAddedToMap())
	{
		CGroupMap *gm = CTool::getWorldMap();
		if (gm)
		{
			gm->removeDeco(&_MapDeco);
		}
	}
}

// *********************************************************************************************************
void CDisplayerVisualShape::onPreRender()
{
	//H_AUTO(R2_CDisplayerVisualShape_onPreRender)
	if (SkipFrame != 0)
	{
		// a tp was done -> invalidate visual collision entity
		deleteVisualCollisionEntity();
	}
	if (!_Instance.empty())
	{
		bool inIsland = false;
		CIslandCollision &col = getEditor().getIslandCollision();
		R2::CScenarioEntryPoints::CCompleteIsland	*currIsland = col.getCurrIslandDesc();
		if (currIsland)
		{
			inIsland = currIsland->isIn(getWorldPos2f());
		}
		if (getActualVisibility() && inIsland)
		{
			_Instance.show();
			if (_WorldMapDisplay && !_MapDeco.getActive())
			{
				_MapDeco.setActive(true);
				_Touched = true;
			}
		}
		else
		{
			_Instance.hide();
			_MapDeco.setActive(false);
		}
	}
	if (!getActualVisibility()) return;
	if (_BadShapeName) return;
	if (_Active)
	{
		if (testNeedZEval()) _VisualSnapToGroundDone = false;
		if (!_VisualSnapToGroundDone && SkipFrame == 0)
		{
			snapToGround(); // force visual snap to ground if not already done + icon update
		}
	}
	if (!_Touched)
	{
		if (!_Instance.empty()) _BBoxMatrix = _Instance.getMatrix();
		else  _BBoxMatrix = CMatrix::Identity;
		return;
	}
	if (_Active)
	{
		updateMapDeco();
		if (_Instance.empty())
		{
			sint64 startTime = 0;
			static volatile bool bench = false;
			if (bench)
			{
				startTime = CTime::getPerformanceTime();
			}
			_Instance = Scene->createInstance(_ShapeName);
			if (bench)
			{
				sint64 endTime = CTime::getPerformanceTime();
				nlwarning("clip time = %.2f ms", 1000.f * CTime::ticksToSecond(endTime - startTime));
			}
			if (_Instance.empty())
			{
				_BadShapeName = true;
				return;
			}
			_Instance.setTransformMode(NL3D::UTransform::DirectMatrix);
			_Instance.enableCastShadowMap(true);
		}
		CMatrix instanceMat;
		instanceMat.setScale(_Scale);
		instanceMat.setPos(getWorldPos().asVector());
		_Instance.setMatrix(instanceMat);
		_VisualSnapToGroundDone = false;
		visualSnapToGround(); // force visual snap to ground if not already done
	}
	else
	{
		if (!_Instance.empty())
		{
			Scene->deleteInstance(_Instance);
		}
	}

	if (!_Instance.empty())
	{
		_BBoxMatrix = _Instance.getMatrix(); // ensure that bbox and shape displayed at same pos
											 // (events that modify the instance pos may be received after the display of this frame)
	}
	_MapDeco.setInvalidPosFlag(getDisplayFlag(FlagBadPos));
	_Touched = false;
}

// *********************************************************************************************************
void CDisplayerVisualShape::drawBBox(NLMISC::CRGBA color) const
{
	//H_AUTO(R2_CDisplayerVisualShape_drawBBox)
	if (getRotateInProgress()) return; // no drawn while drawing (bbox moved one frame too late, must solve this)
	NLMISC::CAABBox bbox;
	_Instance.getShapeAABBox(bbox);
	Driver->setModelMatrix(_BBoxMatrix);
	::drawBox(bbox.getMin(),     bbox.getMax(), color);
}

// *********************************************************************************************************
void CDisplayerVisualShape::onPostRender()
{
	//H_AUTO(R2_CDisplayerVisualShape_onPostRender)
	if (!_Active || !getActualVisibility()) return;
	if (_BadShapeName) return;
	if (!_Instance.empty())
	{
		if (getDisplayFlag(FlagSelected))
		{
			//visualSnapToGround();
			drawBBox(CRGBA::Green);
			setEmissive(_Instance, getBlinkColor(CV_SelectedInstanceColor.get()));
		}
		else if (getDisplayFlag(FlagHasFocus))
		{
			//visualSnapToGround();
			drawBBox(CRGBA::White);
			setEmissive(_Instance, getBlinkColor(CV_FocusedInstanceColor.get()));
		}
		else
		{
			CRGBA color = getDisplayModeColorInScene();
			setEmissive(_Instance, color);
			::makeInstanceTransparent(_Instance, color.A, color.A != 255);
		}
	}
	CDisplayerVisual::onPostRender();
}

// *********************************************************************************************************
void CDisplayerVisualShape::onAttrModified(const std::string &name,    sint32 index)
{
	//H_AUTO(R2_CDisplayerVisualShape_onAttrModified)
	CDisplayerVisual::onAttrModified(name, index);
	if (name == "Position")
	{
		_VisualSnapToGroundDone = false;
		_Touched = true;
	}
	/*else if (name == "DisplayMode")
	{
		updateMapDeco();
	}*/
}

// *********************************************************************************************************
void CDisplayerVisualShape::setDisplayMode(sint32 mode)
{
	//H_AUTO(R2_CDisplayerVisualShape_setDisplayMode)
	CDisplayerVisual::setDisplayMode(mode);
	updateMapDeco();
}


// *********************************************************************************************************
void CDisplayerVisualShape::onParentDisplayModeChanged()
{
	//H_AUTO(R2_CDisplayerVisualShape_onParentDisplayModeChanged)
	updateMapDeco();
}

// *********************************************************************************************************
void CDisplayerVisualShape::onFocus(bool focused)
{
	//H_AUTO(R2_CDisplayerVisualShape_onFocus)
	CDisplayerVisual::onFocus(focused);
	updateMapDeco();
}

// *********************************************************************************************************
void CDisplayerVisualShape::onSelect(bool selected)
{
	//H_AUTO(R2_CDisplayerVisualShape_onSelect)
	CDisplayerVisual::onSelect(selected);
	updateMapDeco();
}


// *********************************************************************************************************
void CDisplayerVisualShape::setActive(bool active)
{
	//H_AUTO(R2_CDisplayerVisualShape_setActive)
	if (active == _Active) return;
	if (!active)
	{
		deleteShape();
	}
	else
	{
		if (!_MapDeco.isAddedToMap() && _WorldMapDisplay)
		{
			CGroupMap *gm = CTool::getWorldMap();
			if (gm)
			{
				_MapDeco.setDisplayedInstance(getDisplayedInstance(), false);
				gm->addDeco(&_MapDeco);
				_MapDeco.invalidateCoords();
			}
		}
	}
	_Touched = true;
	_Active = active;
}

// *********************************************************************************************************
void CDisplayerVisualShape::updateMapDeco()
{
	//H_AUTO(R2_CDisplayerVisualShape_updateMapDeco)
	if (_MapDeco.isAddedToMap())
	{
		CGroupMap *gm = CTool::getWorldMap();
		if (gm)
		{
			_MapDeco.onUpdate(*gm);
			_MapDeco.invalidateCoords();
		}
	}
}

// *********************************************************************************************************
bool CDisplayerVisualShape::getActive() const
{
	//H_AUTO(R2_CDisplayerVisualShape_getActive)
	return _Active;
}

// *********************************************************************************************************
bool CDisplayerVisualShape::init(const CLuaObject &parameters)
{
	//H_AUTO(R2_CDisplayerVisualSh_initSape)
	_ShapeName = parameters["ShapeName"].toString();
	if (parameters["Scale"].isNumber())
	{
		_Scale = parameters["Scale"];
	}
	NL3D::UShapeBank	*shapeBank= Driver->getShapeBank();
	if (shapeBank)
	{
		shapeBank->buildSystemGeometryForshape(_ShapeName);
	}
	return CDisplayerVisual::init(parameters);
}

// *********************************************************************************************************
bool CDisplayerVisualShape::getLastClip() const
{
	//H_AUTO(R2_CDisplayerVisualShape_getLastClip)
	if (_Instance.empty()) return true;
	return !_Instance.getLastClippedState();
}

// *********************************************************************************************************
NLMISC::CAABBox CDisplayerVisualShape::getSelectBox() const
{
	//H_AUTO(R2_CDisplayerVisualShape_getSelectBox)
	if (_Instance.empty()) return CDisplayerVisual::getSelectBox();
	// TODO nico : cache the bbox
	NLMISC::CAABBox bbox;
	_Instance.getShapeAABBox(bbox);
	bbox.setMinMax(_Scale * bbox.getMin(), _Scale * bbox.getMax());
	return bbox;
}

// *********************************************************************************************************
float CDisplayerVisualShape::preciseIntersectionTest(const NLMISC::CVector &worldRayStart,const NLMISC::CVector &worldRayDir) const
{
	//H_AUTO(R2_CDisplayerVisualShape_preciseIntersectionTest)
	if (_Instance.empty()) return FLT_MAX;
	if(_Instance.supportFastIntersect())
	{
		float dist2D, distZ;
		if (const_cast<NL3D::UInstance &>(_Instance).fastIntersect(worldRayStart, worldRayDir, dist2D, distZ, false))
		{
			if (dist2D == 0.f)
			{
				return distZ;
			}
		}
	}
	return FLT_MAX;
}

// *********************************************************************************************************
const NLMISC::CMatrix &CDisplayerVisualShape::getInvertedMatrix() const
{
	//H_AUTO(R2_CDisplayerVisualShape_getInvertedMatrix)
	// no rot part for now
	_InvertedMatrix.setPos(- getWorldPos().asVector());
	return _InvertedMatrix;
}

// *********************************************************************************************************
void CDisplayerVisualShape::updateWorldPos()
{
	//H_AUTO(R2_CDisplayerVisualShape_updateWorldPos)
	CDisplayerVisual::updateWorldPos();
	// must always snap pos (if world pos was modified because of parent move,
	// in this case our relative pos remains unmodified)
	_VisualSnapToGroundDone = false;
	snapToGround();
	updateMapDeco();

}

// *********************************************************************************************************
void CDisplayerVisualShape::snapToGround()
{
	//H_AUTO(R2_CDisplayerVisualShape_snapToGround)
	if (!GR)
	{
		return;
	}

	NLPACS::UGlobalPosition gpos = GR->retrievePosition(getWorldPos());
	if (gpos.InstanceId != -1)
	{
		CVector snappedPos = GR->getGlobalPosition(gpos);
		// locally modify the z
		_WorldPos.z = snappedPos.z;
		//setDisplayFlag(FlagBadPos, false);
	}
	else
	{
		//setDisplayFlag(FlagBadPos, true);
	}
	visualSnapToGround();
	_Touched = true;
}

// *********************************************************************************************************
void CDisplayerVisualShape::visualSnapToGround()
{
	//H_AUTO(R2_CDisplayerVisualShape_visualSnapToGround)
	if (!_VisualSnapToGroundDone)
	{
		if (!_Instance.empty())
		{
			CMatrix mat = _Instance.getMatrix();
			CVector pos3f = _WorldPos.asVector();
			// VisualCollisionEntity's  'snap to ground' requires that the entity is not farther than 100 meters from it
			// Eval first coarse height from height map + coarse collision mesh
			CTool::TRayIntersectionType interType = CTool::NoIntersection;
			static volatile bool coarseSnap = true;
			if (coarseSnap)
			{
				CVector inter;
				CTool::TRayIntersectionType interType = CTool::computeWorldMapIntersection(pos3f.x, pos3f.y, inter);
				if (interType != CTool::NoIntersection)
				{
					pos3f.z = inter.z;
				}
			}
			bool snapped = false;
			if (CollisionManager)
			{
				// create if necessary
				if (!_VisualCollisionEntity)
				{
					_VisualCollisionEntity = CollisionManager->createEntity();
				}
				// eval finer pos from previous pos
				snapped = _VisualCollisionEntity->snapToGround(pos3f); // refine z from current pos
			}
			//
			if (snapped || interType != CTool::NoIntersection) // take precise version or else default to coarsest one
			{
				_WorldPos.z = pos3f.z;
			}
			_VisualSnapToGroundDone = true;
			mat.setPos(_WorldPos);
			_Instance.setMatrix(mat);
		}
	}
}

// *********************************************************************************************************
NLMISC::CVector CDisplayerVisualShape::evalLinkPoint(bool /* leader */)
{
	//H_AUTO(R2_CDisplayerVisualShape_evalLinkPoint)
	if (!_Instance.empty())
	{
		visualSnapToGround();
		// use the visual snapped pos instead
		return _Instance.getMatrix().getPos();
	}
	// return position that is visually snapped to the ground
	return _WorldPos;
}


// *********************************************************************************************************
void CDisplayerVisualShape::onAdd(CGroupMap &/* owner */)
{
	//H_AUTO(R2_CDisplayerVisualShape_onAdd)

}

// *********************************************************************************************************
void CDisplayerVisualShape::onRemove(CGroupMap &/* owner */)
{
	//H_AUTO(R2_CDisplayerVisualShape_onRemove)
}

// *********************************************************************************************************
void CDisplayerVisualShape::onPreRender(CGroupMap &/* owner */)
{
	//H_AUTO(R2_CDisplayerVisualShape_onPreRender)

}

// *********************************************************************************************************
void CDisplayerVisualShape::onUpdate(CGroupMap &/* owner */)
{
	//H_AUTO(R2_CDisplayerVisualShape_onUpdate)
	//
}




} // R2

