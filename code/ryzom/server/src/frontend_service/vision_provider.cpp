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

#include "game_share/action_association.h"

#include "vision_provider.h"
#include "frontend_service.h"
#include "client_host.h"
#include "fe_stat.h"
#include "vision_array.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace CLFECOMMON;


bool verboseVision = false;
extern NLMISC::CMemDisplayer *TmpDebugDisplayer;// = NULL;
NLMISC::CLog		  *TmpDebugLogger = NULL;


/*
 * Constructor
 */
CVisionProvider::CVisionProvider() :
	_VisionArray( NULL ),
	_History( NULL ),
	_EntityToClient( NULL )
{
}


//TEMP
/*static CFileDisplayer fd("addrem.log", true);
static CLog flog;*/


#ifdef MEASURE_FRONTEND_TABLES
sint32 NbX, NbY;
NLMISC_VARIABLE( sint32, NbX, "NbX" );
NLMISC_VARIABLE( sint32, NbY, "NbY" );
#endif


NLMISC_DYNVARIABLE( uint, NbEntitiesSeenByMonitoredClient, "NbEntitiesSeenByMonitoredClient" )
{
	// We can only read the value
	if ( get )
	{
		CFrontEndService *fe = CFrontEndService::instance();
		//nlassert( fe->MonitoredClient <= MAX_NB_CLIENTS );
		CClientHost *client = fe->receiveSub()->clientIdCont()[fe->MonitoredClient];
		if ( client )
		{
			*pointer = MAX_SEEN_ENTITIES_PER_CLIENT - client->NbFreeEntityItems;
			return;
		}
		*pointer = 0;
	}
}


/*
 * Initialization
 */
void				CVisionProvider::init( CVisionArray *va, CHistory *h, CClientIdLookup *cl )
{
	_VisionArray = va;
	_History = h;
	_EntityToClient = cl;
	
	DistanceSpreader.init();
	_VisionReceiver.init();
	//TEMP
	//flog.addDisplayer( &fd );

	if ( ! TmpDebugLogger )
	{
		TmpDebugDisplayer = new CMemDisplayer( "VisionLog" );
		TmpDebugDisplayer->setParam( 2000 );
		TmpDebugLogger = new CLog( CLog::LOG_DEBUG );
		TmpDebugLogger->addDisplayer( TmpDebugDisplayer );
	}
}


/*
 * Destructor
 */
CVisionProvider::~CVisionProvider()
{
	if ( TmpDebugLogger )
	{
		delete TmpDebugLogger;
		delete TmpDebugDisplayer;
	}
}


/*
 * Easy access to the client host object. Returns NULL and makes warning if not found.
 */
CClientHost			*CVisionProvider::clientHost( TClientId clientid )
{
	nlassert( clientid <= MaxNbClients );
	CClientHost *client = CFrontEndService::instance()->sendSub()->clientIdCont()[clientid];
	if ( !client )
		nlwarning( "C%hu not found in cont", clientid );
	return client;
}


/*
 * Process the vision differences
 */
