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
#include "nel/gui/ctrl_polygon.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/interface_group.h"

using namespace NLMISC;

namespace NLGUI
{

	// *********************************************************************************
    CCtrlPolygon::CCtrlPolygon( const TCtorParam &param ) : CCtrlBase( param )
	{
		// Construct
		_Color = CRGBA::White;
		//_Matrix = CMatrix::Identity;
		_Valid = true;
	}

	// *********************************************************************************
	void CCtrlPolygon::updateBoudingRect()
	{
		H_AUTO(Rz_CCtrlPolygon_updateBoudingRect)
		if (_Poly.Vertices.empty())
		{
			setX(0);
			setY(0);
			setW(0);
			setH(0);
			return;
		}
		//
		sint32 xmin = INT_MAX;
		sint32 ymin = INT_MAX;
		sint32 xmax = INT_MIN;
		sint32 ymax = INT_MIN;
		uint numVerts = (uint)_Poly.Vertices.size();
		_XFormPoly.Vertices.resize(numVerts);
		for(uint k = 0; k < numVerts; ++k)
		{
			CVector2f &finalPos = _XFormPoly.Vertices[k];
			//finalPos = _Matrix * _Poly.Vertices[k];
			computeScaledVertex(finalPos, CVector2f(_Poly.Vertices[k].x, _Poly.Vertices[k].y));
			xmin = std::min(xmin, (sint32) floorf(finalPos.x));
			xmax = std::max(xmax, (sint32) ceilf(finalPos.x));
			ymin = std::min(ymin, (sint32) floorf(finalPos.y));
			ymax = std::max(ymax, (sint32) ceilf(finalPos.y));
		}
		setX(xmin);
		setY(ymin);
		setW(xmax - xmin);
		setH(ymax - ymin);
	}

	// *********************************************************************************
	bool CCtrlPolygon::contains(const CVector2f &pos) const
	{
		H_AUTO(Rz_CCtrlPolygon_contains)
		if (!_Valid) return false;
		return _XFormPoly.contains(pos, false);
	}

	// *********************************************************************************
	void CCtrlPolygon::setVertices(const std::vector<NLMISC::CVector> &vertices)
	{
		H_AUTO(Rz_CCtrlPolygon_setVertices)
		if (vertices.size() == _Poly.Vertices.size() &&
			std::equal(vertices.begin(), vertices.end(), _Poly.Vertices.begin())) return; // remains unchanged


		//TTicks startTime = CTime::getPerformanceTime();

		_Poly.Vertices = vertices;
		_Tris.clear();
		std::list<CPolygon> polys;
		bool splitDone = _Poly.toConvexPolygons(polys, NLMISC::CMatrix::Identity);
		if (!splitDone)
		{
			polys.clear();
			// maybe wrong orientation
			std::reverse(_Poly.Vertices.begin(), _Poly.Vertices.end());
			splitDone = _Poly.toConvexPolygons(polys, NLMISC::CMatrix::Identity);
			std::reverse(_Poly.Vertices.begin(), _Poly.Vertices.end());
		}
		_Tris.clear();
		if (splitDone)
		{
			for(std::list<CPolygon>::iterator it = polys.begin(); it != polys.end(); ++it)
			{
				it->toTriFan(_Tris);
			}
		}
		_Touched = true;
		updateBoudingRect();
		_Valid = splitDone;

		//TTicks endTime = CTime::getPerformanceTime();		
		//nlinfo("%d ms for CCtrlPolygon::setVertices", (int) (1000 * CTime::ticksToSecond(endTime - startTime)));	
	}


	static inline bool totallyInside(const CVector &minCorner, const CVector &maxCorner, sint32 cx, sint32 cy, sint32 cw, sint32 ch)
	{
		return (sint32) maxCorner.x < (cx + cw) &&
				(sint32) minCorner.x >= cx &&
				(sint32) maxCorner.y < (cy + ch) &&
				(sint32) minCorner.y >= cy;
	}

	static inline bool totallyOutside(const CVector &minCorner, const CVector &maxCorner, sint32 cx, sint32 cy, sint32 cw, sint32 ch)
	{
		return (sint32) minCorner.x >= (cx + cw) ||
				(sint32) maxCorner.x < cx ||
				(sint32) minCorner.y >= (cy + ch) ||
				(sint32) maxCorner.y < cy;
	}


	// *********************************************************************************
	/*void CCtrlPolygon::setMatrix(const NLMISC::CMatrix &mat)
	{
		const float *lhs = mat.get();
		const float *rhs = _Matrix.get();
		if (std::equal(lhs, lhs + 16, rhs)) return; // unmodified...
		_Matrix = mat;
		updateBoudingRect();
		_Touched = true;
	}*/


