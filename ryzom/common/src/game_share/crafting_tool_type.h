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




#ifndef RY_CRAFTING_TOOL_TYPE_H
#define RY_CRAFTING_TOOL_TYPE_H


namespace TOOL_TYPE
{
	enum TCraftingToolType
	{
		ArmorTool,
		JewelryTool,
		AmmoTool,
		RangeWeaponTool,
		MeleeWeaponTool,
		ToolMaker,

		Unknown,
		NUM_CRAFTING_TOOL_TYPE = Unknown,
	};

	/**
	  * get the right string from the given enum value
	  * \param jop the TCraftingToolType value to convert
	  * \return the string associated to this enum number (Unknown if the enum number not exist)
	  */
	const std::string& toString( TCraftingToolType type );

	/**
	  * get the right TCraftingToolType from its string
	  * \param str the input string
	  * \return the TCraftingToolType associated to this string (unknown if the string cannot be interpreted)
	  */
	TCraftingToolType toToolType( const std::string& str );

} // TOOL_TYPE

#endif // RY_CRAFTING_TOOL_TYPE_H //

