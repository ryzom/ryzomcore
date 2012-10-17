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
/*
#include "tool_draw_road.h"
#include "game_share/object.h"
#include "dmc/idmc.h"
#include "editor.h"
//
#include "nel/misc/vectord.h"


using namespace NLMISC;

namespace R2
{

// ***************************************************************
CToolDrawRoad::CToolDrawRoad()
{
	_NumWayPoints = 0;
	_ValidPos = false;
}

// ***************************************************************
void CToolDrawRoad::updateAfterRender()
{
}

void CToolDrawRoad::cancel()
{
	// no-op there
}


// ***************************************************************
void CToolDrawRoad::updateBeforeRender()
{
	// Build vector for direction pointed by mouse in world
	sint32 mouseX,  mouseY;
	getMousePos(mouseX,  mouseY);
	if (!isInScreen(mouseX,  mouseY))
	{
		// mouse not in screen so don't display the last way point
		_WayPoints.resize(_NumWayPoints);
		_Road.setWayPoints(_WayPoints, true);
		return;
	}
	//
	CTool::CWorldViewRay worldViewRay;
	//
	computeWorldViewRay(mouseX,  mouseY,  worldViewRay);
	//
	CVector wpPos; // the pos where the ghost will be shown
	CVector inter;     // intersection of view ray with landscape
	_ValidPos = false;
	switch(computeLandscapeRayIntersection(worldViewRay,  inter))
	{
		case NoIntersection:
			if (worldViewRay.OnMiniMap)
			{
				_WayPoints.resize(_NumWayPoints);
				_Road.setWayPoints(_WayPoints, true);
				return;
			}
			// no collision,  can't drop entity
			wpPos = worldViewRay.Origin + 3.f * worldViewRay.Dir;
		break;
		case ValidPacsPos:
			wpPos = inter;
			_ValidPos = true; // good pos to drop entity
		break;
		case InvalidPacsPos:
			wpPos = inter;
		break;
		default:
			nlassert(0);
		break;
	}
	//
	_WayPoints.resize(_NumWayPoints + 1);
	_WayPoints.back() = wpPos;
	_Road.setWayPoints(_WayPoints, _ValidPos);
	// change mouse depending on result
	if (!isMouseOnUI())
	{
		setMouseCursor(_ValidPos ? "curs_create.tga" : "curs_stop.tga");
	}
	else
	{
		setMouseCursor("curs_create.tga");
	}
}

// ***************************************************************
bool CToolDrawRoad::onMouseLeftButtonClicked()
{
	if (_ValidPos)
	{
		++ _NumWayPoints;
	}
	startDoubleClickCheck();
	return true;
}

// ***************************************************************
bool CToolDrawRoad::onMouseLeftButtonDown()
{
	if (!checkDoubleClick()) return false;
	_WayPoints.resize(_NumWayPoints);
	if (_WayPoints.empty()) return true;
	// send network command to create a new road
	CObject *desc = getDMC().newComponent("Road");
	if (desc)
	{
		static volatile bool wantDump = false;
		if (wantDump)
		{
			desc->dump();
		}
		CObject *points = desc->getAttr("Points");
		if (points)
		{
			for(uint k = 0; k < _WayPoints.size(); ++k)
			{
				CObject *wp = getDMC().newComponent("WayPoint");
				if (!wp) continue;
				wp->setObject("Position", buildVector(CVectorD(_WayPoints[k])));
				points->insert("", wp, -1);
				if (wantDump)
				{
					desc->dump();
				}
			}
		}
		// send creation command
		// tmp : static npc counter
		static int roadCounter = 0;
		// add in component list of default feature
		if (getEditor().getDefaultFeature())
		{
			getDMC().requestInsertNode(getEditor().getDefaultFeature()->getId(),
									   "Components",
									   -1,
									   toString("ROAD_%d", roadCounter++),
									   desc);
		}
		//
		delete desc;
	}
	getEditor().setCurrentTool(NULL);
	return true;
}


// ***************************************************************
bool CToolDrawRoad::onMouseRightButtonClicked()
{
	// cancel the drawing
	getEditor().setCurrentTool(NULL);
	return true;
}


} // R2
*/