	// *********************************************************************************
	void CCtrlPolygon::draw()
	{
		H_AUTO(Rz_CCtrlPolygon_draw)
		if (_Tris.empty()) return;
		if (!_Parent) return;
		CViewRenderer &vr = *CViewRenderer::getInstance();
		if (_Touched)
		{
			_RealTris.clear();
			uint numTris = (uint)_Tris.size();
			sint32 cornerX, cornerY;
			static std::vector<NLMISC::CTriangle> winTris;
			winTris.resize(numTris);
			_Parent->getCorner(cornerX, cornerY, _ParentPosRef);
			/*CMatrix m = _Matrix;
			m.setPos(m.getPos() + CVector((float) cornerX, (float) cornerY, 0.f));*/
			for(uint k = 0; k < numTris; ++k)
			{
				/*winTris[k].V0 = m * _Tris[k].V0;
				winTris[k].V1 = m * _Tris[k].V1;
				winTris[k].V2 = m * _Tris[k].V2;*/

				CVector2f result;
				computeScaledVertex(result, _Tris[k].V0);
				winTris[k].V0.set(result.x + cornerX, result.y + cornerY, 0.f);
				computeScaledVertex(result, _Tris[k].V1);
				winTris[k].V1.set(result.x + cornerX, result.y + cornerY, 0.f);
				computeScaledVertex(result, _Tris[k].V2);
				winTris[k].V2.set(result.x + cornerX, result.y + cornerY, 0.f);
			}
			// recompute & reclip poly
			_RealTris.clear();
			sint32 cx, cy, cw, ch;
			vr.getClipWindow(cx, cy, cw, ch);
			// per tri clip
			NLMISC::CVector minCorner;
			NLMISC::CVector maxCorner;
			for(uint k = 0; k < numTris; ++k)
			{
				winTris[k].getMinCorner(minCorner);
				winTris[k].getMaxCorner(maxCorner);
				if (totallyOutside(minCorner, maxCorner, cx, cy, cw, ch)) continue;
				if (totallyInside(minCorner, maxCorner, cx, cy, cw, ch))
				{
					_RealTris.push_back(winTris[k]);
				}
				else
				{
					const uint maxNumCorners = 8;
					static CVector	outPos0[maxNumCorners];
					static CVector	outPos1[maxNumCorners];
					//
					outPos0[0] = winTris[k].V0;
					outPos0[1] = winTris[k].V1;
					outPos0[2] = winTris[k].V2;
					//
					CVector *pPos0 = outPos0;
					CVector *pPos1 = outPos1;
					//
					sint count = 3;
					//
					if ((sint32) minCorner.x < cx)
					{
						// clip left
						CPlane clipper(-1.f, 0.f, 0.f, (float) cx);
						count = clipper.clipPolygonBack(pPos0, pPos1, count);
						std::swap(pPos0, pPos1);
					}
					if ((sint32) maxCorner.x > cx + cw)
					{
						// clip right
						CPlane clipper(1.f, 0.f, 0.f, - (float) (cx + cw));
						count = clipper.clipPolygonBack(pPos0, pPos1, count);
						std::swap(pPos0, pPos1);
					}
					//
					if ((sint32) minCorner.y < cy)
					{
						// clip bottom
						CPlane clipper(0.f, -1.f, 0.f, (float) cy);
						count = clipper.clipPolygonBack(pPos0, pPos1, count);
						std::swap(pPos0, pPos1);
					}
					if ((sint32) maxCorner.y > cy + ch)
					{
						// clip top
						CPlane clipper(0.f, 1.f, 0.f, - (float) (cy + ch));
						count = clipper.clipPolygonBack(pPos0, pPos1, count);
						std::swap(pPos0, pPos1);
					}
					nlassert(count <= 8);
					if (count >= 3)
					{
						for(uint k = 0; k < (uint) (count - 2); ++k)
						{
							_RealTris.push_back(NLMISC::CTriangle(pPos0[0], pPos0[k + 1], pPos0[k + 2]));
						}
					}
				}
			}
			_Touched = false;
		}
		if (_RealTris.empty()) return;
		CRGBA col;
		if(getModulateGlobalColor())
		{
			col.modulateFromColor (_Color, CWidgetManager::getInstance()->getGlobalColorForContent());
		}
		else
		{
			col= _Color;
			col.A = (uint8)(((sint32)col.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
		}
		vr.drawUnclippedTriangles(_RenderLayer, _RealTris, col);
	}

	// *********************************************************************************
	void CCtrlPolygon::updateCoords()
	{
		H_AUTO(Rz_CCtrlPolygon_updateCoords)
		CCtrlBase::updateCoords();
		updateBoudingRect();
		// assume that clipping will have to be done again, real update of triangle will be done at render time
		_Touched = true;
	}

	// *********************************************************************************
	void CCtrlPolygon::setAlpha(sint32 a)
	{
		H_AUTO(Rz_CCtrlPolygon_setAlpha)
		_Color.A = (uint8) a;
	}

	// *********************************************************************************
	bool CCtrlPolygon::handleEvent(const NLGUI::CEventDescriptor &/* event */)
	{
		H_AUTO(Rz_CCtrlPolygon_handleEvent)
		return false;
	}

	// *********************************************************************************
	// TMP TMP
	void CCtrlPolygon::computeScaledVertex(NLMISC::CVector2f &dest, const NLMISC::CVector2f &src)
	{
		H_AUTO(Rz_CCtrlPolygon_computeScaledVertex)
		dest.set(src.x, src.y);
	}

	// *********************************************************************************
	// TMP TMP
	void CCtrlPolygon::touch()
	{
		H_AUTO(Rz_CCtrlPolygon_touch)
		updateBoudingRect();
		_Touched = true;
	}

}

