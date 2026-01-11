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

#include <string>
#include "nel/gui/libwww.h"
#include "nel/gui/css_length.h"
#include "nel/gui/css_background.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

void CSSBackground::setImage(const std::string &value)
{
	image = value;
}

void CSSBackground::setPosition(const std::string &value)
{
	std::vector<std::string> parts;
	splitString(toLowerAscii(value), " ", parts);

	if (parts.empty() || parts.size() > 4)
		return;

	switch(parts.size())
	{
		case 1:
			positionFromOne(parts);
			break;
		case 2:
			positionFromTwo(parts);
			break;
		case 3:
			positionFromThree(parts);
			break;
		case 4:
			positionFromFour(parts);
			break;
		default:
			return;
	}
}

void CSSBackground::setSize(const std::string &value)
{
	std::vector<std::string> parts;
	splitString(toLowerAscii(value), " ", parts);
	if (parts.size() > 2)
		return;

	if (parts.size() == 1 && (parts[0] == "cover" || parts[0] == "contain"))
	{
		if (parts[0] == "cover")
			size = CSS_VALUE_COVER;
		else
			size = CSS_VALUE_CONTAIN;

		width.setAuto();
		height.setAuto();
		return;
	}

	// height will default to 'auto' if not set
	if (parts.size() == 1)
		parts.push_back("auto");

	if (parts[0] == "auto" && parts[1] == "auto")
	{
		size = CSS_VALUE_AUTO;
		width.setAuto();
		height.setAuto();
		return;
	}

	CSSLength newW, newH;
	bool success = true;
	if (parts[0] == "auto")
	{
		newW.setAuto();
	}
	else
	{
		float fval;
		std::string unit;
		if (!getCssLength(fval, unit, parts[0]))
		{
			nlwarning("Failed to parse background-size[0] '%s'", parts[0].c_str());
			return;
		}
		newW.setFloatValue(fval, unit);
	}

	if (parts[1] == "auto")
	{
		newH.setAuto();
	}
	else
	{
		float fval;
		std::string unit;
		if (!getCssLength(fval, unit, parts[1]))
		{
			nlwarning("Failed to parse background-size[1] '%s'", parts[1].c_str());
			return;
		}
		newH.setFloatValue(fval, unit);
	}

	size = CSS_VALUE_LENGTH;
	width = newW;
	height = newH;
}

void CSSBackground::setRepeat(const std::string &value)
{
	std::vector<std::string> parts;
	splitString(toLowerAscii(value), " ", parts);
	if (parts.size() == 0 || parts.size() > 2)
		return;

	if (parts.size() == 1)
	{
		if (parts[0] == "repeat-x")
			parts.push_back("no-repeat");
		else if (parts[0] == "repeat-y")
			parts.insert(parts.begin(), "no-repeat");
		else //repeat, space, round, no-repeat
			parts.push_back(parts[0]);
	}


	if (parts[0] == "repeat") repeatX = CSS_VALUE_REPEAT;
	else if (parts[0] == "no-repeat") repeatX = CSS_VALUE_NOREPEAT;
	else if (parts[0] == "space") repeatX = CSS_VALUE_SPACE;
	else if (parts[0] == "round") repeatX = CSS_VALUE_ROUND;
	else repeatX = CSS_VALUE_REPEAT;

	if (parts[1] == "repeat") repeatY = CSS_VALUE_REPEAT;
	else if (parts[1] == "no-repeat") repeatY = CSS_VALUE_NOREPEAT;
	else if (parts[1] == "space") repeatY = CSS_VALUE_SPACE;
	else if (parts[1] == "round") repeatY = CSS_VALUE_ROUND;
	else repeatY = CSS_VALUE_REPEAT;
}

void CSSBackground::setOrigin(const std::string &value)
{
	if (value == "border-box") origin = CSS_VALUE_BORDER_BOX;
	else if (value == "padding-box") origin = CSS_VALUE_PADDING_BOX;
	else if (value == "content-box") origin = CSS_VALUE_CONTENT_BOX;
	else origin = CSS_VALUE_PADDING_BOX;
}

void CSSBackground::setClip(const std::string &value)
{
	if (value == "border-box") clip = CSS_VALUE_BORDER_BOX;
	else if (value == "padding-box") clip = CSS_VALUE_PADDING_BOX;
	else if (value == "content-box") clip = CSS_VALUE_CONTENT_BOX;
	//else if (value == "text") clip = CSSValueType::Text;
	else clip = CSS_VALUE_PADDING_BOX;
}

void CSSBackground::setAttachment(const std::string &value)
{
	if (value == "fixed") attachment = CSS_VALUE_FIXED;
	else if (value == "local") attachment = CSS_VALUE_LOCAL;
	else if (value == "scroll") attachment = CSS_VALUE_SCROLL;
	else attachment = CSS_VALUE_SCROLL;
}

void CSSBackground::setColor(const std::string &value)
{
	NLMISC::CRGBA tmp;
	if (scanHTMLColor(value.c_str(), tmp))
		color = tmp;
}

static bool isHorizontalKeyword(const std::string &val)
{
	return val == "left" || val == "right";
}

static bool isVerticalKeyword(const std::string &val)
{
	return val == "top" || val == "bottom";
}

