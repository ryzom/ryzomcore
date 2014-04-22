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
#include "tool.h"
#include "editor.h"
#include "r2_config.h"
//
#include "../interface_v3/interface_manager.h"
#include "nel/gui/event_descriptor.h"
#include "../motion/user_controls.h"
#include "../global.h"
#include "../entities.h"
#include "../water_map.h"
#include "../input.h"
#include "../time_client.h"
#include "displayer_visual.h"
#include "../interface_v3/group_map.h"
#include "nel/gui/lua_ihm.h"
//
#include "nel/pacs/u_global_position.h"
#include "nel/pacs/u_global_retriever.h"
//
#include "nel/misc/matrix.h"
#include "nel/misc/vector_2f.h"
//
#include "nel/3d/u_landscape.h"
//
#include "nel/3d/packed_world.h"

extern NLPACS::UGlobalRetriever	*GR;


using namespace NLMISC;
using namespace NL3D;

namespace R2
{

const uint32 DEFAULT_ENTITY_MIN_OPACITY = 128;


bool CTool::_MouseCaptured = false;
NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> CTool::_UserCharFade;

static const CVector cardinals[] =
{
	CVector(1.f, 0.f, 0.f),
	CVector(1.f, 1.f, 0.f).normed(),
	CVector(0.f, 1.f, 0.f),
	CVector(-1.f, 1.f, 0.f).normed(),
	CVector(-1.f, 0.f, 0.f),
	CVector(-1.f, -1.f, 0.f).normed(),
	CVector(0.f, -1.f, 0.f),
	CVector(1.f, -1.f, 0.f).normed()
};

// ***************************************************************
CTool::CTool()
{
	_DoubleClickStartTime = -1;
	_DoubleClickX = -1;
	_DoubleClickY = -1;
	_AutoPanLastHandlingFrame = 0;
	_AutoPanDelay = 0;
	_NumPans = 0;
}


// ***************************************************************
void CTool::startDoubleClickCheck()
{
	//H_AUTO(R2_CTool_startDoubleClickCheck)
	_DoubleClickStartTime = T0;
	getMousePos(_DoubleClickX, _DoubleClickY);
}

// ***************************************************************
bool CTool::checkDoubleClick()
{
	//H_AUTO(R2_CTool_checkDoubleClick)
	if (_DoubleClickStartTime == -1) return false;
	if (T0 - _DoubleClickStartTime >= CWidgetManager::getInstance()->getUserDblClickDelay()) return false;
	sint32 mx, my;
	getMousePos(mx, my);
	const sint32 moveThrehsold = 2;
	// test that mouse hasn't moved too much between the 2 clicks
	return abs(mx - _DoubleClickX) <= moveThrehsold && abs(my - _DoubleClickY) <= moveThrehsold;
}

// ***************************************************************
bool CTool::isShiftDown()
{
	//H_AUTO(R2_CTool_isShiftDown)
	return Driver->AsyncListener.isKeyDown(KeySHIFT) ||
	Driver->AsyncListener.isKeyDown(KeyLSHIFT) ||
	Driver->AsyncListener.isKeyDown(KeyRSHIFT);
}

// ***************************************************************
bool CTool::isCtrlDown()
{
	//H_AUTO(R2_CTool_isCtrlDown)
	return Driver->AsyncListener.isKeyDown(KeyCONTROL) ||
	Driver->AsyncListener.isKeyDown(KeyLCONTROL) ||
	Driver->AsyncListener.isKeyDown(KeyRCONTROL);
}

// ***************************************************************
CInterfaceManager &CTool::getUI()
{
	//H_AUTO(R2_CTool_getUI)
	return CEditor::getUI();
}

// ***************************************************************
void CTool::getScreenSize(uint32 &scrW, uint32 &scrH)
{
	//H_AUTO(R2_CTool_getScreenSize)
	CViewRenderer::getInstance()->getScreenSize(scrW, scrH);
}

// ***************************************************************
uint32 CTool::getScreenWidth()
{
	//H_AUTO(R2_CTool_getScreenWidth)
	uint32 scrW, scrH;
	CViewRenderer::getInstance()->getScreenSize(scrW, scrH);
	return scrW;
}

// ***************************************************************
uint32 CTool::getScreenHeight()
{
	//H_AUTO(R2_CTool_getScreenHeight)
	uint32 scrW, scrH;
	CViewRenderer::getInstance()->getScreenSize(scrW, scrH);
	return scrH;
}

// ***************************************************************
void CTool::getMousePos(sint32 &x, sint32 &y)
{
	//H_AUTO(R2_CTool_getMousePos)
	CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
	if(cursor == NULL)
	{
		x = y = -1;
		return;
	}
	cursor->getPointerPos(x, y);
}

// ***************************************************************
sint32 CTool::getMouseX()
{
	//H_AUTO(R2_CTool_getMouseX)
	sint32 x, y;
	getMousePos(x, y);
	return x;
}

// ***************************************************************
sint32 CTool::getMouseY()
{
	//H_AUTO(R2_CTool_getMouseY)
	sint32 x, y;
	getMousePos(x, y);
	return y;
}

// ***************************************************************
bool CTool::isMouseOnUI()
{
	//H_AUTO(R2_CTool_isMouseOnUI)
	return CWidgetManager::getInstance()->getWindowUnder(getMouseX(), getMouseY()) != NULL;
}


// ***************************************************************
CGroupMap *CTool::getWorldMap()
{
	//H_AUTO(R2_CTool_getWorldMap)
	static NLMISC::CRefPtr<CGroupMap> mapPtr;
	static volatile bool cacheTest = true;
	if (!mapPtr || !cacheTest)
	{
		mapPtr = dynamic_cast<CGroupMap*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:map:content:map_content:actual_map"));
	}
	return mapPtr;
}

// ***************************************************************
CGroupMap *CTool::isMouseOnWorldMap()
{
	//H_AUTO(R2_CTool_isMouseOnWorldMap)
	const std::vector<CInterfaceGroup *> &groupsUnder = CWidgetManager::getInstance()->getGroupsUnderPointer();
	if (groupsUnder.empty()) return NULL;
	for(uint k = 0; k < groupsUnder.size(); ++k)
	{
		CGroupMap *gm = dynamic_cast<CGroupMap *>(groupsUnder[k]);
		if (gm) return gm;
	}
	return NULL;
}

// ***************************************************************
CGroupContainer *CTool::isMouseOnContainer()
{
	//H_AUTO(R2_CTool_isMouseOnContainer)
	const std::vector<CInterfaceGroup *> &groupsUnder = CWidgetManager::getInstance()->getGroupsUnderPointer();
	if (groupsUnder.empty()) return NULL;
	for(uint k = 0; k < groupsUnder.size(); ++k)
	{
		CInterfaceGroup* gc = groupsUnder[k]->getParentContainer();
		if (gc)
			return static_cast< CGroupContainer* >( gc );
	}
	return NULL;
}

// ***************************************************************
void CTool::computeWorldViewRay(sint32 posX, sint32 posY, CWorldViewRay &dest)
{
	//H_AUTO(R2_CTool_computeWorldViewRay)
	CGroupMap *gm = isMouseOnWorldMap();
	if (gm)
	{
		sint32 windowX = posX - gm->getXReal();
		sint32 windowY = posY - gm->getYReal();
		CVector2f mapPos;
		gm->windowToMap(mapPos, windowX, windowY);
		if (mapPos.x >= 0.f && mapPos.y >= 0.f && mapPos.x <= 1.f && mapPos.y <= 1.f)
		{
			CVector2f worldPos;
			gm->mapToWorld(worldPos, mapPos);
			dest.Dir = -CVector::K;
			dest.Origin.set(worldPos.x, worldPos.y, 10000.f); // look from above
			dest.OnMiniMap = true;
			dest.Right = CVector(1.f, 0.f, 0.f);
			dest.Up = CVector(0.f, 1.f, 0.f);
			dest.Valid = true;
			return;
		}
		else
		{
			dest.OnMiniMap = true;
			dest.Valid = false;
			return;
		}
	}

	uint32 scrW, scrH;
	getScreenSize(scrW, scrH);
	const NL3D::CFrustum &fru = MainCam.getFrustum();
	//
	dest.Dir.x = posX / (float) scrW;
	dest.Dir.x = fru.Left + (fru.Right - fru.Left) * dest.Dir.x;
	dest.Dir.z = posY / (float) scrH;
	dest.Dir.z = fru.Bottom + (fru.Top - fru.Bottom) * dest.Dir.z;
	dest.Dir.y = fru.Near;
	//
	dest.Dir /= fru.Near;
	CMatrix camMatrix = MainCam.getMatrix();
	dest.Right = camMatrix.getI().normed();
	dest.Up = camMatrix.getK().normed();
	dest.Dir = camMatrix.mulVector(dest.Dir);
	dest.Origin = camMatrix.getPos();
	dest.OnMiniMap = false;
	dest.Valid = true;

	// When looking upward, the camera may be "snapped" to the ground
	// In this case, false intersection with the ground may be detected, so move forward
	// in ray direction to resolve that problem
	const float distBias = 0.20f;
	dest.Origin += distBias * dest.Dir;
}

// ***************************************************************
bool CTool::isInScreen(sint32 x, sint32 y)
{
	//H_AUTO(R2_CTool_isInScreen)
	uint32 scrW, scrH;
	getScreenSize(scrW, scrH);
	return x >= 0 && y >=0 && x < (sint32) scrW && y < (sint32) scrH;
}

// ***************************************************************
CTool::TRayIntersectionType CTool::getPacsType(const NLMISC::CVector &pos, float threshold, NLPACS::UGlobalPosition &destPos)
{
	//H_AUTO(R2_CTool_getPacsType)
	if (!GR) return NoIntersection;
	destPos = GR->retrievePosition(pos, threshold);
	if (destPos.InstanceId != -1)
	{
		float waterHeight = 0.0f;
		if(GR->isWaterPosition(destPos, waterHeight))
		{
			// for now, forbid to put something in water
			return InvalidPacsPos;
		}
		return ValidPacsPos;
	}
	else
	{
		return InvalidPacsPos;
	}
}

// ***************************************************************
bool CTool::raytrace(const NLMISC::CVector &segmentStart, const NLMISC::CVector &dir, NLMISC::CVector &inter)
{
	//H_AUTO(R2_CTool_raytrace)
	// try precise collision first if ray not vertical (not supported by CollisionManager)
	H_AUTO ( RZ_Client_Camera_Collision_For_R2 )
	if (dir.x != 0.f || dir.y != 0.f)
	{
		if (Landscape)
		{
			// use a shortest distance for collision manager (for speed)
			CVector segmentEnd = segmentStart + 100.f * dir;
			float dist = Landscape->getRayCollision(segmentStart, segmentEnd);
			if (dist != 1.f)
			{
				inter = segmentStart + dist * (segmentEnd - segmentStart);
				return true;
			}
		}
	}
	// else try with coarsest version of the island
	// ... else try with coarsest version of the whole island
	CPackedWorld *pw = getEditor().getIslandCollision().getPackedIsland();
	if (pw)
	{
		if (pw->raytrace(segmentStart, segmentStart + 2000.f * dir, inter))
		{
			return true;
		}
	}
	//
	return false;
}

// ***************************************************************
CTool::TRayIntersectionType CTool::computeWorldMapIntersection(float x, float y, NLMISC::CVector &inter)
{
	//H_AUTO(R2_CTool_computeWorldMapIntersection)
	const CArray2D<sint16> &heightMap = getEditor().getIslandCollision().getHeightMap();
	if (!heightMap.empty())
	{
		// here we take only the surface with a valid z, so the 'InvalidPacsPocs' case is not possible
		if (computeNearestValidSurfaceFromHeightMap(x, y, inter)) return ValidPacsPos;
		return NoIntersection;
	}
	if (!GR) return NoIntersection;
	// if heightmap not present then relies on old pacs test method...(transition code before all goes through the heightmap)
	CVector pos(x, y, 0.f);
	//
	NLPACS::UGlobalPosition pacsPos;
	TRayIntersectionType interType = getPacsType(pos, 100000.f, pacsPos);
	nlassert(interType != NoIntersection);
	CVector firstGuess = pacsPos.LocalPosition.Estimation;
	firstGuess.x = x;
	firstGuess.y = y;
	// now we have the good z, try to refine pos using the camera collision
	float deltaZ = 2.f;
	float bias = 0.001f; // if (dx == 0) and (dy == 0), the camera collision fails (not supported, visibly)
						 // workaround it by adding a small bias
	if (interType == InvalidPacsPos)
	{
		// no good z here..., so must test the whole range
		firstGuess.z = 0.f;
		deltaZ *= 1000.f;
		bias *= 1000.f;
	}
	CVector refinedStart = firstGuess + deltaZ * CVector::K + bias * CVector::I;
	CVector refinedEnd = firstGuess - deltaZ * CVector::K - bias * CVector::I;
	//
	float dist = CollisionManager->getCameraCollision(refinedStart, refinedEnd, 0.1f, false);
	if (dist == 1.f) return NoIntersection;
	//
	inter = refinedStart + dist * (refinedEnd - refinedStart);
	return interType;
}

// ***************************************************************
inline bool CTool::isIslandValidPos(const CArray2D<sint16> &heightMap, const CScenarioEntryPoints::CCompleteIsland &islandDesc, float x, float y)
{
	//H_AUTO(R2_CTool_isIslandValidPos)
	sint mapX = (sint) (x - islandDesc.XMin);
	sint mapY = (sint) (y - islandDesc.YMin);
	clamp(mapX, 0, (sint) heightMap.getWidth() - 1);
	clamp(mapY, 0, (sint) heightMap.getHeight() - 1);
	sint hmZ = heightMap(mapX, mapY);
	return hmZ < 0x7ffe;
}


// do several raytracing test on the small region until a hit is found (at boundary of zones, welding is not perfect so it helps to remove this problem)
static bool areaRaytrace(CPackedWorld &pw, const CVector &start, const CVector &end, CVector &inter, CVector *normal)
{
	//H_AUTO(R2_areaRaytrace)
	static volatile float distMax = 0.2f;
	static volatile float distStep = 0.05f;
	if (pw.raytrace(start, end, inter, NULL, normal)) return true;
	for (float dist = distStep; dist <= distMax; dist += distStep)
	{

		for (uint k = 0; k < sizeofarray(cardinals); ++k)
		{
			CVector delta = dist * cardinals[k];
			if (pw.raytrace(start + delta, end + delta, inter, NULL, normal))
			{
				inter -= delta; // remove correction
				return true;
			}
		}
	}
	return false;
}

// ***************************************************************
bool CTool::computeNearestValidSurfaceFromHeightMap(float x, float y, NLMISC::CVector &inter)
{
	//H_AUTO(R2_CTool_computeNearestValidSurfaceFromHeightMap)
	const CArray2D<sint16> &heightMap = getEditor().getIslandCollision().getHeightMap();
	nlassert (!heightMap.empty());
	//

	const CScenarioEntryPoints::CCompleteIsland *islandDesc = getEditor().getIslandCollision().getCurrIslandDesc();
	if (!islandDesc) return false;

	sint mapX = (sint) (x - islandDesc->XMin);
	sint mapY = (sint) (y - islandDesc->YMin);
	if (mapX < 0 || mapY < 0 || mapX >= (islandDesc->XMax - islandDesc->XMin) || mapY >= (islandDesc->YMax - islandDesc->YMin)) return false;
	sint hmZ = heightMap(mapX, mapY);
	if (hmZ >= 0x7ffe) return false; // not an accessible pos
	if (!isIslandValidPos(heightMap, *islandDesc, x + 0.5f, y) ||
		!isIslandValidPos(heightMap, *islandDesc, x - 0.5f, y) ||
		!isIslandValidPos(heightMap, *islandDesc, x, y + 0.5f) ||
		!isIslandValidPos(heightMap, *islandDesc, x, y - 0.5f)) return false;
	float z = 1.f + 2.f * hmZ;
	// this is a possibly valid position
	// compute nearest surface from here, and see if not far from the intersection
	CPackedWorld *pw = getEditor().getIslandCollision().getPackedIsland();
	nlassert(pw); // packed world should be always present when heightmap is
	//
	CVector inter1;
	CVector inter2;
	CVector origin(x, y, z);
	CVector bias(0.f, 0.f, 0.1f);
	NLMISC::CVector normal1;
	NLMISC::CVector normal2;
	static volatile float minAngleSin = 0.2f;
	bool inter1Found = areaRaytrace(*pw, origin - bias, origin + 10000.f * CVector::K, inter1, &normal1);
	bool inter2Found = areaRaytrace(*pw, origin + bias, origin - 10000.f * CVector::K, inter2, &normal2);
	inter1Found = inter1Found && normal1.z >= minAngleSin;
	inter2Found = inter2Found && normal2.z >= minAngleSin;
	if (!inter1Found && !inter2Found) return false;
	if (inter1Found && inter2Found)
	{
		// because z in heightmap in usually a 'ceil' of real height, tends to favor surface below
		// to avoid special case were the cliff top is very close to the ground. add a bias for this (equal
		// to the heightmap precision)
		origin.z -= 2.f;
		inter = ((inter1 - origin).norm() < (inter2 - origin).norm()) ? inter1 : inter2;
	}
	else if (inter1Found)
	{
		inter = inter1;
	}
	else
	{
		inter = inter2;
	}
	return true;
}

// ***************************************************************
bool CTool::isValid2DPos(const NLMISC::CVector2f &pos)
{
	//H_AUTO(R2_CTool_isValid2DPos)
	return getEditor().getIslandCollision().isValidPos(CVector2f(pos.x, pos.y));
}

// ***************************************************************
CTool::TRayIntersectionType CTool::computeLandscapeRayIntersection(const CWorldViewRay &worldViewRay, NLMISC::CVector &inter)
{
	//H_AUTO(R2_CTool_computeLandscapeRayIntersection)
	if (!worldViewRay.Valid) return NoIntersection;
	if (worldViewRay.OnMiniMap)
	{
		return computeWorldMapIntersection(worldViewRay.Origin.x, worldViewRay.Origin.y, inter);
	}
	// Compute collision point with landscape
	if (!raytrace(worldViewRay.Origin, worldViewRay.Dir, inter))
	{
		// NB following code is intended to fix the problem of holes betweens zone in the packed collision
		static volatile float bias = 0.15f;
		bool found = false;
		for (uint k = 0; k < sizeofarray(cardinals) && !found; ++k)
		{
			CVector delta = bias * (cardinals[k].x * worldViewRay.Right + cardinals[k].y * worldViewRay.Up);
			found = raytrace(worldViewRay.Origin + delta, worldViewRay.Dir, inter);
		}
		if	(!found)
		{
			// Because of holes near zones boundaries, add a small bias
			return NoIntersection;
		}
	}
	//
	const CArray2D<sint16> &heightMap = getEditor().getIslandCollision().getHeightMap();
	if (!heightMap.empty())
	{
		// if heightmap is present, use it because it gives us more reliable information
		CVector surfPos;
		if (!computeNearestValidSurfaceFromHeightMap(inter.x, inter.y, surfPos)) return InvalidPacsPos;
		static volatile float threshold = 2.f;
		return (inter - surfPos).norm() < threshold ? ValidPacsPos : InvalidPacsPos;
	}

	// get pacs type at intersection
	if (!GR)
	{
		return NoIntersection;
	}
	else
	{
		// see if pacs collisions are ok at that pos
		NLPACS::UGlobalPosition dummyPos;
		return getPacsType(inter, 2.f, dummyPos);
	}
}

// **********************************************
void CTool::handleMouseOverPlayer(bool over)
{
	//H_AUTO(R2_CTool_handleMouseOverPlayer)
	// If the mouse is over the player make the player transparent
	CCDBNodeLeaf *pNL = _UserCharFade ? &*_UserCharFade
		: &*(_UserCharFade = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:USER_CHAR_FADE", false));
	if ((pNL != NULL) && (pNL->getValue32() == 1) && UserEntity->selectable())
	{
		// If the nearest entity is the player, hide!
		if (over)
			UserEntity->makeTransparent(true);
		else
			UserEntity->makeTransparent(false);
	}
	else
		UserEntity->makeTransparent(false);
}

// ***************************************************************
CInstance *CTool::checkInstanceUnderMouse(IDisplayerUIHandle **miniMapHandle /*= NULL*/)
{
	//H_AUTO(R2_CTool_checkInstanceUnderMouse)
	// Get the pointer position (in pixels)
	if (miniMapHandle)
	{
		*miniMapHandle = NULL;
	}
	sint32 x, y;
	getMousePos(x, y);
	if (isMouseOnUI())
	{
		CGroupMap *gm = isMouseOnWorldMap();
		if (gm)
		{
			CInstance *inst = getEditor().getSelectedInstance();
			CDisplayerVisual *currSelectedDV = inst ? inst->getDisplayerVisual() : NULL;
			sint32 mouseXInWindow;
			sint32 mouseYInWindow;
			gm->getCorner(mouseXInWindow, mouseYInWindow, Hotspot_BL);
			mouseXInWindow = x - mouseXInWindow;
			mouseYInWindow = y - mouseYInWindow;
			IDisplayerUIHandle *bestCandidate = NULL;
			sint8 bestCandidateLayer = -128;
			// see if the element is under the mouse
			const std::vector<CCtrlBase *> &ctrlsUnder = CWidgetManager::getInstance()->getCtrlsUnderPointer();
			for(sint k = (sint)ctrlsUnder.size() - 1; k >= 0; --k)
			{
				IDisplayerUIHandle *handle = dynamic_cast<IDisplayerUIHandle *>(ctrlsUnder[k]);
				if (handle != NULL)
				{
					CDisplayerVisual *dv = handle->getDisplayedInstance().getDisplayerVisual();
					if (ctrlsUnder[k]->getRenderLayer() > bestCandidateLayer ||
						(ctrlsUnder[k]->getRenderLayer() == bestCandidateLayer && currSelectedDV == dv)
					   )
					{

						if (dv && dv->isSelectable())
						{
							// test for real hit
							if (handle->contains(mouseXInWindow, mouseYInWindow))
							{
								bestCandidate = handle;
								bestCandidateLayer = ctrlsUnder[k]->getRenderLayer();
							}
						}
					}
				}
			}
			if (bestCandidate)
			{
				if (miniMapHandle)
				{
					*miniMapHandle = bestCandidate;
				}
				return &(bestCandidate->getDisplayedInstance());
			}
		}
		// else asks lua if there's a selectable instance under the mouse
		// (so that we don't need to know of the UI ..., world map is a special case)
		{
			CLuaState &ls = getEditor().getLua();
			CLuaStackRestorer lsr(&ls, 0);
			if (getEditor().callEnvMethod("getInstanceIdFromUIUnderMouse", 0, 1))
			{
				if (ls.isString(1))
				{
					CInstance *inst = getEditor().getInstanceFromId(ls.toString(1));
					if (inst) return inst;
				}
			}
		}
	}
	else if (!IsMouseFreeLook() && !CWidgetManager::getInstance()->getCapturePointerLeft() && !CWidgetManager::getInstance()->getCapturePointerRight())
	{
		// Over the screen ?
		if (isInScreen(x, y))
		{
			// Get the pointer position (in float)
			float cursX, cursY;
			cursX = x / (float) getScreenWidth();
			cursY = y / (float) getScreenHeight();

			// Get the entities under position
			bool	isPlayerUnderCursor;
			CInstance *inst = getEditor().getInstanceUnderPos(cursX,  cursY,  2000.f,  isPlayerUnderCursor);
			handleMouseOverPlayer(isPlayerUnderCursor);
			return inst;
		}
	}
	return NULL;
}


// ***************************************************************
void CTool::handleMouseOverInstance(const char *cursorDefault, const char *cursorOverUnselectedInstance, const char *cursorOverSelectedInstance)
{
	//H_AUTO(R2_CTool_handleMouseOverInstance)
	setMouseCursor(cursorDefault);
	CInstance *instanceUnder = checkInstanceUnderMouse();
	if (!instanceUnder)
	{
		getEditor().setFocusedInstance(NULL);
		return;
	}

	getEditor().setFocusedInstance(instanceUnder);
	if (instanceUnder != getEditor().getSelectedInstance())
	{
		setMouseCursor(cursorOverUnselectedInstance);
	}
	else
	{
		// indicate that the user can move the instance
		setMouseCursor(cursorOverSelectedInstance);
	}
}

// ***************************************************************
bool CTool::onMouseRightButtonClicked()
{
	//H_AUTO(R2_CTool_onMouseRightButtonClicked)
	if (!getEditor().getFocusedInstance())
	{
		getEditor().setSelectedInstance(NULL);
		return false;
	}
	getEditor().setSelectedInstance(getEditor().getFocusedInstance());
	getEditor().displayContextMenu();
	return true;
}

// ***************************************************************
bool CTool::defaultRightButtonDownHandling()
{
	//H_AUTO(R2_CTool_defaultRightButtonDownHandling)
	/*if (!getEditor().getFocusedInstance())
	{
		// cancel any current selection, this will cause the default context menu to beshown
		getEditor().setSelectedInstance(NULL);
		return false;
	}
	getEditor().setSelectedInstance(getEditor().getFocusedInstance());
	getEditor().displayContextMenu();
	return true;	*/
	return false;
}

// ***************************************************************
void CTool::captureMouse()
{
	//H_AUTO(R2_CTool_captureMouse)
	CGroupMap *gm = isMouseOnWorldMap();
	if (gm)
	{
		CWidgetManager::getInstance()->setCapturePointerLeft(gm);
	}
	else
	{
		UserControls.captureMouse();
		CWidgetManager::getInstance()->enableMouseHandling(false);
	}
	CWidgetManager::getInstance()->setContextHelpActive(false);
	_MouseCaptured = true;
}

// ***************************************************************
void CTool::releaseMouse()
{
	//H_AUTO(R2_CTool_releaseMouse)
	CWidgetManager::getInstance()->setCapturePointerLeft(NULL);
	UserControls.releaseMouse();
	CWidgetManager::getInstance()->enableMouseHandling(true);
	CWidgetManager::getInstance()->setContextHelpActive(true);
	_MouseCaptured = false;
}

// *********************************************************************************************************
bool CTool::isMouseCaptured()
{
	//H_AUTO(R2_CTool_isMouseCaptured)
	return _MouseCaptured;
}

// *********************************************************************************************************
void CTool::setMouseCursor(const char *cursorTexture)
{
	//H_AUTO(R2_CTool_setMouseCursor)
	CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
	if(cursor)
	{
		cursor->setCursor(cursorTexture);
	}
}

// ***************************************************************
bool CTool::handleEvent(const NLGUI::CEventDescriptor &event)
{
	//H_AUTO(R2_CTool_handleEvent)
	bool handled = false;
	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		NLGUI::CEventDescriptorMouse &eventDesc = (NLGUI::CEventDescriptorMouse&)event;
		switch(eventDesc.getEventTypeExtended())
		{
			case NLGUI::CEventDescriptorMouse::mousemove:
				handled = onMouseMove();
			break;
			case NLGUI::CEventDescriptorMouse::mouseleftdown:
				handled = onMouseLeftButtonDown();
				//if (handled) nlwarning("onMouseLeftButtonDown handled");
			break;
			case NLGUI::CEventDescriptorMouse::mouserightdown:
				handled = onMouseRightButtonDown();
				//if (handled) nlwarning("onMouseRightButtonDown handled");
			break;
			case NLGUI::CEventDescriptorMouse::mouseleftup:
				handled = onMouseLeftButtonUp();
				//if (handled) nlwarning("onMouseLeftButtonUp handled");
			break;
			case NLGUI::CEventDescriptorMouse::mouserightup:
				handled = onMouseRightButtonUp();
				//if (handled) nlwarning("onMouseRightButtonUp handled");
			break;
		}
	}
	if (event.getType() == NLGUI::CEventDescriptor::system)
	{
		const NLGUI::CEventDescriptorSystem &eds = (const NLGUI::CEventDescriptorSystem &) event;
		if (eds.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::setfocus)
		{
			const NLGUI::CEventDescriptorSetFocus &edsf = (const NLGUI::CEventDescriptorSetFocus &) eds;
			if (edsf.hasFocus() == true)
			{
				onFocusGained();
			}
		}
	}
	return handled;
}

// ***************************************************************
CDynamicMapClient &CTool::getDMC()
{
	//H_AUTO(R2_CTool_getDMC)
	return  getEditor().getDMC();
}

// ***************************************************************
void CTool::handleWorldMapAutoPan(sint32 &outDx, sint32 &outDy)
{
	//H_AUTO(R2_CTool_handleWorldMapAutoPan)
	outDx = outDy = 0;
	CGroupMap *gm = getWorldMap();
	if (!gm) return;
	// if not handled at previous frame then reset
	uint64 currFrameCounter = Driver->getSwapBufferCounter();
	if (currFrameCounter == _AutoPanLastHandlingFrame) return; // already handled this frame
	if (_AutoPanLastHandlingFrame <= currFrameCounter - 2)
	{
		_AutoPanDelay = 0;
		_NumPans = 0;
	}
	// see if mouse pos is valid
	sint32 mx, my;
	getMousePos(mx, my);
	if (!gm->isIn(mx, my))
	{
		return;
	}
	sint32 autoPanBorder = CV_MapAutoPanBorder.get();
	sint32 x, y, w, h;
	gm->computeMapRectInsideGroup(x, y, w, h);
	sint32 dx = (gm->getXReal() + w - mx) <= autoPanBorder ? 1 : 0;
	dx -= (mx - gm->getXReal()) <= autoPanBorder ? 1 : 0;
	sint32 dy = (gm->getYReal() + h - my) <= autoPanBorder ? 1 : 0;
	dy -= (my - gm->getYReal()) <= autoPanBorder ? 1 : 0;
	if (!dx && !dy)
	{
		_AutoPanDelay = 0;
		_NumPans = 0;
		return;
	}
	_AutoPanLastHandlingFrame = currFrameCounter;
	// select delay (shorter in fast-pan mode)
	sint32 delay = (_NumPans > CV_MapAutoFastPanNumTicks.get()) ? CV_MapAutoFastPanDeltaInMs.get()
																: CV_MapAutoPanDeltaInMs.get();

	_AutoPanDelay += DT64;
	if (_AutoPanDelay >= delay)
	{
		_AutoPanDelay = _AutoPanDelay % delay;
		sint32 panDelta = CV_MapAutoPanSpeedInPixels.get();
		outDx = panDelta * dx;
		outDy = panDelta * dy;
		gm->pan(outDx, outDy);
		++ _NumPans; // one step toward 'fast pan' mode
		return;
	}
	return;
}

// ***************************************************************
int CTool::luaIsPickTool(CLuaState &ls)
{
	//H_AUTO(R2_CTool_luaIsPickTool)
	CLuaIHM::checkArgCount(ls, "isPickTool", 0);
	ls.push(isPickTool());
	return 1;
}

// ***************************************************************
void CTool::setContextHelp(const ucstring &contextHelp)
{
	//H_AUTO(R2_CTool_setContextHelp)
	// forward the call to lua (all ui handling done bye lua)
	CLuaState &ls = getEditor().getLua();
	CLuaStackChecker lsc(&ls);
	CLuaIHM::push(ls, contextHelp);
	getEditor().callEnvMethod("setToolContextHelp", 1, 0);
}

// ***************************************************************
NLMISC::CRGBA CTool::getInvalidPosColor()
{
	//H_AUTO(R2_CTool_getInvalidPosColor)
	double duration = CV_InaccessiblePosAnimDurationInMS.get();
	if (duration <= 0)  duration = 0.1;
	return blend(CV_InaccessiblePosColor0.get(),
				 CV_InaccessiblePosColor1.get(),
				 0.5f + 0.5f * (float) cos(NLMISC::Pi * fmod((double) T1, duration) / duration));

}


} // R2
