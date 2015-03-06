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
#include "mirror.h"
#include "synchronised_message.h"
#include "tick_proxy_time_measure.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

#ifdef NL_OS_WINDOWS
#    ifdef FAST_MIRROR
#        pragma message(NL_LOC_MSG "Using **** FAST_MIRROR ****")
#    else
#    	 pragma message(NL_LOC_MSG "Not using FAST_MIRROR")
#    endif
#endif // NL_OS_WINDOWS


const string MirrorVersion = string("1.10-")+string(ListRowSizeString); // ADDED: Unidirectional Mode (don't wait for delta)

CMirror *MirrorInstance = NULL;

extern NLMISC::TTime TimeBeforeTickUpdate;

extern CVariable<sint32> TotalSpeedLoop;

extern CVariable<sint16> ProcessMirrorUpdatesSpeed;
extern CVariable<sint16> ReceiveMessagesViaMirrorSpeed;

uint32 TimeInMirrorCallback = 0;
uint32 TotalMirrorCallbackCalled = 0;

CVariable<uint32>	MirrorCallbackCount("ms", "MirrorCallbackCount", "Mirror synchronised callback counter per update", 0, 100);
CVariable<uint32>	MirrorCallbackTime("ms", "MirrorCallbackTime", "Mirror synchronised callback time per tick (in ms)", 0, 100);


CMirroredDataSet *TNDataSets::InvalidDataSet; // not used because expection thrown instead

struct CEntityAndPropName
{
	CEntityId	EntityId;
	string		PropName;
};

const std::string WatchNotUpdatedStr = "not updated yet";

static std::string	MWATCH0=WatchNotUpdatedStr,MWATCH1=WatchNotUpdatedStr,
					MWATCH2=WatchNotUpdatedStr,MWATCH3=WatchNotUpdatedStr,
					MWATCH4=WatchNotUpdatedStr,MWATCH5=WatchNotUpdatedStr,
					MWATCH6=WatchNotUpdatedStr,MWATCH7=WatchNotUpdatedStr,
					MWATCH8=WatchNotUpdatedStr,MWATCH9=WatchNotUpdatedStr;
static std::string *mWatchStrings[]=
{
	&MWATCH0,&MWATCH1,&MWATCH2,&MWATCH3,&MWATCH4,
	&MWATCH5,&MWATCH6,&MWATCH7,&MWATCH8,&MWATCH9
};
static CEntityAndPropName *WatchedPropValues[sizeof(mWatchStrings)/sizeof(mWatchStrings[0])]=
{
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL
};

NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH0, "watch string 0");
NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH1, "watch string 1");
NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH2, "watch string 2");
NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH3, "watch string 3");
NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH4, "watch string 4");
NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH5, "watch string 5");
NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH6, "watch string 6");
NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH7, "watch string 7");
NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH8, "watch string 8");
NLMISC_CATEGORISED_VARIABLE(mirror, string, MWATCH9, "watch string 9");


/*
 * Callback called at MS start-up
 */
void cbMSUpDn( const string& /* serviceName */, TServiceId serviceId, void *upOrDn )
{
	MirrorInstance->detectLocalMS( serviceId, (bool)(upOrDn!=NULL) );
}


/*
 * Callback when any service up or down is detected
 */
void cbAnyServiceUpDn( const string& serviceName, TServiceId serviceId, void *vEventType )
{
	// here we have an integer value stored in the 'void*' vEventType and we need to convert back to an integer
	// for this we use pointer arithmetic with char* pointers in order to avoid compiler warnings
	MirrorInstance->processServiceEvent( serviceName, serviceId, (CMirror::TServiceEventType)((char*)vEventType-(char*)NULL) );
}


/*
 * Callback called when any service (the mirror of which is ready) is detected
 *
 * A service broadcasts the SMIRUB message when its mirror becomes ready (up), so that
 * other services can react to the new service (for example by sending messages to it).
 * If a service B is launched after a service A is ready, B will not receive SMIRUB.
 * That's why A has got to send a SMIRUR (reply) to B to tell that its mirror is up,
 * when receiving the SMIRUB (broadcast) from B.
 */
void cbServiceMirrorUpForSMIRUB( const string& /* serviceName */, TServiceId serviceId, void * )
{
	// If the incoming service is connected after us and we already have mirror ready, tell it
	if ( MirrorInstance->mirrorIsReady() )
	{
		CMessage msgout( "SMIRUR" ); // 'Service has MIRror system Up Reply'
		MirrorInstance->pushEntityRanges( msgout );
		sendMessageViaMirror( serviceId, msgout ); // via mirror to ensure that MARCS was sent to all remote MS before their client services get SMIRU (otherwise their potential messages couldn't be routed through their MS)
	}
	// else "SMIRU" will be broadcasted to all when mirror gets ready
}


/*
 * Callback when receiving the first game cycle
 */
void	cbSyncGameCycle()
{
	if ( ! MirrorInstance->mirrorIsReady() )
	{
		MirrorInstance->testMirrorReadyStatus();
	}

	MirrorInstance->userSyncCallback();
}


/*
 * Declare the current service as an owner of the entities of the specified entity type.
 * It means the current service will create (at most maxNbEntities entities) and remove
 * this kind of entities (it can remove only the ones it has created) in all the datasets
 * corresponding to the entity type.
 * Another service can be a co-owner of the same entity type, but it will create and
 * remove other entity ids.
 */
void	CMirror::declareEntityTypeOwner( uint8 entityTypeId, sint32 maxNbEntities )
{
	// Ask the local mirror for a range of maxNbEntities in the datasets corresponding
	// to entityTypeId
	SEND_MSG_2P_TO_MIRROR( DET, entityTypeId, maxNbEntities );
	MIRROR_INFO( "MIRROR: Declaring %d max entities of type %hu", maxNbEntities, (uint16)entityTypeId );

	TEntitiesCreatedInEntityType newEntityTypeOwned( maxNbEntities );

	// Set the datasetlist
	TNDataSets::iterator ids;
	for ( ids=_NDataSets.begin(); ids!=_NDataSets.end(); ++ids )
	{
		/*vector<uint8>::iterator it;
		for ( it=GET_NDATASET(ids)._EntityTypesFilter.begin(); it!=GET_NDATASET(ids)._EntityTypesFilter.end(); ++it )
		{
			nldebug( "%d", (*it) );
		}*/
		if ( find( GET_NDATASET(ids)._EntityTypesFilter.begin(), GET_NDATASET(ids)._EntityTypesFilter.end(), entityTypeId ) != GET_NDATASET(ids)._EntityTypesFilter.end() )
		{
			newEntityTypeOwned.DataSetList.push_back( &GET_NDATASET(ids) );
		}
	}

	if ( ! _EntityTypesOwned.insert( make_pair( entityTypeId, newEntityTypeOwned ) ).second )
	{
		nlwarning( "MIRROR: Cannot declare owning twice the same entity type" );
	}

	++_PendingEntityTypesRanges;
}


/*
 * Declare the current service as a user of the entities of the specified entity type.
 * It means the current service wants to be receive the corresponding entities. It will
 * be notified about their additions and removals.
 * If more than one service is an owner of a particular entity type, declaring or not
 * as a user the same entity type will specify if the service wants to receive or not
 * the entities created by other owners; of course the service is always aware of the
 * entities it creates, although the mirror system does not notify them to itself.
 */
/*void	CMirror::declareEntityTypeUser( uint8 entityTypeId )
{

}*/


/*
 * Note: there is not undeclaration of the current service as an owner of the entities
 * of the specified entity type. This is done automatically when the service shuts down.
 */


/*
 * RG
 */
void	CMirror::receiveRangesForEntityType( CMessage& msgin )
{
	// Get "entitytype { <dataset, first, last> } and map to entity type for add/removeEntity
	string datasetName;
	TDataSetIndex first, last;

	TServiceId declaratorServiceId;
	msgin.serial( declaratorServiceId );
	//nlassert( declaratorServiceId == IService::getServiceId() );

	uint8 entityTypeId;
	msgin.serial( entityTypeId );
	TEntityTypesOwned::iterator ito = _EntityTypesOwned.find( entityTypeId );

	uint8 nbDatasets;
	msgin.serial( nbDatasets );
	for ( sint i=0; i!=(sint)nbDatasets; ++i )
	{
		msgin.serial( datasetName );
		msgin.serial( first, last );
		if ( first != INVALID_DATASET_INDEX )
		{
			TNDataSets::iterator ids = _NDataSets.find( datasetName );
			if ( ids != _NDataSets.end() )
			{
				// Fill range manager
				const TEntityRange& entityRange = GET_NDATASET(ids).addEntityTypeRange( entityTypeId, first, last );

				// Fill homogeneous information about ranges
				GET_NDATASET(ids).addDeclaredEntityRange( entityTypeId, entityRange, declaratorServiceId );

				// Now, it's already done in declareEntityTypesOwned()
				/*if ( ito != _EntityTypesOwned.end() )
				{
					GET_ENTITY_TYPE_OWNED(ito).DataSetList.push_back( &GET_NDATASET(ids) );
				}*/
				nlassert( find( GET_ENTITY_TYPE_OWNED(ito).DataSetList.begin(), GET_ENTITY_TYPE_OWNED(ito).DataSetList.end(), &GET_NDATASET(ids) ) != GET_ENTITY_TYPE_OWNED(ito).DataSetList.end() );
			}
		}
		else
		{
			string errorStr;
			msgin.serial( errorStr );
			nlerror( "MIRROR: Could not acquire range for row management of entity type %hu in dataset %s. Reason:\n%s", (uint16)entityTypeId, datasetName.c_str(), errorStr.c_str() );
		}
	}

	--_PendingEntityTypesRanges;

	testMirrorReadyStatus();

	/*
	Note:
	An entity type can have several datasets (ex: player has 'common', 'playercharacteristics')
	A dataset can have several entity types owned (ex: dataset 'common' with player, npc)
	*/
}


/*
 * Add an entity into the mirror
 */
bool	CMirror::addEntity(bool fillEntityId, NLMISC::CEntityId& entityId, bool declare )
{
	// Find the structure of entities created for the entity type
	TEntityTypesOwned::iterator ito = _EntityTypesOwned.find( entityId.getType() );
	if ( (ito != _EntityTypesOwned.end()) &&
		(GET_ENTITY_TYPE_OWNED(ito).CurrentNb < GET_ENTITY_TYPE_OWNED(ito).MaxNb) )
	{
		// For each dataset, add entity
		TDataSetList& datasetList = GET_ENTITY_TYPE_OWNED(ito).DataSetList;
		if ( datasetList.empty() )
		{
			nlwarning( "MIRROR: No dataset to add entity %s (check if the %s is online and has good paths)", entityId.toString().c_str(), RANGE_MANAGER_SERVICE.c_str() );
		}
		bool allOk = true;
		uint nbAdded = 0;
		TDataSetList::iterator idsl;
		for ( idsl=datasetList.begin(); idsl!=datasetList.end(); ++idsl )
		{
			uint res = (*idsl)->addEntityToDataSet(fillEntityId, entityId, declare );
			if ( res == 0 )
				allOk = false;
			nbAdded += res;
			// in any case, only set the entityId from the first dataset
			fillEntityId = false;
		}
		if ( nbAdded != 0 )
			++(GET_ENTITY_TYPE_OWNED(ito).CurrentNb);
		return allOk;
	}
	else
	{
		nlwarning( "MIRROR: Cannot add entity %s", entityId.toString().c_str() );
		return false;
	}
}




/*
 * Add an entity into the mirror. The caller must be an owner of the corresponding
 * entity type and it must not create more entities than maxNbEntities (see
 * declareEntityTypeOwner()). The entity will be added into all the datasets
 * corresponding to the entity type (possibly with a different row in each one).
 *
 * The entity will not be declared to all services until you call declareEntity(),
 * either in CMirror (global) or in CMirroredDataSet (per dataset).
 *
 * Returns false if part or all of the process failed (for example, if the entity
 * already exists in a dataset).
 */
bool	CMirror::createEntity( NLMISC::CEntityId& entityId, bool fillEntityId )
{
	return addEntity( fillEntityId, entityId, false );
}

/*
 * Declare the entity (previously added by createEntity()) in all datasets
 * concerned by its entity type.
 *
 * Returns false in case of failure.
 *
 * \seealso CMirroredDataSet::declareEntity() for alternate version.
 */
bool	CMirror::declareEntity( const NLMISC::CEntityId& entityId )
{
	// Find the structure of entities created for the entity type
	TEntityTypesOwned::iterator ito = _EntityTypesOwned.find( entityId.getType() );
	if ( ito != _EntityTypesOwned.end() )
	{
		// For each dataset, declare entity
		TDataSetList& datasetList = GET_ENTITY_TYPE_OWNED(ito).DataSetList;
#ifdef NL_DEBUG
		if ( datasetList.empty() )
		{
			nlwarning( "MIRROR: No dataset to add entity %s (check if the %s is online and has good paths)", entityId.toString().c_str(), RANGE_MANAGER_SERVICE.c_str() );
			return false;
		}
#endif
		TDataSetList::iterator idsl;
		for ( idsl=datasetList.begin(); idsl!=datasetList.end(); ++idsl )
		{
			TDataSetRow entityIndex = (*idsl)->getDataSetRow( entityId );
			if ( entityIndex.isValid() )
			{
				(*idsl)->declareEntity( entityIndex );
			}
		}
		return true;
	}
	else
	{
#ifdef NL_DEBUG
		nlwarning( "MIRROR: Cannot declare entity %s", entityId.toString().c_str() );
#endif
		return false;
	}
}


