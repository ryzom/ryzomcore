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
#include "crafting_tool_type.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace TOOL_TYPE
{
	// The conversion table
	const CStringConversion<TCraftingToolType>::CPair stringTable [] =
	{
		{ "None" , Unknown },
		{ "ArmorTool", ArmorTool },
		{ "JewelryTool", JewelryTool },
		{ "AmmoTool", AmmoTool },
		{ "RangeWeaponTool", RangeWeaponTool },
		{ "MeleeWeaponTool", MeleeWeaponTool },
		{ "ToolMaker", ToolMaker },
	};

	CStringConversion<TCraftingToolType> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  Unknown);

// convert type id to type name string
const std::string& toString( TCraftingToolType type )
{
	return conversion.toString(type);
}

// convert type name to type enum value
TCraftingToolType toToolType( const std::string& str )
{
	return conversion.fromString(str);
}


} // TOOL_TYPE
