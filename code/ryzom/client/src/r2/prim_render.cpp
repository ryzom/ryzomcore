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
#include "prim_render.h"
#include "tool.h"
#include "r2_config.h"
//
#include "nel/misc/vector_2f.h"
#include "nel/misc/time_nl.h"
//
#include "nel/gui/view_bitmap.h"
#include "nel/gui/ctrl_quad.h"
#include "nel/gui/ctrl_polygon.h"
#include "../interface_v3/interface_manager.h"
#include "nel/gui/view_renderer.h"
#include "../interface_v3/group_map.h"


using namespace NL3D;
using namespace NLMISC;

namespace R2
{

// helper function for lua init
void readFromLua(const CLuaObject &table, const char *key, float &dest)
{
	if (table[key].isNumber()) dest = (float) table[key].toNumber();
}
void readFromLua(const CLuaObject &table, const char *key, uint &dest)
{
	if (table[key].isNumber()) dest = (uint) table[key].toNumber();
}
void readFromLua(const CLuaObject &table, const char *key, std::string &dest)
{
	if (table[key].isString()) dest = table[key].toString();
}

void readFromLua(const CLuaObject &table, const char *key, CStringShared &dest)
{
	if (table[key].isString()) dest = table[key].toString();
}

void readFromLua(const CLuaObject &table, const char *key, bool &dest)
{
	if (table[key].isBoolean()) dest = table[key].toBoolean();
}
void readFromLua(const CLuaObject &table, const char *key, CRGBA &dest)
{
	if (table[key].isRGBA()) dest = table[key].toRGBA();
}

#define READ_FROM_LUA(dest) readFromLua(params, #dest, dest);

// *********************************************************
void CVertexLook::init(const CLuaObject &params)
{
	//H_AUTO(R2_CVertexLook_init)
	READ_FROM_LUA(DecalTexture);
	READ_FROM_LUA(DecalSize);
	READ_FROM_LUA(DecalColor);
	READ_FROM_LUA(DecalDistToEdgeDecal);
	READ_FROM_LUA(WorldMapTexture);
	READ_FROM_LUA(WorldMapColor);
}


// *********************************************************
void CEdgeLook::init(const CLuaObject &params)
{
	//H_AUTO(R2_CEdgeLook_init)
	READ_FROM_LUA(ShapeName);
	READ_FROM_LUA(ShapeScale);
	READ_FROM_LUA(DecalTexture);
	READ_FROM_LUA(DecalUScale);
	READ_FROM_LUA(DecalWidth);
	READ_FROM_LUA(DecalColor);
	READ_FROM_LUA(DecalFiltered);
	READ_FROM_LUA(WorldMapTexture);
	READ_FROM_LUA(WorldMapWidth);
	READ_FROM_LUA(WorldMapColor);
	READ_FROM_LUA(WorldMapUScale);
	READ_FROM_LUA(WorldMapFiltered);
	uint decalWrapMode = Repeat;
	readFromLua(params, "DecalWrapMode", decalWrapMode);
	if (decalWrapMode < WrapModeCount) DecalWrapMode = (TWrapMode) decalWrapMode;
	uint worldMapWrapMode = Scaled;
	readFromLua(params, "WorldMapWrapMode", worldMapWrapMode);
	if (worldMapWrapMode < WrapModeCount) WorldMapWrapMode = (TWrapMode) worldMapWrapMode;
}

// *********************************************************
bool operator ==(const CPrimLook &lhs, const CPrimLook &rhs)
{
	return lhs.Shape == rhs.Shape &&
	lhs.VertexShapeName == rhs.VertexShapeName &&
	lhs.VertexShapeScale == rhs.VertexShapeScale &&
	lhs.VertexLook == rhs.VertexLook &&
	lhs.FirstVertexLook == rhs.FirstVertexLook &&
	lhs.EdgeLook == rhs.EdgeLook &&
	lhs.LastEdgeIsValid == rhs.LastEdgeIsValid &&
	lhs.ClipDownFacing == rhs.ClipDownFacing;
}


// *********************************************************
void CPrimLook::init(const CLuaObject &params)
{
	//H_AUTO(R2_CPrimLook_init)
	if (!params.isTable())
	{
		nlwarning("<CPrimLook::init> parameters are not a table");
		return;
	}
	// vertices
	READ_FROM_LUA(VertexShapeName);
	READ_FROM_LUA(VertexShapeScale);
	VertexLook.init(params["VertexLook"]);
	if (!params["FirstVertexLook"].isNil())
	{
		FirstVertexLook.init(params["FirstVertexLook"]);
	}
	else
	{
		FirstVertexLook = VertexLook;
	}
	EdgeLook.init(params["EdgeLook"]);
	READ_FROM_LUA(LastEdgeIsValid);
	READ_FROM_LUA(ClipDownFacing);
	// enums
	uint shape = Star;
	readFromLua(params, "Shape", shape);
	if (shape < ShapeCount) Shape = (TShape) shape;
	//
}

// *********************************************************
void CPrimRender::setLook(const CPrimLook &look)
{
	//H_AUTO(R2_CPrimRender_setLook)
	if (look == _Look) return;
	_Look = look;
	if (!look.EdgeLook.WorldMapTexture.empty())
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CViewRenderer &vr = *CViewRenderer::getInstance();
		sint32 width, height;
		sint32 id = vr.getTextureIdFromName(look.EdgeLook.WorldMapTexture);
		vr.getTextureSizeFromId(id, width, height);
		_InvWorldTextureWidth = width > 0 ? 1.f / width : 0.f;
	}
	update();
}

