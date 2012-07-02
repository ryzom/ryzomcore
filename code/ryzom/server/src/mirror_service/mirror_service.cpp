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



#include "mirror_service.h"
#include "tick_proxy.h"
#include "game_share/ryzom_version.h"
#include <nel/misc/command.h>
#include <nel/misc/variable.h>
#include <nel/net/transport_class.h>
#include <nel/georges/load_form.h>

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace NLMISC;
using namespace NLNET;
using namespace std;

// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}


const string MirrorServiceVersion = string("1.10-")+string(ListRowSizeString); // ADDED: Unidirectional Mode (don't wait for delta)

CMirrorService *MSInstance = NULL;
bool DestroyGhostSharedMemSegments = false;
bool VerboseMessagesSent = false;
CDataSetMS *TNDataSetsMS::InvalidDataSet = NULL; // not used because expection thrown instead


/*
 * Return the service id string with the format "MS/128"
 */
std::string servStr( NLNET::TServiceId serviceId )
{
	return CUnifiedNetwork::getInstance()->getServiceUnifiedName( serviceId );
}


/*
 * Callback called at remote MS start-up
 */
void cbServiceUp( const string& serviceName, NLNET::TServiceId serviceId, void *cb )
{
	if ( serviceName == "MS" )
	{
		if ( CUnifiedNetwork::getInstance()->isServiceLocal( serviceId ) )
		{
			//nlwarning( "Error: Another mirror service on the same computer is detected!" ); // commented out because now the seconds instance is not granted to start
		}
		else 
		{
			MSInstance->addRemoteMS( serviceId );
		}
	}
	else if ( serviceName == RANGE_MANAGER_SERVICE )
	{
		MSInstance->setRangeManagerReady( true );
	}
	else if ( serviceName != "WS" && serviceName != "SBS"  && serviceName != "DSS" ) // workaround for MIRO warning on the welcome service, sbs & dss
	{
		MSInstance->testAndSendMirrorsOnline( (TServiceId)serviceId );

		if ( CUnifiedNetwork::getInstance()->isServiceLocal( serviceId ) )
		{
			MSInstance->testIfNotInQuittingServices( (TServiceId)serviceId );
		}
	}
}


/*
 *
 */
void cbServiceDown( const string& serviceName, NLNET::TServiceId serviceId, void *cb )
{
	if ( serviceName == RANGE_MANAGER_SERVICE )
	{
		MSInstance->setRangeManagerReady( false );
	}
	else if ( CUnifiedNetwork::getInstance()->isServiceLocal( serviceId )
			  && (serviceName != "WS" && serviceName != "SBS" && serviceName != "DSS") ) // workaround for MIRO warning on the welcome service, sbs & dss
	{
		// Remove all entities owned by the service, and release ranges to Range Manager
		MSInstance->releaseRangesOfService( serviceId );

		// Give one tick to apply the row removals from the quitting service, before releasing the trackers and unsubscribing
		MSInstance->deferTrackerRemovalForQuittingService( (TServiceId)serviceId );

		CTickProxy::removeService( serviceId );

		// TODO: Remove the trackers to other services if the leaving client is the last one able to write
		// into the trackers
	}
	else if ( serviceName == string("MS") )
	{
		MSInstance->removeRemoteMS( serviceId );
	}
}


/*
 * Change MaxOutBandwidth in real-time.
 */
void cfcbChangeMaxOutBandwidth( CConfigFile::CVar& var )
{
	uint32 mob = var.asInt();
	TNDataSetsMS::iterator ids;
	for ( ids=MSInstance->NDataSets.begin(); ids!=MSInstance->NDataSets.end(); ++ids )
		GET_NDATASET(ids).setMaxOutBandwidth( mob );
	nlinfo( "\tMax output bandwidth per dataset per MS set to %g KB", (float)mob/1024.0f );
}


extern void cbOnMasterTick();
extern void cbOnMasterSync();


/*
 * Initialization
 */
void	CMirrorService::init()
{
	setVersion (RYZOM_VERSION);

	MSInstance = this;

	nlinfo( "MSV%s (ChangeTrackerHeader: %u bytes)", MirrorServiceVersion.c_str(), sizeof(TChangeTrackerHeader) );

	// Config file management
	nlinfo( "Reading config file..." );
	const char *mobstr = "MaxOutBandwidth";
	CConfigFile::CVar *varB = ConfigFile.getVarPtr( mobstr );
	CConfigFile::CVar *varD = ConfigFile.getVarPtr( "DestroyGhostSegments" );
	if ( varD )
	{
		DestroyGhostSharedMemSegments = (varD->asInt() == 1);
		nlinfo( "\tDestroyGhostSegments mode is %s", DestroyGhostSharedMemSegments?"on":"off" );
	}
	CConfigFile::CVar *varF = ConfigFile.getVarPtr( "PureReceiverMode" );
	if ( varF && (varF->asInt() == 1) )
	{
		_IsPureReceiver = true;
		nlinfo( "\tThis MS is in pure receiver mode" );
	}

	// Register to ServiceUp and ServiceDown
	CUnifiedNetwork::getInstance()->setServiceUpCallback( "*", cbServiceUp, 0 );
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "*", cbServiceDown, 0 );

	// Initialize the tick system
	nlinfo( "Initializing tick proxy subsystem..." );
	CTickProxy::init( cbOnMasterTick, cbOnMasterSync );

	// Read sheets
	nlinfo( "Loading sheets..." );

	// Fill temporary sheet map, loading dataset information
	TSDataSetSheets sDataSetSheets;
	loadForm( "dataset", IService::getInstance()->WriteFilesDirectory.toString()+"datasets.packed_sheets", sDataSetSheets );
	if ( sDataSetSheets.empty() )
	{
		nlwarning( "No dataset found, check if dataset.packed_sheets and the georges sheets are in the path" );
	}

	// Init all the CDataSetMS objects from the TDataSetSheets
	TSDataSetSheets::iterator ism;
	for ( ism=sDataSetSheets.begin(); ism!=sDataSetSheets.end(); ++ism )
	{
		//nlinfo( "\tDataset: %s", (*ism).second.DataSetName.c_str() );
			
		// Create a CDataSetMS and insert into _SDataSets
		CDataSetMS newDataSet;
		pair<TSDataSetsMS::iterator,bool> res = _SDataSets.insert( make_pair( (*ism).first, newDataSet ) );

		// Init CDataSetMS object
		GET_SDATASET(res.first).init( (*ism).first, (*ism).second, _PropertiesInMirror );

		// Insert into _NDataSets
		NDataSets.insert( make_pair( GET_SDATASET(res.first).name(), &GET_SDATASET(res.first) ) );
	}
	nlinfo( "%u datasets loaded", _SDataSets.size() );

	if ( varB )
	{
		cfcbChangeMaxOutBandwidth( *varB );
	}
	else
	{
		nlinfo( "\tUsing default max output bandwidth" );
	}
	ConfigFile.setCallback( mobstr, cfcbChangeMaxOutBandwidth );
}


/*
 * Release
 */
void	CMirrorService::release()
{
	// This code is commented out because:
	//- The releasing of ranges will be automatically done in the range mirror manager when detecting the MS is down
	//- The removal of client services will be done when removing the MS by any remote MS
	//- We can't send the removals of entities are we are quitting now!
	/*TClientServices::iterator ics;
	for ( ics=_ClientServices.begin(); ics!=_ClientServices.end(); ++ics )
	{
		releaseRangesOfService( (*ics).first );
	}*/

	destroyPropertySegments();
}


/*
 * Tell if the range manager service is up or down
 */
void	CMirrorService::setRangeManagerReady( bool b )
{
	if ( b )
	{
		if ( _RangeManagerReady )
		{
			nlwarning( "ERROR: Detected multiple range manager service (%s)!", RANGE_MANAGER_SERVICE.c_str() );
		}
		else
		{
			_RangeManagerReady = b;

			// Reacquire ranges if the range manager fell down with some ranges acquired
			CMessage msgout( "RARG" ); // Re-acquire range
			NLNET::TServiceId::size_type nbServices = (NLNET::TServiceId::size_type) _ClientServices.size();
			if ( nbServices != 0 )
			{
				uint nbSerialized = 0;
				msgout.serial( nbServices );
				TClientServices::const_iterator ics;
				for ( ics=_ClientServices.begin(); ics!=_ClientServices.end(); ++ics )
				{
//					msgout.serial( const_cast<uint16&>((*ics).first) ); // serviceId
					nlWrite(msgout, serial, ics->first);
					uint16 nbRanges = (uint16)(GET_CLIENT_SERVICE_INFO(ics).OwnedRanges.size());
					msgout.serial( nbRanges );
					if ( nbRanges != 0 )
					{
						vector<TClientServiceInfoOwnedRange>::const_iterator ior;
						for ( ior=GET_CLIENT_SERVICE_INFO(ics).OwnedRanges.begin(); ior!=GET_CLIENT_SERVICE_INFO(ics).OwnedRanges.end(); ++ior )
						{
							msgout.serial( const_cast<string&>((*ior).DataSet->name()) );
							msgout.serial( const_cast<TDataSetIndex&>((*ior).First) );
							msgout.serial( const_cast<TDataSetIndex&>((*ior).Last) );
							++nbSerialized;
						}
					}
				}
				if ( nbSerialized != 0 )
				{
					CUnifiedNetwork::getInstance()->send( RANGE_MANAGER_SERVICE, msgout );
					nlinfo( "ROWMGT: Requested reacquisition of %u ranges for %u services", nbSerialized, nbServices );
				}
			}

			testAndSendMirrorsOnline();
		}
	}
	else
	{
		_RangeManagerReady = false;
		_MirrorsOnline = false;
		nlwarning( "ERROR: Range manager service (%s) is down!", RANGE_MANAGER_SERVICE.c_str() );

		// Subsequent range acquisitions would fail, but because the RANGE_MANAGER_SERVICE is currently
		// the TICKS, they are very improbable.
	}
}


/*
 * Add a MS
 */
void	CMirrorService::addRemoteMS( NLNET::TServiceId serviceId )
{
	++_NumberOfOnlineMS;
	nldebug( "Adding remote MS at %u (now %hd MS)", CTickProxy::getGameCycle(), _NumberOfOnlineMS );
	testIfNotInQuittingServices( (TServiceId)serviceId );
	_RemoteMSList.addRemoteMS( _SDataSets, (TServiceId)serviceId );

	testAndSendMirrorsOnline();

	synchronizeSubscriptionsToNewMS( (TServiceId)serviceId );

	if ( _TimeOfLatestEmittedBytesAvg == 0 )
		_TimeOfLatestEmittedBytesAvg = CTime::getLocalTime();
}


/*
 * Remove a MS
 */
void	CMirrorService::removeRemoteMS( NLNET::TServiceId serviceId )
{
	// Reset tags
	for ( uint i=0; i!=256; ++i )
	{
		if ( _RemoteMSList.getCorrespondingMS( TServiceId(i) ) == serviceId )
			setNewTag( TServiceId(i), IniTag );
	}
	setNewTag( serviceId, IniTag );

	// Remove remote MS and trackers
	_RemoteMSList.removeRemoteMS( (TServiceId)serviceId );
	removeTrackers( serviceId );
	--_NumberOfOnlineMS;

	// Remove remote MS from the "delta update expectation list"
	list<NLNET::TServiceId>::iterator itm = find( _RemoteMSToWaitForDelta.begin(), _RemoteMSToWaitForDelta.end(), serviceId );
	if ( itm != _RemoteMSToWaitForDelta.end() )
	{
		_RemoteMSToWaitForDelta.erase( itm );
		--_NbExpectedDeltaUpdates;
	}

	doNextTask();
}


/*
 * If all the expected MS and the range manager service are ready, send a message to all the local clients
 */
void	CMirrorService::testAndSendMirrorsOnline()
{
	if ( _RangeManagerReady /*&& (_NumberOfOnlineMS == _NumberOfMachines)*/ )
	{
		const vector<NLNET::TServiceId>& vec = CUnifiedNetwork::getInstance()->getConnectionList();
		vector<NLNET::TServiceId>::const_iterator ics;
		for ( ics=vec.begin(); ics!=vec.end(); ++ics )
		{
			testAndSendMirrorsOnline( (TServiceId)(*ics) );
		}
		_MirrorsOnline = true;
	}
}


/*
 * If all the expected MS and the range manager service are ready, send a message to the specified local clients
 */
void	CMirrorService::testAndSendMirrorsOnline( TServiceId servId )
{
	if ( CUnifiedNetwork::getInstance()->isServiceLocal( servId )
		&& (CUnifiedNetwork::getInstance()->getServiceName( servId ) != RANGE_MANAGER_SERVICE)
		&& _RangeManagerReady /*&& (_NumberOfOnlineMS == _NumberOfMachines)*/ )
	{
		CMessage msgout( "MIRO" ); // Mirrors Online
		msgout.serial( _NumberOfOnlineMS );
		msgout.serial( const_cast<string&>(MirrorServiceVersion) );
		CUnifiedNetwork::getInstance()->send( servId , msgout );
	 }
}


/*
 * Wait before doing rescan for service
 */
void	CMirrorService::deferRescanOfEntitiesForNewService( TServiceId serviceId )
{
	_ServicesWaitingForRescan.push_back( make_pair(9,serviceId) );
	nldebug( "Defering rescan for service %hu", serviceId.get() );
}


/*
 * Wait one tick before applying the row removals from the quitting service, before releasing the trackers and unsubscribing
 * It works because the rate of emitted row management is not limited!
 *
 * Note: because the trackers remain in the tracker list while the corresponding services are disconnected,
 * it is necessary to check which services are up when sending the trackers to new client services
 * (see in allocateEntityTrackers() and processPropSubscription()).
 */
void	CMirrorService::deferTrackerRemovalForQuittingService( TServiceId servId )
{
	_QuittingServices.push_back( make_pair(2,servId) ); // 2 because we will skip current onTick()
	nldebug( "Defering removal of trackers of leaving service %hu at %u", servId.get(), CTickProxy::getGameCycle() );
}


/*
 * Test if new service was not in pending list; if it is, apply quitting
 */
void	CMirrorService::testIfNotInQuittingServices( TServiceId servId )
{
	list<TServiceAndTimeLeft>::iterator it;
	for ( it=_QuittingServices.begin(); it!=_QuittingServices.end(); ++it )
	{
		if ( (*it).second == servId )
		{
			nlinfo( "New service %s has the same serviceId as our leaving client, bailing out at %u", servStr(servId).c_str(), CTickProxy::getGameCycle() );
			applyQuittingOf( servId );
			_QuittingServices.erase( it );
			computeNewMainTag();
			break;
		}
	}
}


/*
 * Do the unsubscription and tracker removal job when all entity removals about a quitting have been applied
 */
void	CMirrorService::applyServicesQuitting()
{
	H_AUTO(applyServicesQuitting);

	// Decrement the time counters
	list<TServiceAndTimeLeft>::iterator its;
	for ( its=_QuittingServices.begin(); its!=_QuittingServices.end(); ++its )
	{
		--((*its).first);
	}

	// When time to do the job, do it!
	bool sthgToDo = false;
	while ( (! _QuittingServices.empty()) && (_QuittingServices.front().first == 0) )
	{
		applyQuittingOf( _QuittingServices.front().second );
		_QuittingServices.pop_front();
		sthgToDo = true;
	}

	if ( sthgToDo )
	{
		computeNewMainTag();
	}
}


/*
 *
 */
void	CMirrorService::applyQuittingOf( NLNET::TServiceId clientServiceId )
{
	nldebug( "Applying unsubscription and tracker removal for leaving service %hu", _QuittingServices.front().second.get() );

	/// Tell all other MS one service that had subscribed to some properties is leaving
	broadcastUnsubscribeProperties( clientServiceId );

	// Remove the trackers of the service
	removeTrackers( clientServiceId );
}


/*
 *
 */
void	CMirrorService::computeNewMainTag()
{
	// Update main MTR tag, and send to all other MS if changed
	TMTRTag prevTag = mainTag().rawTag();
	mainTag().reset();
	for ( TClientServices::const_iterator ics=_ClientServices.begin(); ics!=_ClientServices.end(); ++ics )
	{
		mainTag().addTag( getTagOfService( (TServiceId)((*ics).first) ).rawTag() );
	}
	if ( mainTag().rawTag() != prevTag )
	{
		nldebug( "applyServicesQuitting: MainTag: Before: %d After: %d", prevTag, mainTag().rawTag() );
		CMessage msgout( "TAG_CHG" );
		msgout.serial( mainTag() );
		CUnifiedNetwork::getInstance()->send( "MS", msgout, false );
	}
}


/*
 * Do the rescans if it's time to do so
 */
void	CMirrorService::applyPendingRescans()
{
	H_AUTO(applyPendingRescans);

	// Decrement the time counters
	list<TServiceAndTimeLeft>::iterator its;
	for ( its=_ServicesWaitingForRescan.begin(); its!=_ServicesWaitingForRescan.end(); ++its )
	{
		--((*its).first);
	}

	// When time to do the job, do it!
	while ( (! _ServicesWaitingForRescan.empty()) && (_ServicesWaitingForRescan.front().first == 0) )
	{
		rescanEntitiesForNewService( _ServicesWaitingForRescan.front().second );
		_ServicesWaitingForRescan.pop_front();
	}
}


/*
 * Delete a tracker from its vector
 */
void	CMirrorService::deleteTracker( CChangeTrackerMS& tracker, std::vector<CChangeTrackerMS>& vect )
{
	nldebug( "TRACKER: Removing 1 tracker for service %s (%s)", 
		servStr(tracker.destServiceId()).c_str(),
		tracker.isAllocated() ? toString("smid %d", tracker.smid()).c_str() : "notallocd" );

	if ( tracker.destroy() )
	{
		_SMIdPool.releaseId( tracker.smid() );
		_MutIdPool.releaseId( tracker.mutid() );
	}

	// Delete the tracker object
	if ( vect.size() > 1 )
	{
		tracker.~CChangeTrackerMS();
		tracker = vect.back();
	}
	vect.pop_back();
}


/*
 * Remove all trackers of the specified service, and tell all services that have them to do the same
 */
void	CMirrorService::removeTrackers( NLNET::TServiceId serviceId )
{
	//nlinfo( "REMOVE TRACKERS %s", servStr(serviceId).c_str() );
	// Vectors of smids, indexed by serviceId
	map< NLNET::TServiceId, vector<sint32> > servicesForTrackerRemoval;

	// Browse datasets
	TSDataSetsMS::iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		CDataSetMS& dataset = GET_SDATASET(ids);
		//nlinfo( "DATASET %s", dataset.name().c_str() );

		sint32 eti;
		for ( eti=ADDING; eti<=REMOVING; ++eti )
		{
			//nlinfo( "ENTITY TRACKER %d", eti );
			// Find the tracker corresponding to the specified service
			CChangeTrackerMS *ptracker = NULL;
			TSubscriberListForEnt::iterator isl;
			for ( isl=dataset._EntityTrackers[eti].begin(); isl!=dataset._EntityTrackers[eti].end(); ++isl )
			{
				if ( (*isl).destServiceId() == serviceId )
				{
					ptracker = &(*isl);
					break;
				}
			}

			// Check if tracker found in the current dataset
			if ( (! ptracker) /*|| (!ptracker->isAllocated())*/ )
				break; // no entity tracker

			//nlinfo( "FOUND ENTITY TRACKER" );
			
			// Tell every other local service to remove the tracker
			for ( isl=dataset._EntityTrackers[eti].begin(); isl!=dataset._EntityTrackers[eti].end(); ++isl )
			{
				if ( (*isl).isLocal() && ((*isl).destServiceId() != serviceId) )
				{
					//nlinfo( "ADDING TO REMOVE LIST OF %s", servStr((*isl).destServiceId()).c_str() );
					servicesForTrackerRemoval[(*isl).destServiceId()].push_back( ptracker->smid() );
				}
			}

			// Erase the entity tracker (move the last one instead)
			deleteTracker( *ptracker, dataset._EntityTrackers[eti] );
			//nlinfo( "AFTER REMOVE ENTITY TRACKER [%d]", eti ); dataset.displayTrackers();
		}

		TPropertyIndex propIndex;
		for ( propIndex=0; propIndex!= dataset.nbProperties(); ++propIndex )
		{
			//nldebug( "PROP TRACKER %Phd", propIndex );
			// TODO: Skip properties not declared

			// Find the tracker corresponding to the specified service and the current property
			CChangeTrackerMS *ptracker = NULL;
			TSubscriberListForProp::iterator isl;
			for ( isl=dataset._SubscribersByProperty[propIndex].begin(); isl!=dataset._SubscribersByProperty[propIndex].end(); ++isl )
			{
				if ( (*isl).destServiceId() == serviceId )
				{
					ptracker = &(*isl);
				}
			}

			// Check if tracker found in the current dataset, for the current property
			if ( (! ptracker) /*|| (! ptracker->isAllocated())*/ )
				continue;
			//nldebug( "ABOUT TO REMOVE PROP TRACKER P%hd", propIndex );


			// Tell every other local service that has the tracker to remove it
			for ( isl=dataset._SubscribersByProperty[propIndex].begin(); isl!=dataset._SubscribersByProperty[propIndex].end(); ++isl )
			{
				if ( (*isl).isLocal() && ((*isl).destServiceId() != serviceId) )
				{
					//nlinfo( "ADDING TO REMOVE LIST OF %s", servStr((*isl).destServiceId()).c_str() );
					// In fact we don't check if the service has the tracker, but no matter if it hasn't, it will just ignore the smid
					servicesForTrackerRemoval[(*isl).destServiceId()].push_back( ptracker->smid() );
				}
			}

			// Decrement the local writer prop counter
			if ( ptracker->isLocal() && (! ptracker->isReadOnly()) )
			{
#ifdef NL_DEBUG
				nlassert( dataset._NbLocalWriterProps != 0 );
#endif
				dataset.decNbLocalWriterProps();
				nldebug( "%s_NbLocalWriterProps down to %u", dataset.name().c_str(), dataset._NbLocalWriterProps );
			}

			// Erase the property tracker (move the last one)
			deleteTracker( *ptracker, dataset._SubscribersByProperty[propIndex] );
			//nlinfo( "AFTER REMOVE PROP TRACKER FOR PROP %hd", propIndex ); dataset.displayTrackers();
		}
	}

	sendTrackersToRemoveToLocalServices( servicesForTrackerRemoval );
}


