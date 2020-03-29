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



#ifndef ID_IMPULSIONS_H
#define ID_IMPULSIONS_H

#include "fe_types.h"

// misc
#include "nel/misc/types_nl.h"
#include "nel/net/unified_network.h"

/*
 * Message received from identified clients, using CEntityId sender
 */
typedef void	(*TImpulsionIdCb)( const NLMISC::CEntityId& sender, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gamecycle, NLNET::TServiceId serviceId );

void	initImpulsionId();
void	routeImpulsionIdFromClient( NLMISC::CBitMemStream& bms, const NLMISC::CEntityId& sender, const NLMISC::TGameCycle& gamecycle );
void	routeImpulsionIdSlotFromClient( const uint32& targetLootHarvest, const TDataSetRow& targetIndex, const TDataSetRow& senderIndex, const NLMISC::TGameCycle& gamecycle );

#endif // ID_IMPULSIONS_H

/* End of id_impulsions.h */
