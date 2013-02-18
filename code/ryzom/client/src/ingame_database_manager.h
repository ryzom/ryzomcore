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




#ifndef CL_INGAME_DATABASE_MANAGER_H
#define CL_INGAME_DATABASE_MANAGER_H


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
// Game share.
#include "nel/misc/cdb.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/cdb_branch.h"
#include "nel/misc/cdb_branch_observing_handler.h"
#include "cdb_synchronised.h"



////////////
// EXTERN //
////////////
extern CCDBSynchronised IngameDbMngr;


#endif // CL_INGAME_DATABASE_MANAGER_H

/* End of ingame_database_manager.h */
