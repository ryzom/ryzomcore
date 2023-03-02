// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
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
		CurrentAlpha = 255;

		m_Scale = 1.f;
		m_RenderLayer = 0;
		m_ModulateGlobalColor = false;

		m_Dirty = true;
		m_MustComputeValues = true;
		m_XReal = 0;
		m_YReal = 0;
		m_WReal = 0;
		m_HReal = 0;

		m_Viewport = NULL;
		m_FontSize = 16.f;
		m_RootFontSize = 16.f;

		m_Computed.Top = m_Computed.Right = m_Computed.Bottom = m_Computed.Left = 0;
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

		m_Dirty = (w > 0 && h > 0);
	}

	// ----------------------------------------------------------------------------
	bool CSSBorderRenderer::hasInnerShape(CSSLineStyle style) const
	{
		return style == CSS_LINE_STYLE_DOUBLE ||
				style == CSS_LINE_STYLE_GROOVE ||
				style == CSS_LINE_STYLE_RIDGE;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::computeValues()
	{
		m_MustComputeValues = false;

		// TODO :should save as computed value
		sint32 vpW=0;
		sint32 vpH=0;
		if (m_Viewport)
		{
			vpW = m_Viewport->getWReal();
			vpH = m_Viewport->getHReal();
		}

		m_Computed.Top    = m_Border.Top.empty()    ? 0 : m_Border.Top.Width.calculate(0, m_FontSize, m_RootFontSize, vpW, vpH);
		m_Computed.Right  = m_Border.Right.empty()  ? 0 : m_Border.Right.Width.calculate(0, m_FontSize, m_RootFontSize, vpW, vpH);
		m_Computed.Bottom = m_Border.Bottom.empty() ? 0 : m_Border.Bottom.Width.calculate(0, m_FontSize, m_RootFontSize, vpW, vpH);
		m_Computed.Left   = m_Border.Left.empty()   ? 0 : m_Border.Left.Width.calculate(0, m_FontSize, m_RootFontSize, vpW, vpH);
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::getAdjacentBorders(EBorderSide side, EBorderSide &adjBorderL, EBorderSide &adjBorderR) const
	{
		switch(side)
		{
			case BORDER_TOP:
				adjBorderL = BORDER_TOP_LEFT;
				adjBorderR = BORDER_TOP_RIGHT;
				break;
			case BORDER_RIGHT:
				adjBorderL = BORDER_RIGHT_BOTTOM;
				adjBorderR = BORDER_RIGHT_TOP;
				break;
			case BORDER_BOTTOM:
				adjBorderL = BORDER_BOTTOM_LEFT;
				adjBorderR = BORDER_BOTTOM_RIGHT;
				break;
			case BORDER_LEFT:
				adjBorderL = BORDER_LEFT_BOTTOM;
				adjBorderR = BORDER_LEFT_TOP;
				break;
			default:
				adjBorderL = adjBorderR = BORDER_INVALID;
				break;
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::getAdjacentBorderWidth(EBorderSide side, sint32 &adjWidthL, sint32 &adjWidthR) const
	{
		switch(side)
		{
			case BORDER_TOP:
			case BORDER_BOTTOM:
				adjWidthL = m_Computed.Left;
				adjWidthR = m_Computed.Right;
				break;
			case BORDER_LEFT:
			case BORDER_RIGHT:
				adjWidthL = m_Computed.Bottom;
				adjWidthR = m_Computed.Top;
				break;
			default:
				adjWidthL = adjWidthR = 0;
				break;
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::buildDotCornerStart(EBorderSide side, SDrawBorder shape, float x1, float y1, float radius)
	{
		NLMISC::CLine line;
		switch(side)
		{
			case BORDER_TOP:
				line.V0.set(m_XReal + m_Computed.Left, m_YReal + m_HReal - m_Computed.Top, 0);
				line.V1.set(m_XReal, m_YReal + m_HReal, 0);
				buildDotCorner(shape, x1, y1, radius, line);
				break;
			case BORDER_RIGHT:
				line.V0.set(m_XReal + m_WReal, m_YReal, 0);
				line.V1.set(m_XReal + m_WReal - m_Computed.Right, m_YReal + m_Computed.Bottom, 0);
				buildDotCorner(shape, x1, y1, radius, line);
				break;
			case BORDER_BOTTOM:
				line.V0.set(m_XReal, m_YReal, 0);
				line.V1.set(m_XReal + m_Computed.Left, m_YReal + m_Computed.Bottom, 0);
				buildDotCorner(shape, x1, y1, radius, line);
				break;
			case BORDER_LEFT:
				line.V0.set(m_XReal + m_Computed.Left, m_YReal + m_Computed.Bottom, 0);
				line.V1.set(m_XReal, m_YReal, 0);
				buildDotCorner(shape, x1, y1, radius, line);
				break;
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::buildDotCornerEnd(EBorderSide side, SDrawBorder shape, float x1, float y1, float radius)
	{
		NLMISC::CLine line;
		switch(side)
		{
			case BORDER_TOP:
				line.V0.set(m_XReal + m_WReal, m_YReal + m_HReal, 0);
				line.V1.set(m_XReal + m_WReal - m_Computed.Right, m_YReal + m_HReal - m_Computed.Top, 0);
				buildDotCorner(shape, x1, y1, radius, line);
				break;
			case BORDER_RIGHT:
				line.V0.set(m_XReal + m_WReal - m_Computed.Right, m_YReal + m_HReal - m_Computed.Top, 0);
				line.V1.set(m_XReal + m_WReal, m_YReal + m_HReal, 0);
				buildDotCorner(shape, x1, y1, radius, line);
				break;
			case BORDER_BOTTOM:
				line.V0.set(m_XReal + m_WReal - m_Computed.Right, m_YReal + m_Computed.Bottom, 0);
				line.V1.set(m_XReal + m_WReal, m_YReal, 0);
				buildDotCorner(shape, x1, y1, radius, line);
				break;
			case BORDER_LEFT:
				line.V0.set(m_XReal, m_YReal + m_HReal, 0);
				line.V1.set(m_XReal + m_Computed.Left, m_YReal + m_HReal - m_Computed.Top, 0);
				buildDotCorner(shape, x1, y1, radius, line);
				break;
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::buildDashedBorder(EBorderSide side)
	{
		CSSBorder border;

		float x, y;
		bool horizontal;
		sint32 width, thickness;
		switch(side)
		{
			case BORDER_TOP:
				horizontal = true;
				border = m_Border.Top;
				width = m_WReal;
				thickness = m_Computed.Top;
				x = m_XReal;
				y = m_YReal + m_HReal - thickness;
				break;
			case BORDER_RIGHT:
				horizontal = false;
				border = m_Border.Right;
				width = m_HReal;
				thickness = m_Computed.Right;
				x = m_XReal + m_WReal - thickness;
				y = m_YReal;
				break;
			case BORDER_BOTTOM:
				horizontal = true;
				border = m_Border.Bottom;
				width = m_WReal;
				thickness = m_Computed.Bottom;
				x = m_XReal;
				y = m_YReal;
				break;
			case BORDER_LEFT:
				horizontal = false;
				border = m_Border.Left;
				width = m_HReal;
				thickness = m_Computed.Left;
				x = m_XReal;
				y = m_YReal;
				break;
			default:
				return;
		}

		if (width < 1) return;
		if (thickness < 1) return;

		SDrawBorder shape;
		shape.Color = border.Color;
		shape.Quad.Uv0.set(0.f, 0.f);
		shape.Quad.Uv1.set(1.f, 0.f);
		shape.Quad.Uv2.set(1.f, 1.f);
		shape.Quad.Uv3.set(0.f, 1.f);

		EBorderSide adjBorderL, adjBorderR;
		getAdjacentBorders(side, adjBorderL, adjBorderR);

		sint32 adjWidthL, adjWidthR;
		getAdjacentBorderWidth(side, adjWidthL, adjWidthR);

		// get alias to x/y
		float &xy = horizontal ? x : y;

		if (border.Style == CSS_LINE_STYLE_DOTTED)
		{
			// thick border with little or no content might try to draw larger dot that fits
			float radius = std::min(thickness / 2.f, width / 2.f);
			float dot = thickness;

			sint32 count = std::floor((float)width / dot);
			// 3n (dot, gap, dot) groups; count should be odd
			if ((count % 2) == 0) count += 1;

			if (count == 1)
			{
				// fallback to single dot
				if (horizontal)
				{
					x += width / 2.f;
					y += radius;
				}
				else
				{
					x += radius;
					y += width / 2.f;
				}

				buildDotSegments(shape, x, y, radius);
				return;
			}

			// center-to-center spacing for dots
			float spacing = dot + (width - dot * count) / (count-1);

			x += radius;
			y += radius;

			if (adjWidthL > 0)
				buildDotCornerStart(side, shape, x, y, radius);
			else
				buildDotSegments(shape, x, y, radius);

			xy += spacing;
			count--;

			if (adjWidthR > 0)
				count--;

			bool isDot = false;
			while(count > 0)
			{
				if (isDot)
					buildDotSegments(shape, x, y, radius);

				isDot = !isDot;
				xy += spacing;
				count--;
			}

			if (adjWidthR > 0)
				buildDotCornerEnd(side, shape, x, y, radius);
		}
		else
		{
			sint32 innerWidth = width;
			if (adjWidthL > 0) innerWidth -= adjWidthL;
			if (adjWidthR > 0) innerWidth -= adjWidthR;

			sint32 count = std::floor((float)innerWidth * 2.f / (thickness * 3));

			if ((float)innerWidth < 2.f * thickness)
			{
				buildSolidBorder(side);
				return;
			}

			// 4n groups (halfDash, halfGap, halfGap, halfDash)
			     if ((count % 4) == 1) count += 3;
			else if ((count % 4) == 2) count += 2;
			else if ((count % 4) == 3) count += 1;

			float halfDash = (float)innerWidth / (float)count;
			float fullDash = halfDash * 2.f;

			// draw half dash or full corner
			makeBorderQuad(side, shape, x, y, adjWidthL + halfDash, thickness);
			if (adjWidthL > 0)
				makeCornerQuad(adjBorderL, shape);
			m_DrawBorders.push_back(shape);
			xy += adjWidthL + halfDash;

			// start/end half dash that are merged with corners
			count -= 2;

			bool isDash = false;
			while(count > 0)
			{
				if (isDash)
				{
					makeBorderQuad(side, shape, x, y, fullDash, thickness);
					m_DrawBorders.push_back(shape);
				}
				isDash = !isDash;

				xy += fullDash;
				count -= 2;
			}

			// draw half dash or full corner
			makeBorderQuad(side, shape, x, y, adjWidthR + halfDash, thickness);
			if (adjWidthR > 0)
				makeCornerQuad(adjBorderR, shape);

			m_DrawBorders.push_back(shape);
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::buildSolidBorder(EBorderSide side)
	{
		float x, y;
		sint width, thickness;
		CSSBorder border;
		switch(side)
		{
			case BORDER_TOP:
				border = m_Border.Top;
				width = m_WReal;
				thickness = m_Computed.Top;
				x = m_XReal;
				y = m_YReal + m_HReal - thickness;
				break;
			case BORDER_RIGHT:
				border = m_Border.Right;
				width = m_HReal;
				thickness = m_Computed.Right;
				x = m_XReal + m_WReal - thickness;
				y = m_YReal;
				break;
			case BORDER_BOTTOM:
				border = m_Border.Bottom;
				width = m_WReal;
				thickness = m_Computed.Bottom;
				x = m_XReal;
				y = m_YReal;
				break;
			case BORDER_LEFT:
				border = m_Border.Left;
				width = m_HReal;
				thickness = m_Computed.Left;
				x = m_XReal;
				y = m_YReal;
				break;
			default:
				return;
		}

		SDrawBorder shape;
		shape.Color = border.Color;
		shape.Quad.Uv0.set(0.f, 0.f);
		shape.Quad.Uv1.set(1.f, 0.f);
		shape.Quad.Uv2.set(1.f, 1.f);
		shape.Quad.Uv3.set(0.f, 1.f);

		if (border.Style == CSS_LINE_STYLE_INSET && (side == BORDER_TOP || side == BORDER_LEFT))
			shape.Color = blend(border.Color, CRGBA::Black, 0.5f);
		else if (border.Style == CSS_LINE_STYLE_OUTSET && (side == BORDER_BOTTOM || side == BORDER_RIGHT))
			shape.Color = blend(border.Color, CRGBA::Black, 0.5f);

		// solid border
		{
			EBorderSide adjBorderL, adjBorderR;
			getAdjacentBorders(side, adjBorderL, adjBorderR);

			makeBorderQuad(side, shape, x, y, width, thickness);
			makeCornerQuad(adjBorderL, shape);
			makeCornerQuad(adjBorderR, shape);

			m_DrawBorders.push_back(shape);
		}

		if (hasInnerShape(border.Style))
		{
			if (side == BORDER_TOP || side == BORDER_LEFT)
			{
				if (border.Style == CSS_LINE_STYLE_GROOVE)
					m_DrawBorders.back().Color = blend(border.Color, CRGBA::Black, 0.5f);
				else if (border.Style == CSS_LINE_STYLE_RIDGE)
					shape.Color = blend(border.Color, CRGBA::Black, 0.5f);
			}
			else if (side == BORDER_BOTTOM || side == BORDER_RIGHT)
			{
				if (border.Style == CSS_LINE_STYLE_GROOVE)
					shape.Color = blend(border.Color, CRGBA::Black, 0.5f);
				else if (border.Style == CSS_LINE_STYLE_RIDGE)
					m_DrawBorders.back().Color = blend(border.Color, CRGBA::Black, 0.5f);
			}

			sint32 adjWidthL, adjWidthR;
			getAdjacentBorderWidth(side, adjWidthL, adjWidthR);

			float iStart, iMiddle, iEnd;
			if (border.Style == CSS_LINE_STYLE_DOUBLE)
			{
				iStart  = 2 * adjWidthL  / 3.f;
				iMiddle = 2 * thickness  / 3.f;
				iEnd    = 2 * adjWidthR  / 3.f;
			} else {
				iStart  = adjWidthL / 2.f;
				iMiddle = thickness / 2.f;
				iEnd    = adjWidthR / 2.f;
			}

			// create inner shape and remove overlapping from outer shape
			switch(side)
			{
				case BORDER_TOP:
					m_DrawBorders.back().Quad.V0.x -= iStart; m_DrawBorders.back().Quad.V0.y  += iMiddle;
					m_DrawBorders.back().Quad.V1.x += iEnd;   m_DrawBorders.back().Quad.V1.y  += iMiddle;
					shape.Quad.V3.x += iStart; shape.Quad.V3.y -= iMiddle;
					shape.Quad.V2.x -= iEnd;   shape.Quad.V2.y -= iMiddle;
					break;
				case BORDER_BOTTOM:
					m_DrawBorders.back().Quad.V2.x += iEnd;   m_DrawBorders.back().Quad.V2.y -= iMiddle;
					m_DrawBorders.back().Quad.V3.x -= iStart; m_DrawBorders.back().Quad.V3.y -= iMiddle;
					shape.Quad.V1.x -= iEnd;   shape.Quad.V1.y += iMiddle;
					shape.Quad.V0.x += iStart; shape.Quad.V0.y += iMiddle;
					break;
				case BORDER_RIGHT:
					m_DrawBorders.back().Quad.V3.x += iMiddle; m_DrawBorders.back().Quad.V3.y += iEnd;
					m_DrawBorders.back().Quad.V0.x += iMiddle; m_DrawBorders.back().Quad.V0.y -= iStart;
					shape.Quad.V2.x -= iMiddle; shape.Quad.V2.y -= iEnd;
					shape.Quad.V1.x -= iMiddle; shape.Quad.V1.y += iStart;
					break;
				case BORDER_LEFT:
					m_DrawBorders.back().Quad.V2.x -= iMiddle; m_DrawBorders.back().Quad.V2.y += iEnd;
					m_DrawBorders.back().Quad.V1.x -= iMiddle; m_DrawBorders.back().Quad.V1.y -= iStart;
					shape.Quad.V3.x += iMiddle; shape.Quad.V3.y -= iEnd;
					shape.Quad.V0.x += iMiddle; shape.Quad.V0.y += iStart;
					break;
			}
			m_DrawBorders.push_back(shape);
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::makeBorderQuad(EBorderSide side, SDrawBorder &shape, float x, float y, float width, float thickness) const
	{
		float quadW, quadH;
		switch(side)
		{
			case BORDER_TOP:   quadW = width; quadH = thickness; break;
			case BORDER_BOTTOM:quadW = width; quadH = thickness; break;
			case BORDER_LEFT:  quadW = thickness; quadH = width; break;
			case BORDER_RIGHT: quadW = thickness; quadH = width; break;
			default: return;
		}
		shape.Quad.V3.x = x;         shape.Quad.V3.y = y + quadH;
		shape.Quad.V2.x = x + quadW; shape.Quad.V2.y = y + quadH;
		shape.Quad.V1.x = x + quadW; shape.Quad.V1.y = y;
		shape.Quad.V0.x = x;         shape.Quad.V0.y = y;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::makeCornerQuad(EBorderSide side, SDrawBorder &shape) const
	{
		switch(side)
		{
			case BORDER_TOP_LEFT:     shape.Quad.V0.x += m_Computed.Left; break;
			case BORDER_TOP_RIGHT:    shape.Quad.V1.x -= m_Computed.Right; break;
			case BORDER_RIGHT_TOP:    shape.Quad.V3.y -= m_Computed.Top; break;
			case BORDER_RIGHT_BOTTOM: shape.Quad.V0.y += m_Computed.Bottom; break;
			case BORDER_BOTTOM_LEFT:  shape.Quad.V3.x += m_Computed.Left; break;
			case BORDER_BOTTOM_RIGHT: shape.Quad.V2.x -= m_Computed.Right; break;
			case BORDER_LEFT_TOP:     shape.Quad.V2.y -= m_Computed.Top; break;
			case BORDER_LEFT_BOTTOM:  shape.Quad.V1.y += m_Computed.Bottom; break;
		}
	}

	// ----------------------------------------------------------------------------
	static bool getCircleLineIntersection(float x, float y, float r, const NLMISC::CLine &line, NLMISC::CLine &result)
	{
		float dx = line.V0.x - line.V1.x;
		float dy = line.V0.y - line.V1.y;
		float rx = line.V0.x-x;
		float ry = line.V0.y-y;
		float a = dx*dx + dy*dy;
		float b = 2*(dx * rx + dy * ry);
		float c = rx * rx + ry * ry - r*r;
		float d = b*b - 4 * a * c;

		if (d < 0)
			return false;

		if (d == 0)
		{
			// single intersection
			//float t = -b / (2*a);
			//result.V0.x = result.V1.x = line.V0.x + t * dx;
			//result.V0.y = result.V1.y = line.V0.y + t * dy;
			return false;
		}

		float t;
		t = (-b + sqrt(d)) / (2 * a);
		result.V0.x = line.V0.x + t * dx;
		result.V0.y = line.V0.y + t * dy;

		t = (-b - sqrt(d)) / (2 * a);
		result.V1.x = line.V0.x + t * dx;
		result.V1.y = line.V0.y + t * dy;

		return true;
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::buildDotCorner(SDrawBorder shape, float cX, float cY, float cR, const NLMISC::CLine &line)
	{
		static const float twopi = 2 * NLMISC::Pi;
		NLMISC::CLine out;
		if (getCircleLineIntersection(cX, cY, cR, line, out))
		{
			float fromAngle = std::atan2(out.V0.y - cY, out.V0.x - cX);
			float toAngle   = std::atan2(out.V1.y - cY, out.V1.x - cX);
			if (fromAngle < 0) fromAngle += twopi;
			if (toAngle < 0) toAngle += twopi;
			fromAngle /= twopi;
			toAngle /= twopi;
			buildDotSegments(shape, cX, cY, cR, fromAngle, toAngle);
		} else {
			buildDotSegments(shape, cX, cY, cR);
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::buildDotSegments(SDrawBorder shape, float x, float y, float radius, float fromAngle, float toAngle)
	{
		static const float pi = (float)NLMISC::Pi;
		static const float twopi = (float)(NLMISC::Pi * 2);
		static const uint minSectors = 12;

		// use single quad if dot is small
		if (2 * radius * m_Scale < 4)
		{
			makeBorderQuad(BORDER_TOP, shape, x - radius, y - radius, radius * 2, radius * 2);
			m_DrawBorders.push_back(shape);
			return;
		}

		// number of sectors for full circle
		uint sectors = std::max(minSectors, (uint)std::ceil(radius * m_Scale));

		float arcLength;
		if (toAngle < fromAngle)
			arcLength = twopi * (1 + toAngle - fromAngle);
		else
			arcLength = twopi * (toAngle - fromAngle);

		// sectors to draw
		float arcSectors = ceil(arcLength * sectors / twopi );
		float arcSectorLength = arcLength / arcSectors;

		if (arcSectors <= 1)
			return;

		if (arcLength < pi)
		{
			// only small segment is visible
			float px1 = x + radius * cosf(twopi * fromAngle);
			float py1 = y + radius * sinf(twopi * fromAngle);
			float px2 = x + radius * cosf(twopi * toAngle);
			float py2 = y + radius * sinf(twopi * toAngle);
			float cx = (px1 + px2) / 2.f;
			float cy = (py1 + py2) / 2.f;
			shape.Quad.V0.x = cx; shape.Quad.V0.y = cy;
			shape.Quad.V1.x = cx; shape.Quad.V1.y = cy;
		}
		else
		{
			shape.Quad.V0.x = x; shape.Quad.V0.y = y;
			shape.Quad.V1.x = x; shape.Quad.V1.y = y;
		}

		float a1 = fromAngle * twopi;
		uint step;
		for(step = 0; step < (uint)arcSectors; step++)
		{
			float a2 = a1 + arcSectorLength;

			shape.Quad.V2.x = x + radius * cosf(a1); shape.Quad.V2.y = y + radius * sinf(a1);
			shape.Quad.V3.x = x + radius * cosf(a2); shape.Quad.V3.y = y + radius * sinf(a2);

			m_DrawBorders.push_back(shape);

			a1 = a2;
		}

		// build last sector if requested range is over 180deg
		if (arcLength > pi && arcLength < twopi)
		{
			float a2 = fromAngle * twopi;
			shape.Quad.V2.x = x + radius * cosf(a1); shape.Quad.V2.y = y + radius * sinf(a1);
			shape.Quad.V3.x = x + radius * cosf(a2); shape.Quad.V3.y = y + radius * sinf(a2);
			m_DrawBorders.push_back(shape);
		}
	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::updateCoords()
	{
		m_Dirty = false;
		m_DrawBorders.clear();

		if (m_MustComputeValues)
			computeValues();

		if (m_Computed.Top > 0 && m_Border.Top.Color.A > 0)
		{
			if (m_Border.Top.Style == CSS_LINE_STYLE_DASHED || m_Border.Top.Style == CSS_LINE_STYLE_DOTTED)
				buildDashedBorder(BORDER_TOP);
			else
				buildSolidBorder(BORDER_TOP);
		}

		if (m_Computed.Bottom > 0 && m_Border.Bottom.Color.A > 0)
		{
			if (m_Border.Bottom.Style == CSS_LINE_STYLE_DASHED || m_Border.Bottom.Style == CSS_LINE_STYLE_DOTTED)
				buildDashedBorder(BORDER_BOTTOM);
			else
				buildSolidBorder(BORDER_BOTTOM);
		}

		if (m_Computed.Right > 0 && m_Border.Right.Color.A > 0)
		{
			if (m_Border.Right.Style == CSS_LINE_STYLE_DASHED || m_Border.Right.Style == CSS_LINE_STYLE_DOTTED)
				buildDashedBorder(BORDER_RIGHT);
			else
				buildSolidBorder(BORDER_RIGHT);
		}

		if (m_Computed.Left > 0 && m_Border.Left.Color.A > 0)
		{
			if (m_Border.Left.Style == CSS_LINE_STYLE_DASHED || m_Border.Left.Style == CSS_LINE_STYLE_DOTTED)
				buildDashedBorder(BORDER_LEFT);
			else
				buildSolidBorder(BORDER_LEFT);
		}

	}

	// ----------------------------------------------------------------------------
	void CSSBorderRenderer::draw() {
		if (m_Dirty) updateCoords();
		if (m_DrawBorders.empty()) return;

		CViewRenderer &rVR = *CViewRenderer::getInstance();

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