/*
 * Send RT to local services
 */
void	CMirrorService::sendTrackersToRemoveToLocalServices( map< NLNET::TServiceId, vector<sint32> >& servicesForTrackerRemoval )
{
	// Send the smids to release to the other services
	map< NLNET::TServiceId, vector<sint32> >::iterator it;
	for ( it=servicesForTrackerRemoval.begin(); it!=servicesForTrackerRemoval.end(); ++it )
	{
		CMessage msgout( "RT" );
		msgout.serialCont( (*it).second );
		CUnifiedNetwork::getInstance()->send( (TServiceId)((*it).first), msgout ); // can produce the warning "HNETL5: Can't find selected connection id"... if several services are disconnected at the same moment
		nlinfo( "TRACKER: Sent Release Trackers (RT) with %u smids to service %s", (*it).second.size(), servStr((*it).first).c_str() );
	}
}


/*
 * Browse all datasets and build deltas to remote MS, then send them
 */
void	CMirrorService::buildAndSendAllDeltas()
{
	H_AUTO(buildAndSendAllDeltas);

	TTime BeforeBuildAndSendAllDeltas = CTime::getLocalTime();

	// Browse datasets
	TSDataSetsMS::iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		CDataSetMS& dataset = GET_SDATASET(ids);
		
		// Prevent useless filling and sending
		if ( dataset._NbLocalWriterProps != 0 ) // TODO: change criterion: one service could spawn entities without writing any properties
		{
			// Browse non-local, non-blocked, entity trackers

			// ADDING
			TSubscriberListForEnt::iterator isl;
			for ( isl=dataset._EntityTrackers[ADDING].begin(); isl!=dataset._EntityTrackers[ADDING].end(); ++isl )
			{
				CChangeTrackerMS& tracker = (*isl);

				if ( ! tracker.isLocal() )
				{
					CDeltaToMS *delta = tracker.getLinkToDeltaBuffer();
					if ( delta && delta->mustTransmitAllBindingCounters() )
					{
						// Transmit all the binding counters (important for offline entities)
						dataset.transmitAllBindingCounters( delta, _ClientServices );
					}

					if ( ! tracker.isAllowedToSendToRemoteMS() )
					{
						if ( tracker.needsSyncAfterAllATEAcknowledgesReceived() )
						{
							// "Sync delta". Switches tracker.isAllowedToSendToRemoteMS() to true (and the corresponding removal one as well).
							syncOnlineLocallyManagedEntitiesIntoDeltaAndOpenEntityTrackersToRemoteMS( delta, dataset, tracker, (uint)(isl-dataset._EntityTrackers[ADDING].begin()) );
						}
						else
						{
							// Don't push the changes if the tracker is blocked
							continue;
						}
					}

					//nlassert( tracker.isAllowedToSendToRemoteMS() );
#ifdef COUNT_MIRROR_PROP_CHANGES
					tracker.storeNbChanges(); // store before popping the changes
#endif
					// Push additions into delta buffer (with whole actual dataset row information)
					if ( delta )
					{
						if ( delta->beginRowManagement( tracker, ADDING ) )
						{
#ifdef DISPLAY_DELTAS
							nldebug( "ROWMGT: Addition delta:" );
#endif

							//nldebug( "Pushing row management %u", (uint32)eti );
							TDataSetRow datasetrow ( tracker.getFirstChanged() );

							/// Push changes until no more changes, pop changes from tracker
							while ( datasetrow != LAST_CHANGED ) // limiting the size would lead to inconsistencies
							{
								datasetrow.setCounter( dataset.getBindingCounter( datasetrow.getIndex() ) );
								delta->pushRowManagement( ADDING, datasetrow );
#ifdef DISPLAY_DELTAS
								DebugLog->displayRawNL( "%u_%hu", datasetrow.getIndex(), datasetrow.counter() );
#endif
								tracker.popFirstChanged();
								datasetrow.initFromIndex( tracker.getFirstChanged() );
							}

							delta->endRowManagement( ADDING );
						}
					}
				}
			}

			// REMOVING
			for ( isl=dataset._EntityTrackers[REMOVING].begin(); isl!=dataset._EntityTrackers[REMOVING].end(); ++isl )
			{
				CChangeTrackerMS& tracker = (*isl);

				if ( ! tracker.isLocal() )
				{
					CDeltaToMS *delta = tracker.getLinkToDeltaBuffer();
					if ( tracker.isAllowedToSendToRemoteMS() )
					{
#ifdef COUNT_MIRROR_PROP_CHANGES
						tracker.storeNbChanges(); // store before popping the changes
#endif
						// Push removals into delta buffer (with whole actual dataset row information)
						if ( delta )
						{
							if ( delta->beginRowManagement( tracker, REMOVING ) )
							{
#ifdef DISPLAY_DELTAS
								nldebug( "ROWMGT: Removal delta:" );
#endif
								//nldebug( "Pushing row management %u", (uint32)eti );
								TDataSetRow datasetrow ( tracker.getFirstChanged() );

								/// Push changes until no more changes, pop changes from tracker
								while ( datasetrow != LAST_CHANGED ) // limiting the size would lead to inconsistencies
								{
									datasetrow.setCounter( dataset.getBindingCounter( datasetrow.getIndex() ) );
									delta->pushRowManagement( REMOVING, datasetrow );
#ifdef DISPLAY_DELTAS
									DebugLog->displayRawNL( "%u_%hu", datasetrow.getIndex(), datasetrow.counter() );
#endif
									tracker.popFirstChanged();
									datasetrow.initFromIndex( tracker.getFirstChanged() );
								}

								delta->endRowManagement( REMOVING );
							}
						}
					}
				}
			}

			// Browse non-local, non-blocked, property trackers
			TPropertyIndex propIndex;
			for ( propIndex=0; propIndex!= dataset.nbProperties(); ++propIndex )
			{
				// TODO: Skip properties not declared
				TSubscriberListForProp::iterator isl;
				for ( isl=dataset._SubscribersByProperty[propIndex].begin(); isl!=dataset._SubscribersByProperty[propIndex].end(); ++isl )
				{
					CChangeTrackerMS& tracker = (*isl);

					if ( (! tracker.isLocal()) && tracker.isAllowedToSendToRemoteMS() )
					{
#ifdef COUNT_MIRROR_PROP_CHANGES
						tracker.storeNbChanges(); // store before popping the changes
#endif
						switch ( dataset.getPropType( propIndex ) )
						{
						case TypeUint8: pushPropertyChanges( dataset, tracker, propIndex, (uint8*)NULL ); break;
						case TypeSint8: pushPropertyChanges( dataset, tracker, propIndex, (sint8*)NULL ); break;
						case TypeUint16: pushPropertyChanges( dataset, tracker, propIndex, (uint16*)NULL ); break;
						case TypeSint16: pushPropertyChanges( dataset, tracker, propIndex, (sint16*)NULL ); break;
						case TypeUint32: pushPropertyChanges( dataset, tracker, propIndex, (uint32*)NULL ); break;
						case TypeSint32: pushPropertyChanges( dataset, tracker, propIndex, (sint32*)NULL ); break;
						case TypeUint64: pushPropertyChanges( dataset, tracker, propIndex, (uint64*)NULL ); break;
						case TypeSint64: pushPropertyChanges( dataset, tracker, propIndex, (sint64*)NULL ); break;
						case TypeFloat: pushPropertyChanges( dataset, tracker, propIndex, (float*)NULL ); break;
						case TypeDouble: pushPropertyChanges( dataset, tracker, propIndex, (double*)NULL ); break;
						case TypeCEntityId: pushPropertyChanges( dataset, tracker, propIndex, (CEntityId*)NULL ); break;
						case TypeBool: pushPropertyChanges( dataset, tracker, propIndex, (bool*)NULL ); break;
						default: nlwarning( "Unknown type for writing" );
						}
					}
				}
			}
			// The INVALID_PROPERTY_INDEX end flag is set in sendAllDeltas
		}
	}

	sendAllDeltas();

	CTickProxy::TimeMeasures.MirrorMeasure[BuildAndSendDeltaDuration] = (uint16)(CTime::getLocalTime() - BeforeBuildAndSendAllDeltas);
}


/*
 * Fill a remote entity tracker with the locally managed entities
 *
 * - Browse all the entities and add the online entities belonging to local services to the delta to the MS,
 *   will full dataset row.
 * - Open (unblock) the entity trackers to the remote MS.
 */
void	CMirrorService::syncOnlineLocallyManagedEntitiesIntoDeltaAndOpenEntityTrackersToRemoteMS( CDeltaToMS *delta, CDataSetMS& dataset, CChangeTrackerMS& entityTrackerAdding, uint entityTrackerIndexInVector )
{
	// We don't clear EntityTrackerAdding and EntityTrackerRemoving, as they may contain changes for other services
	// than the new one that triggered the sync. Ex:
	// Before: S1(0) MS_HERE -> REMOTE_MS S2(1)         S1 sends changes to S2
	// After:  S1(0) MS_HERE -> REMOTE_MS S2(1) S3(0)   S1 sends changes to S2 and sync to S3

	// Browse all the entities and add the online entities belonging to local services
	// to the delta to the MS, will full dataset row (filtering with dest MS tag).
	uint nbChanges = 0;
	sint32 sizeBefore = delta->getDeltaBuffer().getPos();
	vector<uint8> onlineEntityCounters( dataset.maxNbRows(), 0 ); // 0 is a skipped counter
	delta->beginRowManagementSync( entityTrackerAdding.remoteClientServicesToSync()/*, _ClientServices*/ );
	TDataSetIndex entityIndex;
#ifdef DISPLAY_DELTAS
	nldebug( "ROWMGT: Sync delta contains, for the new service(s):" );
#endif
	const CMTRTag& destTag = getTagOfService( entityTrackerAdding.destServiceId() );
	for ( entityIndex=0; entityIndex!=dataset.maxNbRows(); ++entityIndex )
	{
		if ( dataset._PropertyContainer.EntityIdArray.isOnline( entityIndex ) )
		{
			const CEntityId& entityId = dataset._PropertyContainer.EntityIdArray.EntityIds[entityIndex];
			NLNET::TServiceId8 spawnerId = dataset.getSpawnerServiceId( entityIndex );
			if ( _ClientServices.find( spawnerId ) != _ClientServices.end() )
			{
				if ( destTag.doesAcceptTag( getTagOfService( spawnerId ) ) )
				{
					TDataSetRow datasetrow = dataset.getCurrentDataSetRow( entityIndex );
					delta->pushRowManagementSync( datasetrow );
					onlineEntityCounters[entityIndex] = datasetrow.counter();
#ifdef DISPLAY_DELTAS
					DebugLog->displayRawNL( " %u_%hu", datasetrow.getIndex(), datasetrow.counter() );
#endif
					++nbChanges;
				}
			}
		}
	}
	delta->endRowManagementSync();

	// IMPORTANT: a client service must not add/remove an entity in non mirror-synchronized code
	// (i.e. outside of the tick update, of the mirror notification callback or of a message callback
	// of a message sent "via mirror" (including CMirrorTransportClass callbacks)).
	// Otherwise, a change can be filled while we're here. This would lead to a change *not notified*
	// to the new service(s) listed in entityTrackerAdding.remoteClientServicesToSync().

	// Open (unblock) the entity trackers to the remote MS.
	// It's not a problem to have blocked sending for a short while, because when we get all acks we
	// get here and unblock (compliant with snapshots of SLV test service).
	// If some entities were added or removed by another service in the current tick, they will
	// be sent in the same delta (see buildAndSendDeltas()). But they won't be notified twice
	// to the new service(s) (listed in entityTrackerAdding.deltaremoteClientServicesToSync())
	// thanks to the exclude list passed to setupDestEntityTrackersInterestedByDelta().
	entityTrackerAdding.allowSendingToRemoteMS( true );
	CChangeTrackerMS& entityTrackerRemoving = dataset._EntityTrackers[REMOVING][entityTrackerIndexInVector];
	nlassert( entityTrackerRemoving.destServiceId() == entityTrackerAdding.destServiceId() );
	entityTrackerRemoving.allowSendingToRemoteMS( true );

	sint sizeOfSync = delta->getDeltaBuffer().getPos() - sizeBefore;
	nlinfo( "ROWMGT: Delta to remote MS has a sync of %u entities (%.3f KB)", nbChanges, ((float)sizeOfSync) / 1024.0f );
}


/*
 * Push the property changes on the local machine for a remote mirror service to its delta buffer
 */
template <class T>
void	CMirrorService::pushPropertyChanges( CDataSetMS& dataset, CChangeTrackerMS& tracker, TPropertyIndex propIndex, T *typ )
{
	// Push changes into delta buffer
	CDeltaToMS *delta = tracker.getLinkToDeltaBuffer();
	if ( delta )
	{
		if ( delta->beginPropChanges( tracker, propIndex ) )
		{
			//nldebug( "Pushing prop changes %hd", propIndex );
			TDataSetRow datasetrow( tracker.getFirstChanged() );

			/// Push changes until full or no more changes, pop changes from tracker
			while ( (datasetrow != LAST_CHANGED) && (! delta->full( propIndex )) )
			{
				if ( datasetrow.isNull() )
				{
					nlwarning( "Tracker %s-P%hd corrupted", CUnifiedNetwork::getInstance()->getServiceUnifiedName( tracker.destServiceId() ).c_str(), propIndex );
					break;
				}
				else
				{
					datasetrow.setCounter( dataset.getBindingCounter( datasetrow.getIndex() ) );
					//MIRROR_DEBUG( "MIRROR: Pushing prop change for entity %u", datasetrow.getIndex() );
					delta->pushPropChange( propIndex, datasetrow, typ );
					tracker.popFirstChanged();
					datasetrow.initFromIndex( tracker.getFirstChanged() );
				}
			}

			delta->endPropChanges( propIndex );
			//nldebug( "Prop changes pushed for P%hd: %d", propIndex, delta->getNbChangesPushed() );
		}
	}
#ifdef NL_DEBUG
	else
		nlstop;
#endif
}


/*
 * Send the delta buffers to the remote mirror services
 */
void	CMirrorService::sendAllDeltas()
{
	H_AUTO(sendAllDeltas);
	//nldebug( "%u: Send Delta", CTickProxy::getGameCycle() );

	// Browse the other known Mirror Services
	TServiceIdList::iterator isl;
	for ( isl=_RemoteMSList.begin(); isl!=_RemoteMSList.end(); ++isl )
	{
		// Serialize the delta buffers of each dataset
		TDeltaToMSList& deltaList = _RemoteMSList.getDeltaToMSList( (*isl) );

		CMessage msgout( "DELTA" );

		// Put "wait for" mode (warning: it must not change during our lifetime)
		bool waitForMe = ! _IsPureReceiver;
		msgout.serial( waitForMe );

		// Put current tick in delta
		TGameCycle gamecycle = CTickProxy::getGameCycle();
		msgout.serial( gamecycle );

		TDeltaToMSList::iterator idl;
		for ( idl=deltaList.begin(); idl!=deltaList.end(); ++idl )
		{
			if ( (*idl)->getDeltaBuffer().length() != 0 )
			{
				/// If this was not done before (i.e. when starting mirror delta), update the message counter in the delta buffer (messages are pushed in CRemoteMSList::storeMessageForRemoteClientService())
				if ( ! (*idl)->hasMirrorDelta() )
					(*idl)->endMessages();

				// Fill the sheet id of the dataset and the header byte & close prop management (work even there was no beginMirrorDelta())
				(*idl)->endMirrorDelta();

				// Serial the delta buffer of the dataset
				msgout.serialBuffer( const_cast<uint8*>((*idl)->getDeltaBuffer().buffer()), (*idl)->getDeltaBuffer().length() );
				//nldebug( "Delta buffer (len = %u): %s", (*idl)->getDeltaBuffer().length(), (*idl)->getDeltaBuffer().toString( true ).c_str() );

				// Reset the delta buffer
				(*idl)->clearBuffer();
			}

			(*idl)->storeNbChangesPushed();
		}

		// Send
		uint32 len = msgout.length();
		if ( VerboseMessagesSent && len > (5+5+6*3)*1 ) //TEMP
			nlinfo( "Sending message (%u bytes) to %hu", len, isl->get());
		CUnifiedNetwork::getInstance()->send( (*isl), msgout );

		// Compute output rate stats
		_EmittedBytesPartialSum += len;
	}

	computeDeltaStats();
}


/*
 * Compute output rate stats if needed (delta + messages)
 */
void	CMirrorService::computeDeltaStats()
{
	TTime localTime = CTime::getLocalTime();
#ifdef NL_OS_WINDOWS
	const uint32 AVG_DELAY_MS = 2000; // for window displayer
#else
	const uint32 AVG_DELAY_MS = 20000; // for admin tool graph
#endif
	if ( localTime > _TimeOfLatestEmittedBytesAvg + AVG_DELAY_MS )
	{
		// EmittedKBPerSecond = (EmittedBytes/1000) / (DeltaTimeMS/1000)
		_EmittedKBytesPerSecond = (float)_EmittedBytesPartialSum / (float)(localTime-_TimeOfLatestEmittedBytesAvg);
		_EmittedBytesPartialSum = 0;
		_TimeOfLatestEmittedBytesAvg = localTime;
	}
}


/*
 * Process a delta
 */
