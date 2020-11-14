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

#include "nel/misc/string_conversion.h"
#include "item_service_type.h"

using namespace std;
using namespace NLMISC;

namespace ITEM_SERVICE_TYPE
{

	NL_BEGIN_STRING_CONVERSION_TABLE (TItemServiceType)
		NL_STRING_CONVERSION_TABLE_ENTRY(StableFeedAnimal1)
		NL_STRING_CONVERSION_TABLE_ENTRY(StableFeedAnimal2)
		NL_STRING_CONVERSION_TABLE_ENTRY(StableFeedAnimal3)
		NL_STRING_CONVERSION_TABLE_ENTRY(StableFeedAnimal4)
		NL_STRING_CONVERSION_TABLE_ENTRY(StableFeedAllAnimals)
		NL_STRING_CONVERSION_TABLE_ENTRY(SpeedUpDPLoss)
	NL_END_STRING_CONVERSION_TABLE(TItemServiceType, ItemServiceTypeConversion, Unknown)

	TItemServiceType fromString(const std::string & str)
	{
		return ItemServiceTypeConversion.fromString(str);
	}

	const std::string & toString(TItemServiceType itemServiceType)
	{
		return ItemServiceTypeConversion.toString(itemServiceType);
	}

} // namespace ITEM_SERVICE_TYPE
