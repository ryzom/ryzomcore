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

#ifndef R2_PRIM_RENDER_H
#define R2_PRIM_RENDER_H

#include "displayer_visual.h"
#include "mesh_array.h"
#include "../custom_matrix.h"
//
#include "../interface_v3/group_map.h"


namespace NLGUI
{
	class CLuaObject;
	class CCtrlPolygon;
}

class CEntityCL;

namespace R2
{

// *******************************************************************************
class CVertexLook
{
public:
	CStringShared	DecalTexture;  // "" for no decal
	float			DecalSize;
	NLMISC::CRGBA	DecalColor;
	float			DecalDistToEdgeDecal;
	//
	CStringShared	WorldMapTexture;
	NLMISC::CRGBA	WorldMapColor;
public:
	CVertexLook()
	{
		DecalSize = 1.f;
		DecalDistToEdgeDecal = 0.f;
		DecalColor = CRGBA::White;
		WorldMapColor = CRGBA::White;
	};
	void init(const CLuaObject &params);
};

inline bool operator == (const CVertexLook &lhs, const CVertexLook &rhs)
{
	return lhs.DecalTexture == rhs.DecalTexture &&
	lhs.DecalSize == rhs.DecalSize &&
	lhs.DecalColor == rhs.DecalColor &&
	lhs.DecalDistToEdgeDecal == rhs.DecalDistToEdgeDecal &&
	lhs.WorldMapTexture == rhs.WorldMapTexture &&
	lhs.WorldMapColor == rhs.WorldMapColor;
}
inline bool operator != (const CVertexLook &lhs, const CVertexLook &rhs) { return !(lhs == rhs); }


// *******************************************************************************
class CEdgeLook
{
public:
	enum TWrapMode { Repeat = 0,
					 Centered,
					 Scaled, // not supported for decals, resume to 'Repeat' ...
					 WrapModeCount
					};
	CStringShared	ShapeName;    // "" for no mesh for edges
	float			ShapeScale;
	//
	CStringShared	DecalTexture; // "" for no decals for edges
	float			DecalUScale;
	float			DecalWidth;
	TWrapMode		DecalWrapMode;
	NLMISC::CRGBA	DecalColor;
	bool			DecalFiltered;
	//
	CStringShared	WorldMapTexture;
	float			WorldMapUScale;
	float			WorldMapWidth;
	TWrapMode		WorldMapWrapMode;
	NLMISC::CRGBA	WorldMapColor;
	bool			WorldMapFiltered;
	//
public:
	CEdgeLook()
	{
		DecalUScale = 1.f;
		WorldMapUScale = 1.f;
		DecalWidth = 1.f;
		ShapeScale = 1.f;
		DecalColor = CRGBA::White;
		DecalFiltered = true;
		WorldMapWidth = 1.f;
		WorldMapColor = CRGBA::White;
		DecalWrapMode = Repeat;
		WorldMapWrapMode = Repeat;
		WorldMapFiltered = true;
	}
	void init(const CLuaObject &params);
};

inline bool operator==(const CEdgeLook &lhs, const CEdgeLook &rhs)
{
	return lhs.ShapeName == rhs.ShapeName &&
	lhs.ShapeScale == rhs.ShapeScale &&
	lhs.DecalTexture == rhs.DecalTexture &&
	lhs.DecalUScale == rhs.DecalUScale &&
	lhs.DecalWidth == rhs.DecalWidth &&
	lhs.DecalWrapMode == rhs.DecalWrapMode &&
	lhs.DecalColor == rhs.DecalColor &&
	lhs.DecalFiltered == rhs.DecalFiltered &&
	lhs.WorldMapTexture == rhs.WorldMapTexture &&
	lhs.WorldMapWidth == rhs.WorldMapWidth &&
	lhs.WorldMapColor == rhs.WorldMapColor &&
	lhs.WorldMapUScale == rhs.WorldMapUScale &&
	lhs.WorldMapWrapMode == rhs.WorldMapWrapMode &&
	lhs.WorldMapFiltered == rhs.WorldMapFiltered;


}

inline bool operator!=(const CEdgeLook &lhs, const CEdgeLook &rhs) { return !(lhs == rhs); }


// *******************************************************************************
// look of a primitive
class CPrimLook
{
public:
	enum TShape { Star = 0, PolyLine, ClosedPolyLine, ShapeCount };
	//
	TShape		Shape;
	// vertices
	CStringShared	VertexShapeName;     // "" for no mesh at vertices
	float			VertexShapeScale;
	CVertexLook		VertexLook;
	CVertexLook		FirstVertexLook;
	CEdgeLook		EdgeLook;
	bool			LastEdgeIsValid;
	bool			ClipDownFacing;
	//
public:
	CPrimLook()
	{
		Shape = Star;
		VertexShapeScale = 1.f;
		LastEdgeIsValid = true;
		ClipDownFacing = false;
	}
	// init parameters from a lua table
	void init(const CLuaObject &params);
};

bool operator == (const CPrimLook &lhs, const CPrimLook &rhs);
inline bool operator != (const CPrimLook &lhs, const CPrimLook &rhs) { return !(lhs == rhs); }

/** class to display a primitive (zone, roads, groups ...) in scene, and possibly on the world map
  */
class CPrimRender : public CGroupMap::IDeco
{
public:
	CPrimRender();
	virtual ~CPrimRender();
	//
	void			 setLook(const CPrimLook &look);
	const CPrimLook &getLook() const { return _Look; }
	// set vertices of the primitive.
	void			 setVertices(const std::vector<NLMISC::CVector> &vertices);
	void			 setVertices(const std::vector<NLMISC::CVector2f> &vertices);
	const std::vector<NLMISC::CVector> &getVertices() const { return _Vertices; }
	void			 setEmissive(NLMISC::CRGBA emissive);
	void			 addDecalsToRenderList();
	const CMeshArray &getVerticesShapeInstance() const { return _VertexShapeInstances; }
	// test decals against a 2D position
	bool			  contains(const NLMISC::CVector2f &pos) const;
	// test edges against a 2D position
	sint			  isOnEdge(const NLMISC::CVector2f &pos) const;
	// clear content
	void			 clear();
	// set world map polygon color
	void			 setWorldMapPolyColor(NLMISC::CRGBA color);
	// Return view in the world map associated with the nth vertex (or NULL if not added in the map)
	CViewBitmap     *getWorldMapVertexView(uint index) const;
	CCtrlPolygon	*getWorldMapPoly() const { return _WorldMapPoly; }
	//
	bool			 isAddedToWorldMap() const { return _AddedToWorldMap; }

