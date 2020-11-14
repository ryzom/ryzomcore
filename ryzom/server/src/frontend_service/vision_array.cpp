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

#include "vision_array.h"
#include "client_entity_id_translator.h"
#include "frontend_service.h"

using namespace CLFECOMMON;

/*
 * Constructor
 */
CVisionArray::CVisionArray()
{
	// Reset table
	sint c, e;
	for ( c=0; c!=MAX_NB_CLIENTS+1; ++c )
	{
		for ( e=0; e!=MAX_SEEN_ENTITIES_PER_CLIENT; ++e )
		{
			getPairState( c, e ).resetItem();
			getPairState( c, e ).resetAssociation();
		}
	}
}


/*
 * Return the status of an item (see enum in CClientEntityIdTranslator)
 */
/*
// INLINED
uint8					CVisionArray::getAssociationState( const CClientHost *client, TCLEntityId ceid ) const
{
	if ( ! client->IdTranslator.isUsed( ceid ) )
	{
		return CClientEntityIdTranslator::CEntityInfo::UnusedAssociation;
	}
	else
	{
		return client->IdTranslator.getInfo(ceid).AssociationState;
	}
}
*/


/*
 * Easy and safe access to the client host object
 */
CClientHost				*CVisionArray::clientHost( TClientId clientid )
{
	nlassert( clientid <= MaxNbClients );
	CClientHost *client = CFrontEndService::instance()->sendSub()->clientIdCont()[clientid];
	nlassert( client );
	return client;
}


/* Return the seen entity that has the biggest distance with the client, and the corresponding
 * entity id as an out argument, or NULL if there is no visible entity at all.
 * Takes into account the entites in NormalState or PendingAssociation.
 */
/*CVisionArray::TVAItem	*CVisionArray::currentFurthestSeenEntity( TClientId clientid, TCLEntityId& resultceid )
{
	sint ceid, maxceid;
	TCoord maxdist = 0;
	TVAItem *maxitem = NULL;

	// Find max distance among used items
	for ( ceid=0; ceid!=MAX_SEEN_ENTITIES_PER_CLIENT; ++ceid )
	{
		TVAItem *item = &vaItem( (TClientId)clientid, (TCLEntityId)ceid );
		uint8 state = getAssociationState( (TClientId)clientid, (TCLEntityId)ceid );
		if ( ( state == ClientEntityIdTranslator::CEntityInfo::NormalState ) && ( item->DistanceCE >= maxdist ) )
		{
			maxitem = item;
			maxdist = item->DistanceCE;
			maxceid = ceid;
		}
	}

	// Return max
	resultceid = (TClientId)maxceid;
	return maxitem;
}*/


/*
 * Print the contents of an item, except the properties (debugging)
 */
/*void				CVisionArray::printItem( TClientId clientid, TCLEntityId ceid ) const
{
	const TVAItem& item = vaItem( clientid, ceid );
	stringstream ss;
	ss << endl;
	ss << "****** ClientId: " << clientid << " CeId: " << ceid << endl;
	ss << "Seen entity ind: " << item.entityIndex() << endl;
	ss << "Distance:        " << item.distanceCE() << endl;
	nlinfo( "%s", ss.str().c_str() );
}*/



/*NLMISC_COMMAND( displayVisionPair, "Display pair in the vision array", "<clientid> <slot>" )
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 2) return false;
	
	// get the values
	TClientId clientid;
	NLMISC::fromString(args[0], clientid);
	TCLEntityId slot;
	NLMISC::fromString(args[1], slot);

	if ( (clientid <= MaxNbClients) && (slot < MAX_SEEN_ENTITIES_PER_CLIENT) )
	{
		CFrontEndService::instance()->PrioSub.VisionArray.printItem( clientid, slot );
		return true;
	}
	else
	{
		return false;
	}
}*/

