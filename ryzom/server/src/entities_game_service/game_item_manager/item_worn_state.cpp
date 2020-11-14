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

#include "nel/misc/debug.h"
#include "item_worn_state.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace ITEM_WORN_STATE
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TItemWornState)
		NL_STRING_CONVERSION_TABLE_ENTRY(Unspoiled)
		NL_STRING_CONVERSION_TABLE_ENTRY(WornState1)
		NL_STRING_CONVERSION_TABLE_ENTRY(WornState2)
		NL_STRING_CONVERSION_TABLE_ENTRY(WornState3)
		NL_STRING_CONVERSION_TABLE_ENTRY(WornState4)
		NL_STRING_CONVERSION_TABLE_ENTRY(Worned)
	NL_END_STRING_CONVERSION_TABLE(TItemWornState, Conversion, Unspoiled)
		
	
	//-----------------------------------------------
	// fromString:
	//-----------------------------------------------
	TItemWornState fromString(const std::string &str)
	{
		return Conversion.fromString(str);
	}


	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const std::string& toString(TItemWornState state)
	{
		return Conversion.toString(state);
	}


	string stateMsgs[] =
	{
		"ITEM_UNSPOILED",
		"ITEM_WORN_STATE_1",
		"ITEM_WORN_STATE_2",
		"ITEM_WORN_STATE_3",
		"ITEM_WORN_STATE_4",
		"ITEM_WORNED"	
	};

	const std::string& getMessageForState(TItemWornState state)
	{
		return stateMsgs[state];
	}

}; // ITEM_WORN_STATE