	/** For derivers : build a new CCtrlPolygon (or maybe a derived class)
	  */
	virtual CCtrlPolygon *newCtrlPolygon() const;
	/** For derivers : build a new CCtrlQuad for display of edges on the worldmap (or maybe a derived class)
	  */
	virtual CCtrlQuad *newCtrlQuad(uint edgeIndex) const;

	/** Set a custom matrix to compute uvs for both decal edge & world map edge
	  * Useful to project the accessibility texture, to show the user were he can't draw / move a primitive
	  */
	void setCustomWorldMapEdgeUVMatrix(bool on, const NLMISC::CMatrix &matrix = NLMISC::CMatrix::Identity);
	void setCustomDecalEdgeUVMatrix(bool on, const NLMISC::CMatrix &matrix = NLMISC::CMatrix::Identity);

	// set visibility in both scene and world map
	void setActive(bool active);
	bool getActive() const { return _Active; }

private:
	CPrimLook					   _Look;	//
	std::vector<NLMISC::CVector>   _Vertices;
	//
	CMeshArray			 _VertexShapeInstances;
	CMeshArray			 _EdgeShapeInstances;
	std::vector<CDecal::TSmartPtr>   _VertexDecals;
	std::vector<CDecal::TSmartPtr>	 _EdgeDecals;     // Decal of edges on landscape
	NLMISC::CRGBA					 _Emissive;
	sint							 _NumEdges;
	float							 _InvWorldTextureWidth;
	CCustomMatrix					 _CustomWorldMapEdgeUVMatrix;
	CCustomMatrix					 _CustomDecalEdgeUVMatrix;
	//
	bool							 _Active;
private:
	void update();
	void updatePos();
	void updateEdge(NL3D::UInstance edge, const NLMISC::CVector &start, const NLMISC::CVector &end);
	void updateEdgeDecal(CDecal &edgeDecal, const NLMISC::CVector &start, const NLMISC::CVector &end, float distToStartVertex, float distToEndVertex);
	void forceSetEmissive(NLMISC::CRGBA emissive);
	// world map display
	void setWorldMapNumVertices(uint count);
	void setWorldMapNumEdges(uint count);
	void updateWorldMapDisplay();
protected:
	// from CGroupMap::IDeco
	virtual void onAdd(CGroupMap &owner);
	virtual void onRemove(CGroupMap &owner);
	virtual void onPreRender(CGroupMap &owner);
	virtual void onUpdate(CGroupMap &owner);
private:
	bool						_AddedToWorldMap;
	std::vector<CViewBitmap *>	_WorldMapVertices;
	std::vector<CCtrlQuad *>	_WorldMapEdges;
	CCtrlPolygon				*_WorldMapPoly;
};

} // R2

#endif