void				CVisionProvider::processVision()
{
	CFrontEndService::instance()->ProcessVisionWatch.start();
	TEntityIndex index;

	/* 1. Process the changes in the vision pairs.
	 * The property receiver says which entities are seen, limited to MAX_SEEN_ENTITIES_PER_CLIENT per observer.
	 */
	{
		H_AUTO(ScanVision);

		/* Multi-pass because the frontend_property_receiver may have received several deltas
		 * that cannot be merged (for example ADD 25 into slot 1, REM 25, ADD 25 into slot 3)
		 * but are stored in a queue.
		 */
		while ( _VisionReceiver.visionChanged() )
		{
			/* TEMP
			 * Begin perf measures for vision processing
			 */
			/*static bool resetM = true;
			if ( resetM )
			{
				IService::getInstance()->requireResetMeasures();
				resetM = false;
			}*/

			index=_VisionReceiver.getFirstUpdatedVision();
			while (index.isValid())
			{
				// Get the description of the entity who sees some changes
				CEntity *observerEntity = TheEntityContainer->getEntity( index );

				// Look-up the corresponding client (player character) connected on this front-end
				TClientId observerClientId = _EntityToClient->getClientId( index );
				if ( observerClientId == INVALID_CLIENT )
				{
					const CEntityId &observerEntityId = TheDataset.getEntityId( index );
#ifdef NL_DEBUG
					nldebug( "%u: Observer E%u not known by this FS...", CTickEventHandler::getGameCycle(), index.getIndex() );
#endif

					observerClientId = TheEntityContainer->EntityToClient->getClientId( observerEntityId );
					if ( observerClientId != INVALID_CLIENT )
					{
						CClientHost *clienthost = CFrontEndService::instance()->sendSub()->clientIdCont()[observerClientId];
						clienthost->setEntityIndex( index );
						TheEntityContainer->EntityToClient->addEntityIndex( index, observerClientId );
						nlinfo( "%u: Setting mirror row to C%hu uid %u E%u (recovered)", CTickEventHandler::getGameCycle(), observerClientId, clienthost->Uid, index.getIndex() );
						CFrontEndService::instance()->receiveSub()->ConnectionStatLog->displayNL( "Setting mirror row to C%hu uid %u E%u", observerClientId, clienthost->Uid, index.getIndex() );
					}
					else
					{
#ifdef NL_DEBUG
						nldebug( "Observer not found" );
#endif
						index = _VisionReceiver.getNextUpdatedVision( index );
						continue;
					}
				}

				// 1. Remove the pairs corresponding to entities not seen anymore
				TSetOfRemovedEntities::iterator iso;
				for ( iso=observerEntity->VisionOut.begin(); iso!=observerEntity->VisionOut.end(); ++iso )
				{
					removePair( observerClientId, *iso );
				}

				// 2. Add or replace the pairs corresponding to entities now seen
				TMapOfVisionAssociations::iterator isi;
				for ( isi=observerEntity->VisionIn.begin(); isi!=observerEntity->VisionIn.end(); ++isi )
				{
					// Add or replace?
					if ( (*isi).second.second )
					{
						replacePair( observerClientId, (*isi).first, (TCLEntityId)((*isi).second.first) );
					}
					else
					{
						addPair( observerClientId, (*isi).first, (TCLEntityId)((*isi).second.first) );
					}
					// Note: we don't add a pair for the symetric association
				}

				index = _VisionReceiver.getNextUpdatedVision( index );
			}

			_VisionReceiver.endUpdatedVision();
		}
	}
	//flog.displayRawNL( "--" );

	/* 2. Calculate new distances for active clients on this front-end.
	 * The entities that are modified were not advertised by getFirstUpdatedProperties() and getNextUpdatedProperties()
	 * because they are considered to be always modified, although their position is up to date in the mirror.
	 */

	/*
	 * Distance processing is spread over several cycles.
	 * DPClientMapIndex is the current index in the client map, DPIterator is the current iterator
	 * Note: the number of clients can increase or decrease between two cycles.
	 */

	if ( DistanceSpreader.mustProcessNow() )
	{
		H_AUTO(UpdateDistances);
		THostMap::iterator icm;
		sint clientmapindex, outerBoundIndex;
		DistanceSpreader.getProcessingBounds( icm, clientmapindex, outerBoundIndex );

		while ( clientmapindex < outerBoundIndex )
		{
			CClientHost*	client = GETCLIENTA(icm);
			TPairState*		state = _VisionArray->getClientStateArray(client->clientId()) + 1;

			// Calculate the distance for all used slots (except slot 0 which always remains at distance 0)
			for ( sint e=1; e!=MAX_SEEN_ENTITIES_PER_CLIENT; ++e, ++state )
			{
				//TPairState&	state = _VisionArray->getPairState(client->clientId(), (TCLEntityId)e);

				//if ( _VisionArray->getAssociationState( client, (TCLEntityId)e ) != CClientEntityIdTranslator::CEntityInfo::UnusedAssociation )	// CHANGED BEN
				if (state->AssociationState != TPairState::UnusedAssociation )
				{
					state->DistanceCE = calcDistance( client, (TCLEntityId)e, state->EntityIndex );
				}
			}
			++clientmapindex;
			++icm;
		}
		DistanceSpreader.endProcessing( icm );
	}

	DistanceSpreader.incCycle();

	CFrontEndService::instance()->ProcessVisionWatch.stop();

	// After having removed the pairs of removed clients, free their ids
	CFrontEndService::instance()->receiveSub()->freeIdsOfRemovedClients();

	/*CFrontEndPropertyReceiver::SEntity *ent = CFrontEndPropertyReceiver::getEntity( 5 );
	nlinfo( "Mileage = %u", ent->Mileage );*/
}