void	CMirrorService::processReceivedDelta( CMessage& msgin, TServiceId serviceId )
{
	H_AUTO(processReceivedDelta);

	const CMTRTag& sourceMSTag = getTagOfService( serviceId );

	// Skip the header, already processed when receiving the delta, before storing it
	bool waitForIt;
	msgin.serial( waitForIt );
	TGameCycle gamecycle;
	msgin.serial( gamecycle );

	uint currentBlock = 0;
	uint32 nbMsgs;
	uint i;
	uint currentState = 0;
	try
	{
		while( (uint32)msgin.getPos() < msgin.length() )
		{
			// Get the messages (if any)
			msgin.serial( nbMsgs );
			for ( i=0; i!=nbMsgs; ++i )
			{
				TServiceId destId;
				msgin.serial( destId );
				if ( destId == DEST_MSG_BROADCAST )
				{
					pushMessageToLocalQueueFromMessage( _MessagesToBroadcastLocally, msgin );
					//CMessage msg( _MessagesToBroadcastLocally.back().Msg ); // TEMP-debug
					//nldebug( "MSG: Received msg from %s to broadcast via MS-%hu (msg %s, %u b)", servStr(_MessagesToBroadcastLocally.back().SenderId).c_str(), serviceId, msg.getName().c_str(), _MessagesToBroadcastLocally.back().Msg.length() );
				}
				else
				{
					TClientServices::iterator ics = _ClientServices.find( destId );
					if ( ics != _ClientServices.end() )
					{
						// Buffer the message for local destination service
						pushMessageToLocalQueueFromMessage( GET_CLIENT_SERVICE_INFO(ics).Messages, msgin );
						//CMessage msg( GET_CLIENT_SERVICE_INFO(ics).Messages.back().Msg ); // TEMP-debug
						//nldebug( "MSG: Received msg from %s to %s via MS-%hu (msg %s, %u b)", servStr(GET_CLIENT_SERVICE_INFO(ics).Messages.back().SenderId).c_str(), servStr(destId).c_str(), serviceId, msg.getName().c_str(), GET_CLIENT_SERVICE_INFO(ics).Messages.back().Msg.length() );
					}
					else
					{
						// Skip message in stream
						TServiceId senderId;
						msgin.serial( senderId );
						CMemStream ms( true );
						msgin.serialMemStream( ms );
						CMessage msg( ms );
						string msgName;
						getNameOfMessageOrTransportClass( msg, msgName );
						nlwarning( "MSG: Can't locate client service %s for message from MS-%hu (%s from %s)", 
							servStr(destId).c_str(), 
							serviceId.get(), 
							msgName.c_str(), 
							servStr(senderId).c_str() );
					}
				}
			}

			//MIRROR_DEBUG( "MIRROR: \tReading delta (pos=%u, length=%u)", (uint32)msgin.getPos(), msgin.length() );
			// Access the right dataset
			CSheetId sheetId;
			msgin.serial( sheetId );
			TSDataSetsMS::iterator isd = _SDataSets.find( sheetId );
			if ( isd != _SDataSets.end() )
			{
				CDataSetMS& dataset = GET_SDATASET(isd);

				// Read the header of the delta buffer
				uint8 header;
				msgin.fastRead( header );
				//nldebug( "HEADER=%hu", (uint16)header );

				// Row management: binding counters
				if ( (header & CDeltaToMS::RowManagementBindingCountersBit) != 0 )
				{
					currentState = CDeltaToMS::RowManagementBindingCountersBit;
					dataset.applyAllBindingCounters( msgin );
				}

				vector<NLNET::TServiceId8> listOfServicesToSync; // will remain empty if there is not sync in this delta => no exclude list provided for additions/removals
				TDataSetRow datasetRow;

				// Row management: sync to new services
				if ( (header & CDeltaToMS::RowManagementSyncBit) != 0 )
				{
					H_AUTO( AddEntitiesSync )
					currentState = CDeltaToMS::RowManagementSyncBit;

					// List of services to notify of the entities
					msgin.serialCont( listOfServicesToSync );

					/*// List of services corresponding to the sync
					uint16 nbRemoteClientServices;
					msgin.serial( nbRemoteClientServices );
					vector<uint16> listOfCreatorServices( nbRemoteClientServices );
					for ( uint i=0; i!=nbRemoteClientServices; ++i )
						msgin.serial( listOfCreatorServices[i] );*/

					// Online entities
					setupDestEntityTrackersInterestedBySyncDelta( dataset, sourceMSTag, listOfServicesToSync );
					nldebug( "%u: Applying sync delta in %s", CTickProxy::getGameCycle(), dataset.name().c_str() );
					CEntityId entityId;
					TServiceId8 spawnerId;
					msgin.fastRead( datasetRow );
					while ( datasetRow.isValid() )
					{
						msgin.fastRead( entityId );
						msgin.fastRead( spawnerId );
						dataset.syncEntityToDataSet( datasetRow, entityId, spawnerId );
						msgin.fastRead( datasetRow );
					}

					/*// Erase entities of creator services that are not in the sync
					dataset.removeAllEntitiesFrom( listOfCreatorServices, CTickProxy::getGameCycle() );*/
				}

				// Row management: adding
				if ( (header & CDeltaToMS::RowManagementAddingBit) != 0 )
				{
					H_AUTO( AddEntities )
					currentState = CDeltaToMS::RowManagementAddingBit;
					setupDestEntityTrackersInterestedByDelta( dataset, sourceMSTag, ADDING, listOfServicesToSync );
					//nldebug( "%u: Applying additions in %s", CTickProxy::getGameCycle(), dataset.name().c_str() );
					CEntityId entityId;
					TServiceId8 spawnerId;
					msgin.fastRead( datasetRow );
					while ( datasetRow != INVALID_DATASET_INDEX )
					{
						msgin.fastRead( entityId );
						msgin.fastRead( spawnerId );
						dataset.addEntityToDataSet( datasetRow, entityId, spawnerId );
						msgin.fastRead( datasetRow );
					}
				}

				// Row management: removing
				if ( (header & CDeltaToMS::RowManagementRemovingBit) != 0 )
				{
					H_AUTO( RemoveEntities )
					currentState = CDeltaToMS::RowManagementRemovingBit;
					setupDestEntityTrackersInterestedByDelta( dataset, sourceMSTag, REMOVING, listOfServicesToSync );
					//nldebug( "%u: Applying removals in %s", CTickProxy::getGameCycle(), dataset.name().c_str() );
					msgin.fastRead( datasetRow );
					while ( datasetRow != INVALID_DATASET_INDEX )
					{
						dataset.removeEntityFromDataSet( datasetRow ); // ignoring the remCounter
						msgin.fastRead( datasetRow );
					}
				}

				// Property changes
				if ( (header & CDeltaToMS::RowManagementPropChangeBit) != 0 )
				{
					H_AUTO( applyPropertyChanges )
					currentState = CDeltaToMS::RowManagementPropChangeBit;
					TPropertyIndex propIndex;
					msgin.fastRead( propIndex );
					//nldebug( "%u: Applying changes of P%hd in %s", CTickProxy::getGameCycle(), propIndex, dataset.name().c_str() );
					while ( propIndex != INVALID_PROPERTY_INDEX )
					{
						setupDestPropTrackersInterestedByDelta( dataset, sourceMSTag, serviceId, propIndex );
						switch( dataset.getPropType( propIndex ) )
						{
							case TypeUint8: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (uint8*)NULL ); break;
							case TypeSint8: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (sint8*)NULL ); break;
							case TypeUint16: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (uint16*)NULL ); break;
							case TypeSint16: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (sint16*)NULL ); break;
							case TypeUint32: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (uint32*)NULL ); break;
							case TypeSint32: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (sint32*)NULL ); break;
							case TypeUint64: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (uint64*)NULL ); break;
							case TypeSint64: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (sint64*)NULL ); break;
							case TypeFloat: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (float*)NULL ); break;
							case TypeDouble: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (double*)NULL ); break;
							case TypeCEntityId: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (CEntityId*)NULL ); break;
							case TypeBool: applyPropertyChanges( msgin, dataset, propIndex, serviceId, (bool*)NULL ); break;
							default: nlwarning( "Unknown type for reading" );
						}
						msgin.fastRead( propIndex );
					}
				}
			}
			else
			{
				nlwarning( "Received delta for unknown dataset %s from %s", sheetId.toString().c_str(), servStr(serviceId).c_str() );
				break;
			}
			++currentBlock;
		}
	}
	catch (const EStreamOverflow&)
	{
		nlwarning( "Stream overflow in received delta from %s, currentBlock=%u nbMsgs=%u iMsg=%u currentState=%u",
			servStr(serviceId).c_str(), currentBlock, nbMsgs, i, currentState );
	}
}


/*
 *
 */
void	CMirrorService::setupDestEntityTrackersInterestedByDelta( CDataSetMS& dataset, const CMTRTag& sourceTag, TEntityTrackerIndex addRem, const std::vector<NLNET::TServiceId8>& listOfExcludedServices )
{
	dataset._DestTrackersForDelta.clear();
	for ( TSubscriberListForEnt::const_iterator it=dataset._EntityTrackers[addRem].begin(); it!=dataset._EntityTrackers[addRem].end(); ++it )
	{
		if ( (*it).isLocal() && // note: (*it).isAllocated() is always true for entity trackers
			 getTagOfService( (TServiceId)((*it).destServiceId()) ).doesAcceptTag( sourceTag ) &&
			 (find( listOfExcludedServices.begin(), listOfExcludedServices.end(), (*it).destServiceId() ) == listOfExcludedServices.end()) )
		{
			dataset._DestTrackersForDelta.push_back( const_cast<CChangeTrackerMS*>(&(*it)) );
		}
	}
}


/*
 *
 */
void	CMirrorService::setupDestEntityTrackersInterestedBySyncDelta( CDataSetMS& dataset, const CMTRTag& sourceTag, std::vector<NLNET::TServiceId8> listOfInterestedServices )
{
	dataset._DestTrackersForDelta.clear();
	for ( TSubscriberListForEnt::const_iterator it=dataset._EntityTrackers[ADDING].begin(); it!=dataset._EntityTrackers[ADDING].end(); ++it )
	{
		if ( (*it).isLocal() && // note: (*it).isAllocated() is always true for entity trackers
			 (find( listOfInterestedServices.begin(), listOfInterestedServices.end(), (*it).destServiceId() ) != listOfInterestedServices.end()) &&
			 getTagOfService( (TServiceId)((*it).destServiceId()) ).doesAcceptTag( sourceTag ) )
		{
			dataset._DestTrackersForDelta.push_back( const_cast<CChangeTrackerMS*>(&(*it)) );
		}
	}
}

/*
 *
 */
void	CMirrorService::setupDestPropTrackersInterestedByDelta( CDataSetMS& dataset, const CMTRTag& sourceTag, NLNET::TServiceId sourceMSId, TPropertyIndex propIndex )
{
	dataset._DestTrackersForDelta.clear();
	for ( TSubscriberListForProp::const_iterator it=dataset._SubscribersByProperty[propIndex].begin(); it!=dataset._SubscribersByProperty[propIndex].end(); ++it )
	{
		if ( (*it).isLocal() && (*it).isAllocated() &&
			 getTagOfService( (TServiceId)((*it).destServiceId()) ).doesAcceptTag( sourceTag ) )
		{
			dataset._DestTrackersForDelta.push_back( const_cast<CChangeTrackerMS*>(&(*it)) );
		}
	}
	// Obsolete: now we don't handle the case anymore when a service can overwrite a property value in the same
	// game cycle as another service.
	/*// Add tracker to list of bounce dest trackers (rare event, the list could be built only 'on demand')
	if ( (*it).isAllowedToSendToRemoteMS() && // skips only blocked remote trackers
		 (wantsTag || ((*it).destServiceId()==sourceMSId)) )
		dataset._DestTrackersForDeltaBounce.push_back( const_cast<CChangeTrackerMS*>(&(*it)) );
	*/
}


/*
 * Declare the datasets used by a client service
 * Possibly received before allocation of entityIds.
 */
void	CMirrorService::declareDataSets( CMessage& msgin, NLNET::TServiceId serviceId )
{
	vector<string> datasetNames;
	TMTRTag tagOfNewClientService;
	msgin.serialCont( datasetNames );
	msgin.serial( tagOfNewClientService );

	// Set tag of new client service, and compute main MS tag
	setNewTag( serviceId, tagOfNewClientService );
	CMTRTag previousTag = mainTag();
	mainTag().addTag( tagOfNewClientService );

	// If the main tag is changing, we've got to prepare for a resync of obsolete remotely managed entities
	// (except if it was AllTag, meaning we always received all entity additions/removals)
	//bool mustEraseObsoleteRemoteEntities = (! (mainTag() == previousTag)) && (previousTag.rawTag() != AllTag);

	// Insert serviceId in _ClientServices (with no owned ranges yet, and of course no pending messages)
	// At this time, the client service is at least ready at level1
	TClientServiceInfo csi;
	_ClientServices.insert( make_pair( serviceId, csi ) ); // the deletion is done in releaseRangesOfServices() (for historical reasons ;)

	// Process the requested datasets
	uint nballocs = 0;
	vector<string>::iterator idsn;
	for ( idsn=datasetNames.begin(); idsn!=datasetNames.end(); ++idsn )
	{
		TNDataSetsMS::iterator ids = NDataSets.find( *idsn );
		if ( ids != NDataSets.end() )
		{
			CDataSetMS& dataset = GET_NDATASET(ids);

			// Allocate entity trackers and tell everyone about the trackers of each other
			allocateEntityTrackers( dataset, serviceId, serviceId, true, getTagOfService( serviceId ) );

			// Erase obsolete remotely managed entities. This is necessary as when a remote MS did not send
			// its removals to us, we kepts obsolete entities.
			// Disabled. Now:
			// - A MS1 continues to send row management to a MS2 that has no clients (IniTag).
			// - MS1 sends all the binding counters and removed entities when a new service connects on MS2
			// See CDataSetMS::receiveAllBindingCounters().
			// => a new service can notify / create some entities with the right counters
			/*if ( mustEraseObsoleteRemoteEntities &&
				 (dataset._PropertyContainer.EntityIdArray.EntityIds != NULL) ) // not allocated if it's the first service to connect
			{
				setupDestEntityTrackersInterestedByDelta( dataset, ExcludedTag, REMOVING ); // don't notify to any client service
				for ( TDataSetIndex entityIndex=0; entityIndex!=dataset.maxNbRows(); ++entityIndex )
				{
					if ( dataset._PropertyContainer.EntityIdArray.isOnline( entityIndex ) )
					{
						CEntityId& entityId = dataset._PropertyContainer.EntityIdArray.EntityIds[entityIndex];
						TServiceId spawnerId = dataset.getSpawnerServiceId( entityIndex );
						if ( _ClientServices.find( spawnerId ) == _ClientServices.end() ) // if not local, remote
						{
							if ( (previousTag == IniTag) || // no entity additions/removals came here
								 (getTagOfService( spawnerId ) == previousTag) ) // no entity additions/removals of this tag came here
							{
								dataset.removeEntityFromDataSet( TDataSetRow::createFromRawIndex( entityIndex ) ); // resets counter, they will be received with the sync
								dataset.clearValuesOfRow( entityIndex );
							}
						}
					}
				}
			}*/
		}
	}

	nlinfo( "Service %s: %u datasets declared", servStr(serviceId).c_str(), datasetNames.size() );

	// Tell all other MS that we are in charge of this service
	CMessage msgout( "MARCS" ); // Mirror Add/Remove Client Service
	msgout.serial( serviceId );
	bool addOrRemove = true;
	msgout.serial( addOrRemove );
	msgout.serial( tagOfNewClientService );
	msgout.serial( const_cast<string&>(MirrorServiceVersion) );
	CUnifiedNetwork::getInstance()->send( "MS", msgout, false );

	// Defer rescanning of online entities to know entities from other MS.
	// This is not done now, because we will do it only after we have received the corresponding properties
	// Otherwise, the service would have it's entity notification with some empty properties.
	// Before that, it can receive notifications for other local services, though.
	deferRescanOfEntitiesForNewService( (TServiceId)serviceId );
}


/*
 * Allocate a property segment with shared memory and return the smid to the sender
 */
void	CMirrorService::allocateProperty( CMessage& msgin, NLNET::TServiceId serviceId )
{
	// Get property name
	string propName;
	msgin.serial( propName );

	uint32 dataTypeSize;
	TPropertiesInMirrorMS::iterator ipim = _PropertiesInMirror.find( propName );
	if ( ipim != _PropertiesInMirror.end() )
	{
		TPropertyInfoMS& propinfo = GET_PROPERTY_INFO(ipim);
		if ( ! propinfo.allocated() )
		{
			// Retrieve the corresponding dataset
			TPropertyIndex propIndex;
			CDataSetMS *ds = _PropertiesInMirror.getDataSetByPropName( propName, propIndex );
			if ( ds )
			{
				uint32 segmentSize;
				if ( propIndex != INVALID_PROPERTY_INDEX )
				{
					// PropertyValueArray
					dataTypeSize = ds->getDataSizeOfProp( propIndex );
					if ( ds->_PropIsList[propIndex] )
					{
						// List of values
						segmentSize = (ds->maxNbRows() * (sizeof(TGameCycle)+sizeof(TServiceId8)+sizeof(TSharedListRow)))
									+ sizeof(TSharedListRow) + (NB_SHAREDLIST_CELLS * (sizeof(TSharedListRow)+dataTypeSize));
					}	
					else
					{
						// Single values
						segmentSize = ds->maxNbRows() * (sizeof(TGameCycle)+sizeof(TServiceId8)+dataTypeSize);
					}
				}
				else
				{
					// EntityIdArray
					dataTypeSize = sizeof(CEntityId); // 8 bytes by entity id
					segmentSize = (ds->maxNbRows()*dataTypeSize) + (ds->maxNbRows()/8+4) + (ds->maxNbRows() * (sizeof(NLMISC::TGameCycle)+sizeof(uint8)+sizeof(TServiceId8))); // CEntityIds + OnlineBitField rounded to uint32 + timestamps + counters + spawnerIds
				}

				do
				{
					// Get a new shared memory id
					propinfo.SMId = _SMIdPool.getNewId();
					if ( propinfo.SMId == InvalidSMId )
					{
						nlwarning( "SHDMEM: No more free shared memory ids (alloc prop)" );
						beep( 660, 150 ); // TODO: error handling
						return;
					}

					// Allocate shared memory
					propinfo.Segment = CSharedMemory::createSharedMemory( toSharedMemId(propinfo.SMId), segmentSize );
					if ( (propinfo.Segment == NULL) && DestroyGhostSharedMemSegments )
					{
						// Under Linux, segment may have not been destroyed if the MS crashed: destroy (if asked in the config file, and retry)
						//              See /proc/sysvipc/shm for the list of allocated segments
						CSharedMemory::destroySharedMemory( toSharedMemId(propinfo.SMId), true );
						nlinfo( "SHDMEM: Destroyed shared memory segment, smid %d", propinfo.SMId );
						propinfo.Segment = CSharedMemory::createSharedMemory( toSharedMemId(propinfo.SMId), segmentSize );
					}
					if ( propinfo.Segment != NULL )
					{
						// Init to zero
						memset( propinfo.Segment, 0, segmentSize );

						// Set pointer and init (readOnly and monitorAssignment are used only by client services)
						ds->setPropertyPointer( propName, propIndex, propinfo.Segment, true, dataTypeSize, false, false, ds->_BindingCountsToSet, CTickProxy::getGameCycle(), segmentSize ); // deletes ds->_BindingCountsToSet
						ds->_BindingCountsToSet = NULL;

						_PropertiesInMirror.insert( make_pair( propName, propinfo ) ); // obsolete
						if ( propIndex == -1 )
							nlinfo( "SHDMEM: Allocated a segment of %g KB (smid %d, %d rows, dataTypeSize=%u) for entity ids (%s)", (float)segmentSize/1024.0f, propinfo.SMId, ds->maxNbRows(), dataTypeSize, propName.c_str() );
						else
							nlinfo( "SHDMEM: Allocated a segment of %g KB (smid %d, %d rows, dataTypeSize=%u%s) for property %s (dataset %s, propIndex %hd)", (float)segmentSize/1024.0f, propinfo.SMId, ds->maxNbRows(), dataTypeSize, ds->_PropIsList[propIndex]?", list":"", propName.c_str(), ds->name().c_str(), propIndex );
					}
					else
					{
						nlwarning( "SHDMEM: Cannot allocate shared memory for property (smid %d)", propinfo.SMId );
						beep( 280, 150 );
						propinfo.SMId = InvalidSMId;
					}
				}
				while ( propinfo.SMId == InvalidSMId ); // iterate to force to get another smid when chosen smid is locked by another program

				// If there are local services that own a portion of the dataset, tell them about the new segment (same as giveOtherProperties())
				for ( TClientServices::iterator ics=_ClientServices.begin(); ics!=_ClientServices.end(); ++ics )
				{
					if ( (*ics).first == serviceId )
						continue;

					TClientServiceInfo& serviceInfo = GET_CLIENT_SERVICE_INFO(ics);
					for ( vector<TClientServiceInfoOwnedRange>::iterator ior=serviceInfo.OwnedRanges.begin(); ior!=serviceInfo.OwnedRanges.end(); ++ior )
					{
						if ( (*ior).DataSet == ds )
						{
							CMessage msgoutAop( "AOP" ); // Add/access Other Property
							uint32 sheet = ds->sheetId().asInt();
							msgoutAop.serial( sheet );
							msgoutAop.serial( propinfo.PropertyIndex );
							msgoutAop.serial( propinfo.SMId );
							msgoutAop.serial( dataTypeSize );
							CUnifiedNetwork::getInstance()->send( (*ics).first, msgoutAop );
							break;
						}
					}
				}
			}
			else
			{
				nlwarning( "Property %s not found in the loaded datasets", propName.c_str() );
			}
		}
		else
		{
			nlinfo( "SHDMEM: Returning a segment already allocated for property %s (smid %d)", propName.c_str(), propinfo.SMId );
			
			if ( propinfo.PropertyIndex != INVALID_PROPERTY_INDEX )
			{
				dataTypeSize = propinfo.DataSet->_PropertyContainer.PropertyValueArrays[propinfo.PropertyIndex].DataTypeSize;
			}
		}

		// Return the smid
		CMessage msgout( "RAP" );
		msgout.serial( propName );
		msgout.serial( propinfo.SMId );
		msgout.serial( dataTypeSize ); // for type checking
		CUnifiedNetwork::getInstance()->send( serviceId, msgout );

		// Handle special case of _CEntityId_
		if ( propName.substr(0,LENGTH_OF_ENTITYID_PREFIX) == ENTITYID_PREFIX )
		{
			string dataSetName = propName.substr( LENGTH_OF_ENTITYID_PREFIX );
			broadcastSubscribeDataSetEntities( dataSetName, serviceId );
		}
		else
		{
			// Broadcast a subscription to all other MS and process it locally
			broadcastSubscribeProperty( msgin, propName, serviceId );
		}
	}
	else
	{
		nlwarning( "Invalid property %s for allocation", propName.c_str() );
	}
}