/*
 * Remove an entity from the mirror. The caller must be an owner of the corresponding
 * entity type (see declareEntityTypeOwner()) and it must have added the specified
 * entity before (see addEntity()).
 *
 * Returns false if part or all of the process failed.
 */
bool	CMirror::removeEntity( const CEntityId& entityId )
{
	// Find the structure of entities created for the entity type
	TEntityTypesOwned::iterator ito = _EntityTypesOwned.find( entityId.getType() );
	if ( (ito != _EntityTypesOwned.end()) &&
		 (GET_ENTITY_TYPE_OWNED(ito).CurrentNb > 0 ) )
	{
		// For each dataset, ask for removal of entity (which will be deferred)
		TDataSetList& datasetList = GET_ENTITY_TYPE_OWNED(ito).DataSetList;
		uint nbRemoved = 0;
		bool allOk = true;
		TDataSetList::iterator idsl;
		for ( idsl=datasetList.begin(); idsl!=datasetList.end(); ++idsl )
		{
			uint res = (*idsl)->removeEntityFromDataSet( entityId );
			if ( res == 0 )
				allOk = false;
			nbRemoved += res;
		}
		if ( nbRemoved != 0 ) // fix
			--(GET_ENTITY_TYPE_OWNED(ito).CurrentNb);
		return allOk;
	}
	else
	{
		nlwarning( "MIRROR: Cannot remove entity %s", entityId.toString().c_str() );
		return false;
	}
}


/*
 *
 */
void	CMirror::receiveSMIdToAccessPropertySegment( NLNET::CMessage& msgin )
{
	// Read info from local Mirror Service
	string propName;
	sint32 smid;
	uint32 dataTypeSize;
	msgin.serial( propName );
	msgin.serial( smid );
	msgin.serial( dataTypeSize );

	void *pt = _PropAllocator.accessPropertySegment( propName, smid );
	setPropertyInfo( propName, pt, dataTypeSize );

	testMirrorReadyStatus();
}


/*
 * Set the segment pointer in the property container
 */
void				CMirror::setPropertyInfo( std::string& propName, void *segmentPt, uint32 dataTypeSize )
{
	TPropertiesInMirror::iterator ipm = _PropAllocator._PropertiesInMirror.find( propName );
	if ( ipm != _PropAllocator._PropertiesInMirror.end() )
	{
		TPropertyInfo& propInfo = GET_PROPERTY_INFO(ipm);
		CMirroredDataSet *ds = propInfo.DataSet;
		if ( ds )
		{
#ifdef NL_DEBUG
			if ( propInfo.PropertyIndex != INVALID_PROPERTY_INDEX ) // avoid entity ids
			{
				// Check data type size consistency between mirror client and mirror service
				uint32 clientDataSize = ds->getDataSizeOfProp( propInfo.PropertyIndex );
				if ( clientDataSize != dataTypeSize )
				{
					nlwarning( "MIRROR: %s/P%hd (%s): data size %u does not match with the size on the local MS (%u)", ds->name().c_str(), propInfo.PropertyIndex, propName.c_str(), clientDataSize, dataTypeSize );
				}
			}
#endif
			// Set pointers
			ds->setPropertyPointer( propName, propInfo.PropertyIndex, segmentPt, false,
									dataTypeSize, propInfo.flagReadOnly(), propInfo.flagMonitorAssignment() );
		}
	}
}


/*
 * Access segments for non-subscribed properties allocated on the local MS (useful for cleaning a whole row when creating an entity)
 * Added: V1.8
 *
 * Justification:
 * --------------
 * Example: Let say one machine holds two services A and B. Another machine holds a service C.
 * A is the owner of a dataset range (it will create entities in the dataset).
 * B is the writer of a property P (it will init and write P values after A has created an entity).
 * C is the reader of P.
 * A must know the segment for P, even if it does not subscribe to P. Indeed, when A creates an entity E,
 * it has to clean the row E for all the properties allocated on the machine.
 * If A did not clean E.P, B would not always have a blank E.P (but C would).
 * Then if B wrote E.P=V when it detected the creation of E, when the row E would be
 * reassigned the new assignment E.P=V would overwrite an existing same V => the value would
 * not be emitted to the other machine and C would remain with its 0 value in E.P.
 *
 * Note:
 * -----
 * Whenever a service starts, after declaring the properties it subscribes, it is sent by the local MS
 * the list of other properties, to access the segments. The service won't start until the list is
 * received, so that it never creates entity without cleaning all the properties, even the non-subscribes
 * ones.
 * Whenever another service starts, our service is sent by the local MS the new properties the other
 * service uses. Our service may create some entities before the message is received, but it is not
 * a problem because, as the non-subscribed properties are newly created, they can't have non-null
 * content. Our service can remove some entities, but it can recreate an entity in the same raw before
 * the message is received, because a row reassignment is blocked for sufficient time (usually 100 game
 * cycles).
 */
void				CMirror::applyListOfOtherProperties( CMessage& msgin, uint nbProps, const char *msgName, bool isInitial )
{
	for ( uint32 i=0; i!=nbProps; ++i )
	{
		uint32 sheet;
		msgin.serial( sheet );
		CSheetId sheetId( sheet );
		TSDataSets::iterator isd = _SDataSets.find( sheetId );
		TPropertyIndex propIndex;
		sint32 smid;
		uint32 dataTypeSize;
		msgin.serial( propIndex );
		msgin.serial( smid );
		msgin.serial( dataTypeSize );
		if ( isd != _SDataSets.end() )
		{
			// Set pointers from smid if the property is not already subscribed
			if ( GET_SDATASET(isd)._PropertyContainer.PropertyValueArrays[propIndex].Values == NULL )
			{
				void *segmentPt = _PropAllocator.accessOtherPropertySegment( smid );
				string emptyPropName;
				GET_SDATASET(isd).setPropertyPointer( emptyPropName, propIndex, segmentPt, false, dataTypeSize, false, false ); // not readonly because killList needs write access (checked in debug mode) in CMirroredDataSet::addEntityToDataSet()
				MIRROR_DEBUG( "MIRROR: Added other property for P%hd smid %d", propIndex, smid );
			}
		}
		else
		{
			nlwarning( "Invalid dataset %s in %s", sheetId.toString().c_str(), msgName );
		}
	}

	if ( isInitial )
	{
		_ListOfOtherPropertiesReceived = true;
		testMirrorReadyStatus();
	}
}


/*
 *
 */
bool				CMirror::datasetMatchesEntityTypesOwned( const CMirroredDataSet& dataset ) const
{
	TEntityTypesOwned::const_iterator ieto;
	for ( ieto=_EntityTypesOwned.begin(); ieto!=_EntityTypesOwned.end(); ++ieto )
	{
		const TDataSetList& dslist = GET_ENTITY_TYPE_OWNED(ieto).DataSetList;
		TDataSetList::const_iterator idsl;
		if ( find( dslist.begin(), dslist.end(), &dataset ) != dslist.end() )
		{
			return true;
		}
	}
	return false;
}


/*
 * Receive trackers (access or ignore them)
 */
void				CMirror::receiveTracker( bool entitiesOrProp, NLNET::CMessage& msgin )
{
	uint8 nb;
	string name;
	msgin.serial( name ); // for entities (ATE): dataset name; for properties (ATP): property name
	msgin.serial( nb );
	static uint nbSent = 0;

	if ( entitiesOrProp )
	{
		MIRROR_DEBUG( "MIRROR:ROWMGT:ATE> Receiving %hu trackers for rows of %s", (uint16)nb, name.c_str() );
		try
		{
			CMirroredDataSet& dataset = _NDataSets[name];

			for ( uint i=0; i!=nb; ++i )
			{
				bool self; // self <=> tracker for reading. !self <=> tracker for writing
				sint32 smidAdd, smidRemove;
				msgin.serial( self );
				msgin.serial( smidAdd );
				msgin.serial( smidRemove );
				nlassert( smidAdd != InvalidSMId );
				nlassert( smidRemove != InvalidSMId );
				sint32 mutidAdd, mutidRemove;
				msgin.serial( mutidAdd );
				msgin.serial( mutidRemove );
				nlassert( mutidAdd != InvalidSMId ); // same ids as smids
				nlassert( mutidRemove != InvalidSMId );
				TServiceId serviceIdTracker;
				msgin.serial( serviceIdTracker );
				bool isInitialTransmitTracker;
				msgin.serial( isInitialTransmitTracker );

				// Ignore trackers corresponding to datasets in which we can't create any entities
				// (the entity types owned were declared at the same time as when sending the subscribe
				// message 'DATASETS' to the MS, i.e. when receiving the trackers the entity types
				// owned are necessarily set).
				bool trackerAdded = false;
				if ( self || datasetMatchesEntityTypesOwned( dataset ) )
				{
					// Add entity management tracker
					dataset.accessEntityTrackers( self, smidAdd, smidRemove, mutidAdd, mutidRemove );
					trackerAdded = true;
				}

				CMessage msgout;

				// If the tracker belongs to a local service, reply to it, otherwise reply to the local MS.
				// Note: if the tracker belongs to a local service who just exited, the local MS will have to detect it and do nothing.
				// Note: corresponds to (!self) && CUnifiedNetwork::getInstance()->getServiceName( serviceIdTracker ).empty()
				TServiceId destServiceId = ( CUnifiedNetwork::getInstance()->isServiceLocal( serviceIdTracker ) ? serviceIdTracker : localMSId() );
				TServiceId from = IService::getInstance()->getServiceId();
				if ( (! self) && trackerAdded )
				{
					// Send acknowledge of "add tracker for entities", because the destination service
					// could have added some entities before we got the tracker, so it has to send
					// its entity list to us for sync
					msgout.setType( "FWD_ACKATE" ); // sent via MS to ensure ordering on the dest service!
				}
				else
				{
					msgout.setType( "FWD_ACKATE_FCO" ); // "for counter only"
				}
				msgout.serial( destServiceId );
				msgout.serial( from );
				msgout.serial( isInitialTransmitTracker );
				msgout.serial( name ); // dataset name
				msgout.serial( smidAdd );
				dataset.serialOutOwnedRanges( msgout ); // required at least for the local case
				// If there is no owned range, no entity can have been added (yet)
				// but we still have to send the message, in order to unblock the tracker
				CUnifiedNetwork::getInstance()->send( localMSId(), msgout );
				MIRROR_INFO( "MIRROR:ROWMGT:ATE> Sent %s to %hu for %s, tracker of %hu",
					msgout.getName().c_str(),
					destServiceId.get(),
					name.c_str(),
					serviceIdTracker.get() );
				++nbSent;
			}
		}
		catch(const EMirror& )
		{
			nlwarning( "MIRROR:ROWMGT:ATE> Invalid dataset name %s for adding tracker", name.c_str() );
		}
	}
	else
	{
		MIRROR_DEBUG( "MIRROR:PROPMGT:ATP> Receiving %hu trackers for property %s", (uint16)nb, name.c_str() );
		TPropertyIndex propIndex;
		CMirroredDataSet *ds = getDataSetByPropName( name, propIndex );
		if ( ds )
		{
			// Reset PropTrackersPending for the property
			TPropertiesInMirror::iterator ipm = _PropAllocator._PropertiesInMirror.find( name );
			nlassert( ipm != _PropAllocator._PropertiesInMirror.end() );
			GET_PROPERTY_INFO(ipm).PropTrackersPending = false;

			// Access the property trackers
			for ( uint i=0; i!=nb; ++i )
			{
				bool self; // self <=> tracker for reading. !self <=> tracker for writing
				sint32 smid;
				sint32 mutid;
				TServiceId serviceIdTracker;
				msgin.serial( self );
				msgin.serial( smid );
				msgin.serial( mutid );
				msgin.serial( serviceIdTracker );
				// Add property tracker (if allocated)
				if ( smid != InvalidSMId )
				{
					ds->accessNewPropTracker( self, propIndex, smid, mutid );

					if ( ! self )
					{
						// Send acknowledge of "add tracker for property changes"

						// If the tracker belongs to a local service, reply to it, otherwise reply to the local MS.
						// Note: if the tracker belongs to a local service who just exited, the local MS will have to detect it and do nothing.
						// Note: corresponds to CUnifiedNetwork::getInstance()->getServiceName( serviceIdTracker ).empty()
						TServiceId destServiceId = ( CUnifiedNetwork::getInstance()->isServiceLocal( serviceIdTracker ) ? serviceIdTracker : localMSId() );
						TServiceId from = IService::getInstance()->getServiceId();
						CMessage msgout( "FWD_ACKATP" );
						msgout.serial( destServiceId ); // sent via MS to ensure ordering on the dest service!
						msgout.serial( from );
						msgout.serial( name ); // property name
						msgout.serial( smid );
						CUnifiedNetwork::getInstance()->send( localMSId(), msgout );
					}
					else
					{
						CMessage msgout( "FWD_ACKATP_FCO" ); // "for counter only"
						CUnifiedNetwork::getInstance()->send( localMSId(), msgout );
					}
				}
				else
				{
					CMessage msgout( "FWD_ACKATP_FCO" ); // "for counter only"
					CUnifiedNetwork::getInstance()->send( localMSId(), msgout );
					MIRROR_DEBUG( "MIRROR: \t(tracker is disabled)" );
				}
				++nbSent;
				//nldebug( "Sent FWD properties %u", nbSent );
			}
		}
		else
		{
			nlwarning( "MIRROR:PROPMGT:ATP> Invalid property name %s for adding tracker", name.c_str() );
		}
	}

	testMirrorReadyStatus();
}


