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

#ifndef CL_CSS_BORDER_H
#define CL_CSS_BORDER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/gui/css_types.h"
#include "nel/gui/css_length.h"

namespace NLGUI
{
	/**
	 * \brief CSS border info
	 * \date 2021-07-23 09:51 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CSSBorder
	{
	public:
		CSSBorder()
		{
			reset();
		}

		CSSBorder(uint width, CSSLineStyle style, NLMISC::CRGBA color)
		{
			set(width, style, color);
		}

		void reset()
		{
			set(CSS_LINE_WIDTH_MEDIUM, CSS_LINE_STYLE_NONE, NLMISC::CRGBA::Transparent);
		}

		void set(uint width, CSSLineStyle style, NLMISC::CRGBA color)
		{
			Width.setFloatValue(width, "px");
			Style = style;
			Color = color;
		}

		bool empty() const
		{
			return Style == CSS_LINE_STYLE_NONE || Style == CSS_LINE_STYLE_HIDDEN
				|| Width.getFloat() == 0;
		}

		CSSLength Width;
		CSSLineStyle Style;
		NLMISC::CRGBA Color;
	};

}//namespace

#endif // CL_CSS_BORDER_H


