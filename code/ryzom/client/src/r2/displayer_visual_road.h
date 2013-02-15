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

#ifndef R2_VISUAL_DISPLAYER_ROAD_H
#define R2_VISUAL_DISPLAYER_ROAD_H

#include "displayer_visual.h"
//
#include "mesh_array.h"


class CEntityCL;


namespace R2
{

/** class to display a primitive (zone or roads)
  * Used by both CDisplayerVisualRoad && CToolDrawRoad
  */

	/*
class CPrimRender2
{
public:
	CPrimRender2();
	void setVertexShapeName(const std::string &name);
	void setEdgeShapeName(const std::string &name);
	// set zway points for the road. This also update the content of the scene
	void setPoints(const std::vector<NLMISC::CVector> &wp, bool lastIsValid, bool closed);
	void setVertexScale(float scale) { _VertexScale = scale; }
	//
	const CMeshArray &getVertices() const { return _VertexMeshs; }
	const CMeshArray &getEdges() const { return _EdgeMeshs; }
	void  setEmissive(NLMISC::CRGBA color);
private:
	float	   _VertexScale;
	CMeshArray _VertexMeshs;
	CMeshArray _EdgeMeshs;
};
*/

/*
class CDisplayerVisualRoad : public CDisplayerVisual
{
public:
	NLMISC_DECLARE_CLASS(R2::CDisplayerVisualRoad);
	// dtor
	~CDisplayerVisualRoad();
	virtual void onCreate();
	// from ISelectableObject
	virtual bool			isSelectable() const { return false; }
private:
	CRoad _Road;
private:
	void rebuild();
};
*/

} // R2

#endif
