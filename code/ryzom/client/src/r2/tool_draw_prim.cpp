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
#include "../interface_v3/group_map.h"
//
#include "tool_draw_prim.h"
#include "game_share/object.h"
#include "dmc/idmc.h"
#include "editor.h"
#include "r2_config.h"
#include "object_factory_client.h"
#include "displayer_visual_group.h"
#include "tool_maintained_action.h"
//
#include "nel/misc/vectord.h"
#include "nel/misc/i18n.h"
#include "nel/misc/polygon.h"
//
// tmp tmp
#include "nel/gui/ctrl_quad.h"


using namespace NLMISC;

namespace R2
{


// ***************************************************************
CToolDrawPrim::CToolDrawPrim(TPrimType primType, CInstance *extending /*= NULL*/)
{
	_NumPoints = 0;
	_ValidPos = false;
	_PrimType = primType;
	_MustClose = false;
	_PrimInitialized = false;
	_ValidPrim = true;
	_DistinctLastPoint = true;
	_ExtendedPrimitive = extending;
	_Extending = (_ExtendedPrimitive != NULL);
	_StartNumPoints = 0;
	_InaccessibleParts = false;
	_Commited = false;
	_ForceShowPrims = false;
}


// ***************************************************************
bool CToolDrawPrim::canTerminate() const
{
	//H_AUTO(R2_CToolDrawPrim_canTerminate)
	return ((_PrimType == Road && _NumPoints >= 1) || _NumPoints >= 3) && _ValidPrim;
}

// ***************************************************************
const char *CToolDrawPrim::getToolUIName() const
{
	//H_AUTO(R2_CToolDrawPrim_getToolUIName)
	return _PrimType == Road ? "drawRoad" : "drawRegion";
}


// ***************************************************************
bool CToolDrawPrim::init(const CLuaObject &parameters)
{
	//H_AUTO(R2_CToolDrawPrim_init)
	_Points.clear();
	_PrimLook.init(parameters["Look"]);
	if (!parameters["InvalidLook"].isNil())
	{
		_PrimLookInvalid.init(parameters["InvalidLook"]);
	}
	else
	{
		_PrimLookInvalid = _PrimLook;
	}
	if (!parameters["CanCloseLook"].isNil())
	{
		_PrimLookCanClose.init(parameters["CanCloseLook"]);
	}
	else
	{
		_PrimLookCanClose = _PrimLook;
	}
	/*
	if (!parameters["InaccessibleLook"].isNil())
	{
		_PrimLookInaccessible.init(parameters["InaccessibleLook"]);
	}
	else
	{
		_PrimLookInaccessible = _PrimLook;
	}
	*/
	//TMP TMP hardcoded for first test
	_PrimLookInaccessible.init(getEditor().getEnv()["PrimRender"]["RoadLookInaccessible"]);
	//
	_InaccessiblePrim.setLook(_PrimLookInaccessible);
	//
	CLuaObject vertices = parameters["Vertices"];
	if (vertices.isTable())
	{
		// if start points where given, use them
		ENUM_LUA_TABLE(vertices, it)
		{
			CLuaObject &vertex = it.nextValue();
			_Points.push_back(NLMISC::CVector((float) vertex["x"].toNumber(), (float) vertex["y"].toNumber(), (float) vertex["z"].toNumber()));
		}
	}
	if (!parameters["ExtendedPrimitiveId"].isNil())
	{
		std::string extendedPrimitiveId = parameters["ExtendedPrimitiveId"];
		_ExtendedPrimitive = getEditor().getInstanceFromId(extendedPrimitiveId);
		if (!_ExtendedPrimitive)
		{
			_Extending = false;
			nlwarning("Can't extend primitive with id %s", extendedPrimitiveId.c_str());
			return false;
		}
		_Extending = true;
	}
	_CancelFunc = parameters["OnCancel"];
	_CookieKey = parameters["CookieKey"].toString();
	_CookieValue = parameters["CookieValue"];
	_PrimType = parameters["Type"].toString() == "Region" ? Region : Road;
	_Prim.setLook(_PrimLook);
	_NumPoints = (uint)_Points.size();

	if (!parameters["ForceShowPrims"].isNil())
	{
		_ForceShowPrims = parameters["ForceShowPrims"].toBoolean();
	}

	if (!parameters["SelectInstance"].isNil())
	{
		_SelectInstance = parameters["SelectInstance"].toBoolean();
	}
	else
	{
		_SelectInstance = true;
	}

	CGroupMap *worldMap = getWorldMap();
	if (worldMap)
	{
		worldMap->addDeco(&_Prim); // display primitive on the map
	}
	return true;
}

// ***************************************************************
void CToolDrawPrim::updateAfterRender()
{
	//H_AUTO(R2_CToolDrawPrim_updateAfterRender)
}

// ***************************************************************
void CToolDrawPrim::removeFromWorldMap()
{
	//H_AUTO(R2_CToolDrawPrim_removeFromWorldMap)
	CGroupMap *worldMap = getWorldMap();
	if (worldMap)
	{
		if (_Prim.isAddedToWorldMap())
		{
			worldMap->removeDeco(&_Prim);
		}
		if (_InaccessiblePrim.isAddedToWorldMap())
		{
			worldMap->removeDeco(&_InaccessiblePrim);
		}
	}
}

// ***************************************************************
void CToolDrawPrim::cancel()
{
	//H_AUTO(R2_CToolDrawPrim_cancel)
	if (_ExtendedPrimitive)
	{
		CDisplayerVisualGroup *dv = dynamic_cast<CDisplayerVisualGroup *>(_ExtendedPrimitive->getDisplayerVisual());
		if (dv)
		{
			dv->setActiveRecurse(true);
		}
	}
	removeFromWorldMap();
	if (!_Commited)
	{
		if (_CancelFunc.isFunction())
		{
			_CancelFunc.callNoThrow(0, 0);
		}
	}
}

// ***************************************************************
void CToolDrawPrim::setPrimLook(bool closed, bool lastEdgeIsValid, bool valid)
{
	//H_AUTO(R2_CToolDrawPrim_setPrimLook)
	CPrimLook *look;
	if (!valid) look = &_PrimLookInvalid;
	else if (closed) look = &_PrimLookCanClose;
	else look = &_PrimLook;
	look->LastEdgeIsValid = lastEdgeIsValid;
	_Prim.setLook(*look);
}

// ***************************************************************
void CToolDrawPrim::updateBeforeRender()
{
	//H_AUTO(R2_CToolDrawPrim_updateBeforeRender)
	doUpdateBeforeRender();
	CGroupMap *worldMap = getWorldMap();
	if (worldMap && !_InaccessibleParts && _InaccessiblePrim.isAddedToWorldMap())
	{
		worldMap->removeDeco(&_InaccessiblePrim); // display primitive on the map
	}
}

// ***************************************************************
void CToolDrawPrim::doUpdateBeforeRender()
{
	//H_AUTO(R2_CToolDrawPrim_doUpdateBeforeRender)
	if (_Extending && !_ExtendedPrimitive)
	{
		cancel();
		return;
	}
	_MustClose = false;
	// Build vector for direction pointed by mouse in world
	sint32 mouseX,  mouseY;
	getMousePos(mouseX,  mouseY);
	if (!isInScreen(mouseX,  mouseY))
	{
		// mouse not in screen so don't display the last point
		_Points.resize(_NumPoints);
		_Prim.setVertices(_Points);
		updateValidityFlag(false);
		setPrimLook(false, true, _ValidPrim);
		_Prim.addDecalsToRenderList();
		return;
	}

	sint32 autoPanDx, autoPanDy;
	if (_NumPoints >= 1)
	{
		handleWorldMapAutoPan(autoPanDx, autoPanDy);
	}
	//
	CTool::CWorldViewRay worldViewRay;
	//
	computeWorldViewRay(mouseX,  mouseY,  worldViewRay);
	//
	_ValidPos = false;
	CVector inter;
	CVector wpPos;
	TRayIntersectionType interType = computeLandscapeRayIntersection(worldViewRay,  inter);
	switch(interType)
	{
		case NoIntersection:
			if (worldViewRay.OnMiniMap)
			{
				_Points.resize(_NumPoints);
				_Prim.setVertices(_Points);
				updateValidityFlag(false);
				setPrimLook(false, true, _ValidPrim);
				_Prim.addDecalsToRenderList();
				setMouseCursor("curs_stop.tga");
				return;
			}
			// no collision,  can't drop entity
			wpPos = worldViewRay.Origin + 3.f * worldViewRay.Dir;
		break;
		case ValidPacsPos:
			wpPos = inter;
			_ValidPos = isValid2DPos(inter); // good pos to drop entity
		break;
		case InvalidPacsPos:
			wpPos = inter;
		break;
		default:
			nlassert(0);
		break;
	}
	// If there are at least 3 vertices and the primitive is a region, then see whether mouse
	// is near the first vertex -> in this case, propose to the user to close the primitive
	// If landscape is intersected too, then the vertex must be nearer then the first vertex
	if (_PrimType == Region && _NumPoints >= 3)
	{
		bool canClose = false;
		CGroupMap *gm = isMouseOnWorldMap();
		if (gm)
		{
			CViewBitmap *bm = _Prim.getWorldMapVertexView(0);
			if (bm)
			{
				canClose = bm->isIn(mouseX, mouseY);
			}
		}
		else
		{
			if (!_Prim.getVerticesShapeInstance().empty())
			{
				if (!_Prim.getVerticesShapeInstance()[0].empty())
				{
					NLMISC::CAABBox vertexBBox;
					_Prim.getVerticesShapeInstance()[0].getShapeAABBox(vertexBBox);
					vertexBBox.setCenter(_Points[0]);
					if (vertexBBox.intersect(worldViewRay.Origin, worldViewRay.Origin + 200.f * worldViewRay.Dir))
					{
						// if vertex nearer than landscape
						bool landscapeNearest = false;
						if (interType != NoIntersection)
						{
							// TODO nico : the test is not exact : should use intersection with the bbox to do
							// length comparison over the same line ...
							landscapeNearest = (wpPos - worldViewRay.Origin).norm() < (_Points[0] - worldViewRay.Origin).norm();
						}
						canClose = !landscapeNearest;
					}
				}
			}
		}
		if (canClose)
		{
			_Points.resize(_NumPoints);
			_Prim.setVertices(_Points);
			updateValidityFlag(false);
			setPrimLook(true, true, _ValidPrim); // show a closed polygon
			_MustClose = true;
			setMouseCursor("curs_pick.tga");
			_Prim.addDecalsToRenderList();
			return;
		}
	}
	_Prim.setEmissive(CRGBA::Black);
	//
	_Points.resize(_NumPoints + 1);
	if (_Points.size() >= 2)
	{
		_DistinctLastPoint = (_Points[_NumPoints] != _Points[_NumPoints - 1]);
	}
	else
	{
		_DistinctLastPoint = true;
	}

	_Points.back() = wpPos;
	_Prim.setVertices(_Points);
	updateValidityFlag(!_ValidPos);
//	updateValidityFlag(interType == NoIntersection);
	setPrimLook(false, _ValidPos, _ValidPrim);

	// change mouse depending on result
	if (!isMouseOnUI() || isMouseOnWorldMap() || !_ValidPrim)
	{
		setMouseCursor(_ValidPos /* && !_InaccessibleParts */ ? "curs_create.tga" : "curs_stop.tga");
	}
	else
	{
		setMouseCursor("curs_create.tga");
	}
	_Prim.addDecalsToRenderList();
	CGroupMap *worldMap = getWorldMap();
	//static CCtrlQuad *testQuad = NULL;
	if (_InaccessibleParts)
	{
		/*if (!testQuad)
		{
			testQuad = new CCtrlQuad;
			testQuad->setModulateGlobalColor(false);
			float thickness = 256.f;
			testQuad->setQuad(CVector(0.f, thickness, 0.f), CVector(thickness * 2.f, thickness, 0.f), thickness);
			testQuad->setTexture("*accessibility_texture*");
			worldMap->addCtrl(testQuad);
			testQuad->setParent(worldMap);
		}*/
		if (worldMap && !_InaccessiblePrim.isAddedToWorldMap())
		{
			worldMap->addDeco(&_InaccessiblePrim); // display primitive on the map
			//
		}
		updateInaccessiblePrimRenderLook(_InaccessiblePrim);
		_InaccessiblePrim.setVertices(_Points);
		_InaccessiblePrim.addDecalsToRenderList();
	}
	// TMP TMP
	static volatile bool testDecal = false;
	if (testDecal)
	{
		R2::CScenarioEntryPoints::CCompleteIsland	*id = getEditor().getIslandCollision().getCurrIslandDesc();
		if (id)
		{
			CMatrix m;
			m.setScale(CVector(40000.f, 40000.f, 40000.f));
			m.setPos(CVector(0.f, -40000.f, 0.f));
			_TestDecal.setWorldMatrix(m);
			_TestDecal.setCustomUVMatrix(true, getEditor().getIslandCollision().getWorldToAccessibilityTexMat(true));
			_TestDecal.setTexture("*accessibility_texture*");
			_TestDecal.addToRenderList();
		}
	}
}

// ***************************************************************
void CToolDrawPrim::updateInaccessiblePrimRenderLook(CPrimRender &dest)
{
	//H_AUTO(R2_CToolDrawPrim_updateInaccessiblePrimRenderLook)
	double duration = CV_InaccessiblePosAnimDurationInMS.get();
	if (duration <= 0)  duration = 0.1;
	CRGBA color = blend(CV_InaccessiblePosColor0.get(),
						CV_InaccessiblePosColor1.get(),
						0.5f + 0.5f * (float) cos(NLMISC::Pi * fmod((double) T1, duration) / duration));
	CPrimLook look = dest.getLook();
	look.EdgeLook.WorldMapColor = color;
	look.EdgeLook.DecalColor = color;
	dest.setLook(look);
	dest.setCustomWorldMapEdgeUVMatrix(true, getEditor().getIslandCollision().getWorldToAccessibilityTexMat());
	dest.setCustomDecalEdgeUVMatrix(true, getEditor().getIslandCollision().getWorldToAccessibilityTexMat(true)); // cropped version for decal rendering
}

// ***************************************************************
bool CToolDrawPrim::onMouseLeftButtonClicked()
{
	//H_AUTO(R2_CToolDrawPrim_onMouseLeftButtonClicked)
	if (!checkRoomLeft())
	{
		displayNoMoreRoomLeftMsg();
		return true;
	}
	if (_ValidPos /*&& !_InaccessibleParts*/ && _DistinctLastPoint)
	{
		++ _NumPoints;
	}
	if (_ValidPos)
	{
		startDoubleClickCheck();
	}
	return true;
}

// ***************************************************************
bool CToolDrawPrim::onMouseLeftButtonDown()
{
	//H_AUTO(R2_CToolDrawPrim_onMouseLeftButtonDown)
	if (_MustClose && _ValidPos)
	{
		commit();
		captureMouse(); // mouse will be released on "mouse up" by the default tool
		return true;
	}
	if (!checkDoubleClick()) return false;
	if (canTerminate())
	{
		// the 2 last points must have a different position
		commit();
		CToolMaintainedAction *tma = dynamic_cast<CToolMaintainedAction *>(getEditor().getCurrentTool());
		if (tma)
		{
			tma->markPreviousToolClickEnd();
		}
	}
	return true;
}


// ***************************************************************
void CToolDrawPrim::commit()
{
	//H_AUTO(R2_CToolDrawPrim_commit)
	if (_Extending && !_ExtendedPrimitive)
	{
		cancel();
		return;
	}
	removeFromWorldMap();
	_Points.resize(_NumPoints);
	if (_Points.empty()) return;
	// send network command to create a new road
	CObject *desc = NULL;
	if (!_Extending)
	{
		getDMC().newAction(NLMISC::CI18N::get(_PrimType == Road ? "uiR2EDCreateRouteAction" : "uiR2EDCreateZoneAction"));
	}
	else
	{
		getDMC().newAction(NLMISC::CI18N::get(_PrimType == Road ? "uiR2EDExtendRouteAction" : "uiR2EDExtendZoneAction"));
	}
	if (!_Extending)
	{
		// creating a new primitive, rather than growing an existing one
		std::string className = _PrimType == Road ? "Road": "Region";
		desc = getDMC().newComponent(className);
	}
	if (desc || _Extending)
	{
		CObject *points = NULL;
		if (!_Extending)
		{
			std::string instanceId = getString(desc, "InstanceId");
			if (!instanceId.empty())
			{
				// ask a flag to prompt user to enter name for this object when it is created
				getEditor().setCookie(instanceId, "AskName", true);
				if (_CookieValue.isValid())
				{
					if (!_CookieKey.empty() && !_CookieValue.isNil())
					{
						getEditor().setCookie(instanceId, _CookieKey, _CookieValue);
					}
				}

				getEditor().setCookie(instanceId, "Select", _SelectInstance);
			}
			static volatile bool wantDump = false;
			if (wantDump)
			{
				desc->dump();
			}
			points = desc->getAttr("Points");
			for(uint k = 0; k < _Points.size(); ++k)
			{
				CObject *wp = getDMC().newComponent(_PrimType == Road ? "WayPoint" : "RegionVertex");
				if (!wp) continue;
				wp->setObject("Position", buildVector(CVectorD(_Points[k])));
				points->insert("", wp, -1);
			}
		}
		if (!_Extending)
		{
			// set readable name
			ucstring readableName = NLMISC::CI18N::get(_PrimType == Road ? "uiR2EDNameBotRoad" : "uiR2EDNameBotRegion");
			readableName = getEditor().genInstanceName(readableName);
			desc->set("Name", readableName.toUtf8());
			// send creation command
			// tmp : static npc counter
			// add in component list of default feature
			if (getEditor().getDefaultFeature())
			{
				getDMC().requestInsertNode(getEditor().getDefaultFeature(getEditor().getBaseAct())->getId(),
										   "Components",
										   -1,
										   "",
										   desc);
			}
			//
			delete desc;
			// NB : if display of primitives is 'hide all', then force to 'show all', else
			// the new prim wouldn't be visible
			if (_ForceShowPrims)
			{
				if (getEditor().getEnv()["PrimDisplayVisible"].toBoolean() == false ||
					getEditor().getEnv()["PrimDisplayContextualVisibility"].toBoolean() == true)
				{
					getEditor().callEnvMethod("notifyPrimDisplayShowAll", 0, 0);
				}
			}
		}
		else
		{
			nlassert(!desc);
			CVectorD offset = CVectorD::Null;
			CDisplayerVisualGroup *dv = dynamic_cast<CDisplayerVisualGroup *>(_ExtendedPrimitive->getDisplayerVisual());
			if (dv)
			{
				offset = - dv->getWorldPos();
			}

			std::vector<CDisplayerVisual *> sons;
			dv->getSons(sons);

			for(uint k = _StartNumPoints - 1; k > _Points.size() - 1; --k)
			{
				getDMC().requestEraseNode(sons[k]->getDisplayedInstance()->getId(), "", -1);
			}

			for (uint k = 0; k < std::min(_StartNumPoints, uint(_Points.size())); ++k)
			{
				// change modified vertices
				CVector newWorldPos = CVectorD(_Points[k]).asVector();
				if (newWorldPos != _InitialPoints[k])
				{
					CObject *wp = getDMC().newComponent(_PrimType == Road ? "WayPoint" : "RegionVertex");
					if (wp)
					{
						CObject *newPos = buildVector(offset + CVectorD(_Points[k]));
						getDMC().requestSetNode(sons[k]->getDisplayedInstance()->getId(), "Position", newPos);
						delete newPos;
					}
				}
			}
			// insert newly created vertices
			for(uint k = _StartNumPoints; k < _Points.size(); ++k)
			{
				CObject *wp = getDMC().newComponent(_PrimType == Road ? "WayPoint" : "RegionVertex");
				if (!wp) continue;
				wp->setObject("Position", buildVector(offset + CVectorD(_Points[k])));
				getDMC().requestInsertNode(_ExtendedPrimitive->getId(), "Points", -1, "", wp);
				delete wp;
			}
			if (dv)
			{
				dv->setActiveRecurse(true);
			}
		}
	}
	_Commited = true;
	getEditor().setCurrentTool(NULL);	// set the default tool
	return;
}


// ***************************************************************
bool CToolDrawPrim::onMouseRightButtonClicked()
{
	//H_AUTO(R2_CToolDrawPrim_onMouseRightButtonClicked)
	// TMP : exception for primitive drawing : right button finish the road
	if (!checkRoomLeft())
	{
		displayNoMoreRoomLeftMsg();
		return true;
	}
	if (_ValidPos && _DistinctLastPoint)
	{
		if (!_MustClose)
		{
			++ _NumPoints;
		}
		if (canTerminate())
		{
			commit();
		}
	}
	return true;
	// cancel the drawing
	//getEditor().setCurrentTool(NULL);
	//return true;
}

// ***************************************************************
bool CToolDrawPrim::onDeleteCmd()
{
	//H_AUTO(R2_CToolDrawPrim_onDeleteCmd)
	CTool::TSmartPtr hold(this);
	if (_NumPoints == 0)
	{
		if (_Extending)
		{
			getDMC().newAction(CI18N::get("uiR2EDDeleteRoad") + _ExtendedPrimitive->getDisplayName());
			// just erase the road
			getDMC().requestEraseNode(_ExtendedPrimitive->getId(), "", -1);
			getDMC().getActionHistoric().endAction();
		}
		// cancel the drawing
		getEditor().setCurrentTool(NULL);
	}
	else
	{
		-- _NumPoints;
		_Points.resize(_NumPoints);
	}
	return true;
}


// ***************************************************************
bool CToolDrawPrim::isValidPolyShape(bool ignoreLast, std::list<CPolygon> &splitPoly) const
{
	//H_AUTO(R2_CToolDrawPrim_isValidPolyShape)
	splitPoly.clear();
	if(_Points.size() <= 3)
	{
		return true;
	}
	// test that no self-intersection is found
	static NLMISC::CPolygon2D poly2D;
	poly2D.Vertices.resize(_Points.size());
	for(uint k = 0; k < _Points.size(); ++k)
	{
		poly2D.Vertices[k] = _Points[k];
	}
	if (poly2D.selfIntersect()) return false;
	CPolygon poly;
	poly.Vertices = _Points;
	if (poly.Vertices.back() == poly.Vertices[poly.Vertices.size() - 2] || ignoreLast)
	{
		poly.Vertices.pop_back(); // duplicated point at the end will always fails the test, so remove it
	}
	bool validPrim = poly.toConvexPolygons(splitPoly, CMatrix::Identity);
	if (!validPrim)
	{
		std::reverse(poly.Vertices.begin(), poly.Vertices.end());
		validPrim = poly.toConvexPolygons(splitPoly, CMatrix::Identity);
	}
	return validPrim;
}

// ***************************************************************
bool CToolDrawPrim::testAccessibleEdges(bool ignoreLast)
{
	//H_AUTO(R2_CToolDrawPrim_testAccessibleEdges)
	uint numPoints = (uint)_Points.size();
	if (_PrimType == Road)
	{
		numPoints -= ignoreLast ? 2 : 1;
	}
	else
	{
		numPoints -= ignoreLast ? 2 : 0;
	}
	for (sint k = 0; k < (sint) numPoints; ++k)
	{
		if (!getEditor().getIslandCollision().isValidSegment(_Points[k], _Points[(k + 1) % _Points.size()]))
		{
			return false;
		}
	}
	return true;
}

// ***************************************************************
void CToolDrawPrim::updateValidityFlag(bool ignoreLast)
{
	//H_AUTO(R2_CToolDrawPrim_updateValidityFlag)
	_ValidPrim = true;
	if (_PrimType != Road)
	{
		std::list<CPolygon> splitPoly;
		if (!isValidPolyShape(ignoreLast, splitPoly))
		{
			_ValidPrim = false;
		}
	}
}

/*
{
	nlassert(0); // old code
	if (_LastPoints.size() == _Points.size() && std::equal(_Points.begin(), _Points.end(), _LastPoints.begin()))
		return; // not modified, so last flag remains valid
	_InaccessibleParts = false;
	_LastPoints = _Points;
	// CHANGE: accessibility for edges not tested anymore ...
	//
	// bool accessible = testAccessibleEdges(ignoreLast);
	// _InaccessibleParts = !accessible;
	//
	_InaccessibleParts = false;
	sint testPointIndex = _Points.size() - (ignoreLast ? 2 : 1);
	if (testPointIndex < 0)
	{
		_ValidPrim = false;
		return;
	}
	// change : valid to create if current mouse pos is valid (test 0.5f meters around)
	CVector2f testPos = _Points[testPointIndex];
	_ValidPrim = getEditor().getIslandCollision().isValidPos(testPos);

	if (!_ValidPrim) return;
	// old test : all parts crossed by the edges needed to be accessible
	//
	// if (!accessible)
	// {
	//	_ValidPrim = false;
	//}
	if (_PrimType != Road)
	{
		std::list<CPolygon> splitPoly;
		if (!isValidPolyShape(ignoreLast, splitPoly))
		{
			_ValidPrim = false;
		}
	}
}*/

// ***************************************************************
void CToolDrawPrim::onActivate()
{
	//H_AUTO(R2_CToolDrawPrim_onActivate)
	setContextHelp(CI18N::get("uiR2EDToolDrawPrim"));
	if (_ExtendedPrimitive)
	{
		CDisplayerVisualGroup *dv = dynamic_cast<CDisplayerVisualGroup *>(_ExtendedPrimitive->getDisplayerVisual());
		if (!dv)
		{
			cancel();
			return;
		}
		else
		{
			dv->setActiveRecurse(false); // hide the primitive being extended, because we display
										 // our own version until we are finished
		}
		std::vector<CDisplayerVisual *> sons;
		dv->getSons(sons);
		for(uint k = 0; k < sons.size(); ++k)
		{
			_Points.push_back(sons[k]->getWorldPos().asVector());
		}
		_NumPoints = (uint)_Points.size();
		_StartNumPoints = _NumPoints;
		_InitialPoints = _Points;
	}
}

// ***************************************************************
bool CToolDrawPrim::checkRoomLeft()
{
	//H_AUTO(R2_CToolDrawPrim_checkRoomLeft)
	return _NumPoints < PrimMaxNumPoints;
}

// ***************************************************************
void CToolDrawPrim::displayNoMoreRoomLeftMsg()
{
	//H_AUTO(R2_CToolDrawPrim_displayNoMoreRoomLeftMsg)
	getEditor().callEnvMethod("noMoreRoomLeftInPrimitveMsg", 0, 0);
}


} // R2
