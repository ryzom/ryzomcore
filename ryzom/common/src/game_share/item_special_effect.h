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



#ifndef RY_ITEM_SPECIAL_EFFECT_H
#define RY_ITEM_SPECIAL_EFFECT_H

#include "nel/misc/types_nl.h"

namespace ITEM_SPECIAL_EFFECT
{
	// This file is generated, do not edit it. Source file is (probably) item_special_effect.lua.
	enum TItemSpecialEffect
	{
		ISE_FIGHT_ADD_CRITICAL,
		ISE_FIGHT_VAMPIRISM,
		ISE_MAGIC_DIVINE_INTERVENTION,
		ISE_MAGIC_SHOOT_AGAIN,
		ISE_CRAFT_ADD_STAT_BONUS,
		ISE_CRAFT_ADD_LIMIT,
		ISE_FORAGE_ADD_RM,
		ISE_FORAGE_NO_RISK,


		UNDEFINED,
		NB_ITEM_SPECIAL_EFFECT = UNDEFINED
	};


	/**
	 * get the right item special effect from the input string
	 * \param str the input string
	 * \return the TItemSpecialEffect associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	TItemSpecialEffect stringToItemSpecialEffect( const std::string& str );

	/**
	 * get the right item special effect from the input string
	 * \param str the input string
	 * \return the TItemSpecialEffect associated to this string (UNDEFINED if the string cannot be interpreted)
	 */
	TItemSpecialEffect fromString( const std::string& str );

	/**
	 * return the item special effect as a string
	 * \param itemSpecialEffect item special effect to transform into a string
	 * \return the item special effect as a string
	 */
	const std::string& toString( TItemSpecialEffect itemSpecialEffect );

}; // ITEM_SPECIAL_EFFECT

#endif // RY_ITEM_SPECIAL_EFFECT_H
/* End of item_special_effect.h */