/*
 * Scan for existing entities in the received ranges
 */
void				CMirror::receiveAcknowledgeAddEntityTracker( NLNET::CMessage& msgin, TServiceId serviceId )
{
	bool isInitialTransmitTracker;
	string datasetname;
	msgin.serial( isInitialTransmitTracker ); // not used here, only by MS
	msgin.serial( datasetname );
	vector<TServiceId8> blankIgnoreList;
	try
	{
		CMirroredDataSet& dataset = _NDataSets[datasetname];

		// Fill new service's EntityTrackerAdding with the current used dataset rows (not already removed),
		// so that it will be notified of the entities that exist already, in the range of the sender
		if ( dataset._PropertyContainer.EntityIdArray.EntityIds && dataset._SelfEntityTrackers[ADDING].isAllocated() ) // only if already allocated
		{
			uint32 nbRanges, smid;
			msgin.serial( smid, nbRanges );
			for ( uint i=0; i!=nbRanges; ++i )
			{
				TDataSetIndex first, last;
				msgin.serial( first, last );
				uint nbAdded = dataset.scanAndResyncExistingEntities( first, last+1, false, blankIgnoreList );
				MIRROR_INFO( "MIRROR:ROWMGT:ACKATE> AddingTracker (smid %d): filled %u entities in %s from new %s, range [%d..%d]", dataset._SelfEntityTrackers[ADDING].smid(), nbAdded, dataset.name().c_str(), CUnifiedNetwork::getInstance()->getServiceUnifiedName(serviceId).c_str(), first, last );
			}
			if ( nbRanges == 0 )
			{
				MIRROR_INFO( "MIRROR:ROWMGT:ACKATE> AddingTracker (smid %d): no entity to add", smid );
			}
		}
		else
		{
			// WORKAROUND:
			//   If it occurs when starting, it's not a problem because SC_EI will follow
			//   (AND SC_EI must scan also entities from local services)
			// TODO:
			//   Handle this case, and set SC_EI back to non-local entities only
			MIRROR_INFO( "MIRROR:ROWMGT:ACKATE> %s not allocated yet while receiving tracker ack", dataset._PropertyContainer.EntityIdArray.EntityIds?"Entity tracker":"Entity Id storage" );
		}

	}
	catch(const EMirror& )
	{
		nlwarning( "MIRROR: Invalid dataset name %s for receiving ack of addEntityTracker", datasetname.c_str() );
	}
}


/*
 * Scan for non-null property values
 */
void				CMirror::receiveAcknowledgeAddPropTracker( NLNET::CMessage& msgin, TServiceId serviceId )
{
	string propName;
	// uint32 smid;
	msgin.serial( propName );
	//msgin.serial( smid );
	TPropertyIndex propIndex;
	CMirroredDataSet *ds = getDataSetByPropName( propName, propIndex );
	if ( ds )
	{
		// Fill tracker with non-zero values
		if ( ds->_PropertyContainer.PropertyValueArrays[propIndex].Values ) // only if already allocated
		{
			// Find self prop tracker for the specified property
			CChangeTrackerClientProp *tracker = ds->getSelfPropTracker( propIndex );
			if ( ! tracker )
			{
				//nlwarning( "PROPMGT: Tracker not found for property %s in %s", propName.c_str(), ds->name().c_str() );
				MIRROR_INFO( "MIRROR:PROPMGT:ACKATP> Tracker for prop %s not received yet while receiving tracker ack (normal only if pointing to group)!", propName.c_str() );
			}
			else if ( tracker->isAllocated() )
			{
				uint nbChanges = ds->scanAndResyncProp( propIndex, tracker, serviceId );
				MIRROR_INFO( "MIRROR:PROPMGT:ACKATP> Tracker (smid %d): filled %u changes for %s/P%hd from new %s", tracker->smid(), nbChanges, ds->name().c_str(), propIndex, CUnifiedNetwork::getInstance()->getServiceUnifiedName(serviceId).c_str() );
				// Note: this will lead to unjustified changes! More are better than not enough!
			}
			else
			{
				// Should not occur since ack forwarded by the MS
				nlwarning( "MIRROR:PROPMGT:ACKATP> Tracker not allocated while receiving tracker ack for prop %s!!", propName.c_str() );
			}
		}
		else
		{
			// Should not occur since ack forwarded by the MS
			nlwarning( "MIRROR:PROPMGT:ACKATP> Values not ready for %s/P%hd!!", ds->name().c_str(), propIndex );
		}
	}
	else
	{
		nlwarning( "MIRROR:PROPMGT:ACKATP> Invalid property name %s for receiving ack of AddPropTracker", propName.c_str() );
	}
}


/*
 *
 */
void				CMirror::deleteTracker( CChangeTrackerClient& tracker, std::vector<CChangeTrackerClient>& vect )
{
	MIRROR_DEBUG( "MIRROR: Releasing tracker with smid %d", tracker.smid() );

	// Release the tracker
	if ( ! tracker.isPointingToGroupTracker() )
		tracker.release();

	// Delete the tracker object
	tracker.~CChangeTrackerClient();
	tracker = vect.back();
	vect.pop_back();
}


/*
 * Can handle case of tracker not known
 */
void				CMirror::releaseTrackers( NLNET::CMessage& msgin )
{
	vector<sint32> smidsToFindAndRemove;
	msgin.serialCont( smidsToFindAndRemove );
#ifdef NL_DEBUG
	vector<sint32>::iterator istfar;
	for ( istfar=smidsToFindAndRemove.begin(); istfar!=smidsToFindAndRemove.end(); ++istfar )
	{
		if ( (*istfar) != InvalidSMId )
		{
			MIRROR_INFO( "MIRROR: Need to remove tracker with smid %d", (*istfar) );
		}
	}
#endif

	// Browse datasets
	TSDataSets::iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		CMirroredDataSet& dataset = GET_SDATASET(ids);

		sint32 eti;
		for ( eti=REMOVING; eti>=ADDING; --eti )
		{
			for (uint i = 0 ; i < dataset._EntityTrackers[eti].size(); )
			{
				if ( find( smidsToFindAndRemove.begin(), smidsToFindAndRemove.end(), dataset._EntityTrackers[eti][i].smid() ) != smidsToFindAndRemove.end() )
					deleteTracker( dataset._EntityTrackers[eti][i], dataset._EntityTrackers[eti] );
				else
					++i; // deleteTracker() moves the last entry to the deleted place
			}
/*			ace: deleting an entry in a vector invalidates all iterators
			TTrackerListForEnt::iterator itl;
			for ( itl=dataset._EntityTrackers[eti].begin(); itl<dataset._EntityTrackers[eti].end(); )
			{
				if ( find( smidsToFindAndRemove.begin(), smidsToFindAndRemove.end(), (*itl).smid() ) != smidsToFindAndRemove.end() )
					deleteTracker( (*itl), dataset._EntityTrackers[eti] );
				else
					++itl; // deleteTracker() moves the last entry to the deleted place
			}
*/
		}

		TPropertyIndex propIndex;
		for ( propIndex=0; propIndex!= dataset.nbProperties(); ++propIndex )
		{
			// TODO: Skip properties not declared
			for (uint i=0; i < dataset._PropTrackers[propIndex].size(); )
			{
				if ( find( smidsToFindAndRemove.begin(), smidsToFindAndRemove.end(), dataset._PropTrackers[propIndex][i].smid() ) != smidsToFindAndRemove.end() )
					deleteTracker( dataset._PropTrackers[propIndex][i], dataset._PropTrackers[propIndex] );
				else
					++i; // deleteTracker() moves the last entry to the deleted place
			}
/*			ace: deleting an entry in a vector invalidates all iterators
			TTrackerListForProp::iterator itp;
			for ( itp=dataset._PropTrackers[propIndex].begin(); itp<dataset._PropTrackers[propIndex].end(); )
			{
				if ( find( smidsToFindAndRemove.begin(), smidsToFindAndRemove.end(), (*itp).smid() ) != smidsToFindAndRemove.end() )
					deleteTracker( (*itp), dataset._PropTrackers[propIndex] );
				else
					++itp; // deleteTracker() moves the last entry to the deleted place
			}
*/
		}
	}
}


/*
 *
 */
void				CMirror::scanAndResyncEntitiesExceptIgnored( const std::vector<NLNET::TServiceId8>& creatorIdsToIgnore )
{
	// Open self entity trackers
	for ( TSDataSets::iterator itd=_SDataSets.begin(); itd!=_SDataSets.end(); ++itd )
	{
		CMirroredDataSet& dataset = GET_SDATASET(itd);
		dataset.enableEntityNotification();

		// Here, we could clear the entity trackers, so that the first notification session
		// would report only the entities filled by scanAndResyncExistingEntities() below.
		// But it would necessary to block the MS or any other service to write in it.
		// Letting the trackers as they are is not harmful. Reasons:
		// First
		// - An online entity not in the tracker will be rescanned and added, thus notified
		//   Ex(*): an entity created by S1 before S2 connected
		// - An online entity in the adding tracker will be rescanned thus notified (only once, it can't be added twice)
		//   Ex(*): an entity created by S1 after MS2 added S2's entity trackers
		// - An offline entity in the adding tracker will not be notified ("lifetime too short")
		//   Ex(*): an entity added by S1 after MS2 added S2's entity trackers, removed since
		//   Ex(**):an entity added by S1 before S2 first connection, removed before now
		// Then (removing notification MUST come after adding notification!)
		// - An offline entity in the removg tracker will not be notified ("lifetime too short") because it's addition not notified so not in the entity map
		//   Ex(*): an entity added by S1 after MS2 added S2's entity trackers, removed since
		// - An online entity in the removg tracker would be notified if its addition notification has added it into the entityId map
		//   It can't occur, because a row can't be reassigned before a minimal delay (100 gc) greater than the delay needed to receive SC_EI (10 gc).
		// (*) S1 MS1 MS2 S2+-+ Example on S2 at second connection
		// (**)S1 MS1 MS2 S2+   Example on S2 at first connection
	}

	// Prepare to execute "mirror ready commands" (note: they will be executed when receiving UMM from the MS for proper synchronization) and tell other services we are ready
	testMirrorReadyStatus();
	nlassert( _MirrorGotReadyLevel2 );

	// Add existing entities
	for ( TSDataSets::iterator itd=_SDataSets.begin(); itd!=_SDataSets.end(); ++itd )
	{
		CMirroredDataSet& dataset = GET_SDATASET(itd);
		uint nbAdded = dataset.scanAndResyncExistingEntities( 0, dataset.maxNbRows(), false, creatorIdsToIgnore );
		MIRROR_INFO( "MIRROR:ROWMGT:SC_EI> Filled %u entities in %s, with %u creators ignored", nbAdded, dataset.name().c_str(), creatorIdsToIgnore.size() );
	}
}


/*
 * Scan the entities to find if some are missing (and add them) (debug feature)
 */
void				CMirror::rescanExistingEntities( CMirroredDataSet& dataset, NLMISC::CLog& log, bool addUnknown )
{
	if ( addUnknown )
	{
		vector<TServiceId8> blankIgnoreList;
		uint nbAdded = dataset.scanAndResyncExistingEntities( 0, dataset.maxNbRows(), true, blankIgnoreList );
		log.displayNL( "MIRROR:ROWMGT: Filled %u entities in %s!", nbAdded, dataset.name().c_str() );
	}
	else
	{
		log.displayNL( "Online entities not known by this service:" );
		dataset.displayUnknownEntities( 0, dataset.maxNbRows(), log );
	}
}


/*
 * Callback for msg receive from mirror service: receive shared memory id and access memory
 */
void cbRecvSMIdToAccessPropertySegment( NLNET::CMessage& msgin, const std::string &/* serviceName */, TServiceId /* serviceId */ )
{
	MirrorInstance->receiveSMIdToAccessPropertySegment( msgin );
}


/*
 *
 */
void cbRecvRangesForEntityType( NLNET::CMessage& msgin, const std::string &/* serviceName */, TServiceId /* serviceId */ )
{
	MirrorInstance->receiveRangesForEntityType( msgin );
}


/*
 *
 */
void cbRecvAddEntityTracker( NLNET::CMessage& msgin, const std::string &/* serviceName */, TServiceId /* serviceId */ )
{
	MirrorInstance->receiveTracker( true, msgin );
}


/*
 *
 */
void cbRecvAddPropTracker( NLNET::CMessage& msgin, const std::string &/* serviceName */, TServiceId /* serviceId */ )
{
	MirrorInstance->receiveTracker( false, msgin );
}


/*
 *
 */
void cbRecvAcknowledgeAddEntityTracker( NLNET::CMessage& msgin, const std::string &/* serviceName */, TServiceId /* serviceId */ )
{
	TServiceId from;
	msgin.serial( from );
	MirrorInstance->receiveAcknowledgeAddEntityTracker( msgin, from );
}


/*
 *
 */
void cbRecvAcknowledgeAddPropTracker( NLNET::CMessage& msgin, const std::string &/* serviceName */, TServiceId /* serviceId */ )
{
	TServiceId from;
	msgin.serial( from );
	MirrorInstance->receiveAcknowledgeAddPropTracker( msgin, from );
}


/*
 *
 */