void CSSBackground::positionFromOne(const std::vector<std::string> &parts)
{
	CSSValueType newH = CSS_VALUE_LEFT;
	CSSValueType newV = CSS_VALUE_TOP;
	CSSLength newX, newY;
	newX.setFloatValue(0, "%");
	newY.setFloatValue(0, "%");

	uint index = 0;
	float fval;
	std::string unit;
	if (isHorizontalKeyword(parts[index]))
	{
		newH = parts[index] == "left" ? CSS_VALUE_LEFT : CSS_VALUE_RIGHT;
		newV = CSS_VALUE_CENTER;
	}
	else if (isVerticalKeyword(parts[index]))
	{
		newH = CSS_VALUE_CENTER;
		newV = parts[index] == "top" ? CSS_VALUE_TOP : CSS_VALUE_BOTTOM;
	}
	else if (parts[index] == "center")
	{
		newH = CSS_VALUE_CENTER;
		newV = CSS_VALUE_CENTER;
	}
	else if (getCssLength(fval, unit, parts[index], true))
	{
		newX.setFloatValue(fval, unit);
		newV = CSS_VALUE_CENTER;
	}
	else
	{
		return;
	}

	xAnchor = newH;
	yAnchor = newV;
	xPosition = newX;
	yPosition = newY;
}

void CSSBackground::positionFromTwo(const std::vector<std::string> &parts)
{
	CSSValueType newH = CSS_VALUE_LEFT;
	CSSValueType newV = CSS_VALUE_TOP;
	CSSLength newX, newY;
	newX.setFloatValue(0, "%");
	newY.setFloatValue(0, "%");

	float fval;
	std::string unit;
	uint index = 0;
	bool hasCenter = false;
	bool hasX = false;
	bool hasY = false;
	for (uint index = 0; index < parts.size(); index++)
	{
		if (parts[index] == "center")
		{
			hasCenter = true;
		}
		else if (isHorizontalKeyword(parts[index]))
		{
			if (hasX) return;
			hasX = true;
			newH = parts[index] == "left" ? CSS_VALUE_LEFT : CSS_VALUE_RIGHT;
		}
		else if (isVerticalKeyword(parts[index]))
		{
			if (hasY) return;
			hasY = true;
			newV = parts[index] == "top" ? CSS_VALUE_TOP : CSS_VALUE_BOTTOM;
		}
		else if (getCssLength(fval, unit, parts[index], true))
		{
			// invalid: 'top 50%';
			if (hasY) return;
			if (!hasX)
			{
				hasX = true;
				newX.setFloatValue(fval, unit);
			}
			else
			{
				hasY = true;
				newY.setFloatValue(fval, unit);
			}
		}
		else
		{
			return;
		}
	}

	if (hasCenter)
	{
		if (!hasX)
			newH = CSS_VALUE_CENTER;
		if (!hasY)
			newV = CSS_VALUE_CENTER;
	}

	xAnchor = newH;
	yAnchor = newV;
	xPosition = newX;
	yPosition = newY;
}

void CSSBackground::positionFromThree(const std::vector<std::string> &parts)
{
	CSSValueType newH = CSS_VALUE_LEFT;
	CSSValueType newV = CSS_VALUE_TOP;
	CSSLength newX, newY;
	newX.setFloatValue(0, "%");
	newY.setFloatValue(0, "%");

	float fval;
	std::string unit;
	bool hasCenter = false;
	bool hasX = false;
	bool hasY = false;
	for(uint index = 0; index < 3; index++)
	{
		if (parts[index] == "center")
		{
			if (hasCenter) return;
			hasCenter = true;
		}
		else if (isHorizontalKeyword(parts[index]))
		{
			if (hasX) return;
			hasX = true;
			newH = parts[index] == "left" ? CSS_VALUE_LEFT : CSS_VALUE_RIGHT;
			if ((index+1) < parts.size() && getCssLength(fval, unit, parts[index+1], true))
			{
				newX.setFloatValue(fval, unit);
				index++;
			}
		}
		else if (isVerticalKeyword(parts[index]))
		{
			if (hasY) return;
			hasY = true;
			newV = parts[index] == "top" ? CSS_VALUE_TOP : CSS_VALUE_BOTTOM;
			if ((index+1) < parts.size() && getCssLength(fval, unit, parts[index+1], true))
			{
				newY.setFloatValue(fval, unit);
				index++;
			}
		}
		else
		{
			return;
		}
	}
	if (hasCenter)
	{
		if (hasX && hasY)
			return;

		if (!hasX)
			newH = CSS_VALUE_CENTER;
		else
			newV = CSS_VALUE_CENTER;
	}

	xAnchor = newH;
	yAnchor = newV;
	xPosition = newX;
	yPosition = newY;
}

void CSSBackground::positionFromFour(const std::vector<std::string> &parts)
{
	CSSValueType newH = CSS_VALUE_LEFT;
	CSSValueType newV = CSS_VALUE_TOP;
	CSSLength newX, newY;
	newX.setFloatValue(0, "%");
	newY.setFloatValue(0, "%");

	float fval;
	std::string unit;
	bool hasX = false;
	bool hasY = false;
	for(uint index = 0; index<4; index+=2)
	{
		if (parts[index] == "center")
			return;

		if (isHorizontalKeyword(parts[index]))
		{
			if (hasX) return;
			hasX = true;
			if (!getCssLength(fval, unit, parts[index+1], true)) return;
			newH = parts[index] == "left" ? CSS_VALUE_LEFT : CSS_VALUE_RIGHT;
			newX.setFloatValue(fval, unit);
		}
		else if (isVerticalKeyword(parts[index]))
		{
			if (hasY) return;
			hasY = true;
			if (!getCssLength(fval, unit, parts[index+1], true)) return;
			newV = parts[index] == "top" ? CSS_VALUE_TOP : CSS_VALUE_BOTTOM;
			newY.setFloatValue(fval, unit);
		}
		else
		{
			return;
		}
	}

	xAnchor = newH;
	yAnchor = newV;
	xPosition = newX;
	yPosition = newY;
}

} // namespace

