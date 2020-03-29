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


#ifndef COMMON_PD_H
#define COMMON_PD_H

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <vector>
#include <map>

// User #includes

namespace EGSPD
{
	
//
// Forward declarations
//



//
// Typedefs & Enums
//

/** TCharacterId
 * defined at entities_game_service\pd_scripts\egs_pd.pds:9
 */
typedef NLMISC::CEntityId TCharacterId;


/** TGuildId
 * defined at entities_game_service\pd_scripts\egs_pd.pds:10
 */
typedef uint32 TGuildId;


	
} // End of EGSPD


//
// Inline implementations
//

#include "common_pd_inline.h"

#endif
