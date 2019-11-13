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

#ifndef CL_CSS_STYLE_H
#define CL_CSS_STYLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/gui/css_selector.h"
#include "nel/gui/css_types.h"

namespace NLGUI
{
	class CHtmlElement;

	typedef std::map<std::string, std::string> TStyle;
	typedef std::pair<std::string, std::string> TStylePair;
	typedef std::vector<TStylePair> TStyleVec;

	/**
	 * \brief CSS style rules
	 * \date 2019-03-15 10:50 GMT
	 * \author Meelis Mägi (Nimetu)
	 */
	class CStyleParams
	{
	public:
		struct STextShadow
		{
		public:
			STextShadow(bool enabled = false, bool outline = false, sint32 x=1, sint32 y=1, NLMISC::CRGBA color=NLMISC::CRGBA::Black)
				: Enabled(enabled), Outline(outline), X(x), Y(y), Color(color)
			{ }

			bool Enabled;
			bool Outline;
			sint32 X;
			sint32 Y;
			NLMISC::CRGBA Color;
		};
	public:
		CStyleParams () : FontFamily(""), TextColor(255,255,255,255), TextShadow()
		{
			FontSize=10;
			FontWeight=400;
			FontOblique=false;
			Underlined=false;
			StrikeThrough=false;
			GlobalColor=false;
			DisplayBlock=false;
			Width=-1;
			Height=-1;
			MaxWidth=-1;
			MaxHeight=-1;
			// border style
			BorderTopWidth = BorderRightWidth = BorderBottomWidth = BorderLeftWidth = CSS_LINE_WIDTH_MEDIUM;
			BorderTopStyle = BorderRightStyle = BorderBottomStyle = BorderLeftStyle = CSS_LINE_STYLE_NONE;
			BorderTopColor = BorderRightColor = BorderBottomColor = BorderLeftColor = NLMISC::CRGBA::Transparent;
			// background
			BackgroundColor=NLMISC::CRGBA::Black;
			BackgroundColorOver=NLMISC::CRGBA::Black;
			PaddingTop = PaddingRight = PaddingBottom = PaddingLeft = 0;
		}

		bool hasStyle(const std::string &key) const
		{
			return StyleRules.find(key) != StyleRules.end();
		}

		std::string getStyle(const std::string &key) const
		{
			TStyle::const_iterator it = StyleRules.find(key);
			return (it != StyleRules.end() ? it->second : "");
		}

	public:
		uint FontSize;
		uint FontWeight;
		bool FontOblique;
		std::string FontFamily;
		NLMISC::CRGBA TextColor;
		STextShadow TextShadow;
		bool GlobalColor;
		bool Underlined;
		bool StrikeThrough;
		bool DisplayBlock;
		sint32 Width;
		sint32 Height;
		sint32 MaxWidth;
		sint32 MaxHeight;
		uint32 BorderTopWidth, BorderRightWidth, BorderBottomWidth, BorderLeftWidth;
		CSSLineStyle BorderTopStyle, BorderRightStyle, BorderBottomStyle, BorderLeftStyle;
		NLMISC::CRGBA BorderTopColor, BorderRightColor, BorderBottomColor, BorderLeftColor;
		NLMISC::CRGBA BackgroundColor;
		NLMISC::CRGBA BackgroundColorOver;
		uint32 PaddingTop, PaddingRight, PaddingBottom, PaddingLeft;

		std::string WhiteSpace;
		std::string TextAlign;
		std::string VerticalAlign;

		TStyle StyleRules;
	};

	class CCssStyle {
	public:

		struct SStyleRule {
			std::vector<CCssSelector> Selector;
			TStyleVec Properties;

			// pseudo element like ':before'
			std::string PseudoElement;

			// returns selector specificity
			uint specificity() const;
		};

		// 'browser' style, overwriten with '<html>'
		CStyleParams Root;

		// current element style
		CStyleParams Current;

		// known style rules sorted by specificity
		std::vector<SStyleRule> _StyleRules;

	private:
		std::vector<CStyleParams> _StyleStack;

		// test if str is one of "thin/medium/thick" and return its pixel value
		bool scanCssLength(const std::string& str, uint32 &px) const;

		// split css properties string, ie '1px solid rgb(100, 100, 100)' split by ' ' returns 3 parts.
		void splitParams(const std::string &str, char sep, std::vector<std::string> &result) const;

		// read style attribute
		void getStyleParams(const std::string &styleString, CStyleParams &style, const CStyleParams &current) const;
		void getStyleParams(const TStyle &styleRules, CStyleParams &style, const CStyleParams &current) const;