void cbReleaseTrackers( NLNET::CMessage& msgin, const std::string &/* serviceName */, TServiceId /* serviceId */ )
{
	MirrorInstance->releaseTrackers( msgin );
}


/*
 *
 */
void cbAllMirrorsOnline( NLNET::CMessage& msgin, const std::string &/* serviceName */, TServiceId /* serviceId */ )
{
	sint16 nbOfMirrors;
	string versionStr = "prior to 1.5";
	bool hasVersion = false;
	msgin.serial( nbOfMirrors );
	try
	{
		msgin.serial( versionStr );
		hasVersion = true;
	}
	catch (const EStreamOverflow&)
	{}
	if ( (! hasVersion) || (MirrorVersion != versionStr) )
		nlerror( "Mirror version mismatch! This service: %s; Local MS: %s", MirrorVersion.c_str(), versionStr.c_str() );
	MirrorInstance->initAfterMirrorUp();
}


/*
 * Receive the broadcasting of a service that has its mirror system ready
 */
void cbRecvServiceHasMirrorReadyBroadcast( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	cbServiceMirrorUpForSMIRUB( serviceName, serviceId, 0 );

	MirrorInstance->receiveServiceHasMirrorReady( serviceName, serviceId, msgin );
}


/*
 * Receive the reply of a SMIRUB
 */
void cbRecvServiceHasMirrorReadyReply( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MirrorInstance->receiveServiceHasMirrorReady( serviceName, serviceId, msgin );
}


/*
 * Set service mirror/up/down callback
 */
void CMirror::setServiceMUDCallback( const string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg, bool back, bool synchronizedCallback, TServiceEventType et )
{
	nlassert( cb != NULL );

	TCallbackArgItemM cbItem;
	cbItem.Cb = cb;
	cbItem.Arg = arg;
	cbItem.Synchronized = synchronizedCallback;
	cbItem.EventType = et;

	if ( serviceName == "*" )
	{
		if ( back )
			_ServiceUpDnUniCallbacks.push_back( cbItem );
		else
			_ServiceUpDnUniCallbacks.insert( _ServiceUpDnUniCallbacks.begin(), cbItem );
	}
	else
	{
		if ( back )
			_ServiceUpDnCallbacks[serviceName].push_back( cbItem );
		else
			_ServiceUpDnCallbacks[serviceName].insert( _ServiceUpDnCallbacks[serviceName].begin(), cbItem );
	}
}


/*
 * Set callbacks to call when a particular service has its mirror system ready. See .h
 */
void	CMirror::setServiceMirrorUpCallback( const string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg, bool back, bool synchronizedCallback )
{
	setServiceMUDCallback( serviceName, cb, arg, back, synchronizedCallback, ETMirrorUp );
}


/*
 * This is the counterpart of setServiceMirrorUpCallback(). See .h
 */
void	CMirror::setServiceMirrorDownCallback( const std::string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg, bool back, bool synchronizedCallback )
{
	setServiceMUDCallback( serviceName, cb, arg, back, synchronizedCallback, ETMirrorDn );
}


/*
 * This is similar to CUnifiedNetwork::setServiceUpCallback() but it allows you to spawn or despawn
 * entities in the callback if you set synchronizedCallback to true (see isRunningSynchronizedCode())
 */
void	CMirror::setServiceUpCallback( const std::string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg, bool back, bool synchronizedCallback )
{
	setServiceMUDCallback( serviceName, cb, arg, back, synchronizedCallback, ETServiceUp );
}


/*
 * This is similar to CUnifiedNetwork::setServiceUpCallback() but it allows you to spawn or despawn
 * entities in the callback if you set synchronizedCallback to true (see isRunningSynchronizedCode())
 */
void	CMirror::setServiceDownCallback( const std::string &serviceName, NLNET::TUnifiedNetCallback cb, void *arg, bool back, bool synchronizedCallback )
{
	setServiceMUDCallback( serviceName, cb, arg, back, synchronizedCallback, ETServiceDn );
}


/*
 * Receive the broadcasting of a service that has its mirror system ready
 */
void	CMirror::receiveServiceHasMirrorReady( const std::string& serviceName, TServiceId serviceId, NLNET::CMessage& msgin )
{
	if ( _ServicesMirrorUpNotified.find( serviceId ) != _ServicesMirrorUpNotified.end() )
	{
		// This occurs when A and B cross-send a SMIRUB msg to each other at the same time
		// (then A and B send SMIRUR, making A and B come here)
		//MIRROR_DEBUG( "MIRROR: Ignoring second ServiceHasMirrorReady from %s-%hu", serviceName.c_str(), serviceId.get() );
		return;
	}
	MIRROR_INFO( "MIRROR: Service %s-%hu has mirror ready", serviceName.c_str(), serviceId.get() );

	// Read the entity ranges declared by the sender service
	try
	{
		uint32 nbDatasets;
		msgin.serial( nbDatasets );
		for ( uint32 i=0; i!=nbDatasets; ++i )
		{
			CSheetId dsSheetId;
			msgin.serial( dsSheetId );

			TEntityRangeOfType entityRanges;
			msgin.serialCont( entityRanges );

			TSDataSets::iterator it = _SDataSets.find( dsSheetId );
			if ( it != _SDataSets.end() )
			{
				CMirroredDataSet& dataset = (*it).second;
				dataset.addDeclaredEntityRanges( entityRanges, serviceId );
			}
		}
	}
	catch (const EStreamOverflow&)
	{
		nlwarning( "Received SMIRU from old version service %s-%hu", serviceName.c_str(), serviceId.get() );
	}

	// Callbacks
	processServiceEvent( serviceName, serviceId, ETMirrorUp );
}


/*
 * React to a service mirror/up/down event
 */
void	CMirror::processServiceEvent( const std::string &serviceName, TServiceId serviceId, TServiceEventType et )
{
	switch ( et )
	{
	case ETMirrorUp:
		{
			// Store which 'service mirror up' are known
			_ServicesMirrorUpNotified.insert( serviceId );
			break;
		}
	case ETServiceDn:
		{
			// Check if a 'service down' must call a 'service mirror down' callback as well
			TNotifiedServices::iterator isn = _ServicesMirrorUpNotified.find( serviceId );
			if ( isn != _ServicesMirrorUpNotified.end() )
			{
				processServiceEvent( serviceName, serviceId, ETMirrorDn );
				_ServicesMirrorUpNotified.erase( isn );
			}
			break;
		}
	default:;
	}

	// Callbacks
	TNameMappedCallbackM::iterator it = _ServiceUpDnCallbacks.find( serviceName );
	if ( it != _ServiceUpDnCallbacks.end() )
	{
		// For MirrorUp event requested, call them, if queue them if flagged as synchronized
		for ( list<TCallbackArgItemM>::const_iterator it2=(*it).second.begin(); it2!=(*it).second.end(); ++it2 )
		{
			const TCallbackArgItemM& cbItem = (*it2);
			if ( cbItem.EventType != et )
				continue;

			if ( cbItem.Synchronized )
			{
				_SynchronizedServiceUpDnCallbackQueue.push_back( TCallbackArgItemMExt( cbItem, serviceName, serviceId ) );
			}
			else
			{
				if ( cbItem.Cb )
					cbItem.Cb( serviceName, serviceId, cbItem.Arg );
			}
		}
	}

	// For *
	for ( uint c=0; c!=_ServiceUpDnUniCallbacks.size(); ++c )
	{
		const TCallbackArgItemM& cbItem = _ServiceUpDnUniCallbacks[c];
		if ( cbItem.EventType != et )
			continue;

		if ( cbItem.Synchronized )
		{
			_SynchronizedServiceUpDnCallbackQueue.push_back( TCallbackArgItemMExt( cbItem, serviceName, serviceId ) );
		}
		else
		{
			if ( cbItem.Cb != NULL )
				cbItem.Cb( serviceName, serviceId, cbItem.Arg );
		}
	}
}


/*
 * Return true if the specified service is known as "mirror ready"
 */
bool	CMirror::serviceHasMirrorReady( NLNET::TServiceId serviceId ) const
{
	return (_ServicesMirrorUpNotified.find( serviceId ) != _ServicesMirrorUpNotified.end());
}


/*
 * Return true if the service reported by cbServiceDown("MS") or cbServiceUp("MS") is the local Mirror Service
 */
bool	CMirror::detectLocalMS( TServiceId serviceId, bool upOrDown )
{
	/* // OLD CODE
	TSockId hostid;
	CCallbackNetBase *cnb = CUnifiedNetwork::getInstance()->getNetBase( (TServiceId)serviceId, hostid );
	if ( cnb && (cnb->hostAddress( hostid ).ipAddress() == CInetAddress::localHost().ipAddress()) )*/

	if ( CUnifiedNetwork::getInstance()->isServiceLocal( serviceId ) )
	{
		if ( upOrDown )
		{
			_PropAllocator.setLocalMirrorServiceId( serviceId );
			_AwaitingAllMirrorsOnline = true;

			// TODO: if MS resync supported, reset closure state back (see below)
		}
		else
		{
			_PropAllocator.setLocalMirrorServiceId( TServiceId::InvalidId );
			nlwarning( "MIRROR: Local Mirror Service %hu is down! (%s)", serviceId.get(), CInetAddress::localHost().ipAddress().c_str() );

			// Allow closure, otherwise the service won't quit (no more UMM message received)
			// However, the tickReleaseFunc (if provided) must be called.
			if ( _TickReleaseFunc )
			{
				_IsExecutingSynchronizedCode = true; // prevent assertion failure
				_TickReleaseFunc(); // no need of synchronization... the MS is down
				_IsExecutingSynchronizedCode = false;
			}
			IService::getInstance()->clearForClosure();
			IService::getInstance()->exit(); // we're not supporting MS resync at the moment => quit now
		}
		return true;
	}
	return false;
}


/*
 * Info: display the properties allocated on the mirror
 */
void	CMirror::displayProperties( NLMISC::CLog& log ) const
{
	TPropertiesInMirror::const_iterator ipim;
	log.displayNL( "Allocated properties:" );
	for ( ipim=_PropAllocator._PropertiesInMirror.begin(); ipim!=_PropAllocator._PropertiesInMirror.end(); ++ipim )
	{
		if ( GET_PROPERTY_INFO(ipim).Segment )
		{
			log.displayNL( "%s: %p", (*ipim).first.c_str(), GET_PROPERTY_INFO(ipim).Segment );
		}
	}

	TSDataSets::const_iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		log.displayNL( "* DataSet %s (sheetid %s) *", GET_SDATASET(ids).name().c_str(), GET_SDATASET(ids).sheetId().toString().c_str() );
		log.displayNL( "  EntityId: %p", GET_SDATASET(ids)._PropertyContainer.EntityIdArray.EntityIds );
		log.displayNL( "  Properties:" );
		for ( uint i=0; i!=GET_SDATASET(ids)._PropertyContainer.PropertyValueArrays.size(); ++i )
		{
			log.displayNL( "  %p", GET_SDATASET(ids)._PropertyContainer.PropertyValueArrays[i].Values );
		}
	}
}


/*
 * Info: display the number of active entities, created by the current mirror
 */
void	CMirror::displayEntityTypesOwned( NLMISC::CLog& log ) const
{
	TEntityTypesOwned::const_iterator ieto;
	for ( ieto=_EntityTypesOwned.begin(); ieto!=_EntityTypesOwned.end(); ++ieto )
	{
		log.displayNL( "* EntityType %hu *", (uint16)(*ieto).first );
		log.displayNL( "  %d entities (max %d), concerning %u datasets", GET_ENTITY_TYPE_OWNED(ieto).CurrentNb, GET_ENTITY_TYPE_OWNED(ieto).MaxNb, GET_ENTITY_TYPE_OWNED(ieto).DataSetList.size() );
	}

	TSDataSets::const_iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		log.displayNL( "* DataSet %s *", GET_SDATASET(ids).name().c_str() );
		TEntityRangeOfType::const_iterator irot;
		for ( irot=GET_SDATASET(ids)._EntityTypesRanges.begin(); irot!=GET_SDATASET(ids)._EntityTypesRanges.end(); ++irot )
		{
			log.displayNL( "  EntityType %hu", (uint16)(*irot).first );
			log.displayNL( "    Range: E%d to E%d", GET_ENTITY_TYPE_RANGE(irot).first(), GET_ENTITY_TYPE_RANGE(irot).last() );
			log.displayNL( "    NextFree: %d; Number of released indexes: %u", GET_ENTITY_TYPE_RANGE(irot).nextFree(), GET_ENTITY_TYPE_RANGE(irot).nbReleasedIndexes() );
		}
	}
}


/*
 * Info: display the contents of the rows corresponding to the entity id
 */
