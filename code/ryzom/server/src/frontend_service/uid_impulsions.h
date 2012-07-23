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



#ifndef UID_IMPULSIONS_H
#define UID_IMPULSIONS_H


// misc
#include "nel/misc/types_nl.h"

/*
 * Message received from not yet identified clients (uid, for User Id)
 */
typedef void	(*TImpulsionUidCb)(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle);


void	initImpulsionUid();
void	routeImpulsionUidFromClient( NLMISC::CBitMemStream& bms, const uint32& userId, const NLMISC::TGameCycle& gamecycle );


#endif // UID_IMPULSIONS_H

/* End of uid_impulsions.h */
