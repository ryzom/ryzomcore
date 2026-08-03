// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
	~CIF3DSceneParser() NL_OVERRIDE;

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup ) NL_OVERRIDE;
};

class CIFDDXParser : public CInterfaceParser::IParserModule
{
public:
	CIFDDXParser();
	~CIFDDXParser() NL_OVERRIDE;

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup ) NL_OVERRIDE;
};

class CActionCategoryParser : public CInterfaceParser::IParserModule
{
public:
	CActionCategoryParser();
	~CActionCategoryParser() NL_OVERRIDE;

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup ) NL_OVERRIDE;
};

class CCommandParser : public CInterfaceParser::IParserModule
{
public:
	CCommandParser();
	~CCommandParser() NL_OVERRIDE;

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup ) NL_OVERRIDE;
};

class CKeyParser : public CInterfaceParser::IParserModule
{
public:
	CKeyParser();
	~CKeyParser() NL_OVERRIDE;

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup ) NL_OVERRIDE;
};

class CMacroParser : public CInterfaceParser::IParserModule
{
public:
	CMacroParser();
	~CMacroParser() NL_OVERRIDE;

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup ) NL_OVERRIDE;
};

class CLandmarkParser : public CInterfaceParser::IParserModule
{
public:
	CLandmarkParser();
	~CLandmarkParser() NL_OVERRIDE;

	bool parse( xmlNodePtr cur, CInterfaceGroup *parentGroup ) NL_OVERRIDE;
};

#endif
