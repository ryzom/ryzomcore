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
#include "tool_new_vertex.h"
#include "tool_draw_prim.h"
#include "editor.h"
#include "displayer_visual_group.h"
//
#include "nel/misc/i18n.h"

using namespace NLMISC;

namespace R2
{


// *********************************************************************************************************
CToolNewVertex::CToolNewVertex() : CToolChoosePos(-1, "curs_create.tga", "curs_create_vertex_invalid.tga"), _CurrEdge(-1), _CurrPos(CVector::Null)
{
	lockMultiPos();
}

// *********************************************************************************************************
bool CToolNewVertex::isValidChoosePos(const CVector2f &pos) const
{
	//H_AUTO(R2_CToolNewVertex_isValidChoosePos)
	if (!isValid2DPos(pos)) return false;
	// pos must be on one edge of the road
	CInstance *selection = getEditor().getSelectedInstance();
	if (!selection) return false;
	IDisplayerUIHandle *miniMapHandle;
	CInstance *underMouse = checkInstanceUnderMouse(&miniMapHandle);
	if (underMouse != selection) return false;
	if (miniMapHandle)
	{
		if (miniMapHandle->isEdge())
		{
			_CurrEdge = (sint) miniMapHandle->getEdgeIndex();
			return true;
		}
		else
		{
			return false;
		}
	}
	CDisplayerVisualGroup *dvg = dynamic_cast<CDisplayerVisualGroup *>(selection->getDisplayerVisual());
	if (!dvg) return false;
	_CurrEdge = dvg->isOnEdge(CVector2f(pos.x, pos.y));
	_CurrPos = pos;
	return _CurrEdge != -1;
}

// *********************************************************************************************************
void CToolNewVertex::commit(const NLMISC::CVector &createPosition, float /* createAngle */)
{
	//H_AUTO(R2_CToolNewVertex_commit)
	CInstance *selection = getEditor().getSelectedInstance();
	if (!selection) return;
	// check that there is room left for a new vertex
	CObject *points = selection->getObjectTable()->getAttr("Points");
	if (!points) return;
	if (points->getSize() >= CToolDrawPrim::PrimMaxNumPoints)
	{
		getEditor().callEnvMethod("noMoreRoomLeftInPrimitveMsg", 0, 0);
		return;
	}
	//
	if (!selection->getDisplayerVisual()) return;
	getDMC().newAction(NLMISC::CI18N::get(selection->isKindOf("Road") ? "uiR2EDInsertNewWayPointAction" : "uiR2EDInsertNewZoneVertexAction"));
	CObject *wp = getDMC().newComponent(selection->isKindOf("Road") ? "WayPoint" : "RegionVertex");
	if (!wp) return;
	wp->setObject("Position", buildVector(CVectorD(createPosition - selection->getDisplayerVisual()->getWorldPos())));
	if (_CurrEdge == (sint) (points->getSize() - 1))
	{
		getDMC().requestInsertNode(selection->getId(), "Points", -1, "", wp);
	}
	else
	{
		getDMC().requestInsertNode(selection->getId(), "Points", _CurrEdge + 1, "", wp);
	}
	delete wp;
}

// *********************************************************************************************************
const char *CToolNewVertex::getToolUIName() const
{
	//H_AUTO(R2_CToolNewVertex_getToolUIName)
	return "new_vertex";
}

// *********************************************************************************************************
bool CToolNewVertex::onDeleteCmd()
{
	//H_AUTO(R2_CToolNewVertex_onDeleteCmd)
	CTool::TSmartPtr hold(this);
	getEditor().setCurrentTool(NULL);
	return false; // don't handle event because the current zone will be deleted
}




} // R2
