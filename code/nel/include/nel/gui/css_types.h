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
		CSS_LINE_STYLE_SOLID,
		CSS_LINE_STYLE_INSET,
		CSS_LINE_STYLE_OUTSET
	};

	// ie, border-width (px)
	enum CSSLineWidth {
		CSS_LINE_WIDTH_THIN = 1,
		CSS_LINE_WIDTH_MEDIUM = 3,
		CSS_LINE_WIDTH_THICK = 5
	};

}//namespace

#endif // CL_CSS_TYPES_H