/*
 * Calculate the absolute distance between a client and an entity
 */
inline TCoord	CVisionProvider::calcDistance( CClientHost *client, TCLEntityId slot, const TEntityIndex& seenIndex )
{
	// Indexes are not valid in the case when a client has just resetted, before getting the vision update from the GPMS
	if ( (!client->entityIndex().isValid()) || (!seenIndex.isValid()) )
		return UNSET_DISTANCE;

	// Get client pos (assumes it is given by the GPMS)
	CEntity *clientEntity = TheEntityContainer->getEntity( client->entityIndex() );
	CEntity *seenEntity = TheEntityContainer->getEntity( seenIndex );

	// The mirror properties can be not ready in such a multi-machine case (TODO: check it):
	// Machine A: Service S1 spawns an entity
	// Machine B: GPMS gets the entity and adds it to the vision of a player
	// Machine C: FS gets the vision message
	//            FS gets the entity
	if ( ! seenEntity->X.isReadable() )
	{
		nlwarning( "Multi-machine vision bug with entity E%u", seenIndex.getIndex() );
		return UNSET_DISTANCE;
	}
	if ( ! clientEntity->X.isReadable() )
	{
		nlwarning( "Multi-machine vision bug with client %s", client->eId().toString().c_str() );
		return UNSET_DISTANCE;
	}

	TCoord pos1x = clientEntity->X(), pos1y = clientEntity->Y();
	TCoord pos2x = seenEntity->X(), pos2y = seenEntity->Y();

	// Obsolete: we used to use the position in history (last sent pos). Now it's the official position
	// in order to avoid problem in mode COMBAT_FLOAT...
	// Get absolute entity pos (note: in local mode, the relative pos is not stored in the history)
	//CAction::TValue	vlastposx, vlastposy, vlastposz;
	//bool histohasvalue = _History->getPosition( client->clientId(), slot, vlastposx, vlastposy, vlastposz );
	//if ( histohasvalue )
	//{
	//	pos2x = getAbsoluteCoordinateFrom64( vlastposx );
	//	pos2y = getAbsoluteCoordinateFrom64( vlastposy );
	//	//nlinfo( "lastpos: %d %d, vlastpos: %"NL_I64"u %"NL_I64"u", pos2x, pos2y, vlastposx, vlastposy );
/*#ifdef NL_DEBUG
		TCoord d = (TCoord)(abs(pos1x-pos2x) + abs(pos1y-pos2y));
		if ( d < 0 )
		{
			nlwarning( "Invalid distance calculation with lastsent pos (C%hu S%hu D=%d)", client->clientId(), (uint16)slot, d );
			nlstop;
		}
#endif*/
/*#ifdef NL_DEBUG
		TCoord d = (TCoord)(abs(pos1x-pos2x) + abs(pos1y-pos2y));
		if ( d < 0 )
		{
			nlwarning( "Invalid distance calculation with real pos (C%hu S%hu D=%d)", client->clientId(), (uint16)slot, d );
			nlstop;
		}
#endif*/
	//}

	// Calculate 2D Manhattan distance
	TCoord result = (TCoord)(abs(pos1x-pos2x) + abs(pos1y-pos2y));
	if ( result < 0 )
		result = UNSET_DISTANCE; // correct error if positions are wrong
	return result;
}


/*
 * Add a pair. The argument 'furthestceid' is used when removing an item is required.
 */
