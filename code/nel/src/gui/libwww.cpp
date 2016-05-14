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

using namespace NLMISC;

namespace NLGUI
{

	// ***************************************************************************

	/// the cookie value for session identification (nel cookie)
	std::string CurrentCookie;

	/// store all cookies we receive and resent them depending of the domain
	static std::map<std::string, std::map<std::string, std::string> > HTTPCookies;

	// ***************************************************************************

	// Some DTD table

	// Here, modify the DTD table to change the HTML parser (add new tags for examples)

	#undef HTML_ATTR
	#define HTML_ATTR(a,b) { (char*) #b }

	HTAttr a_attr[] =
	{
		HTML_ATTR(A,ACCESSKEY),
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
		{ 0 }
	};

	HTAttr table_attr[] =
	{
		HTML_ATTR(TABLE,ALIGN),
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
			HTML_ATTR(TABLE,WIDTH),
		{ 0 }
	};

	HTAttr tr_attr[] =
	{
		HTML_ATTR(TR,ALIGN),
			HTML_ATTR(TR,BGCOLOR),
			HTML_ATTR(TR,L_MARGIN),
			HTML_ATTR(TR,NOWRAP),
			HTML_ATTR(TR,VALIGN),
		{ 0 }
	};

	HTAttr td_attr[] =
	{
		HTML_ATTR(TD,ABBR),
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
		{ 0 }
	};

	HTAttr img_attr[] =
	{
		HTML_ATTR(IMG,ALIGN),
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
			{ 0 }
	};

	HTAttr input_attr[] =
	{
		HTML_ATTR(INPUT,ACCEPT),
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
		{ 0 }
	};

	HTAttr textarea_attr[] =
	{
		HTML_ATTR(TEXTAREA,CLASS),
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
		{ 0 }
	};

	HTAttr p_attr[] =
	{
		HTML_ATTR(P,QUICK_HELP_CONDITION),
			HTML_ATTR(P,QUICK_HELP_EVENTS),
			HTML_ATTR(P,QUICK_HELP_LINK),
			HTML_ATTR(P,NAME),
		{ 0 }
	};


	HTAttr div_attr[] =
	{
		HTML_ATTR(DIV,CLASS),
			HTML_ATTR(DIV,ID),
			HTML_ATTR(DIV,NAME),
			HTML_ATTR(DIV,STYLE),
		{ 0 }
	};

	HTAttr span_attr[] =
	{
		HTML_ATTR(SPAN,CLASS),
		HTML_ATTR(SPAN,ID),
		HTML_ATTR(SPAN,STYLE),
		{ 0 }
	};

	HTAttr h1_attr[] =
	{
		HTML_ATTR(H1,CLASS),
		HTML_ATTR(H1,ID),
		HTML_ATTR(H1,STYLE),
		{ 0 }
	};

	HTAttr h2_attr[] =
	{
		HTML_ATTR(H2,CLASS),
		HTML_ATTR(H2,ID),
		HTML_ATTR(H2,STYLE),
		{ 0 }
	};

	HTAttr h3_attr[] =
	{
		HTML_ATTR(H3,CLASS),
		HTML_ATTR(H3,ID),
		HTML_ATTR(H3,STYLE),
		{ 0 }
	};

	HTAttr h4_attr[] =
	{
		HTML_ATTR(H4,CLASS),
		HTML_ATTR(H4,ID),
		HTML_ATTR(H4,STYLE),
		{ 0 }
	};

	HTAttr h5_attr[] =
	{
		HTML_ATTR(H5,CLASS),
		HTML_ATTR(H5,ID),
		HTML_ATTR(H5,STYLE),
		{ 0 }
	};

	HTAttr h6_attr[] =
	{
		HTML_ATTR(H6,CLASS),
		HTML_ATTR(H6,ID),
		HTML_ATTR(H6,STYLE),
		{ 0 }
	};

	// ***************************************************************************

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

	void initLibWWW()
	{
		static bool initialized = false;
		if (!initialized)
		{

			// Change the HTML DTD
			SGML_dtd *HTML_DTD = HTML_dtd ();
			HTML_DTD->tags[HTML_TABLE].attributes = table_attr;
			HTML_DTD->tags[HTML_TABLE].number_of_attributes = sizeof(table_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_TR].attributes = tr_attr;
			HTML_DTD->tags[HTML_TR].number_of_attributes = sizeof(tr_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_TD].attributes = td_attr;
			HTML_DTD->tags[HTML_TD].number_of_attributes = sizeof(td_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_TH].attributes = td_attr;
			HTML_DTD->tags[HTML_TH].number_of_attributes = sizeof(td_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_IMG].attributes = img_attr;
			HTML_DTD->tags[HTML_IMG].number_of_attributes = sizeof(img_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_INPUT].attributes = input_attr;
			HTML_DTD->tags[HTML_INPUT].number_of_attributes = sizeof(input_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_TEXTAREA].attributes = textarea_attr;
			HTML_DTD->tags[HTML_TEXTAREA].number_of_attributes = sizeof(textarea_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_P].attributes = p_attr;
			HTML_DTD->tags[HTML_P].number_of_attributes = sizeof(p_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_A].attributes = a_attr;
			HTML_DTD->tags[HTML_A].number_of_attributes = sizeof(a_attr) / sizeof(HTAttr) - 1;
			//HTML_DTD->tags[HTML_I].attributes = a_attr;
			HTML_DTD->tags[HTML_I].number_of_attributes = 0;
			HTML_DTD->tags[HTML_DIV].attributes = div_attr;
			HTML_DTD->tags[HTML_DIV].number_of_attributes = sizeof(div_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_SPAN].attributes = span_attr;
			HTML_DTD->tags[HTML_SPAN].number_of_attributes = sizeof(span_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_H1].attributes = h1_attr;
			HTML_DTD->tags[HTML_H1].number_of_attributes = sizeof(h1_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_H2].attributes = h2_attr;
			HTML_DTD->tags[HTML_H2].number_of_attributes = sizeof(h2_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_H3].attributes = h3_attr;
			HTML_DTD->tags[HTML_H3].number_of_attributes = sizeof(h3_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_H4].attributes = h4_attr;
			HTML_DTD->tags[HTML_H4].number_of_attributes = sizeof(h4_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_H5].attributes = h5_attr;
			HTML_DTD->tags[HTML_H5].number_of_attributes = sizeof(h5_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_H6].attributes = h6_attr;
			HTML_DTD->tags[HTML_H6].number_of_attributes = sizeof(h6_attr) / sizeof(HTAttr) - 1;

			// Initialized
			initialized = true;
		}
	}

	// ***************************************************************************
}

