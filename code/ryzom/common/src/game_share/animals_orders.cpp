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
#include "animals_orders.h"
using namespace std;

namespace ANIMALS_ORDERS
{
// orders table
const std::string  BeastOrders[] =
{
	"follow",
	"stop",
	"free",
	"call",
	"enter_stable",
	"leave_stable",
	"graze",
	"attack",
	"mount",
	"unmount",
};

//-----------------------------------------------
// stringToBeastOrder :
//-----------------------------------------------
EBeastOrder stringToBeastOrder(const std::string &str)
{
	sint i = 0;
	for (i = 0; i< BEAST_ORDERS_SIZE; i++ )
	{
		if ( str == BeastOrders[i] )
			return (EBeastOrder)i;
	}
	return (EBeastOrder)i;
}

//-----------------------------------------------
// stringToBeastOrder :
//-----------------------------------------------
const std::string &creatureSizeToString(EBeastOrder order)
{
	return BeastOrders[order];
}

}; // ANIMALS_ORDERS
