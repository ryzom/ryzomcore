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

#include "client_action_type.h"
#include "nel/misc/string_conversion.h"


using namespace NLMISC;

namespace CLIENT_ACTION_TYPE
{

// ***************************************************************************
// The conversion table
const CStringConversion<TClientActionType>::CPair stringTable [] =
{
	{ "None", None },

	{ "Combat", Combat },
	{ "Spell", Spell },
	{ "Faber", Faber },
	{ "Repair", Repair },
	{ "Refine", Refine },
	{ "Memorize", Memorize },
	{ "Forage", Forage },
	{ "Harvest", Harvest },
	{ "Training", Training },
	{ "Tame", Tame },
	{ "Teleport", Teleport },
	{ "Disconnect", Disconnect },
	{ "Mount", Mount },
	{ "Unmount", Unmount },
	{ "ConsumeItem", ConsumeItem },
};


CStringConversion<TClientActionType> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  None);


// ***************************************************************************
const std::string	&toString(TClientActionType e)
{
	return conversion.toString(e);
}

// ***************************************************************************
TClientActionType	fromString(const std::string &s)
{
	return conversion.fromString(s);
}

};
