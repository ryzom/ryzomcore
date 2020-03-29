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



#ifndef RY_STATS_STATUS_H
#define RY_STATS_STATUS_H

#include "nel/misc/types_nl.h"

namespace STATS_STATUS
{
	// Size
	enum EStatsStatus
	{
		NORMAL = 0,
		UNBUFFED,
		BUFFED,

		// the number of size existing
		NB_STATS_STAUS,
		UNKNOWN,
	};
}; // STATS_STATUS

#endif // RY_STATS_STATUS_H
/* End of file stats_status.h */