/*
 * Give the list of properties not subscribed but allocated on this machine for owned datasets
 */
void	CMirrorService::giveOtherProperties( CMessage& msgin, NLNET::TServiceId serviceId )
{
	uint32 nbPropsSent = 0;
	CMessage msgout( "LOP" ); // List of Other Properties (gives all property segmens in fact)
	sint32 numPos = msgout.reserve( sizeof(nbPropsSent) );
	TClientServiceInfo& clientServiceInfo = _ClientServices[serviceId];
	vector<TClientServiceInfoOwnedRange>::const_iterator ior;
	for ( ior=clientServiceInfo.OwnedRanges.begin(); ior!=clientServiceInfo.OwnedRanges.end(); ++ior )
	{
		for ( TPropertiesInMirrorMS::iterator ipim=_PropertiesInMirror.begin(); ipim!=_PropertiesInMirror.end(); ++ipim )
		{
			TPropertyInfoMS& propinfo = GET_PROPERTY_INFO(ipim);
			if ( propinfo.allocated() )
			{
				if ( (propinfo.DataSet == (*ior).DataSet) &&
					 (propinfo.PropertyIndex != INVALID_PROPERTY_INDEX) )
				{
					uint32 sheet = propinfo.DataSet->sheetId().asInt();
					msgout.serial( sheet );
					msgout.serial( propinfo.PropertyIndex );
					msgout.serial( propinfo.SMId );
					msgout.serial( propinfo.DataSet->_PropertyContainer.PropertyValueArrays[propinfo.PropertyIndex].DataTypeSize );
					++nbPropsSent;
				}
			}
		}
	}
	msgout.poke( nbPropsSent, numPos );
	CUnifiedNetwork::getInstance()->send( serviceId, msgout );
}


/*
 * Unallocate (destroy) a segment
 */
void	CMirrorService::destroyPropertySegments()
{
	// Property values
	TPropertiesInMirrorMS::iterator ip;
	for ( ip=_PropertiesInMirror.begin(); ip!=_PropertiesInMirror.end(); ++ip )
	{
		if ( GET_PROPERTY_INFO(ip).allocated() )
		{
			CSharedMemory::closeSharedMemory( GET_PROPERTY_INFO(ip).Segment );
			GET_PROPERTY_INFO(ip).Segment = NULL;
			CSharedMemory::destroySharedMemory( toSharedMemId(GET_PROPERTY_INFO(ip).SMId) );
			_SMIdPool.releaseId( GET_PROPERTY_INFO(ip).SMId );
		}
	}

	// Remaining trackers
	TSDataSetsMS::iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		CDataSetMS& dataset = GET_SDATASET(ids);

		sint32 eti;
		for ( eti=ADDING; eti<=REMOVING; ++eti )
		{
			TSubscriberListForEnt::iterator isl;
			for ( isl=dataset._EntityTrackers[eti].begin(); isl!=dataset._EntityTrackers[eti].end(); ++isl )
			{
				CChangeTrackerMS& tracker = (*isl);
				if ( tracker.destroy() )
				{
					_SMIdPool.releaseId( tracker.smid() );
					_MutIdPool.releaseId( tracker.mutid() );
				}
			}
		}

		TPropertyIndex propIndex;
		for ( propIndex=0; propIndex!= dataset.nbProperties(); ++propIndex )
		{
			// TODO: Skip properties not declared
			TSubscriberListForProp::iterator isl;
			for ( isl=dataset._SubscribersByProperty[propIndex].begin(); isl!=dataset._SubscribersByProperty[propIndex].end(); ++isl )
			{
				CChangeTrackerMS& tracker = (*isl);
				if ( tracker.destroy() )
				{
					_SMIdPool.releaseId( tracker.smid() );
					_MutIdPool.releaseId( tracker.mutid() );
				}
			}
		}
	}	
}


/*
 * Declare an entity type
 */
void	CMirrorService::declareEntityTypeOwner( CMessage& msgin, NLNET::TServiceId serviceId )
{
	// Read params
	uint8 entityTypeId;
	sint32 maxNbEntities;
	msgin.serial( entityTypeId );
	msgin.serial( maxNbEntities );

	// Retrieve corresponding datasets
	vector<string> datasetNames;
	TNDataSetsMS::iterator ids;
	for ( ids=NDataSets.begin(); ids!=NDataSets.end(); ++ids )
	{
		/*vector<uint8>::iterator it;
		for ( it=GET_NDATASET(ids)._EntityTypesFilter.begin(); it!=GET_NDATASET(ids)._EntityTypesFilter.end(); ++it )
		{
			nldebug( "%d", (*it) );
		}*/
		if ( find( GET_NDATASET(ids)._EntityTypesFilter.begin(), GET_NDATASET(ids)._EntityTypesFilter.end(), entityTypeId ) != GET_NDATASET(ids)._EntityTypesFilter.end() )
		{
			datasetNames.push_back( (*ids).first );

			// Store the serviceId <-> dataset. The range bounds will be filled in giveRange()
			TClientServiceInfoOwnedRange newrange( &(GET_NDATASET(ids)) );
			_ClientServices[serviceId].OwnedRanges.push_back( newrange );
			nldebug( "Entity type %hd declared in %s for %s", (uint16)entityTypeId, GET_NDATASET(ids).name().c_str(), servStr(serviceId).c_str() );
		}
	}

	/*// Testing code
	{
		// For testing:
		CMessage msgSimulatedFromTheManager( "RG" );
		msgSimulatedFromTheManager.serial( serviceId );
		msgSimulatedFromTheManager.serial( entityTypeId );
		uint8 nbDataSets = 1;
		msgSimulatedFromTheManager.serial( nbDataSets );
		string datasetName = "player";
		msgSimulatedFromTheManager.serial( datasetName );
		sint32 first = 0, last = maxNbEntities-1;
		msgSimulatedFromTheManager.serial( first, last );
		msgSimulatedFromTheManager.invert();
		giveRange( msgSimulatedFromTheManager, 0 );
		return;
	}
	nlstop;*/

	// Ask the centralized range manager for a range per dataset
	CMessage msgout;
	msgout.setType( "GRG" ); // get range
	msgout.serialCont( datasetNames );
	msgout.serial( serviceId );
	msgout.serial( maxNbEntities );
	msgout.serial( entityTypeId );
	if ( ! CUnifiedNetwork::getInstance()->send( RANGE_MANAGER_SERVICE, msgout ) )
		nlwarning( "Error: Cannot request ranges to manager service %s (offline?)", RANGE_MANAGER_SERVICE.c_str() );
	nlinfo( "ROWMGT: Requested %d rows of type %hu in %u ranges for service %s", maxNbEntities, (uint16)entityTypeId, datasetNames.size(), servStr(serviceId).c_str() );
}


/*
 * Give a range (message from the range manager)
 */
void	CMirrorService::giveRange( CMessage& msgin, NLNET::TServiceId serviceId )
{
	uint8 nbDatasets;
	string dataset;
	TDataSetIndex first, last;
	NLNET::TServiceId declaratorServiceId;
	uint8 entityTypeId;
	msgin.serial( declaratorServiceId );
	msgin.serial( entityTypeId );
	msgin.serial( nbDatasets );
	for ( sint i=0; i!=nbDatasets; ++i )
	{
		msgin.serial( dataset );
		msgin.serial( first, last );

		if ( first != INVALID_DATASET_INDEX )
		{
			// Fill the first and last fields of the TClientServiceInfoOwnedRange struct
			TClientServiceInfo& serviceInfo = _ClientServices[declaratorServiceId];
			std::vector<TClientServiceInfoOwnedRange>::iterator icsir;
			for ( icsir=serviceInfo.OwnedRanges.begin(); icsir!=serviceInfo.OwnedRanges.end(); ++icsir )
			{
				// Find a blank range for the dataset
				if ( ((*icsir).First == INVALID_DATASET_INDEX) && ((*icsir).DataSet->name() == dataset) )
				{
					(*icsir).First = first;

					// Check range bounds, regarding number of rows of dataset
					if ( last < (*icsir).DataSet->maxNbRows() )
					{
						(*icsir).Last = last;
					}
					else
					{
						(*icsir).Last = first;
					}

					/*// Store to the main owned ranges list
					TDeclaredEntityRange declaredRange;
					declaredRange._ServiceId = declaratorServiceId;
					(*icsir).fillToDeclaredEntityRange( declaredRange );
					(*icsir).DataSet->_SortedOwnedEntityRanges.insert( declaredRange );

					// Broadcast to all other MSes
					CMessage msgout( "DRG" );
					msgout.serial( declaredRange );
					CUnifiedNetwork::getInstance()->send( "MS", msgout, false );*/

					nlinfo( "ROWMGT: Service %s got range [%d..%d] for type %hu in dataset %s", servStr(declaratorServiceId).c_str(), first, (*icsir).Last, (uint16)entityTypeId, dataset.c_str() );
					break;
				}
			}
			if ( icsir == serviceInfo.OwnedRanges.end() )
			{
				nlwarning( "Dataset %s not found in the client service info ranges", dataset.c_str() );
			}
		}
		else
			nlwarning( "ROWMGT: Service %s didn't get a range for type %hu in dataset %s", servStr(declaratorServiceId).c_str(), (uint16)entityTypeId, dataset.c_str() );
	}

	// Tell the declarator the range
	msgin.invert();
	CUnifiedNetwork::getInstance()->send( declaratorServiceId, msgin );
}


/*
 *
 */
/*void	CMirrorService::receiveDeclaredRange( NLNET::CMessage& msgin )
{
	NLMISC::CSheetId sheetId;
	msgin.serial( sheetId );
	TSDataSetsMS::iterator isd = _SDataSets.find( sheetId );
	if ( isd != _SDataSets.end() )
	{
		TDeclaredEntityRange range;
		msgin.serial( range );
		(*isd)._SortedOwnedEntityRanges.insert( range );
	}
}*/


/*
 * For client services
 */
void	CMirrorService::releaseRangesOfService( NLNET::TServiceId serviceId )
{
	TClientServices::iterator ics = _ClientServices.find( serviceId );
	if ( ics != _ClientServices.end() )
	{
		TClientServiceInfo& cs = GET_CLIENT_SERVICE_INFO(ics);
		if ( ! cs.OwnedRanges.empty() )
		{
			// Get names of datasets and send them to the centralized range manager (TEMP: names instead of id)
			CMessage msgout( "RRG" ); // release range
			vector<string> datasetNames( cs.OwnedRanges.size() );
			uint i;
			for ( i=0; i!=cs.OwnedRanges.size(); ++i )
			{
				datasetNames[i] = (cs.OwnedRanges[i].DataSet)->name();
			}
			msgout.serialCont( datasetNames );
			msgout.serial( serviceId );
			CUnifiedNetwork::getInstance()->send( RANGE_MANAGER_SERVICE, msgout );
			nlinfo( "ROWMGT: Released ranges owned by service %s", servStr( serviceId ).c_str() );
		}

		const CMTRTag& tagOfLeavingService = getTagOfService( serviceId );

		sint nbremoved = 0;
		for ( uint i=0; i!=cs.OwnedRanges.size(); ++i )
		{
			TDataSetIndex entityIndex;
			if ( cs.OwnedRanges[i].First != INVALID_DATASET_INDEX )
			{
				CDataSetMS& dataset = *(cs.OwnedRanges[i].DataSet);

				if ( dataset._PropertyContainer.EntityIdArray.EntityIds ) // can be not allocated if no service uses the dataset
				{
					TDataSetIndex first = cs.OwnedRanges[i].First;
					TDataSetIndex last = cs.OwnedRanges[i].Last;
					if ( first < dataset.maxNbRows() )
					{
						last = min(dataset.maxNbRows()-1,last); // prevent to crash when there was a range problem

						// Free the remaining entity ids/rows owned by the specified service
						// Uses the MTR tag of this new service to block some entities
						std::vector<NLNET::TServiceId8> emptyList;
						setupDestEntityTrackersInterestedByDelta( dataset, tagOfLeavingService, REMOVING, emptyList );
						for( entityIndex=first; entityIndex<=last; ++entityIndex )
						{
							// Test if the entityIndex is used (and not already removed)
							if ( (! dataset.getEntityId( TDataSetRow(entityIndex) ).isUnknownId()) &&
								dataset._PropertyContainer.EntityIdArray.isOnline( entityIndex ) )
							{
								// Push to remote services' remove lists
								TSubscriberListForEnt::iterator it;
								for ( it=dataset._EntityTrackers[REMOVING].begin(); it!=dataset._EntityTrackers[REMOVING].end(); ++it )
								{
									if ( ! (*it).isLocal() )
									{
										// Push to removal tracker even for IniTag destination (because it could have been
										// different from IniTag before)
										const CMTRTag& destTag = getTagOfService( (TServiceId)((*it).destServiceId()) );
										if ( destTag.doesAcceptTag( tagOfLeavingService ) )
											(*it).recordChange( entityIndex );
									}
								}

								// Now receiving a removal in the same tick as an addition is supported by the MS (MV1.5)
								// Obsolete: If the quitting service just spawned the entity in the current it, remove it from adding trackers!
								/*for ( it=dataset._EntityTrackers[ADDING].begin(); it!=dataset._EntityTrackers[ADDING].end(); ++it )
								{
									if ( ! (*it).isLocal() ) // local services can handle the case
									{
										// Test timestamp to avoid searching all in each tracker (needs to be done before calling removeEntityFromDataSet())
										if ( dataset._PropertyContainer.EntityIdArray.OnlineTimestamps[entityIndex] >= CTickProxy::getGameCycle()-1 )
										{
											(*it).cancelChange( entityIndex );
											//nldebug( "Cancelled addition of E%u", entityIndex );
										}
									}
								}*/

								// Remove entity
								uint8 newCounter = dataset.getBindingCounter( entityIndex ) + 1;
								if ( newCounter == 0 )
									++newCounter; // skip 0
								TDataSetRow datasetrow( entityIndex, newCounter );
								dataset.removeEntityFromDataSet( datasetrow );

								++nbremoved;
							}
						}

						// If a remote MS did not received additions/removals, transmit binding counters & offline entities
						CMessage msgout( "BIDG_CNTR" );
						msgout.serial( const_cast<string&>(dataset.name()) );
						dataset.transmitBindingCountersOfRange( msgout, first, last+1, (TServiceId)serviceId );
						for ( TServiceIdList::iterator iml=_RemoteMSList.begin(); iml!=_RemoteMSList.end(); ++iml )
						{
							// Here we send whatever the destination tag, because a new service (with the
							// same not compatible tag) can start and acquire the range(s) that our
							// leaving service had.
							CUnifiedNetwork::getInstance()->send( (*iml), msgout );
						}
					}
				}
			}
			else
			{
				nlwarning( "Seems that the %s was down when the service that leaves now came online", RANGE_MANAGER_SERVICE.c_str() );
			}
		}
		if ( nbremoved != 0 )
		{
			nlinfo( "ROWMGT: Auto-released %d entities when service %s disconnected", nbremoved, servStr(serviceId).c_str() );
		}

		_ClientServices.erase( ics );

		// If the service was no fully ready, erase its _NbExpectedATAcknowledges entry
		_NbExpectedATAcknowledges.erase( serviceId );

		// If we were expecting an ATE acknowledge, "cancel the expectation"
		TSDataSetsMS::iterator ids;
		for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
		{
			CDataSetMS& dataset = GET_SDATASET(ids);
			for ( TSubscriberListForEnt::iterator isl=dataset._EntityTrackers[ADDING].begin(); isl!=dataset._EntityTrackers[ADDING].end(); ++isl )
			{
				CChangeTrackerMS& entityTrackerAdding = (*isl);
				if ( entityTrackerAdding.isExpectingATEAckFrom( serviceId ) )
				{
					entityTrackerAdding.popATEAcknowledgesExpected( serviceId, false );
				}
			}
		}

		// Tell all other MS that we are not in charge of this service anymore
		CMessage msgout( "MARCS" ); // Mirror Add/Remove Client Service
		msgout.serial( serviceId );
		bool addOrRemove = false;
		msgout.serial( addOrRemove ); // no need of the tag
		CUnifiedNetwork::getInstance()->send( "MS", msgout, false );
	}
	// else nothing to do
}


/*
 * Process an entity management subscription request from a client service, to forward it to all other mirror services
 */
void	CMirrorService::broadcastSubscribeDataSetEntities( const std::string& dataSetName, NLNET::TServiceId clientServiceId )
{
	// Broadcast subscription to all other mirror services
	CMessage msgout( "SDSE" );
	msgout.serial( const_cast<string&>(dataSetName) );
	msgout.serial( mainTag() );
	msgout.serial( clientServiceId );
	CUnifiedNetwork::getInstance()->send( "MS", msgout, false );
	nlinfo( "ROWMGT: Sent SDSE (subscribe dataset entities) %s to all MS", dataSetName.c_str() );
}


CMTRTag DefaultMTRTag ( AllTag );


/*
 *
 */
void			CMirrorService::setNewTagOfRemoteMS( const CMTRTag& tag, TServiceId msId )
{
	setNewTag( msId, tag );
	nlinfo( "Remote MS has now tag %hd", tag.rawTag() );
	if ( (! _ClientServices.empty()) )
	{
		// Update existing trackers for new tag of remote MS:
		// Tell client service having incompatible tag to remote their trackers to this remote MS.
		// If services are already pushing entities to the remote MS, it's not a problem because
		// the new destination service (with the different tag) will rescan entities ignoring those
		// services. For properties, they are not sent after subscription (unlike entities) so the
		// new service will not receive property notifications from those services.

		// Vectors of smids, indexed by serviceId
		map< NLNET::TServiceId, vector<sint32> > servicesForTrackerRemoval;

		// Browse datasets
		TSDataSetsMS::iterator ids;
		for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
		{
			CDataSetMS& dataset = GET_SDATASET(ids);
			//nlinfo( "DATASET %s", dataset.name().c_str() );

			sint32 eti;
			for ( eti=ADDING; eti<=REMOVING; ++eti )
			{
				//nlinfo( "ENTITY TRACKER %d", eti );
				// Find the tracker corresponding to the specified service
				CChangeTrackerMS *ptracker = NULL;
				TSubscriberListForEnt::iterator isl;
				for ( isl=dataset._EntityTrackers[eti].begin(); isl!=dataset._EntityTrackers[eti].end(); ++isl )
				{
					if ( (*isl).destServiceId() == msId )
					{
						ptracker = &(*isl);
						break;
					}
				}

				// Check if tracker found in the current dataset
				if ( (! ptracker) /*|| (!ptracker->isAllocated())*/ )
					break; // no entity tracker

				//nlinfo( "FOUND ENTITY TRACKER" );
				
				// Tell every other local service having tag incompatibility to remove the tracker
				for ( isl=dataset._EntityTrackers[eti].begin(); isl!=dataset._EntityTrackers[eti].end(); ++isl )
				{
					if ( (*isl).isLocal() )
					{
						// if the dest MS is now IniTag, we remove the tracker from our clients services (in doesAcceptTag())
						if ( /*(tag == IniTag) ||*/ (! tag.doesAcceptTag( getTagOfService( (*isl).destServiceId() ) )) )
						{
							//nlinfo( "ADDING TO REMOVE LIST OF %s", servStr((*isl).destServiceId()).c_str() );
							servicesForTrackerRemoval[(*isl).destServiceId()].push_back( ptracker->smid() );
						}
					}
				}
			}

			TPropertyIndex propIndex;
			for ( propIndex=0; propIndex!= dataset.nbProperties(); ++propIndex )
			{
				//nldebug( "PROP TRACKER %Phd", propIndex );
				// TODO: Skip properties not declared

				// Find the tracker corresponding to the specified service and the current property
				CChangeTrackerMS *ptracker = NULL;
				TSubscriberListForProp::iterator isl;
				for ( isl=dataset._SubscribersByProperty[propIndex].begin(); isl!=dataset._SubscribersByProperty[propIndex].end(); ++isl )
				{
					if ( (*isl).destServiceId() == msId )
					{
						ptracker = &(*isl);
					}
				}

				// Check if tracker found in the current dataset, for the current property
				if ( (! ptracker) /*|| (! ptracker->isAllocated())*/ )
					continue;
				//nldebug( "ABOUT TO REMOVE PROP TRACKER P%hd", propIndex );


				// Tell every other local service that has the tracker to remove it
				for ( isl=dataset._SubscribersByProperty[propIndex].begin(); isl!=dataset._SubscribersByProperty[propIndex].end(); ++isl )
				{
					if ( (*isl).isLocal() )
					{
						if ( /*(tag == IniTag) ||*/ (! tag.doesAcceptTag( getTagOfService( (*isl).destServiceId() ) )) )
						{
							//nlinfo( "ADDING TO REMOVE LIST OF %s", servStr((*isl).destServiceId()).c_str() );
							// In fact we don't check if the service has the tracker, but no matter if it hasn't, it will just ignore the smid
							servicesForTrackerRemoval[(*isl).destServiceId()].push_back( ptracker->smid() );
						}
					}
				}
			}
		}

		sendTrackersToRemoveToLocalServices( servicesForTrackerRemoval );
	}
}