void	CMirror::displayRows( const NLMISC::CEntityId& entityId, NLMISC::CLog& log ) const
{
	bool found = false;

	log.displayNL("Dumping row for entity %s:", entityId.toString().c_str());
	TSDataSets::const_iterator first(_SDataSets.begin()), last(_SDataSets.end());
	for (; first != last; ++first)
	{
		const CMirroredDataSet &ds = first->second;
		TDataSetRow	dsr = ds.getDataSetRow(entityId);

		if (dsr.isValid())
		{
			found = true;

			const CSheetId &dataSetId = first->first;
			log.displayNL("Dumping properties in dataset '%s':", dataSetId.toString().c_str());

			for (sint16 i=0; i<ds.nbProperties(); ++i)
			{
				// retrive the i'th propertie
				TPropertiesInMirror::const_iterator it(_PropAllocator._PropertiesInMirror.begin());

				for (; it != _PropAllocator._PropertiesInMirror.end(); ++it)
				{
					if (it->second.PropertyIndex == i && it->second.DataSet == &ds)
					{
						// we found the property
						ds.displayPropValue( it->first, dsr, i, it->second.getFlagsString().c_str(), log );
						break;
					}
				}

			}
		}
	}

/*
	TPropertiesInMirror::const_iterator ipim;
	log.displayNL( "* Properties of entity %s (values, timestamps...) *", entityId.toString().c_str() );
	for ( ipim=_PropAllocator._PropertiesInMirror.begin(); ipim!=_PropAllocator._PropertiesInMirror.end(); ++ipim )
	{
		const string& propName = (*ipim).first;
		if ( GET_PROPERTY_INFO(ipim).allocated() )
		{
			CMirroredDataSet *dataset = GET_PROPERTY_INFO(ipim).DataSet;
			if ( dataset )
			{
				TDataSetRow entityIndex = dataset->getDataSetRow( entityId );
				if ( entityIndex.isValid() )
				{
					TPropertyIndex propertyIndex = dataset->getPropertyIndex( propName );
					if ( propertyIndex != INVALID_PROPERTY_INDEX ) // skip _EID_...
					{
						 dataset->displayPropValue( propName, entityIndex, propertyIndex, GET_PROPERTY_INFO(ipim).getFlagsString().c_str(), log );
					}
					found = true;
				}
			}
		}
//		else // this occurs if the service did not declare the property
//		{
//			log.displayNL( "%s: property not allocated", propName.c_str() );
//		}
	}
*/
	if ( ! found )
	{
		log.displayNL( "No such entity in any dataset" );
	}
}


/*
 * Debug: change a value from a string
 */
void CMirror::changeValue( const NLMISC::CEntityId& entityId, const std::string& propName, const std::string& valueStr )
{
	bool propok = true;
	TPropertiesInMirror::iterator ipim = _PropAllocator._PropertiesInMirror.find( propName );
	if ( ipim != _PropAllocator._PropertiesInMirror.end() )
	{
		const string& propName = (*ipim).first;
		if ( GET_PROPERTY_INFO(ipim).allocated() )
		{
			CMirroredDataSet *dataset = GET_PROPERTY_INFO(ipim).DataSet;
			if ( dataset )
			{
				TDataSetRow entityIndex = dataset->getDataSetRow( entityId );
				if ( entityIndex.isValid() )
				{
					TPropertyIndex propertyIndex = dataset->getPropertyIndex( propName );
					if ( propertyIndex != INVALID_PROPERTY_INDEX )
					{
						dataset->setValueFromString( entityIndex, propertyIndex, valueStr );
					}
					else propok = false;
				}
				else
				{
					nlwarning( "MIRROR: Entity %s not found in dataset %s", entityId.toString().c_str(), dataset->name().c_str() );
				}
			}
			else propok = false;
		}
		else propok = false;
	}
	else propok = false;
	if ( ! propok )
	{
		nlwarning( "MIRROR: Property %s not found or allocated", propName.c_str() );
	}
}


/*
 * Constructor
 */
CMirror::CMirror() :
	_PendingEntityTypesRanges(0),
	_ReadyL1Callback(NULL),
	_NotificationCallback(NULL),
	_UserSyncCallback(NULL),
	_MirrorAllReady(false),	
	_MirrorGotReadyLevel1(false),
	_MirrorGotReadyLevel2(false),
	_ListOfOtherPropertiesReceived(false),
	_AwaitingAllMirrorsOnline(false),
	_IsExecutingSynchronizedCode(false),
	_ClosureRequested(false),
	MonitoredEntity(CEntityId::Unknown)
{
	nlassert( ! MirrorInstance ); // singleton check
	MirrorInstance = this;
}


/*
 *
 */
void	CMirror::initAfterMirrorUp()
{
	if ( _MirrorAllReady )
	{
		if ( _AwaitingAllMirrorsOnline )
		{
			// This is when a MS has been shutdown while we were using it! When it cames back,
			// send our mirror info to it
			resyncMirrorInfo();
			_AwaitingAllMirrorsOnline = false; // now if receiving a "mirrors online" msg, it will not mean it's our local MS
		}
	}
	else
	{
		doInitMirrorLevel1();
	}
}


/*
 * Closure clearance callback
 */
bool	cbRequestClosure()
{
	return MirrorInstance->requestClosure();
}


/*
 *
 */
void	CMirror::doInitMirrorLevel1()
{
	if ( _MirrorGotReadyLevel1 )
		return;

	_AwaitingAllMirrorsOnline = false; // see in initAfterMirrorUp()

	nlassert( mirrorServiceIsUp() ); // could fail if the serviceUp from the NS arrives later than the MIRO from the MS!
	MIRROR_INFO( "MIRROR: Local Mirror Service found (service %hu)", localMSId().get() );

	MIRROR_INFO( "MIRROR: %u datasets used by this service", _DataSetsToLoad.size() );
	if ( _SDataSetSheets.empty() )
	{
		nlwarning( "MIRROR: No dataset found, check if dataset.packed_sheets and the georges sheets are in the path" );
	}

	// Inform the MS about the datasets we know (and this will add us in the client services list of the MS)
	CMessage msgoutDATASETS ( "DATASETS" );
	msgoutDATASETS.serialCont( _DataSetsToLoad );
	msgoutDATASETS.serial( _MTRTag );
	CUnifiedNetwork::getInstance()->send( localMSId(), msgoutDATASETS );

	// Init the needed CDataSetMS objects from the corresponding TDataSetSheets
	uint16 dsIndex = 0;
	string skippedDataSets;
	sint nbSkipped = 0;
	TSDataSetSheets::iterator ism;
	for ( ism=_SDataSetSheets.begin(); ism!=_SDataSetSheets.end(); ++ism )
	{
		TDataSetSheet& dataSetSheet = (*ism).second;
		if ( find( _DataSetsToLoad.begin(), _DataSetsToLoad.end(), dataSetSheet.DataSetName ) != _DataSetsToLoad.end() )
		{
			MIRROR_INFO( "MIRROR: \tDataset: %s", dataSetSheet.DataSetName.c_str() );

			// Create a CDataSetMS and insert into _SDataSets
			CMirroredDataSet newDataSet;
			pair<TSDataSets::iterator,bool> res = _SDataSets.insert( make_pair( (*ism).first, newDataSet ) );

			// Init CMirroredDataSet object (request allocation of shared memory for entity ids)
			GET_SDATASET(res.first).init( (*ism).first, dataSetSheet, &_PropAllocator );

			// Insert into _NDataSets
			_NDataSets.insert( make_pair( GET_SDATASET(res.first).name(), &GET_SDATASET(res.first) ) );

			storeDatasetPtToQuickArray( dsIndex++, &GET_SDATASET(res.first) );
		}
		else
		{
			skippedDataSets += " " + (*ism).second.DataSetName;
			++nbSkipped;
		}

	}
	MIRROR_INFO( "MIRROR: %u datasets loaded", _SDataSets.size() );
	MIRROR_INFO( "MIRROR: Skipped datasets:%s", skippedDataSets.empty()?" (none)":skippedDataSets.c_str() );
	if ( _DataSetsToLoad.size() + nbSkipped > _SDataSetSheets.size() )
	{
		nlwarning( "MIRROR: Some required datasets are missing in sheetid.bin" );
	}

	// We don't need the sheet information structures anymore
	_SDataSetSheets.clear();

	// Call the user callback
	_ReadyL1Callback( this );

	// Request the list of other properties (that will be cleaned when creating an entity, even if this service did not subscribe them)
	CMessage msgoutGop( "GOP" ); // Get Other Properties
	CUnifiedNetwork::getInstance()->send( localMSId(), msgoutGop );

	// Set closure clearance callback (not in CMirror::init()) because
	// launching the service with -Q would never exit if the MS is not up
	IService::getInstance()->setClosureClearanceCallback( cbRequestClosure );

	_MirrorGotReadyLevel1 = true;
}


/*
 * Send to a reconnecting MS the state of our mirror info
 */
void CMirror::resyncMirrorInfo()
{
	CMessage msgout( "SYNCMI" );

	// Resync entity types owned and trackers
	uint16 nbdatasets = (uint16)_SDataSets.size();
	msgout.serial( nbdatasets );
	TSDataSets::	iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		msgout.serial( const_cast<string&>(GET_SDATASET(ids).name()) );
		GET_SDATASET(ids).serialOutMirrorInfo( msgout );
	}

	// Resync properties
	_PropAllocator.serialOutMirrorInfo( msgout );

	CUnifiedNetwork::getInstance()->send( localMSId(), msgout );
}

/*
 *
 */
CMirror::~CMirror()
{
//	release();
}

/*
 * Release - mandatory to be called by the user if the object is static,
 * no matter if it is called again by the destructor
 */
void CMirror::release()
{
	// Execute mirror release command before destroying all the data
	executeMirrorReleaseCommands();

	// Release the trackers
	TSDataSets::iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		CMirroredDataSet& ds = GET_SDATASET(ids);

		ds._SelfEntityTrackers[ADDING].release();
		ds._SelfEntityTrackers[REMOVING].release();

		TSelfPropTrackers::iterator isp;
		for ( isp=ds._SelfPropTrackers.begin(); isp!=ds._SelfPropTrackers.end(); ++isp )
			(*isp).release();

		TTrackerListForEnt::iterator iet;
		for ( iet=ds._EntityTrackers[ADDING].begin(); iet!=ds._EntityTrackers[ADDING].end(); ++iet )
			(*iet).release();
		for ( iet=ds._EntityTrackers[REMOVING].begin(); iet!=ds._EntityTrackers[REMOVING].end(); ++iet )
			(*iet).release();

		TTrackersLists::iterator itl;
		for ( itl=ds._PropTrackers.begin(); itl!=ds._PropTrackers.end(); ++itl )
		{
			TTrackerListForProp::iterator ipt;
			for ( ipt=(*itl).begin(); ipt!=(*itl).end(); ++ipt )
			{
				(*ipt).release();
			}
		}
	}
}


#ifdef NL_DEBUG

// Helper functor for testMirrorReadyStatus
class PropIndexIs : public binary_function< CChangeTrackerClientProp, TPropertyIndex, bool >
{
public:
	bool	operator() ( const CChangeTrackerClientProp& tracker, TPropertyIndex propIndex ) const
	{
		return (tracker.propIndex() == propIndex);
	}
};

#endif


/*
 * Set _MirrorAllReady to true if the conditions are met
 */
void	CMirror::testMirrorReadyStatus()
{
	if ( _MirrorAllReady )
		return;

	// Check if mirrors and range manager are online
	if ( ! _MirrorGotReadyLevel1 )
	{
		MIRROR_DEBUG( "MIRROR: Still waiting for mirrors and range manager to be online" );
		return;
	}

	// Check that we have received the first game cycle
	if ( CTickEventHandler::getGameCycle() == 0 )
	{
		return;
	}

	// Check if mirror service is up and entity ranges ready
	if ( ! mirrorIsUp() )
	{
		//MIRROR_DEBUG( "MIRROR: Still waiting for entity ranges ready" );
		return;
	}

	// For each dataset
	TNDataSets::iterator ids;
	for ( ids=_NDataSets.begin(); ids!=_NDataSets.end(); ++ids )
	{
		// Test if SC_EI was received
		if ( ! GET_NDATASET(ids)._IsEntityNotificationEnabled )
		{
			MIRROR_DEBUG( "MIRROR: Still waiting for SC_EI" );
			return;
		}

		// Test if entity ids are allocated
		if ( ! propIsAllocated( string("_EId_") + (*ids).first ) )
		{
			MIRROR_DEBUG( "MIRROR: %s: Still waiting for allocation of entity ids", (*ids).first.c_str() );
			return;
		}

		// Test if the self entity trackers are allocated
		if ( ! GET_NDATASET(ids)._SelfEntityTrackers[ADDING].isAllocated() && GET_NDATASET(ids)._SelfEntityTrackers[REMOVING].isAllocated() ) // note: if one is allocated, the other one should be
		{
			MIRROR_DEBUG( "MIRROR: %s: Still waiting for self entity trackers", (*ids).first.c_str() );
			return;
		}
	}

	// For each declared property
	TPropertiesInMirror::iterator ipm;
	for ( ipm=_PropAllocator._PropertiesInMirror.begin(); ipm!=_PropAllocator._PropertiesInMirror.end(); ++ipm )
	{
		TPropertyInfo& propinfo = GET_PROPERTY_INFO(ipm);

		// Test if a declared property is still pending (i.e. not allocated yet)
		if ( propinfo.Pending )
		{
			MIRROR_DEBUG( "MIRROR: Still waiting for allocation of property %s/%hd", propinfo.DataSet->name().c_str(), propinfo.PropertyIndex );
			return;
		}

		// Among the allocated properties, test the prop trackers
		if ( propinfo.allocated() )
		{
			// Test if the prop tracker list has been received
			if ( propinfo.PropTrackersPending && (propinfo.PropertyIndex != INVALID_PROPERTY_INDEX) ) // ignore "entity id" propinfo
			{
				MIRROR_DEBUG( "MIRROR: Still waiting for property tracker list of %s/%hd", propinfo.DataSet->name().c_str(), propinfo.PropertyIndex );
				return;
			}

#ifdef NL_DEBUG
			// Additional tests
			if ( propinfo.flagNotifyChanges() && (!propinfo.flagGroupNotifiedByOtherProp()) )
			{
				nlassert( propinfo.DataSet );
				TSelfPropTrackers::iterator ipt = find_if( propinfo.DataSet->_SelfPropTrackers.begin(), propinfo.DataSet->_SelfPropTrackers.end(), bind2nd( PropIndexIs(), propinfo.PropertyIndex ) );
				if ( ipt == propinfo.DataSet->_SelfPropTrackers.end() )
				{
					nlwarning( "MIRROR: Property %s/%hd with notification of changes: selfprop tracker not received", propinfo.DataSet->name().c_str(), propinfo.PropertyIndex );
				}
			}
#endif
		}
	}

	// Expect the list of non-subscribed properties allocated on the local MS
	if ( ! _ListOfOtherPropertiesReceived )
	{
		MIRROR_DEBUG( "MIRROR: Still waiting for LOP" );
		return;
	}

	// Success
	if ( ! _MirrorGotReadyLevel2 )
	{
		// Broadcast our ready state to all other services!
		CMessage msgout( "SMIRUB" ); // 'Service has MIRror system Up Broadcast'
		pushEntityRanges( msgout );
		sendMessageViaMirrorToAll( msgout ); // via mirror to ensure that MARCS was sent to all remote MS before their client services get SMIRU (otherwise their potential messages couldn't be routed through their MS)

		nlinfo( "Mirror system ready, %s", (_MTRTag==AllTag) ? "all MTR Tags" : toString( "MTR Tag=%d", _MTRTag ).c_str() );

		if ( ! _NotificationCallback )
		{
			nlerror( "No notification callback has been set, entities won't be updated in the mirror!" );
		}

		// The start callbacks & commands will be executed when receiving the first UMM
		// because they need to be synchronized with the MS

		_MirrorGotReadyLevel2 = true;
	}
}


