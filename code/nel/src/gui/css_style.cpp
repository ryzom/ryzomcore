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
#include "nel/gui/html_element.h"
#include "nel/gui/css_style.h"
#include "nel/gui/css_parser.h"
#include "nel/gui/libwww.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	uint CCssStyle::SStyleRule::specificity() const
	{
		uint count = 0;
		for(uint i = 0; i < Selector.size(); ++i)
		{
			count += Selector[i].specificity();
		}
		// counted as element tag like DIV
		if (!PseudoElement.empty())
		{
			count += 0x000001;
		}

		return count;
	}

	// ***************************************************************************
	void CCssStyle::reset()
	{
		_StyleRules.clear();
		_StyleStack.clear();

		Root = CStyleParams();
		Current = CStyleParams();
	}

	// ***************************************************************************
	// Sorting helper
	struct CCssSpecificityPred
	{
		bool operator()(CCssStyle::SStyleRule lhs, CCssStyle::SStyleRule rhs) const
		{
			return lhs.specificity() < rhs.specificity();
		}
	};

	// ***************************************************************************
	void CCssStyle::parseStylesheet(const std::string &styleString)
	{
		CCssParser parser;
		parser.parseStylesheet(styleString, _StyleRules);

		// keep the list sorted
		std::stable_sort(_StyleRules.begin(), _StyleRules.end(), CCssSpecificityPred());
	}

	void CCssStyle::getStyleFor(CHtmlElement &elm) const
	{
		std::vector<SStyleRule> mRules;
		for (std::vector<SStyleRule>::const_iterator it = _StyleRules.begin(); it != _StyleRules.end(); ++it)
		{
			if (match(it->Selector, elm))
			{
				mRules.push_back(*it);
			}
		}

		elm.Style.clear();
		elm.clearPseudo();

		if (!mRules.empty())
		{
			// style is sorted by specificity (lowest first), eg. html, .class, html.class, #id, html#id.class
			for(std::vector<SStyleRule>::const_iterator i = mRules.begin(); i != mRules.end(); ++i)
			{
				if (i->PseudoElement.empty())
				{
					merge(elm.Style, i->Properties);
				}
				else
				{
					elm.setPseudo(i->PseudoElement, i->Properties);
				}
			}
		}

		// style from "style" attribute overrides <style>
		if (elm.hasNonEmptyAttribute("style"))
		{
			TStyle styles = CCssParser::parseDecls(elm.getAttribute("style"));
			merge(elm.Style, styles);
		}
	}

	void CCssStyle::merge(TStyle &dst, const TStyle &src) const
	{
		// TODO: does not use '!important' flag
		for(TStyle::const_iterator it = src.begin(); it != src.end(); ++it)
		{
			dst[it->first] = it->second;
		}
	}

	bool CCssStyle::match(const std::vector<CCssSelector> &selector, const CHtmlElement &elm) const
	{
		if (selector.empty()) return false;

		// first selector, '>' immediate parent
		bool matches = false;
		bool mustMatchNext = true;
		bool matchGeneralChild = false;
		bool matchGeneralSibling = false;

		const CHtmlElement *child;
		child = &elm;
		std::vector<CCssSelector>::const_reverse_iterator ritSelector = selector.rbegin();
		char matchCombinator = '\0';
		while(ritSelector != selector.rend())
		{
			if (!child)
			{
				return false;
			}

			matches = ritSelector->match(*child);
			if (!matches && mustMatchNext)
			{
				return false;
			}

			if (!matches)
			{
				if (matchCombinator == ' ')
				{
					if (!child->parent)
					{
						return false;
					}
					child = child->parent;
					// walk up the tree until there is match for current selector
					continue;
				}

				if (matchCombinator == '~')
				{
					// any previous sibling must match current selector
					if (!child->previousSibling)
					{
						return false;
					}
					child = child->previousSibling;

					// check siblings until there is match for current selector
					continue;
				}
			}

			mustMatchNext = false;
			switch(ritSelector->Combinator)
			{
			case '\0':
				// default case when single selector
				break;
			case ' ':
				{
					// general child - match child->parent to current/previous selector
					if (!child->parent)
					{
						return false;
					}
					child = child->parent;
					matchCombinator = ritSelector->Combinator;
				}
				break;
			case '~':
				{
					// any previous sibling must match current selector
					if (!child->previousSibling)
					{
						return false;
					}
					child = child->previousSibling;
					matchCombinator = ritSelector->Combinator;
				}
				break;
			case '+':
				{
					// adjacent sibling - previous sibling must match previous selector
					if (!child->previousSibling)
					{
						return false;
					}
					child = child->previousSibling;
					mustMatchNext = true;
				}
				break;
			case '>':
				{
					// child of - immediate parent must match previous selector
					if (!child->parent)
					{
						return false;
					}
					child = child->parent;
					mustMatchNext = true;
				}
				break;
			default:
				// should not reach
				return false;
			}

			++ritSelector;
		}

		return matches;
	}

	// ***************************************************************************
	void CCssStyle::applyRootStyle(const std::string &styleString)
	{
		getStyleParams(styleString, Root, Root);
	}

	// ***************************************************************************
	void CCssStyle::applyRootStyle(const TStyle &styleRules)
	{
		getStyleParams(styleRules, Root, Root);
	}

	// ***************************************************************************
	void CCssStyle::applyStyle(const std::string &styleString)
	{
		if (styleString.empty()) return;

		if (_StyleStack.empty())
		{
			getStyleParams(styleString, Current, Root);
		}
		else
		{
			getStyleParams(styleString, Current, _StyleStack.back());
		}
	}

	// ***************************************************************************
	void CCssStyle::applyStyle(const TStyle &styleRules)
	{
		if (_StyleStack.empty())
		{
			getStyleParams(styleRules, Current, Root);
		}
		else
		{
			getStyleParams(styleRules, Current, _StyleStack.back());
		}
	}

	// ***************************************************************************
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
		TStyle styles = CCssParser::parseDecls(styleString);

		getStyleParams(styles, style, current);
	}

	void CCssStyle::getStyleParams(const TStyle &styleRules, CStyleParams &style, const CStyleParams &current) const
	{
		float tmpf;
		TStyle::const_iterator it;

		if(styleRules.empty())
		{
			return;
		}

		normalize(styleRules, style, current);
		apply(style, current);
	}

	// first pass
	// - get font-size for 'em' sizes
	// - split shorthand to its parts
	// - get TextColor value that could be used for 'currentcolor'
	// - normalize values
	void CCssStyle::normalize(const TStyle &styleRules, CStyleParams &style, const CStyleParams &current) const
	{
		TStyle::const_iterator it;
		for (it=styleRules.begin(); it != styleRules.end(); ++it)
		{
			// update local copy of applied style
			style.StyleRules[it->first] = it->second;

			if (it->first == "color")
			{
				if (it->second == "inherit")
				{
					style.TextColor = current.TextColor;
				}
				else
				{
					scanHTMLColor(it->second.c_str(), style.TextColor);
				}
			}
			else
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
					float tmpf;
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
			else
			if (it->first == "background")
			{
				parseBackgroundShorthand(it->second, style);
			}
			else
			if (it->first == "background-repeat")
			{
				// old ryzom specific value
				if (it->second == "1")
					style.StyleRules[it->first] = "repeat";
			}
			else
			if (it->first == "background-scale")
			{
				// replace old ryzom specific rule with background-size
				if (it->second != "1")
				{
					style.StyleRules["background-size"] = "auto";
				}
				else
				{
					style.StyleRules["background-size"] = "100%";
				}

				TStyle::iterator pos = style.StyleRules.find(it->first);
				if (pos != style.StyleRules.end())
					style.StyleRules.erase(pos);
			}
			else
			if (it->first == "display")
			{
				if (it->second == "inherit")
					style.DisplayBlock = current.DisplayBlock;
				else
					style.DisplayBlock = (it->second == "block" || it->second == "table");
			}
		}
	}

	// apply style rules
	void CCssStyle::apply(CStyleParams &style, const CStyleParams &current) const
	{
		float tmpf;
		TStyle::const_iterator it;
		for (it=style.StyleRules.begin(); it != style.StyleRules.end(); ++it)
		{
			if (it->first == "border" || it->first == "border-width")
			{
				// TODO: border: 1px solid red;
				if (it->second == "inherit")
				{
					style.BorderWidth = current.BorderWidth;
				}
				else if (it->second == "none")
				{
					style.BorderWidth = 0;
				}
				else if (it->second == "thin")
				{
					style.BorderWidth = 1;
				}
				else if (it->second == "medium")
				{
					style.BorderWidth = 3;
				}
				else if (it->second == "thick")
				{
					style.BorderWidth = 5;
				}
				else
				{
					float tmpf;
					std::string unit;
					if (getCssLength(tmpf, unit, it->second.c_str()))
					{
						if (unit == "rem")
							style.BorderWidth = Root.FontSize * tmpf;
						else if (unit == "em")
							style.BorderWidth = current.FontSize * tmpf;
						else if (unit == "pt")
							style.BorderWidth = tmpf / 0.75f;
						else if (unit == "%")
							style.BorderWidth = 0; // no support for % in border width
						else
							style.BorderWidth = tmpf;
					}
				}
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
			if (it->first == "text-align")
			{
				if (it->second == "inherit")
					style.TextAlign = current.TextAlign;
				else if (it->second == "left" || it->second == "right" || it->second == "center" || it->second == "justify")
					style.TextAlign = it->second;
			}
			else
			if (it->first == "vertical-align")
			{
				if (it->second == "inherit")
					style.VerticalAlign = current.VerticalAlign;
				else if (it->second == "top" || it->second == "middle" || it->second == "bottom")
					style.VerticalAlign = it->second;
			}
			else
			if (it->first == "white-space")
			{
				if (it->second == "inherit")
					style.WhiteSpace = current.WhiteSpace;
				else if (it->second == "normal" || it->second == "nowrap" || it->second == "pre")
					style.WhiteSpace = it->second;
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
					else if (unit == "%")
						style.Width = 0; // TODO: style.WidthRatio
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
					else if (unit == "%")
						style.Height = 0; // TODO: style.HeightRatio
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
					else if (unit == "%")
						style.MaxWidth = 0; // TODO: style.MaxWidthRatio
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
					else if (unit == "%")
						style.MaxHeight = 0; // TODO: style.MaxHeightRatio
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
				else if (it->second == "transparent")
					style.BackgroundColor = CRGBA(0, 0, 0, 0);
				else if (it->second == "currentcolor")
					style.BackgroundColorOver = style.TextColor;
				else
					scanHTMLColor(it->second.c_str(), style.BackgroundColor);
			}
			else
			if (it->first == "-ryzom-background-color-over")
			{
				if (it->second == "inherit")
					style.BackgroundColorOver = current.BackgroundColorOver;
				else if (it->second == "transparent")
					style.BackgroundColorOver = CRGBA(0, 0, 0, 0);
				else if (it->second == "currentcolor")
					style.BackgroundColorOver = style.TextColor;
				else
					scanHTMLColor(it->second.c_str(), style.BackgroundColorOver);
			}
			else
			if (it->first == "background-image")
			{
				// normalize
				std::string image = trim(it->second);
				if (toLower(image.substr(0, 4)) == "url(")
				{
					image = image.substr(4, image.size()-5);
				}
				style.StyleRules[it->first] = trimQuotes(image);
			}
			else
			if (it->first == "background-repeat")
			{
				// normalize
				std::string val = toLower(trim(it->second));
				std::vector<std::string> parts;
				NLMISC::splitString(val, " ", parts);
				// check for "repeat repeat"
				if (parts.size() == 2 && parts[0] == parts[1])
					val = parts[0];

				style.StyleRules[it->first] = val;
			}
			else
			if (it->first == "background-size")
			{
				// normalize
				std::string val = toLower(trim(it->second));
				std::vector<std::string> parts;
				NLMISC::splitString(val, " ", parts);
				if (parts.size() == 2 && parts[0] == parts[1])
					val = parts[0];

				style.StyleRules[it->first] = val;
			}
		}

		// if outer element has underline set, then inner element cannot remove it
		if (current.Underlined)
			style.Underlined = current.Underlined;

		// if outer element has line-through set, then inner element cannot remove it
		if (current.StrikeThrough)
			style.StrikeThrough = current.StrikeThrough;
	}

	void CCssStyle::parseBackgroundShorthand(const std::string &value, CStyleParams &style) const
	{
		// background: url(image.jpg) top center / 200px 200px no-repeat fixed padding-box content-box red;
		// background-image      : url(image.jpg)
		// background-position   : top center
		// background-size       : 200px 200px
		// background-repeat     : no-repeat
		// background-attachment : fixed
		// background-origin     : padding-box
		// background-clip       : content-box
		// background-color      : red

		const uint nbProps = 8;
		std::string props[nbProps] = {"background-image", "background-position", "background-size", "background-repeat",
			"background-attachment", "background-origin", "background-clip", "background-color"};
		std::string values[nbProps];
		bool found[nbProps] = {false};
		bool bgClipFound = false;
		std::string bgClipValue;
		std::string bgPositionX;
		std::string bgPositionY;

		uint partIndex = 0;
		std::vector<std::string> parts;
		std::vector<std::string>::iterator it;
		// FIXME: this will fail if url() contains ' ' chars
		// FIXME: this will also fail on 'background: rgb(255, 0, 0)'
		NLMISC::splitString(value, " ", parts);

		bool failed = false;
		bool allowSize = false;
		uint index = 0;
		while(!failed && index < parts.size())
		{
			std::string val = toLower(parts[index]);
			bool matches = false;
			for(uint i = 0; i < nbProps; i++)
			{
				if (found[i]) continue;

				if (props[i] == "background-image")
				{
					if (val.substr(0, 4) == "url(")
					{
						matches = true;
						found[i] = true;
						// use original value as 'val' is lowercase
						values[i] = parts[index];
					}
				}
				else if (props[i] == "background-position")
				{
					uint next = index;
					bool loop = false;
					do
					{
						float fval;
						std::string unit;

						// first loop -> true
						// second loop -> false && break
						loop = !loop;

						val = toLower(parts[next]);
						if (val == "center")
						{
							if (bgPositionX.empty()) bgPositionX = "center";
							if (bgPositionY.empty()) bgPositionY = "center";
							// consume 'center'
							next++;
						}
						else if (val == "left" || val == "right")
						{
							bgPositionX = val;
							// consume 'left|right'
							next++;
							if(next < parts.size() && getCssLength(fval, unit, parts[next]))
							{
								bgPositionX += " " + toString("%.0f%s", fval, unit.c_str());
								// consume css length
								next++;
							}
						}
						else if (val == "top" || val == "bottom")
						{
							bgPositionY = val;
							// consume top|bottom
							next++;
							if (next < parts.size() && getCssLength(fval, unit, parts[next]))
							{
								bgPositionY += " " + toString("%.0f%s", fval, unit.c_str());
								// consume css length
								next++;
							}
						}
					} while (loop);

					//
					if (!bgPositionX.empty() && !bgPositionY.empty())
					{
						matches = true;
						found[i] = true;
						// consume position values if there were any
						index = next-1;

						// look ahead to see if size is next
						if (next < parts.size() && parts[next] == "/")
							allowSize = true;
					}
				}
				else if (props[i] == "background-size")
				{
					if (allowSize && val == "/")
					{
						uint next = index + 1;
						if (next < parts.size())
						{
							val = toLower(parts[next]);
							if (val == "cover" || val == "contain")
							{
								matches = true;
								found[i] = true;
								values[i] = val;
								index = next;
							}
							else
							{
								float fval;
								std::string unit;
								std::string h, v;

								if (val == "auto" || getCssLength(fval, unit, val))
								{
									if (val == "auto")
										h = v = "auto";
									else
										h = v = toString("%.0f%s", fval, unit.c_str());

									next++;
									if (next < parts.size())
									{
										val = toLower(parts[next]);
										if (val == "auto")
											v = "auto";
										else if (getCssLength(fval, unit, val))
											v = toString("%.0f%s", fval, unit.c_str());
										else
											next--; // not size token
									}
									else
									{
										// not size token
										next--;
									}
								}

								if (!h.empty() && !v.empty())
								{
									matches = true;
									found[i] = true;
									values[i] = h + " " + v;
									index = next;
								}
							}
						}
						else
						{
							// no size, just '/'
							failed = true;
							break;
						}
					}
				}
				else if (props[i] == "background-repeat")
				{
					if (val == "repeat-x" || val == "repeat-y" || val == "repeat" || val == "space" || val == "round" || val == "no-repeat")
					{
						matches = true;
						found[i] = true;

						if (val == "repeat-x")
						{
							values[i] = "repeat no-repeat";
						}
						else if (val == "repeat-y")
						{
							values[i] = "no-repeat repeat";
						}
						else
						{
							std::string horiz = val;
							std::string vert = val;
							uint next = index + 1;
							if (next < parts.size())
							{
								val = toLower(parts[next]);
								if (val == "repeat" || val == "space" || val == "round" || val == "no-repeat")
								{
									vert = val;
									index = next;
								}
							}
							if (vert == horiz)
								values[i] = vert;
							else
								values[i] = horiz + " " + vert;
						}
					}
				}
				else if (props[i] == "background-attachment")
				{
					if (val == "scroll" || val == "fixed" || val == "local")
					{
						matches = true;
						found[i] = true;
						values[i] = val;
					}
				}
				else if (props[i] == "background-origin")
				{
					if (val == "padding-box" || val == "border-box" || val == "content-box")
					{
						matches = true;
						found[i] = true;
						values[i] = val;

						// first time background-origin is set, also set background-clip
						if (!bgClipFound)
							bgClipValue = val;
					}
				}
				else if (props[i] == "background-clip")
				{
					if (val == "text" || val == "padding-box" || val == "border-box" || val == "content-box")
					{
						matches = true;
						found[i] = true;
						bgClipFound = true;
						bgClipValue = val;
					}
				}
				else if (props[i] == "background-color")
				{
					CRGBA color;
					if (val == "transparent" || val == "currentcolor" || scanHTMLColor(val.c_str(), color))
					{
						matches = true;
						found[i] = true;
						values[i] = val;
					}
				}

				// prop was found and parsed
				if (found[i])
					break;
			}
			failed = !matches;

			index++;
		}

		// invalidate whole rule
		if (failed)
		{
			bgClipFound = false;
			for(uint i = 0; i < nbProps; i++)
			{
				found[i] = false;
			}
		}

		// apply found styles or use default
		for(uint i = 0; i < nbProps; i++)
		{
			if (found[i])
			{
				if (props[i] == "background-position")
				{
					style.StyleRules["background-position-x"] = bgPositionX;
					style.StyleRules["background-position-y"] = bgPositionY;
				}
				else if (props[i] == "background-clip")
				{
					style.StyleRules["background-clip"] = bgClipValue;
				}
				else
				{
					style.StyleRules[props[i]] = values[i];
				}
			}
			else
			{
				// fill in default if one is set
				if (props[i] == "background-image")
				{
					style.StyleRules[props[i]] = "none";
				}
				else if (props[i] == "background-position")
				{
					style.StyleRules[props[i]] = "0% 0%";
					style.StyleRules["background-position-x"] = "left 0%";
					style.StyleRules["background-position-y"] = "top 0%";
				}
				else if (props[i] == "background-size")
				{
					style.StyleRules[props[i]] = "auto auto";
				}
				else if (props[i] == "background-repeat")
				{
					style.StyleRules[props[i]] = "repeat";
				}
				else if(props[i] == "background-attachment")
				{
					style.StyleRules[props[i]] = "scroll";
				}
				else if(props[i] == "background-origin")
				{
					style.StyleRules[props[i]] = "padding-box";
				}
				else if (props[i] == "background-clip")
				{
					if (bgClipFound)
						style.StyleRules[props[i]] = bgClipValue;
					else
						style.StyleRules[props[i]] = "border-box";
				}
				else if (props[i] == "background-color")
				{
					style.StyleRules[props[i]] = "transparent";
				}
			}
		}
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