// *********************************************************
CPrimRender::~CPrimRender()
{
	if (_AddedToWorldMap)
	{
		CGroupMap *gm = CTool::getWorldMap();
		nlassert(gm);
		this->onRemove(*gm);
	}
	else
	{
		nlassert(_WorldMapVertices.empty());
		nlassert(_WorldMapEdges.empty());
	}
}

// *********************************************************
CCtrlPolygon *CPrimRender::newCtrlPolygon() const
{
	//H_AUTO(R2_CPrimRender_newCtrlPolygon)
	class CCtrlMapPolygon : public CCtrlPolygon
	{
	public:
		CCtrlMapPolygon( CViewBase::TCtorParam &param ) : CCtrlPolygon( param ){}
	protected:
		// from CCtrlPolygon
		void computeScaledVertex(NLMISC::CVector2f &dest, const NLMISC::CVector2f &src)
		{
			CGroupMap *gm = CTool::getWorldMap();
			if (!gm) dest = CVector::Null;
			gm->worldToWindow(dest, src);
		}
	};
	CViewBase::TCtorParam param;
	return new CCtrlMapPolygon( param );
}

// *********************************************************
CCtrlQuad *CPrimRender::newCtrlQuad(uint /* edgeIndex */) const
{
	//H_AUTO(R2_CPrimRender_newCtrlQuad)
	CViewBase::TCtorParam param;
	return new CCtrlQuad( param );
}

// *********************************************************
bool CPrimRender::contains(const NLMISC::CVector2f &pos) const
{
	//H_AUTO(R2_CPrimRender_contains)
	for(uint k = 0; k < _VertexDecals.size(); ++k)
	{
		if (_VertexDecals[k]->contains(pos)) return true;
	}
	for(uint k = 0; k < _EdgeDecals.size(); ++k)
	{
		if (_EdgeDecals[k]->contains(pos)) return true;
	}
	return false;
}

// *********************************************************
sint CPrimRender::isOnEdge(const NLMISC::CVector2f &pos) const
{
	//H_AUTO(R2_CPrimRender_isOnEdge)
	for(uint k = 0; k < _EdgeDecals.size(); ++k)
	{
		if (_EdgeDecals[k]->contains(pos)) return (sint) k;
	}
	return -1;
}

