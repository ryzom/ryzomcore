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

#include "nel/gui/html_parser.h"

#include <string>
#include <libxml/HTMLparser.h>

#include "nel/misc/types_nl.h"
#include "nel/gui/libwww.h"
#include "nel/gui/group_html.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	// ***************************************************************************
	void CHtmlParser::parseStyle(xmlNode *a_node, std::string &styleString) const
	{
		xmlNode *node = a_node;
		while(node)
		{
			if (node->type == XML_CDATA_SECTION_NODE)
			{
				styleString += (const char*)node->content;
			}
			else
			{
				nlwarning("<style> tag has child elements other than cdata[%d]", node->type);
			}

			node = node->next;
		}
	}

	// ***************************************************************************
	// recursive function to walk html document
	void CHtmlParser::parseNode(xmlNode *a_node, CHtmlElement &parent, std::string &styleString, std::vector<std::string> &links) const
	{
		uint childIndex = 0;
		uint element_number;
		xmlNode *node = a_node;
		while(node)
		{
			if (node->type == XML_TEXT_NODE)
			{
				parent.Children.push_back(CHtmlElement(CHtmlElement::TEXT_NODE, (const char*)(node->content)));
			}
			else
			if (node->type == XML_ELEMENT_NODE)
			{
				// find html element
				element_number = htmlElementLookup((const char*)node->name);

				// get pointer to previous sibling
				CHtmlElement *prevSibling = NULL;
				if (!parent.Children.empty())
				{
					// skip text nodes
					for(std::list<CHtmlElement>::reverse_iterator it = parent.Children.rbegin(); it != parent.Children.rend(); ++it)
					{
						if (it->Type == CHtmlElement::ELEMENT_NODE)
						{
							prevSibling = &(*it);
							break;
						}
					}
				}

				parent.Children.push_back(CHtmlElement(CHtmlElement::ELEMENT_NODE, toLower((const char*)node->name)));
				CHtmlElement &elm = parent.Children.back();
				elm.ID = element_number;
				elm.parent = &parent;
				elm.childIndex = childIndex;

				// previous/next sibling that is ELEMENT_NODE
				elm.previousSibling = prevSibling;
				if (prevSibling)
				{
					prevSibling->nextSibling = &parent.Children.back();
				}

				childIndex++;

				// TODO: harvest <link type="css">, <style>, <img>

				elm.Attributes.clear();

				for (xmlAttr *cur_attr = node->properties; cur_attr; cur_attr = cur_attr->next) {
					std::string key(toLower((const char *)(cur_attr->name)));
					std::string value;
					if (cur_attr->children)
					{
						value = (const char *)(cur_attr->children->content);
					}
					elm.Attributes[key] = value;
				}

				if (elm.hasAttribute("class"))
				{
					std::vector<std::string> parts;
					NLMISC::splitString(elm.getAttribute("class"), " ", parts);
					for(uint i = 0; i<parts.size();++i)
					{
						elm.ClassNames.insert(toLower(trim(parts[i])));
					}
				}

				if (elm.Value == "style")
				{
					// <style type="text/css" media="all, screen">
					// ...
					// </style>
					bool useStyle = true;
					if (elm.hasAttribute("media"))
					{
						std::string media = trim(toLower(elm.Attributes["media"]));
						useStyle = media.empty() || media.find("all") != std::string::npos || media.find("screen") != std::string::npos;

						// <style media="ryzom"> for ingame browser
						useStyle = useStyle || media == "ryzom";
					}

					if (useStyle)
					{
						parseStyle(node->children, styleString);
					}
					// style tag is kept in dom
				}
				if (elm.Value == "link" && elm.getAttribute("rel") == "stylesheet")
				{
					bool useStyle = true;
					if (elm.hasAttribute("media"))
					{
						std::string media = trim(toLower(elm.Attributes["media"]));
						useStyle = media.empty() || media.find("all") != std::string::npos || media.find("screen") != std::string::npos;

						// <style media="ryzom"> for ingame browser
						useStyle = useStyle || media == "ryzom";
					}

					if (useStyle)
					{
						links.push_back(elm.getAttribute("href"));
					}
					// link tag is kept in dom
				}
				else if (node->children)
				{
					parseNode(node->children, elm, styleString, links);

					// must cleanup nested tags that libxml2 does not fix
					// dt without end tag: <dl><dt><dt></dl>
					// dd without end tag: <dl><dd><dd></dl>
					if (!elm.Children.empty() && (elm.Value == "dt" || elm.Value == "dd"))
					{
						std::string tag = elm.Value;
						std::list<CHtmlElement>::iterator it;
						for(it = elm.Children.begin(); it != elm.Children.end(); ++it)
						{
							if (it->Type == CHtmlElement::ELEMENT_NODE && it->Value == tag)
							{
								// relocate this and remaining elements over to parent
								parent.Children.splice(parent.Children.end(), elm.Children, it, elm.Children.end());
								break;
							}
						}
						elm.reindexChilds();
						parent.reindexChilds();
					}
				}
			}

			// move into next sibling
			node = node->next;
		}
	}

	// ***************************************************************************
	// http://stackoverflow.com/a/18335183
	static std::string correctNonUtf8(const std::string &str)
	{
		int i, f_size=str.size();
		unsigned char c,c2,c3,c4;
		std::string to;
		to.reserve(f_size);

		for(i=0 ; i<f_size ; i++)
		{
			c=(unsigned char)(str[i]);
			if (c<32)
			{
				//control char
				if(c==9 || c==10 || c==13)
				{
					//allow only \t \n \r
					to.append(1,c);
				}
				continue;
			}
			else if (c<127)
			{
				//normal ASCII
				to.append(1,c);
				continue;
			}
			else if (c < 160)
			{
				//control char (nothing should be defined here either ASCI, ISO_8859-1 or UTF8, so skipping)
				if (c == 128)
				{
					//fix microsoft mess, add euro
					to.append(1,226);
					to.append(1,130);
					to.append(1,172);
				}

				if (c == 133)
				{
					//fix IBM mess, add NEL = \n\r
					to.append(1,10);
					to.append(1,13);
				}
				continue;
			}
			else if (c < 192)
			{
				//invalid for UTF8, converting ASCII
				to.append(1,(unsigned char)194);
				to.append(1,c);
				continue;
			}
			else if (c < 194)
			{
				//invalid for UTF8, converting ASCII
				to.append(1,(unsigned char)195);
				to.append(1,c-64);
				continue;
			}
			else if (c < 224 && i + 1 < f_size)
			{
				//possibly 2byte UTF8
				c2 = (unsigned char)(str[i+1]);

				if (c2 > 127 && c2 < 192)
				{
					//valid 2byte UTF8
					if (c == 194 && c2 < 160)
					{
						//control char, skipping
						;
					}
					else
					{
						to.append(1,c);
						to.append(1,c2);
					}
					i++;
					continue;
				}
			}
			else if (c < 240 && i + 2 < f_size)
			{
				// possibly 3byte UTF8
				c2 = (unsigned char)(str[i+1]);
				c3 = (unsigned char)(str[i+2]);

				if (c2 > 127 && c2 < 192 && c3 > 127 && c3 < 192)
				{
					// valid 3byte UTF8
					to.append(1,c);
					to.append(1,c2);
					to.append(1,c3);
					i+=2;
					continue;
				}
			}
			else if (c < 245 && i + 3 < f_size)
			{
				//possibly 4byte UTF8
				c2 = (unsigned char)(str[i+1]);
				c3 = (unsigned char)(str[i+2]);
				c4 = (unsigned char)(str[i+3]);
				if (c2 > 127 && c2 < 192 && c3 > 127 && c3 < 192 && c4 > 127 && c4 < 192)
				{
					//valid 4byte UTF8
					to.append(1,c);
					to.append(1,c2);
					to.append(1,c3);
					to.append(1,c4);
					i+=3;
					continue;
				}
			}

			//invalid UTF8, converting ASCII (c>245 || string too short for multi-byte))
			to.append(1,(unsigned char)195);
			to.append(1,c-64);
		}
		return to;
	}

	// ***************************************************************************
	static void patchHtmlQuirks(std::string &htmlString)
	{
		size_t npos = std::string::npos;
		size_t pos;

		// get rid of BOM (some ingame help files does not show up otherwise)
		if (htmlString.substr(0, 3) == "\xEF\xBB\xBF")
		{
			htmlString.erase(0, 3);
		}

		// if any element is before <html>, then parser adds <html><body>
		// and original tags are ignored (their attributes not processed)
		//
		// only fix situation when there is <body> tag with attributes
		//
		// tags are considered to be lowercase

		pos = htmlString.find("<body ");
		if (pos != npos)
		{
			size_t start = htmlString.find("<");
			// skip <!doctype html>
			if (htmlString.substr(start, 2) == "<!")
				start = htmlString.find("<", start + 1);

			// if there is no html tag, then abort
			size_t end = htmlString.find("<html>");
			if (end != npos && start < end && end < pos)
			{
				// body tag end position
				size_t insert = htmlString.find(">", pos);
				if (insert != npos)
				{
					std::string str = htmlString.substr(start, end - start);
					htmlString.insert(insert+1, str);
					htmlString.erase(start, str.size());
				}
			}
		}

		// make sure </html> (if present) is last in document or tags coming after it are ignored
		pos = htmlString.find("</html>");
		if (pos != npos && htmlString.find("<", pos+1) > pos)
		{
			htmlString.erase(pos, 7);
			htmlString += "</html>";
		}

		// if there is invalid utf-8 chars, then libxml will break everything after first it finds.
		htmlString = correctNonUtf8(htmlString);
	}

	// ***************************************************************************
	void CHtmlParser::getDOM(std::string htmlString, CHtmlElement &dom, std::string &styleString, std::vector<std::string> &links) const
	{
		htmlParserCtxtPtr parser = htmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL, XML_CHAR_ENCODING_UTF8);
		if (!parser)
		{
			nlwarning("Creating html parser context failed");
			return;
		}

		htmlCtxtUseOptions(parser, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

		// parser is little strict on tag order, so fix whats needed
		patchHtmlQuirks(htmlString);

		htmlParseChunk(parser, htmlString.c_str(), htmlString.size(), 0);
		htmlParseChunk(parser, "", 0, 1);

		if (parser->myDoc)
		{
			xmlNode *root = xmlDocGetRootElement(parser->myDoc);
			if (root)
			{
				styleString.clear();
				parseNode(root, dom, styleString, links);
			}
			else
			{
				nlwarning("html root node failed");
			}
			xmlFreeDoc(parser->myDoc);
		}
		else
		{
			nlwarning("htmlstring parsing failed");
		}

		htmlFreeParserCtxt(parser);
	}

}


