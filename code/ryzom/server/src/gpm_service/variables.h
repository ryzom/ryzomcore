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



#ifndef NL_VARIABLES_H
#define NL_VARIABLES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/variable.h"
#include <string>

/// \name Various service Info
//@{
/// Number of entities referenced on the GPMS
extern uint32		NumEntities;
/// Number of players referenced on the GPMS
extern uint32		NumPlayers;
/// Verbose mode
extern bool			Verbose;
//@}


/// \name Player control
//@{
/// Allow check player speed
extern bool			CheckPlayerSpeed;
//@}

#endif // NL_VARIABLES_H

/* End of variables.h */