/*
 * Process a subscription of dataset entities management, received from another MS
 */
void	CMirrorService::processDataSetEntitiesSubscription( const std::string& dataSetName, NLNET::TServiceId msId, NLNET::TServiceId clientServiceId, const CMTRTag& newTagOfRemoteMS )
{
	try
	{
		// TODO?: add trackers only if there are local services able to manage rows (owning) in the dataset,
		// else defer for when they will log on

		setNewTag( (TServiceId)msId, newTagOfRemoteMS );

		nlinfo( "ROWMGT: MS %hu subscribes to dataset %s", msId.get(), dataSetName.c_str() );
		allocateEntityTrackers( NDataSets[dataSetName], msId, clientServiceId, false, newTagOfRemoteMS );
	}		
	catch (const EMirror&)
	{
		nlwarning( "MIRROR: Invalid dataset while receiving subscription" );
	}
}


/*
 *
 */
void	CMirrorService::decNbExpectedATAcknowledges( NLNET::TServiceId clientServiceId, uint whichEorP )
{
	map<NLNET::TServiceId,sint>::iterator it = _NbExpectedATAcknowledges.find( clientServiceId );
	if ( it != _NbExpectedATAcknowledges.end() )
	{
		--(*it).second;
		nldebug( "--_NbExpectedATAcknowledges -> %d", (*it).second );
		nlassert( (*it).second >= 0 );
		if ( (*it).second == 0 )
			_NbExpectedATAcknowledges.erase( it ); // all acks received, we don't need the entry anymore
	}

	if ( _BlockedAwaitingATAck && (getTotalNbExpectedATAcknowledges() == 0) )
	{
		// Unblock the automaton
		nldebug( "Unblocking automaton when all acks received" );
		_BlockedAwaitingATAck = false;
		doNextTask();
	}
}


/*
 *
 */
sint	CMirrorService::getTotalNbExpectedATAcknowledges() const
{
	// I can't figure how to use std::accumulate() with a map, even with std::with_data :-(
	sint sum = 0;
	for ( map<NLNET::TServiceId,sint>::const_iterator it=_NbExpectedATAcknowledges.begin(); it!=_NbExpectedATAcknowledges.end(); ++it )
		sum += (*it).second;
	return sum;
}



/*
 * (Works even if the entity ids not allocated yet)
 *
 * local:
 * - serviceId = id of local client service
 * - clientServiceId = id of local client service
 * !local:
 * - serviceId = id of remote MS
 * - clientServiceId = id of remote client service
 */
void	CMirrorService::allocateEntityTrackers( CDataSetMS& dataset, NLNET::TServiceId serviceId, NLNET::TServiceId clientServiceId, bool local, const CMTRTag& tagOfNewPeer )
{
	nlinfo( "ROWMGT: Service %s (%s with tag %d) subscribes to dataset %s row management (at %u)", servStr(serviceId).c_str(), local?"local client":"MS", tagOfNewPeer.rawTag(), dataset.name().c_str(), CTickProxy::getGameCycle() );

	// TODO: only if serviceId is an owner of the dataset
	// TODO: maybe, filter out the services who aren't owners of the dataset

	// Check if the serviceId has also a tracker (ex: 2 services declare the same property
	// to their MS A which sends 2 prop subscriptions to a MS B) and its state (read-only...)
	bool mustCreateTrackers = true;
	CChangeTrackerMS *pEntityTrackerAdding = NULL, *pEntityTrackerRemoving = NULL;
	if ( (! local) && ((pEntityTrackerAdding = dataset.findEntityTracker( ADDING, serviceId )) != NULL) )
	{
		if ( hasTagOfServiceChanged( (TServiceId)serviceId ) ) // serviceId is a remote MS
		{
			// Tracker already created, but we need to communicate it to the local services who don't know it
			// In fact, send to all, and discard if received twice.
			mustCreateTrackers = false;
			pEntityTrackerRemoving = dataset.findEntityTracker( REMOVING, serviceId );
		}
		else
		{
			// No need to create the tracker, no need to communicate it to the local services
			nldebug( "ROWMGT: Service %s already subscribed to row management", servStr(serviceId).c_str() );
			nlassert( ! pEntityTrackerAdding->isLocal() );
			return;
		}
	}

	if ( mustCreateTrackers )
	{
		// Allocate shared memory
		sint32 smidAdd, smidRemove;
		do
		{
			if ( ! pEntityTrackerAdding )
				smidAdd = _SMIdPool.getNewId();
			if ( ! pEntityTrackerRemoving )
				smidRemove = _SMIdPool.getNewId();
			if ( (smidAdd == InvalidSMId) || (smidRemove == InvalidSMId) )
			{
				nlwarning( "ROWMGT: No more free shared memory ids (entity tracker)" );
				beep( 660, 150 ); // TODO: error handling
				return;
			}

			if ( ! pEntityTrackerAdding )
			{
				CChangeTrackerMS& entityTrackerAdding = dataset.addEntityTracker( ADDING, serviceId, local, smidAdd );
				pEntityTrackerAdding = &entityTrackerAdding;
			}
			if ( ! pEntityTrackerRemoving )
			{
				CChangeTrackerMS& entityTrackerRemoving = dataset.addEntityTracker( REMOVING, serviceId, local, smidRemove );
				pEntityTrackerRemoving = &entityTrackerRemoving;
			}

			if ( ! pEntityTrackerAdding->allocate( smidAdd, dataset.maxNbRows() ) )
			{
				pEntityTrackerAdding = NULL;
				dataset._EntityTrackers[ADDING].pop_back();
			}
			if ( ! pEntityTrackerRemoving->allocate( smidRemove, dataset.maxNbRows() ) )
			{
				pEntityTrackerRemoving = NULL;
				dataset._EntityTrackers[REMOVING].pop_back();
			}
		}
		while ( ! (pEntityTrackerAdding && pEntityTrackerRemoving) ); // iterate to force to get another smid when chosen smid is locked by another program

		// Create the mutexes (note: mutexids are not necessary to create CFastMutex objects)
		sint32 mutidAdd, mutidRemove;
		while ( (! pEntityTrackerAdding->createMutex( (mutidAdd = _MutIdPool.getNewId()), true )) && (mutidAdd != InvalidSMId) )
			nlwarning( "ROWMGT: Skipping mutex id %d for creation of adding entity tracker", mutidAdd );
		while ( (! pEntityTrackerRemoving->createMutex( (mutidRemove = _MutIdPool.getNewId()), true )) && (mutidRemove != InvalidSMId) )
			nlwarning( "ROWMGT: Skipping mutex id %d for creation of removing entity tracker", mutidRemove );
		if ( (mutidAdd == InvalidSMId) || (mutidRemove == InvalidSMId) )
		{
			nlwarning( "ROWMGT: Failed to find an id and create mutex for an entity tracker" );
			beep( 660, 150 ); // TODO: error handling
			return;
		}

		// Link to delta buffers (for remote trackers)
		if ( ! local )
		{
			CDeltaToMS *delta = _RemoteMSList.getDeltaToMS( (TServiceId)serviceId, &dataset );
			nlassert( delta );
			pEntityTrackerAdding->setLinkToDeltaBuffer( delta ); //   can pass pointer because only allocated by 'new'
			pEntityTrackerRemoving->setLinkToDeltaBuffer( delta ); // (only pointers to CDeltaToMS are in a vector)
		}
	}
	if ( ! local )
		nlassert( pEntityTrackerAdding->getLinkToDeltaBuffer() && pEntityTrackerRemoving->getLinkToDeltaBuffer() );

	// List of services from which we will expect a FWD_ACKATE
	vector<NLNET::TServiceId> entityTrackerPairsSentTo;

	// Tell to the other services to add these trackers (give them the smids) (TODO: filter)
	uint8 nb = 1;
	bool self = false;
	TSubscriberListForEnt& entityTrackersAdding = const_cast<TSubscriberListForEnt&>(dataset.getEntityTrackers( ADDING ));

	// This message will be sent to all other local services
	CMessage msgout1( "ATE" );
	msgout1.serial( const_cast<string&>(dataset.name()) );
	msgout1.serial( nb );
	msgout1.serial( self );
	msgout1.serial( const_cast<sint32&>(pEntityTrackerAdding->smid()) );
	msgout1.serial( const_cast<sint32&>(pEntityTrackerRemoving->smid()) );
	msgout1.serial( const_cast<sint32&>(pEntityTrackerAdding->mutid()) );
	msgout1.serial( const_cast<sint32&>(pEntityTrackerRemoving->mutid()) );
	msgout1.serial( serviceId );
	bool isInitialTransmitTracker = false;
	msgout1.serial( isInitialTransmitTracker );
	TSubscriberListForEnt::const_iterator it;
	for ( it=entityTrackersAdding.begin(); it!=entityTrackersAdding.end(); ++it )
	{
		if ( (*it).isLocal() )
		{
			// Comment out the following test if you want the services to be notified for their own changes
			if ( ((*it).destServiceId() != serviceId) )
			{
				if ( (_ClientServices.find( (*it).destServiceId() ) != _ClientServices.end() ) && // skip tracker if the destination service is down (because the trackers remain in the list for some time after service disconnection, see deferTrackerRemovalForQuittingService())
					tagOfNewPeer.doesAcceptTag( getTagOfService( (*it).destServiceId() ) ) )
				{
					// Send the new tracker to each other local service
					CUnifiedNetwork::getInstance()->send( (*it).destServiceId(), msgout1 );
					++_NbExpectedATAcknowledges[(*it).destServiceId()];
					nldebug( "++_NbExpectedATAcknowledges -> %d", _NbExpectedATAcknowledges[(*it).destServiceId()] );
					entityTrackerPairsSentTo.push_back( (*it).destServiceId() );
				}
			}
		}
	}

	if ( local )
	{
		// This message, containing all entity trackers, will be sent to the subscriber, if it is local
		CMessage msgout2( "ATE" );
		msgout2.serial( const_cast<string&>(dataset.name()) );
		sint32 nbTrackersBufPos = msgout2.reserve( sizeof( nb ) );
		nb = 0;
		const TSubscriberListForEnt& entityTrackersRemoving = dataset.getEntityTrackers( REMOVING );
		nlassert( entityTrackersAdding.size() == entityTrackersRemoving.size() );

		// Note: this loop on entityTrackersAdding and entityTrackersRemoving assumes their are symetric
		for ( uint i=0; i!=entityTrackersAdding.size(); ++i )
		{
			NLNET::TServiceId destServId = entityTrackersAdding[i].destServiceId();

			if ( (destServId == serviceId) || // self trackers
				 (getTagOfService( (TServiceId)destServId ).doesAcceptTag( tagOfNewPeer )) )
			{
				// Skip tracker if the destination service is down (because the trackers remain in the list for some time after service disconnection, see deferTrackerRemovalForQuittingService())
				if ( (! (entityTrackersAdding[i].isLocal() && (_ClientServices.find( destServId ) == _ClientServices.end()))) &&
					 (! CUnifiedNetwork::getInstance()->getServiceName( destServId ).empty()) )
				{
					self = ( destServId == serviceId );
					msgout2.serial( self );
					msgout2.serial( const_cast<sint32&>(entityTrackersAdding[i].smid()) );
					msgout2.serial( const_cast<sint32&>(entityTrackersRemoving[i].smid()) );
					msgout2.serial( const_cast<sint32&>(entityTrackersAdding[i].mutid()) );
					msgout2.serial( const_cast<sint32&>(entityTrackersRemoving[i].mutid()) );
					msgout2.serial( destServId );
					++nb;

					// Our new client service needs to have a tracker to a remote MS.
					// We don't need to block the tracker, because the new client can't write into it yet
					// (because it will be able to do it only after receiving SC_EI).
					// THUS the corresponding FWD_ACKATE must not call popATEAcknowledgesExpected().
					// This info will be transmitted in the FWD_ACKATE. However, when receiving FWD_ACKATE,
					// we will have to ensure that the tracker is open, 
					// Instead of doing this:
					//if ( (! self) && (! entityTrackersAdding[i].isLocal()) )
					//{
					//	entityTrackerPairsSentTo.push_back( serviceId );
					//}
					bool isInitialTransmitTracker = true;
					msgout2.serial( isInitialTransmitTracker );
				}
			}
		}

		// Tell the subscribing service to add a tracker for each other service (give it the smids)
		msgout2.poke( nb, nbTrackersBufPos );
		CUnifiedNetwork::getInstance()->send( serviceId, msgout2 );
		_NbExpectedATAcknowledges[serviceId] += nb;
		nldebug( "+++_NbExpectedATAcknowledges -> %d", _NbExpectedATAcknowledges[serviceId] );
	}
	else
	{
		// A new remote client has connected to a remote MS.
		// We will open or reopen the entity delta traffic this the remote MS.
		// Store the number of expected FWD_ACKATE into the new adding tracker
		//
		// If this MS does not have any client services at the moment, only the binding counters will be emitted.
		// Thus, the following case will work fine:
		// 1. MS1 and MS2 are online
		// 2. S1 connects to MS1, creates some entities, then disconnects
		// 3. S2 connects to MS2 => MS2 receives the binding counters modified by S1; S2 can create some entities.
		pEntityTrackerAdding->pushATEAcknowledgesExpectedBeforeUnblocking( clientServiceId, entityTrackerPairsSentTo );
	}
}


/*
 * Do the rescan
 */
void	CMirrorService::rescanEntitiesForNewService( TServiceId clientServiceId )
{
	// Fill new service's EntityTrackerAdding with the current used dataset rows (not already removed),
	// so that it will be notified of the entities that exist already.
	// Even if this operation is done by the subscriber when it receives an acknowledge of ATE (ACKATE),
	// we've got to do it here as well for the multi-machine case.
	//
	// Ex:
	//			MS1-------------MS2
	//			 |               |
	//			S1               S2			1) S2 connects 2) S2 disconnects 3) S2 connects again.
	//
	// 1) S1 gets EntityTrackers for MS2 and sends a FWD_ACKATE acknowledge to MS1
	//    => MS1 sends the entities to MS2 and MS2 fills them to S2
	// 2) Even if S2 leaves, S1 keeps its EntityTrackers
	// 3) S1 does not get again the EntityTrackers so does not send FWD_ACKATE
	//    => MS2 has got to fill the entities to the new S2 (this is what's done here)
	//
	// Uses the MTR tag of this new service to block some entities

	// Build the list of entities to ignore (note: what if a new incompatible service connects and spawns entities after this?)
	vector<uint8> creatorIdsToIgnore;
	CMTRTag tagOfRequestor = getTagOfService( clientServiceId );
	//Include local services for workaround of MIRROR:ROWMGT:ACKATE> when entityId pt not receive
	//for ( TClientServices::iterator ics=_ClientServices.begin(); ics!=_ClientServices.end(); ++ics )
	//{
	//	creatorIdsToIgnore.push_back( (uint8)((*ics).first) ); // ignore all local clients (because entities scanned by ACKATE), including requestor
	//}
	for ( uint8 servId=0; servId!=255; ++servId )
	{
	    //Include local services for workaround of MIRROR:ROWMGT:ACKATE> when entityId pt not received
		//if ( _RemoteMSList.getCorrespondingMS( servId ) != 0 ) // remote client services only
		{
			if ( ! tagOfRequestor.doesAcceptTag( getTagOfService( NLNET::TServiceId8(servId) ) ) ) // tag of client service, not of its MS
				creatorIdsToIgnore.push_back( servId );
		}
	}
	CMessage msgout( "SC_EI" ); // "Scan except ignored"
	msgout.serialCont( creatorIdsToIgnore );
	CUnifiedNetwork::getInstance()->send( clientServiceId, msgout );
	nlinfo( "Asked %s to scan except ignored entities", CUnifiedNetwork::getInstance()->getServiceUnifiedName( clientServiceId ).c_str() );
}


/*
 * Receive a subscription request from a client service, to forward it to all other mirror services
 */
void	CMirrorService::broadcastSubscribeProperty( CMessage& msgin, const std::string& propName, NLNET::TServiceId serviceId )
{
	// Read input msg
	TPropSubscribingOptions options;
	string notifyGroupByPropName;
	msgin.serial( options );
	msgin.serial( notifyGroupByPropName );

	if ( ! (options & PSOWriteOnly) )
	{
		// Broadcast subscription to all other mirror services
		bool subscribe = true;
		CMessage msgout( "SPROP" );
		msgout.serial( subscribe );
		msgout.serial( const_cast<string&>(propName) );
		CUnifiedNetwork::getInstance()->send( "MS", msgout, false );
		nlinfo( "PROPMGT: Sent SPROP (subscribe property) %s to all MS", propName.c_str() );
	}
	
	// Add in local subscribers list
	processPropSubscriptionByName( propName, serviceId, true, (options & PSOReadOnly)!=0, (options & PSONotifyChanges)!=0, notifyGroupByPropName, (options & PSOWriteOnly)!=0 );
}


/*
 * Tell all other MS one service that had subscribed to some properties is leaving
 */
void	CMirrorService::broadcastUnsubscribeProperties( NLNET::TServiceId serviceId )
{
	CMessage msgout( "SPROP" );
	bool subscribeOrUnsubscribe = false;
	msgout.serial( subscribeOrUnsubscribe );
	uint16 size = (uint16)_SDataSets.size();
	msgout.serial( size );

	// Browse datasets
	TSDataSetsMS::iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		CDataSetMS& dataset = GET_SDATASET(ids);
		msgout.serial( const_cast<CSheetId&>(dataset.sheetId()) );

		vector<TPropertyIndex> propIndices;
		TPropertyIndex propIndex;
		for ( propIndex=0; propIndex!= dataset.nbProperties(); ++propIndex )
		{
			// TODO: Skip properties not declared
			TSubscriberListForProp::iterator isl;
			for ( isl=dataset._SubscribersByProperty[propIndex].begin(); isl!=dataset._SubscribersByProperty[propIndex].end(); ++isl )
			{
				CChangeTrackerMS& tracker = (*isl);
				if ( tracker.destServiceId() == serviceId )
				{
					propIndices.push_back( propIndex );
					break;
				}
			}
		}
		msgout.serialCont( propIndices );
	}

	CUnifiedNetwork::getInstance()->send( "MS", msgout, false );
	nlinfo( "PROPMGT: Sent SPROP (UNsubscribe property) to all MS" );
}


/*
 * Process a subscription of property (possibly received from another MS) by name
 */
