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
#include "displayer_visual_road.h"
#include "instance.h"
#include "editor.h"
#include "../global.h"
//


using namespace NLMISC;
using namespace NL3D;

namespace R2
{

/*
// *********************************************************************************************************
CPrimRender2::CPrimRender2()
{
	_VertexMeshs.setShapeName("road_flag.shape");
	_EdgeMeshs.setShapeName("instance_link.shape");
	_VertexScale = 1.f;
}

// *********************************************************************************************************
void CPrimRender2::setVertexShapeName(const std::string &name)
{
	_VertexMeshs.setShapeName(name);
}

// *********************************************************************************************************
void CPrimRender2::setEdgeShapeName(const std::string &name)
{
	_EdgeMeshs.setShapeName(name);
}

// *********************************************************************************************************
void CPrimRender2::setPoints(const std::vector<NLMISC::CVector> &wp, bool lastIsValid, bool closed)
{
	uint iclosed = closed ? 1 : 0;
	_VertexMeshs.resize(wp.size());
	_EdgeMeshs.resize(std::max((sint) (wp.size() - 1 + iclosed), (sint) 0));
	for(uint k = 0; k < wp.size(); ++k)
	{
		if (!_VertexMeshs[k].empty())
		{
			_VertexMeshs[k].setTransformMode(UTransform::DirectMatrix);
			CMatrix flagMat;
			flagMat.setPos(wp[k]);
			flagMat.setScale(_VertexScale);
			_VertexMeshs[k].setMatrix(flagMat);
			_VertexMeshs[k].enableCastShadowMap(true);
		}
		if ((sint) k < (sint) (wp.size() - 1 + iclosed))
		{
			if (!_EdgeMeshs[k].empty())
			{
				if ((sint) k == (sint) (wp.size() - 2) && !lastIsValid)
				{
					_EdgeMeshs[k].hide();
				}
				else
				{
					CVector I = wp[(k + 1) % wp.size()] - wp[k];
					CVector INormed = I.normed();
					CVector K = (CVector::K - (CVector::K * INormed) * INormed).normed();
					CVector J = K ^ INormed;
					CMatrix connectorMat;
					static volatile float scale =0.5f;
					connectorMat.setRot(I, scale * J, scale * K);
					connectorMat.setPos(wp[k]);
					_EdgeMeshs[k].setTransformMode(UTransform::DirectMatrix);
					_EdgeMeshs[k].setMatrix(connectorMat);
					_EdgeMeshs[k].show();
				}
			}
		}
	}
}

// *********************************************************************************************************
void CPrimRender2::setEmissive(NLMISC::CRGBA color)
{
	_VertexMeshs.setEmissive(color);
	_EdgeMeshs.setEmissive(color);
}
*/

/*

// *********************************************************************************************************
CDisplayerVisualRoad::~CDisplayerVisualRoad()
{

}

// *********************************************************************************************************
void CDisplayerVisualRoad::onCreate()
{
	rebuild();
	CVisualDisplayer::onCreate();
}


// *********************************************************************************************************
void CDisplayerVisualRoad::rebuild()
{
	const CObject *points = getObject(&getProps(), "Points");
	static volatile bool wantDump = false;
	if (wantDump)
	{
		points->dump();
	}
	if (!points) return;
	std::vector<CVector> wayPoints;
	wayPoints.resize(points->getSize());
	for(uint k = 0; k < points->getSize(); ++k)
	{
		wayPoints[k] = getVector(points->getValue((uint32) k));
	}
	_Road.setWayPoints(wayPoints, true);
}
*/

} // R2