// *********************************************************
CPrimRender::CPrimRender()
{
	_Emissive = CRGBA::Black;
	_NumEdges = 0;
	_AddedToWorldMap = false;
	_WorldMapPoly = NULL;
	_InvWorldTextureWidth = 0.f;
	_Active = true;
}

// *********************************************************
void CPrimRender::clear()
{
	//H_AUTO(R2_CPrimRender_clear)
	setVertices(std::vector<NLMISC::CVector>());
}

// *********************************************************
void CPrimRender::setVertices(const std::vector<NLMISC::CVector> &vertices)
{
	//H_AUTO(R2_CPrimRender_setVertices)
	if (_Vertices.size() == vertices.size())
	{
		if (std::equal(vertices.begin(), vertices.end(), _Vertices.begin()))
		{
			return;
		}
		else
		{
			_Vertices = vertices;
			updatePos();
			return;
		}
	}
	_Vertices = vertices;
	update();
}

// *********************************************************
void CPrimRender::setVertices(const std::vector<NLMISC::CVector2f> &vertices)
{
	//H_AUTO(R2_CPrimRender_setVertices)
	std::vector<CVector> vertices3D(vertices.size());
	for(uint k = 0; k < vertices.size(); ++k)
	{
		vertices3D[k] = vertices[k];
	}
	setVertices(vertices3D);
}

// *********************************************************
void CPrimRender::setCustomWorldMapEdgeUVMatrix(bool on, const NLMISC::CMatrix &matrix)
{
	//H_AUTO(R2_CPrimRender_setCustomWorldMapEdgeUVMatrix)
	if (_CustomWorldMapEdgeUVMatrix.set(on, matrix))
	{
		update();
	}
}

// *********************************************************
void CPrimRender::setCustomDecalEdgeUVMatrix(bool on, const NLMISC::CMatrix &matrix)
{
	//H_AUTO(R2_CPrimRender_setCustomDecalEdgeUVMatrix)
	if (_CustomDecalEdgeUVMatrix.set(on, matrix))
	{
		update();
	}
}

// *********************************************************
void CPrimRender::setEmissive(NLMISC::CRGBA color)
{
	//H_AUTO(R2_CPrimRender_setEmissive)
	if (color == _Emissive) return;
	forceSetEmissive(color);
}

// *********************************************************
void CPrimRender::forceSetEmissive(NLMISC::CRGBA emissive)
{
	//H_AUTO(R2_CPrimRender_forceSetEmissive)
	_Emissive = emissive;
	_VertexShapeInstances.setEmissive(emissive);
	_EdgeShapeInstances.setEmissive(emissive);
	for(uint k = 0; k < _VertexDecals.size(); ++k)
	{
		_VertexDecals[k]->setEmissive(emissive);
	}
	for(uint k = 0; k < _EdgeDecals.size(); ++k)
	{
		_EdgeDecals[k]->setEmissive(emissive);
	}
}