void	CMirrorService::processPropSubscriptionByName( const std::string& propName, NLNET::TServiceId serviceId, bool local, bool readOnly, bool allocate, const std::string& notifyGroupByPropName, bool writeOnly )
{
	nlinfo( "PROPMGT: Service %s (%s) subscribes to property %s (%s, %sallocated)", servStr(serviceId).c_str(), local?"local client":"MS", propName.c_str(), readOnly?"RO":"W", allocate?"":"not " );

	// Retrieve the dataset and propindex for the property
	TPropertyIndex propIndex;
	CDataSetMS *ds = _PropertiesInMirror.getDataSetByPropName( propName, propIndex );
	if ( ds )
		processPropSubscription( *ds, propIndex, propName, serviceId, local, readOnly, allocate, notifyGroupByPropName, writeOnly );
	else
		nlwarning( "PROPMGT: Invalid property name while receiving subscription" );
}


/*
 * Process a subscription (possibly received from another MS)
 */
void	CMirrorService::processPropSubscription( CDataSetMS& dataset, TPropertyIndex propIndex, const std::string& propName, NLNET::TServiceId serviceId, bool local, bool readOnly, bool allocate, const std::string& notifyGroupByPropName, bool writeOnly )
{
	TPropertyIndex notifyGroupPropIndex = INVALID_PROPERTY_INDEX;
	CChangeTrackerMS *groupTracker = NULL;
	CDataSetMS *ds = &dataset; // quick copy&paste wrapper

	// Retrieve the property index of the group notification property if any
	if ( ! notifyGroupByPropName.empty() )
	{
		CDataSetMS *groupDs = _PropertiesInMirror.getDataSetByPropName( notifyGroupByPropName, notifyGroupPropIndex );
		if ( groupDs != ds )
		{
			nlwarning( "PROPMGT: The group property %hd is not is the same dataset than the subscribed property %hd!", notifyGroupPropIndex, propIndex );
			notifyGroupPropIndex = INVALID_PROPERTY_INDEX;
		}

		// If the group property is already declared, find the tracker
		if ( (notifyGroupPropIndex != INVALID_PROPERTY_INDEX) && (notifyGroupPropIndex != propIndex) )
		{
			if ( ! (groupTracker = ds->findPropTracker( notifyGroupPropIndex, serviceId )) )
				nlwarning( "PROPMGT: Cannot find the tracker of the group property %s", notifyGroupByPropName.c_str() );
		}
	}

	// Check if the serviceId has also a tracker (ex: 2 services declare the same property
	// to their MS A which sends 2 prop subscriptions to a MS B) and its state (read-only...)
	bool mustCreateTracker = true;
	CChangeTrackerMS *newtracker;
	if (! local)
	{
		const CMTRTag& newTagOfMS = getTagOfService( (TServiceId)serviceId );
		if ( (newtracker = ds->findPropTracker( propIndex, serviceId )) != NULL )
		{
			newtracker->addInterestedService();

			if ( hasTagOfServiceChanged( (TServiceId)serviceId ) )
			{
				// Send the tracker to all local services that did not received the tracker at first.
				// In fact, send to all, and discard if received twice.
				mustCreateTracker = false;
			}
			else
			{
				// No need to create the tracker, no need to communicate it to the local services
				nldebug( "PROPMGT: Service %s already subscribed to %s", servStr(serviceId).c_str(), propName.c_str() );
				return;
			}
		}
	}

	// Add to subscribers list

	// Create and allocate the new tracker
	if ( mustCreateTracker )
	{
		newtracker = &ds->addPropTracker( propIndex, serviceId, local, readOnly, allocate, writeOnly );
		newtracker->addInterestedService();
		if ( groupTracker )
		{
			// Point to the group tracker
			newtracker->useGroupTracker( groupTracker );
			nldebug( "PROPMGT: Using group tracker of prop %s (propindex %hd)", notifyGroupByPropName.c_str(), notifyGroupPropIndex );
		}
		else
		{
			// Allocate the new tracker
			sint32 smid;
			sint32 mutid;

			// TODO: do not add if there are only read-only subscribers for the prop, or even no subscriber
			//       (but defer for when a subscribe logs on later);
			if ( allocate )
			{
				// Allocate shared memory for the tracker, except if it's a local tracker with no notification
				do
				{
					smid = _SMIdPool.getNewId();
					if ( smid == InvalidSMId )
					{
						nlwarning( "ROWMGT: No more free shared memory ids (prop tracker)" );
						beep( 660, 150 ); // TODO: error handling
						return;
					}
					if ( ! newtracker->allocate( smid, ds->maxNbRows() ) )
						smid = InvalidSMId;
				}
				while ( smid == InvalidSMId ); // iterate to force to get another smid when chosen smid is locked by another program
				nldebug( "PROPMGT: Prop tracker for P%hd of service %s has smid %d", propIndex, servStr(serviceId).c_str(), smid );

				// Create the mutex
				while ( (! newtracker->createMutex( (mutid = _MutIdPool.getNewId()), true )) && (mutid != InvalidSMId) )
					nlwarning( "ROWMGT: Skipping mutex id %d for creation of prop entity tracker", mutid );
				if ( mutid == InvalidSMId )
				{
					nlwarning( "ROWMGT: Failed to find an id and create mutex for a prop tracker" );
					beep( 660, 150 ); // TODO: error handling
					return;
				}
			}

			// Set as new group tracker if it is the first of a group-notified tracker series
			if ( notifyGroupPropIndex != INVALID_PROPERTY_INDEX )
			{
				newtracker->useGroupTracker( NULL );
				nldebug( "PROPMGT: Tracker is a group tracker" );
			}

			// Link to delta buffer (for a remote tracker)
			if ( ! local )
			{
				CDeltaToMS *delta = _RemoteMSList.getDeltaToMS( (TServiceId)serviceId, &dataset );
				nlassert( delta );
				newtracker->setLinkToDeltaBuffer( delta ); // see comment in allocateEntityTrackers()
			}
		}
	}
	if ( ! local )
		nlassert( newtracker->getLinkToDeltaBuffer() );

	CMTRTag tagOfSubscriptor = getTagOfService( (TServiceId)serviceId );
	nldebug( "PROPMGT: Subscriptor has tag %d", tagOfSubscriptor.rawTag() );

	uint8 nb;
	bool self;
	const TSubscriberListForProp& propTrackers = ds->getPropTrackers( propIndex );

	if ( allocate )
	{
		// This message will be sent to all other local services, if the tracker was allocated (external tracker or local with notification)
		CMessage msgout1( "ATP" );
		nb = 1;
		self = false;
		msgout1.serial( const_cast<string&>(propName) );
		msgout1.serial( nb );
		msgout1.serial( self );
		msgout1.serial( const_cast<sint32&>(newtracker->smid()) );
		msgout1.serial( const_cast<sint32&>(newtracker->mutid()) );
		msgout1.serial( serviceId );

		TSubscriberListForProp::const_iterator it;
		for ( it=propTrackers.begin(); it!=propTrackers.end(); ++it )
		{
			if ( (*it).isLocal() && !(*it).isReadOnly() )
			{
				// Comment out the following test if you want the services to be notified for their own changes
				if ( (*it).destServiceId() != serviceId )
				{
					if ( (_ClientServices.find( (*it).destServiceId() ) != _ClientServices.end()) && // skip tracker if the destination service is down (because the trackers remain in the list for some time after service disconnection, see deferTrackerRemovalForQuittingService())
						 tagOfSubscriptor.doesAcceptTag( getTagOfService( (*it).destServiceId() ) ) )
					{
						// Send the new tracker to each other local service
						CUnifiedNetwork::getInstance()->send( (*it).destServiceId(), msgout1 );
						++_NbExpectedATAcknowledges[(*it).destServiceId()];
						nldebug( "++_NbExpectedATAcknowledges -> %d", _NbExpectedATAcknowledges[(*it).destServiceId()] );
					}
				}
			}
		}
	}
	
	if ( local && (allocate || (!readOnly)) )
	{
		CMessage msgout2( "ATP" );
		msgout2.serial( const_cast<string&>(propName) );
		if ( ! readOnly )
		{
			// Send all allocated trackers' smids for the property (including self) to the subscriber for him to be able to write
			sint32 nbTrackersBufPos = msgout2.reserve( sizeof(nb) );
			nb = 0;

			TSubscriberListForProp::const_iterator it;
			for ( it=propTrackers.begin(); it!=propTrackers.end(); ++it )
			{
				// If the tracker is not allocated (smid -1), it will be skipped by the subscriber
				NLNET::TServiceId destServId = (*it).destServiceId();

				if ( (destServId == serviceId) || // self tracker
					 (getTagOfService( (TServiceId)destServId ).doesAcceptTag( tagOfSubscriptor )) )
				{
					// Skip tracker if the destination service is down (because the trackers remain in the list for some time after service disconnection, see deferTrackerRemovalForQuittingService())
					if ( (! ((*it).isLocal() && (_ClientServices.find( (*it).destServiceId() ) == _ClientServices.end() ))) &&
						 (! CUnifiedNetwork::getInstance()->getServiceName( destServId ).empty()) )
					{
						self = ( destServId == serviceId );
						msgout2.serial( self );
						msgout2.serial( const_cast<sint32&>((*it).smid()) );
						msgout2.serial( const_cast<sint32&>((*it).mutid()) );
						msgout2.serial( destServId );
						++nb;
					}
				}
			}

			msgout2.poke( nb, nbTrackersBufPos );

			// Increment the number of writer properties for the dataset
			ds->incNbLocalWriterProps();
			nldebug( "%s_NbLocalWriterProps up to %u", ds->name().c_str(), ds->_NbLocalWriterProps );
		}
		else // if ( allocate ) // already excluded by "if ( local && (allocate || (!readOnly)) )"
		{
			// Send the self tracker's smid to the subscriber for his notification
			nb = 1;
			self = true;
			msgout2.serial( nb );
			msgout2.serial( self );
			msgout2.serial( const_cast<sint32&>(newtracker->smid()) );
			msgout2.serial( const_cast<sint32&>(newtracker->mutid()) );
			msgout2.serial( serviceId );
		}

		// Send
		CUnifiedNetwork::getInstance()->send( serviceId, msgout2 );
		_NbExpectedATAcknowledges[serviceId] += nb;
		nldebug( "+++_NbExpectedATAcknowledges -> %d", _NbExpectedATAcknowledges[serviceId] );
		/*if ( serviceId != _ClientServices.end() )
			++GET_CLIENT_SERVICE_INFO(ics).NbExpectedATAcknowledges;
		else
			nlwarning( "Sending ATP to an unknown client service %hu", serviceId );*/
	}

	// Operation now done by the subscriber when it receives an acknowledge of ATP (ACKATP)
	/*if ( allocate )
	{
		// Fill tracker with non-zero values
		if ( ds->_PropertyContainer.PropertyValueArrays[propIndex].Values ) // only if already allocated
		{
			uint nbChanges = 0;
			for ( TDataSetRow i=0; i!=ds->maxNbRows(); ++i )
			{
				if ( ds->_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[i] != 0 )
				{
					newtracker->recordChange( i );
					++nbChanges;
					//nldebug( "PROMPMGT: smid %d: marked E%d as changed", newtracker->smid(), i );
				}
			}
			nlinfo( "PROPMGT: New tracker (smid %d): filled %u changes for P%hd (%s)", newtracker->smid(), nbChanges, propIndex, ds->getPropertyName(propIndex).c_str() );
		}
		else
		{
			nldebug( "PROPMGT: Values not ready (filling changes of P%hd)", propIndex );
		}
	}*/
}


/*
 * Process an unsubscription of property from another MS (corresponding to one client service leaving)
 */
void	CMirrorService::processPropUnsubscription( CMessage& msgin, NLNET::TServiceId serviceId )
{
	map< NLNET::TServiceId, vector<sint32> > servicesForTrackerRemoval;
	uint16 size;
	msgin.serial( size );

	// Browse datasets
	uint ids;
	for ( ids=0; ids!=size; ++ids )
	{
		CSheetId sheetId;
		msgin.serial( sheetId );
		TSDataSetsMS::iterator isd = _SDataSets.find( sheetId );
		if ( isd != _SDataSets.end() )
		{
			CDataSetMS& dataset = GET_SDATASET(isd);

			vector<TPropertyIndex> propIndices;
			msgin.serialCont( propIndices );

			// Browse the trackers for the unsubscribing properties
			vector<TPropertyIndex>::iterator ipi;
			for ( ipi=propIndices.begin(); ipi!=propIndices.end(); ++ipi )
			{
				CChangeTrackerMS *tracker;
				if ( (tracker = dataset.findPropTracker( *ipi, serviceId )) != NULL )
				{
					// Check if the property tracker is still useful (if an interested service remains on the remote machine)
					nlassert( tracker->nbInterstedServices() > 0 );
					tracker->remInterestedService();
					if ( tracker->nbInterstedServices() == 0 )
					{
						// Tell every other local service that has the tracker to remove it
						for ( TSubscriberListForProp::iterator isl=dataset._SubscribersByProperty[*ipi].begin(); isl!=dataset._SubscribersByProperty[*ipi].end(); ++isl )
						{
							if ( (*isl).isLocal() /*&& ((*isl).destServiceId() != serviceId)*/ ) // useless because serviceId is a remote MS
							{
								//nlinfo( "ADDING TO REMOVE LIST OF %s", servStr((*isl).destServiceId()).c_str() );
								// In fact we don't check if the service has the tracker, but no matter if it hasn't, it will just ignore the smid
								servicesForTrackerRemoval[(*isl).destServiceId()].push_back( tracker->smid() );
							}
						}

						// Remove the tracker
						deleteTracker( *tracker, dataset._SubscribersByProperty[*ipi] );
					}
				}
				else
				{
					nlwarning( "PROPMGT: Tracker not found (unsubscribing P%hd from %s, normal if local services have left)", *ipi, servStr(serviceId).c_str() );
				}
			}
		}
		else
		{
			nlerror( "MS datasets do not match" );
		}
	}

	sendTrackersToRemoveToLocalServices( servicesForTrackerRemoval );
}


/*
 * Scan for existing entities in the received ranges
 */
void	CMirrorService::receiveAcknowledgeAddEntityTrackerMS( NLNET::CMessage& msgin, NLNET::TServiceId serviceIdFrom, bool isTrackerAdded )
{
	bool isInitialTransmitTracker;
	string datasetname;
	msgin.serial( isInitialTransmitTracker );

	msgin.serial( datasetname );
	try
	{
		// Send message to the remote MS with the list of online entities, and their full datasetrow.
		// The remote MS will have to apply them as a resync. This is mandatory because the remote MS
		// could be not aware of the entities, if it did not receive the entity additions/removals in
		// case of an incompatible MTR tag.
		CDataSetMS& dataset = NDataSets[datasetname];
		if ( dataset._PropertyContainer.EntityIdArray.EntityIds ) // only if already allocated
		{
			// Find the addition tracker to the remote MS
			sint32 smid;
			msgin.serial( smid );
			CChangeTrackerMS *addingEntityTracker = dataset.findEntityTrackerBySmid( ADDING, smid );
			if ( addingEntityTracker )
			{
				if ( addingEntityTracker->isLocal() )
				{
					// This can occur if a local service B sends ACKATE to a service A, immediately after A leaves.
					// As B::isServiceLocal(A) will fail, B will think A is remote and send ACKATE (via FWD_ACKATE)
					// to us (local MS). As the ACKATE is obsolete, we just return.
					nlinfo( "ROWMGT: Ignoring FWD_ACKATE to remote MS for local tracker %u", smid );
					return;
				}

				// For a new local client service, we did not block the tracker to the remote MS by calling
				// pushATEAcknowledgesExpected(). However, we may have to ensure it is open because it
				// could be blocked before.
				if ( isInitialTransmitTracker )
				{
					if ( isTrackerAdded &&
						 (! addingEntityTracker->isAllowedToSendToRemoteMS()) &&
						 (addingEntityTracker->nbATEAcksExpectedBeforeUnblocking() == 0) )
					{
						nlinfo( "ROWMGT: Unblocking tracker to MS-%hu for new writer client service %s", 
							addingEntityTracker->destServiceId().get(),
							CUnifiedNetwork::getInstance()->getServiceUnifiedName( serviceIdFrom ).c_str() );
						addingEntityTracker->allowSendingToRemoteMS( true );
						CChangeTrackerMS *removingEntityTracker = dataset.findEntityTracker( REMOVING, addingEntityTracker->destServiceId() );
						if ( removingEntityTracker )
							removingEntityTracker->allowSendingToRemoteMS( true ); // may be already open, if it was open and not closed before
						else
							nlwarning( "Can't find REMOVING tracker (smid %d)", smid );
					}
				}
				else
				{
					addingEntityTracker->popATEAcknowledgesExpected( serviceIdFrom, isTrackerAdded );
				}
				nlinfo( "ROWMGT: Decoded ACKATE from %s for remote MS-%hu", servStr(serviceIdFrom).c_str(), addingEntityTracker->destServiceId().get() );
			}
			else
				nlwarning( "Can't find ADDING tracker (smid %d)", smid );
			/*// Obsolete, now replaced by one complete sync in buildAndSentDeltas()
			if ( itl!=subList.end() )
			{
				uint32 nbRanges;
				msgin.serial( nbRanges );
				for ( uint i=0; i!=nbRanges; ++i )
				{
					TDataSetIndex first, last;
					msgin.serial( first, last );
					uint nbAdded = 0;
					for ( TDataSetIndex i=first; i!=(last+1); ++i )
					{
						const CEntityId& entityId = dataset._PropertyContainer.EntityIdArray.EntityIds[i];

						// Add all online entities, created by the sender
						// (note: even if some entities have been added into
						// the tracker before, there have not been sent to the remote MS because the
						// sending is blocked while the tracker is not acknowledged).

						if ( (! entityId.isUnknownId()) &&
							 dataset._PropertyContainer.EntityIdArray.isOnline( i ) &&
							 dataset.getSpawnerServiceId( i ) == serviceIdFrom )
						{
							//nldebug( "ROWMGT: Adding E%d into EntityTrackerAdding (smid %d) for %s", i, entityTrackerAdding.smid(), servStr(serviceId).c_str() );
							(*itl).recordChange( i );
							++nbAdded;
						}
					}
					nlinfo( "ROWMGT: AddingTracker (smid %d): filled %u entities to %s in %s from %s, range [%d..%d]", (*itl).smid(), nbAdded, CUnifiedNetwork::getInstance()->getServiceUnifiedName( (*itl).destServiceId() ).c_str(), dataset.name().c_str(), CUnifiedNetwork::getInstance()->getServiceUnifiedName( serviceIdFrom ).c_str(), first, last );
				}
				if ( nbRanges == 0 )
				{
					nlinfo( "ROWMGT: AddingTracker (smid %d): no entity to add", smid );
				}

				// Unblock the adding tracker
				(*itl).allowSendingToRemoteMS( true );

				// Unblock the removing tracker as well, for the same destination remote MS
				TSubscriberListForEnt& subList2 = const_cast<TSubscriberListForEnt&>(dataset.getEntityTrackers( REMOVING ));
				TSubscriberListForEnt::iterator itl2;
				for ( itl2=subList2.begin(); itl2!=subList2.end(); ++itl2 )
				{
					if ( (*itl2).destServiceId() == (*itl).destServiceId() )
					{
						(*itl2).allowSendingToRemoteMS( true );
					}
				}
			}*/
		}
	}
	catch(const EMirror& )
	{
		nlwarning( "Invalid dataset name %s for receiving ack of addEntityTracker", datasetname.c_str() );
	}
}


/*
 * Scan for non-null property values, written by the specified service
 */
