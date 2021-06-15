// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
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

		m_RenderLayer = 0;
		m_ModulateGlobalColor = false;

		m_Border = true;
		m_Dirty = true;
		m_BorderTop = m_BorderRight = m_BorderBottom = m_BorderLeft = false;
		m_XReal = 0;
		m_YReal = 0;
		m_WReal = 0;
		m_HReal = 0;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setRenderLayer(sint layer)
	{
		m_RenderLayer = layer;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setModulateGlobalColor(bool s)
	{
		m_ModulateGlobalColor = s;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setRect(sint32 x, sint32 y, sint32 w, sint32 h)
	{
		m_XReal = x;
		m_YReal = y;
		m_WReal = w;
		m_HReal = h;

		m_Dirty = m_Border = (w > 0 && h > 0);
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setWidth(uint32 top, uint32 right, uint32 bottom, uint32 left)
	{
		TopWidth = top;
		RightWidth = right;
		BottomWidth = bottom;
		LeftWidth = left;

		m_Dirty = m_Border = (top > 0 || right > 0 || bottom > 0 || left > 0);
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setStyle(CSSLineStyle top, CSSLineStyle right, CSSLineStyle bottom, CSSLineStyle left)
	{
		TopStyle = top;
		RightStyle = right;
		BottomStyle = bottom;
		LeftStyle = left;

		m_Dirty = m_Border = true;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::setColor(const NLMISC::CRGBA &top, const NLMISC::CRGBA &right, const NLMISC::CRGBA &bottom, const NLMISC::CRGBA &left)
	{
		TopColor = top;
		RightColor = right;
		BottomColor = bottom;
		LeftColor = left;

		m_Dirty = true;
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
	bool CSSBorderRenderer::hasInnerShape(CSSLineStyle style) const
	{
		return style == CSS_LINE_STYLE_DOUBLE ||
				style == CSS_LINE_STYLE_GROOVE ||
				style == CSS_LINE_STYLE_RIDGE;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::updateCoords()
	{
		m_Dirty = false;
		m_DrawBorders.clear();
		if (!m_Border) return;

		sint dTop    = getTopWidth();    m_BorderTop    = dTop > 0;
		sint dRight  = getRightWidth();  m_BorderRight  = dRight > 0;
		sint dBottom = getBottomWidth(); m_BorderBottom = dBottom > 0;
		sint dLeft   = getLeftWidth();   m_BorderLeft   = dLeft > 0;

		m_Border = m_BorderTop || m_BorderRight || m_BorderBottom || m_BorderLeft;
		if (!m_Border) return;

		sint xTop = m_YReal + m_HReal;
		sint xRight = m_XReal + m_WReal;

		sint bLeft = m_XReal + dLeft;
		sint bRight = xRight - dRight;
		sint bTop = xTop - dTop;
		sint bBottom = m_YReal + dBottom;

		SDrawBorder shape;
		shape.Quad.Uv0.set(0.f, 0.f);
		shape.Quad.Uv1.set(1.f, 0.f);
		shape.Quad.Uv2.set(1.f, 1.f);
		shape.Quad.Uv3.set(0.f, 1.f);

		// V3 - top-left
		// V2 - top-right
		// V1 - bottom-right
		// V0 - bottom-left
		if (m_BorderTop)
		{
			if (TopStyle == CSS_LINE_STYLE_INSET || TopStyle == CSS_LINE_STYLE_GROOVE)
				shape.Color = blend(TopColor, CRGBA::Black, 0.5f);
			else
				shape.Color = TopColor;

			shape.Quad.V3.x = m_XReal; shape.Quad.V3.y = xTop;
			shape.Quad.V2.x = xRight; shape.Quad.V2.y = xTop;
			shape.Quad.V1.x = bRight; shape.Quad.V1.y = bTop;
			shape.Quad.V0.x = bLeft;  shape.Quad.V0.y = bTop;
			m_DrawBorders.push_back(shape);

			if (hasInnerShape(TopStyle))
			{
				float iLeft, iTop, iRight;
				if (TopStyle == CSS_LINE_STYLE_DOUBLE)
				{
					iLeft  = 2*dLeft   / 3.f;
					iTop   = 2*dBottom / 3.f;
					iRight = 2*dRight  / 3.f;
				} else {
					iLeft  = dLeft  / 2.f;
					iTop   = dTop   / 2.f;
					iRight = dRight / 2.f;
				}

				if (TopStyle == CSS_LINE_STYLE_RIDGE)
					shape.Color = blend(TopColor, CRGBA::Black, 0.5f);
				else
					shape.Color = TopColor;

				// create inner border shape and remove overlapping from outer shape
				m_DrawBorders.back().Quad.V0.x  -= iLeft; m_DrawBorders.back().Quad.V0.y  += iTop;
				m_DrawBorders.back().Quad.V1.x  += iRight; m_DrawBorders.back().Quad.V1.y  += iTop;
				shape.Quad.V3.x += iLeft; shape.Quad.V3.y -= iTop;
				shape.Quad.V2.x -= iRight; shape.Quad.V2.y -= iTop;
				m_DrawBorders.push_back(shape);
			}
		}

		if (m_BorderBottom)
		{
			if (BottomStyle == CSS_LINE_STYLE_OUTSET || BottomStyle == CSS_LINE_STYLE_RIDGE)
				shape.Color = blend(BottomColor, CRGBA::Black, 0.5f);
			else
				shape.Color = BottomColor;

			shape.Quad.V3.x = bLeft; shape.Quad.V3.y = bBottom;
			shape.Quad.V2.x = bRight; shape.Quad.V2.y = bBottom;
			shape.Quad.V1.x = xRight; shape.Quad.V1.y = m_YReal;
			shape.Quad.V0.x = m_XReal; shape.Quad.V0.y = m_YReal;
			m_DrawBorders.push_back(shape);

			if (hasInnerShape(BottomStyle))
			{
				float iLeft, iBottom, iRight;
				if (BottomStyle == CSS_LINE_STYLE_DOUBLE)
				{
					iLeft   = 2*dLeft   / 3.f;
					iBottom = 2*dBottom / 3.f;
					iRight  = 2*dRight  / 3.f;
				}
				else
				{
					iLeft   = dLeft   / 2.f;
					iBottom = dBottom / 2.f;
					iRight  = dRight  / 2.f;
				}

				if (BottomStyle == CSS_LINE_STYLE_GROOVE)
					shape.Color = blend(shape.Color, CRGBA::Black, 0.5f);
				else
					shape.Color = BottomColor;

				m_DrawBorders.back().Quad.V2.x  += iRight;  m_DrawBorders.back().Quad.V2.y -= iBottom;
				m_DrawBorders.back().Quad.V3.x  -= iLeft;  m_DrawBorders.back().Quad.V3.y -= iBottom;
				shape.Quad.V1.x -= iRight; shape.Quad.V1.y += iBottom;
				shape.Quad.V0.x += iLeft; shape.Quad.V0.y += iBottom;
				m_DrawBorders.push_back(shape);
			}
		}

		if (m_BorderRight)
		{
			if (RightStyle == CSS_LINE_STYLE_OUTSET || RightStyle == CSS_LINE_STYLE_RIDGE)
				shape.Color = blend(RightColor, CRGBA::Black, 0.5f);
			else
				shape.Color = RightColor;

			shape.Quad.V3.x = bRight; shape.Quad.V3.y = bTop;
			shape.Quad.V2.x = xRight; shape.Quad.V2.y = xTop;
			shape.Quad.V1.x = xRight; shape.Quad.V1.y = m_YReal;
			shape.Quad.V0.x = bRight; shape.Quad.V0.y = bBottom;
			m_DrawBorders.push_back(shape);

			if (hasInnerShape(RightStyle))
			{
				float iTop, iRight, iBottom;
				if (RightStyle == CSS_LINE_STYLE_DOUBLE)
				{
					iTop = 2*dTop / 3.f;
					iRight = 2*dRight / 3.f;
					iBottom = 2*dBottom / 3.f;
				} else {
					iTop = dTop / 2.f;
					iRight = dRight / 2.f;
					iBottom = dBottom / 2.f;
				}

				if (RightStyle == CSS_LINE_STYLE_GROOVE)
					shape.Color = blend(shape.Color, CRGBA::Black, 0.5f);
				else
					shape.Color = RightColor;

				m_DrawBorders.back().Quad.V3.x  += iRight;  m_DrawBorders.back().Quad.V3.y += iTop;
				m_DrawBorders.back().Quad.V0.x  += iRight;  m_DrawBorders.back().Quad.V0.y -= iBottom;
				shape.Quad.V2.x -= iRight; shape.Quad.V2.y -= iTop;
				shape.Quad.V1.x -= iRight; shape.Quad.V1.y += iBottom;
				m_DrawBorders.push_back(shape);
			}
		}

		if (m_BorderLeft)
		{
			if (LeftStyle == CSS_LINE_STYLE_INSET || LeftStyle == CSS_LINE_STYLE_GROOVE)
				shape.Color = blend(LeftColor, CRGBA::Black, 0.5f);
			else
				shape.Color = LeftColor;

			shape.Quad.V3.x = m_XReal; shape.Quad.V3.y = xTop;
			shape.Quad.V2.x = bLeft; shape.Quad.V2.y = bTop;
			shape.Quad.V1.x = bLeft; shape.Quad.V1.y = bBottom;
			shape.Quad.V0.x = m_XReal; shape.Quad.V0.y = m_YReal;

			m_DrawBorders.push_back(shape);

			if (hasInnerShape(LeftStyle))
			{
				if (LeftStyle == CSS_LINE_STYLE_RIDGE)
					shape.Color = blend(LeftColor, CRGBA::Black, 0.5f);
				else
					shape.Color = LeftColor;

				float iTop, iLeft, iBottom;
				if (LeftStyle == CSS_LINE_STYLE_DOUBLE)
				{
					iTop    = 2*dTop    / 3.f;
					iLeft   = 2*dLeft   / 3.f;
					iBottom = 2*dBottom / 3.f;
				} else {
					iTop = dTop / 2.f;
					iLeft = dLeft / 2.f;
					dBottom = dBottom / 2.f;
				}

				m_DrawBorders.back().Quad.V2.x  -= iLeft;  m_DrawBorders.back().Quad.V2.y += iTop;
				m_DrawBorders.back().Quad.V1.x  -= iLeft;  m_DrawBorders.back().Quad.V1.y -= iBottom;
				shape.Quad.V3.x += iLeft; shape.Quad.V3.y -= iTop;
				shape.Quad.V0.x += iLeft; shape.Quad.V0.y += iBottom;
				m_DrawBorders.push_back(shape);
			}
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::draw() {
		if (m_Dirty) updateCoords();
		if (!m_Border) return;

		CViewRenderer &rVR = *CViewRenderer::getInstance();

		// TODO: no need for widget manager, if global color is set from parent
		CRGBA globalColor;
		if (m_ModulateGlobalColor)
			globalColor = CWidgetManager::getInstance()->getGlobalColor();

		sint32 texId = rVR.getBlankTextureId();
		for(uint i = 0; i < m_DrawBorders.size(); ++i)
		{
			CRGBA color = m_DrawBorders[i].Color;
			if (m_ModulateGlobalColor)
				color.modulateFromColor (color, globalColor);

			color.A = (uint8) (((uint16) CurrentAlpha * (uint16) color.A) >> 8);
			rVR.drawQuad(m_RenderLayer, m_DrawBorders[i].Quad, texId, color, false);
		}
	}

}//namespace

