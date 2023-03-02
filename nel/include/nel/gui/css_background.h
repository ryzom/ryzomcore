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

#ifndef CL_CSS_BACKGROUND_H
#define CL_CSS_BACKGROUND_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/gui/css_types.h"
#include "nel/gui/css_length.h"

namespace NLGUI
{
	/**
	 * \brief CSS background info
	 * \date 2021-07-02 11:36 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CSSBackground
	{
	public:
		CSSBackground()
		:color(NLMISC::CRGBA::Transparent),
			repeatX(CSS_VALUE_REPEAT), repeatY(CSS_VALUE_REPEAT), attachment(CSS_VALUE_SCROLL),
			xAnchor(CSS_VALUE_LEFT), yAnchor(CSS_VALUE_TOP),
			clip(CSS_VALUE_BORDER_BOX), origin(CSS_VALUE_PADDING_BOX), size(CSS_VALUE_AUTO)
		{}

		void setImage(const std::string &value);
		void setPosition(const std::string &value);
		void setSize(const std::string &value);
		void setRepeat(const std::string &value);
		void setOrigin(const std::string &value);
		void setClip(const std::string &value);
		void setAttachment(const std::string &value);
		void setColor(const std::string &value);

	public:
		// TODO: only final layer has color
		NLMISC::CRGBA color;
		std::string image;

		CSSValueType repeatX;
		CSSValueType repeatY;
		CSSValueType attachment;

		CSSValueType xAnchor;
		CSSValueType yAnchor;
		CSSLength xPosition;
		CSSLength yPosition;

		CSSValueType clip;
		CSSValueType origin;

		CSSValueType size;
		CSSLength width;
		CSSLength height;

	private:
		void positionFromOne(const std::vector<std::string> &parts);
		void positionFromTwo(const std::vector<std::string> &parts);
		void positionFromThree(const std::vector<std::string> &parts);
		void positionFromFour(const std::vector<std::string> &parts);
	};

}//namespace

#endif // CL_CSS_BACKGROUND_H


