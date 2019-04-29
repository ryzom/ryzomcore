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

#include <string>
#include "nel/misc/types_nl.h"
#include "nel/gui/css_style.h"
#include "nel/gui/css_parser.h"
#include "nel/gui/libwww.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	// ***************************************************************************
	void CCssStyle::reset()
	{
		_StyleStack.clear();

		Root = CStyleParams();
		Current = CStyleParams();
	}

	// ***************************************************************************
	void CCssStyle::applyRootStyle(const std::string &styleString)
	{
		getStyleParams(styleString, Root, Root);
	}

	// ***************************************************************************
	void CCssStyle::applyStyle(const std::string &styleString)
	{
		if (_StyleStack.empty())
		{
			getStyleParams(styleString, Current, Root);
		}
		else
		{
			getStyleParams(styleString, Current, _StyleStack.back());
		}
	}

	bool CCssStyle::scanCssLength(const std::string& str, uint32 &px) const
	{
		if (fromString(str, px))
			return true;

		if (str == "thin")
		{
			px = 1;
			return true;
		}
		if (str == "medium")
		{
			px = 3;
			return true;
		}
		if (str == "thick")
		{
			px = 5;
			return true;
		}

		return false;
	}

	// ***************************************************************************
	// CStyleParams style;
	// style.FontSize;    // font-size: 10px;
	// style.TextColor;   // color: #ABCDEF;
	// style.Underlined;  // text-decoration: underline;     text-decoration-line: underline;
	// style.StrikeThrough; // text-decoration: line-through;  text-decoration-line: line-through;
	void CCssStyle::getStyleParams(const std::string &styleString, CStyleParams &style, const CStyleParams &current) const
	{
		float tmpf;
		TStyle styles = CCssParser::parseDecls(styleString);
		TStyle::iterator it;

		// first pass: get font-size for 'em' sizes
		for (it=styles.begin(); it != styles.end(); ++it)
		{
			if (it->first == "font")
			{
				if (it->second == "inherit")
				{
					style.FontSize = current.FontSize;
					style.FontFamily = current.FontFamily;
					style.FontWeight = current.FontWeight;
					style.FontOblique = current.FontOblique;
				}
			}
			else
			if (it->first == "font-size")
			{
				if (it->second == "inherit")
				{
					style.FontSize = current.FontSize;
				}
				else if (it->second == "x-small")
				{
					style.FontSize = 10; // 62.5%
				}
				else if (it->second == "small")
				{
					style.FontSize = 13; // 80%;
				}
				else if (it->second == "medium")
				{
					style.FontSize = 16; // 100%;
				}
				else if (it->second == "large")
				{
					style.FontSize = 18; // 112.5%
				}
				else if (it->second == "x-large")
				{
					style.FontSize = 24; // 150%
				}
				else if (it->second == "xx-large")
				{
					style.FontSize = 32; // 200%;
				}
				else if (it->second == "smaller")
				{
					if (style.FontSize < 5)
						style.FontSize = 3;
					else
						style.FontSize -= 2;
				}
				else if (it->second == "larger")
				{
					style.FontSize += 2;
				}
				else
				{
					std::string unit;
					if (getCssLength(tmpf, unit, it->second.c_str()))
					{
						if (unit == "rem")
							style.FontSize = Root.FontSize * tmpf;
						else if (unit == "em")
							style.FontSize = current.FontSize * tmpf;
						else if (unit == "pt")
							style.FontSize = tmpf / 0.75f;
						else if (unit == "%")
							style.FontSize = current.FontSize * tmpf / 100.f;
						else
							style.FontSize = tmpf;
					}
				}
			}
		}

		// second pass: rest of style
		for (it=styles.begin(); it != styles.end(); ++it)
		{
			if (it->first == "border")
			{
				sint32 b;
				if (it->second == "none")
					style.BorderWidth = 0;
				else
				if (fromString(it->second, b))
					style.BorderWidth = b;
			}
			else
			if (it->first == "font-style")
			{
				if (it->second == "inherit")
					style.FontOblique = current.FontOblique;
				else
				if (it->second == "italic" || it->second == "oblique")
					style.FontOblique = true;
			}
			else
			if (it->first == "font-family")
			{
				if (it->second == "inherit")
					style.FontFamily = current.FontFamily;
				else
					style.FontFamily = it->second;
			}
			else
			if (it->first == "font-weight")
			{
				// https://developer.mozilla.org/en-US/docs/Web/CSS/font-weight
				uint weight = 400;
				if (it->second == "inherit")
					weight = current.FontWeight;
				else
				if (it->second == "normal")
					weight = 400;
				else
				if (it->second == "bold")
					weight = 700;
				else
				if (it->second == "lighter")
				{
					const uint lighter[] = {100, 100, 100, 100, 100, 400, 400, 700, 700};
					uint index = current.FontWeight / 100 - 1;
					clamp(index, 1u, 9u);
					weight = lighter[index-1];
				}
				else
				if (it->second == "bolder")
				{
					const uint bolder[] =  {400, 400, 400, 700, 700, 900, 900, 900, 900};
					uint index = current.FontWeight / 100 + 1;
					clamp(index, 1u, 9u);
					weight = bolder[index-1];
				}
				else
				if (fromString(it->second, weight))
				{
					weight = (weight / 100);
					clamp(weight, 1u, 9u);
					weight *= 100;
				}
				style.FontWeight = weight;
			}
			else
			if (it->first == "color")
				if (it->second == "inherit")
					style.TextColor = current.TextColor;
				else
					scanHTMLColor(it->second.c_str(), style.TextColor);
			else
			if (it->first == "text-decoration" || it->first == "text-decoration-line")
			{
				std::string prop(toLower(it->second));
				style.Underlined = (prop.find("underline") != std::string::npos);
				style.StrikeThrough = (prop.find("line-through") != std::string::npos);
			}
			else
			if (it->first == "text-stroke" || it->first == "-webkit-text-stroke")
			{
				// text-stroke: length || color
				bool success = false;
				uint px = 0;
				CRGBA color;
				std::vector<std::string> parts;
				NLMISC::splitString(it->second, " ", parts);
				if (parts.size() == 1)
				{
					success = scanCssLength(parts[0], px);
					if (!success)
						success = scanHTMLColor(parts[0].c_str(), color);
				}
				else if (parts.size() == 2)
				{
					success = scanCssLength(parts[0], px);
					if (success)
						success = scanHTMLColor(parts[1].c_str(), color);
					else
					{
						success = scanHTMLColor(parts[0].c_str(), color);
						success = success && scanCssLength(parts[1], px);
					}
				}

				// do not disable shadow if one is already set
				if (success)
				{
					style.TextShadow.Enabled = (px > 0);
					style.TextShadow.Color = color;
					style.TextShadow.X = px;
					style.TextShadow.Y = px;
					style.TextShadow.Outline = true;
				}
			}
			else
			if (it->first == "text-shadow")
			{
				if (it->second == "none")
					style.TextShadow = CStyleParams::STextShadow(false);
				else
				if (it->second == "inherit")
					style.TextShadow = current.TextShadow;
				else
				{
					// text-shadow: offset-x offset-y | blur | #color
					// text-shadow: #color | offset-x offset-y
					bool success = true;
					std::string prop(it->second);
					size_t pos;
					pos = prop.find_first_of(",\n\r");
					if (pos != std::string::npos)
						prop = prop.substr(0, pos);

					std::vector<std::string> parts;
					NLMISC::splitString(prop, " ", parts);
					switch(parts.size())
					{
						case 1:
						{
							success = scanHTMLColor(it->second.c_str(), style.TextShadow.Color);
							break;
						}
						// no case 2:
						case 3:
						{
							if (!fromString(parts[0], style.TextShadow.X))
							{
								success = scanHTMLColor(parts[0].c_str(), style.TextShadow.Color);
								success = success && fromString(parts[1], style.TextShadow.X);
								success = success && fromString(parts[2], style.TextShadow.Y);
							}
							else
							{
								success = fromString(parts[1], style.TextShadow.Y);
								success = success && scanHTMLColor(parts[2].c_str(), style.TextShadow.Color);
							}
							break;
						}
						case 4:
						{
							if (!fromString(parts[0], style.TextShadow.X))
							{
								success = scanHTMLColor(parts[0].c_str(), style.TextShadow.Color);
								success = success && fromString(parts[1], style.TextShadow.X);
								success = success && fromString(parts[2], style.TextShadow.Y);
								// ignore blur [3]
							}
							else
							{
								success = fromString(parts[0], style.TextShadow.X);
								success = success && fromString(parts[1], style.TextShadow.Y);
								// ignore blur [2]
								success = success && scanHTMLColor(parts[3].c_str(), style.TextShadow.Color);
							}
							break;
						}
						default:
						{
							// unsupported rule
							break;
						}
					}

					style.TextShadow.Enabled = success;
				}
			}
			else
			if (it->first == "width")
			{
				std::string unit;
				if (getCssLength(tmpf, unit, it->second.c_str()))
				{
					if (unit == "rem")
						style.Width = tmpf * Root.FontSize;
					else if (unit == "em")
						style.Width = tmpf * style.FontSize;
					else if (unit == "pt")
						style.FontSize = tmpf / 0.75f;
					else
						style.Width = tmpf;
				}
			}
			else
			if (it->first == "height")
			{
				std::string unit;
				if (getCssLength(tmpf, unit, it->second.c_str()))
				{
					if (unit == "rem")
						style.Height = tmpf * Root.FontSize;
					else if (unit == "em")
						style.Height = tmpf * style.FontSize;
					else if (unit == "pt")
						style.FontSize = tmpf / 0.75f;
					else
						style.Height = tmpf;
				}
			}
			else
			if (it->first == "max-width")
			{
				std::string unit;
				if (getCssLength(tmpf, unit, it->second.c_str()))
				{
					if (unit == "rem")
						style.MaxWidth = tmpf * Root.FontSize;
					else if (unit == "em")
						style.MaxWidth = tmpf * style.FontSize;
					else if (unit == "pt")
						style.FontSize = tmpf / 0.75f;
					else
						style.MaxWidth = tmpf;
				}
			}
			else
			if (it->first == "max-height")
			{
				std::string unit;
				if (getCssLength(tmpf, unit, it->second.c_str()))
				{
					if (unit == "rem")
						style.MaxHeight = tmpf * Root.FontSize;
					else if (unit == "em")
						style.MaxHeight = tmpf * style.FontSize;
					else if (unit == "pt")
						style.FontSize = tmpf / 0.75f;
					else
						style.MaxHeight = tmpf;
				}
			}
			else
			if (it->first == "-ryzom-modulate-color")
			{
				bool b;
				if (it->second == "inherit")
					style.GlobalColor = current.GlobalColor;
				else
				if (fromString(it->second, b))
					style.GlobalColor = b;
			}
			else
			if (it->first == "background-color")
			{
				if (it->second == "inherit")
					style.BackgroundColor = current.BackgroundColor;
				else
					scanHTMLColor(it->second.c_str(), style.BackgroundColor);
			}
			else
			if (it->first == "-ryzom-background-color-over")
			{
				if (it->second == "inherit")
					style.BackgroundColorOver = current.BackgroundColorOver;
				else
					scanHTMLColor(it->second.c_str(), style.BackgroundColorOver);
			}
		}

		// if outer element has underline set, then inner element cannot remove it
		if (current.Underlined)
			style.Underlined = current.Underlined;

		// if outer element has line-through set, then inner element cannot remove it
		if (current.StrikeThrough)
			style.StrikeThrough = current.StrikeThrough;
	}

	// ***************************************************************************
	void CCssStyle::applyCssMinMax(sint32 &width, sint32 &height, sint32 minw, sint32 minh, sint32 maxw, sint32 maxh) const
	{
		if (maxw <= 0) maxw = width;
		if (maxh <= 0) maxh = height;

		maxw = std::max(minw, maxw);
		maxh = std::max(minh, maxh);

		float ratio = (float) width / std::max(1, height);
		if (width > maxw)
		{
			width = maxw;
			height = std::max((sint32)(maxw /ratio), minh);
		}
		if (width < minw)
		{
			width = minw;
			height = std::min((sint32)(minw / ratio), maxh);
		}
		if (height > maxh)
		{
			width = std::max((sint32)(maxh * ratio), minw);
			height = maxh;
		}
		if (height < minh)
		{
			width = std::min((sint32)(minh * ratio), maxw);
			height = minh;
		}
		if (width > maxw && height > maxh)
		{
			if (maxw/width <= maxh/height)
			{
				width = maxw;
				height = std::max(minh, (sint32)(maxw / ratio));
			}
			else
			{
				width = std::max(minw, (sint32)(maxh * ratio));
				height = maxh;
			}
		}
		if (width < minw && height < minh)
		{
			if (minw / width <= minh / height)
			{
				width = std::min(maxw, (sint32)(minh * ratio));
				height = minh;
			}
			else
			{
				width = minw;
				height = std::min(maxh, (sint32)(minw / ratio));
			}
		}
		if (width < minw && height > maxh)
		{
			width = minw;
			height = maxh;
		}
		if (width > maxw && height < minh)
		{
			width = maxw;
			height = minh;
		}
	}

} // namespace

