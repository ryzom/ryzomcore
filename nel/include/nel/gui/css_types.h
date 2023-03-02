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

#ifndef CL_CSS_TYPES_H
#define CL_CSS_TYPES_H

#include "nel/misc/types_nl.h"

namespace NLGUI
{
	/**
	 * \brief CSS types used in GUI classes
	 * \date 2019-09-03 10:50 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	// ie. border-style
	enum CSSLineStyle {
		CSS_LINE_STYLE_NONE = 0,
		CSS_LINE_STYLE_HIDDEN,
		CSS_LINE_STYLE_DOTTED,
		CSS_LINE_STYLE_DASHED,
		CSS_LINE_STYLE_SOLID,
		CSS_LINE_STYLE_DOUBLE,
		CSS_LINE_STYLE_GROOVE,
		CSS_LINE_STYLE_RIDGE,
		CSS_LINE_STYLE_INSET,
		CSS_LINE_STYLE_OUTSET
	};

	// ie, border-width (px)
	enum CSSLineWidth {
		CSS_LINE_WIDTH_THIN = 1,
		CSS_LINE_WIDTH_MEDIUM = 3,
		CSS_LINE_WIDTH_THICK = 5
	};

	enum CSSUnitType {
		CSS_UNIT_NONE = 0,
		CSS_UNIT_EM,
		CSS_UNIT_REM,
		CSS_UNIT_PERCENT,
		CSS_UNIT_PX,
		CSS_UNIT_PT,
		CSS_UNIT_VW,
		CSS_UNIT_VH,
		CSS_UNIT_VI,
		CSS_UNIT_VB,
		CSS_UNIT_VMIN,
		CSS_UNIT_VMAX
	};

	enum CSSValueType {
		CSS_VALUE_NONE = 0,
		CSS_VALUE_REPEAT,
		CSS_VALUE_SPACE,
		CSS_VALUE_ROUND,
		CSS_VALUE_NOREPEAT,
		CSS_VALUE_FIXED,
		CSS_VALUE_LOCAL,
		CSS_VALUE_SCROLL,
		CSS_VALUE_LEFT,
		CSS_VALUE_CENTER,
		CSS_VALUE_RIGHT,
		CSS_VALUE_TOP,
		CSS_VALUE_BOTTOM,
		CSS_VALUE_BORDER_BOX,
		CSS_VALUE_PADDING_BOX,
		CSS_VALUE_CONTENT_BOX,
		CSS_VALUE_LENGTH,
		CSS_VALUE_AUTO,
		CSS_VALUE_COVER,
		CSS_VALUE_CONTAIN
	};

	template<typename T>
	struct CSSRect
	{
		T Top;
		T Right;
		T Bottom;
		T Left;
	};

}//namespace

#endif // CL_CSS_TYPES_H


