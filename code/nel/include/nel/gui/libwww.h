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

extern "C"
{
#include "WWWInit.h"
}

#include "nel/misc/rgba.h"

namespace NLGUI
{
	class CCtrlBaseButton;
	class CCtrlScroll;
	class CGroupList;

	// ***************************************************************************

	// Init the libwww
	void initLibWWW();

	// Get an url and setup a local domain
	const std::string &setCurrentDomain(const std::string &url);

	extern std::string CurrentCookie;

	// ***************************************************************************

	// Some DTD table

	// Here, modify the DTD table to change the HTML parser (add new tags for exemples)

	#undef HTML_ATTR
	#define HTML_ATTR(t,a) MY_HTML_##t##_##a

	enum
	{
		HTML_ATTR(A,ACCESSKEY) = 0,
			HTML_ATTR(A,CHARSET),
			HTML_ATTR(A,CLASS),
			HTML_ATTR(A,COORDS),
			HTML_ATTR(A,DIR),
			HTML_ATTR(A,HREF),
			HTML_ATTR(A,HREFLANG),
			HTML_ATTR(A,ID),
			HTML_ATTR(A,NAME),
			HTML_ATTR(A,REL),
			HTML_ATTR(A,REV),
			HTML_ATTR(A,SHAPE),
			HTML_ATTR(A,STYLE),
			HTML_ATTR(A,TABINDEX),
			HTML_ATTR(A,TARGET),
			HTML_ATTR(A,TYPE),
			HTML_ATTR(A,TITLE),
			HTML_ATTR(A,Z_ACTION_CATEGORY),
			HTML_ATTR(A,Z_ACTION_PARAMS),
			HTML_ATTR(A,Z_ACTION_SHORTCUT),
	};

	enum
	{
		HTML_ATTR(TABLE,ALIGN) = 0,
			HTML_ATTR(TABLE,BGCOLOR),
			HTML_ATTR(TABLE,BORDER),
			HTML_ATTR(TABLE,BORDERCOLOR),
			HTML_ATTR(TABLE,CELLPADDING),
			HTML_ATTR(TABLE,CELLSPACING),
			HTML_ATTR(TABLE,CLASS),
			HTML_ATTR(TABLE,DIR),
			HTML_ATTR(TABLE,FRAME),
			HTML_ATTR(TABLE,ID),
			HTML_ATTR(TABLE,L_MARGIN),
			HTML_ATTR(TABLE,LANG),
			HTML_ATTR(TABLE,NOWRAP),
			HTML_ATTR(TABLE,RULES),
			HTML_ATTR(TABLE,SUMMARY),
			HTML_ATTR(TABLE,STYLE),
			HTML_ATTR(TABLE,TITLE),
			HTML_ATTR(TABLE,VALIGN),
			HTML_ATTR(TABLE,WIDTH)
	};

	enum
	{
		HTML_ATTR(TR,ALIGN) = 0,
			HTML_ATTR(TR,BGCOLOR),
			HTML_ATTR(TR,L_MARGIN),
			HTML_ATTR(TR,NOWRAP),
			HTML_ATTR(TR,VALIGN),
	};

	enum
	{
		HTML_ATTR(TD,ABBR) = 0,
			HTML_ATTR(TD,ALIGN),
			HTML_ATTR(TD,AXIS),
			HTML_ATTR(TD,BGCOLOR),
			HTML_ATTR(TD,CHAR),
			HTML_ATTR(TD,CHAROFF),
			HTML_ATTR(TD,CLASS),
			HTML_ATTR(TD,COLSPAN),
			HTML_ATTR(TD,DIR),
			HTML_ATTR(TD,ID),
			HTML_ATTR(TD,HEADERS),
			HTML_ATTR(TD,HEIGHT),
			HTML_ATTR(TD,L_MARGIN),
			HTML_ATTR(TD,LANG),
			HTML_ATTR(TD,NOWRAP),
			HTML_ATTR(TD,ROWSPAN),
			HTML_ATTR(TD,SCOPE),
			HTML_ATTR(TD,STYLE),
			HTML_ATTR(TD,TITLE),
			HTML_ATTR(TD,VALIGN),
			HTML_ATTR(TD,WIDTH),
	};

	enum
	{
		HTML_ATTR(IMG,ALIGN) = 0,
			HTML_ATTR(IMG,ALT),
			HTML_ATTR(IMG,BORDER),
			HTML_ATTR(IMG,CLASS),
			HTML_ATTR(IMG,DIR),
			HTML_ATTR(IMG,GLOBAL_COLOR),
			HTML_ATTR(IMG,HEIGHT),
			HTML_ATTR(IMG,HSPACE),
			HTML_ATTR(IMG,ID),
			HTML_ATTR(IMG,ISMAP),
			HTML_ATTR(IMG,LANG),
			HTML_ATTR(IMG,LONGDESC),
			HTML_ATTR(IMG,SRC),
			HTML_ATTR(IMG,STYLE),
			HTML_ATTR(IMG,TITLE),
			HTML_ATTR(IMG,USEMAP),
			HTML_ATTR(IMG,VSPACE),
			HTML_ATTR(IMG,WIDTH),
	};

