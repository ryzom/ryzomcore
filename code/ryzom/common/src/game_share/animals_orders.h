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



#ifndef RY_ANIMALS_ORDERS_H
#define RY_ANIMALS_ORDERS_H

#include "nel/misc/types_nl.h"



namespace ANIMALS_ORDERS
{
	// orders that can be given to a beast
	enum EBeastOrder
	{
		FOLLOW = 0,
		STOP,
		FREE,
		CALL,
		ENTER_STABLE,
		LEAVE_STABLE,
		GRAZE,			//must be added later
		ATTACK,
		MOUNT,			// For animal of type : Mount
		UNMOUNT,		// For animal of type : Mount

		// the number of size existing
		BEAST_ORDERS_SIZE,
		UNKNOWN_BEAST_ORDER = BEAST_ORDERS_SIZE
	};

	/**
	 * get the EBeastOrder size from the input string
	 * \param str the input string
	 * \return the EBeastOrder associated to this string (UNKNOWN_BEAST_ORDER if the string cannot be interpreted)
	 */
	EBeastOrder stringToBeastOrder(const std::string &str);


	/**
	 * get a EBeastOrder  as a string
	 * \param order is the EBeastOrder
	 * \return the string associated to this EBeastOrder
	 */
	const std::string & stringToBeastOrder(EBeastOrder order);


}; // ANIMALS_ORDERS

#endif // RY_ANIMALS_ORDERS_H

/* End of animals_orders.h */
