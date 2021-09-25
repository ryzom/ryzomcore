// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
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



#ifndef XML_MACROS_H
#define XML_MACROS_H

//
// xmlNodePtr cur;
// CXMLAutoPtr prop;
//
// sint i;
// XML_READ_SINT(cur, "prop_name", i, -1);
//

#define XML_READ_UINT(node, name, var, def) { \
	uint tmp; \
	prop = (char *) xmlGetProp(node, (xmlChar*)name); \
	if (prop && fromString((const char*)prop, tmp)) \
		var = tmp; \
	else \
		var = def; \
}

#define XML_READ_SINT(node, name, var, def) { \
	sint tmp; \
	prop = (char *) xmlGetProp(node, (xmlChar*)name); \
	if (prop && fromString((const char*)prop, tmp)) \
		var = tmp; \
	else \
		var = def; \
}

#define XML_READ_BOOL(node, name, var, def) { \
	prop = (char *) xmlGetProp(node, (xmlChar*)name); \
	if (prop) \
		var = NLMISC::toBool((const char*)prop); \
	else \
		var = def; \
}

#define XML_READ_COLOR(node, name, var, def) { \
	NLMISC::CRGBA tmp; \
	prop = (char *) xmlGetProp(node, (xmlChar*)name); \
	if (prop && fromString((const char*)prop, tmp)) \
		var = tmp; \
	else \
		var = def; \
}

#define XML_READ_STRING(node, name, var, def) { \
	prop = (char *) xmlGetProp(node, (xmlChar*)name); \
	if (prop) \
		var = (const char*)prop; \
	else \
		var = def; \
}

#endif // XML_MACROS_H