/*
 * Execute start callbacks & commands
 */
void CMirror::executeMirrorReady2CallbacksAndStartCommands()
{
	// Ready level2 callbacks
	vector<TMirrorReadyCallback>::const_iterator icb;
	for ( icb=_ReadyL2Callbacks.begin(); icb!=_ReadyL2Callbacks.end(); ++icb )
	{
		(*icb)(this);
	}

	// Start commands when mirror ready
	string cmdRoot("StartCommandsWhenMirrorReady");
	vector<string>	posts;

	if (IService::getInstance()->haveArg('m'))
	{
		string s = IService::getInstance()->getArg('m');
		NLMISC::explode(s, string(":"), posts, false);
	}
	else
		// add an empty string
		posts.push_back(string());

	CConfigFile::CVar *var;
	for (uint i=0; i<posts.size(); ++i)
	{
		string varName = cmdRoot + posts[i];
		if ((var = IService::getInstance()->ConfigFile.getVarPtr (varName)) != NULL)
		{
			MIRROR_INFO( "MIRROR: Executing '%s'...", varName.c_str() );
			for (uint i = 0; i < var->size(); i++)
			{
				ICommand::execute (var->asString(i), *InfoLog);
			}
		}
	}

}

/*
 * Execute start callbacks & commands
 */
void CMirror::executeMirrorReleaseCommands()
{
	// Start commands when mirror ready
	string cmdRoot("CommandsWhenMirrorRelease");
	vector<string>	posts;

	if (IService::getInstance()->haveArg('m'))
	{
		string s = IService::getInstance()->getArg('m');
		NLMISC::explode(s, string(":"), posts, false);
	}
	else
		// add an empty string
		posts.push_back(string());

	CConfigFile::CVar *var;
	for (uint i=0; i<posts.size(); ++i)
	{
		string varName = cmdRoot + posts[i];
		if ((var = IService::getInstance()->ConfigFile.getVarPtr (varName)) != NULL)
		{
			MIRROR_INFO( "MIRROR: Executing '%s'...", varName.c_str() );
			for (uint i = 0; i < var->size(); i++)
			{
				ICommand::execute (var->asString(i), *InfoLog);
			}
		}
	}
}



/*
 *
 */
void CMirror::pushEntityRanges( CMessage& msgout )
{
	uint32 nbDatasets = (uint32)_SDataSets.size();
	msgout.serial( nbDatasets );
	for ( TSDataSets::iterator ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		msgout.serial( const_cast<CSheetId&>((*ids).first) );
		msgout.serialCont( (*ids).second._EntityTypesRanges );
	}
}


/*
 * Global routine to be called by update
 */
void CMirror::updateWatches()
{
	if ( mirrorIsReady() )
	{
		for (uint i=0; i!=sizeof(mWatchStrings)/sizeof(mWatchStrings[0]); ++i )
		{
			if ( WatchedPropValues[i] )
				getPropValueStr( WatchedPropValues[i]->EntityId, WatchedPropValues[i]->PropName, *(mWatchStrings[i]) );
			else
				mWatchStrings[i]->clear();
		}
	}
}


/*
 *
 */
void CMirror::getPropValueStr( const NLMISC::CEntityId& entityId, const std::string& propName, std::string& result ) const
{
	bool propok = true;
	TPropertiesInMirror::const_iterator ipim = _PropAllocator._PropertiesInMirror.find( propName );
	if ( ipim != _PropAllocator._PropertiesInMirror.end() )
	{
		const string& propName = (*ipim).first;
		if ( GET_PROPERTY_INFO(ipim).allocated() )
		{
			CMirroredDataSet *dataset = GET_PROPERTY_INFO(ipim).DataSet;
			if ( dataset )
			{
				TDataSetRow entityIndex = dataset->getDataSetRow( entityId );
				if ( entityIndex.isValid() )
				{
					TPropertyIndex propertyIndex = dataset->getPropertyIndex( propName );
					if ( propertyIndex != INVALID_PROPERTY_INDEX )
					{
						dataset->getValueToString( entityIndex, propertyIndex, result );
						result = toString( "%s,%s=%s", entityId.toString().c_str(), propName.c_str(), result.c_str() );
					}
					else propok = false;
				}
				else
				{
					result = toString( "<%s not found>", entityId.toString().c_str() );
				}
			}
			else propok = false;
		}
		else propok = false;
	}
	else propok = false;
	if ( ! propok )
	{
		result = toString( "<%s not found>", propName.c_str() );
	}
}


/*
 * Provide a callback that will be called the first time mirrorIsReady() return true.
 * Note: immediately after having called all the callbacks passed to this method, the
 * commands founds in StartCommandsWhenMirrorReady (in the config file) will be executed.
 */
void	CMirror::addCallbackWhenMirrorReadyForUse( TMirrorReadyCallback cbLevel2 )
{
	nlassert( cbLevel2 );
	_ReadyL2Callbacks.push_back( cbLevel2 );
}


/*
 * Send the transport class to a specified service using the service id
 */
//void	CMirrorTransportClass::send( uint8 sid )
//{
	/*// Experimental: forwarding without copy
	CMessage& msgout = write();
	msgout.changeType( "MT1MSG" ); // same size as CT_MSG
	msgout.serial( sid );
	CUnifiedNetwork::getInstance()->send( MirrorInstance->localMSId(), msgout );*/

	/*CMessage msgout( "MTC_MSG" );
	CMessage& ctmsg = write();
	msgout.serialBuffer( const_cast<uint8*>(ctmsg.buffer()+ctmsg.getHeaderSize()), ctmsg.length()-ctmsg.getHeaderSize() );
	CUnifiedNetwork::getInstance()->send( sid, msgout );
	nldebug( "%u: Sending MTC to service %hu", CTickEventHandler::getGameCycle(), (uint16)sid );*/

	// See in class declaration
//}


/*
 * Send the transport class to a specified service using the service name
 */
//void	CMirrorTransportClass::send( std::string serviceName )
//{
	/*// Experimental: forwarding without copy
	CMessage& msgout = write();
	msgout.changeType( "MT2MSG" ); // same size as CT_MSG
	msgout.serial( serviceName );
	uint8 sns = serviceName.size();
	msgout.serial( sns );
	CUnifiedNetwork::getInstance()->send( MirrorInstance->localMSId(), msgout );*/

	/*CMessage msgout( "MTC_MSG" );
	CMessage& ctmsg = write();
	msgout.serialBuffer( const_cast<uint8*>(ctmsg.buffer()+ctmsg.getHeaderSize()), ctmsg.length()-ctmsg.getHeaderSize() );
	CUnifiedNetwork::getInstance()->send( serviceName, msgout );
	nldebug( "%u: Sending MTC to %s", CTickEventHandler::getGameCycle(), serviceName.c_str() );*/

	// See in class declaration
//}


// Standard transport class callback
//extern void cbTCReceiveMessage (CMessage &msgin, const string &name, uint16 sid);


/*
 * Mirror transport class callback
 */
/*void cbMirrorTCReceiveMessage( CMessage &msgin, const string &name, uint16 sid )
{
	//nldebug( "Receiving MTC from service %hu", sid );
	MirrorInstance->_BufferedMTC.push_back( make_pair(msgin,sid) );
}*/


/*
 * The mirror transport class are applied at the end of the service "tick update" method,
 * to ensure all entities have been processed.
 * NOW this is replaced by the message chain provided in the message UMM, and the notification callback
 * is called before.
 */
/*void	CMirror::applyBufferedTransportClasses()
{
	CBufferedMirrorTCs::const_iterator it;
	for ( it=_BufferedMTC.begin(); it!=_BufferedMTC.end(); ++it )
	{
		nldebug( "%u: Applying MTC from service %hu", CTickEventHandler::getGameCycle(), (*it).second );
		//string senderName; // blank because not used
		cbTCReceiveMessage( const_cast<CMessage&>((*it).first), "", (*it).second ); // change the sid: sender instead of forwarder MS
	}
	_BufferedMTC.clear();
}*/


/*
 * Receive the UMM Message (callback)
 */
void	cbUpdateMirrorAndReceiveMessages( CMessage &msgin, const string&, TServiceId )
{
	MirrorInstance->updateMirrorAndReceiveMessages( msgin );


	// update the mirror counter var
	MirrorCallbackCount = TotalMirrorCallbackCalled;
	MirrorCallbackTime = TimeInMirrorCallback;

	// reset the counters
	TimeInMirrorCallback = 0;
	TotalMirrorCallbackCalled = 0;
}


/*
 * Receive the UMM Message
 */
void	CMirror::updateMirrorAndReceiveMessages( CMessage& msgin )
{
	_IsExecutingSynchronizedCode = true;

	// Synchronized release code
	if ( _ClosureRequested )
	{
		if ( _TickReleaseFunc ) // should be non-null when we get here anyway
			_TickReleaseFunc();
		IService::getInstance()->clearForClosure();
		return;
	}

	// Start callbacks & commands
	if ( (!_MirrorAllReady) && _MirrorGotReadyLevel2 )
	{
		_MirrorAllReady = true;
		executeMirrorReady2CallbacksAndStartCommands();
		IService::getInstance()->clearCurrentStatus("WaitingMirrorReady");
	}

	TAccurateTime TimeBeforeUMM = getAccurateTime();

	// Synchronized Service Up / Service Mirror Up / Service Down / Service Mirror Down callbacks
	for ( std::vector<TCallbackArgItemMExt>::const_iterator iscq=_SynchronizedServiceUpDnCallbackQueue.begin(); iscq!=_SynchronizedServiceUpDnCallbackQueue.end(); ++iscq )
	{
		const TCallbackArgItemMExt& cbItem = (*iscq);
		if ( cbItem.Cb )
			cbItem.Cb( cbItem.ServiceName, cbItem.ServiceId, cbItem.Arg );
	}
	_SynchronizedServiceUpDnCallbackQueue.clear();

	// Trigger the notification of mirror changes
	if ( mirrorIsReady() && _NotificationCallback )
	{
		_NotificationCallback();
	}
	TAccurateTime TimeBeforeReceiveMsgViaMirror = getAccurateTime();
	ProcessMirrorUpdatesSpeed = (sint16)accTimeToMs( TimeBeforeReceiveMsgViaMirror - TimeBeforeUMM );

	// Apply the messages
	uint32 nbMsgs;
	msgin.serial( nbMsgs );
	if ( mirrorIsReady() )
	{
		uint32 i;
		for ( i=0; i!=nbMsgs; ++i )
		{
			// --------------------------------------------------------------------------------------------------------------
			// timer code added here by Sadge
			// copied feom unified_network.cpp
			static map<string, CHTimer> timers;
			map<string, CHTimer>::iterator it;

			{
				H_AUTO(L5UCHTimerOverhead);
				string callbackName = "USRCB_" + msgin.getName();
				it = timers.find(callbackName);
				if(it == timers.end())
				{
					it = timers.insert(make_pair(callbackName, CHTimer(NULL))).first;
					(*it).second.setName((*it).first.c_str());
				}
			}
			// --------------------------------------------------------------------------------------------------------------

			(*it).second.before();
			TServiceId senderId;
			msgin.serial( senderId );
			sint32 msgSize, msgPos;
			msgin.serial( msgSize );

			TTime before = CTime::getLocalTime();

			// Modify msgin so that the upcoming submessages looks like a normal message for the corresponding callback
			msgPos = msgin.getPos();
			string name = msgin.lockSubMessage( msgSize ); // fake the length for the callback
			TUnifiedMsgCallback cb = CUnifiedNetwork::getInstance()->findCallback( name );
			string sn = CUnifiedNetwork::getInstance()->getServiceName( senderId );
			if ( cb && !sn.empty())
				cb( msgin, sn, senderId );
			msgin.unlockSubMessage(); // set original length back and skip sub message
			(*it).second.after();

			TTime after = CTime::getLocalTime();
			// sum the time used to do callback
			TimeInMirrorCallback += uint32(after-before);
			TotalMirrorCallbackCalled++;
		}
	}
	else if ( nbMsgs != 0 )
	{
		string names;
		uint32 i;
		for ( i=0; i!=nbMsgs; ++i )
		{
			TServiceId senderId;
			msgin.serial( senderId );
			sint32 msgSize, msgPos;
			msgin.serial( msgSize );
			msgPos = msgin.getPos();
			std::string messageName = msgin.lockSubMessage( msgSize );

			// Get the class name if the message is a Transport Class
			if (messageName == "CT_MSG")
			{
				string className;
				CTransportClass::readHeader(msgin, className);
				messageName += "__" + className;
			}

			// Report the name of the message
			if (messageName != "SMIRUB") // keep quiet if receiving a SMIRUB (Service Mirror Up Broadcast) at this time (info will be sent by SMIRUR instead)
				names += messageName + " ";

			msgin.unlockSubMessage(); // skip sub message
		}
		if (!names.empty())
		{
			// This should not occur much anymore, as sendMessageViaMirror() no longer sends if !serviceHasMirrorReady()
			nlwarning( "MIRROR: MSG: %u messages received via MS before mirror is ready, discarded: %s", nbMsgs, names.c_str() );
		}
	}

	_IsExecutingSynchronizedCode = false;

	TAccurateTime TimeAfterUMM = getAccurateTime();
	ReceiveMessagesViaMirrorSpeed = sint16(accTimeToMs( TimeAfterUMM - TimeBeforeReceiveMsgViaMirror ));
	TotalSpeedLoop = accTimeToMs( TimeAfterUMM - TimeBeforeTickUpdate );
}

