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



#ifndef NL_EGS_MIRROR_H
#define NL_EGS_MIRROR_H

#include "game_share/mirror.h"
#include "game_share/ryzom_mirror_properties.h"


extern CMirror				Mirror;
extern CMirroredDataSet		*FeTempDataset;
extern CMirroredDataSet		*FameDataset;
extern TPropertyIndex		DSFirstPropertyAvailableImpulseBitSize;

extern uint32				MaxNbPlayers, MaxNbObjects, MaxNbNpcSpawnedByEGS, MaxNbForageSources, MaxNbToxicClouds;
extern uint32				MaxNbGuilds;


/*
 * Initialisation of the mirror
 */
void	initMirror( void (*cbUpdate)(), void (*cbSync)() );

/*
 * Initialisation of the properties in the mirror when the mirror service is up
 */
void	cbMirrorIsReadyForInit( CMirror *mirror );

/*
 * Callback called when mirror is ready for manage entity
 */
void	cbMirrorReadyForAddEntity( CMirror *mirror );

/*
 * Clear any target lists from previous uncleared sessions
 */
void	cleanOrphanTargetLists( NLNET::TServiceId serviceId );


#define TheDataset (*FeTempDataset)
#define TheFameDataset (*FameDataset)


void processMirrorUpdates();

/// get an entityId from a mirro data set row
inline NLMISC::CEntityId getEntityIdFromRow( const TDataSetRow & rowId )
{
	if ( TheDataset.isAccessible(rowId) )
		return TheDataset.getEntityId( rowId );
	return NLMISC::CEntityId::Unknown;
}

#endif // NL_EGS_MIRROR_H

/* End of egs_mirror.h */