bool				CVisionProvider::addPair( TClientId clientid, const TEntityIndex& entityindex, CLFECOMMON::TCLEntityId slot )
{
	CClientHost *client = clientHost(clientid);
	if ( ! client )
		return false;

	// If we have unassociated but no removed the pair yet, complete the removal
	//if ( client->IdTranslator.getInfo(slot).AssociationState != CClientEntityIdTranslator::CEntityInfo::UnusedAssociation )	// CHANGED BEN
	if (_VisionArray->getAssociationState(clientid, slot) != TPairState::UnusedAssociation )
	{
		//LOG_VISION( "FEVIS:%u: Replacing E%u by E%u in slot %hu", CTickEventHandler::getGameCycle(), client->IdTranslator.getInfo(slot).EntityInSlot.getIndex(), entityindex.getIndex(), (uint16)slot );	// CHANGED BEN
		LOG_VISION( "FEVIS:%u: Replacing E%u by E%u in slot %hu", CTickEventHandler::getGameCycle(), _VisionArray->getEntityIndex(clientid, slot).getIndex(), entityindex.getIndex(), (uint16)slot );
		postRemovePair( clientid, slot );
	}

	//TEMP
	//flog.displayRawNL( "ADD C%hu E%u, slot %hu, id %s", clientid, entityindex, slot, CFrontEndPropertyReceiver::getEntity(entityindex)->id.toString().c_str() );

	/* 1. Associate Entity Index to a new TCLEntityId (or get exiting association if this new association occured before a dissociation acknowledge from the client)
	 */

	if ( slot < MAX_SEEN_ENTITIES_PER_CLIENT )
	{
		// Check if the entity is not still in a slot of the client
		TCLEntityId slotFromEntityIndex = client->IdTranslator.getCEId( entityindex );
		if ( slotFromEntityIndex != INVALID_SLOT )
		{
			// The entity was moved to a new slot (previous one unassociated but not removed yet)// The entity was moved to a new slot (previous one unassociated but not removed yet)
			LOG_VISION( "FEVIS: Moving E%u from S%hu to S%hu", entityindex.getIndex(), (uint16)slotFromEntityIndex, (uint16)slot );
			postRemovePair( clientid, slotFromEntityIndex );
		}
		if ( ! client->IdTranslator.acquireCEId( entityindex, (TCLEntityId)slot ) )
		{
			return false; // should not occur (see warning displayed by acquireCEId())
		}
	}
	else
	{
		//nlwarning( "Invalid slot %hu given by GPMS for entity E%u seen by client %hu", slot, entityindex, clientid );
		nlstop;
		return false; // should not occur
	}

	++AssocCounter;
	//CClientEntityIdTranslator::CEntityInfo& info = client->IdTranslator.getInfo((TCLEntityId)slot);	// CHANGED BEN
#ifdef NL_DEBUG
	//info.AssociationState = CClientEntityIdTranslator::CEntityInfo::AwaitingAssAck;	// CHANGED BEN
	_VisionArray->setAssociationState(clientid, slot, TPairState::AwaitingAssAck);
	// NormalAssociation set in CDistancePrioritizer::fillDiscreetProperty() with Sheet
#else
	//info.AssociationState = CClientEntityIdTranslator::CEntityInfo::NormalAssociation;	// CHANGED BEN
	_VisionArray->setAssociationState(clientid, slot, TPairState::NormalAssociation);
#endif

	/* 2. Fill the item attributes
	 */
	LOG_VISION( "FEVIS: %u: Adding pair C%hu -> slot %hu (E%u_%hu)", CTickEventHandler::getGameCycle(), clientid, (uint16)slot, entityindex.getIndex(), entityindex.counter() );
	CEntity *sentity = TheEntityContainer->getEntity( entityindex );
	//nlinfo( "AddPair %hu %hu (E%u)", clientid, (uint16)slot, entityindex );
	_VisionArray->setEntityIndex( clientid, slot, entityindex );
	TPairState& pairState = _VisionArray->getPairState( clientid, slot );
	pairState.associate();
	if ( slot == 0 )
		pairState.DistanceCE = 0; // slot 0's distance is always 0 because we can't use the last sent pos if it is not sent
	else
		pairState.DistanceCE = calcDistance( client, (TCLEntityId)slot, entityindex );
	--client->NbFreeEntityItems;
#ifdef NL_DEBUG
	if ( TheDataset.getEntityId( entityindex ).getType() == RYZOMID::player )
		LOG_VISION( "FEVIS: Seen E%u is a player", entityindex.getIndex() );
#endif

	CFrontEndService::instance()->PrioSub.Prioritizer.addEntitySeenByClient( clientid, slot );

	// Add into history
	//if ( slot != 0 )
	{
		if ( ! _History->addEntityToClient( (TCLEntityId)slot, client->clientId() ) )
		{
			nlwarning( "Cannot add entity to client in property history: entity already used" );
		}
	}

	// Add to observer list: the clients who see the entity (and its ceid for them)
	//_ObserverList[entityindex].insert( TPairClientSlot( clientid, (TCLEntityId)slot ) );

	//client->displayClientProperties();

	return true;
}


