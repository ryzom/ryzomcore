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

namespace NLGUI
{
	typedef std::map<std::string, std::string> TStyle;

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
			Width=-1;
			Height=-1;
			MaxWidth=-1;
			MaxHeight=-1;
			BorderWidth=1;
			BackgroundColor=NLMISC::CRGBA::Black;
			BackgroundColorOver=NLMISC::CRGBA::Black;
		}
		uint FontSize;
		uint FontWeight;
		bool FontOblique;
		std::string FontFamily;
		NLMISC::CRGBA TextColor;
		STextShadow TextShadow;
		bool GlobalColor;
		bool Underlined;
		bool StrikeThrough;
		sint32 Width;
		sint32 Height;
		sint32 MaxWidth;
		sint32 MaxHeight;
		sint32 BorderWidth;
		NLMISC::CRGBA BackgroundColor;
		NLMISC::CRGBA BackgroundColorOver;
	};

	class CCssStyle {
	public:


		// 'browser' style, overwriten with '<html>'
		CStyleParams Root;

		// current element style
		CStyleParams Current;

	private:
		std::vector<CStyleParams> _StyleStack;

		// test if str is one of "thin/medium/thick" and return its pixel value 
		bool scanCssLength(const std::string& str, uint32 &px) const;

		// read style attribute
		void getStyleParams(const std::string &styleString, CStyleParams &style, const CStyleParams &current) const;

	public:
		void reset();

		inline uint getFontSizeSmaller() const
		{
			if (Current.FontSize < 5)
				return 3;
			return Current.FontSize-2;
		}

		inline void pushStyle()
		{
			_StyleStack.push_back(Current);
		}

		inline void popStyle()
		{
			if (_StyleStack.empty())
				Current = Root;
			else
			{
				Current = _StyleStack.back();
				_StyleStack.pop_back();
			}
		}

		// apply style string to this.Root
		void applyRootStyle(const std::string &styleString);

		// apply style string to this.Current
		void applyStyle(const std::string &styleString);

		void applyCssMinMax(sint32 &width, sint32 &height, sint32 minw=0, sint32 minh=0, sint32 maxw=0, sint32 maxh=0) const;

	};

}//namespace

#endif // CL_CSS_STYLE_H

