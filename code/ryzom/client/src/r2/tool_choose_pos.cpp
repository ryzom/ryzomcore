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
#include "tool_choose_pos.h"
#include "../interface_v3/interface_manager.h"
#include "../global.h"
#include "nel/gui/group_tree.h"
#include "../interface_v3/group_map.h"
#include "../landscape_poly_drawer.h"
//
#include "../entities.h"
//
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/u_instance.h"
//
#include "game_share/object.h"
#include "dmc/idmc.h"
#include "r2_config.h"

using namespace NLPACS;
using namespace NLMISC;

namespace R2
{


// ***************************************************************
CToolChoosePos::CToolChoosePos(sint ghostSlot,
							   const std::string &cursValid,
							   const std::string &cursInvalid,
							   const std::vector<CPolygon2D> &polyList,
							   const CPrimLook &polyValidLook,
							   const CPrimLook &polyInvalidLook
							  )
{
	_CursValid = cursValid;
	_CursInvalid = cursInvalid;
	_GhostSlot = ghostSlot,
	_Valid = false;
	_WantedAction = NoAction;
	_WarningNoMeshOrSkeletonShown = false;
	CEntityCL *entity = getGhost();
	_LocalSelectBoxSize = 1;
	if (entity)
	{
		entity->getMeshDefaultScale(_DefaultScale);
		if (!entity->skeleton())
		{
			const CVector &hs = getEditor().getLocalSelectBox(*entity).getHalfSize();
			_LocalSelectBoxSize = maxof(fabsf(hs.x), fabsf(hs.y), fabsf(hs.z));
		}
	}
	else
	{
		_DefaultScale.set(1.f, 1.f, 1.f);
	}
	_MultiPos = false;
	_MultiPosLocked = false;
	_BadPlaceDecal.setTexture("*accessibility_texture*", true, true, false);
	_TestDecal.setTexture("encyclopedia_box_zo.tga", true, true, false);
	//
	_PolyList = polyList;
	_PolyRender.resize(polyList.size());
	_PolyValidLook = polyValidLook;
	_PolyInvalidLook = polyInvalidLook;
	CGroupMap *gm = getWorldMap();
	for (uint k = 0; k < _PolyRender.size(); ++k)
	{
		_PolyRender[k].setActive(false);
		if (gm)
		{
			gm->addDeco(&_PolyRender[k]);
		}
	}
}

// ***************************************************************
void CToolChoosePos::showPolys(bool visible)
{
	//H_AUTO(R2_CToolChoosePos_showPolys)
	for (uint k = 0; k < _PolyRender.size(); ++k)
	{
		_PolyRender[k].setActive(visible);
	}
}

// ***************************************************************
void CToolChoosePos::enableMultiPos(const std::string &cursValidMulti /*="curs_create_multi.tga"*/)
{
	//H_AUTO(R2_CToolChoosePos_enableMultiPos)
	_MultiPos = true;
	_CursValidMulti = cursValidMulti;
}

// ***************************************************************
CToolChoosePos::~CToolChoosePos()
{
	cancel();
	CGroupMap *gm = getWorldMap();
	if (gm)
	{
		for (uint k = 0; k < _PolyRender.size(); ++k)
		{
			_PolyRender[k].setActive(false);
			gm->removeDeco(&_PolyRender[k]);
		}
	}
}

// ***************************************************************
CEntityCL *CToolChoosePos::getGhost()
{
	//H_AUTO(R2_CToolChoosePos_getGhost)
	if (_GhostSlot == -1) return NULL;
	return EntitiesMngr.entity(_GhostSlot);
}

// ***************************************************************
void CToolChoosePos::hideMapSelectionAxis()
{
	//H_AUTO(R2_CToolChoosePos_hideMapSelectionAxis)
	CGroupMap *worldMap = getWorldMap();
	if (worldMap) worldMap->setSelectionAxis(false);
}

// ***************************************************************
void CToolChoosePos::updateInvalidCursorOnUI()
{
	//H_AUTO(R2_CToolChoosePos_updateInvalidCursorOnUI)
	setMouseCursor(DEFAULT_CURSOR);
}

// ***************************************************************
void CToolChoosePos::updateBeforeRender()
{
	//H_AUTO(R2_CToolChoosePos_updateBeforeRender)
	CGroupMap *worldMap = getWorldMap();
	CEntityCL *entity = getGhost();
	float dtBackup  = DT;
	DT = RZ_TIME_TO_BECOME_TRANSPARENT_IN_SECOND; // don't want a transition
	if (entity)
	{
		uint32 oldOpacity = CEntityCL::getOpacityMin();
		CEntityCL::setOpacityMin(DEFAULT_ENTITY_MIN_OPACITY);
		entity->makeTransparent(true);
		CEntityCL::setOpacityMin(oldOpacity);
	}
	DT = dtBackup;
	// Build vector for direction pointed by mouse in world
	sint32 mouseX,  mouseY;
	getMousePos(mouseX,  mouseY);
	if (!isInScreen(mouseX,  mouseY) || (isMouseOnUI() && !isMouseOnWorldMap()))
	{
		if (worldMap) worldMap->setSelectionAxis(false);
		if (entity)
		{
			entity->show(false);
		}
		updateInvalidCursorOnUI();
		showPolys(false);
		return;
	}
	//
	CTool::CWorldViewRay worldViewRay;
	//
	computeWorldViewRay(mouseX,  mouseY,  worldViewRay);
	//
	CVector entityPos; // the pos where the ghost will be shown
	CVector inter;     // intersection of view ray with landscape
	_Valid = false;
	CVector scale = _DefaultScale;
	TRayIntersectionType rayIntersectionType = computeLandscapeRayIntersection(worldViewRay,  inter);
	switch(rayIntersectionType)
	{
		case NoIntersection:
		{
			// no collision,  can't drop entity
			entityPos = worldViewRay.Origin + 3.f * worldViewRay.Dir;
			// change the scale so that the entity has always the same size on screen
			float refScale = CV_FloatingShapeRefScale.get();
			if (refScale == 0.f) refScale = 1.f;
			if (_LocalSelectBoxSize != 0.f)
			{
				scale *= refScale / _LocalSelectBoxSize;
			}
			if (worldMap) worldMap->setSelectionAxis(false);
			showPolys(false);
		}
		break;
		case ValidPacsPos:
			entityPos = inter;
			_CreatePosition = entityPos;
			_Valid = isValidChoosePos(inter); // good pos to drop entity
			if (worldMap) worldMap->setSelectionAxis(true, inter);
			showPolys(true);
		break;
		case InvalidPacsPos:
			entityPos = inter;
			if (worldMap) worldMap->setSelectionAxis(true, inter);
			showPolys(true);
		break;
		default:
			nlassert(0);
		break;
	}
	// compute front vector
	CVector front = - MainCam.getMatrix().getJ();
	front.z = 0.f;
	front.normalize();
	// compute angle around Z
	_CreateAngle = (float) atan2(front.y,  front.x);
	//
	if (entity)
	{
		CMatrix mat;
		bool shown = false;
		if (worldViewRay.OnMiniMap && rayIntersectionType == NoIntersection)
		{
			entity->show(false);
		}
		else
		if (!entity->skeleton())
		{
			/*
			nlwarning("Selected entity for the 'create' tool has no skeleton");
			entity->show(false);
			return;
			*/
			//bool loading = true;
			NL3D::UInstance inst = entity->instance();
			if (inst.empty())
			{
				if (!entity->instances().empty())
				{
					inst = entity->instances()[0].Current;
					//loading = !entity->instances()[0].Loading.empty() && entity->instances()[0].Current.empty();
				}
			}
			if (inst.empty())
			{
				if (!_WarningNoMeshOrSkeletonShown)
				{
					nlwarning("Selected entity for the 'create' tool has no instance");
					_WarningNoMeshOrSkeletonShown = true;
				}
				entity->show(false);
				showPolys(false);
				return;
			}
			entity->show(true);
			shown = true;
			//
			CQuat frontQuat(CVector::K,  _CreateAngle - (float) (NLMISC::Pi / 2));
			inst.setRotQuat(frontQuat);
			inst.setPos(entityPos);
			inst.setScale(scale);
			mat = inst.getMatrix();
		}
		else
		{
			entity->show(true);
			shown = true;
			//
			entity->updateVisible(T1,  NULL);
			entity->updatePos(T1,  NULL);

			/*
			CMatrix skelMatrix = entity->skeleton()->getMatrix();
			// relative position to skeleton root
			CVector skelRootRelativePos = entity->skeleton()->getMatrix().getPos() - entity->pos().asVector();
			// combine quat for front face xform & anim quat
			CQuat frontQuat(CVector::K,  _CreateAngle);
			entity->skeleton()->setRotQuat(frontQuat * entity->skeleton()->getRotQuat());
			entity->skeleton()->setPos(entityPos + skelRootRelativePos);
			mat.setRot(frontQuat);
			mat.setPos(entityPos);
			*/
			CMatrix skelMatrix = entity->skeleton()->getMatrix();
			// relative position to skeleton root
			CVector skelRootRelativePos = entity->skeleton()->getMatrix().getPos() - entity->pos().asVector();
			// combine quat for front face xform & anim quat
			CQuat frontQuat(CVector::K,  _CreateAngle);
			entity->skeleton()->setRotQuat(frontQuat * entity->skeleton()->getRotQuat());
			CMatrix frontMat;
			frontMat.setRot(frontQuat);
			entity->skeleton()->setPos(entityPos + frontMat * skelRootRelativePos);
			/*mat.setRot(frontQuat);
			mat.setPos(entityPos);*/

		}
		// see if all pos are accessible and update the _Valid flag
		// NB NICO : THE FOLLOWING CODE IS WORKING BUT
		// see with others if it's useful to check this type of collisions
		// finally -> since check is never done, limited/no interest ...
		/*
		if (shown && entity->getPrimitive())
		{
			float w;
			float h;
			if (entity->getPrimitive()->getPrimitiveType() == UMovePrimitive::_2DOrientedBox)
			{
				entity->getPrimitive()->getSize(w, h);
			}
			else
			{
				w = h = 2.f * entity->getPrimitive()->getRadius();
			}
			// transform from [0, 1] x [0, 1] to local bbox
			CMatrix unitBoxToLocalPrim;
			unitBoxToLocalPrim.setScale(CVector(w, h, 1.f));
			//unitBoxToLocalBox.setPos(CVector(localBox.getMin().x / favoid0(w), localBox.getMin().y / favoid0(h), 1.f);
			unitBoxToLocalPrim.setPos(CVector(- 0.5f * w, - 0.5f * h, 0.f));
			CMatrix worldMat = mat * unitBoxToLocalPrim;
			// test if place is valid
			CPolygon poly;
			poly.Vertices.resize(4);
			poly.Vertices[0].set(0.f, 0.f, 0.f);
			poly.Vertices[1].set(1.f, 0.f, 0.f);
			poly.Vertices[2].set(1.f, 1.f, 0.f);
			poly.Vertices[3].set(0.f, 1.f, 0.f);
			CPolygon2D poly2D(poly, worldMat);

			if (getEditor().getIslandCollision().isValidPoly(poly2D))
			{
				_TestDecal.setWorldMatrix(worldMat);
				_TestDecal.addToRenderList();
			}
			else
			{
				_Valid = false;
				_BadPlaceDecal.setWorldMatrix(worldMat);
				_BadPlaceDecal.setDiffuse(getInvalidPosColor());
				_BadPlaceDecal.setCustomUVMatrix(true, getEditor().getIslandCollision().getWorldToAccessibilityTexMat(true));
				_BadPlaceDecal.addToRenderList();

			}
		}
		*/
	}
	// additionnal polygons
	if (!_PolyRender.empty() && _PolyRender[0].getActive())
	{
		static NLMISC::CPolygon2D tmpPoly;
		std::vector<NLMISC::CVector2f> &verts = tmpPoly.Vertices;
		nlassert(_PolyRender.size() == _PolyList.size());
		CIslandCollision &ic = getEditor().getIslandCollision();
		for (uint k = 0; k < _PolyRender.size(); ++k)
		{
			verts.resize(_PolyList[k].Vertices.size());
			for (uint l = 0; l < verts.size(); ++l)
			{
				verts[l].set(_PolyList[k].Vertices[l].x + _CreatePosition.x,
							 _PolyList[k].Vertices[l].y + _CreatePosition.y);
			}
			bool validPoly = ic.isValidPoly(tmpPoly); // Test all position because entities may be created in that zone


			// also test zone vertices because in the end, a valid zone is a zone for which all vertices
			// are valid
			for (uint l = 0; validPoly && l < verts.size(); ++l)
			{
				if (!ic.isValidPos(verts[l])) validPoly = false;
			}


			if (!validPoly) _Valid = false;

			/*
			bool validPoly = true;
			for (uint l = 0; l < verts.size() && validPoly; ++l)
			{
				if (!ic.isValidSegment(verts[l], verts[(l + 1) % verts.size()]))
				{
					validPoly = false;
					_Valid = false;
				}
			}
			*/
			_PolyRender[k].setLook(validPoly ? _PolyValidLook : _PolyInvalidLook);
			_PolyRender[k].setVertices(verts);
			_PolyRender[k].addDecalsToRenderList();

			if (_PolyRender[k].getLook().Shape == CPrimLook::ClosedPolyLine)
			{
				NLMISC::CAABBox bbox;
				static volatile bool showPoly = true;
				if (showPoly)
				{
					CLandscapePolyDrawer::getInstance().computeBBoxFromPolygon(tmpPoly, bbox);
					CLandscapePolyDrawer::getInstance().addPoly(tmpPoly,  _PolyRender[k].getLook().EdgeLook.WorldMapColor, bbox);
				}
				_PolyRender[k].setWorldMapPolyColor(_PolyRender[k].getLook().EdgeLook.WorldMapColor);
			}
		}
	}
	// change mouse depending on result
	if (!isMouseOnUI() || isMouseOnWorldMap())
	{

		if (_Valid)
		{
			if (_MultiPos && isShiftDown() && !_MultiPosLocked)
			{
				setMouseCursor(_CursValidMulti.c_str());
			}
			else
			{
				setMouseCursor(_CursValid.c_str());
			}
		}
		else
		{
			setMouseCursor(_CursInvalid.c_str());
		}
	}
	else
	{
		setMouseCursor(_CursValid.c_str());
	}
}

// ***************************************************************
void CToolChoosePos::updateAfterRender()
{
	//H_AUTO(R2_CToolChoosePos_updateAfterRender)
	CTool::TSmartPtr oldThis(this); // may be changed during commit
	switch(_WantedAction)
	{
		case SelectPos:
		{
			showPolys(false);
			hideMapSelectionAxis();
			if (_Valid)
			{
				// signal deriver that a position has been chosen
				commit(_CreatePosition, _CreateAngle);
				if (stopAfterCommit() && (!_MultiPos || !isShiftDown()) && !_MultiPosLocked)
				{
					if (_GhostSlot != -1)
					{
						//EntitiesMngr.remove(_GhostSlot,  true);
						CEntityCL *ghost = EntitiesMngr.entity(_GhostSlot);
						if (ghost)
						{
							// just hide the slot because creation may happen just after, so keep
							// a ref on textures and shape
							ghost->show(false);
						}
					}
					getEditor().setCurrentTool(NULL);
				}
				else
				{
					_WantedAction = NoAction;
				}
			}
		}
		break;
		case Cancel:
		{
			cancel();
			getEditor().setCurrentTool(NULL);
		}
		break;
		case NoAction:
			// no-op
		break;
		default:
			nlassert(0);
		break;
	}
}


// ***************************************************************
void CToolChoosePos::removeGhostSlot()
{
	//H_AUTO(R2_CToolChoosePos_removeGhostSlot)
	if (_GhostSlot != - 1 && getGhost())
	{
		CEntityCL *ghost = EntitiesMngr.entity(_GhostSlot);
		if (ghost)
		{
			// just hide the slot because creation may happen just after, so keep
			// a ref on textures and shape (this avoid a release from memory)
			ghost->show(false);
			ghost->displayable(false);
		}
		//EntitiesMngr.remove(_GhostSlot,  true); // remove the ghost
		_GhostSlot = -1;
	}
}

// ***************************************************************
bool CToolChoosePos::onMouseLeftButtonClicked()
{
	//H_AUTO(R2_CToolChoosePos_onMouseLeftButtonClicked)
	if (_Valid)
	{
		_WantedAction = SelectPos;
	}
	return true;
}

// ***************************************************************
bool CToolChoosePos::onMouseRightButtonClicked()
{
	//H_AUTO(R2_CToolChoosePos_onMouseRightButtonClicked)
	_WantedAction = Cancel;
	return true;
}

// ***************************************************************
void CToolChoosePos::cancel()
{
	//H_AUTO(R2_CToolChoosePos_cancel)
	showPolys(false);
	// TODO nico : potential crash here if called just before render (in updateBeforeRender)
	// TODO nico : rethink the deferred action stuff to avoid this possible case
	hideMapSelectionAxis();
	removeGhostSlot();
}




} // R2