/*
 * Remove a pair.
 */
void				CVisionProvider::removePair( TClientId clientid, TCLEntityId slot )
{
	LOG_VISION( "FEVIS:%u: Unassociating C%hu -> slot %hu", CTickEventHandler::getGameCycle(), clientid, (uint16)slot );

	// Prevent a vision bug where two removals would occur (e.g. when the PC is stressed-slow)
	CClientHost *client = clientHost( clientid );
	if ( ! client ) // we had such a situation live once
		return;

	//CClientEntityIdTranslator::CEntityInfo &info = client->IdTranslator.getInfo(slot);	// CHANGED BEN
	//if ( info.AssociationState == CClientEntityIdTranslator::CEntityInfo::UnusedAssociation )	// CHANGED BEN
	if (_VisionArray->getAssociationState(clientid, slot) == TPairState::UnusedAssociation )
	{
		nlwarning( "%u: Cannot remove pair twice (or pair not associated): C%hu S%hu", CTickEventHandler::getGameCycle(), clientid, (uint16)slot );
	}
	else
	{
		TPairState& pairState = _VisionArray->getPairState( clientid, slot );
		pairState.unassociate();
	}
}


/*
 *
 */
void				CVisionProvider::postRemovePair( TClientId clientid, TCLEntityId slot )
{
	++DisasCounter;

	//TEMP
	//flog.displayRawNL( "REM C%hu E%u, id %s", clientid, entityindex, CFrontEndPropertyReceiver::getEntity(entityindex)->id.toString().c_str() );

	nlassert ( (clientid != INVALID_CLIENT) && (slot != INVALID_SLOT) );
	CClientHost *client = clientHost( clientid );
	if ( ! client )
		return;

	//CClientEntityIdTranslator::CEntityInfo &info = client->IdTranslator.getInfo(slot);	// CHANGED BEN
	//TEntityIndex entityindex = info.EntityInSlot;	// CHANGED BEN

	TEntityIndex entityindex = _VisionArray->getEntityIndex(clientid, slot);
	if ( !entityindex.isValid() )
	{
		// No warning when slot 0 because this is normal at the beginning of the life of a client
		if ( slot != 0 )
		{
			nlwarning( "Cannot remove vision pair C%hu -> slot %hu!", clientid, (uint16)slot );
		}
		return;
	}

	CFrontEndService::instance()->PrioSub.Prioritizer.removeEntitySeenByClient( clientid, slot );

	//client->displayClientProperties();

	LOG_VISION( "FEVIS: %u: Removing pair C%hu -> slot %hu (E%u_%hu)", CTickEventHandler::getGameCycle(), clientid, (uint16)slot, entityindex.getIndex(), entityindex.counter() );
	CEntity *sentity = TheEntityContainer->getEntity( entityindex );
#ifdef NL_DEBUG
	if ( TheDataset.getEntityId( entityindex ).getType() == RYZOMID::player )
		LOG_VISION( "FEVIS: Unseen E%u is a player", entityindex.getIndex() );
#endif

	//info.AssociationState = CClientEntityIdTranslator::CEntityInfo::UnusedAssociation;	// CHANGED BEN
	_VisionArray->setAssociationState(clientid, slot, TPairState::UnusedAssociation);
		
	// Remove from history
	//if ( slot != 0 )
		_History->removeEntityOfClient( slot, clientid );

	// Remove from observer list: the clients who see the entity (and its ceid for them)
	//removeFromObserverList( entityindex, clientid, slot );

	// Release Id !
	client->IdTranslator.releaseId( entityindex /*_VisionArray->getEntityIndex( client->clientId(), ceid )*/ );
	++client->NbFreeEntityItems;
	//LOG_VISION( "FEVIS: Client %hu has %hu free items (+)", client->clientId(), client->NbFreeEntityItems );

	// Reset item in the vision array
	TPairState& pairState = _VisionArray->getPairState( clientid, slot );
	pairState.resetItem();
}


