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



#ifndef CL_CURSOR_FUNCTIONS_H
#define CL_CURSOR_FUNCTIONS_H


/////////////
// INCLUDE //
/////////////
// Misc.
#include "nel/misc/types_nl.h"
// Client.
#include "contextual_cursor.h"


////////////
// GLOBAL //
////////////
// Contextual Cursor.
extern CContextualCursor ContextCur;
extern CLFECOMMON::TCLEntityId	SlotUnderCursor;


///////////////
// FUNCTIONS //
///////////////
/// Initialize Contextual Cursor.
void initContextualCursor();


#endif // CL_CURSOR_FUNCTIONS_H

/* End of cursor_functions.h */