		// extract from styleRules into style.StyleRules (expand shorthand, normalize, calculate current font-size)
		void normalize(const TStyle &styleRules, CStyleParams &style, const CStyleParams &current) const;

		// apply style.StyleRyles
		void apply(CStyleParams &style, const CStyleParams &current) const;

		// merge src into dest by overwriting key in dest
		void merge(TStyle &dst, const TStyleVec &src) const;

		// match selector to dom path
		bool match(const std::vector<CCssSelector> &selector, const CHtmlElement &elm) const;

		// get shorthang 'top right bottom left' index values based size, ie 'padding' syntax
		bool getShorthandIndices(const uint32 size, uint8 &t, uint8 &r, uint8 &b, uint8 &l) const;

		// break 'border' into 'border-top-color', 'border-top-style', etc rules
		bool tryBorderWidthShorthand(const std::string &prop, const std::string &value, TStyle &style) const;
		bool tryBorderStyleShorthand(const std::string &prop, const std::string &value, TStyle &style) const;
		bool tryBorderColorShorthand(const std::string &prop, const std::string &value, TStyle &style) const;
		void expandBorderShorthand(const std::string &prop, const std::string &value, TStyle &style) const;

		// parse 'background' into 'background-color', 'background-image', etc
		void expandBackgroundShorthand(const std::string &value, TStyle &style) const;

		// parse 'padding' into 'padding-top', 'padding-left', etc
		void expandPaddingShorthand(const std::string &value, TStyle &style) const;

		// expand shorthand rule, ie "border", into longhand names, ie "border-top-width"
		// if shorthand is present in style, then its removed
		void expandShorthand(const std::string &prop, const std::string &value, TStyle &style) const;

		// parse string value into corresponding value
		void applyBorderWidth(const std::string &value, uint32 *dest, const uint32 currentWidth, const uint32 fontSize) const;
		void applyBorderColor(const std::string &value, NLMISC::CRGBA *dest, const NLMISC::CRGBA &currentColor, const NLMISC::CRGBA &textColor) const;
		void applyLineStyle(const std::string &value, CSSLineStyle *dest, const CSSLineStyle &currentStyle) const;
		void applyPaddingWidth(const std::string &value, uint32 *dest, const uint32 currentPadding, uint32 fontSize) const;

	public:
		void reset();

		// parse <style>..</style> tag or css file content
		void parseStylesheet(const std::string &styleString);

		// set element style from matching css rules
		void getStyleFor(CHtmlElement &elm) const;

		inline uint getFontSizeSmaller() const
		{
			if (Current.FontSize < 5)
				return 3;
			return Current.FontSize-2;
		}

		inline void pushStyle()
		{
			_StyleStack.push_back(Current);

			Current.GlobalColor = false;
			Current.DisplayBlock = false;
			Current.Width=-1;
			Current.Height=-1;
			Current.MaxWidth=-1;
			Current.MaxHeight=-1;

			Current.BorderTopWidth = Current.BorderRightWidth = Current.BorderBottomWidth = Current.BorderLeftWidth = CSS_LINE_WIDTH_MEDIUM;
			Current.BorderTopStyle = Current.BorderRightStyle = Current.BorderBottomStyle = Current.BorderLeftStyle = CSS_LINE_STYLE_NONE;
			Current.BorderTopColor = Current.BorderRightColor = Current.BorderBottomColor = Current.BorderLeftColor = Current.TextColor;
			Current.PaddingTop = Current.PaddingRight = Current.PaddingBottom = Current.PaddingLeft = 0;

			Current.StyleRules.clear();
		}

		inline void popStyle()
		{
			if (_StyleStack.empty())
			{
				Current = Root;
			}
			else
			{
				Current = _StyleStack.back();
				_StyleStack.pop_back();
			}
		}

		// apply style to this.Root
		void applyRootStyle(const std::string &styleString);
		void applyRootStyle(const TStyle &styleRules);

		// apply style to this.Current
		void applyStyle(const std::string &styleString);
		void applyStyle(const TStyle &styleRules);

		void applyCssMinMax(sint32 &width, sint32 &height, sint32 minw=0, sint32 minh=0, sint32 maxw=0, sint32 maxh=0) const;

		// check if current style property matches value
		bool checkStyle(const std::string &key, const std::string &val) const
		{
			return Current.hasStyle(key) && Current.getStyle(key) == val;
		}

		bool hasStyle(const std::string &key) const
		{
			return Current.hasStyle(key);
		}

		std::string getStyle(const std::string &key) const
		{
			return Current.getStyle(key);
		}
	};
}//namespace

#endif // CL_CSS_STYLE_H

