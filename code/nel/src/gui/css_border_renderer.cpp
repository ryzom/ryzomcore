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
#include "nel/gui/css_border_renderer.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/widget_manager.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{


	// ----------------------------------------------------------------------------
	CSSBorderRenderer::CSSBorderRenderer()
	{
		TopWidth = RightWidth = BottomWidth = LeftWidth = 0;
		TopColor = RightColor = BottomColor = LeftColor = CRGBA(128, 128, 128, 255);
		TopStyle = RightStyle = BottomStyle = LeftStyle = CSS_LINE_STYLE_SOLID;
		CurrentAlpha = 255;

		_RenderLayer = 0;
		_ModulateGlobalColor = false;

		_Border = true;
		_Dirty = true;
		_BorderTop = _BorderRight = _BorderBottom = _BorderLeft = false;

		//
		_QuadT.Uv0.set(0.f, 0.f);
		_QuadT.Uv1.set(0.f, 0.f);
		_QuadT.Uv2.set(1.f, 1.f);
		_QuadT.Uv3.set(0.f, 1.f);
		//
		_QuadR.Uv0.set(0.f, 0.f);
		_QuadR.Uv1.set(0.f, 0.f);
		_QuadR.Uv2.set(1.f, 1.f);
		_QuadR.Uv3.set(0.f, 1.f);
		//
		_QuadB.Uv0.set(0.f, 0.f);
		_QuadB.Uv1.set(0.f, 0.f);
		_QuadB.Uv2.set(1.f, 1.f);
		_QuadB.Uv3.set(0.f, 1.f);
		//
		_QuadL.Uv0.set(0.f, 0.f);
		_QuadL.Uv1.set(0.f, 0.f);
		_QuadL.Uv2.set(1.f, 1.f);
		_QuadL.Uv3.set(0.f, 1.f);
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setRenderLayer(sint layer)
	{
		_RenderLayer = layer;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setModulateGlobalColor(bool s)
	{
		_ModulateGlobalColor = s;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setRect(sint32 x, sint32 y, sint32 w, sint32 h)
	{
		_XReal = x;
		_YReal = y;
		_WReal = w;
		_HReal = h;

		_Dirty = _Border = (w > 0 && h > 0);
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setWidth(uint32 top, uint32 right, uint32 bottom, uint32 left)
	{
		TopWidth = top;
		RightWidth = right;
		BottomWidth = bottom;
		LeftWidth = left;

		_Dirty = _Border = (top > 0 || right > 0 || bottom > 0 || left > 0);
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setStyle(CSSLineStyle top, CSSLineStyle right, CSSLineStyle bottom, CSSLineStyle left)
	{
		TopStyle = top;
		RightStyle = right;
		BottomStyle = bottom;
		LeftStyle = left;

		_Dirty = _Border = true;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setColor(const NLMISC::CRGBA &top, const NLMISC::CRGBA &right, const NLMISC::CRGBA &bottom, const NLMISC::CRGBA &left)
	{
		TopColor = top;
		RightColor = right;
		BottomColor = bottom;
		LeftColor = left;

		_Dirty = true;
	}

	// ----------------------------------------------------------------------------
	uint32 CSSBorderRenderer::getTopWidth() const
	{
		if (TopStyle == CSS_LINE_STYLE_NONE || TopStyle == CSS_LINE_STYLE_HIDDEN)
			return 0;

		return TopWidth;
	}

	// ----------------------------------------------------------------------------
	uint32 CSSBorderRenderer::getRightWidth() const
	{
		if (RightStyle == CSS_LINE_STYLE_NONE || RightStyle == CSS_LINE_STYLE_HIDDEN)
			return 0;

		return RightWidth;
	}

	// ----------------------------------------------------------------------------
	uint32 CSSBorderRenderer::getBottomWidth() const
	{
		if (BottomStyle == CSS_LINE_STYLE_NONE || BottomStyle == CSS_LINE_STYLE_HIDDEN)
			return 0;

		return BottomWidth;
	}

	// ----------------------------------------------------------------------------
	uint32 CSSBorderRenderer::getLeftWidth() const
	{
		if (LeftStyle == CSS_LINE_STYLE_NONE || LeftStyle == CSS_LINE_STYLE_HIDDEN)
			return 0;

		return LeftWidth;
	}

	// ----------------------------------------------------------------------------
	uint32 CSSBorderRenderer::getLeftRightWidth() const
	{
		return getLeftWidth() + getRightWidth();
	}

	// ----------------------------------------------------------------------------
	uint32 CSSBorderRenderer::getTopBottomWidth() const
	{
		return getTopWidth() + getBottomWidth();
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::updateCoords()
	{
		_Dirty = false;
		if (!_Border) return;

		sint dTop    = getTopWidth();    _BorderTop    = dTop > 0;
		sint dRight  = getRightWidth();  _BorderRight  = dRight > 0;
		sint dBottom = getBottomWidth(); _BorderBottom = dBottom > 0;
		sint dLeft   = getLeftWidth();   _BorderLeft   = dLeft > 0;

		_Border = _BorderTop || _BorderRight || _BorderBottom || _BorderLeft;
		if (!_Border) return;

		if (_BorderTop)
		{
			// top-left
			_QuadT.V3.x = _XReal;
			_QuadT.V3.y = _YReal + _HReal;
			// top-right
			_QuadT.V2.x = _XReal + _WReal;
			_QuadT.V2.y = _YReal + _HReal;
			// bottom-right
			_QuadT.V1.x = _XReal + _WReal - dRight;
			_QuadT.V1.y = _YReal + _HReal - dTop;
			// bottom-left
			_QuadT.V0.x = _XReal + dLeft;
			_QuadT.V0.y = _YReal + _HReal - dTop;
		}

		if (_BorderRight)
		{
			// top-left
			_QuadR.V3.x = _XReal + _WReal - dRight;
			_QuadR.V3.y = _YReal + _HReal - dTop;
			// top-right
			_QuadR.V2.x = _XReal + _WReal;
			_QuadR.V2.y = _YReal + _HReal;
			// bottom-right
			_QuadR.V1.x = _XReal + _WReal;
			_QuadR.V1.y = _YReal;
			// bottom-left
			_QuadR.V0.x = _XReal + _WReal - dRight;
			_QuadR.V0.y = _YReal + dBottom;
		}

		if (_BorderBottom)
		{
			// top-left
			_QuadB.V3.x = _XReal + dLeft;
			_QuadB.V3.y = _YReal + dBottom;
			// top-right
			_QuadB.V2.x = _XReal + _WReal - dRight;
			_QuadB.V2.y = _YReal + dBottom;
			// bottom-right
			_QuadB.V1.x = _XReal + _WReal;
			_QuadB.V1.y = _YReal;
			// bottom-left
			_QuadB.V0.x = _XReal;
			_QuadB.V0.y = _YReal;
		}

		if (_BorderLeft)
		{
			// top-left
			_QuadL.V3.x = _XReal;
			_QuadL.V3.y = _YReal + _HReal;
			// top-right
			_QuadL.V2.x = _XReal + dLeft;
			_QuadL.V2.y = _YReal + _HReal - dTop;
			// bottom-right
			_QuadL.V1.x = _XReal + dLeft;
			_QuadL.V1.y = _YReal + dBottom;
			// bottom-left
			_QuadL.V0.x = _XReal;
			_QuadL.V0.y = _YReal;
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::draw() {
		if (_Dirty) updateCoords();
		if (!_Border) return;

		CViewRenderer &rVR = *CViewRenderer::getInstance();

		// TODO: no need for widget manager, if global color is set from parent
		CRGBA globalColor;
		if (_ModulateGlobalColor)
			globalColor = CWidgetManager::getInstance()->getGlobalColor();

		// TODO: monitor ModulateGlobalColor and CurrentAlpha in table checkCoords and recalculate colors on change only
		// OUTSET - bottom/right darker than normal (default table style)
		// INSET  - top/left darker than normal
		if (_BorderTop)
		{
			CRGBA borderColorT = TopColor;
			if (TopStyle == CSS_LINE_STYLE_INSET)
				borderColorT = blend(borderColorT, CRGBA::Black, 0.5f);

			if (_ModulateGlobalColor)
				borderColorT.modulateFromColor (borderColorT, globalColor);

			borderColorT.A = (uint8) (((uint16) CurrentAlpha * (uint16) borderColorT.A) >> 8);
			rVR.drawQuad(_RenderLayer, _QuadT, rVR.getBlankTextureId(), borderColorT, false);
		}
		if (_BorderRight)
		{
			CRGBA borderColorR = RightColor;
			if (RightStyle == CSS_LINE_STYLE_OUTSET)
				borderColorR = blend(borderColorR, CRGBA::Black, 0.5f);

			if (_ModulateGlobalColor)
				borderColorR.modulateFromColor (borderColorR, globalColor);

			borderColorR.A = (uint8) (((uint16) CurrentAlpha * (uint16) borderColorR.A) >> 8);
			rVR.drawQuad(_RenderLayer, _QuadR, rVR.getBlankTextureId(), borderColorR, false);
		}
		if (_BorderBottom)
		{
			CRGBA borderColorB = BottomColor;
			if (BottomStyle == CSS_LINE_STYLE_OUTSET)
				borderColorB = blend(borderColorB, CRGBA::Black, 0.5f);

			if (_ModulateGlobalColor)
				borderColorB.modulateFromColor (borderColorB, globalColor);

			borderColorB.A = (uint8) (((uint16) CurrentAlpha * (uint16) borderColorB.A) >> 8);
			rVR.drawQuad(_RenderLayer, _QuadB, rVR.getBlankTextureId(), borderColorB, false);
		}
		if (_BorderLeft)
		{
			CRGBA borderColorL = LeftColor;
			if (LeftStyle == CSS_LINE_STYLE_INSET)
				borderColorL = blend(borderColorL, CRGBA::Black, 0.5f);

			if (_ModulateGlobalColor)
				borderColorL.modulateFromColor (borderColorL, globalColor);

			borderColorL.A = (uint8) (((uint16) CurrentAlpha * (uint16) borderColorL.A) >> 8);
			rVR.drawQuad(_RenderLayer, _QuadL, rVR.getBlankTextureId(), borderColorL, false);
		}
	}

}//namespace