// *********************************************************
void CPrimRender::update()
{
	//H_AUTO(R2_CPrimRender_update)
	////////////
	// RESIZE //
	////////////

	// vertices
	_VertexShapeInstances.clear();
	if (!_Look.VertexShapeName.empty())
	{
		_VertexShapeInstances.setShapeName(_Look.VertexShapeName);
		_VertexShapeInstances.resize((uint)_Vertices.size());
	}
	else
	{
		_VertexShapeInstances.clear();
	}
	//
	_VertexDecals.clear();
	if (!_Look.VertexLook.DecalTexture.empty())
	{
		_VertexDecals.resize(_Vertices.size());
		for(uint k = 0; k < _Vertices.size(); ++k)
		{
			_VertexDecals[k] = new CDecal;
			_VertexDecals[k]->setClipDownFacing(_Look.ClipDownFacing);
			if (k == 0 && !_Look.FirstVertexLook.DecalTexture.empty())
			{
				_VertexDecals[k]->setTexture(_Look.FirstVertexLook.DecalTexture, true);
				_VertexDecals[k]->setDiffuse(_Look.FirstVertexLook.DecalColor);
			}
			else
			{
				_VertexDecals[k]->setTexture(_Look.VertexLook.DecalTexture);
				_VertexDecals[k]->setDiffuse(_Look.VertexLook.DecalColor);
			}
		}
	}
	//edges
	switch(_Look.Shape)
	{
		case CPrimLook::Star:
			_NumEdges = _Vertices.size() <= 1 ? 0 : (sint)_Vertices.size() - 1;
		break;
		case CPrimLook::PolyLine:
			_NumEdges = _Vertices.size() <= 1 ? 0 : (sint)_Vertices.size() - 1;
			if (!_Look.LastEdgeIsValid && _NumEdges != 0) -- _NumEdges;
		break;
		case CPrimLook::ClosedPolyLine:
			_NumEdges = _Vertices.size() >= 3 ? (sint)_Vertices.size() : std::max((sint) 0, (sint) (_Vertices.size() - 1));
			if (!_Look.LastEdgeIsValid && _NumEdges != 0) -- _NumEdges;
		break;
		default:
			nlassert(0);
		break;
	}
	//
	nlassert(_NumEdges >= 0);
	//
	_EdgeShapeInstances.clear();
	if (!_Look.EdgeLook.ShapeName.empty())
	{
		_EdgeShapeInstances.setShapeName(_Look.EdgeLook.ShapeName);
		_EdgeShapeInstances.resize(_NumEdges);
	}
	else
	{
		_EdgeShapeInstances.clear();
	}
	//
	_EdgeDecals.clear();
	if (!_Look.EdgeLook.DecalTexture.empty())
	{
		_EdgeDecals.resize(_NumEdges);
		for(sint k = 0; k < _NumEdges; ++k)
		{
			_EdgeDecals[k] = new CDecal;
			_EdgeDecals[k]->setClipDownFacing(_Look.ClipDownFacing);
			_EdgeDecals[k]->setTexture(_Look.EdgeLook.DecalTexture, _Look.EdgeLook.DecalWrapMode == CEdgeLook::Centered, true, _Look.EdgeLook.DecalFiltered);
			_EdgeDecals[k]->setDiffuse(_Look.EdgeLook.DecalColor);
			_EdgeDecals[k]->setCustomUVMatrix(_CustomDecalEdgeUVMatrix.On, _CustomDecalEdgeUVMatrix.Matrix);
		}
	}
	//////////////////
	// POSITIONNING //
	//////////////////
	updatePos();
	//
	forceSetEmissive(_Emissive);
	//
	/*
	TTime endTime = CTime::getLocalTime();
	nlinfo("%.2f seconds for : CPrimRender::update", (endTime - _StartTime) / 1000.f);
	*/
}


