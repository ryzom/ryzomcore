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

#include "nel/gui/libwww.h"
#include "nel/gui/group_html.h"

#include <curl/curl.h>

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	struct CNameToHtmlElement
	{
		HTMLElement ID;
		const char* Name;
		CNameToHtmlElement(HTMLElement id, const char*name)
			: ID(id), Name(name) {}
	};

	// sorted list of HTML_ELEMENT enum to TAG name
	static CNameToHtmlElement htmlElementToName[] =
	{
		CNameToHtmlElement(HTML_A, "a"),
		CNameToHtmlElement(HTML_ABBR, "abbr"),
		CNameToHtmlElement(HTML_ADDRESS, "address"),
		CNameToHtmlElement(HTML_APPLET, "applet"),
		CNameToHtmlElement(HTML_AREA, "area"),
		CNameToHtmlElement(HTML_ARTICLE, "article"),
		CNameToHtmlElement(HTML_ASIDE, "aside"),
		CNameToHtmlElement(HTML_AUDIO, "audio"),
		CNameToHtmlElement(HTML_B, "b"),
		CNameToHtmlElement(HTML_BASE, "base"),
		CNameToHtmlElement(HTML_BDI, "bdi"),
		CNameToHtmlElement(HTML_BDO, "bdo"),
		CNameToHtmlElement(HTML_BLOCKQUOTE, "blockquote"),
		CNameToHtmlElement(HTML_BODY, "body"),
		CNameToHtmlElement(HTML_BR, "br"),
		CNameToHtmlElement(HTML_BUTTON, "button"),
		CNameToHtmlElement(HTML_CANVAS, "canvas"),
		CNameToHtmlElement(HTML_CAPTION, "caption"),
		CNameToHtmlElement(HTML_CITE, "cite"),
		CNameToHtmlElement(HTML_CODE, "code"),
		CNameToHtmlElement(HTML_COL, "col"),
		CNameToHtmlElement(HTML_COLGROUP, "colgroup"),
		CNameToHtmlElement(HTML_DATA, "data"),
		CNameToHtmlElement(HTML_DATALIST, "datalist"),
		CNameToHtmlElement(HTML_DD, "dd"),
		CNameToHtmlElement(HTML_DEL, "del"),
		CNameToHtmlElement(HTML_DETAILS, "details"),
		CNameToHtmlElement(HTML_DFN, "dfn"),
		CNameToHtmlElement(HTML_DIALOG, "dialog"),
		CNameToHtmlElement(HTML_DIR, "dir"),
		CNameToHtmlElement(HTML_DIV, "div"),
		CNameToHtmlElement(HTML_DL, "dl"),
		CNameToHtmlElement(HTML_DT, "dt"),
		CNameToHtmlElement(HTML_EM, "em"),
		CNameToHtmlElement(HTML_EMBED, "embed"),
		CNameToHtmlElement(HTML_FIELDSET, "fieldset"),
		CNameToHtmlElement(HTML_FIGCAPTION, "figcaption"),
		CNameToHtmlElement(HTML_FIGURE, "figure"),
		CNameToHtmlElement(HTML_FONT, "font"),
		CNameToHtmlElement(HTML_FOOTER, "footer"),
		CNameToHtmlElement(HTML_FORM, "form"),
		CNameToHtmlElement(HTML_H1, "h1"),
		CNameToHtmlElement(HTML_H2, "h2"),
		CNameToHtmlElement(HTML_H3, "h3"),
		CNameToHtmlElement(HTML_H4, "h4"),
		CNameToHtmlElement(HTML_H5, "h5"),
		CNameToHtmlElement(HTML_H6, "h6"),
		CNameToHtmlElement(HTML_HEAD, "head"),
		CNameToHtmlElement(HTML_HEADER, "header"),
		CNameToHtmlElement(HTML_HGROUP, "hgroup"),
		CNameToHtmlElement(HTML_HR, "hr"),
		CNameToHtmlElement(HTML_HTML, "html"),
		CNameToHtmlElement(HTML_I, "i"),
		CNameToHtmlElement(HTML_IFRAME, "iframe"),
		CNameToHtmlElement(HTML_IMG, "img"),
		CNameToHtmlElement(HTML_INPUT, "input"),
		CNameToHtmlElement(HTML_INS, "ins"),
		CNameToHtmlElement(HTML_KBD, "kbd"),
		CNameToHtmlElement(HTML_LABEL, "label"),
		CNameToHtmlElement(HTML_LEGEND, "legend"),
		CNameToHtmlElement(HTML_LI, "li"),
		CNameToHtmlElement(HTML_LINK, "link"),
		CNameToHtmlElement(HTML_LUA, "lua"),
		CNameToHtmlElement(HTML_MAIN, "main"),
		CNameToHtmlElement(HTML_MAP, "map"),
		CNameToHtmlElement(HTML_MARK, "mark"),
		CNameToHtmlElement(HTML_MENU, "menu"),
		CNameToHtmlElement(HTML_MENUITEM, "menuitem"),
		CNameToHtmlElement(HTML_META, "meta"),
		CNameToHtmlElement(HTML_METER, "meter"),
		CNameToHtmlElement(HTML_NAV, "nav"),
		CNameToHtmlElement(HTML_NOEMBED, "noembed"),
		CNameToHtmlElement(HTML_NOSCRIPT, "noscript"),
		CNameToHtmlElement(HTML_OBJECT, "object"),
		CNameToHtmlElement(HTML_OL, "ol"),
		CNameToHtmlElement(HTML_OPTGROUP, "optgroup"),
		CNameToHtmlElement(HTML_OPTION, "option"),
		CNameToHtmlElement(HTML_OUTPUT, "output"),
		CNameToHtmlElement(HTML_P, "p"),
		CNameToHtmlElement(HTML_PARAM, "param"),
		CNameToHtmlElement(HTML_PICTURE, "picture"),
		CNameToHtmlElement(HTML_PRE, "pre"),
		CNameToHtmlElement(HTML_PROGRESS, "progress"),
		CNameToHtmlElement(HTML_Q, "q"),
		CNameToHtmlElement(HTML_RB, "rb"),
		CNameToHtmlElement(HTML_RP, "rp"),
		CNameToHtmlElement(HTML_RT, "rt"),
		CNameToHtmlElement(HTML_RTC, "rtc"),
		CNameToHtmlElement(HTML_RUBY, "ruby"),
		CNameToHtmlElement(HTML_S, "s"),
		CNameToHtmlElement(HTML_SAMP, "samp"),
		CNameToHtmlElement(HTML_SCRIPT, "script"),
		CNameToHtmlElement(HTML_SECTION, "section"),
		CNameToHtmlElement(HTML_SELECT, "select"),
		CNameToHtmlElement(HTML_SMALL, "small"),
		CNameToHtmlElement(HTML_SOURCE, "source"),
		CNameToHtmlElement(HTML_SPAN, "span"),
		CNameToHtmlElement(HTML_STRONG, "strong"),
		CNameToHtmlElement(HTML_STYLE, "style"),
		CNameToHtmlElement(HTML_SUB, "sub"),
		CNameToHtmlElement(HTML_SUMMARY, "summary"),
		CNameToHtmlElement(HTML_SUP, "sup"),
		CNameToHtmlElement(HTML_TABLE, "table"),
		CNameToHtmlElement(HTML_TBODY, "tbody"),
		CNameToHtmlElement(HTML_TD, "td"),
		CNameToHtmlElement(HTML_TEXTAREA, "textarea"),
		CNameToHtmlElement(HTML_TFOOT, "tfoot"),
		CNameToHtmlElement(HTML_TH, "th"),
		CNameToHtmlElement(HTML_THEAD, "thead"),
		CNameToHtmlElement(HTML_TIME, "time"),
		CNameToHtmlElement(HTML_TITLE, "title"),
		CNameToHtmlElement(HTML_TR, "tr"),
		CNameToHtmlElement(HTML_TRACK, "track"),
		CNameToHtmlElement(HTML_TT, "tt"),
		CNameToHtmlElement(HTML_U, "u"),
		CNameToHtmlElement(HTML_UL, "ul"),
		CNameToHtmlElement(HTML_VAR, "var"),
		CNameToHtmlElement(HTML_VIDEO, "video"),
		CNameToHtmlElement(HTML_WBR, "wbr")
	};

	HTMLElement htmlElementLookup(const char* name)
	{
		uint end = sizeofarray(htmlElementToName);
		uint mid = end >> 1;
		sint ret;
		while(mid < end)
		{
			sint ret = nlstricmp(name, htmlElementToName[mid].Name);
			if (ret == 0)
			{
				return htmlElementToName[mid].ID;
			}
			else if (ret < 0)
			{
				// lower half
				end = mid;
				mid = end >> 1;
			}
			else
			{
				// upper half
				mid++;
				mid += (end - mid) >> 1;
			}
		}

		// not found
		return HTML_NB_ELEMENTS;
	}

	// ***************************************************************************

	/// the cookie value for session identification (nel cookie)
	std::string CurrentCookie;

	/// store all cookies we receive and resent them depending of the domain
	static std::map<std::string, std::map<std::string, std::string> > HTTPCookies;

	// ***************************************************************************

	// ***************************************************************************
	bool getCssLength (float &value, std::string &unit, const std::string &str)
	{
		std::string::size_type pos = 0;
		std::string::size_type len = str.size();
		if (len == 1 && str[0] == '.')
		{
			return false;
		}

		while(pos < len)
		{
			bool isNumeric = (str[pos] >= '0' && str[pos] <= '9')
				|| (pos == 0 && str[pos] == '.')
				|| (pos > 0 && str[pos] == '.' && str[pos-1] >= '0' && str[pos-1] <= '9');
			if (!isNumeric)
			{
				break;
			}

			pos++;
		}

		unit = toLower(str.substr(pos));
		if (unit == "%" || unit == "rem" || unit == "em" || unit == "px" || unit == "pt")
		{
			std::string tmpstr = str.substr(0, pos);
			return fromString(tmpstr, value);
		}

		return false;
	}

	// Read a width HTML parameter. "100" or "100%". Returns true if percent (0 ~ 1) else false
	bool getPercentage (sint32 &width, float &percent, const char *str)
	{
		// Percent ?
		const char *percentChar;
		if ((percentChar = strchr (str, '%')) != NULL)
		{
			std::string toto = str;
			toto = toto.substr (0, percentChar - str);
			fromString(toto, percent);
			percent /= 100.f;
			return true;
		}
		else
		{
			fromString(str, width);
			return false;
		}
	}

	static bool isHexa(char c)
	{
		return isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f');
	}

	static uint8 convertHexa(char c)
	{
		return (uint8) (tolower(c) - (isdigit(c) ? '0' : ('a' - 10)));
	}

	// scan a color component, and return pointer to next position
	static const char *scanColorComponent(const char *src, uint8 &intensity)
	{
		if (!src) return NULL;
		if (!isHexa(*src)) return NULL;
		uint8 value = convertHexa(*src++) << 4;
		if (!isHexa(*src)) return NULL;
		value += convertHexa(*src++);
		intensity = value;
		return src;
	}

	static float hueToRgb(float m1, float m2, float h)
	{
		if (h < 0) h += 1.0f;
		if (h > 1) h -= 1.0f;
		if (h*6 < 1.0f) return m1 + (m2 - m1)*h*6;
		if (h*2 < 1.0f) return m2;
		if (h*3 < 2.0f) return m1 + (m2 - m1) * (2.0f/3.0f - h)*6;
		return m1;
	}

	static void hslToRgb(float h, float s, float l, CRGBA &result)
	{
		float m1, m2;
		if (l <= 0.5f)
			m2 = l * (s + 1.0f);
		else
			m2 = l + s - l * s;
		m1 = l*2 - m2;

		result.R = 255 * hueToRgb(m1, m2, h + 1.0f/3.0f);
		result.G = 255 * hueToRgb(m1, m2, h);
		result.B = 255 * hueToRgb(m1, m2, h - 1.0f/3.0f);
		result.A = 255;
	}

	class CNameToCol
	{
	public:
		const char *Name;
		CRGBA Color;
		CNameToCol(const char *name, CRGBA color) : Name(name), Color(color) {}
	};

	static CNameToCol htmlColorNameToRGBA[] =
	{
		CNameToCol("AliceBlue", CRGBA(0xF0, 0xF8, 0xFF)),
		CNameToCol("AntiqueWhite", CRGBA(0xFA, 0xEB, 0xD7)),
		CNameToCol("Aqua", CRGBA(0x00, 0xFF, 0xFF)),
		CNameToCol("Aquamarine", CRGBA(0x7F, 0xFF, 0xD4)),
		CNameToCol("Azure", CRGBA(0xF0, 0xFF, 0xFF)),
		CNameToCol("Beige", CRGBA(0xF5, 0xF5, 0xDC)),
		CNameToCol("Bisque", CRGBA(0xFF, 0xE4, 0xC4)),
		CNameToCol("Black", CRGBA(0x00, 0x00, 0x00)),
		CNameToCol("BlanchedAlmond", CRGBA(0xFF, 0xEB, 0xCD)),
		CNameToCol("Blue", CRGBA(0x00, 0x00, 0xFF)),
		CNameToCol("BlueViolet", CRGBA(0x8A, 0x2B, 0xE2)),
		CNameToCol("Brown", CRGBA(0xA5, 0x2A, 0x2A)),
		CNameToCol("BurlyWood", CRGBA(0xDE, 0xB8, 0x87)),
		CNameToCol("CadetBlue", CRGBA(0x5F, 0x9E, 0xA0)),
		CNameToCol("Chartreuse", CRGBA(0x7F, 0xFF, 0x00)),
		CNameToCol("Chocolate", CRGBA(0xD2, 0x69, 0x1E)),
		CNameToCol("Coral", CRGBA(0xFF, 0x7F, 0x50)),
		CNameToCol("CornflowerBlue", CRGBA(0x64, 0x95, 0xED)),
		CNameToCol("Cornsilk", CRGBA(0xFF, 0xF8, 0xDC)),
		CNameToCol("Crimson", CRGBA(0xDC, 0x14, 0x3C)),
		CNameToCol("Cyan", CRGBA(0x00, 0xFF, 0xFF)),
		CNameToCol("DarkBlue", CRGBA(0x00, 0x00, 0x8B)),
		CNameToCol("DarkCyan", CRGBA(0x00, 0x8B, 0x8B)),
		CNameToCol("DarkGoldenRod", CRGBA(0xB8, 0x86, 0x0B)),
		CNameToCol("DarkGray", CRGBA(0xA9, 0xA9, 0xA9)),
		CNameToCol("DarkGreen", CRGBA(0x00, 0x64, 0x00)),
		CNameToCol("DarkKhaki", CRGBA(0xBD, 0xB7, 0x6B)),
		CNameToCol("DarkMagenta", CRGBA(0x8B, 0x00, 0x8B)),
		CNameToCol("DarkOliveGreen", CRGBA(0x55, 0x6B, 0x2F)),
		CNameToCol("Darkorange", CRGBA(0xFF, 0x8C, 0x00)),
		CNameToCol("DarkOrchid", CRGBA(0x99, 0x32, 0xCC)),
		CNameToCol("DarkRed", CRGBA(0x8B, 0x00, 0x00)),
		CNameToCol("DarkSalmon", CRGBA(0xE9, 0x96, 0x7A)),
		CNameToCol("DarkSeaGreen", CRGBA(0x8F, 0xBC, 0x8F)),
		CNameToCol("DarkSlateBlue", CRGBA(0x48, 0x3D, 0x8B)),
		CNameToCol("DarkSlateGray", CRGBA(0x2F, 0x4F, 0x4F)),
		CNameToCol("DarkTurquoise", CRGBA(0x00, 0xCE, 0xD1)),
		CNameToCol("DarkViolet", CRGBA(0x94, 0x00, 0xD3)),
		CNameToCol("DeepPink", CRGBA(0xFF, 0x14, 0x93)),
		CNameToCol("DeepSkyBlue", CRGBA(0x00, 0xBF, 0xFF)),
		CNameToCol("DimGray", CRGBA(0x69, 0x69, 0x69)),
		CNameToCol("DodgerBlue", CRGBA(0x1E, 0x90, 0xFF)),
		CNameToCol("Feldspar", CRGBA(0xD1, 0x92, 0x75)),
		CNameToCol("FireBrick", CRGBA(0xB2, 0x22, 0x22)),
		CNameToCol("FloralWhite", CRGBA(0xFF, 0xFA, 0xF0)),
		CNameToCol("ForestGreen", CRGBA(0x22, 0x8B, 0x22)),
		CNameToCol("Fuchsia", CRGBA(0xFF, 0x00, 0xFF)),
		CNameToCol("Gainsboro", CRGBA(0xDC, 0xDC, 0xDC)),
		CNameToCol("GhostWhite", CRGBA(0xF8, 0xF8, 0xFF)),
		CNameToCol("Gold", CRGBA(0xFF, 0xD7, 0x00)),
		CNameToCol("GoldenRod", CRGBA(0xDA, 0xA5, 0x20)),
		CNameToCol("Gray", CRGBA(0x80, 0x80, 0x80)),
		CNameToCol("Green", CRGBA(0x00, 0x80, 0x00)),
		CNameToCol("GreenYellow", CRGBA(0xAD, 0xFF, 0x2F)),
		CNameToCol("HoneyDew", CRGBA(0xF0, 0xFF, 0xF0)),
		CNameToCol("HotPink", CRGBA(0xFF, 0x69, 0xB4)),
		CNameToCol("IndianRed ", CRGBA(0xCD, 0x5C, 0x5C)),
		CNameToCol("Indigo  ", CRGBA(0x4B, 0x00, 0x82)),
		CNameToCol("Ivory", CRGBA(0xFF, 0xFF, 0xF0)),
		CNameToCol("Khaki", CRGBA(0xF0, 0xE6, 0x8C)),
		CNameToCol("Lavender", CRGBA(0xE6, 0xE6, 0xFA)),
		CNameToCol("LavenderBlush", CRGBA(0xFF, 0xF0, 0xF5)),
		CNameToCol("LawnGreen", CRGBA(0x7C, 0xFC, 0x00)),
		CNameToCol("LemonChiffon", CRGBA(0xFF, 0xFA, 0xCD)),
		CNameToCol("LightBlue", CRGBA(0xAD, 0xD8, 0xE6)),
		CNameToCol("LightCoral", CRGBA(0xF0, 0x80, 0x80)),
		CNameToCol("LightCyan", CRGBA(0xE0, 0xFF, 0xFF)),
		CNameToCol("LightGoldenRodYellow", CRGBA(0xFA, 0xFA, 0xD2)),
		CNameToCol("LightGrey", CRGBA(0xD3, 0xD3, 0xD3)),
		CNameToCol("LightGreen", CRGBA(0x90, 0xEE, 0x90)),
		CNameToCol("LightPink", CRGBA(0xFF, 0xB6, 0xC1)),
		CNameToCol("LightSalmon", CRGBA(0xFF, 0xA0, 0x7A)),
		CNameToCol("LightSeaGreen", CRGBA(0x20, 0xB2, 0xAA)),
		CNameToCol("LightSkyBlue", CRGBA(0x87, 0xCE, 0xFA)),
		CNameToCol("LightSlateBlue", CRGBA(0x84, 0x70, 0xFF)),
		CNameToCol("LightSlateGray", CRGBA(0x77, 0x88, 0x99)),
		CNameToCol("LightSteelBlue", CRGBA(0xB0, 0xC4, 0xDE)),
		CNameToCol("LightYellow", CRGBA(0xFF, 0xFF, 0xE0)),
		CNameToCol("Lime", CRGBA(0x00, 0xFF, 0x00)),
		CNameToCol("LimeGreen", CRGBA(0x32, 0xCD, 0x32)),
		CNameToCol("Linen", CRGBA(0xFA, 0xF0, 0xE6)),
		CNameToCol("Magenta", CRGBA(0xFF, 0x00, 0xFF)),
		CNameToCol("Maroon", CRGBA(0x80, 0x00, 0x00)),
		CNameToCol("MediumAquaMarine", CRGBA(0x66, 0xCD, 0xAA)),
		CNameToCol("MediumBlue", CRGBA(0x00, 0x00, 0xCD)),
		CNameToCol("MediumOrchid", CRGBA(0xBA, 0x55, 0xD3)),
		CNameToCol("MediumPurple", CRGBA(0x93, 0x70, 0xD8)),
		CNameToCol("MediumSeaGreen", CRGBA(0x3C, 0xB3, 0x71)),
		CNameToCol("MediumSlateBlue", CRGBA(0x7B, 0x68, 0xEE)),
		CNameToCol("MediumSpringGreen", CRGBA(0x00, 0xFA, 0x9A)),
		CNameToCol("MediumTurquoise", CRGBA(0x48, 0xD1, 0xCC)),
		CNameToCol("MediumVioletRed", CRGBA(0xC7, 0x15, 0x85)),
		CNameToCol("MidnightBlue", CRGBA(0x19, 0x19, 0x70)),
		CNameToCol("MintCream", CRGBA(0xF5, 0xFF, 0xFA)),
		CNameToCol("MistyRose", CRGBA(0xFF, 0xE4, 0xE1)),
		CNameToCol("Moccasin", CRGBA(0xFF, 0xE4, 0xB5)),
		CNameToCol("NavajoWhite", CRGBA(0xFF, 0xDE, 0xAD)),
		CNameToCol("Navy", CRGBA(0x00, 0x00, 0x80)),
		CNameToCol("OldLace", CRGBA(0xFD, 0xF5, 0xE6)),
		CNameToCol("Olive", CRGBA(0x80, 0x80, 0x00)),
		CNameToCol("OliveDrab", CRGBA(0x6B, 0x8E, 0x23)),
		CNameToCol("Orange", CRGBA(0xFF, 0xA5, 0x00)),
		CNameToCol("OrangeRed", CRGBA(0xFF, 0x45, 0x00)),
		CNameToCol("Orchid", CRGBA(0xDA, 0x70, 0xD6)),
		CNameToCol("PaleGoldenRod", CRGBA(0xEE, 0xE8, 0xAA)),
		CNameToCol("PaleGreen", CRGBA(0x98, 0xFB, 0x98)),
		CNameToCol("PaleTurquoise", CRGBA(0xAF, 0xEE, 0xEE)),
		CNameToCol("PaleVioletRed", CRGBA(0xD8, 0x70, 0x93)),
		CNameToCol("PapayaWhip", CRGBA(0xFF, 0xEF, 0xD5)),
		CNameToCol("PeachPuff", CRGBA(0xFF, 0xDA, 0xB9)),
		CNameToCol("Peru", CRGBA(0xCD, 0x85, 0x3F)),
		CNameToCol("Pink", CRGBA(0xFF, 0xC0, 0xCB)),
		CNameToCol("Plum", CRGBA(0xDD, 0xA0, 0xDD)),
		CNameToCol("PowderBlue", CRGBA(0xB0, 0xE0, 0xE6)),
		CNameToCol("Purple", CRGBA(0x80, 0x00, 0x80)),
		CNameToCol("Red", CRGBA(0xFF, 0x00, 0x00)),
		CNameToCol("RosyBrown", CRGBA(0xBC, 0x8F, 0x8F)),
		CNameToCol("RoyalBlue", CRGBA(0x41, 0x69, 0xE1)),
		CNameToCol("SaddleBrown", CRGBA(0x8B, 0x45, 0x13)),
		CNameToCol("Salmon", CRGBA(0xFA, 0x80, 0x72)),
		CNameToCol("SandyBrown", CRGBA(0xF4, 0xA4, 0x60)),
		CNameToCol("SeaGreen", CRGBA(0x2E, 0x8B, 0x57)),
		CNameToCol("SeaShell", CRGBA(0xFF, 0xF5, 0xEE)),
		CNameToCol("Sienna", CRGBA(0xA0, 0x52, 0x2D)),
		CNameToCol("Silver", CRGBA(0xC0, 0xC0, 0xC0)),
		CNameToCol("SkyBlue", CRGBA(0x87, 0xCE, 0xEB)),
		CNameToCol("SlateBlue", CRGBA(0x6A, 0x5A, 0xCD)),
		CNameToCol("SlateGray", CRGBA(0x70, 0x80, 0x90)),
		CNameToCol("Snow", CRGBA(0xFF, 0xFA, 0xFA)),
		CNameToCol("SpringGreen", CRGBA(0x00, 0xFF, 0x7F)),
		CNameToCol("SteelBlue", CRGBA(0x46, 0x82, 0xB4)),
		CNameToCol("Tan", CRGBA(0xD2, 0xB4, 0x8C)),
		CNameToCol("Teal", CRGBA(0x00, 0x80, 0x80)),
		CNameToCol("Thistle", CRGBA(0xD8, 0xBF, 0xD8)),
		CNameToCol("Tomato", CRGBA(0xFF, 0x63, 0x47)),
		CNameToCol("Turquoise", CRGBA(0x40, 0xE0, 0xD0)),
		CNameToCol("Violet", CRGBA(0xEE, 0x82, 0xEE)),
		CNameToCol("VioletRed", CRGBA(0xD0, 0x20, 0x90)),
		CNameToCol("Wheat", CRGBA(0xF5, 0xDE, 0xB3)),
		CNameToCol("White", CRGBA(0xFF, 0xFF, 0xFF)),
		CNameToCol("WhiteSmoke", CRGBA(0xF5, 0xF5, 0xF5)),
		CNameToCol("Yellow", CRGBA(0xFF, 0xFF, 0x00)),
		CNameToCol("YellowGreen", CRGBA(0x9A, 0xCD, 0x32))
	};

	// scan a color from a HTML form (#rrggbb format)
	bool scanHTMLColor(const char *src, CRGBA &dest)
	{
		if (!src || *src == '\0') return false;
		if (*src == '#')
		{
			++src;
			if (strlen(src) == 3 || strlen(src) == 4)
			{
				bool hasAlpha = (strlen(src) == 4);
				// check RGB for valid hex
				if (isHexa(src[0]) && isHexa(src[1]) && isHexa(src[2]))
				{
					// check optional A for valid hex
					if (hasAlpha && !isHexa(src[3])) return false;

					dest.R = convertHexa(src[0]);
					dest.G = convertHexa(src[1]);
					dest.B = convertHexa(src[2]);

					dest.R = dest.R << 4 | dest.R;
					dest.G = dest.G << 4 | dest.G;
					dest.B = dest.B << 4 | dest.B;

					if (hasAlpha)
					{
						dest.A = convertHexa(src[3]);
						dest.A = dest.A << 4 | dest.A;
					}
					else
						dest.A = 255;

					return true;
				}

				return false;
			}

			CRGBA result;
			src = scanColorComponent(src, result.R); if (!src) return false;
			src = scanColorComponent(src, result.G); if (!src) return false;
			src = scanColorComponent(src, result.B); if (!src) return false;
			src = scanColorComponent(src, result.A);
			if (!src)
			{
				// Alpha is optional
				result.A = 255;
			}
			dest = result;
			return true;
		}

		// TODO: CSS Colors Level 4 support
		// Whitespace syntax, aliases: rgb == rgba, hsl == hsla
		// rgb(51 170 51 / 0.4)    /* 40% opaque green */ 
		// rgb(51 170 51 / 40%)    /* 40% opaque green */ 

		if (strnicmp(src, "rgb(", 4) == 0 || strnicmp(src, "rgba(", 5) == 0)
		{
			src += 4;
			if (*src == '(') src++;

			std::vector<std::string> parts;
			NLMISC::splitString(src, ",", parts);
			if (parts.size() >= 3)
			{
				CRGBA result;
				sint tmpv;
				float tmpf;

				// R
				if (getPercentage(tmpv, tmpf, parts[0].c_str())) tmpv = 255 * tmpf;
				clamp(tmpv, 0, 255);
				result.R = tmpv;

				// G
				if (getPercentage(tmpv, tmpf, parts[1].c_str())) tmpv = 255 * tmpf;
				clamp(tmpv, 0, 255);
				result.G = tmpv;

				// B
				if (getPercentage(tmpv, tmpf, parts[2].c_str())) tmpv = 255 * tmpf;
				clamp(tmpv, 0, 255);
				result.B = tmpv;

				// A
				if (parts.size() == 4)
				{
					if (!fromString(parts[3], tmpf)) return false;
					if (parts[3].find_first_of("%") != std::string::npos)
						tmpf /= 100;

					tmpv = 255 * tmpf;
					clamp(tmpv, 0, 255);
					result.A = tmpv;
				}
				else
					result.A = 255;

				dest = result;
				return true;
			}

			return false;
		}

		if (strnicmp(src, "hsl(", 4) == 0 || strnicmp(src, "hsla(", 5) == 0)
		{
			src += 4;
			if (*src == '(') src++;

			std::vector<std::string> parts;
			NLMISC::splitString(src, ",", parts);
			if (parts.size() >= 3)
			{
				sint tmpv;
				float h, s, l;
				// hue
				if (!fromString(parts[0], tmpv)) return false;
				tmpv = ((tmpv % 360) + 360) % 360;
				h = (float) tmpv / 360.0f;

				// saturation
				if (!getPercentage(tmpv, s, parts[1].c_str())) return false;
				clamp(s, 0.0f, 1.0f);

				// lightness
				if (!getPercentage(tmpv, l, parts[2].c_str())) return false;
				clamp(l, 0.0f, 1.0f);

				CRGBA result;
				hslToRgb(h, s, l, result);

				// A
				if (parts.size() == 4)
				{
					float tmpf;
					if (!fromString(parts[3], tmpf)) return false;
					if (parts[3].find_first_of("%") != std::string::npos)
						tmpf /= 100;
					clamp(tmpf, 0.0f, 1.0f);
					result.A = 255 * tmpf;
				}

				dest = result;
				return true;
			}

			return false;
		}

		{
			// slow but should suffice for now
			for(uint k = 0; k < sizeofarray(htmlColorNameToRGBA); ++k)
			{
				if (nlstricmp(src, htmlColorNameToRGBA[k].Name) == 0)
				{
					dest = htmlColorNameToRGBA[k].Color;
					return true;
				}
			}
			return false;
		}
	}

	// ***************************************************************************

	CRGBA getColor (const char *color)
	{
		if (strlen (color) != 7 && strlen (color) != 9 )
			return CRGBA::White;
		char tmp[3] = {0,0,0};
		CRGBA dst;
		int value;
		tmp[0] = color[1];
		tmp[1] = color[2];
		sscanf (tmp, "%x", &value);
		dst.R = uint8(value);
		tmp[0] = color[3];
		tmp[1] = color[4];
		sscanf (tmp, "%x", &value);
		dst.G = uint8(value);
		tmp[0] = color[5];
		tmp[1] = color[6];
		sscanf (tmp, "%x", &value);
		dst.B = uint8(value);
		if (strlen (color) == 9)
		{
			tmp[0] = color[7];
			tmp[1] = color[8];
			sscanf (tmp, "%x", &value);
			dst.A = uint8(value);
		}
		else
		{
			// extension to html ; try to parse an additional alpha
			dst.A = 255;
		}
		return dst;
	}

	std::string getRGBAString(const CRGBA &color)
	{
		return toString("rgba(%d, %d, %d, %.1f)", color.R, color.G, color.B, color.A / 255.f);
	}

	// update HTTPCookies list
	static void receiveCookie(const char *nsformat, const std::string &domain, bool trusted)
	{
		// 0        1           2       3       4       5       6
		// domain	tailmatch	path	secure	expires	name	value
		// .app.ryzom.com	TRUE	/	 FALSE	1234	ryzomId	AAAAAAAA|BBBBBBBB|CCCCCCCC
		// #HttpOnly_app.ryzom.com	FALSE	/	FALSE	0	PHPSESSID	sess-id-value
		std::string cookie(nsformat);

		std::vector<std::string> chunks;
		splitString(cookie, "\t", chunks);
		if (chunks.size() < 6)
		{
			nlwarning("invalid cookie format '%s'", cookie.c_str());
		}

		if (chunks[0].find("#HttpOnly_") == 0)
		{
			chunks[0] = chunks[0].substr(10);
		}

		// make sure domain is lowercase
		chunks[0] = toLower(chunks[0]);

		if (chunks[0] != domain && chunks[0] != std::string("." + domain))
		{
			// cookie is for different domain
			//nlinfo("cookie for different domain ('%s')", nsformat);
			return;
		}

		if (chunks[5] == "ryzomId")
		{
			// we receive this cookie because we are telling curl about this on send
			// normally, this cookie should be set from client and not from headers
			// it's used for R2 sessions
			if (trusted && CurrentCookie != chunks[6])
			{
				CurrentCookie = chunks[6];
				nlwarning("received ryzomId cookie '%s' from trusted domain '%s'", CurrentCookie.c_str(), domain.c_str());
			}
		}
		else
		{
			uint32 expires = 0;
			fromString(chunks[4], expires);
			// expires == 0 is session cookie
			if (expires > 0)
			{
				time_t now;
				time(&now);
				if (expires < (uint32)now)
				{
					nlwarning("cookie expired, remove from list '%s'", nsformat);
					HTTPCookies[domain].erase(chunks[5]);

					return;
				}
			}

			// this overrides cookies with same name, but different paths
			//nlwarning("save domain '%s' cookie '%s' value '%s'", domain.c_str(), chunks[5].c_str(), nsformat);
			HTTPCookies[domain][chunks[5]] = nsformat;
		}
	}

	// update HTTPCookies with cookies received from curl
	void receiveCookies (CURL *curl, const std::string &domain, bool trusted)
	{
		struct curl_slist *cookies = NULL;
		if (curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies) == CURLE_OK)
		{
			struct curl_slist *nc;
			nc = cookies;
			while(nc)
			{
				//nlwarning("received cookie '%s'", nc->data);
				receiveCookie(nc->data, domain, trusted);
				nc = nc->next;
			}

			curl_slist_free_all(cookies);
		}
	}

	// add all cookies for domain to curl handle
	void sendCookies(CURL *curl, const std::string &domain, bool trusted)
	{
		// enable curl cookie engine
		curl_easy_setopt(curl, CURLOPT_COOKIELIST, "");

		if (domain.empty())
			return;

		if (trusted && !CurrentCookie.empty())
		{
			// domain	tailmatch	path	secure	expires	name	value
			// .app.ryzom.com	TRUE	/	 FALSE	1234	ryzomId	AAAAAAAA|BBBBBBBB|CCCCCCCC
			// #HttpOnly_app.ryzom.com	FALSE	/	FALSE	0	PHPSESSID	sess-id-value
			std::string cookie;
			// set tailmatch
			if (domain[0] != '.' && domain[0] != '#')
				cookie = "." + domain + "\tTRUE";
			else
				cookie = domain + "\tFALSE";
			cookie += "\t/\tFALSE\t0\tryzomId\t" + CurrentCookie;
			curl_easy_setopt(curl, CURLOPT_COOKIELIST, cookie.c_str());
			//nlwarning("domain '%s', cookie '%s'", domain.c_str(), cookie.c_str());
		}

		if(!HTTPCookies[domain].empty())
		{
			for(std::map<std::string, std::string>::iterator it = HTTPCookies[domain].begin(); it != HTTPCookies[domain].end(); it++)
			{
				curl_easy_setopt(curl, CURLOPT_COOKIELIST, it->second.c_str());
				//nlwarning("set domain '%s' cookie '%s'", domain.c_str(), it->second.c_str());
			}
		}
	}

	// ***************************************************************************
}