/*
 * Replace the entity in a slot (e.g. when transforming a dead character to a sack)
 */
void				CVisionProvider::replacePair( TClientId clientid, const TEntityIndex& newEntityIndex, CLFECOMMON::TCLEntityId slot )
{
	/*
	 * 1. Clear the old entityindex
	 */
	++DisasCounter;

	nlassert ( (clientid != INVALID_CLIENT) && (slot != INVALID_SLOT) );
	CClientHost *client = clientHost( clientid );
	if ( ! client )
		return;

	//CClientEntityIdTranslator::CEntityInfo &info = client->IdTranslator.getInfo(slot);	// CHANGED BEN
	//TEntityIndex entityindex = info.EntityInSlot;	// CHANGED BEN

	TEntityIndex entityindex = _VisionArray->getEntityIndex(clientid, slot);
	if ( !entityindex.isValid() )
	{
		nlwarning( "Cannot replace vision pair C%hu -> slot %hu!", clientid, (uint16)slot );
		return;
	}

	//info.AssociationChannel == CClientEntityIdTranslator::CEntityInfo::NormalAssociation;

	//client->displayClientProperties();

	LOG_VISION( "FEVIS: %u: Replacing pair C%hu -> slot %hu (E%u becomes E%u)", CTickEventHandler::getGameCycle(), clientid, (uint16)slot, entityindex.getIndex(), newEntityIndex.getIndex() );
	CEntity *sentity = TheEntityContainer->getEntity( entityindex );
#ifdef NL_DEBUG
	if ( TheDataset.getEntityId( entityindex ).getType() == RYZOMID::player )
		LOG_VISION( "FEVIS: Unseen E%u is a player", entityindex.getIndex() );
#endif

	// Remove from history
	//if ( slot != 0 )
		_History->removeEntityOfClient( slot, clientid );

	// Remove from observer list: the clients who see the entity (and its ceid for them)
	//removeFromObserverList( entityindex, clientid, slot );

	// Release Id !
	client->IdTranslator.releaseId( entityindex /*_VisionArray->getEntityIndex( client->clientId(), ceid )*/ );
	//LOG_VISION( "FEVIS: Client %hu has %hu free items (+)", client->clientId(), client->NbFreeEntityItems );

	/*
	 * 2. Set the new entityindex
	 */

	++AssocCounter;

	// Associate Entity Index to the slot
	if ( ! client->IdTranslator.acquireCEId( newEntityIndex, (TCLEntityId)slot ) )
	{
		// Should not occur
		return;
	}

	/* 2. Fill the item attributes
	 */
	//LOG_VISION( "FEVIS: Adding pair C%hu -> slot %hu (E%u) (replace)", clientid, slot, entityindex );
	sentity = TheEntityContainer->getEntity( newEntityIndex );
	//nlinfo( "AddPair %hu %hu (E%u)", clientid, (uint16)slot, entityindex );
	TPairState& pairState = _VisionArray->getPairState( clientid, slot );
	pairState.changeAssociation();
	pairState.resetPrio();
	pairState.EntityIndex = newEntityIndex;
	if ( slot == 0 )
		pairState.DistanceCE = 0;
	else
		pairState.DistanceCE = calcDistance( client, (TCLEntityId)slot, newEntityIndex );
#ifdef NL_DEBUG
	if ( TheDataset.getEntityId( newEntityIndex ).getType() == RYZOMID::player )
		LOG_VISION( "FEVIS: Seen E%u is a player", newEntityIndex.getIndex() );
#endif

	// Add into history
	//if ( slot != 0 )
	{
		if ( ! _History->addEntityToClient( (TCLEntityId)slot, clientid ) )
		{
			nlwarning( "Cannot add entity to client in property history: entity already used" );
		}
	}

	// Add to observer list: the clients who see the entity (and its ceid for them)
	//_ObserverList[newEntityIndex].insert( TPairClientSlot( clientid, (TCLEntityId)slot ) );

}