/*
 *
 */
void	cbscanAndResyncEntitiesExceptIgnored( CMessage &msgin, const string&, TServiceId )
{
	vector<TServiceId8> creatorIdsToIgnore;
	msgin.serialCont( creatorIdsToIgnore );
	MirrorInstance->scanAndResyncEntitiesExceptIgnored( creatorIdsToIgnore );
}


/*
 *
 */
void	cbApplyListOfOtherProperties( CMessage &msgin, const string&, TServiceId )
{
	uint32 nbProps;
	msgin.serial( nbProps );
	MirrorInstance->applyListOfOtherProperties( msgin, nbProps, "LOP", true );
}


/*
 *
 */
void	cbAddListOfOtherProperties( CMessage &msgin, const string&, TServiceId )
{
	MirrorInstance->applyListOfOtherProperties( msgin, 1, "AOP", false );
}


/*
 * Callback array
 */
TUnifiedCallbackItem MirrorCbArray [] =
{
	{ "RAP", cbRecvSMIdToAccessPropertySegment },
	{ "RG", cbRecvRangesForEntityType },
	{ "ATE", cbRecvAddEntityTracker },
	{ "ATP", cbRecvAddPropTracker },
	{ "ACKATE", cbRecvAcknowledgeAddEntityTracker },
	{ "ACKATP", cbRecvAcknowledgeAddPropTracker },
	{ "RT", cbReleaseTrackers },
	{ "MIRO", cbAllMirrorsOnline },
	{ "SMIRUB", cbRecvServiceHasMirrorReadyBroadcast },
	{ "SMIRUR", cbRecvServiceHasMirrorReadyReply },
	{ "UMM", cbUpdateMirrorAndReceiveMessages },
	{ "LOP", cbApplyListOfOtherProperties },
	{ "AOP", cbAddListOfOtherProperties },
	{ "SC_EI", cbscanAndResyncEntitiesExceptIgnored }
};
const sint NB_MIRROR_CALLBACKS = sizeof(MirrorCbArray)/sizeof(MirrorCbArray[0]);


/*
 *
 */
void	cbTickUpdateFunc()
{
	MirrorInstance->onTick();
}


/*
 * Initialize. Call before doing any mirror-related action.
 */
