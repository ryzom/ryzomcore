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

#include "nel/gui/group_html.h"

// LibWWW
extern "C"
{
#include "WWWLib.h"			      /* Global Library Include file */
#include "WWWApp.h"
#include "WWWInit.h"
}

#include "nel/gui/group_html.h"
#include "nel/gui/libwww_nel_stream.h"

using namespace NLMISC;

// The HText structure for libwww
struct _HText
{
	NLGUI::CGroupHTML *Parent;
};

namespace NLGUI
{

	/// the cookie value for session identification (nel cookie)
	std::string CurrentCookie;

	/// store all cookies we receive and resent them depending of the domain
	std::map<std::string, std::map<std::string, std::string> > HTTPCookies;
	std::string HTTPCurrentDomain;	// The current domain that will be used to get which cookies to send

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

	// ***************************************************************************

	void _VerifyLibWWW(const char *function, bool ok, const char *file, int line)
	{
		if (!ok)
			nlwarning("%s(%d) : LIBWWW %s returned a bad status", file, line, function);
	}
	#define VerifyLibWWW(a,b) _VerifyLibWWW(a,(b)!=FALSE,__FILE__,__LINE__)

	// ***************************************************************************

	int NelPrinter (const char * fmt, va_list pArgs)
	{
		char info[1024];
		int ret;

		ret = vsnprintf(info, sizeof(info), fmt, pArgs);
		nlinfo("%s", info);
		return ret;
	}

	// ***************************************************************************

	int NelTracer (const char * fmt, va_list pArgs)
	{
		char err[1024];
		int ret;

		ret = vsnprintf(err, sizeof(err), fmt, pArgs);
		nlwarning ("%s", err);
		return ret;
	}

	// ***************************************************************************

	HText * TextNew (HTRequest *		request,
				  HTParentAnchor *	/* anchor */,
				  HTStream *		/* output_stream */)
	{
		HText *text = new HText;
		text->Parent = (CGroupHTML *) HTRequest_context(request);
		return text;
	}

	// ***************************************************************************

	BOOL TextDelete (HText * me)
	{
		delete me;
		return YES;
	}

	// ***************************************************************************

	void TextBuild (HText * me, HTextStatus status)
	{
		// Do the work in the class
		if (status == HTEXT_BEGIN)
			me->Parent->beginBuild ();
		else if (status == HTEXT_END)
			me->Parent->endBuild ();
	}

	// ***************************************************************************

	void TextAdd (HText * me, const char * buf, int len)
	{
		// Do the work in the class
		me->Parent->addText (buf, len);
	}

	// ***************************************************************************

	void TextLink (HText * 	me,
				int		element_number,
				int		attribute_number,
				HTChildAnchor *	anchor,
				const BOOL * 	present,
				const char **	value)
	{
		// Do the work in the class
		me->Parent->addLink (element_number, attribute_number, anchor, present, value);
	}

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

	// ***************************************************************************

	void TextBeginElement (HText *me, int element_number, const BOOL *present, const char **value)
	{
		// Do the work in the class
		me->Parent->beginElement (element_number, present, value);
	}

	// ***************************************************************************

	void TextEndElement (HText *me, int element_number)
	{
		// Do the work in the class
		me->Parent->endElement (element_number);
	}

	// ***************************************************************************
	void TextBeginUnparsedElement(HText *me, const char *buffer, int length)
	{
		me->Parent->beginUnparsedElement(buffer, length);
	}

	// ***************************************************************************
	void TextEndUnparsedElement(HText *me, const char *buffer, int length)
	{
		me->Parent->endUnparsedElement(buffer, length);
	}

	// ***************************************************************************
	void TextUnparsedEntity (HText * /* HText */, const char *buffer, int length)
	{
		std::string str(buffer, buffer+length);
		nlinfo("Unparsed entity '%s'", str.c_str());
	}

