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
#include "nel/misc/types_nl.h"
#include "nel/gui/css_parser.h"
#include "nel/gui/css_style.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	// ***************************************************************************
	// Parse style declarations style, eg. "color:red; font-size: 10px;"
	//
	// key is converted to lowercase
	// value is left as is
	TStyle CCssParser::parseDecls(const std::string &styleString)
	{
		TStyle styles;
		std::vector<std::string> elements;
		NLMISC::splitString(styleString, ";", elements);

		for(uint i = 0; i < elements.size(); ++i)
		{
			std::string::size_type pos;
			pos = elements[i].find_first_of(':');
			if (pos != std::string::npos)
			{
				std::string key = trim(toLower(elements[i].substr(0, pos)));
				std::string value = trim(elements[i].substr(pos+1));
				styles[key] = value;
			}
		}

		return styles;
	}
} // namespace