void	CMirrorService::receiveAcknowledgeAddPropTrackerMS( NLNET::CMessage& msgin, NLNET::TServiceId serviceIdFrom )
{
	string propName;
	sint32 smid;
	msgin.serial( propName );
	msgin.serial( smid );
	TPropertyIndex propIndex;
	CDataSetMS *ds = _PropertiesInMirror.getDataSetByPropName( propName, propIndex );
	if ( ds )
	{
		// Fill tracker with non-zero values
		if ( ds->_PropertyContainer.PropertyValueArrays[propIndex].Values ) // only if already allocated
		{
			// Find the tracker
			CChangeTrackerMS *tracker = ds->findPropTrackerBySmid( propIndex, smid );
			if ( tracker )
			{
				if ( tracker->isLocal() )
				{
					// This can occur if a local service B sends ACKATP to a service A, immediately after A leaves.
					// As B::isServiceLocal(A) will fail, B will think A is remote and send ACKATP (via FWD_ACKATP)
					// to us (local MS). As the ACKATP is obsolete, we just return.
					nlinfo( "PROPMGT: Ignoring FWD_ACKATP to remote MS for local tracker %u", smid );
					return;
				}

				// Scan non-null values and add them
				uint nbChanges = 0;
				for ( TDataSetIndex i=0; i!=ds->maxNbRows(); ++i )
				{
					if ( ds->_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[i] != 0 )
					{
						// Don't send properties of offline entities, they will be set as changed in declareEntity()
						// Property timestamps are reset when an entity is deleted => non-zero timestamps indicate properties for current entity in row
						if ( ds->_PropertyContainer.EntityIdArray.isOnline( i ) &&
							 (ds->_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[i] != 0) )
						{
							// Notify only the values modified by the specified service (will not work if another service overwrites it!)
							if ( ds->_PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[i] == serviceIdFrom )
							{
								tracker->recordChange( i );
								++nbChanges;
								//nldebug( "PROMPMGT: smid %d: marked E%d as changed", newtracker->smid(), i );
							}
						}
					}
				}
				nlinfo( "PROPMGT: Tracker (smid %d): filled %u changes for %s %s/P%hd from %s", tracker->smid(), nbChanges, CUnifiedNetwork::getInstance()->getServiceUnifiedName( tracker->destServiceId() ).c_str(), ds->name().c_str(), propIndex, CUnifiedNetwork::getInstance()->getServiceUnifiedName( serviceIdFrom ).c_str() );
				// Note: unlike in CMirror::receiveAcknowledgeAddPropTracker(), this will not lead to
				// unjustified changes because the tracker was blocked until now

				// Unblock the tracker
				tracker->allowSendingToRemoteMS( true );
			}
			else
			{
				// This can occur when a service disconnects while another new service is being
				// acknowledging its trackers
				nlwarning( "Can't find tracker for prop %s (P%hd, smid %u) in %s", propName.c_str(), propIndex, smid, ds->name().c_str() );
			}
		}
		else
		{
			nldebug( "Values not ready (filling changes of P%hd)", propIndex );
		}
	}
	else
	{
		nlwarning( "MIRROR: Invalid property name %s for receiving ack of AddPropTracker", propName.c_str() );
	}
}


/*
 * Forward ACKATE from a local service to a local service, if their tags allow it
 */
void	CMirrorService::forwardLocalACKATE( NLNET::CMessage& msgin, NLNET::TServiceId srcServiceId, NLNET::TServiceId destServiceId )
{
	// A service with incompatible tag will not receive ATE, then will not send ACKATE.
	// It can't forward if the destination service does not want to receive entities with Tag
	if ( ! getTagOfService( destServiceId ).doesAcceptTag( getTagOfService( srcServiceId ) ) )
	{
		nlwarning( "ERROR: Forwarding ACKATE from %s to %s not allowed", servStr(srcServiceId).c_str(), servStr(destServiceId).c_str() );
		return;
	}
	CMessage msgout( "ACKATE" );
	msgout.serialBuffer( const_cast<uint8*>(msgin.buffer() + msgin.getPos()), msgin.length() - msgin.getPos() );
	CUnifiedNetwork::getInstance()->send( destServiceId, msgout );
	nldebug( "ROWMGT: Forwarded ACKATE from %s to %s", servStr(srcServiceId).c_str(), servStr(destServiceId).c_str() );
}


/*
 * Send to the remote MS all it needs to be sync'd with us (row management & property subscriptions, etc.)
 */
void	CMirrorService::synchronizeSubscriptionsToNewMS( TServiceId newRemoteMSId )
{
	CMessage msgout( "SYNC_MS" );

	// Send the MS version
	msgout.serial( const_cast<string&>(MirrorServiceVersion) );

	// Send the main tag
	msgout.serial( mainTag() );

	// Send sync corresponding to MARCS message (list of client services)
	uint32 nbClientServices = (uint32)_ClientServices.size();
	msgout.serial( nbClientServices );
	TClientServices::const_iterator ics;
	for ( ics=_ClientServices.begin(); ics!=_ClientServices.end(); ++ics )
	{
///		msgout.serial( const_cast<uint16&>((*ics).first) );
		nlWrite(msgout, serial, ics->first);
//		msgout.serial( const_cast<CMTRTag&>(getTagOfService( (*ics).first )) );
		nlWrite(msgout, serial, getTagOfService(ics->first));
	}

	// Browse datasets
	uint32 nbSubscribedDatasets = 0;
	sint32 posNbSubscribedDatasets = msgout.reserve( sizeof(nbSubscribedDatasets) );
	TSDataSetsMS::iterator ids;
	for ( ids=_SDataSets.begin(); ids!=_SDataSets.end(); ++ids )
	{
		CDataSetMS& dataset = GET_SDATASET(ids);

		// If "entity property" not subscribed yet, we must not subscribe to dataset entities on the
		// remote MS, because we won't be able to receive deltas for this dataset
		if ( ! dataset._PropertyContainer.EntityIdArray.EntityIds )
			continue;

		// Name of used dataset (row management)
		++nbSubscribedDatasets;
		msgout.serial( const_cast<string&>(dataset.name()) );
		//nldebug( "> %s", dataset.name().c_str() );

		// Entity subscriptions
		vector<TServiceId> subscribedServices;
		TSubscriberListForEnt::iterator isl;
		for ( isl=dataset._EntityTrackers[ADDING].begin(); isl!=dataset._EntityTrackers[ADDING].end(); ++isl )
		{
			CChangeTrackerMS& tracker = (*isl);
			if ( tracker.isLocal() )
			{
				subscribedServices.push_back( tracker.destServiceId() );
			}
		}
		msgout.serialCont( subscribedServices );

		// Property subscriptions
		vector<TPropertyIndex> propIndices;
		TPropertyIndex propIndex;
		for ( propIndex=0; propIndex!= dataset.nbProperties(); ++propIndex )
		{
			// TODO: Skip properties not declared
			TSubscriberListForProp::iterator isl;
			for ( isl=dataset._SubscribersByProperty[propIndex].begin(); isl!=dataset._SubscribersByProperty[propIndex].end(); ++isl )
			{
				const CChangeTrackerMS& tracker = (*isl);
				if ( tracker.isLocal() && (! tracker.isWriteOnly()) )
				{
					propIndices.push_back( propIndex );
					//nldebug( "> P%hd", propIndex );
				}
			}
		}
		msgout.serialCont( propIndices );

		// The online entities will be emitted when a service on the new machine
		// declare its datasets (or when its MS syncs out to us the service info if it was
		// declared before). But first, we need to send the binding counters etc.
		dataset.transmitAllBindingCounters( msgout, _ClientServices, false, false );
	}

	msgout.poke( nbSubscribedDatasets, posNbSubscribedDatasets );
	CUnifiedNetwork::getInstance()->send( newRemoteMSId, msgout );
	nlinfo( "Sent SYNC_MS to new MS-%hu for %u datasets", newRemoteMSId.get(), nbSubscribedDatasets );

	// Send an empty delta update to unblock the new MS
	//CMessage msgout( "DELTA" );
	//CUnifiedNetwork::getInstance()->send( newRemoteMSId, msgout );
}


/*
 * Receive SYNC_MS
 */
void	CMirrorService::receiveSyncFromRemoteMS( CMessage& msgin, TServiceId srcRemoteMSId )
{
	// MS version
	string remoteMSVersion;
	msgin.serial( remoteMSVersion );
	if ( MirrorServiceVersion != remoteMSVersion )
		nlerror( "MS version mismatch! This MS: %s; Remote MS-%hu: %s", MirrorServiceVersion.c_str(), srcRemoteMSId.get(), remoteMSVersion.c_str() ); 

	// MS Tag
	CMTRTag tag;
	msgin.serial( tag );
	setNewTag( srcRemoteMSId, tag );

	// Remote client services
	uint32 nbClientServices;
	msgin.serial( nbClientServices );
	for ( uint i=0; i!=nbClientServices; ++i )
	{
		TServiceId clientServiceId;
		msgin.serial( clientServiceId );
		CMTRTag tagOfClientService;
		msgin.serial( tagOfClientService );
		addRemoveRemoteClientService( (TServiceId)clientServiceId, (TServiceId)srcRemoteMSId, true, tagOfClientService );
	}

	// Datasets
	uint32 nbSubscribedDatasets;
	msgin.serial( nbSubscribedDatasets );
	for ( uint i=0; i!=nbSubscribedDatasets; ++i )
	{
		string datasetname;
		msgin.serial( datasetname );
		nlinfo( "Receiving SYNC_MS from MS-%hu for dataset %s", srcRemoteMSId.get(), datasetname.c_str() );
		TNDataSetsMS::iterator ids = NDataSets.find( datasetname );
		if ( ids != NDataSets.end() )
		{
			CDataSetMS& dataset = GET_NDATASET(ids);

			// Entity subscriptions
			vector<TServiceId> subscribedServices;
			msgin.serialCont( subscribedServices );
			for ( uint iss=0; iss!=subscribedServices.size(); ++iss )
			{
				const CMTRTag& tag = getTagOfService( subscribedServices[iss] ); // just set at the beginning of the current method
				processDataSetEntitiesSubscription( datasetname, srcRemoteMSId, subscribedServices[iss], tag );
			}

			// Properties
			vector<TPropertyIndex> propIndices;
			msgin.serialCont( propIndices );
			for ( uint ipi=0; ipi!=propIndices.size(); ++ipi )
			{
				//nldebug( "> P%hd", propIndices[ipi] );
				string propName = dataset.getPropertyName( propIndices[ipi] );
				processPropSubscription( dataset, propIndices[ipi], propName, srcRemoteMSId, false, false, true, "" );
			}

			// Binding counters
			dataset.receiveAllBindingCounters( msgin ); // note: this method *must* read the msg to move the position for the next dataset
		}
		else
		{
			nlerror( "Dataset %s not found", datasetname.c_str() );
		}
	}
}


/*
 * Remove all entities in the specified ranges (received from the range manager when a MS is down)
 */
void	CMirrorService::releaseEntitiesInRanges( CDataSetMS& dataset, const std::vector<TRowRange>& erasedRanges )
{
	std::vector<TServiceId8> emptyList;
	setupDestEntityTrackersInterestedByDelta( dataset, AllTag, REMOVING, emptyList ); // notify all, as we don't know the tag of the down MS (client services not knowing an entity will produce some warnings)
	if ( dataset._PropertyContainer.EntityIdArray.EntityIds )
	{
		vector<TRowRange>::const_iterator ier;
		for ( ier=erasedRanges.begin(); ier!=erasedRanges.end(); ++ier )
		{
			uint nbChanges = 0;
			for ( TDataSetIndex i=(*ier).First; i<=(*ier).Last; ++i )
			{
				if ( dataset._PropertyContainer.EntityIdArray.isOnline( i ) )
				{
					uint8 newCounter = dataset.getBindingCounter( i ) + 1;
					if ( newCounter == 0 )
						++newCounter; // skip 0
					TDataSetRow datasetrow( i, newCounter );
					dataset.removeEntityFromDataSet( datasetrow );
					++nbChanges;
				}
			}
			nlinfo( "ROWMGT: Removed %u entities of owner %s (range [%u..%u])", 
				nbChanges, 
				servStr((*ier).OwnerServiceId).c_str(), 
				(*ier).First, (*ier).Last );
		}
	}
}


/*
 * Receive SYNCMI
 */
void	CMirrorService::receiveSyncMirrorInformation( CMessage& msgin, TServiceId serviceId )
{
	nlwarning( "TODO: Resync from client service not supported with multi-MS" );

	// Receive mirror info of entity types owned for each dataset, and trackers, and properties

	uint16 nbDatasets;
	msgin.serial( nbDatasets );
	uint i;
	try
	{
		for ( i=0; i!=nbDatasets; ++i )
		{
			string datasetName;
			msgin.serial( datasetName );
			CDataSetMS& dataset = NDataSets[datasetName];

			// Owned ranges
			uint16 nbRanges;
			msgin.serial( nbRanges );
			for ( uint j=0; j!=nbRanges; ++j )
			{
				uint8 entityTypeId;
				msgin.serial( entityTypeId );
				TClientServiceInfoOwnedRange newrange( &dataset );
				TEntityRange entityRange;
				msgin.serial( entityRange );
				newrange.fillFromEntityRange( entityRange );
				_ClientServices[serviceId].OwnedRanges.push_back( newrange ); // does _ClientService.insert() automatically
				nlinfo( "ROWMGT: Readding range for type %hu owned by %s", (uint16)entityTypeId, servStr(serviceId).c_str() );
			}

			// Allocated prop and entity trackers
			dataset.readdAllocatedTrackers( msgin, serviceId, _SMIdPool, _MutIdPool );
		}

		// Reacquire ranges to Range Manager Service
		CMessage msgout( "RARG" );
		uint16 nbServices = 1;
		msgout.serial( nbServices );
		TClientServices::const_iterator ics;
		msgout.serial( serviceId );
		TClientServiceInfo& clientServiceInfo = _ClientServices[serviceId];
		uint16 nbRanges = (uint16)(clientServiceInfo.OwnedRanges.size());
		msgout.serial( nbRanges );
		if ( nbRanges != 0 )
		{
			vector<TClientServiceInfoOwnedRange>::const_iterator ior;
			for ( ior=clientServiceInfo.OwnedRanges.begin(); ior!=clientServiceInfo.OwnedRanges.end(); ++ior )
			{
				msgout.serial( const_cast<string&>((*ior).DataSet->name()) );
				msgout.serial( const_cast<TDataSetIndex&>((*ior).First) );
				msgout.serial( const_cast<TDataSetIndex&>((*ior).Last) );
			}
		}
		CUnifiedNetwork::getInstance()->send( RANGE_MANAGER_SERVICE, msgout );
		nlinfo( "ROWMGT: Requested reacquisition of %hu ranges for %s", nbRanges, servStr(serviceId).c_str() );

		// Properties
		uint nbTrackersAdded = 0;
		uint16 nbProps;
		msgin.serial( nbProps );
		for ( uint p=0; p!=nbProps; ++p )
		{
			string propName;
			msgin.serial( propName );
			TPropertyInfo propInfoClient;
			msgin.serial( propInfoClient );

			TPropertiesInMirrorMS::iterator ipim = _PropertiesInMirror.find( propName );
			if ( ipim != _PropertiesInMirror.end() )
			{
				TPropertyInfoMS& propInfo = GET_PROPERTY_INFO(ipim);
				if ( ! propInfo.allocated() )
				{
					// Reaccess shared memory
					_SMIdPool.reacquireId( propInfoClient.SMId );
					_PropertiesInMirror[propName].reaccess( propInfoClient.SMId );

					// Set property pointers but don't init values
					uint32 dataTypeSize = (propInfo.PropertyIndex != -1) ? propInfo.DataSet->getDataSizeOfProp( propInfo.PropertyIndex ) : sizeof(CEntityId);
					propInfo.DataSet->setPropertyPointer( propName, propInfo.PropertyIndex, propInfo.Segment, false, dataTypeSize, false, false ); // readOnly and monitorAssignment are used only by client services

					nlinfo( "SHDMEM: Reaccessed property %s", propName.c_str() );
				}

				if ( propInfo.PropertyIndex != -1 ) // avoid dummy entity_id prop
				{
					CChangeTrackerMS *tracker = propInfo.DataSet->findPropTracker( propInfo.PropertyIndex, serviceId );
					if ( tracker )
					{
						// The tracker was added before (because it's allocated) => set its flags
						tracker->resetFlags( propInfoClient.flagReadOnly(), propInfoClient.flagWriteOnly() );
					}
					else
					{
						// The tracker is not here yet, add it as 'not allocated' and set its flags
						propInfo.DataSet->readdTracker( serviceId, propInfo.PropertyIndex, propInfoClient.flagReadOnly(), propInfoClient.flagWriteOnly(), propInfoClient.flagGroupNotifiedByOtherProp(), propInfoClient.propNotifyingTheGroup() );
						++nbTrackersAdded;
					}

					if ( ! propInfoClient.flagReadOnly() )
					{
						propInfo.DataSet->incNbLocalWriterProps();
						nldebug( "%s_NbLocalWriterProps up to %u", propInfo.DataSet->name().c_str(), propInfo.DataSet->_NbLocalWriterProps );
					}
					
					
				}
			}
			else
			{
				nlwarning( "PROPMGT: Property %s not found while resyncing from %s", propName.c_str(), servStr(serviceId).c_str() );
			}
		}
		nlinfo( "TRACKER: Reaccessed %u unallocated/pointing prop trackers for service %hu", nbTrackersAdded, serviceId.get() );

		// Tell again all other MS that we are in charge of this service
		CMessage msgoutM( "MARCS" ); // Mirror Add/Remove Client Service
		msgoutM.serial( serviceId );
		bool addOrRemove = true;
		msgoutM.serial( addOrRemove );
		CMTRTag tagOfNewClientService; // TODO
		msgoutM.serial( tagOfNewClientService );
		msgoutM.serial( const_cast<string&>(MirrorServiceVersion) );
		CUnifiedNetwork::getInstance()->send( "MS", msgoutM, false );
		
		nlinfo( "Resync from client service %s succeeded", servStr(serviceId).c_str() );

	}
	catch (const EMirror&)
	{
		nlwarning( "Dataset not found (SYNCMI)" );
	}
}


/*
 *
 */
void	TPropertyInfoMS::reaccess( sint32 shmid )
{
	SMId = shmid;
	Segment = CSharedMemory::accessSharedMemory( toSharedMemId(SMId) );
}


/*
 * Change a weight
 */
void	CMirrorService::changeWeightOfProperty( const std::string& propName, sint32 weight )
{
	TPropertiesInMirrorMS::iterator ip = _PropertiesInMirror.find( propName );
	if ( ip != _PropertiesInMirror.end() )
	{
		TPropertyInfoMS& propInfo = GET_PROPERTY_INFO(ip);
		nlassert( propInfo.PropertyIndex < propInfo.DataSet->nbProperties() );
		propInfo.DataSet->changeWeightOfProperty( propInfo.PropertyIndex, weight );
	}
	else
	{
		nlwarning( "Property %s not found", propName.c_str() );
	}
}


/*
 * Add or remove a remote client service (supports multiple adding if the same)
 */
void	CMirrorService::addRemoveRemoteClientService( TServiceId clientServiceId, TServiceId msId, bool addOrRemove, const CMTRTag& tagOfNewClientService )
{
	if ( addOrRemove )
	{
		nlinfo( "Adding remote client %s of MS-%hu at %u", servStr( clientServiceId ).c_str(), msId.get(), CTickProxy::getGameCycle() );
		testIfNotInQuittingServices( (TServiceId)clientServiceId );
		_RemoteMSList.addRemoteClientService( clientServiceId, msId );
		setNewTag( clientServiceId, tagOfNewClientService );
	}
	else
	{
		nlinfo( "Removing remote client %s of MS-%hu at %u", servStr( clientServiceId ).c_str(), msId.get(), CTickProxy::getGameCycle() );
		_RemoteMSList.removeRemoteClientService( clientServiceId );
		setNewTag( clientServiceId, IniTag );
	}
}


/*
 * Return the stored number of changes emitted to other MS for row management or props
 */
sint	CRemoteMSList::getNbChangesSentPerTick( bool rowmgt ) const
{
	sint sum = 0;
	TServiceIdList::const_iterator irm;
	for ( irm=_ServiceIdList.begin(); irm!=_ServiceIdList.end(); ++irm )
	{
		TDeltaToMSList::const_iterator idl;
		for ( idl=_DeltaToMSArray[irm->get()].begin(); idl!=_DeltaToMSArray[irm->get()].end(); ++idl )
		{
			if ( rowmgt )
				sum += (*idl)->getStoredNbChangesPushedRowMgt();
			else
				sum += (*idl)->getStoredNbChangesPushedProps();
		}
	}
	return sum;
}


/*
 * Store the message for later sending within delta packet to the corresponding mirror service
 */
void	CRemoteMSList::pushMessageToRemoteQueue( TServiceId destId, TServiceId senderId, NLMISC::CMemStream& msgin )
{
	if ( destId == DEST_MSG_BROADCAST )
	{
		TServiceIdList::iterator itMs;
		for ( itMs=begin(); itMs!=end(); ++itMs )
		{
			doPushMessageToDelta( *itMs, destId, senderId, msgin );
		}
	}
	else
	{
		TServiceId8 msId = _CorrespondingMS[TServiceId8(destId).get()];
		if ( msId.get() != 0 )
		{
			doPushMessageToDelta( msId, destId, senderId, msgin );
		}
		else
		{
			CMessage msg( msgin );
			string msgName;
			getNameOfMessageOrTransportClass( msg, msgName );
			nlwarning( "MSG: Can't locate MS corresponding to service %s (%s from %s)", servStr(destId).c_str(), msgName.c_str(), servStr(senderId).c_str() );
		}
	}
}


