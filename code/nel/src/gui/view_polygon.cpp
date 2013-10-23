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
#include "nel/gui/view_polygon.h"

#include "nel/gui/view_renderer.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

using namespace NLMISC;

namespace NLGUI
{

	// *********************************************************************************
    CViewPolygon::CViewPolygon( const TCtorParam &param ) : CViewBase( param )
	{
		// Construct
		_Color = CRGBA::White;
	}


	// *********************************************************************************
	void CViewPolygon::setVertices(const std::vector<NLMISC::CVector> &vertices)
	{
		if (vertices.size() == _Poly.Vertices.size() &&
			std::equal(vertices.begin(), vertices.end(), _Poly.Vertices.begin())) return; // remains unchanged
		_Poly.Vertices = vertices;
		_Tris.clear();
		std::list<CPolygon> polys;
		if (_Poly.toConvexPolygons(polys, NLMISC::CMatrix::Identity))
		{
			_Tris.clear();
			for(std::list<CPolygon>::iterator it = polys.begin(); it != polys.end(); ++it)
			{
				it->toTriFan(_Tris);
			}
		}
		else
		{
			nlwarning("CViewPolygon : conversion to conex polygons failed");
			_Tris.clear();
		}
		_Touched = true;
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
	void CViewPolygon::draw()
	{
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
			for(uint k = 0; k < numTris; ++k)
			{
				winTris[k].V0.set((float) (_Tris[k].V0.x + cornerX), (float) (_Tris[k].V0.y + cornerY), 0.f);
				winTris[k].V1.set((float) (_Tris[k].V1.x + cornerX), (float) (_Tris[k].V1.y + cornerY), 0.f);
				winTris[k].V2.set((float) (_Tris[k].V2.x + cornerX), (float) (_Tris[k].V2.y + cornerY), 0.f);
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
	void CViewPolygon::updateCoords()
	{
		// assume that clipping will have to be done again, real update of triangle will be done at render time
		_Touched = true;
	}

	// *********************************************************************************
	void CViewPolygon::setAlpha(sint32 a)
	{
		_Color.A = (uint8) a;
	}

}