/*
 * Remove item from observer list of entityindex
 */
/*void				CVisionProvider::removeFromObserverList( const TEntityIndex& entityindex, TClientId clientid, TCLEntityId ceid )
{
	_ObserverList[entityindex].erase( TPairClientSlot( clientid, ceid ) );
}*/


/*
 * Reset all slots of all clients (e.g. when GPMS falls down)
 */
void				CVisionProvider::resetVision()
{
	nlinfo( "Resetting all slots of all clients..." );
	THostMap& clientmap = CFrontEndService::instance()->receiveSub()->clientMap();
	THostMap::iterator ihm;
	for ( ihm=clientmap.begin(); ihm!=clientmap.end(); ++ihm )
	{
		resetVision( GETCLIENTA(ihm) );
	}
}


/*
 * Reset all slots of one client (e.g. when a client is unspawned)
 */
void				CVisionProvider::resetVision( CClientHost *clienthost )
{
	// Get all entities seen by this client
	sint e;
	for ( e=0; e!=MAX_SEEN_ENTITIES_PER_CLIENT; ++e )
	{
		//if ( _VisionArray->getAssociationState( clienthost, (TCLEntityId)e ) != CClientEntityIdTranslator::CEntityInfo::UnusedAssociation )	// CHANGED BEN
		if ( _VisionArray->getAssociationState( clienthost->clientId(), (TCLEntityId)e ) != TPairState::UnusedAssociation )
		{
			// Remove pair on the FE and on the client
			removePair( clienthost->clientId(), (TCLEntityId)e );
		}
	}
}


/*
 * Display the properties of an entity in the vision
 */
void				CVisionProvider::displayEntityInfo( const CEntity& e, const TEntityIndex& entityIndex, NLMISC::CLog *log ) const
{
	if ( ! e.X.isReadable() )
	{
		log->displayNL( "Entity %u not initialized", entityIndex.getIndex() );
		return;
	}

	const CEntityId& eid = TheDataset.getEntityId( entityIndex );
	uint64 properties[NB_VISUAL_PROPERTIES];
	CEntity::fillVisualPropertiesFromMirror( properties, entityIndex );
	string sheetIdS = CSheetId((uint32)properties[PROPERTY_SHEET]).toString();
	log->displayNL( "E%u %s Id %s Name %s Sheet %u (%s) GameCycle %u",
			entityIndex.getIndex(), (eid.getType()==RYZOMID::player)?"PLAYER":RYZOMID::toString( (RYZOMID::TTypeId)eid.getType() ).c_str(), eid.toString().c_str(), getEntityName(entityIndex).c_str(), (uint32)properties[PROPERTY_SHEET], sheetIdS.c_str(), e.TickPosition() );
	log->displayNL( "%u entities entered vision, %u entities left vision", e.VisionIn.size(), e.VisionOut.size() );
	log->displayNL( "Position (m): %d %d %d - Local: %d %d %d - Mode: %s", e.posXm(entityIndex), e.posYm(entityIndex), e.posZm(entityIndex), e.posLocalXm(entityIndex), e.posLocalYm(entityIndex), e.posLocalZm(entityIndex), (properties[PROPERTY_POSZ]&0x1)?"Relative":"Absolute" );
	e.displayProperties( entityIndex, log );
//	stringstream ss;
	string str;
	str += NLMISC::toString(e.propIsInitializedState(0)) + "''"; // skip 1 & 2
//	ss << e.propIsInitializedState(0) << "''"; // skip 1 & 2
											   /*for ( sint p=0; p!=NB_VISUAL_PROPERTIES; ++p )
	{
		ss << " " << e.properties[p];
	}
	nlinfo( "Property values: %s", ss.str().c_str() );
	ss.clear();*/
	for ( sint p=3; p!=NB_VISUAL_PROPERTIES; ++p )
	{
		//ss << e.propIsInitializedState(p);
		str += NLMISC::toString(e.propIsInitializedState(p));
		if ( (p+1) % 4 == 0 )
			//ss << "-";
			str += "-";
	}
	log->displayNL( "Initialized: %s", str.c_str() );
}


/*
 * Reset assoc/disas counters
 */