// *********************************************************
void CPrimRender::updatePos()
{
	//H_AUTO(R2_CPrimRender_updatePos)
	// world map
	if (_AddedToWorldMap)
	{
		setWorldMapNumVertices(_Look.VertexLook.WorldMapTexture.empty() ? 0 : (uint)_Vertices.size());
		setWorldMapNumEdges(_Look.EdgeLook.WorldMapTexture.empty() ? 0 : _NumEdges);
	}
	//
	if (!_Vertices.empty())
	{
		// edges update
		switch(_Look.Shape)
		{
			case CPrimLook::Star:
			{
				CVector centerPos = _Vertices[0];
				for(uint k = 1; k < _Vertices.size(); ++k)
				{
					//
					if (!_EdgeShapeInstances.empty())
					{
						updateEdge(_EdgeShapeInstances[k - 1], centerPos, _Vertices[k]);
					}
					if (!_EdgeDecals.empty())
					{
						updateEdgeDecal(*(_EdgeDecals[k - 1]), centerPos, _Vertices[k], _Look.FirstVertexLook.DecalDistToEdgeDecal, _Look.VertexLook.DecalDistToEdgeDecal);
					}
				}
			}
			break;
			case CPrimLook::PolyLine:
			case CPrimLook::ClosedPolyLine:
			{
				for(sint k = 0; k < _NumEdges; ++k)
				{
					//
					if (!_EdgeShapeInstances.empty())
					{
						updateEdge(_EdgeShapeInstances[k], _Vertices[k], _Vertices[(k + 1) % _Vertices.size()]);
					}
					if (!_EdgeDecals.empty())
					{
						updateEdgeDecal(*(_EdgeDecals[k]), _Vertices[k], _Vertices[(k + 1) % _Vertices.size()],
									k == 0 ? _Look.VertexLook.DecalDistToEdgeDecal : _Look.FirstVertexLook.DecalDistToEdgeDecal,
									_Look.VertexLook.DecalDistToEdgeDecal
								   );
					}
				}
			}
			break;
			default:
				nlassert(0);
			break;
		}
	}
	// update vertices
	for(uint k = 0; k < _Vertices.size(); ++k)
	{
		if (!_Look.VertexShapeName.empty())
		{
			if (!_VertexShapeInstances[k].empty())
			{
				CMatrix vertexMat;
				vertexMat.setScale(_Look.VertexShapeScale);
				vertexMat.setPos(_Vertices[k]);
				_VertexShapeInstances[k].setTransformMode(UTransform::DirectMatrix);
				_VertexShapeInstances[k].setMatrix(vertexMat);
			}
		}
		if (!_VertexDecals.empty())
		{
			_VertexDecals[k]->setWorldMatrixForSpot(CVector2f(_Vertices[k].x, _Vertices[k].y), k == 0 ? _Look.FirstVertexLook.DecalSize : _Look.VertexLook.DecalSize);
		}
	}
	//
	if (_AddedToWorldMap)
	{
		updateWorldMapDisplay();
	}
}

// *********************************************************
void CPrimRender::updateEdge(NL3D::UInstance edge,const NLMISC::CVector &start, const NLMISC::CVector &end)
{
	//H_AUTO(R2_CPrimRender_updateEdge)
	CVector I = end - start;
	CVector INormed = I.normed();
	CVector K = (CVector::K - (CVector::K * INormed) * INormed).normed();
	CVector J = K ^ INormed;
	CMatrix connectorMat;
	static volatile float scale =0.5f;
	connectorMat.setRot(I, scale * J, scale * K);
	connectorMat.setPos(start);
	edge.setTransformMode(UTransform::DirectMatrix);
	edge.setMatrix(connectorMat);
	edge.show();
}

// *********************************************************
void CPrimRender::updateEdgeDecal(CDecal &edgeDecal, const NLMISC::CVector &start, const NLMISC::CVector &end, float distToStartVertex, float distToEndVertex)
{
	//H_AUTO(R2_CPrimRender_updateEdgeDecal)
	CVector2f start2f(start.x, start.y);
	CVector2f end2f(end.x, end.y);
	// compute real start coordinate that is at 'startRadius' dist from the 'start' pos
	float length = (end2f - start2f).norm();
	if ((distToStartVertex + distToEndVertex) >= length)
	{
		CMatrix nullMat;
		nullMat.setScale(0.f);
		// decal not visible
		edgeDecal.setWorldMatrix(nullMat);
		return;
	}
	CVector dirNormed = (end2f - start2f) / length;
	start2f = start2f + distToStartVertex * dirNormed;
	end2f = end2f - distToEndVertex * dirNormed;
	edgeDecal.setWorldMatrixForArrow(start2f, end2f, _Look.EdgeLook.DecalWidth);
	CMatrix uvMatrix;
	float uScale = _Look.EdgeLook.DecalUScale * (length  - (distToStartVertex + distToEndVertex));
	uvMatrix.setScale(CVector(uScale, 1.f, 1.f));
	switch(_Look.EdgeLook.DecalWrapMode)
	{
		case CEdgeLook::Scaled:
		case CEdgeLook::Repeat:
		break;
		case CEdgeLook::Centered:
			uvMatrix.setPos(CVector(0.5f * (1.f - uScale), 0.f, 0.f));
		break;
		default:
			nlassert(0);
		break;
	}
	edgeDecal.setTextureMatrix(uvMatrix);
}