	// ***************************************************************************
	int requestTerminater (HTRequest * request, HTResponse * /* response */,
					void * param, int /* status */)
	{
		/*
			Yoyo and Boris: we had to make the request terminate by UID and not by pointer (param is an uid).
			Because this method was called at mainLoop time, but for GroupHTML created/deleted at login time !!!
			=> Memory Crash.
		*/

		// TestYoyo
		//nlinfo("** requestTerminater(): uid%d", (uint32)param);

		// the parameter is actually an uint32
		if (param != 0)
		{
			CGroupHTML::TGroupHtmlByUIDMap::iterator	it= CGroupHTML::_GroupHtmlByUID.find((uint32)(size_t)param);
			if(it!=CGroupHTML::_GroupHtmlByUID.end())
			{
				// get the pointer. NB: the refptr should not be NULL
				// since object removed from map when deleted
				CGroupHTML *gh = it->second;
				nlassert(gh);

				// callback the browser
				gh->requestTerminated(request);
			}
		}
		return HT_OK;
	}


	// callback called when receiving a cookie
	BOOL receiveCookie (HTRequest * /* request */, HTCookie * cookie, void * /* param */)
	{
		if (strcmp(HTCookie_name(cookie), "ryzomId") == 0)
		{
			// we receive the ryzom id cookie, store it
			CurrentCookie = HTCookie_value(cookie);
		}
		else
		{
			// store the id/value cookie
			HTTPCookies[HTTPCurrentDomain][HTCookie_name(cookie)] = HTCookie_value(cookie);
	//		nlwarning("get cookie for domain %s %s=%s", HTTPCurrentDomain.c_str(), HTCookie_name(cookie), HTCookie_value(cookie));
		}

		return YES;
	}

	// callback called to add cookie to a request before sending it to the server
	HTAssocList *sendCookie (HTRequest * /* request */, void * /* param */)
	{
		HTAssocList * alist = 0;
		if (!CurrentCookie.empty())
		{
			if(alist == 0) alist = HTAssocList_new();	/* Is deleted by the cookie module */
			HTAssocList_addObject(alist, "ryzomId", CurrentCookie.c_str());
		}

		if(!HTTPCookies[HTTPCurrentDomain].empty())
		{
			if(alist == 0) alist = HTAssocList_new();
			for(std::map<std::string, std::string>::iterator it = HTTPCookies[HTTPCurrentDomain].begin(); it != HTTPCookies[HTTPCurrentDomain].end(); it++)
			{
				HTAssocList_addObject(alist, it->first.c_str(), it->second.c_str());
	//			nlwarning("set cookie for domain '%s' %s=%s", HTTPCurrentDomain.c_str(), it->first.c_str(), it->second.c_str());
			}
		}
		return alist;
	}


	// ***************************************************************************

	HTAnchor * TextFindAnchor (HText * /* me */, int /* index */)
	{
		return NULL;
	}

	int HTMIME_location_custom (HTRequest * request, HTResponse * response, char * token, char * value)
	{
		char * location = HTStrip(value);

		std::string finalLocation;

		//nlinfo("redirect to '%s' '%s'", value, location);

		// If not absolute URI (Error) then find the base
		if (!HTURL_isAbsolute(location))
		{
			char * base = HTAnchor_address((HTAnchor *) HTRequest_anchor(request));
			location = HTParse(location, base, PARSE_ALL);
			//redirection = HTAnchor_findAddress(location);
			finalLocation = location;
			HT_FREE(base);
			HT_FREE(location);
		}
		else
		{
			finalLocation = location;
		}
		//nlinfo("final location '%s'", finalLocation.c_str());

		CGroupHTML *gh = (CGroupHTML *) HTRequest_context(request);

		gh->setURL(finalLocation);

		return HT_OK;
	}


	// ***************************************************************************

	const std::string &setCurrentDomain(const std::string &url)
	{
		if(url.find("http://") == 0)
		{
			HTTPCurrentDomain = url.substr(7, url.find('/', 7)-7);
	//		nlinfo("****cd: %s", HTTPCurrentDomain.c_str());
		}
		else
		{
			HTTPCurrentDomain.clear();
	//		nlinfo("****cd: clear the domain");
		}
		return HTTPCurrentDomain;
	}


