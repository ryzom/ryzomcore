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
#include "teleport_types.h"

namespace TELEPORT_TYPES
{
	//get the type of a teleport type from a string
	TTeleportType getTpTypeFromString(const std::string & str)
	{
		if (str == "NONE")
			return NONE;
		if (str == "KAMI" || str == "KAMI_TP" )
			return KAMI;
		if (str == "KARAVAN" || str == "KARAVAN_TP" )
			return KARAVAN;
		nlwarning("invalid teleport type");
		return NONE;
	}
}
