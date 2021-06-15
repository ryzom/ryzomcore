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



#ifndef NL_CSS_BORDER_RENDERER_H
#define NL_CSS_BORDER_RENDERER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/geom_ext.h"
#include "nel/gui/css_types.h"

namespace NLGUI
{
	/**
	 * \brief Border renderer for GUI classes
	 * \date 2019-09-03 10:50 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CSSBorderRenderer
	{
	private:
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
		// if true, then at least one border is set
		bool m_Border;
		bool m_BorderTop, m_BorderRight, m_BorderBottom, m_BorderLeft;

	public:
		uint32        TopWidth, RightWidth, BottomWidth, LeftWidth;
		NLMISC::CRGBA TopColor, RightColor, BottomColor, LeftColor;
		CSSLineStyle  TopStyle, RightStyle, BottomStyle, LeftStyle;

		// alpha value from parent
		uint8 CurrentAlpha;

	public:
		CSSBorderRenderer();

		void setRenderLayer(sint layer);
		void setModulateGlobalColor(bool m);

		void setRect(sint32 x, sint32 y, sint32 w, sint32 h);

		void setWidth(uint32 top, uint32 right, uint32 bottom, uint32 left);
		void setStyle(CSSLineStyle top, CSSLineStyle right, CSSLineStyle bottom, CSSLineStyle left);
		void setColor(const NLMISC::CRGBA &top, const NLMISC::CRGBA &right, const NLMISC::CRGBA &bottom, const NLMISC::CRGBA &left);

		void updateCoords();
		void invalidateCoords() { m_Dirty = m_Border = true; }
		void invalidateContent() { m_Dirty = m_Border = true; };

		uint32 getTopWidth() const;
		uint32 getRightWidth() const;
		uint32 getBottomWidth() const;
		uint32 getLeftWidth() const;

		uint32 getLeftRightWidth() const;
		uint32 getTopBottomWidth() const;

		bool hasInnerShape(CSSLineStyle style) const;

		void draw();
	}; // CSSBorderRenderer

}//namespace

#endif // NL_CSS_BORDER_RENDERER_H