// *********************************************************
void CPrimRender::addDecalsToRenderList()
{
	//H_AUTO(R2_CPrimRender_addDecalsToRenderList)
	if (!_Active) return;
	for(uint k = 0; k < _VertexDecals.size(); ++k)
	{
		_VertexDecals[k]->addToRenderList();
	}
	for(uint k = 0; k < _EdgeDecals.size(); ++k)
	{
		_EdgeDecals[k]->addToRenderList();
	}
}

// *********************************************************
void CPrimRender::setWorldMapNumVertices(uint count)
{
	//H_AUTO(R2_CPrimRender_setWorldMapNumVertices)
	nlassert(_AddedToWorldMap);
	CGroupMap *gm = CTool::getWorldMap();
	nlassert(gm);
	if (count < _WorldMapVertices.size())
	{
		for(uint k = count; k < _WorldMapVertices.size(); ++k)
		{
			gm->delView(_WorldMapVertices[k]);
		}
	}
	else
	{
		uint left = count - (uint)_WorldMapVertices.size();
		while (left --)
		{
			CViewBitmap *bm = new CViewBitmap(CViewBase::TCtorParam());
			bm->setModulateGlobalColor(false);
			bm->setPosRef(Hotspot_MM);
			bm->setParentPosRef(Hotspot_BL);
			bm->setActive(_Active);
			gm->addView(bm);
			bm->setParent(gm);
			bm->setRenderLayer(2);
			_WorldMapVertices.push_back(bm);
		}
	}
	_WorldMapVertices.resize(count);
	for(uint k = 0; k < count; ++k)
	{
		const CVertexLook &look = k == 0 ? _Look.VertexLook : _Look.FirstVertexLook;
		_WorldMapVertices[k]->setTexture(look.WorldMapTexture);
		_WorldMapVertices[k]->setColor(look.WorldMapColor);
		_WorldMapVertices[k]->fitTexture();
	}
}

// *********************************************************
void CPrimRender::setWorldMapNumEdges(uint count)
{
	//H_AUTO(R2_CPrimRender_setWorldMapNumEdges)
	nlassert(_AddedToWorldMap);
	CGroupMap *gm = CTool::getWorldMap();
	nlassert(gm);
	if (count < _WorldMapEdges.size())
	{
		for(uint k = count; k < _WorldMapEdges.size(); ++k)
		{
			gm->delCtrl(_WorldMapEdges[k]);
		}
	}
	else
	{
		uint left = count - (uint)_WorldMapEdges.size();
		while (left --)
		{
			CCtrlQuad *cq = newCtrlQuad((uint)_WorldMapEdges.size());
			cq->setModulateGlobalColor(false);
			cq->setActive(_Active);
			gm->addCtrl(cq);
			cq->setParent(gm);
			cq->setRenderLayer(1);
			_WorldMapEdges.push_back(cq);
			if (_WorldMapPoly)
			{
				cq->setId(_WorldMapPoly->getId() + toString("_e%d", _WorldMapEdges.size() - 1));
			}
		}
	}
	_WorldMapEdges.resize(count);
	for(uint k = 0; k < count; ++k)
	{
		_WorldMapEdges[k]->setTexture(_Look.EdgeLook.WorldMapTexture);
		_WorldMapEdges[k]->setColorRGBA(_Look.EdgeLook.WorldMapColor);
	}
}

// *********************************************************
void CPrimRender::updateWorldMapDisplay()
{
	//H_AUTO(R2_CPrimRender_updateWorldMapDisplay)
	nlassert(_AddedToWorldMap);
	CGroupMap *gm = CTool::getWorldMap();
	nlassert(gm);
	this->onUpdate(*gm);
}