void	CMirror::init( std::vector<std::string>& dataSetsToLoad,
						TMirrorReadyCallback cbLevel1,
						TNotificationCallback tickUpdateFunc,
						TNotificationCallback tickSyncFunc,
						TNotificationCallback tickReleaseFunc,
						TMTRTag tag )
{
	IService::getInstance()->setCurrentStatus("WaitingMirrorReady");

	_DataSetsToLoad = dataSetsToLoad;
	_ReadyL1Callback = cbLevel1;
	nlassert( _ReadyL1Callback );

	bool expediteTock = false;
	CConfigFile::CVar *varExpTock = IService::getInstance()->ConfigFile.getVarPtr( "ExpediteTOCK" );
	if ( varExpTock )
		expediteTock = (varExpTock->asInt() == 1);
	_TickUpdateFunc = tickUpdateFunc;
	_TickReleaseFunc = tickReleaseFunc;
	CTickEventHandler::init( cbTickUpdateFunc, tickSyncFunc, expediteTock );
	_UserSyncCallback = CTickEventHandler::setSyncCallback( cbSyncGameCycle, true );

	// Register to ServiceUp and ServiceDown
	CUnifiedNetwork::getInstance()->setServiceUpCallback( "MS", cbMSUpDn, (void*)true );
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "MS", cbMSUpDn, (void*)false );

    // in both of the following lines we have an integer value which we want to pass in the 'void*' third parameter to SetService....Callback
    // for this we use pointer arithmetic with char* pointers in order to avoid compiler warnings
 	CUnifiedNetwork::getInstance()->setServiceUpCallback( "*", cbAnyServiceUpDn, (void*)((char*)NULL+ETServiceUp) );
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "*", cbAnyServiceUpDn, (void*)((char*)NULL+ETServiceDn) );

	// Init mirror callbacks
	CUnifiedNetwork::getInstance()->addCallbackArray( MirrorCbArray, NB_MIRROR_CALLBACKS );

	// Load the sheets of the datasets
	// if the 'GeorgePaths' config file var exists then we try to perform a mini-scan for sheet files
	if (IService::isServiceInitialized() && (IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths"))!=NULL))
	{
		loadForm("dataset", IService::getInstance()->WriteFilesDirectory.toString()+"datasets.packed_sheets", _SDataSetSheets, false, false);
	}

	// if we haven't succeeded in minimal scan (or 'GeorgePaths' wasn't found in config file) then perform standard scan
	if (_SDataSetSheets.empty())
	{
		loadForm("dataset", IService::getInstance()->WriteFilesDirectory.toString()+"datasets.packed_sheets", _SDataSetSheets, true);
	}

	// Set the tag
	nlassert( (tag >= AllTag) && (tag != ExcludedTag) );
	_MTRTag = tag;

	MIRROR_INFO( "MIRROR: MV%s - %s - Expecting detection of local Mirror Service...", MirrorVersion.c_str(), (tag==AllTag) ? "No Tag" : NLMISC::toString("Tag=%hu", (uint16)_MTRTag).c_str() );
#ifdef STORE_CHANGE_SERVICEIDS
	MIRROR_INFO( "MIRROR: > Storing change service ids ENABLED" );
#endif
#ifdef CHECK_DATASETROW_VALIDITY
	MIRROR_INFO( "MIRROR: > Checking datasetrow validity ENABLED" );
#endif

#ifdef FAST_MIRROR
	MIRROR_INFO( "MIRROR: > Fast mirror mode ENABLED" );
#else
	MIRROR_INFO( "MIRROR: > Fast mirror mode DISABLED" );
#endif

}


/*
 * Closure clearance callback
 */
bool	CMirror::requestClosure()
{
	_ClosureRequested = true;
	return (_TickReleaseFunc == NULL); // quit immediately if there is no tick release callback
}


/*
 * Execute user callback when receiving first game cycle
 */
void	CMirror::userSyncCallback()
{
	if( _UserSyncCallback )
		_UserSyncCallback();
}


/*
 *
 */
void	sendMessageViaMirror( const std::string& destServiceName, CMessage& msgout )
{
	if (MirrorInstance->localMSId()==TServiceId::InvalidId)
	{
		nlwarning("Ignoring attempt to send a message via the mirror before mirror ready: %s",msgout.getName().c_str());
		return;
	}

	CMessage carrierMsgout( "FWDMSG" ); // to (possibly) multiple services
	/*vector<uint16> vec = CUnifiedNetwork::getInstance()->getConnectionList(), vecDest( vec.size() );
	vector<uint16>::const_iterator it;
	for ( it=vec.begin(); it!=vec.end(); ++it )
	{
		if ( CUnifiedNetwork::getInstance()->getServiceName( *it ) == destServiceName )
			vecDest.push_back( *it );
	}
	uint8 counter = (uint8)vecDest.size();
	carrierMsgout.serial( counter );
	for ( it=vecDest.begin(); it!=vecDest.end(); ++it )
	{
		carrierMsgout.serialCont( *it );
	}*/
	uint8 counter = 0;
	sint32 counterPos = carrierMsgout.reserve( 1 );
	vector<TServiceId> vec = CUnifiedNetwork::getInstance()->getConnectionList();
	for ( vector<TServiceId>::const_iterator it=vec.begin(); it!=vec.end(); ++it )
	{
		if ( CUnifiedNetwork::getInstance()->getServiceName( *it ) == destServiceName )
		{
			// Exclude services that are not known as "mirror ready" (would produce a warning in the MS)
			if ( ! MirrorInstance->serviceHasMirrorReady( *it ) )
			{
				nlinfo("Discarding service %s-%hu in sendMessageViaMirror because not 'mirror ready'", destServiceName.c_str(), (*it).get() );
				continue;
			}
			nlWrite(carrierMsgout, serial, (*it) );
			++counter;
		}
	}
	if ( counter != 0 )
	{
		carrierMsgout.poke( counter, counterPos );
		carrierMsgout.serialBuffer( const_cast<uint8*>(msgout.buffer()), msgout.length() );
		CUnifiedNetwork::getInstance()->send( MirrorInstance->localMSId(), carrierMsgout );
	}
	else
	{
		// Don't send if no corresponding service online (0 would be interpreted as a broadcast!)
		string msgName;
		msgout.invert();
		getNameOfMessageOrTransportClass( msgout, msgName );
		MIRROR_DEBUG( "MIRROR: MSG: No service %s is online (%s)", destServiceName.c_str(), msgName.c_str() );
	}
}


/*
 *
 */
void	sendMessageViaMirror( TServiceId destServiceId, CMessage& msgout )
{
	if (MirrorInstance->localMSId()==TServiceId::InvalidId)
	{
		nlwarning("Ignoring attempt to send a message via the mirror before mirror ready: %s",msgout.getName().c_str());
		return;
	}

	CMessage carrierMsgout( "FWDMSG" ); // to single service
	uint8 counter = 1;
	carrierMsgout.serial( counter );
	carrierMsgout.serial( destServiceId );
	carrierMsgout.serialBuffer( const_cast<uint8*>(msgout.buffer()), msgout.length() );
	CUnifiedNetwork::getInstance()->send( MirrorInstance->localMSId(), carrierMsgout );
}


/*
 *
 */
void	sendMessageViaMirrorToAll( NLNET::CMessage& msgout )
{
	if (MirrorInstance->localMSId()==TServiceId::InvalidId)
	{
		nlwarning("Ignoring attempt to send a message via the mirror before mirror ready: %s",msgout.getName().c_str());
		return;
	}

	CMessage carrierMsgout( "FWDMSG" );
	uint8 counter = MSG_BROADCAST;
	carrierMsgout.serial( counter );
	carrierMsgout.serialBuffer( const_cast<uint8*>(msgout.buffer()), msgout.length() );
	CUnifiedNetwork::getInstance()->send( MirrorInstance->localMSId(), carrierMsgout );
}


/*
 * displayMirrorRow command
 */
NLMISC_CATEGORISED_COMMAND(mirror, displayMirrorRow, "Display the contents of the mirror for an entity (all datasets)", "[<CEntityId> | <dataset> <entityIndex>]" )
{
	CEntityId id;
	switch( args.size() )
	{
	case 0: id = MirrorInstance->MonitoredEntity; break;
	case 1: id.fromString( args[0].c_str() ); break;
	case 2:
		{
			try
			{
				CMirroredDataSet& dataset = MirrorInstance->getDataSet( args[0] );
				TDataSetIndex entityIndex;
				fromString(args[1], entityIndex);
				if ( (entityIndex < dataset.maxNbRows()) && (entityIndex!=INVALID_DATASET_INDEX) )
				{
					id = dataset.getEntityId( dataset.getCurrentDataSetRow( entityIndex ) );
					log.displayNL( "%s", id.toString().c_str() );
				}
				else
				{
					log.displayNL( "Invalid entity index provided" );
					return true;
				}
			}
			catch (const EMirror&)
			{
				log.displayNL( "Dataset not found" );
				return true;
			}
			break;
		}
	default: return false;
	}

	MirrorInstance->displayRows( id, log );
	return true;
}


/*
 * setMirrorValue command
 */
NLMISC_CATEGORISED_COMMAND(mirror, setMirrorValue, "Change a value in the mirror", "[<CEntityId>] <propName> <value>" )
{
	CEntityId id;
	switch( args.size() )
	{
	case 2: id = MirrorInstance->MonitoredEntity; break;
	case 3: id.fromString( args[0].c_str() ); break;

	default: return false;
	}

	const size_t nargs = args.size();
	const char *afld = args[nargs-2].c_str();
	const char *aval = args[nargs-1].c_str();
	static char sval[64];

	// for equivalence with display of mirror values in ai_server
	// if input value is in form AAAA:BBBB:CCCC:DDDD -> treat as 64bit 0xDDDDCCCCBBBBAAAA
	std::string::size_type i = args[nargs-1].find( ':', 0 );
	if( std::string::npos != i )
	{
		uint w0, w1, w2, w3;
		sscanf( aval, "%x:%x:%x:%x", &w0, &w1, &w2, &w3 );
		uint64 u = static_cast<uint16>(w3);
		u <<= 16;
		u |= static_cast<uint16>(w2);
		u <<= 16;
		u |= static_cast<uint16>(w1);
		u <<= 16;
		u |= static_cast<uint16>(w0);
		string strval = NLMISC::toString( u );
//		_ui64toa( u, &sval[0], 10 );	// Microsoft only
		aval = strval.c_str();
	}

	MirrorInstance->changeValue( id, afld, aval );
	return true;
}


/*
 * addMirrorWatch command
 */
NLMISC_CATEGORISED_COMMAND(mirror, addMirrorWatch, "Watch a value of a property of an entity", "[<CEntityId>] <propName>" )
{
	if ( args.size() < 1 )
		return false;
	CEntityId id;
	if( args.size() == 2 )
		id.fromString( args[0].c_str() );
	else
	{
		id = MirrorInstance->MonitoredEntity;
		log.displayNL( "Only propName provided; using monitored entity %s, propName %s", id.toString().c_str(), args[args.size()-1].c_str() );
	}

	uint i = 0;
	bool found = false;
	while ( ! found )
	{
		if ( i > 9 )
		{
			log.displayNL( "No more watch available" );
			return true;
		}
		if ( ! WatchedPropValues[i] )
		{
			WatchedPropValues[i] = new CEntityAndPropName();
			WatchedPropValues[i]->EntityId = id;
			WatchedPropValues[i]->PropName = args[args.size()-1];
			log.displayNL( "Watch %u set", i );
			found = true;
		}
		++i;
	}
	return true;
}


/*
 * setMirrorWatch command
 */
NLMISC_CATEGORISED_COMMAND(mirror, setMirrorWatch, "Set a particular watch of a property value", "watchNb [<CEntityId>] <propName>" )
{
	if ( args.size() < 2 )
		return false;
	CEntityId id;
	if( args.size() == 3 )
		id.fromString( args[1].c_str() );
	else
	{
		id = MirrorInstance->MonitoredEntity;
		log.displayNL( "Only propName provided; using monitored entity %s", id.toString().c_str() );
	}

	uint i;
	fromString(args[0], i);
	if ( ! WatchedPropValues[i] )
	{
		WatchedPropValues[i] = new CEntityAndPropName();
		WatchedPropValues[i]->EntityId = id;
		WatchedPropValues[i]->PropName = args[args.size()-1];
		log.displayNL( "Watch %u set", i );
	}
	return true;
}


/*
 * removeMirrorWatch
 */
NLMISC_CATEGORISED_COMMAND(mirror, removeMirrorWatch, "Remove a particular watch", "watchNb" )
{
	if ( args.size() < 1 )
		return false;
	uint i;
	fromString(args[0], i);
	if ( WatchedPropValues[i] )
	{
		delete WatchedPropValues[i];
		WatchedPropValues[i] = NULL;
		mWatchStrings[i]->clear();
		log.displayNL( "Watch %u removed", i );
	}
	else
		log.displayNL( "Watch %u not found", i );
	return true;
}


/*
 * displayMirrorProperties command
 */
NLMISC_CATEGORISED_COMMAND(mirror, displayMirrorProperties, "Display the properties allocated in the mirror", "" )
{
	MirrorInstance->displayProperties( log );
	return true;
}


/*
 * displayMirrorEntities command
 */
NLMISC_CATEGORISED_COMMAND(mirror, displayMirrorEntities, "Display all of part of the entities known in a dataset (255=all)", "[<dataset> [<sortByDataSetRow=0> [<onlyEntityType=255> [<onlyCreatorId=255> [<onlyDynamicId=255>]]]]]" )
{
	CMirroredDataSet *dataset = NULL;
	bool sortByDataSetRow = false;
	uint8 onlyEntityType = 0xFF;
	uint8 onlyCreatorId = 0xFF;
	uint8 onlyDynamicId = 0xFF;
	if ( args.empty() )
	{
		if ( MirrorInstance->dsBegin() == MirrorInstance->dsEnd() )
		{
			log.displayNL( "No dataset found" );
			return true;
		}
		dataset = &GET_NDATASET(MirrorInstance->dsBegin());
		log.displayNL( "Displaying only first dataset" );
	}
	else try
	{
		dataset = &MirrorInstance->getDataSet( args[0] );
		if ( args.size() > 1 )
		{
			sortByDataSetRow = (args[1] == "1");
			if ( args.size() > 2 )
			{
				fromString(args[2], onlyEntityType);
				if ( args.size() > 3 )
				{
					fromString(args[3], onlyCreatorId);
					if ( args.size() > 4 )
					{
						fromString(args[4], onlyDynamicId);
					}
				}
			}
		}
	}
	catch (const EMirror&)
	{
		log.displayNL( "Dataset not found" );
	}
	dataset->displayEntities( log, onlyEntityType, onlyCreatorId, onlyDynamicId, false, true, !sortByDataSetRow, sortByDataSetRow );
	return true;
}

/*
 * lookForMirrorValue command
 */
NLMISC_COMMAND( lookForMirrorValue, "Look for values with criteria (Value can be * for any value, or a value in the format displayed in displayMirrorRow)", "<dataset> <propName> <valueString> [<onlyEntityType=255> [<onlyCreatorId=255> [<onlyDynamicId=255>]]]" )
{
	if ( args.size() > 2 )
	{
		try
		{
			string propName;
			bool anyValue = false;
			string valueSearchedStr;
			uint8 onlyEntityType = 0xFF;
			uint8 onlyCreatorId = 0xFF;
			uint8 onlyDynamicId = 0xFF;
			propName = args[1];
			if ( args.size() > 2 )
			{
				if ( args[2] == "*" )
					anyValue = true;
				else
					valueSearchedStr = args[2];
				if ( args.size() > 3 )
				{
					fromString(args[3], onlyEntityType);
					if ( args.size() > 4 )
					{
						fromString(args[4], onlyCreatorId);
						if ( args.size() > 5 )
						{
							fromString(args[5], onlyDynamicId);
						}
					}
				}
			}
			MirrorInstance->getDataSet( args[0] ).lookForValue( log, propName, anyValue, valueSearchedStr, onlyEntityType, onlyCreatorId, onlyDynamicId, false, true, false, true );
		}
		catch (const EMirror&)
		{
			log.displayNL( "Dataset not found" );
		}
		return true;
	}
	return false;
}

/*
 * displayMirrorTrackers command
 */
NLMISC_CATEGORISED_COMMAND(mirror, displayMirrorTrackers, "Display the trackers for a dataset", "<dataset>" )
{
	if ( args.size() == 1 )
	{
		try
		{
			MirrorInstance->getDataSet( args[0] ).displayTrackers( log );
		}
		catch (const EMirror&)
		{
			log.displayNL( "Dataset not found" );
		}
		return true;
	}
	return false;
}


/*
 * displayMirrorProperties command
 */
NLMISC_CATEGORISED_COMMAND(mirror, displayMirrorDatasets, "Display the datasets loaded and some stats", "" )
{
	TNDataSets::iterator ids;
	for ( ids=MirrorInstance->dsBegin(); ids!=MirrorInstance->dsEnd(); ++ids )
	{
		log.displayNL( "Dataset %s: %u entities online, %u offline (%d max rows), %hd properties, sheet %s", (*ids).first.c_str(), GET_NDATASET(ids).getNbOnlineEntities(), GET_NDATASET(ids).getNbOfflineEntities(), GET_NDATASET(ids).maxNbRows(), GET_NDATASET(ids).nbProperties(), GET_NDATASET(ids).sheetId().toString().c_str() );
	}
	return true;
}


/*
 * displayMirrorEntityTypesOwned command
 */
NLMISC_CATEGORISED_COMMAND(mirror, displayMirrorEntityTypesOwned, "Display the entity types owned by the service and related info", "" )
{
	MirrorInstance->displayEntityTypesOwned( log );
	return true;
}


/*
 * rescanExistingEntities command
 */
NLMISC_CATEGORISED_COMMAND(mirror, rescanExistingEntities, "Scan the entities to find if some are missing and add them (debug feature)", "<dataset>" )
{
	if ( args.empty() )
		return false;

	try
	{
		CMirroredDataSet& dataset = MirrorInstance->getDataSet( args[0] );
		MirrorInstance->rescanExistingEntities( dataset, log, true );
	}
	catch (const EMirror&)
	{
		log.displayNL( "Dataset not found" );
	}
	return true;
}


/*
 * displayUnknownOnlineEntities command
 */
NLMISC_CATEGORISED_COMMAND(mirror, displayUnknownOnlineEntities, "Scan the entities to find the online entities of which this service is not aware", "<dataset>" )
{
	if ( args.empty() )
		return false;

	try
	{
		CMirroredDataSet& dataset = MirrorInstance->getDataSet( args[0] );
		MirrorInstance->rescanExistingEntities( dataset, log, false );
	}
	catch (const EMirror&)
	{
		log.displayNL( "Dataset not found" );
	}
	return true;
}


/*
 * monitorMirrorEntity command
 */
NLMISC_CATEGORISED_COMMAND(mirror, monitorMirrorEntity, "Set/unset an entity for mirror commands when no argument provided", "[<CEntityId> | <dataset> <entityIndex>]" )
{
	switch( args.size() )
	{
	case 0: MirrorInstance->MonitoredEntity = CEntityId::Unknown; break;
	case 1:	MirrorInstance->MonitoredEntity.fromString( args[0].c_str() ); break;
	case 2:
		{
			try
			{
				CMirroredDataSet& dataset = MirrorInstance->getDataSet( args[0] );
				TDataSetIndex entityIndex;
				fromString(args[1], entityIndex);
				if ( (entityIndex < (uint32)dataset.maxNbRows()) && (entityIndex!=INVALID_DATASET_INDEX) )
					MirrorInstance->MonitoredEntity = dataset.getEntityId( dataset.getCurrentDataSetRow( entityIndex ) );
				else
					log.displayNL( "Invalid entity index provided" );
			}
			catch (const EMirror&)
			{
				log.displayNL( "Dataset not found" );
			}
			break;
		}
	default: return false;
	}
	log.displayNL( "The monitored entity is %s", MirrorInstance->MonitoredEntity.isUnknownId() ?"not set or unknown" : MirrorInstance->MonitoredEntity.toString().c_str() );
	return true;
}

NLMISC_CATEGORISED_DYNVARIABLE(mirror, sint32, MainNbEntities, "Number of online entities in dataset fe_temp" )
{
	// We can only read the value
	if ( get )
	{
		if ( MirrorInstance && MirrorInstance->mirrorIsReady() )
		{
			try
			{
				*pointer = MirrorInstance->getDataSet( "fe_temp" ).getNbOnlineEntities();
			}
			catch (const EMirror&)
			{
				*pointer = -2; // silent
			}
		}
		else
			*pointer = -1;
	}
}

NLMISC_CATEGORISED_DYNVARIABLE(mirror, sint32, LocalEntities, "Number of online local entities in dataset fe_temp" )
{
	// We can only read the value
	if ( get )
	{
		if ( MirrorInstance && MirrorInstance->mirrorIsReady() )
		{
			try
			{
				*pointer = MirrorInstance->getDataSet( "fe_temp" ).getNbOwnedEntities();
			}
			catch (const EMirror&)
			{
				*pointer = -2; // silent
			}
		}
		else
			*pointer = -1;
	}
}


NLMISC_CATEGORISED_DYNVARIABLE(mirror, string, MirrorVersion, "Version of mirror client" )
{
	if ( get )
		*pointer = MirrorVersion;
}

namespace NLNET
{
	extern bool createMessage (CMessage &msgout, const vector<string> &args, CLog &log);
}

NLMISC_CATEGORISED_COMMAND(mirror, msgoutViaMirror, "Send a message to a specified service, via mirror (ex: msgoutViaMirror 131 REGISTER u32 10 b 1 f 1.5)", "msgout <ServiceName>|<ServiceId> <MessageName> [<ParamType> <Param>]*" )
{
	if(args.size() < 2) return false;

	if (!MirrorInstance->mirrorIsReady())
	{
		log.displayNL ("Mirror is not ready");
		return false;
	}

	uint16 sId;
	fromString(args[0], sId);
	TServiceId serviceId(sId);
	string serviceName = args[0];
	string messageName = args[1];

	if (serviceId.get() > 255)
	{
		log.displayNL ("Service Id %d must be between [1;255]", serviceId.get());
		return false;
	}

	if ((args.size()-2) % 2 != 0)
	{
		log.displayNL ("The number of parameter must be a multiple of 2");
		return false;
	}

	CMessage msg (messageName);

	if (!NLNET::createMessage (msg, args, log))
		return false;

	if (serviceId.get() != 0)
		sendMessageViaMirror (serviceId, msg);
	else
		sendMessageViaMirror (serviceName, msg);

	return true;
}
