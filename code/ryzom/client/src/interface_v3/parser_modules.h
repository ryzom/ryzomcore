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


#ifndef PARSER_MODULES_H
#define PARSER_MODULES_H

#include "nel/gui/interface_parser.h"

using namespace NLGUI;

class CIF3DSceneParser : public CInterfaceParser::IParserModule
{
public:
	CIF3DSceneParser();
	~CIF3DSceneParser();

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup );
};

class CIFDDXParser : public CInterfaceParser::IParserModule
{
public:
	CIFDDXParser();
	~CIFDDXParser();

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup );
};

class CActionCategoryParser : public CInterfaceParser::IParserModule
{
public:
	CActionCategoryParser();
	~CActionCategoryParser();

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup );
};

class CCommandParser : public CInterfaceParser::IParserModule
{
public:
	CCommandParser();
	~CCommandParser();

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup );
};

class CKeyParser : public CInterfaceParser::IParserModule
{
public:
	CKeyParser();
	~CKeyParser();

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup );
};

class CMacroParser : public CInterfaceParser::IParserModule
{
public:
	CMacroParser();
	~CMacroParser();

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup );
};

#endif