	enum
	{
		HTML_ATTR(INPUT,ACCEPT) = 0,
			HTML_ATTR(INPUT,ACCESSKEY),
			HTML_ATTR(INPUT,ALIGN),
			HTML_ATTR(INPUT,ALT),
			HTML_ATTR(INPUT,CHECKED),
			HTML_ATTR(INPUT,CLASS),
			HTML_ATTR(INPUT,DIR),
			HTML_ATTR(INPUT,DISABLED),
			HTML_ATTR(INPUT,GLOBAL_COLOR),
			HTML_ATTR(INPUT,ID),
			HTML_ATTR(INPUT,LANG),
			HTML_ATTR(INPUT,MAXLENGTH),
			HTML_ATTR(INPUT,NAME),
			HTML_ATTR(INPUT,READONLY),
			HTML_ATTR(INPUT,SIZE),
			HTML_ATTR(INPUT,SRC),
			HTML_ATTR(INPUT,STYLE),
			HTML_ATTR(INPUT,TABINDEX),
			HTML_ATTR(INPUT,TITLE),
			HTML_ATTR(INPUT,TYPE),
			HTML_ATTR(INPUT,USEMAP),
			HTML_ATTR(INPUT,VALUE),
			HTML_ATTR(INPUT,Z_BTN_TMPL),
			HTML_ATTR(INPUT,Z_INPUT_TMPL),
			HTML_ATTR(INPUT,Z_INPUT_WIDTH),
	};

	enum
	{
		HTML_ATTR(TEXTAREA,CLASS) = 0,
		HTML_ATTR(TEXTAREA,COLS),
		HTML_ATTR(TEXTAREA,DIR),
		HTML_ATTR(TEXTAREA,DISABLED),
		HTML_ATTR(TEXTAREA,ID),
		HTML_ATTR(TEXTAREA,LANG),
		HTML_ATTR(TEXTAREA,MAXLENGTH),
		HTML_ATTR(TEXTAREA,NAME),
		HTML_ATTR(TEXTAREA,READONLY),
		HTML_ATTR(TEXTAREA,ROWS),
		HTML_ATTR(TEXTAREA,STYLE),
		HTML_ATTR(TEXTAREA,TABINDEX),
		HTML_ATTR(TEXTAREA,TITLE),
		HTML_ATTR(TEXTAREA,Z_INPUT_TMPL),
	};

	enum
	{
		HTML_ATTR(P,QUICK_HELP_CONDITION) = 0,
			HTML_ATTR(P,QUICK_HELP_EVENTS),
			HTML_ATTR(P,QUICK_HELP_LINK),
			HTML_ATTR(P,NAME),
	};

	enum
	{
		HTML_ATTR(DIV,CLASS) = 0,
		HTML_ATTR(DIV,ID),
		HTML_ATTR(DIV,NAME),
		HTML_ATTR(DIV,STYLE),
	};


	#undef HTML_ATTR

	// ***************************************************************************

	// A smart ptr for LibWWW strings
	class C3WSmartPtr
	{
	public:
		C3WSmartPtr ()
		{
			_Ptr = NULL;
		}
		C3WSmartPtr (const char *ptr)
		{
			_Ptr = ptr;
		}
		~C3WSmartPtr ()
		{
			clear();
		}
		void operator=(const char *str)
		{
			clear ();
			_Ptr = str;
		}
		operator const char *() const
		{
			return _Ptr;
		}
		void clear()
		{
			if (_Ptr)
			{
				void *ptr = (void*)_Ptr;
				HT_FREE(ptr);
			}
			_Ptr = NULL;
		}
	private:
		const char *_Ptr;
	};

	// ***************************************************************************

	// Read a width HTML parameter. "100" or "100%". Returns true if percent (0 ~ 1) else false
	bool getPercentage (sint32 &width, float &percent, const char *str);

	// ***************************************************************************

	// Parse a HTML color
	NLMISC::CRGBA getColor (const char *color);

	// ***************************************************************************

	void _VerifyLibWWW(const char *function, bool ok, const char *file, int line);
	#define VerifyLibWWW(a,b) _VerifyLibWWW(a,(b)!=FALSE,__FILE__,__LINE__)

	// ***************************************************************************

	// Standard request terminator
	int requestTerminater (HTRequest * request, HTResponse * response, void * param, int status) ;

	// ***************************************************************************
}

#endif
