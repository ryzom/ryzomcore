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
#include "nel/misc/line.h"
#include "nel/3d/u_bone.h"
//
#include "nel/3d/u_landscape.h"

#include "nel/3d/zone.h"
//
#include "displayer_visual_group.h"
#include "r2_config.h"
#include "instance.h"
#include "editor.h"
#include "tool_draw_prim.h"
//
#include "../entity_cl.h"
#include "../global.h"
#include "../misc.h"
#include "../landscape_poly_drawer.h"
#include "nel/gui/ctrl_polygon.h"
#include "nel/gui/ctrl_quad.h"
#include "../time_client.h"
//

class CGroupMap;

using namespace R2;
using namespace std;
using namespace NLMISC;
using namespace NL3D;

extern ULandscape	*Landscape;

namespace R2
{

// create a special selectable polygon
class CCtrlPolygonSelectable : public CCtrlPolygon, public IDisplayerUIHandle
{
public:
	CCtrlPolygonSelectable( CViewBase::TCtorParam &param, CInstance &instance) : Instance(instance), CCtrlPolygon( param ) {}
	// from IDisplayerUIHandle
	virtual CInstance &getDisplayedInstance() { return Instance; }
	// from IDisplayerUIHandle
	virtual bool contains(sint32 mouseXInWindow, sint32 mouseYInWindow) const
	{
		if (!_Parent) return false;

		// relies on parent class CViewPolygon::contains
		return CCtrlPolygon::contains(CVector2f(mouseXInWindow + 0.5f, mouseYInWindow + 0.5f));
	}
	// tooltip
	virtual void		getContextHelp(::ucstring &help) const
	{
		help = Instance.getDisplayName();
		if (std::operator==(help, NLMISC::CI18N::get("uiR2EDNoName") ))
			help.clear();
	}
	bool				emptyContextHelp() const { return true; }
	bool				wantInstantContextHelp() const { return true; }
	// from CCtrlBase
	virtual bool		preciseHitTest(sint32 x, sint32 y) const
	{
		if (!_Parent) return false;
		sint32 winX, winY;
		_Parent->getCorner(winX, winY, _ParentPosRef);
		return contains(x - winX, y - winY);
	}
public:
	CInstance &Instance;
protected:
	// TMP TMP until matrix precision is solved
	// from CCtrlPolygon
	void computeScaledVertex(NLMISC::CVector2f &dest, const NLMISC::CVector2f &src)
	{
		CGroupMap *gm = CTool::getWorldMap();
		if (!gm) dest = CVector::Null;
		gm->worldToWindow(dest, src);
	}
};

// create a special selectable polygon
class CCtrlQuadSelectable : public CCtrlQuad, public IDisplayerUIHandle
{
public:
	CCtrlQuadSelectable( CViewBase::TCtorParam &param, CInstance &instance, uint edgeIndex) : Instance(instance), EdgeIndex(edgeIndex), CCtrlQuad( param ){}
	// from IDisplayerUIHandle
	virtual CInstance &getDisplayedInstance() { return Instance; }
	// from IDisplayerUIHandle
	virtual bool isEdge() const { return true; }
	// from IDisplayerUIHandle
	virtual uint getEdgeIndex() const { return EdgeIndex; }
	// from IDisplayerUIHandle
	virtual bool contains(sint32 mouseXInWindow, sint32 mouseYInWindow) const
	{
		if (!_Parent) return false;

		// relies on parent class CViewPolygon::contains
		return CCtrlQuad::contains(CVector2f(mouseXInWindow + 0.5f, mouseYInWindow + 0.5f));
	}
	// tooltip
	virtual void		getContextHelp(ucstring &help) const
	{
		help = Instance.getDisplayName();
		if (std::operator==(help, NLMISC::CI18N::get("uiR2EDNoName")))
			help.clear();
	}
	bool				emptyContextHelp() const { return true; }
	bool				wantInstantContextHelp() const { return true; }
	// from CCtrlBase
	virtual bool		preciseHitTest(sint32 x, sint32 y) const
	{
		if (!_Parent) return false;
		sint32 winX, winY;
		_Parent->getCorner(winX, winY, _ParentPosRef);
		return contains(x - winX, y - winY);
	}
	virtual bool		handleEvent (const NLGUI::CEventDescriptor &/* event */)
	{
		return false;
	}
public:
	CInstance &Instance;
	uint	  EdgeIndex;
};



// *********************************************************************************************************
CDisplayerVisualGroup::CDisplayerVisualGroup()
{
	_ArrayName = "Components";
	_DrawnThisFrame = false;
	_TimeOver = 0;
	_CurrPrimValid = true;
	touch();
	_Active = false;
	_CurrPrimInaccessible = false;
	_ContextualVisibilityActive = false;
	_ContextualVisibilityDate = -1;
	_VisibleLastFrame = false;
}

// *********************************************************************************************************
void CDisplayerVisualGroup::touch()
{
	//H_AUTO(R2_CDisplayerVisualGroup_touch)
	_PrimTouched = true;
	_InstanceListTouched = true;
	_AccessibilityTouched = true;
}

// *********************************************************************************************************
void CDisplayerVisualGroup::setDisplayedInstance(CInstance *instance)
{
	//H_AUTO(R2_CDisplayerVisualGroup_setDisplayedInstance)
	CDisplayerVisual::setDisplayedInstance(instance);
	_Prim.DisplayedInstance = instance;
}

// *********************************************************************************************************
CCtrlPolygon *CDisplayerVisualGroup::CSelectablePrimRender::newCtrlPolygon() const
{
	//H_AUTO(R2_CDisplayerVisualGroup_CSelectablePrimRender)
	nlassert(DisplayedInstance);
	CViewBase::TCtorParam param;
	CCtrlPolygonSelectable *result = new CCtrlPolygonSelectable( param, *DisplayedInstance);
	result->setId(DisplayedInstance->getId());
	result->setToolTipParent(CCtrlBase::TTMouse);
	result->setToolTipParentPosRef(Hotspot_BR);
	result->setToolTipPosRef(Hotspot_TL);
	return result;
}

// *********************************************************************************************************
CCtrlQuad *CDisplayerVisualGroup::CSelectablePrimRender::newCtrlQuad(uint edgeIndex) const
{
	//H_AUTO(R2_CDisplayerVisualGroup_CSelectablePrimRender)
	nlassert(DisplayedInstance);
	CViewBase::TCtorParam param;
	CCtrlQuadSelectable *result = new CCtrlQuadSelectable( param, *DisplayedInstance, edgeIndex);
	result->setToolTipParent(CCtrlBase::TTMouse);
	result->setToolTipParentPosRef(Hotspot_BR);
	result->setToolTipPosRef(Hotspot_TL);
	return result;
}

// *********************************************************************************************************
bool CDisplayerVisualGroup::init(const CLuaObject &parameters)
{
	//H_AUTO(R2_CDisplayerVisualGroup_init)
	CDisplayerVisual::init(parameters);
	if (parameters["Look"].isTable())
	{
		_PrimLook.init(parameters["Look"]);
	}
	if (parameters["InvalidLook"].isTable())
	{
		_PrimLookInvalid.init(parameters["InvalidLook"]);
	}
	else
	{
		_PrimLookInvalid = _PrimLook;
	}
	_Prim.setLook(_PrimLook);
	if (parameters["ArrayName"].isString())
	{
		_ArrayName = parameters["ArrayName"].toString();
	}
	// look for inaccessible parts (hardcoded for now)
	CPrimLook inaccessibleLook;
	inaccessibleLook.init(getEditor().getEnv()["PrimRender"]["RoadLookInaccessible"]);
	inaccessibleLook.Shape = _PrimLook.Shape;
	_InaccessiblePrim.setLook(inaccessibleLook);

	// meshs
	//_Links.setShapeName(parameters["ShapeName"].toString());
	// decals
	return true;
}

// *********************************************************************************************************
CDisplayerVisualGroup::~CDisplayerVisualGroup()
{

}


// *********************************************************************************************************
void CDisplayerVisualGroup::setActive(bool active)
{
	//H_AUTO(R2_CDisplayerVisualGroup_setActive)
	if (active == _Active) return;
	CGroupMap *worldMap = CTool::getWorldMap();
	if (worldMap)
	{
		if (active)
		{
			if (!_Prim.isAddedToWorldMap())
			{
				worldMap->addDeco(&_Prim);
			}
		}
		else
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
	if (active)
	{
		touch();
	}
	else
	{
		_Prim.clear();
	}
	_Active = active;
}

// *********************************************************************************************************
bool CDisplayerVisualGroup::getActive() const
{
	//H_AUTO(R2_CDisplayerVisualGroup_getActive)
	return _Active;
}


// *********************************************************************************************************
void CDisplayerVisualGroup::onAttrModified(const std::string &attrName,  sint32 attrIndex)
{
	//H_AUTO(R2_CDisplayerVisualGroup_onAttrModified)
	CDisplayerVisual::onAttrModified(attrName,  attrIndex);
	// necessary to handle this because there no event for 'element removed from table' for now ...
	if (attrName == _ArrayName)
	{
		touch();
	}
	/*else if (attrName == "DisplayMode")
	{
		touch();
		updatePrimLook();
	}*/
}

// *********************************************************************************************************
void CDisplayerVisualGroup::setDisplayMode(sint32 mode)
{
	//H_AUTO(R2_CDisplayerVisualGroup_setDisplayMode)
	CDisplayerVisual::setDisplayMode(mode);
	touch();
	updatePrimLook();
}

// *********************************************************************************************************
void CDisplayerVisualGroup::onParentDisplayModeChanged()
{
	//H_AUTO(R2_CDisplayerVisualGroup_onParentDisplayModeChanged)
	touch();
	updatePrimLook();
}

// *********************************************************************************************************
void CDisplayerVisualGroup::updateWorldPos()
{
	//H_AUTO(R2_CDisplayerVisualGroup_updateWorldPos)
	CDisplayerVisual::updateWorldPos();
	// must update position on next display
	_PrimTouched = true;
	_AccessibilityTouched = true;
}

// *********************************************************************************************************
bool CDisplayerVisualGroup::isInProjection(const NLMISC::CVector2f &pos) const
{
	if (!_DrawnThisFrame) return false;
	if ((sint)_Prim.getLook().Shape == (sint)ClosedPolyLine)
	{
		// poly test
		if (_Poly2D.contains(pos, false)) return true;
	}
	// test intersection with all decals
	return _Prim.contains(pos);
}

// *********************************************************************************************************
bool CDisplayerVisualGroup::isInProjectionBorder(const NLMISC::CVector2f &pos) const
{
	if (isOnEdge(pos) != -1)
	{
		return true;
	}
	return false;
}

// *********************************************************************************************************
sint CDisplayerVisualGroup::isOnEdge(const NLMISC::CVector2f &pos) const
{
	return _Prim.isOnEdge(pos);
}

// *********************************************************************************************************
uint CDisplayerVisualGroup::getFadeTimeInMS() const
{
	return (uint) CV_RegionFadeTimeInMs.get();
}


// *********************************************************************************************************
void CDisplayerVisualGroup::onPostRender()
{
	if (!_Active ||!getActualVisibility()) return;
	updatePrimVertices();
	updateAccessibility();
}

// *********************************************************************************************************
bool CDisplayerVisualGroup::isAccessible()
{
	updateAccessibility();
	return !_CurrPrimInaccessible;
}

// *********************************************************************************************************
void CDisplayerVisualGroup::updateAccessibility()
{
	//H_AUTO(R2_CDisplayerVisualGroup_updateAccessibility)
	if (getEditor().getIslandCollision().getPackedIsland() != _LastIsland)
	{
		_AccessibilityTouched = true;

	}
	if (!_AccessibilityTouched) return;
	if (!_Active)
	{
		// if not active, don't update the flag !!! This only is a display flag
		// only npc group use the 'star' shape for now ...
		_CurrPrimInaccessible = false;
		_AccessibilityTouched = false;
		return;
	}
	_LastIsland = getEditor().getIslandCollision().getPackedIsland();
	updatePrimVertices();
	uint numVerts = (uint)_Prim.getVertices().size();
	_CurrPrimInaccessible = false;
	// Old version : path should be accessible along the edges
	/*
	if (_Prim.getLook().Shape != CPrimLook::ClosedPolyLine) -- numVerts;
	for (uint k = 0; k < numVerts; ++k)
	{
		if (!getEditor().getIslandCollision().isValidSegment(_Prim.getVertices()[k], _Prim.getVertices()[(k + 1) % _Prim.getVertices().size()]))
		{
			_CurrPrimInaccessible = true;
			break;
		}
	}
	*/
	// New version : path should be accessible at vertices only
	for (uint k = 0; k < numVerts; ++k)
	{
		if (!getEditor().getIslandCollision().isValidPos(_Prim.getVertices()[k]))
		{
			_CurrPrimInaccessible = true;
			break;
		}
	}
	CGroupMap *gm = CTool::getWorldMap();
	if (gm)
	{
		// error: no more display of inaccessible pos
		/*
		if (_CurrPrimInaccessible)
		{
			_InaccessiblePrim.setVertices(_Prim.getVertices());
			if (!_InaccessiblePrim.isAddedToWorldMap())
			{
				gm->addDeco(&_InaccessiblePrim);
			}
		}
		else
		{
			if (_InaccessiblePrim.isAddedToWorldMap())
			{
				gm->removeDeco(&_InaccessiblePrim);
			}
		}
		*/
	}
	updatePrimLook();
	_AccessibilityTouched = false;
}

// *********************************************************************************************************
void CDisplayerVisualGroup::updatePrimVertices()
{
	//H_AUTO(R2_CDisplayerVisualGroup_updatePrimVertices)
	if (!_Active) return;
	if (!_PrimTouched) return;
	updateInstanceList();
	//
	static std::vector<NLMISC::CVector> vertices;
	vertices.clear();
	_Poly2D.Vertices.clear();
	for(uint k = 0; k < _Instances.size(); ++k)
	{
		if (_Instances[k])
		{
			vertices.push_back(_Instances[k]->evalLinkPoint(k == 0 && _Prim.getLook().Shape == CPrimLook::Star));
			_Poly2D.Vertices.push_back(CVector(vertices.back().x,  vertices.back().y,  0.f));
		}
		else
		{
			nlwarning("_Instances[%d] == 0", k);
		}
	}

	if (_Prim.getLook().Shape == CPrimLook::ClosedPolyLine)
	{
		updateBoundingBox();
	}

	bool valid = true;

	// test against the collision map : all traversed opsition must be valid else poly is invalid
	/*
	if (_PrimLook.Shape == CPrimLook::ClosedPolyLine)
	{
		// test concave poly against map

		// test against water
		#err
	}
	else
	{
		// test polyline against map
		R2::CScenarioEntryPoints::CCompleteIsland *island = getEditor().getIslandCollision().getCurrIslandDesc();
		const NLMISC::CArray2D<sint16> &heightMap = getEditor().getIslandCollision().getHeightMap();
		if (getEditor().getIslandCollision().getHeightMap())
		{

		}
		// test against water
		#err
	}
	*/

	// if displayed on the map, set a first time to see if poly is invalid
	if (_PrimLook.Shape == CPrimLook::ClosedPolyLine)
	{
		if (_Prim.getWorldMapPoly())
		{
			_Prim.getWorldMapPoly()->setVertices(vertices);
			valid = _Prim.getWorldMapPoly()->isValid();
		}
	}
	if (valid != _CurrPrimValid)
	{
		_CurrPrimValid = valid;
		updatePrimLook();
	}
	_Prim.setVertices(vertices);
	_PrimTouched = false;
}

// *********************************************************************************************************
void CDisplayerVisualGroup::updateBoundingBox()
{
	//H_AUTO(R2_CDisplayerVisualGroup_updateBoundingBox)
	CLandscapePolyDrawer::computeBBoxFromPolygon(_Poly2D, _BBox);
}

// *********************************************************************************************************
void CDisplayerVisualGroup::updatePrimLook()
{
	//H_AUTO(R2_CDisplayerVisualGroup_updatePrimLook)
	TDisplayMode displayMode = getActualDisplayMode();
	// if contextual visibility is enabled, then see if really visible
	if (displayMode != DisplayModeHidden && _ContextualVisibilityActive && getDisplayedInstance()->isKindOf("BasePrimitive"))
	{
		if (!isContextuallyVisible())
		{
			displayMode = DisplayModeHidden;
		}
	}
	//
	if (displayMode == DisplayModeHidden)
	{
		_Prim.setActive(false);
		_InaccessiblePrim.setActive(false);
		return;
	}

	_Prim.setActive(true);
	_InaccessiblePrim.setActive(true);

	CPrimLook &look = (_CurrPrimValid && !_CurrPrimInaccessible) ? _PrimLook : _PrimLookInvalid;
	CRGBA oldEdgeWMCol = look.EdgeLook.WorldMapColor;
	CRGBA oldEdgeDecalCol = look.EdgeLook.DecalColor;
	CRGBA oldVertexWMCol = look.VertexLook.WorldMapColor;
	CRGBA oldVertexDecalCol = look.VertexLook.DecalColor;
	CRGBA oldFirstVertexWMCol = look.FirstVertexLook.WorldMapColor;
	CRGBA oldFirstVertexDecalCol = look.FirstVertexLook.DecalColor;
	CRGBA col;
	bool newCol = false;
	switch(displayMode)
	{
		case DisplayModeFrozen:
			col = getBlinkColor(CV_MapEntityFrozenColor.get());
			newCol = true;
		break;
		case DisplayModeLocked:
			col = getBlinkColor(CV_MapEntityLockedColor.get());
			newCol = true;
		break;
		default:
		break;
	}
	if (getDisplayFlag(FlagSelected) && !_CurrPrimInaccessible)
	{
		// NB : don't want the highlight if not all parts are accessible
		//      for readability
		col = CV_MapEntitySelectColor.get();
		newCol = true;
	}
	else if (getDisplayFlag(FlagHasFocus))
	{
		col = CV_MapEntityHighlightColor.get();
		newCol = true;
	}
	if (newCol)
	{
		look.EdgeLook.WorldMapColor = col;
		look.EdgeLook.DecalColor = col;
		look.VertexLook.WorldMapColor = col;
		look.VertexLook.DecalColor = col;
		look.FirstVertexLook.WorldMapColor = col;
		look.FirstVertexLook.DecalColor = col;
	}
	_Prim.setLook(look);
	if (newCol)
	{
		look.EdgeLook.WorldMapColor = oldEdgeWMCol;
		look.EdgeLook.DecalColor = oldEdgeDecalCol;
		look.VertexLook.WorldMapColor = oldVertexWMCol;
		look.VertexLook.DecalColor = oldVertexDecalCol;
		look.FirstVertexLook.WorldMapColor = oldFirstVertexWMCol;
		look.FirstVertexLook.DecalColor = oldFirstVertexDecalCol;
	}
}

// *********************************************************************************************************
void CDisplayerVisualGroup::onFocus(bool focused)
{
	//H_AUTO(R2_CDisplayerVisualGroup_onFocus)
	CDisplayerVisual::onFocus(focused);
	updatePrimLook();
}

// *********************************************************************************************************
void CDisplayerVisualGroup::onSelect(bool selected)
{
	//H_AUTO(R2_CDisplayerVisualGroup_onSelect)
	CDisplayerVisual::onSelect(selected);
	updatePrimLook();
}

// *********************************************************************************************************
void CDisplayerVisualGroup::onPreRender()
{
	//H_AUTO(R2_CDisplayerVisualGroup_onPreRender)
	_DrawnThisFrame = false;
	if (!_Active) return;
	// see if render is on for primitives (don't use a config var because may be dynamically changed)
	//if (!getEditor().getConfig()["PrimDisplayEnabled"].toBoolean()) return;
	_DrawnThisFrame = true;
	bool nowVisible = getActualVisibility();
	if (_VisibleLastFrame != nowVisible)
	{
		touch();
		updatePrimLook();
	}
	_VisibleLastFrame = nowVisible;
	if (!nowVisible) return;
	//
	updatePrimVertices();
	// progressive higlight
	if (!getDisplayFlag(FlagSelected))
	{
		if (getDisplayFlag(FlagHasFocus))
		{
			_TimeOver += DT64;
			if (_TimeOver > getFadeTimeInMS())
			{
				_TimeOver = getFadeTimeInMS();
			}
		}
		else
		{
			_TimeOver -= DT64;
			if (_TimeOver < 0)
			{
				_TimeOver = 0;
			}
		}
	}
	else
	{
		_TimeOver = 0;
	}
	float selectionFactor = 0.f;
	if (_TimeOver != 0)
	{
		uint fadeTime = getFadeTimeInMS();
		if (fadeTime != 0)
		{
			selectionFactor = (float) _TimeOver / fadeTime;
		}
	}
	else
	{
		selectionFactor = getDisplayFlag(FlagHasFocus) ? 1.f : 0.f;
	}
	//
	if (_Prim.getLook().Shape == CPrimLook::ClosedPolyLine && _CurrPrimValid)
	{
		CRGBA baseColor;
		switch(getActualDisplayMode())
		{
			case DisplayModeVisible:
				baseColor = CV_UnselectedRegionColor.get();
			break;
			case DisplayModeFrozen:
				baseColor = CV_FrozenRegionColor.get();
			break;
			case DisplayModeLocked:
				baseColor = CV_LockedRegionColor.get();
			break;
			default:
				baseColor = CV_UnselectedRegionColor.get();
			break;
		}
		CRGBA polyColor;
		if (getDisplayFlag(FlagSelected))
		{
			polyColor = CV_SelectedRegionColor.get();
		}
		else
		{
			polyColor = getBlinkColor(blend(baseColor, CV_FocusedRegionColor.get(), selectionFactor));
		}
		_Prim.setWorldMapPolyColor(polyColor);
		static volatile bool showPoly = true;
		if (showPoly)
		{
			CLandscapePolyDrawer::getInstance().addPoly(_Poly2D,  polyColor, _BBox);
		}
	}
	/*
	if (getDisplayFlag(FlagSelected))
	{
		_Prim.setEmissive(getBlinkColor(CV_SelectedInstanceColor.get()));
	}
	else
	{
		_Prim.setEmissive(getBlinkColor(blend(CV_UnselectedInstanceColor.get(), CV_FocusedInstanceColor.get(), selectionFactor)));
	}
	*/
	//
	_Prim.addDecalsToRenderList();

	if (_CurrPrimInaccessible)
	{
		//  CHANGE: no more display for inaccessible parts
		// update color blinkking for inaccessible
		/*CToolDrawPrim::updateInaccessiblePrimRenderLook(_InaccessiblePrim);
		_InaccessiblePrim.addDecalsToRenderList();
		*/
	}
}



// *********************************************************************************************************
void CDisplayerVisualGroup::updateInstanceList() const
{
	//H_AUTO(R2_CDisplayerVisualGroup_updateInstanceList)
	if (!_InstanceListTouched) return;
	_InstanceListTouched = false;
	_Instances.clear();
	const CObject *sons = getProps().getAttr(_ArrayName);
	if (!sons) return;
	if (sons->getSize() == 0) return;
	_Instances.resize(sons->getSize());
	for(uint k = 0; k < sons->getSize(); ++k)
	{
		CInstance *inst = getEditor().getInstanceFromObject(sons->getValue(k));
		if (inst)
		{
			_Instances[k] = inst->getDisplayerVisual();
		}
	}
}

// *********************************************************************************************************
NLMISC::CVector CDisplayerVisualGroup::evalLinkPoint(bool leader)
{
	//H_AUTO(R2_NLMISC_CVector )
	updateInstanceList();
	if (!_Instances.empty())
	{
		return _Instances[0]->evalLinkPoint(leader);
	}
	else
	{
		nlwarning("_Instances[0] == 0");
		return CVector::Null;
	}
}


// *********************************************************************************************************
NLMISC::CVector CDisplayerVisualGroup::evalExitPoint()
{
	//H_AUTO(R2_NLMISC_CVector )
	updateInstanceList();
	if (!_Instances.empty())
	{
		return _Instances.back()->evalLinkPoint();
	}
	else
	{
		nlwarning("_Instances[0] == 0");
		return CVector::Null;
	}
}


// *********************************************************************************************************
bool CDisplayerVisualGroup::evalEnterPoint(const NLMISC::CVector &startPoint,  NLMISC::CVector &result)
{
	//H_AUTO(R2_CDisplayerVisualGroup_evalEnterPoint)
	updateInstanceList();
	if (_Prim.getLook().Shape != CPrimLook::ClosedPolyLine)
	{
		if (!_Instances.empty())
		{
			result = _Instances[0]->getWorldPos().asVector();
			return true;
		}
		return false;
	}
	_Poly2D.Vertices.resize(_Instances.size());
	for(uint k = 0; k < _Instances.size(); ++k)
	{
		_Poly2D.Vertices[k] = _Instances[k]->getWorldPos().asVector();
	}
	if (_Poly2D.contains(startPoint, false))
	{
		return false;
	}
	float bestDist = FLT_MAX;
	for(uint k = 0; k < _Instances.size(); ++k)
	{
		CVector v0 = _Instances[k]->getWorldPos().asVector();
		CVector v1 = _Instances[(k + 1) % _Instances.size()]->getWorldPos().asVector();
		CLine l(v0, v1);
		CVector proj;

		float coord = (startPoint - v0) * ((v1 - v0).normed());
		float dist;
		if (coord >= 0 && coord < (v1 - v0).norm())
		{
			l.project(startPoint, proj);
			dist = (proj - startPoint).norm();
			if (dist < bestDist)
			{
				bestDist = dist;
				result = proj;
			}
		}

		dist = (startPoint - v0).norm();
		if (dist < bestDist)
		{
			bestDist = dist;
			result = v0;
		}

	}
	return bestDist != FLT_MAX;
}

// *********************************************************************************************************
const NLMISC::CMatrix &CDisplayerVisualGroup::getInvertedMatrix() const
{
	//H_AUTO(R2_NLMISC_CMatrix )
	return CMatrix::Identity; // all coordinates are in world ...
}

// *********************************************************************************************************
void CDisplayerVisualGroup::getSonsWorldPos2f(std::vector<NLMISC::CVector2f> &result)
{
	//H_AUTO(R2_CDisplayerVisualGroup_getSonsWorldPos2f)
	updateInstanceList();
	result.clear();
	for(uint k = 0; k < _Instances.size(); ++k)
	{
		result.push_back(_Instances[k]->getWorldPos2f());
	}
}

// *********************************************************************************************************
void CDisplayerVisualGroup::getSons(std::vector<CDisplayerVisual *> &sons) const
{
	//H_AUTO(R2_CDisplayerVisualGroup_getSons)
	updateInstanceList();
	sons.clear();
	sons.insert(sons.begin(), _Instances.begin(), _Instances.end());
}

// *********************************************************************************************************
uint CDisplayerVisualGroup::getNumSons() const
{
	//H_AUTO(R2_CDisplayerVisualGroup_getNumSons)
	updateInstanceList();
	return (uint)_Instances.size();
}

// *********************************************************************************************************
CDisplayerVisual *CDisplayerVisualGroup::getSon(uint index) const
{
	//H_AUTO(R2_CDisplayerVisualGroup_getSon)
	updateInstanceList();
	nlassert(index < _Instances.size());
	return _Instances[index];
}

// *********************************************************************************************************
void CDisplayerVisualGroup::setActiveRecurse(bool active)
{
	//H_AUTO(R2_CDisplayerVisualGroup_setActiveRecurse)
	setActive(active);
	updateInstanceList();
	for(uint k = 0; k < _Instances.size(); ++k)
	{
		_Instances[k]->setActive(active);
	}
}

// *********************************************************************************************************
void CDisplayerVisualGroup::setContextualVisibilityActive(bool active)
{
	//H_AUTO(R2_CDisplayerVisualGroup_setContextualVisibilityActive)
	_ContextualVisibilityActive = active;
}


// *********************************************************************************************************
CDisplayerVisual::TDisplayMode CDisplayerVisualGroup::getActualDisplayMode() const
{
	//H_AUTO(R2_CDisplayerVisualGroup_getActualDisplayMode)
	TDisplayMode dm = CDisplayerVisual::getActualDisplayMode();
	if (dm == DisplayModeHidden) return dm;
	if (!_ContextualVisibilityActive || const_cast<CDisplayerVisualGroup *>(this)->isContextuallyVisible()) return dm;
	return DisplayModeHidden;
}


// *********************************************************************************************************
bool CDisplayerVisualGroup::isContextuallyVisible()
{
	//H_AUTO(R2_CDisplayerVisualGroup_isContextuallyVisible)
	if (!_ContextualVisibilityActive) return true;
	const CInstance *selection = getEditor().getSelectedInstance();
	if (selection)
	{
		if (this == selection->getDisplayerVisual()) return true;
		if (selection->getDisplayerVisual() && selection->getDisplayerVisual()->getParent() == this) return true;
	}
	return _ContextualVisibilityDate == T1;
}



} // R2
