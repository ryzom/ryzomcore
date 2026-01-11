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

#include <nel/misc/types_nl.h>
#include <nel/misc/command.h>
#include <nel/misc/variable.h>

#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>

#include "pds_database.h"

#include "pd_lib/timestamp.h"

using namespace std;
using namespace NLMISC;



NLMISC_COMMAND(testStamp, "", "")
{
	if (args.size() != 1)
		return false;

	CTimestamp	stamp;

	stamp.fromString(args[0].c_str());

	return true;
}