// *********************************************************
void CPrimRender::onAdd(CGroupMap &owner)
{
	//H_AUTO(R2_CPrimRender_onAdd)
	nlassert(!_AddedToWorldMap);
	_AddedToWorldMap = true;
	_WorldMapPoly = newCtrlPolygon(); // TODO: create only if needed
	_WorldMapPoly->setParent(&owner);
	owner.addCtrl(_WorldMapPoly);
	_WorldMapPoly->setActive(false);
	_WorldMapPoly->setId(owner.getId() + ":" + _WorldMapPoly->getId());
	for(uint k = 0; k < _WorldMapEdges.size(); ++k)
	{
		_WorldMapEdges[k]->setId(_WorldMapPoly->getId() + toString("_e%d", (int) k ));
	}
}

// *********************************************************
void CPrimRender::onRemove(CGroupMap &owner)
{
	//H_AUTO(R2_CPrimRender_onRemove)
	nlassert(_AddedToWorldMap);
	setWorldMapNumVertices(0);
	setWorldMapNumEdges(0);
	if (_WorldMapPoly)
	{
		owner.delCtrl(_WorldMapPoly);
		_WorldMapPoly = NULL;
	}
	_AddedToWorldMap = false;
}

// *********************************************************
void CPrimRender::onPreRender(CGroupMap &/* owner */)
{
	//H_AUTO(R2_CPrimRender_onPreRender)
	// no-op
}