/*
 * Helper for pushMessageToRemoteQueue
 */
void	CRemoteMSList::doPushMessageToDelta( TServiceId msId, TServiceId destId, TServiceId senderId, NLMISC::CMemStream& msg )
{
	//CMessage mmsg( msg ); // TEMP-debug
	//nldebug( "MSG: Pushing message from %s for %s via MS-%hu (msg %s, %u b)", servStr(senderId).c_str(), servStr(destId).c_str(), msId, mmsg.getName().c_str(), msg.length() );
	TDeltaToMSList& dtml = getDeltaToMSList( msId );
	nlassert( ! dtml.empty() );
	dtml[0]->pushMessage( destId, senderId, msg ); // always in the first delta of the list
}


/*
 * Return the number of remaining changes in non-local trackers
 */
sint	CMirrorService::getNbRemainingPropChanges() const
{
	sint sum = 0;
	TNDataSetsMS::const_iterator ids;
	for ( ids=MSInstance->NDataSets.begin(); ids!=MSInstance->NDataSets.end(); ++ids )
	{
		const CDataSetMS& dataset = GET_NDATASET(ids);

		// Browse non-local property trackers
		TPropertyIndex propIndex;
		for ( propIndex=0; propIndex!= dataset.nbProperties(); ++propIndex )
		{
			// TODO: Skip properties not declared
			TSubscriberListForProp::const_iterator isl;
			for ( isl=dataset._SubscribersByProperty[propIndex].begin(); isl!=dataset._SubscribersByProperty[propIndex].end(); ++isl )
			{
				const CChangeTrackerMS& tracker = (*isl);
				if ( ! tracker.isLocal() )
				{
					sum += tracker.nbChanges();
				}
			}
		}
	}
	return sum;
}


/*
 * Display the list of remote MS
 */
void	CMirrorService::displayRemoteMSList( NLMISC::CLog& log )
{
	TServiceIdList::const_iterator isl;
	for ( isl=_RemoteMSList.begin(); isl!=_RemoteMSList.end(); ++isl )
	{
		string clientServices;
		for ( uint i=0; i!=256; ++i )
		{
			if ( _RemoteMSList.getCorrespondingMS( TServiceId8(i) ) == (*isl) )
				clientServices += servStr(TServiceId8(i)) + string(" ");
		}
		TDeltaToMSList& deltaToMSList = _RemoteMSList.getDeltaToMSList( *isl );
		log.displayNL( "MS %hu: %u delta buffers; client services: %s, MTR %s tag: %d", 
			isl->get(), 
			deltaToMSList.size(), 
			clientServices.c_str(), 
			hasTagOfServiceChanged( *isl ) ? "changed" : "steady", 
			getTagOfService( *isl ).rawTag() );
		TDeltaToMSList::const_iterator idml;
		for ( idml=deltaToMSList.begin(); idml!=deltaToMSList.end(); ++idml )
		{
			(*idml)->display( log );
		}
	}
}


/*
 * Display the list of client services
 */
void	CMirrorService::displayClientServices( NLMISC::CLog& log )
{
	TClientServices::const_iterator ics;
	for ( ics=_ClientServices.begin(); ics!=_ClientServices.end(); ++ics )
	{
		const TClientServiceInfo& cs = GET_CLIENT_SERVICE_INFO(ics);
		log.displayNL( "Service %s: %u msgs, %u owned ranges, tag: %d", CUnifiedNetwork::getInstance()->getServiceUnifiedName( (*ics).first ).c_str(), cs.Messages.size(), cs.OwnedRanges.size(), getTagOfService( (*ics).first ).rawTag() );
	}
}


void cbDeclareDataSets( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->declareDataSets( msgin, serviceId );
}

void cbAllocateProperty( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->allocateProperty( msgin, serviceId );
}

void cbGiveOtherProperties( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->giveOtherProperties( msgin, serviceId );
}

void cbDeclareEntityTypeOwner( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->declareEntityTypeOwner( msgin, serviceId );
}

void cbGiveRange( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->giveRange( msgin, serviceId );
}

/*void cbRecvDeclaredRange( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->receiveDeclaredRange();
}*/

void cbRecvDataSetEntitiesSubscription( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	string dataSetName;
	CMTRTag newTagOfRemoteMS;
	TServiceId clientServiceId;
	msgin.serial( dataSetName );
	msgin.serial( newTagOfRemoteMS );
	msgin.serial( clientServiceId );
	MSInstance->processDataSetEntitiesSubscription( dataSetName, serviceId, clientServiceId, newTagOfRemoteMS );
}

void cbRecvPropSubscription( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	// From another MS
	bool subscribeOrUnsubscribe;
	msgin.serial( subscribeOrUnsubscribe );
	if ( subscribeOrUnsubscribe )
	{
		string propName;
		msgin.serial( propName );
		// TODO: maybe, do not allocate yet if there is no local service interested in the property
		MSInstance->processPropSubscriptionByName( propName, serviceId, false, false, true, "" );
	}
	else
	{
		MSInstance->processPropUnsubscription( msgin, serviceId );
	}
}


void cbRecvSyncMS( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->receiveSyncFromRemoteMS( msgin, (TServiceId)serviceId );
}


void cbRecvSyncMi( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->receiveSyncMirrorInformation( msgin, serviceId );
}


void cbRecvServiceMirrorUp( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	if ( CUnifiedNetwork::getInstance()->isServiceLocal( serviceId ) )
		nldebug( "Local client service %s has mirror system ready", servStr(serviceId).c_str() );
}


void cbRecvAcknowledgeAddPropTrackerMS( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	TServiceId from;
	msgin.serial( from );
	MSInstance->receiveAcknowledgeAddPropTrackerMS( msgin, from );
}


/*
 * The message FWD_ACKATE is sent by a client service (A) when it receives the order to add an entity tracker
 * (for writing) to the service (B) for which the tracker is a self tracker (for reading):
 * (A) could have created some entities before receiving ATE. (B) wants to know them.
 * The MS converts FWD_ACKATE to ACKATE.
 * When receiving ACKATE, (B) fills the tracker with all its known entities.
 */
void cbForwardACKATE( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->decNbExpectedATAcknowledges( serviceId, 0 );

	TServiceId destServiceId;
	msgin.serial( destServiceId );
	if ( destServiceId == IService::getInstance()->getServiceId() ) // tracker to a remote MS
	{
		TServiceId from;
		msgin.serial( from );
		MSInstance->receiveAcknowledgeAddEntityTrackerMS( msgin, from, true );
	}
	else
	{
		MSInstance->forwardLocalACKATE( msgin, serviceId, destServiceId );
	}
}


/*
 * The message FWD_ACKATP is sent by a client service (A) when it receives the order to add a property tracker
 * (for writing) to the service (B) for which the tracker is a self tracker (for reading):
 * (A) could have modified some properties before receiving ATP. (B) wants to know them.
 * The MS converts FWD_ACKATP to ACKATP.
 * When receiving ACKATP, (B) fills the tracker with all its non-zero values.
 */
void cbForwardACKATP( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->decNbExpectedATAcknowledges( serviceId, 1 );

	TServiceId destServiceId;
	msgin.serial( destServiceId );
	if ( destServiceId == IService::getInstance()->getServiceId() ) // tracker to a remote MS
	{
		cbRecvAcknowledgeAddPropTrackerMS( msgin, "MS", IService::getInstance()->getServiceId() );
		nlinfo( "ROWMGT: Decoded ACKATP from %s", servStr(serviceId).c_str() );
	}
	else
	{		
		CMessage msgout( "ACKATP" );
		msgout.serialBuffer( const_cast<uint8*>(msgin.buffer() + msgin.getPos()), msgin.length() - msgin.getPos() );
		CUnifiedNetwork::getInstance()->send( destServiceId, msgout );
		nlinfo( "PROPMGT: Forwarded ACKATP from %s to %s", servStr(serviceId).c_str(), servStr(destServiceId).c_str() );
	}
}


void cbForwardACKATE_FCO( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->decNbExpectedATAcknowledges( serviceId, 0 );

	TServiceId destServiceId;
	msgin.serial( destServiceId );
	if ( destServiceId == IService::getInstance()->getServiceId() ) // tracker to a remote MS
	{
		TServiceId from;
		msgin.serial( from );
		MSInstance->receiveAcknowledgeAddEntityTrackerMS( msgin, from, false );
	}
}


void cbForwardACKATP_FCO( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	MSInstance->decNbExpectedATAcknowledges( serviceId, 1 );
}

void cbAddRemoveRemoteClientService( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	TServiceId clientServiceId;
	bool addOrRemove;
	CMTRTag tagOfNewClientService;
	string remoteMSVersion = "prior to 1.5";
	msgin.serial( clientServiceId );
	msgin.serial( addOrRemove );
	if ( addOrRemove )
	{
		try
		{
			msgin.serial( tagOfNewClientService );
			msgin.serial( remoteMSVersion );
		}
		catch (const EStreamOverflow&)
		{}
		if ( MirrorServiceVersion != remoteMSVersion )
			nlerror( "MS version mismatch! This MS: %s; Remote MS-%hu: %s", MirrorServiceVersion.c_str(), serviceId.get(), remoteMSVersion.c_str() ); 
	}
	MSInstance->addRemoveRemoteClientService( (TServiceId)clientServiceId, (TServiceId)serviceId, addOrRemove, tagOfNewClientService );
}


void cbReleaseEntitiesInRanges( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	TServiceId msId;
	msgin.serial( msId );
	nlinfo( "ROWMGT: Receiving request to release entities from down MS-%hu", msId.get() );
	uint16 nbDatasets;
	msgin.serial( nbDatasets );
	for ( uint i=0; i!=(uint)nbDatasets; ++i )
	{
		string datasetName;
		msgin.serial( datasetName );
		vector<TRowRange> erasedRanges;
		msgin.serialCont( erasedRanges );
		try
		{
			MSInstance->releaseEntitiesInRanges( MSInstance->NDataSets[datasetName], erasedRanges );
		}
		catch (const EMirror&)
		{
			nlwarning( "Dataset %s not found", datasetName.c_str() );
		}
	}
}


void cbTagChanged( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	CMTRTag tag;
	msgin.serial( tag );
	MSInstance->setNewTagOfRemoteMS( tag, (TServiceId)serviceId );
}


void cbRecvBindingCountersByMessage( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	string datasetName;
	msgin.serial( datasetName );
	try
	{
		MSInstance->NDataSets[datasetName].receiveAllBindingCounters( msgin );
	}
	catch (const EMirror&)
	{
		nlwarning( "Dataset %s not found", datasetName.c_str() );
	}
}


// For the SLV automatic test
void cbQuit( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	CMessage msgout( "ACK_QUIT" );
	CUnifiedNetwork::getInstance()->send( serviceId, msgout );

	IService::getInstance()->exit();
}


/*void cbForwardTransportClass1( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	// Experimental: forwarding without copy
//	msgin.invert();
//	msgin.changeType( "CT_MSG" );
//	msgin.invert();
//	msgin.seek( 1, NLMISC::IStream::end );
//	uint8 destServiceId;
//	msgin.serial( destServiceId );
//	nldebug( "Forwarding Transport Class to %s", servStr(destServiceId).c_str() );
//	msgin.invert();
//	CUnifiedNetwork::getInstance()->send( destServiceId, msgin );

	uint16 destServiceId;
	msgin.serial( destServiceId );
	CMessage msgout( "MCT_MSG" );
	msgout.serial( serviceId ); // sender
	msgout.serialBuffer( const_cast<uint8*>(msgin.buffer() + msgin.getPos()), msgin.length() - msgin.getPos() );
	CUnifiedNetwork::getInstance()->send( destServiceId, msgout );
	nldebug( "Forwarding TC to %s", servStr(destServiceId).c_str() );
}


void cbForwardTransportClass2( NLNET::CMessage& msgin, const std::string &serviceName, uint16 serviceId )
{
	// Experimental: forwarding without copy
//	msgin.invert();
//	msgin.changeType( "CT_MSG" );
//	msgin.invert();
//	msgin.seek( 1, NLMISC::IStream::end );
//	uint8 sns;
//	msgin.serial( sns );
//	msgin.seek( 4 + sns + 1, NLMISC::IStream::end );
//	string destServiceName;
//	msgin.serial( destServiceName );
//	msgin.seek( 0, NLMISC::IStream::end );
//	nldebug( "Forwarding Transport Class to %s", destServiceName.c_str() );
//	msgin.invert();
//	CUnifiedNetwork::getInstance()->send( destServiceName, msgin );

	string destServiceName;
	msgin.serial( destServiceName );
	CMessage msgout( "MCT_MSG" );
	msgout.serial( serviceId ); // sender
	msgout.serialBuffer( const_cast<uint8*>(msgin.buffer() + msgin.getPos()), msgin.length() - msgin.getPos() );
	CUnifiedNetwork::getInstance()->send( destServiceName, msgout );
	nldebug( "Forwarding TC to %s", destServiceName.c_str() );

	// TODO:
	// - optimize (e.g. with no copy)
	// - allow to forward by MS only if spawning, otherwise send directly
	// - check if it works if this msg callback is processed before calling onTick() (would not ensure the expected ordering)
}*/


extern void cbTock( CMessage& msgin, const string& serviceName, TServiceId serviceId );
extern void cbMessageToForward( CMessage& msgin, const string& serviceName, TServiceId serviceId );
extern void cbRecvDelta( CMessage& msgin, const string& serviceName, TServiceId serviceId );


// Callback Array
TUnifiedCallbackItem MirrorCallbackArray[] =
{
	{ "DATASETS", cbDeclareDataSets },
	{ "AP", cbAllocateProperty },
	{ "DET", cbDeclareEntityTypeOwner },
	{ "RG", cbGiveRange },
	//{ "DRG", cbRecvDeclaredRange },
	{ "SDSE", cbRecvDataSetEntitiesSubscription },
	{ "SPROP", cbRecvPropSubscription },
	{ "SYNC_MS", cbRecvSyncMS },
	{ "SYNCMI", cbRecvSyncMi },
	{ "SMIRU", cbRecvServiceMirrorUp },
	{ "FWD_ACKATE", cbForwardACKATE },
	{ "FWD_ACKATP", cbForwardACKATP },
	{ "FWD_ACKATE_FCO", cbForwardACKATE_FCO },
	{ "FWD_ACKATP_FCO", cbForwardACKATP_FCO },
	{ "TOCK", cbTock },
	{ "FWDMSG", cbMessageToForward },
	{ "DELTA", cbRecvDelta },
	{ "MARCS", cbAddRemoveRemoteClientService },
	{ "REIR", cbReleaseEntitiesInRanges },
	{ "TAG_CHG", cbTagChanged },
	{ "BIDG_CNTR", cbRecvBindingCountersByMessage },
	{ "GOP", cbGiveOtherProperties },
	{ "QUIT", cbQuit }
};




// Service instanciation
NLNET_SERVICE_MAIN (CMirrorService, "MS", "mirror_service", 0, MirrorCallbackArray, "", "");



/*
 * displayMSTrackers command
 */
NLMISC_COMMAND( displayMSTrackers, "Display the trackers for one of all dataset(s)", "[<dataset>]" )
{
	if ( args.size() > 0 )
	{
		try
		{
			MSInstance->NDataSets[args[0]].displayTrackers();
		}
		catch (const EMirror&)
		{
			log.displayNL( "Dataset not found" );
		}
	}
	else
	{
		TNDataSetsMS::const_iterator ids;
		for ( ids=MSInstance->NDataSets.begin(); ids!=MSInstance->NDataSets.end(); ++ids )
		{
			GET_NDATASET(ids).displayTrackers( log );
		}
	}
	return true;
}


NLMISC_COMMAND( changeWeightOfProperty, "Change the weight of a property", "<propName> <weight>" )
{
	if ( args.size() == 2 )
	{
		sint32 weight;
		NLMISC::fromString(args[1], weight);
		MSInstance->changeWeightOfProperty(args[0], weight);
		return true;
	}
	else return false;
}


NLMISC_COMMAND( displayWeights, "Display the weights of the properties", "" )
{
	TNDataSetsMS::const_iterator ids;
	for ( ids=MSInstance->NDataSets.begin(); ids!=MSInstance->NDataSets.end(); ++ids )
		GET_NDATASET(ids).displayWeights( log );
	return true;
}


NLMISC_COMMAND( displayRemoteMS, "Display the list of remote MS", "" )
{
	MSInstance->displayRemoteMSList( log );
	return true;
}


NLMISC_COMMAND( displayClientServices, "Display the list of client services", "" )
{
	MSInstance->displayClientServices( log );
	return true;
}

#ifdef COUNT_MIRROR_PROP_CHANGES

NLMISC_DYNVARIABLE( sint32, TotalNbChangesPerTickRowMgt, "Number of distinct changes per tick for non-local row management" )
{
	// We can only read the value
	if ( get )
	{
		sint32 sum = 0;
		TNDataSetsMS::const_iterator ids;
		for ( ids=MSInstance->NDataSets.begin(); ids!=MSInstance->NDataSets.end(); ++ids )
			sum += GET_NDATASET(ids).getNbChangesForRowManagement();
		*pointer = sum;
	}
}

NLMISC_DYNVARIABLE( sint32, TotalNbChangesPerTickProps, "Number of distinct changes per tick for non-local prop changes" )
{
	// We can only read the value
	if ( get )
	{
		sint32 sum = 0;
		TNDataSetsMS::const_iterator ids;
		for ( ids=MSInstance->NDataSets.begin(); ids!=MSInstance->NDataSets.end(); ++ids )
			sum += GET_NDATASET(ids).getNbChangesForPropChanges();
		*pointer = sum;
	}
}

#endif

NLMISC_DYNVARIABLE( sint32, TotalNbChangesSentPerTickRowMgt, "Number of changes emitted to other MS for row management" )
{
	// We can only read the value
	if ( get )
	{
		*pointer = MSInstance->getNbChangesSentPerTick( true );
	}
}

NLMISC_DYNVARIABLE( sint32, TotalNbChangesSentPerTickProps, "Number of changes emitted to other MS for props" )
{
	// We can only read the value
	if ( get )
	{
		*pointer = MSInstance->getNbChangesSentPerTick( false );
	}
}


NLMISC_DYNVARIABLE( sint32, TotalRemainingChangesProps, "Number of remaining prop changes not sent to other MS" )
{
	// We can only read the value
	if ( get )
	{
		*pointer = MSInstance->getNbRemainingPropChanges();
	}
}


NLMISC_COMMAND( verboseMessagesSent, "Turn on/off the display of msgs sent to other MSes", "1/0" )
{
	if ( args.size() < 1 )
		return false;
	VerboseMessagesSent = args[0] == "1";
	log.displayNL( "verboseMessagesSent is %s", VerboseMessagesSent?"ON":"OFF" );
	return true;
}


NLMISC_DYNVARIABLE( string, Status, "Mirrors online status" )
{
	if ( get )
	{
		if ( MSInstance->mirrorsOnline() )
			*pointer = NLMISC::toString( "Ready (%d machines online)", MSInstance->nbOfOnlineMS() );
		else if ( ! MSInstance->rangeManagerReady() )
			*pointer = NLMISC::toString( "Expecting %s", RANGE_MANAGER_SERVICE.c_str() );
		else
			*pointer = NLMISC::toString( "Waiting for MS (%d detected)", MSInstance->nbOfOnlineMS() ); // not possible anymore
	}
}


static string MainNbEntities = "?";

// This command must be here to prevent to compile the same one in mirror.cpp (which would crash)
NLMISC_VARIABLE( string, MainNbEntities, "Not available on MS" );

NLMISC_DYNVARIABLE( float, EmittedKBPerSec, "Output rate for mirror deltas & messages" )
{
	if ( get )
		*pointer = MSInstance->getEmittedKBytesPerSecond();
}

NLMISC_DYNVARIABLE( sint8, MainMTRTag, "Main MTR Tag" )
{
	if ( get )
		*pointer = MSInstance->mainTag().rawTag();
}

NLMISC_DYNVARIABLE( bool, IsPureReceiver, "IsPureReceiver" )
{
	if ( get )
		*pointer = MSInstance->isPureReceiver();
}

NLMISC_DYNVARIABLE( string, MirrorServiceVersion, "Version of MS" )
{
	if ( get )
		*pointer = MirrorServiceVersion;
}