	void initLibWWW()
	{
		static bool initialized = false;
		if (!initialized)
		{
			//HTProfile_newNoCacheClient("Ryzom", "1.1");

			/* Need our own trace and print functions */
			HTPrint_setCallback(NelPrinter);
			HTTrace_setCallback(NelTracer);

			/* Initiate libwww */
			HTLib_setAppName( CGroupHTML::options.appName.c_str() );
			HTLib_setAppVersion( CGroupHTML::options.appVersion.c_str() );

			/* Set up TCP as transport */
			VerifyLibWWW("HTTransport_add", HTTransport_add("buffered_tcp", HT_TP_SINGLE, HTReader_new, HTBufferWriter_new));
			VerifyLibWWW("HTTransport_add", HTTransport_add("local", HT_TP_SINGLE, HTNeLReader_new, HTWriter_new));
			// VerifyLibWWW("HTTransport_add", HTTransport_add("local", HT_TP_SINGLE, HTANSIReader_new, HTWriter_new));
			// VerifyLibWWW("HTTransport_add", HTTransport_add("local", HT_TP_SINGLE, HTReader_new, HTWriter_new));

			/* Set up HTTP as protocol */
			VerifyLibWWW("HTProtocol_add", HTProtocol_add("http", "buffered_tcp", 80, NO, HTLoadHTTP, NULL));
			VerifyLibWWW("HTProtocol_add", HTProtocol_add("file", 	"local", 	0, 	YES, 	HTLoadNeLFile, 	NULL));
			//VerifyLibWWW("HTProtocol_add", HTProtocol_add("file", 	"local", 	0, 	YES, 	HTLoadFile, 	NULL));
			// HTProtocol_add("cache", 	"local", 	0, 	NO, 	HTLoadCache, 	NULL);

			HTBind_init();
			// HTCacheInit(NULL, 20);

			/* Setup up transfer coders */
			HTFormat_addTransferCoding((char*)"chunked", HTChunkedEncoder, HTChunkedDecoder, 1.0);

			/* Setup MIME stream converters */
			HTFormat_addConversion("message/rfc822", "*/*", HTMIMEConvert, 1.0, 0.0, 0.0);
			HTFormat_addConversion("message/x-rfc822-foot", "*/*", HTMIMEFooter, 1.0, 0.0, 0.0);
			HTFormat_addConversion("message/x-rfc822-head", "*/*", HTMIMEHeader, 1.0, 0.0, 0.0);
			HTFormat_addConversion("message/x-rfc822-cont", "*/*", HTMIMEContinue, 1.0, 0.0, 0.0);
			HTFormat_addConversion("message/x-rfc822-partial","*/*", HTMIMEPartial, 1.0, 0.0, 0.0);
			HTFormat_addConversion("multipart/*", "*/*", HTBoundary, 1.0, 0.0, 0.0);

			/* Setup HTTP protocol stream */
			HTFormat_addConversion("text/x-http", "*/*", HTTPStatus_new, 1.0, 0.0, 0.0);

			/* Setup the HTML parser */
			HTFormat_addConversion("text/html", "www/present", HTMLPresent, 1.0, 0.0, 0.0);

			/* Setup black hole stream */
			HTFormat_addConversion("*/*", "www/debug", HTBlackHoleConverter, 1.0, 0.0, 0.0);
			HTFormat_addConversion("*/*", "www/present", HTBlackHoleConverter, 0.3, 0.0, 0.0);

			/* Set max number of sockets we want open simultaneously */
			HTNet_setMaxSocket(32);

			/* Register our HTML parser callbacks */
			VerifyLibWWW("HText_registerCDCallback", HText_registerCDCallback (TextNew, TextDelete));
			VerifyLibWWW("HText_registerBuildCallback", HText_registerBuildCallback (TextBuild));
			VerifyLibWWW("HText_registerTextCallback", HText_registerTextCallback(TextAdd));
			VerifyLibWWW("HText_registerLinkCallback", HText_registerLinkCallback (TextLink));
			VerifyLibWWW("HText_registerElementCallback", HText_registerElementCallback (TextBeginElement, TextEndElement));
			VerifyLibWWW("HText_registerUnparsedElementCallback", HText_registerUnparsedElementCallback(TextBeginUnparsedElement, TextEndUnparsedElement));
			VerifyLibWWW("HText_registerUnparsedEntityCallback ", HText_registerUnparsedEntityCallback (TextUnparsedEntity ));


			/* Register the default set of MIME header parsers */
			struct {
				const char * string;
			HTParserCallback * pHandler;
			} fixedHandlers[] = {
			{"accept", &HTMIME_accept},
			{"accept-charset", &HTMIME_acceptCharset},
			{"accept-encoding", &HTMIME_acceptEncoding},
			{"accept-language", &HTMIME_acceptLanguage},
			{"accept-ranges", &HTMIME_acceptRanges},
			{"authorization", NULL},
			{"cache-control", &HTMIME_cacheControl},
			{"connection", &HTMIME_connection},
			{"content-encoding", &HTMIME_contentEncoding},
			{"content-length", &HTMIME_contentLength},
			{"content-range", &HTMIME_contentRange},
			{"content-transfer-encoding", &HTMIME_contentTransferEncoding},
			{"content-type", &HTMIME_contentType},
			{"digest-MessageDigest", &HTMIME_messageDigest},
			{"keep-alive", &HTMIME_keepAlive},
			{"link", &HTMIME_link},
			{"location", &HTMIME_location_custom},
			{"max-forwards", &HTMIME_maxForwards},
			{"mime-version", NULL},
			{"pragma", &HTMIME_pragma},
				{"protocol", &HTMIME_protocol},
				{"protocol-info", &HTMIME_protocolInfo},
				{"protocol-request", &HTMIME_protocolRequest},
			{"proxy-authenticate", &HTMIME_authenticate},
			{"proxy-authorization", &HTMIME_proxyAuthorization},
			{"public", &HTMIME_public},
			{"range", &HTMIME_range},
			{"referer", &HTMIME_referer},
			{"retry-after", &HTMIME_retryAfter},
			{"server", &HTMIME_server},
			{"trailer", &HTMIME_trailer},
			{"transfer-encoding", &HTMIME_transferEncoding},
			{"upgrade", &HTMIME_upgrade},
			{"user-agent", &HTMIME_userAgent},
			{"vary", &HTMIME_vary},
			{"via", &HTMIME_via},
			{"warning", &HTMIME_warning},
			{"www-authenticate", &HTMIME_authenticate},
				{"authentication-info", &HTMIME_authenticationInfo},
				{"proxy-authentication-info", &HTMIME_proxyAuthenticationInfo}
			};

			for (uint i = 0; i < sizeof(fixedHandlers)/sizeof(fixedHandlers[0]); i++)
				HTHeader_addParser(fixedHandlers[i].string, NO, fixedHandlers[i].pHandler);

			/* Set up default event loop */
			HTEventInit();

			/* Add our own request terminate handler */
			HTNet_addAfter(requestTerminater, NULL, 0, HT_ALL, HT_FILTER_LAST);

			/* Setup cookies */
			HTCookie_init();
			HTCookie_setCookieMode(HTCookieMode(HT_COOKIE_ACCEPT | HT_COOKIE_SEND));
			HTCookie_setCallbacks(receiveCookie, NULL, sendCookie, NULL);

			/* Start the first request */

			/* Go into the event loop... */
			// HTEventList_newLoop();

			// App_delete(app);

			HTBind_add("htm",	"text/html",			NULL,		"8bit",		NULL,	1.0);	/* HTML			*/
			HTBind_add("html",	"text/html",			NULL,		"8bit",		NULL,	1.0);	/* HTML			*/

			HTBind_caseSensitive(NO);

			// Change the HTML DTD
			SGML_dtd *HTML_DTD = HTML_dtd ();
			HTML_DTD->tags[HTML_TABLE].attributes = table_attr;
			HTML_DTD->tags[HTML_TABLE].number_of_attributes = sizeof(table_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_TR].attributes = tr_attr;
			HTML_DTD->tags[HTML_TR].number_of_attributes = sizeof(tr_attr) / sizeof(HTAttr) - 1;
			HTML_DTD->tags[HTML_TD].attributes = td_attr;
			HTML_DTD->tags[HTML_TD].number_of_attributes = sizeof(td_attr) / sizeof(HTAttr) - 1;
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

			// Set a request timeout
			//		HTHost_setEventTimeout (30000);
			//		HTHost_setActiveTimeout (30000);
			//		HTHost_setPersistTimeout (30000);

			// libwww default value is 2000ms for POST/PUT requests on the first and 3000 on the second, smallest allowed value is 21ms
			// too small values may create timeout problems but we want it low as possible
			// second value is the timeout for the second try to we set that high
			HTTP_setBodyWriteDelay(250, 3000);
	
			// Initialized
			initialized = true;
		}
	}

	// ***************************************************************************
}