void					CVisionProvider::resetAssocCounter()
{
	AssocCounter = 0;
	DisasCounter = 0;
	AssocStartTime = CTime::getLocalTime();
}


/*
 * Display assoc/disas freqs
 */
void					CVisionProvider::displayAssocFreqs(CLog *log)
{
	float duration = ((float)(CTime::getLocalTime()-AssocStartTime))/1000.0f;
	log->displayNL( "Assoc: %.1f Hz - Disac: %.1f Hz", ((float)AssocCounter)/duration, ((float)DisasCounter)/duration);
}


NLMISC_COMMAND( displayEntityInfo, "Display the properties of an entity", "<entityIndex>" )
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 1) return false;
	
	// get the values
	TDataSetIndex entityIndex;
	NLMISC::fromString(args[0], entityIndex);
	if ( entityIndex < (uint32)TheDataset.maxNbRows() )
	{
		TDataSetRow datasetrow = TheDataset.getCurrentDataSetRow( entityIndex );
		CEntity *entity = TheEntityContainer->getEntity( datasetrow );
		if ( ! TheDataset.getEntityId( datasetrow ).isUnknownId() ) // TODO: unknown or 0 ??
		{
			CFrontEndService::instance()->PrioSub.VisionProvider.displayEntityInfo( *entity, datasetrow );
			return true;
		}
	}
	log.displayNL( "There is no such an entity index" );
	return true;
}


NLMISC_COMMAND( displayEntityInfoById, "Display the properties of an entity, by entity id (if found)", "<entityId>" )
{
	if (args.size() < 1)
		return false;

	CEntityId	eid;
	uint64		id;
	uint		type;
	uint		creatorId;
	uint		dynamicId;
	if (sscanf(args[0].c_str(), "(%"NL_I64"x:%x:%x:%x)", &id, &type, &creatorId, &dynamicId) != 4)
		return false;
	eid.setShortId( id );
	eid.setType( type );
	eid.setCreatorId( creatorId );
	eid.setDynamicId( dynamicId );

	TEntityIndex entityIndex = TheEntityContainer->entityIdToIndex( eid );
	if ( entityIndex.isValid() )
	{
		CEntity* entity = TheEntityContainer->getEntity( entityIndex );
		if ( ! TheDataset.getEntityId( entityIndex ).isUnknownId() ) // TODO: unknown or 0 ??
		{
			CFrontEndService::instance()->PrioSub.VisionProvider.displayEntityInfo( *entity, entityIndex );
			return true;
		}
	}
	log.displayNL( "There is no entity with the specified id" );
	return true;
}


NLMISC_COMMAND( displaySlotInfo, "Display info for a particular slot of a client", "<clientid> <slot>" )
{
	if ( args.size() < 2 )
		return false;
	
	TClientId clientid;
	NLMISC::fromString(args[0], clientid);
	CLFECOMMON::TCLEntityId slot;
	NLMISC::fromString(args[1], slot);
	CClientHost *clienthost;
	if ( (clientid <= MaxNbClients) && ((clienthost = CFrontEndService::instance()->sendSub()->clientIdCont()[clientid]) != NULL) )
	{
		clienthost->displaySlotProperties( slot, true, &log );
	}
	return true;
}

NLMISC_COMMAND( resetAssocCounters, "Reset assoc/disas counters", "" )
{
	CFrontEndService::instance()->PrioSub.VisionProvider.resetAssocCounter();
	return true;
}

NLMISC_COMMAND( displayAssocFreqs, "Display assoc/disas freqs", "" )
{
	CFrontEndService::instance()->PrioSub.VisionProvider.displayAssocFreqs(&log);
	return true;
}

/*NLMISC_COMMAND( verboseVision, "Turn on/off or check the state of verbose logging of vision", "" )
{
	if ( args.size() == 1 )
	{
		if ( args[0] == string("on") || args[0] == string("1") )
			verboseVision=true;
		else if ( args[0] == string("off") || args[0] == string("0") )
			verboseVision=false;
	}

	log.displayNL( "verboseVision is %s", verboseVision ? "on" : "off" );
	return true;
}*/


NLMISC_COMMAND( displayVisionLog, "Display Tmp Debug Log", "" )
{
	TmpDebugDisplayer->write( &log );
	return true;
}