// *********************************************************
void CPrimRender::onUpdate(CGroupMap &worldMap)
{
	//H_AUTO(R2_CPrimRender_onUpdate)
	nlassert(_AddedToWorldMap);
	if (!_Look.VertexLook.WorldMapTexture.empty())
	{
		nlassert(_Vertices.size() == _WorldMapVertices.size());
	}
	static std::vector<sint32> px;
	static std::vector<sint32> py;
	px.resize(_Vertices.size());
	py.resize(_Vertices.size());
	for(uint k = 0; k < _Vertices.size(); ++k)
	{
		worldMap.worldToWindowSnapped(px[k], py[k], _Vertices[k]);
		if (!_WorldMapVertices.empty())
		{
			_WorldMapVertices[k]->setX(px[k]);
			_WorldMapVertices[k]->setY(py[k]);
			_WorldMapVertices[k]->updateCoords();
		}
	}
	if (_WorldMapPoly)
	{
		if (_Look.Shape == CPrimLook::ClosedPolyLine)
		{
			_WorldMapPoly->setActive(_Active);

			/*static volatile bool test1 = false;
			const CVector2f &origin = test1 ? worldMap.getWorldOffset() : CVector2f::Null;

			CVector2f ref0(0.f, 0.f);
			CVector2f ref1(1000.f, 1000.f);
			ref0 += origin;
			ref1 += origin;
			worldMap.worldToWindow(ref0, ref0);
			worldMap.worldToWindow(ref1, ref1);

			CMatrix polyMatrix;
			float scaleX = (ref1.x - ref0.x) * 0.001f;
			float scaleY = (ref1.y - ref0.y) * 0.001f;
			polyMatrix.setRot(CVector(scaleX, 0.f, 0.f),
								CVector(0.f, scaleY, 0.f),
								CVector::Null);
			polyMatrix.setPos(CVector(ref0.x - scaleX * origin.x, ref0.y - scaleY * origin.y, 0.f));
			*/
			_WorldMapPoly->setVertices(_Vertices);
			//_WorldMapPoly->setMatrix(polyMatrix);
			//_WorldMapPoly->touch();

			/*
			static volatile bool dumpPoly = false;
			if (dumpPoly)
			{
				nlwarning("================");
				const std::vector<CVector> &verts = _WorldMapPoly->getVertices();
				for(uint k = 0; k < verts.size(); ++k)
				{
					CVector pos = _WorldMapPoly->getMatrix() * verts[k];
					nlwarning("vert %d = (%.1f, %.1f, %.1f)", (int) k , pos.x, pos.y, pos.z);
				}
				dumpPoly = false;
			}*/
		}
		else
		{
			_WorldMapPoly->setActive(false);
		}
	}
	for(uint k = 0; k < _WorldMapEdges.size(); ++k)
	{
		uint startIndex = _Look.Shape == CPrimLook::Star ? 0 : k;
		CVector2f start((float) px[startIndex], (float) py[startIndex]);
		uint nextIndex = (k + 1) % _Vertices.size();
		CVector2f end((float) px[nextIndex], (float) py[nextIndex]);
		_WorldMapEdges[k]->setQuad(start, end, _Look.EdgeLook.WorldMapWidth);
		_WorldMapEdges[k]->setFiltered(_Look.EdgeLook.WorldMapFiltered);
		_WorldMapEdges[k]->updateCoords();
		//
		float length = (end - start).norm();
		if (_CustomWorldMapEdgeUVMatrix.On)
		{
			CUV uvs[4];
			CVector startUV = _CustomWorldMapEdgeUVMatrix.Matrix * _Vertices[startIndex];
			CVector endUV = _CustomWorldMapEdgeUVMatrix.Matrix * _Vertices[nextIndex];
			static uint index0 = 0;
			static uint index1 = 3;
			static uint index2 = 2;
			static uint index3 = 1;
			uvs[index0] = uvs[index1] = CUV(startUV.x, startUV.y);
			uvs[index2] = uvs[index3] = CUV(endUV.x, endUV.y);
			_WorldMapEdges[k]->setCustomUVs(uvs);
		}
		else
		{
			switch(_Look.EdgeLook.WorldMapWrapMode)
			{
				case CEdgeLook::Scaled:
					_WorldMapEdges[k]->setPattern(0.f, 1.f, CCtrlQuad::Repeat);
				break;
				case CEdgeLook::Repeat:
					_WorldMapEdges[k]->setPattern(0.f, _InvWorldTextureWidth * _Look.EdgeLook.WorldMapUScale * length, CCtrlQuad::Repeat);
				break;
				case CEdgeLook::Centered:
					_WorldMapEdges[k]->setPattern(0.5f - 0.5f * length * _InvWorldTextureWidth * _Look.EdgeLook.WorldMapUScale,
												  0.5f + 0.5f * length * _InvWorldTextureWidth * _Look.EdgeLook.WorldMapUScale,
												  CCtrlQuad::Clamp);
				break;
				default:
					nlassert(0);
				break;
			}
		}
	}
}

// *********************************************************
CViewBitmap *CPrimRender::getWorldMapVertexView(uint index) const
{
	//H_AUTO(R2_CPrimRender_getWorldMapVertexView)
	if(index >= _WorldMapVertices.size()) return NULL;
	return _WorldMapVertices[index];
}

// *********************************************************
void CPrimRender::setWorldMapPolyColor(NLMISC::CRGBA color)
{
	//H_AUTO(R2_CPrimRender_setWorldMapPolyColor)
	if (_WorldMapPoly) _WorldMapPoly->setColorRGBA(color);
}

// *********************************************************
void CPrimRender::setActive(bool active)
{
	//H_AUTO(R2_CPrimRender_setActive)
	if (active == _Active) return;
	for(uint k = 0; k < _WorldMapVertices.size(); ++k)
	{
		if (_WorldMapVertices[k]) _WorldMapVertices[k]->setActive(active);
	}
	for(uint k = 0; k < _WorldMapEdges.size(); ++k)
	{
		if (_WorldMapEdges[k]) _WorldMapEdges[k]->setActive(active);
	}
	if (_WorldMapPoly) _WorldMapPoly->setActive(active);
	_VertexShapeInstances.setActive(active);
	_EdgeShapeInstances.setActive(active);
	_Active = active;
}



} // R2



