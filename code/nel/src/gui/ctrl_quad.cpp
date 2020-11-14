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
#include "nel/gui/ctrl_quad.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/polygon.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	// *********************************************************************************
    CCtrlQuad::CCtrlQuad( const TCtorParam &param ) : CCtrlBase( param ), _Color(CRGBA::White),
													  _Additif(false),
													  _Filtered(true),
													  _UMin(0.f),
													  _UMax(1.f),
													  _WrapMode(Repeat)
	{
		setQuad(CQuad(CVector::Null, CVector::Null, CVector::Null, CVector::Null));
		// preset uvs for real quad
		_RealQuad.Uv0.set(0.f, 0.f);
		_RealQuad.Uv1.set(1.f, 0.f);
		_RealQuad.Uv2.set(1.f, 1.f);
		_RealQuad.Uv3.set(0.f, 1.f);
	}

	// *********************************************************************************
	bool CCtrlQuad::parse(xmlNodePtr /* cur */, CInterfaceGroup * /* parentGroup */)
	{
		nlassert(0); // NOT IMPLEMENTED (only created dynamically at this time)
		return false;
	}

	// *********************************************************************************
	void CCtrlQuad::updateCoords()
	{
		H_AUTO(Rz_CCtrlQuad_updateCoords)
		CViewBase::updateCoords();
		nlassert(_Parent);
		// don't use _XReal && _YReal, because coords are given relative to parent
		CVector delta((float) _Parent->getXReal(), (float) _Parent->getYReal(), 0.f);
		_RealQuad.set(_Quad.V0 + delta, _Quad.V1 + delta, _Quad.V2 + delta, _Quad.V3 + delta);
	}

	// *********************************************************************************
	void CCtrlQuad::draw()
	{
		H_AUTO(Rz_CCtrlQuad_draw)
		nlassert(_Parent);
		CViewRenderer &rVR = *CViewRenderer::getInstance();

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

		/*if (_InheritGCAlpha)
		{
			// search a parent container
			CInterfaceGroup *gr = getParent();
			while (gr)
			{
				if (gr->isGroupContainer())
				{
					CGroupContainer *gc = static_cast<CGroupContainer *>(gr);
					col.A = (uint8)(((sint32)col.A*((sint32)gc->getCurrentContainerAlpha()+1))>>8);
					break;
				}
				gr = gr->getParent();
			}
		}*/
		if (_UMin == 0.f && _UMax == 1.f && _WrapMode != CustomUVs)
		{
			// no pattern applied, can draw the quad in a single piece
			rVR.drawQuad(_RenderLayer, _RealQuad, _TextureId, col, _Additif, _Filtered);
		}
		else
		{
			NLMISC::CQuadUV quv;
			if (_WrapMode == Repeat)
			{
				if (_UMax == _UMin)
				{
					(CQuad &) quv = _RealQuad; // copy CQuad part
					float u = fmodf(_UMin, 1.f);
					quv.Uv0.set(u, 0.f);
					quv.Uv1.set(u, 0.f);
					quv.Uv2.set(u, 1.f);
					quv.Uv3.set(u, 1.f);
					rVR.drawQuad(_RenderLayer, quv, _TextureId, col, _Additif, _Filtered);
				}
				else
				{
					// reverse corners if needed to handle case where _UVMin < _UVmax
					NLMISC::CQuad srcQuad;
					float umin, umax;
					if (_UMax < _UMin)
					{
						umin = _UMax;
						umax = _UMin;
						srcQuad.V0 = _RealQuad.V1;
						srcQuad.V1 = _RealQuad.V0;
						srcQuad.V2 = _RealQuad.V3;
						srcQuad.V3 = _RealQuad.V2;
					}
					else
					{
						umin = _UMin;
						umax = _UMax;
						srcQuad = _RealQuad;
					}


					float unitRatio = 1.f / fabsf(umax - umin); // ratio of the real quad delta x in screen for du = 1
					// texture is stretched, mutiple parts needed
					float ceilUMin = ceilf(umin);
					float firstDeltaU =  ceilUMin - umin;
					if (firstDeltaU != 0.f)
					{

						// start quad
						quv.V0 = srcQuad.V0;
						quv.V1 = blend(srcQuad.V0, srcQuad.V1, std::min(1.f, (firstDeltaU * unitRatio)));
						quv.V2 = blend(srcQuad.V3, srcQuad.V2, std::min(1.f, (firstDeltaU * unitRatio)));
						quv.V3 = srcQuad.V3;
						float lastU = std::min(umax + 1.f - ceilUMin, 1.f);
						quv.Uv0.set(1.f - firstDeltaU, 0.f);
						quv.Uv1.set(lastU, 0.f);
						quv.Uv2.set(lastU, 1.f);
						quv.Uv3.set(1.f - firstDeltaU, 1.f);
						rVR.drawQuad(_RenderLayer, quv, _TextureId, col, _Additif, _Filtered);

						if (firstDeltaU * unitRatio >= 1.f) return;
					}

					// TODO optim: reuse of previous uv & pos ... (prb is that they are not always computed)

					// intermediate quads
					sint numQuads = (sint) (floorf(umax) - ceilf(umin));

					for(sint k = 0; k < numQuads; ++k)
					{
						float deltaU =  firstDeltaU + k;
						// start quad
						quv.V0 = blend(srcQuad.V0, srcQuad.V1, deltaU * unitRatio);
						quv.V1 = blend(srcQuad.V0, srcQuad.V1, (deltaU + 1.f) * unitRatio);
						quv.V2 = blend(srcQuad.V3, srcQuad.V2, (deltaU + 1.f) * unitRatio);
						quv.V3 = blend(srcQuad.V3, srcQuad.V2, deltaU * unitRatio);
						quv.Uv0.set(0.f, 0.f);
						quv.Uv1.set(1.f, 0.f);
						quv.Uv2.set(1.f, 1.f);
						quv.Uv3.set(0.f, 1.f);
						rVR.drawQuad(_RenderLayer, quv, _TextureId, col, _Additif, _Filtered);
					}
					// end quad
					float lastDeltaU =  umax - floorf(umax);
					if (lastDeltaU != 0.f)
					{

						// start quad
						quv.V0 = blend(srcQuad.V1, srcQuad.V0, lastDeltaU * unitRatio);
						quv.V1 = srcQuad.V1;
						quv.V2 = srcQuad.V2;
						quv.V3 = blend(srcQuad.V2, srcQuad.V3, lastDeltaU * unitRatio);
						quv.Uv0.set(0.f, 0.f);
						quv.Uv1.set(lastDeltaU, 0.f);
						quv.Uv2.set(lastDeltaU, 1.f);
						quv.Uv3.set(0.f, 1.f);
						rVR.drawQuad(_RenderLayer, quv, _TextureId, col, _Additif, _Filtered);
					}
				}

			}
			else if (_WrapMode == Clamp)
			{
				if (_UMin == _UMax)
				{
					(CQuad &) quv = _RealQuad; // copy CQuad part
					// special case
					float u = _UMin;
					clamp(u, 0.f, 1.f);
					quv.Uv0.set(u, 0.f);
					quv.Uv1.set(u, 1.f);
					quv.Uv2.set(u, 1.f);
					quv.Uv3.set(u, 0.f);
					rVR.drawQuad(_RenderLayer, quv, _TextureId, col, _Additif, _Filtered);
				}
				else
				{
					NLMISC::CQuad srcQuad;
					float umin, umax;
					if (_UMax < _UMin)
					{
						umin = _UMax;
						umax = _UMin;
						srcQuad.V0 = _RealQuad.V1;
						srcQuad.V1 = _RealQuad.V0;
						srcQuad.V2 = _RealQuad.V3;
						srcQuad.V3 = _RealQuad.V2;
					}
					else
					{
						umin = _UMin;
						umax = _UMax;
						srcQuad = _RealQuad;
					}
					float startRatio = - umin / (umax - umin); // start of unclamped u (actually (0.f - umin) / (umax - umin) )
					if (umin < 0.f)
					{
						quv.V0 = srcQuad.V0;
						quv.V1 = blend(srcQuad.V0, srcQuad.V1, std::min(1.f ,startRatio));
						quv.V2 = blend(srcQuad.V3, srcQuad.V2, std::min(1.f ,startRatio));
						quv.V3 = srcQuad.V3;
						// draw first clamped part
						quv.Uv0.set(0.f, 0.f);
						quv.Uv1.set(0.f, 0.f);
						quv.Uv2.set(0.f, 1.f);
						quv.Uv3.set(0.f, 1.f);
						rVR.drawQuad(_RenderLayer, quv, _TextureId, col, _Additif, _Filtered);
					}
					if (startRatio >= 1.f) return;
					float endRatio = (1.f - umin) / (umax - umin);
					if (endRatio > 0.f)
					{
						// draw middle part if visible
						// TODO optim: reuse of previous uv & pos ... (prb is that they are not always computed)
						quv.V0 = blend(srcQuad.V0, srcQuad.V1, std::max(0.f , startRatio));
						quv.V1 = blend(srcQuad.V0, srcQuad.V1, std::min(1.f , endRatio));
						quv.V2 = blend(srcQuad.V3, srcQuad.V2, std::min(1.f , endRatio));
						quv.V3 = blend(srcQuad.V3, srcQuad.V2, std::max(0.f , startRatio));
						// draw first clamped part
						quv.Uv0.set(std::max(0.f, umin), 0.f);
						quv.Uv1.set(std::min(1.f, umax), 0.f);
						quv.Uv2.set(std::min(1.f, umax), 1.f);
						quv.Uv3.set(std::max(0.f, umin), 1.f);
						rVR.drawQuad(_RenderLayer, quv, _TextureId, col, _Additif, _Filtered);
					}
					if (endRatio >= 1.f) return;
					// draw end part
					quv.V0 = blend(srcQuad.V0, srcQuad.V1, std::max(0.f , endRatio));
					quv.V1 = srcQuad.V1;
					quv.V2 = srcQuad.V2;
					quv.V3 = blend(srcQuad.V3, srcQuad.V2, std::max(0.f , endRatio));
					// draw end clamped part
					quv.Uv0.set(1.f, 0.f);
					quv.Uv1.set(1.f, 0.f);
					quv.Uv2.set(1.f, 1.f);
					quv.Uv3.set(1.f, 1.f);
					rVR.drawQuad(_RenderLayer, quv, _TextureId, col, _Additif, _Filtered);
				}
			}
			else
			{
				nlassert(_WrapMode == CustomUVs);
				quv.V0 = _RealQuad.V0;
				quv.V1 = _RealQuad.V1;
				quv.V2 = _RealQuad.V2;
				quv.V3 = _RealQuad.V3;
				quv.Uv0 = _CustomUVs[0];
				quv.Uv1 = _CustomUVs[1];
				quv.Uv2 = _CustomUVs[2];
				quv.Uv3 = _CustomUVs[3];
				rVR.drawQuad(_RenderLayer, quv, _TextureId, col, _Additif, _Filtered);
			}
		}
	}

	// *********************************************************************************
	void CCtrlQuad::setAlpha(sint32 a)
	{
		H_AUTO(Rz_CCtrlQuad_setAlpha)
		_Color.A = (uint8) a;
	}

	// *********************************************************************************
	void CCtrlQuad::setTexture(const std::string &texName)
	{
		H_AUTO(Rz_CCtrlQuad_setTexture)
	//	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	//	CViewRenderer &rVR = *CViewRenderer::getInstance();
		_TextureId.setTexture(texName.c_str());
	}

	// *********************************************************************************
	std::string CCtrlQuad::getTexture() const
	{
		H_AUTO(Rz_CCtrlQuad_getTexture)
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		return rVR.getTextureNameFromId (_TextureId);
	}

	// *********************************************************************************
	void CCtrlQuad::setQuad(const CQuad &quad)
	{
		H_AUTO(Rz_CCtrlQuad_setQuad)
		float qXMin = minof(quad.V0.x, quad.V1.x, quad.V2.x, quad.V3.x);
		float qXMax = maxof(quad.V0.x, quad.V1.x, quad.V2.x, quad.V3.x);
		float qYMin = minof(quad.V0.y, quad.V1.y, quad.V2.y, quad.V3.y);
		float qYMax = maxof(quad.V0.y, quad.V1.y, quad.V2.y, quad.V3.y);
		setPosRef(Hotspot_BL);
		setX((sint32) floorf(qXMin));
		setY((sint32) floorf(qYMin));
		setW((sint32) ceilf(qXMax) - getX());
		setH((sint32) ceilf(qYMax) - getY());
		_Quad = quad;
	}

	// *********************************************************************************
	void CCtrlQuad::setQuad(const NLMISC::CVector &start, const NLMISC::CVector &end, float thickness)
	{
		H_AUTO(Rz_CCtrlQuad_setQuad)
		CVector right = end - start;
		CVector up(-right.y, right.x, 0.f);
		up = thickness * up.normed();
		setQuad(CQuad(start + up, end + up, end - up, start - up));
	}

	// *********************************************************************************
	void CCtrlQuad::setQuad(const NLMISC::CVector &pos, float radius, float angle /*=0.f*/)
	{
		H_AUTO(Rz_CCtrlQuad_setQuad)
		if (angle == 0.f)
		{
			setQuad(pos - radius * CVector::I, pos + radius * CVector::I, radius);
		}
		else
		{
			CVector right(radius * cosf(angle), radius * sinf(angle), 0.f);
			setQuad(pos - right, pos + right, radius);
		}
	}

	// *********************************************************************************
	void CCtrlQuad::setQuad(const std::string &texName, const NLMISC::CVector &srcPos, float angle /*= 0.f*/, float offCenter /* = 0.f*/)
	{
		H_AUTO(Rz_CCtrlQuad_setQuad)
		NLMISC::CVector pos = srcPos;
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		sint32 w, h;
		rVR.getTextureSizeFromId(rVR.getTextureIdFromName(texName), w, h);
		if (angle == 0.f)
		{
			if (offCenter != 0.f)
			{
				pos.x += offCenter;
			}
			setQuad(pos - 0.5f * w * CVector::I, pos + 0.5f * w * CVector::I, 0.5f * h);
		}
		else
		{
			CVector unitRadius(cosf(angle), sinf(angle), 0.f);
			CVector radius = 0.5f * w * unitRadius;
			pos += offCenter * unitRadius;
			setQuad(pos - radius, pos + radius, 0.5f * h);
		}
	}

	// *********************************************************************************
	void CCtrlQuad::setAdditif(bool additif)
	{
		H_AUTO(Rz_CCtrlQuad_setAdditif)
		_Additif = additif;
	}

	// *********************************************************************************
	void CCtrlQuad::setFiltered(bool filtered)
	{
		H_AUTO(Rz_CCtrlQuad_setFiltered)
		_Filtered = filtered;
	}

	// *********************************************************************************
	void CCtrlQuad::setPattern(float umin, float umax, TWrapMode wrapMode)
	{
		H_AUTO(Rz_CCtrlQuad_setPattern)
		nlassert((uint) wrapMode < CustomUVs);
		_UMin = umin;
		_UMax = umax;
		_WrapMode = wrapMode;
	}

	// *********************************************************************************
	void CCtrlQuad::setCustomUVs(const CUV uvs[4])
	{
		H_AUTO(Rz_CCtrlQuad_setCustomUVs)
		std::copy(uvs, uvs + 4, _CustomUVs );
		_WrapMode = CustomUVs;
	}

	// *********************************************************************************
	bool CCtrlQuad::handleEvent(const NLGUI::CEventDescriptor &/* event */)
	{
		H_AUTO(Rz_CCtrlQuad_handleEvent)
		return false;
	}

	// *********************************************************************************
	bool CCtrlQuad::contains(const NLMISC::CVector2f &pos) const
	{
		H_AUTO(Rz_CCtrlQuad_contains)
		static NLMISC::CPolygon2D poly;
		poly.Vertices.resize(4);
		poly.Vertices[0] = _Quad.V0;
		poly.Vertices[1] = _Quad.V1;
		poly.Vertices[2] = _Quad.V2;
		poly.Vertices[3] = _Quad.V3;
		return poly.contains(pos, false);
	}

}



