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
#include <libxml/HTMLparser.h>

#include "nel/misc/types_nl.h"
#include "nel/gui/libwww.h"
#include "nel/gui/group_html.h"
#include "nel/gui/lua_ihm.h"

using namespace std;
using namespace NLMISC;

namespace NLGUI
{
	// ***************************************************************************
	void CGroupHTML::htmlElement(xmlNode *node, int element_number)
	{
		SGML_dtd *HTML_DTD = HTML_dtd ();

		if (element_number < HTML_ELEMENTS)
		{
			CXMLAutoPtr ptr;
			// load attributes into libwww structs
			BOOL present[MAX_ATTRIBUTES] = {0};
			const char *value[MAX_ATTRIBUTES] = {NULL};
			std::string strvalues[MAX_ATTRIBUTES];

			uint nbAttributes = std::min(MAX_ATTRIBUTES, HTML_DTD->tags[element_number].number_of_attributes);
			for(uint i=0; i<nbAttributes; i++)
			{
				std::string name;
				name = toLower(std::string(HTML_DTD->tags[element_number].attributes[i].name));
				ptr = xmlGetProp(node, (const xmlChar *)name.c_str());
				if (ptr)
				{
					// copy xmlChar to string (xmlChar will be released)
					strvalues[i] = (const char *)(ptr);
					// now use string pointer in value[] array
					value[i] = strvalues[i].c_str();
					present[i] = true;
				}
			}

			if (element_number == HTML_A)
			{
				addLink(element_number, present, value);
			}

			beginElement(element_number, present, value);
		}
		else
		{
			beginUnparsedElement((const char *)(node->name), xmlStrlen(node->name));
		}

		// recursive - text content / child nodes
		htmlWalkDOM(node->children);

		// closing tag
		if (element_number < HTML_ELEMENTS)
		{
			endElement(element_number);
		}
		else
		{
			endUnparsedElement((const char *)(node->name), xmlStrlen(node->name));
		}
	}

	// ***************************************************************************
	// recursive function to walk html document
	void CGroupHTML::htmlWalkDOM(xmlNode *a_node)
	{
		SGML_dtd *HTML_DTD = HTML_dtd ();

		uint element_number;
		xmlNode *node = a_node;
		while(node)
		{
			if (node->type == XML_TEXT_NODE)
			{
				addText((const char *)(node->content), xmlStrlen(node->content));
			}
			else
			if (node->type == XML_ELEMENT_NODE)
			{
				// find libwww tag
				for(element_number = 0; element_number<HTML_ELEMENTS; ++element_number)
				{
					if (xmlStrncasecmp(node->name, (const xmlChar *)HTML_DTD->tags[element_number].name.c_str(), xmlStrlen(node->name)) == 0)
						break;
				}

				htmlElement(node, element_number);
			}

			// move into next sibling
			node = node->next;
		}
	}

	// ***************************************************************************
	bool CGroupHTML::parseHtml(std::string htmlString)
	{
		htmlParserCtxtPtr parser = htmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL, XML_CHAR_ENCODING_NONE);
		if (!parser)
		{
			nlwarning("Creating html parser context failed");
			return false;
		}

		htmlCtxtUseOptions(parser, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

		htmlParseChunk(parser, htmlString.c_str(), htmlString.size(), 0);
		htmlParseChunk(parser, "", 0, 1);

		bool success = true;
		if (parser->myDoc)
		{
			xmlNode *root = xmlDocGetRootElement(parser->myDoc);
			if (root)
			{
				htmlWalkDOM(root);
			}
			else
			{
				nlwarning("html root node failed");
				success = false;
			}
		}
		else
		{
			nlwarning("htmlstring parsing failed");
			success = false;
		}

		htmlFreeParserCtxt(parser);
		return success;
	}

	// ***************************************************************************
	int CGroupHTML::luaParseHtml(CLuaState &ls)
	{
		const char *funcName = "parseHtml";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		std::string html = ls.toString(1);

		parseHtml(html);

		return 0;
	}

}

