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




#ifndef CL_LIB_WWW_H
#define CL_LIB_WWW_H

#include "nel/misc/rgba.h"

// forward declaration to avoid curl.h inclusion everywhere
typedef void CURL;

namespace NLGUI
{
	class CCtrlBaseButton;
	class CCtrlScroll;
	class CGroupList;

	// List of HTML elements. Does not need to be sorted
	typedef enum _HTMLElement {
		HTML_HTML,
		HTML_BODY,
		// meta
		HTML_BASE,
		HTML_HEAD,
		HTML_LINK,
		HTML_META,
		HTML_STYLE,
		HTML_TITLE,
		// content sectioning
		HTML_ADDRESS,
		HTML_ARTICLE,
		HTML_ASIDE,
		HTML_FOOTER,
		HTML_HEADER,
		HTML_H1,
		HTML_H2,
		HTML_H3,
		HTML_H4,
		HTML_H5,
		HTML_H6,
		HTML_HGROUP,
		HTML_MAIN,
		HTML_NAV,
		HTML_SECTION,
		// text content
		HTML_BLOCKQUOTE,
		HTML_DD,
		HTML_DIR,
		HTML_DIV,
		HTML_DL,
		HTML_DT,
		HTML_FIGCAPTION,
		HTML_FIGURE,
		HTML_HR,
		HTML_LI,
		HTML_OL,
		HTML_P,
		HTML_PRE,
		HTML_UL,
		// inline text
		HTML_A,
		HTML_ABBR,
		HTML_B,
		HTML_BDI,
		HTML_BDO,
		HTML_BR,
		HTML_CITE,
		HTML_CODE,
		HTML_DATA,
		HTML_DFN,
		HTML_EM,
		HTML_I,
		HTML_KBD,
		HTML_MARK,
		HTML_Q,
		HTML_RB,
		HTML_RP,
		HTML_RT,
		HTML_RTC,
		HTML_RUBY,
		HTML_S,
		HTML_SAMP,
		HTML_SMALL,
		HTML_SPAN,
		HTML_STRONG,
		HTML_SUB,
		HTML_SUP,
		HTML_TIME,
		HTML_TT,
		HTML_U,
		HTML_VAR,
		HTML_WBR,
		// image, multimedia
		HTML_AREA,
		HTML_AUDIO,
		HTML_IMG,
		HTML_MAP,
		HTML_TRACK,
		HTML_VIDEO,
		// embedded content
		HTML_APPLET,
		HTML_EMBED,
		HTML_IFRAME,
		HTML_NOEMBED,
		HTML_OBJECT,
		HTML_PARAM,
		HTML_PICTURE,
		HTML_SOURCE,
		// scripting
		HTML_CANVAS,
		HTML_NOSCRIPT,
		HTML_SCRIPT,
		// demarcating edits
		HTML_DEL,
		HTML_INS,
		// table
		HTML_CAPTION,
		HTML_COL,
		HTML_COLGROUP,
		HTML_TABLE,
		HTML_TBODY,
		HTML_TD,
		HTML_TFOOT,
		HTML_TH,
		HTML_THEAD,
		HTML_TR,
		// forms
		HTML_BUTTON,
		HTML_DATALIST,
		HTML_FIELDSET,
		HTML_FORM,
		HTML_INPUT,
		HTML_LABEL,
		HTML_LEGEND,
		HTML_METER,
		HTML_OPTGROUP,
		HTML_OPTION,
		HTML_OUTPUT,
		HTML_PROGRESS,
		HTML_SELECT,
		HTML_TEXTAREA,
		// interactive elements
		HTML_DETAILS,
		HTML_DIALOG,
		HTML_MENU,
		HTML_MENUITEM,
		HTML_SUMMARY,
		// -- tags for ryzom --
		HTML_FONT,
		HTML_LUA,
		// last entry for unknown elements
		HTML_NB_ELEMENTS
	} HTMLElement;

	// case insensitive lookup for HTMLElement enum by name
	// return HTML_NB_ELEMENTS if no match
	HTMLElement htmlElementLookup(const char *name);

	// ***************************************************************************
	// Read HTML color value from src and set dest
	// Can handle #rgb(a), #rrggbb(aa) or rgb()/rgba(), hsl(), hsla() formats
	// or color name directly
	bool scanHTMLColor(const char *src, NLMISC::CRGBA &dest);

	// ***************************************************************************
	// Read a CSS length value, return true if one of supported units '%, rem, em, px, pt'
	// On failure: 'value' and 'unit' values are undefined
	bool getCssLength (float &value, std::string &unit, const std::string &str);

	// Read a width HTML parameter. "100" or "100%". Returns true if percent (0 ~ 1) else false
	bool getPercentage (sint32 &width, float &percent, const char *str);

	// ***************************************************************************

	// Parse a HTML color
	NLMISC::CRGBA getColor (const char *color);

	// return css color in rgba() format
	std::string getRGBAString(const NLMISC::CRGBA &color);

	// ***************************************************************************

	const std::string &setCurrentDomain(const std::string &uri);
	void receiveCookies (CURL *curl, const std::string &domain, bool trusted);
	void sendCookies(CURL *curl, const std::string &domain, bool trusted);

}

#endif
