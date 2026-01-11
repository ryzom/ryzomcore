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
#include "mount_people.h"

using namespace std;
using namespace NLMISC;

namespace MOUNT_PEOPLE
{

	NL_BEGIN_STRING_CONVERSION_TABLE (TMountPeople)
		NL_STRING_CONVERSION_TABLE_ENTRY(Unknown)
		NL_STRING_CONVERSION_TABLE_ENTRY(Fyros)
		NL_STRING_CONVERSION_TABLE_ENTRY(Matis)
		NL_STRING_CONVERSION_TABLE_ENTRY(Tryker)
	NL_END_STRING_CONVERSION_TABLE(TMountPeople, MountPeopleConversion, Zorai)

	TMountPeople fromString(const std::string & str)
	{
		return MountPeopleConversion.fromString(str);
	}

	const std::string & toString(TMountPeople people)
	{
		return MountPeopleConversion.toString(people);
	}

} // namespace MOUNT_PEOPLE
