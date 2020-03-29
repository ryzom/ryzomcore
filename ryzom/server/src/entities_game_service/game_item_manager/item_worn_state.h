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


#ifndef RYZOM_ITEM_WORN_STATE_H
#define RYZOM_ITEM_WORN_STATE_H

namespace ITEM_WORN_STATE
{
	enum TItemWornState
	{
		Unspoiled = 0,
		WornState1,
		WornState2,
		WornState3,
		WornState4,
		Worned,		
	};


	/**
	 * get item worn state corresponding to input string
	 * \param str the input string
	 * \return the TItemWornState associated to this string (nothing if the string cannot be interpreted)
	 */
	TItemWornState fromString(const std::string &str);

	/**
	 * get the protection type string corresponding to enum
	 * \param type the TProtectionType value
	 * \return type as a string (or nothing)
	 */
	const std::string& toString(TItemWornState state);

	/**
	 * get the chat message to send when an item changes it's state
	 * \param state the worn state of the item
	 */
	const std::string& getMessageForState(TItemWornState state);

}; // ITEM_WORN_STATE

#endif // RYZOM_ITEM_WORN_STATE_H
/* End of item_worn_state.h */
