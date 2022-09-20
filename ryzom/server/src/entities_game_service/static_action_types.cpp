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
#include "static_action_types.h"
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace STATIC_ACT_TYPES
{

NL_BEGIN_STRING_CONVERSION_TABLE (TStaticActTypes)
NL_STRING_CONVERSION_TABLE_ENTRY(Neutral)
NL_STRING_CONVERSION_TABLE_ENTRY(Casting)
NL_STRING_CONVERSION_TABLE_ENTRY(Forage)
NL_STRING_CONVERSION_TABLE_ENTRY(BotChat)
NL_STRING_CONVERSION_TABLE_ENTRY(Teleport)
NL_STRING_CONVERSION_TABLE_ENTRY(Mount)
NL_STRING_CONVERSION_TABLE_ENTRY(Unknown)
NL_END_STRING_CONVERSION_TABLE(TStaticActTypes, StaticActTypesConversion, Unknown)


//-----------------------------------------------
// toSBrickFamily :
//-----------------------------------------------
TStaticActTypes StaticActTypes(const std::string &str)
{
	return StaticActTypesConversion.fromString(str);
} // toBrickFamily //


//-----------------------------------------------
// toString :
//-----------------------------------------------
const std::string &toString(TStaticActTypes type)
{
	return StaticActTypesConversion.toString(type);
} // toString //

}
