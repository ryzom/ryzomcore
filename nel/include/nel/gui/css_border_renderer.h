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



#ifndef NL_CSS_BORDER_RENDERER_H
#define NL_CSS_BORDER_RENDERER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/geom_ext.h"
#include "nel/gui/css_types.h"
#include "nel/gui/css_border.h"

namespace NLGUI
{
	class CInterfaceElement;

	/**
	 * \brief Border renderer for GUI classes
	 * \date 2019-09-03 10:50 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CSSBorderRenderer
	{
	private:
		enum EBorderSide
		{
			BORDER_TOP_LEFT = 0,
			BORDER_TOP_RIGHT,
			BORDER_BOTTOM_LEFT,
			BORDER_BOTTOM_RIGHT,
			BORDER_LEFT_TOP,
			BORDER_RIGHT_TOP,
			BORDER_LEFT_BOTTOM,
			BORDER_RIGHT_BOTTOM,
			BORDER_TOP,
			BORDER_RIGHT,
			BORDER_BOTTOM,
			BORDER_LEFT,
			BORDER_INVALID
		};
		// parent element screen coordinates
		sint32 m_XReal, m_YReal;
		sint32 m_WReal, m_HReal;

		struct SDrawBorder
		{
			NLMISC::CQuadUV Quad;
			NLMISC::CRGBA Color;
		};
		std::vector<SDrawBorder> m_DrawBorders;

		sint8 m_RenderLayer;
		bool m_ModulateGlobalColor;

		// if true, then updateCoords() is called from draw()
		bool m_Dirty;
		// UI scale, used to calculate number of segments to draw for circle
		float m_Scale;

		CSSRect<CSSBorder> m_Border;
		CSSRect<sint32> m_Computed;

		// font size for 'rem'
		float m_RootFontSize;

		// font size for 'em'
		float m_FontSize;

		// if true, then CSSLength values are recomputed
		bool m_MustComputeValues;

		// viewport element for vw,wh,vmin,vmax
		CInterfaceElement* m_Viewport;

		// update CSSLength values
		void computeValues();

		void getAdjacentBorders(EBorderSide side, EBorderSide &adjBorderL, EBorderSide &adjBorderR) const;
		void getAdjacentBorderWidth(EBorderSide side, sint32 &adjWidthL, sint32 &adjWidthR) const;
		// dot
		void buildDotCornerStart(EBorderSide side, SDrawBorder shape, float x1, float y1, float radius);
		void buildDotCornerEnd(EBorderSide side, SDrawBorder shape, float x1, float y1, float radius);
		void buildDotCorner(SDrawBorder shape, float x, float y, float r, const NLMISC::CLine &line);
		// draw circle, angle is CCW between 0..1 (3'o'clock being 0deg).
		void buildDotSegments(SDrawBorder shape, float x, float y, float radius, float fromAngle=0.f, float toAngle=1.f);
		// dash
		void makeBorderQuad(EBorderSide side, SDrawBorder &shape, float x, float y, float width, float thickness) const;
		void makeCornerQuad(EBorderSide side, SDrawBorder &shape) const;
		void buildDashedBorder(EBorderSide side);
		void buildSolidBorder(EBorderSide side);

		bool hasInnerShape(CSSLineStyle style) const;

	public:
		// alpha value from parent
		uint8 CurrentAlpha;

	public:
		CSSBorderRenderer();

		void setRenderLayer(sint layer);
		void setModulateGlobalColor(bool m);

		void setRect(sint32 x, sint32 y, sint32 w, sint32 h);

		void setBorder(const CSSRect<CSSBorder> &b) { m_Dirty = true; m_Border = b; }

		void updateCoords();
		void invalidateCoords() { m_Dirty = true; }
		void invalidateContent() { m_Dirty = true; }

		bool isEmpty() const {
			return (m_Border.Top.Width.getFloat() +
				m_Border.Right.Width.getFloat() +
				m_Border.Bottom.Width.getFloat() +
				m_Border.Left.Width.getFloat()) == 0;
		}

		uint32 getTopWidth()    { if (m_MustComputeValues) computeValues(); return m_Computed.Top; }
		uint32 getRightWidth()  { if (m_MustComputeValues) computeValues(); return m_Computed.Right; }
		uint32 getBottomWidth() { if (m_MustComputeValues) computeValues(); return m_Computed.Bottom; }
		uint32 getLeftWidth()   { if (m_MustComputeValues) computeValues(); return m_Computed.Left; }

		uint32 getLeftRightWidth() { if (m_MustComputeValues) computeValues(); return m_Computed.Left + m_Computed.Right; }
		uint32 getTopBottomWidth() { if (m_MustComputeValues) computeValues(); return m_Computed.Top + m_Computed.Bottom; }

		NLMISC::CRGBA getTopColor() const { return m_Border.Top.Color; }
		NLMISC::CRGBA getRightColor() const { return m_Border.Right.Color; }
		NLMISC::CRGBA getBottomColor() const { return m_Border.Bottom.Color; }
		NLMISC::CRGBA getLeftColor() const { return m_Border.Left.Color; }


		void setWidth(uint width)
		{
			m_Dirty = true;
			m_MustComputeValues = true;
			m_Border.Top.Width.setFloatValue(width, "px");
			m_Border.Right.Width.setFloatValue(width, "px");
			m_Border.Bottom.Width.setFloatValue(width, "px");
			m_Border.Left.Width.setFloatValue(width, "px");
		}

		void setColor(const NLMISC::CRGBA &color)
		{
			m_Dirty = true;
			m_Border.Top.Color = color;
			m_Border.Right.Color = color;
			m_Border.Bottom.Color = color;
			m_Border.Left.Color = color;
		}

		// sizes for em, rem units
		void setFontSize(float rootFontSize, float fontSize)
		{
			m_Dirty = true;
			m_MustComputeValues = true;
			m_RootFontSize = rootFontSize;
			m_FontSize = fontSize;
		}

		void setViewport(CInterfaceElement *root)
		{
			m_Dirty = true;
			m_MustComputeValues = true;
			m_Viewport = root;
		}

		void draw();
	}; // CSSBorderRenderer

}//namespace

#endif // NL_CSS_BORDER_RENDERER_H


