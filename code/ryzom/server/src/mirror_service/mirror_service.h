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




#ifndef MIRROR_SERVICE_H
#define MIRROR_SERVICE_H

#include <nel/net/service.h>
#include <nel/misc/shared_memory.h>

//#include "game_share/tick_event_handler.h" //for debug info
#include "game_share/property_allocator.h"
#include "game_share/mirrored_data_set_types.h"

#include "data_set_ms.h"
#include "tick_proxy.h"

#include <vector>

// Disable this define to reduce the traffic between mirror services of property changes
#define DISABLE_PROPERTY_CHANGE_TRAFFIC_LIMITING


/*
 * Property info (mirror service version)
 */
struct TPropertyInfoMS : public TPropertyInfoBase< CDataSetMS >
{
	/// Constructor
	TPropertyInfoMS( CDataSetMS *ds=NULL, TPropertyIndex propindex=INVALID_PROPERTY_INDEX ) : TPropertyInfoBase<CDataSetMS>(ds, propindex) {}

	void			reaccess( sint32 shmid );
};


class TPropertiesInMirrorMS : public TMapOfPropertyInfoBase < TPropertyInfoMS, CDataSetMS >
{

};


/*
 * TDataSetsMS map with keys as string (name)
 */
class TNDataSetsMS : public CHashMap< std::string, CDataSetMS* >
{
public:

	/// Operator [] which does not create an object when the key is not found, but throws EMirror()
	CDataSetMS& operator [] ( const std::string& key )
	{
		iterator ids = find( key );
		if ( ids != end() )
		{
			return GET_NDATASET(ids);
		}
		else
		{
			throw EMirror();
			return *InvalidDataSet;
		}
	}

	static CDataSetMS	*InvalidDataSet;
};


struct TMessageCarrierWithDestId : public TMessageCarrier
{
	TMessageCarrierWithDestId( bool inputStream ) : TMessageCarrier( inputStream ) {}

	NLNET::TServiceId		DestId;
};


/*
 * TSDataSetsMS map with keys as CSheetId
 */
typedef std::map<NLMISC::CSheetId, CDataSetMS> TSDataSetsMS;


/**
 * Delta buffer in which the mirror service pushes the changes to send to a remove mirror service.
 */
class CDeltaToMS
{
public:

	enum TRowManagementTypeMask { RowManagementBindingCountersBit=1, RowManagementAddingBit=2, RowManagementRemovingBit=4, RowManagementPropChangeBit=8, RowManagementSyncBit=16 /*extension of AddingBit (set at the same time)*/ };

	/// Constructor
	CDeltaToMS( CDataSetMS *dataset ) :
		_DataSet(dataset), _DeltaBuffer( false, false, 0 ), _Header(0), _MustTransmitAllBindingCounters(false),
		_NbChangesPushed(0), _NbChangesPushedInTickRowMgt(0), _NbChangesPushedInTickProps(0),
		_StoredNbChangesPushedRowMgt(0), _StoredNbChangesPushedProps(0),
		_NbMessagesStored(0), _NbMessagesBufPos(0), _DatasetHeaderBufPos(0) {}

	/// Set the initial capacity
	void			setCapacity( uint32 initialByteCapacity )
	{
		_DeltaBuffer.resize( initialByteCapacity );
	}

	/// Return the pointer to the dataset
	CDataSetMS		*dataSet() const { return _DataSet; }

	/// Return the buffer
	NLMISC::CMemStream&		getDeltaBuffer() { return _DeltaBuffer; }

	/**
	 * Clear the buffer
	 * Uses resetBufPos() to avoid reallocating.
	 * TODO: we could decrease the allocated size if it is big because of a punctual peak (e.g. beginning)
	 */
	void			clearBuffer() { _DeltaBuffer.resetBufPos(); _Header = 0; _NbMessagesStored = 0; }

	/// Stores the stored number of changes pushed with the sums of the tick, and reset the sums
	void			storeNbChangesPushed()
	{
		_StoredNbChangesPushedRowMgt = _NbChangesPushedInTickRowMgt;
		_NbChangesPushedInTickRowMgt = 0;
		_StoredNbChangesPushedProps = _NbChangesPushedInTickProps;
		_NbChangesPushedInTickProps = 0;
	}

	/// Return true if the buffer is full, according to the specified property change budget
#ifdef DISABLE_PROPERTY_CHANGE_TRAFFIC_LIMITING
	bool			full( TPropertyIndex ) const { return false; }
#else
	bool			full( TPropertyIndex propIndex ) const { return _NbChangesPushed >= _DataSet->getChangesBudget( propIndex ); }
#endif

	/// Return the header byte
	const uint8&	header() const { return _Header; }

	/// Return true if the delta buffer has additions or removals or properties (false if empty)
	bool			hasMirrorDelta() const { return _Header != 0; }

	/// Return true if the delta buffer has some removals of entities
	bool			hasEntitiesRemoved() const { return (_Header & RowManagementRemovingBit) != 0; }

	/// Return true if the delta buffer has some new entities
	bool			hasEntitiesAdded() const { return (_Header & RowManagementAddingBit) != 0; }

	/// Return true if the delta buffer has some properties changed
	bool			hasProperties() const { return (_Header & RowManagementPropChangeBit) != 0; }

	/// Return the number of changes since the last beginRowManagement() or beginPropChanges()
	sint			getNbChangesPushed() const { return _NbChangesPushed; }

	/// Return the stored number of changes pushed for row management, since the last clearBuffer()
	sint			getStoredNbChangesPushedRowMgt() const { return _StoredNbChangesPushedRowMgt; }

	/// Return the stored number of changes pushed for props, since the last clearBuffer()
	sint			getStoredNbChangesPushedProps() const { return _StoredNbChangesPushedProps; }

	/// Set the flag to transmit the binding counters (see beginTransmitAllBindingCounters()/endTransmitAllBindingCounters())
	void			setMustTransmitAllBindingCounters() { _MustTransmitAllBindingCounters = true; }

	/// Return true if the binding counters must be sent
	bool			mustTransmitAllBindingCounters() const { return _MustTransmitAllBindingCounters; }

	/// Begin pushing mirror delta (after any message block)
	void			beginMirrorDelta()
	{
		/// Update the message counter in the delta buffer (messages are pushed in CRemoteMSList::storeMessageForRemoteClientService())
		endMessages();

		// Reserve sheetId and header (assumes sheetId.asInt() is uint32)
		_DatasetHeaderBufPos = _DeltaBuffer.reserve( sizeof(uint32) + sizeof(uint8) );
		_Header = 0x0;
	}

	/// End pushing mirror delta (after row management & prop management)
	void			endMirrorDelta()
	{
		// Fill the sheet id of the dataset and the header byte
		if ( _DatasetHeaderBufPos == 0 )
		{
			uint32 sheet = _DataSet->sheetId().asInt();
			_DeltaBuffer.serial( sheet );
			_DeltaBuffer.serial( _Header );
		}
		else
		{
			_DeltaBuffer.poke( _DataSet->sheetId().asInt(), _DatasetHeaderBufPos );
			_DeltaBuffer.poke( _Header, _DatasetHeaderBufPos + sizeof(uint32) );
		}
		//nldebug( "Header is %hu", header() );
		_DatasetHeaderBufPos = 0;

		// Close property management
		if ( hasProperties() )
		{
			// Push INVALID_PROPERTY_INDEX to the end of the delta buffer (to close it)
			TPropertyIndex endofchanges = INVALID_PROPERTY_INDEX;
			_DeltaBuffer.fastWrite( endofchanges );
			//MIRROR_DEBUG( "MIRROR: Pushing END_OF_CHANGES for delta to service %hu", *isl );
		}
	}

	/// Begin a push cycle for binding counters
	void			beginTransmitAllBindingCounters()
	{
		if ( ! hasMirrorDelta() ) // first delta block?
			beginMirrorDelta();
		_Header |= RowManagementBindingCountersBit;
	}

	/// End a push cycle for binding counters. One begin/end must correspond to one call of setMustTransmitAllBindingCounter().
	void			endTransmitAllBindingCounters()
	{
		_MustTransmitAllBindingCounters = false;
		// No need of a marker because always maxNbRows items are transmitted
	}

	/// Do not push any binding counter, but unset "must transmit all binding counters"
	void			cancelTransmitAllBindingCounters()
	{
		_MustTransmitAllBindingCounters = false;
	}

	/// Begin a push cycle for row management. Returns false if no push is needed.
	bool			beginRowManagement( const CChangeTrackerMS& tracker, TEntityTrackerIndex eti )
	{
		if ( tracker.getFirstChanged() != LAST_CHANGED )
		{
			if ( ! hasMirrorDelta() ) // first delta block?
				beginMirrorDelta();

			//nldebug( "%u: Begin row mgt pushing %u", CTickProxy::getGameCycle(), eti );
			_NbChangesPushed = 0;
			if ( eti == ADDING )
				_Header |= RowManagementAddingBit;
			else // ADDING
				_Header |= RowManagementRemovingBit;
			return true;
		}
		else
		{
			_StoredNbChangesPushedRowMgt = 0;
			return false;
		}
	}

	/// Push one row management event (precondition: beginRowManagement() returned true)
	void			pushRowManagement( TEntityTrackerIndex eti, const TDataSetRow& datasetRow );

	/// End the current push cycle for row management (precondition: beginRowManagement() returned true)
	void			endRowManagement( TEntityTrackerIndex eti )
	{
		TDataSetIndex theEnd = INVALID_DATASET_INDEX;
		_DeltaBuffer.fastWrite( theEnd );
		_NbChangesPushedInTickRowMgt += _NbChangesPushed;
	}

	/// Begin a push cycle for row management sync (whole datasetrows, with assignment counter)
	void			beginRowManagementSync( const std::vector<NLNET::TServiceId8>& remoteClientServices/*, const TClientServices& clientServices*/ )
	{
		if ( ! hasMirrorDelta() ) // first delta block?
			beginMirrorDelta();

		// Push the list of remote client services to notify
//		_DeltaBuffer.serialCont( const_cast<std::vector<TServiceId8>&>(remoteClientServices) );
		nlWrite(_DeltaBuffer, serialCont, remoteClientServices);

		/*// Push the list of local client services
		uint16 nbClients = clientServices.size();
		_DeltaBuffer.serial( nbClients );
		for ( TClientServices::const_iterator ics=clientServices.begin(); ics!=clientServices.end(); ++ics )
		{
			uint16 clientServiceId = (*ics).first;
			_DeltaBuffer.serial( clientServiceId );
		}*/

		_NbChangesPushed = 0;
		_Header |= RowManagementSyncBit;	
	}

	/// Push one row management sync entity.
	void			pushRowManagementSync( const TDataSetRow& datasetRow );

	/// End the current push cycle for row management sync
	void			endRowManagementSync()
	{
		TDataSetRow theEnd;
		_DeltaBuffer.fastWrite( theEnd );
		_NbChangesPushedInTickRowMgt += _NbChangesPushed;
	}

	/// Begin a push cycle for property changes. Returns false if no push is needed.
	bool			beginPropChanges( const CChangeTrackerMS& tracker, TPropertyIndex propIndex )
	{
		if ( tracker.getFirstChanged() != LAST_CHANGED )
		{
			if ( ! hasMirrorDelta() ) // first delta block?
				beginMirrorDelta();

			_NbChangesPushed = 0;
			_Header |= RowManagementPropChangeBit;
			//nldebug( "%u: Pushing property %hd", CTickProxy::getGameCycle(), propIndex );
			_DeltaBuffer.fastWrite( propIndex );
			return true;
		}
		else
		{
			return false;
		}
	}

	/// Push one property change event (precondition: beginPropChanges() returned true)
	template <class T>
	void			pushPropChange( TPropertyIndex propIndex, TDataSetRow& datasetrow, T * )
	{
		//nldebug( "Pushing prop change for E%d, prop %hd", entityIndex, propIndex );

		sint32 oldPos = _DeltaBuffer.getPos();
		try
		{
			// Write row
			_DeltaBuffer.fastWrite( datasetrow );
		
			// List or single value
			if ( _DataSet->propIsList( propIndex ) )
			{
				// Write local timestamp (not the header, it may be different among Mirror Services)
				TSharedListRow header;
				NLMISC::TGameCycle timestamp;
				_DataSet->getHeaderOfList( datasetrow, propIndex, header, timestamp, (T*)NULL );
				_DeltaBuffer.fastWrite( timestamp );

				// Write all the items!
				CMirrorPropValueListMS<T> theList( const_cast<CDataSetMS&>(*_DataSet), datasetrow, propIndex );
				typename CMirrorPropValueListMS<T>::size_type size = (TSharedListRow)theList.size();
				if ( size == ~0 ) // TEMP workaround
				{
					nlwarning( "ERROR: List size freeze workaround" );
					size = 0;
				}

				TSharedListRow sizeSlr = (TSharedListRow)size;
				_DeltaBuffer.fastWrite( sizeSlr );
				if ( size != 0 )
				{
					typename CMirrorPropValueListMS<T>::iterator it;
					for ( it=theList.begin(); it!=theList.end(); ++it )
					{
						_DeltaBuffer.fastWrite( const_cast<T&>((*it)()) );
					}
				}
			}
			else
			{
				// Write value and local timestamp
				T value;
				NLMISC::TGameCycle timestamp;
				_DataSet->getValue( datasetrow, propIndex, value, timestamp );
				_DeltaBuffer.fastWrite( timestamp );
				_DeltaBuffer.fastWrite( value );
			}
			++_NbChangesPushed;
		}
		catch (const NLMISC::EReallocationFailed&)
		{
			nlwarning( "ERROR: Can't reallocate DeltaBuffer (E%u propIndex %hd %s NbChangesPushed %d oldpos %d bufpos %d bufsize %u StoredNbChangesPushedProps %d NbChangesPushedInTickProps %d StoredNbChangesPushedRowMgt %d NbChangesPushedInTickRowMgt %d)",
				datasetrow.getIndex(), propIndex, _DataSet->propIsList(propIndex) ? "list" : "normal",
				_NbChangesPushed, oldPos, _DeltaBuffer.getPos(), _DeltaBuffer.size(),
				_StoredNbChangesPushedProps, _NbChangesPushedInTickProps,
				_StoredNbChangesPushedRowMgt, _NbChangesPushedInTickRowMgt );

			// Roll-back prop change
			_DeltaBuffer.resize( oldPos );
			_DeltaBuffer.seek( 0, NLMISC::IStream::end );
		}

		//nldebug( "Prop changes pushed: %d, size %u", _NbChangesPushed, _DeltaBuffer.length() );
	}

	/// End the current push cycle for property changes (precondition: beginPropChanges() returned true)
	void			endPropChanges( TPropertyIndex propIndex )
	{
		TDataSetRow theEnd;
		_DeltaBuffer.fastWrite( theEnd );
		_NbChangesPushedInTickProps += _NbChangesPushed;
	}

	// Store a message
	/*void			storeMessage( TServiceId destId, TServiceId senderId, NLMISC::CMemStream& msg )
	{
		TMessageCarrierWithDestId mc;
		_Messages.push_back( mc );
		_Messages.back().Msg = msg;
		_Messages.back().SenderId = senderId;
		_Messages.back().DestId = destId;
	}

	/// Push the messages previously stored to delta buffer
	void			pushStoredMessages()
	{
		std::vector<TMessageCarrierWithDestId>::iterator im;
		uint32 nbMsgs = _Messages.size();
		_DeltaBuffer.serial( nbMsgs );
		for ( im=_Messages.begin(); im!=_Messages.end() ++ im )
		{
			_DeltaBuffer.serial( (*im).DestId );
			_DeltaBuffer.serial( (*im).SenderId );
			_DeltaBuffer.serialMemStream( (*im).Msg );
		}
	}*/

	/// Store (and push directly) a message
	void			pushMessage( NLNET::TServiceId destId, NLNET::TServiceId senderId, NLMISC::CMemStream& msg )
	{
		if ( _NbMessagesStored == 0 )
		{
			_NbMessagesBufPos = _DeltaBuffer.reserve( sizeof(uint32) ); // for poke in endMessages()
		}
		_DeltaBuffer.serial( destId, senderId );
		_DeltaBuffer.serialMemStream( msg );
		++_NbMessagesStored;
	}

	/// Update the message counter in the delta buffer (can be called anytime before sending the delta buffer)
	void			endMessages()
	{
		if ( _NbMessagesStored == 0 )
		{
			_DeltaBuffer.serial( _NbMessagesStored );
		}
		else
		{
			_DeltaBuffer.poke( _NbMessagesStored, _NbMessagesBufPos );
			//nldebug( "MSG: Delta buffer contains %u messages", _NbMessagesStored );
		}
	}

	/// Debug display
	void			display( NLMISC::CLog& log = *NLMISC::InfoLog ) const
	{
		log.displayNL( "  Delta buffer for dataset %s: Capacity %u Size %d Header 0x%x NbChangesPushed %d -InTickRowMgt %d -InTickProps %d -StoredRowMgt %d -StoredProps %d NbMessages %u",
			_DataSet->name().c_str(), _DeltaBuffer.size(), _DeltaBuffer.getPos(), _Header, _NbChangesPushed,
			_NbChangesPushedInTickRowMgt, _NbChangesPushedInTickProps, 
			_StoredNbChangesPushedRowMgt, _StoredNbChangesPushedProps,
			_NbMessagesStored );
	}

private:

	///
	CDataSetMS			*_DataSet;

	///
	NLMISC::CMemStream	_DeltaBuffer;

	/// Number of changes since the last beginRowManagement() or beginPropChanges()
	sint				_NbChangesPushed;

	/// Sum of number of changes for row management in the tick
	sint				_NbChangesPushedInTickRowMgt;

	/// Sum of number of changes for props in the tick
	sint				_NbChangesPushedInTickProps;

	/// Stored number of changes pushed for row management in the tick
	sint				_StoredNbChangesPushedRowMgt;

	/// Stored number of changes pushed for props in the tick
	sint				_StoredNbChangesPushedProps;

	/// Number of messages stored in the delta buffer
	uint32				_NbMessagesStored;

	/// Position in the delta buffer of the room for the number of messages (valid only if _NbMessagesStored > 0)
	sint32				_NbMessagesBufPos;

	/// Position in the delta buffer of the room for dataset sheetid and header (valid only between beginMirrorDelta() and endMirrorDelta())
	sint32				_DatasetHeaderBufPos;

	/// Header byte
	uint8				_Header;

	/// Flag to transmit all binding counters
	bool				_MustTransmitAllBindingCounters;
};


/*
 *
 */
inline void			CDeltaToMS::pushRowManagement( TEntityTrackerIndex eti, const TDataSetRow& datasetRow )
{
	// Transmit the whole dataset row information
	_DeltaBuffer.fastWrite( datasetRow );
	if ( eti == ADDING )
	{
		//nldebug( "Pushing E%d addition", entityIndex );
		_DeltaBuffer.fastWrite( const_cast<NLMISC::CEntityId&>(_DataSet->getEntityId( datasetRow )) );
		NLNET::TServiceId8 spawnerId = _DataSet->getSpawnerServiceId( datasetRow.getIndex() );
		_DeltaBuffer.fastWrite( spawnerId );
	}

	++_NbChangesPushed;
}


/*
 *
 */
inline void			CDeltaToMS::pushRowManagementSync( const TDataSetRow& datasetRow )
{
	_DeltaBuffer.fastWrite( datasetRow );
	//nldebug( "Pushing E%d addition", entityIndex );
	_DeltaBuffer.fastWrite( const_cast<NLMISC::CEntityId&>(_DataSet->getEntityId( datasetRow )) );
	NLNET::TServiceId8 spawnerId = _DataSet->getSpawnerServiceId( datasetRow.getIndex() );
	_DeltaBuffer.fastWrite( spawnerId );
	++_NbChangesPushed;
}


// The vector stores pointers to avoid big reallocations and to allow linking with pointers
typedef std::vector< CDeltaToMS* > TDeltaToMSList;

typedef std::list< NLNET::TServiceId8 > TServiceIdList;


/**
 * List of remote mirror services and their delta buffers (several by service)
 */
class CRemoteMSList
{
public:

	/// Constructor
	CRemoteMSList()
	{
		for ( uint i=0; i!=256; ++i )
		{
			_CorrespondingMS[i].set(0);
		}
	}

	/// Add a remote mirror service
	void			addRemoteMS( TSDataSetsMS& datasets, NLNET::TServiceId serviceId )
	{
		_ServiceIdList.push_back( serviceId );

		TSDataSetsMS::iterator ids;
		for ( ids=datasets.begin(); ids!=datasets.end(); ++ids )
		{
			_DeltaToMSArray[serviceId.get()].push_back( new CDeltaToMS( &(GET_SDATASET(ids)) ) );
			_DeltaToMSArray[serviceId.get()].back()->setCapacity( std::min((int)GET_SDATASET(ids).maxOutBandwidth() + 16, (int)1024*1024) ); // after push_back to avoid a big copy
		}
	}

	/// Remove a remote mirror service
	void			removeRemoteMS( NLNET::TServiceId serviceId )
	{
		TServiceIdList::iterator isl = find( _ServiceIdList.begin(), _ServiceIdList.end(), serviceId );
		if ( isl != _ServiceIdList.end() )
			_ServiceIdList.erase( isl );

		for ( TDeltaToMSList::iterator idl=_DeltaToMSArray[serviceId.get()].begin(); idl!=_DeltaToMSArray[serviceId.get()].end(); ++idl )
			delete (*idl);
		_DeltaToMSArray[serviceId.get()].clear();

		// Reset mappings to MS
		for ( uint i=0; i!=256; ++i )
		{
			if ( _CorrespondingMS[i] == serviceId )
			{
				removeRemoteClientService( NLNET::TServiceId(i) );
			}
		}
	}

	/// Map service -> MS (supports multiple adding if the same)
	void			addRemoteClientService( NLNET::TServiceId8 clientServiceId, NLNET::TServiceId msId )
	{
		nlassert( (_CorrespondingMS[clientServiceId.get()].get() == 0) || (_CorrespondingMS[clientServiceId.get()] == msId) );
		_CorrespondingMS[clientServiceId.get()] = msId;
	}

	/// Unmap service -> MS
	void			removeRemoteClientService( NLNET::TServiceId8 clientServiceId )
	{
		_CorrespondingMS[clientServiceId.get()].set(0);
	}

	/// Get a mapping service -> MS. Return 0 if not found
	NLNET::TServiceId8		getCorrespondingMS( NLNET::TServiceId8 remoteClientService ) const 
	{ 
		return _CorrespondingMS[remoteClientService.get()]; 
	}

	// Random access to a serviceId, get the list of datasets
	TDeltaToMSList&	getDeltaToMSList( NLNET::TServiceId serviceId )
	{
		return _DeltaToMSArray[serviceId.get()];
	}

	// Random access to a serviceId, then find a dataset
	CDeltaToMS		*getDeltaToMS( NLNET::TServiceId serviceId, CDataSetMS *dataset )
	{
		TDeltaToMSList& deltaList = _DeltaToMSArray[serviceId.get()];

		// Find dataset
		TDeltaToMSList::iterator idl;
		for ( idl=deltaList.begin(); idl!=deltaList.end(); ++idl )
		{
			if ( (*idl)->dataSet() == dataset )
			{
				return (*idl);
			}
		}
		return NULL;
	}

	/// Return the stored number of changes emitted to other MS for row management or props
	sint						getNbChangesSentPerTick( bool rowmgt ) const;
	
	// Access the list of remote mirror services
	TServiceIdList::iterator	begin()
	{
		return _ServiceIdList.begin();
	}

	// Access the list of remote mirror services
	TServiceIdList::iterator	end()
	{
		return _ServiceIdList.end();
	}

	/// Store the message for later sending within delta packet to the corresponding mirror service
	void						pushMessageToRemoteQueue( NLNET::TServiceId destId, NLNET::TServiceId senderId, NLMISC::CMemStream& msgin );

private:

	/// Helper for pushMessageToRemoteQueue
	void						doPushMessageToDelta( NLNET::TServiceId msId, NLNET::TServiceId destId, NLNET::TServiceId senderId, NLMISC::CMemStream& msg );

	/// List of remote mirror services
	TServiceIdList				_ServiceIdList;

	/// Vector of deltas indexed by MS serviceids (TODO: optimize size, there are only a few MS in the shard)
	TDeltaToMSList				_DeltaToMSArray [256];

	/// Vector of MS serviceids indexed by their client services ids
	NLNET::TServiceId8			_CorrespondingMS [256];
};


/**
 * CMirrorService
 *
 * A Mirror Service centralizes the mirrored data on one physical machine.
 * The various services on this machine that have subscribed to these data
 * can access them using shared memory. The various services on other
 * machines receive that have subscribed to these data receive them by
 * network (socket) communication.
 *
 * The Mirror Service is responsible for allocating the shared memory,
 * and handling shared memory identifiers (regarding to properties).
 *
 * It sends to remote mirror services the changes and their values
 * and timestamps.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CMirrorService : public NLNET::IService
{
public:

	/// Initialization
	virtual void	init();

	/// Release
	virtual void	release();

	/// Constructor
	CMirrorService() :
		_NumberOfOnlineMS(1),
		_RangeManagerReady(false), _MirrorsOnline(false), _DeltaSent(false), _BlockedAwaitingATAck(false),
		_EmittedKBytesPerSecond(0.0f), _EmittedBytesPartialSum(0), _TimeOfLatestEmittedBytesAvg(0),
		_NbDeltaUpdatesReceived(0), _NbExpectedDeltaUpdates(0), _IsPureReceiver(false)
	{
		for ( uint i=0; i!=256; ++i )
		{
			_AllServiceTags[i][0].reset(); // necessary, for a new MS connected to be considered
			_AllServiceTags[i][1].reset(); // as "wider tag changed"
		}
	}

	/// Declare the datasets used by a client service
	void			declareDataSets( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );

	/// Allocate a property segment with shared memory and return the smid to the sender
	void			allocateProperty( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );

	/// Give the list of properties not subscribed but allocated on this machine for owned datasets
	void			giveOtherProperties( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );

	/// Unallocate (destroy) all the allocated segments
	void			destroyPropertySegments();

	/// Declare/undeclare an entity type
	void			declareEntityTypeOwner( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );

	/// Give a range (message from the range manager)
	void			giveRange( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );

	/// Process a subscription of dataset entities management, received from another MS
	void			processDataSetEntitiesSubscription( const std::string& dataSetName, NLNET::TServiceId msId, NLNET::TServiceId clientServiceId, const CMTRTag& newTagOfRemoteMS );

	/// Process a subscription of property (possibly received from another MS) by name
	void			processPropSubscriptionByName( const std::string& propName, NLNET::TServiceId serviceId, bool local, bool readOnly, bool allocate, const std::string& notifyGroupByPropName, bool writeOnly=false );

	/// Process a subscription of property (possibly received from another MS)
	void			processPropSubscription( CDataSetMS& dataset, TPropertyIndex propIndex, const std::string& propName, NLNET::TServiceId serviceId, bool local, bool readOnly, bool allocate, const std::string& notifyGroupByPropName, bool writeOnly=false );

	/// Process an unsubscription of property from another MS (corresponding to one client service leaving)
	void			processPropUnsubscription( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );

	/// Process a delta
	void			processReceivedDelta( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );

	/// Browse all datasets and build deltas to remote MS, then send them
	void			buildAndSendAllDeltas();

	/// Apply received deltas
	void			applyPendingDeltas();

	/// Send UMM to local client services
	void			tellLocalServicesAndSendMessages();

	/// Do the unsubscription and tracker removal job after all entity removals about a quitting service have been applied
	void			applyServicesQuitting();

	/// Add a MS
	void			addRemoteMS( NLNET::TServiceId serviceId );

	/// Remove a MS
	void			removeRemoteMS( NLNET::TServiceId serviceId );

	/// Add or remove a remote client service
	void			addRemoveRemoteClientService( NLNET::TServiceId clientServiceId, NLNET::TServiceId msId, bool addOrRemove, const CMTRTag& tagOfNewClientService );

	/// Remove all trackers of the specified local client service, and tell all services that have them to do the same
	void			removeTrackers( NLNET::TServiceId serviceId );

	///
	void			releaseRangesOfService( NLNET::TServiceId serviceId );

	/// Tell all other MS one service that had subscribed to some properties is leaving
	void			broadcastUnsubscribeProperties( NLNET::TServiceId serviceId );

	/// Change a weight
	void			changeWeightOfProperty( const std::string& propName, sint32 weight );

	/// Return the stored number of changes emitted to other MS for row management or props
	sint			getNbChangesSentPerTick( bool rowmgt ) const { return _RemoteMSList.getNbChangesSentPerTick( rowmgt ); }

	/// Return the number of remaining changes in non-local trackers
	sint			getNbRemainingPropChanges() const;

	/// Display the list of remote MS
	void			displayRemoteMSList( NLMISC::CLog& log = *NLMISC::InfoLog );

	/// Display the list of client services
	void			displayClientServices( NLMISC::CLog& log );

	/// Tell if the range manager service is up or down
	void			setRangeManagerReady( bool b );

	/// If all the expected MS and the range manager service are ready, send a message to the specified service if it is a local client
	void			testAndSendMirrorsOnline( NLNET::TServiceId servId );

	/// Wait before doing rescan for service
	void			deferRescanOfEntitiesForNewService( NLNET::TServiceId serviceId );

	/// Wait one tick before applying the row removals from the quitting service, before releasing the trackers and unsubscribing
	void			deferTrackerRemovalForQuittingService( NLNET::TServiceId servId );

	/// Do the rescans if it's time to do so
	void			applyPendingRescans();

	/// Do the rescan
	void			rescanEntitiesForNewService( NLNET::TServiceId clientServiceId );

	/// Receive SYNC_MS
	void			receiveSyncFromRemoteMS( NLNET::CMessage& msgin, NLNET::TServiceId srcRemoteMSId );

	/// Remove all entities in the specified ranges (received from the range manager when a MS is down)
	void			releaseEntitiesInRanges( CDataSetMS& dataset, const std::vector<TRowRange>& erasedRanges );
	
	/// Receive SYNCMI
	void			receiveSyncMirrorInformation( NLNET::CMessage& msgin, NLNET::TServiceId serviceId );

	/// Scan for existing entities in the received ranges
	void			receiveAcknowledgeAddEntityTrackerMS( NLNET::CMessage& msgin, NLNET::TServiceId serviceIdFrom, bool isTrackerAdded );

	/// Scan for non-null property values, written by the specified service
	void			receiveAcknowledgeAddPropTrackerMS( NLNET::CMessage& msgin, NLNET::TServiceId serviceIdFrom );

	/// Forward ACKATE from a local service to a local service, if their tags allow it
	void			forwardLocalACKATE( NLNET::CMessage& msgin, NLNET::TServiceId srcServiceId, NLNET::TServiceId destServiceId );

	void			tickClientServices();
	
	void			receiveMessageToForwardFromClient( NLNET::CMessage& msgin, NLNET::TServiceId senderId );
	void			receiveMessageToForwardMultipleFromClient( NLNET::CMessage& msgin, NLNET::TServiceId senderId );

	void			pushMessageToLocalQueue( std::vector<TMessageCarrier>& msgQueue, NLNET::TServiceId senderId, NLMISC::CMemStream& msg );
	void			pushMessageToLocalQueueFromMessage( std::vector<TMessageCarrier>& msgQueue, NLNET::CMessage& msgin );

	void			receiveDeltaFromRemoteMS( NLNET::CMessage& msgin, NLNET::TServiceId senderMSId );

	void			serialToMessageFromLocalQueue( NLNET::CMessage& msgout, const TMessageCarrier& srcMsgInQueue );

	void			testIfNotInQuittingServices( NLNET::TServiceId servId );

	/// Return true if the delta of the current tick has been sent
	bool			deltaUpdateSent() const { return _DeltaSent; }

	/// Return true if all delta updates have been received for the current tick
	bool			checkIfAllIncomingDeltasReceived() const;

	/// Reset the delta sent flag (when a new service is added)
	void			resetDeltaSent() { _DeltaSent = false; }

	/// Reset the number of delta updates received
	void			resetDeltaUpdatesReceived() { _NbDeltaUpdatesReceived = 0; }
	
	/// Advance state in the mirror automaton
	void			doNextTask();

	///
	void			displayAutomatonState( NLMISC::CLog *log );

	///
	void			buildAndSendAllOutgoingDeltas();
	
	///
	void			tockMasterAndProcessReceivedIncomingDeltas();
	
	///
	void			masterTick();

	/**
	 * Increment _NbDeltaUpdatesReceived for the number of times we have received a VALID delta update before sync
	 * (valid = its tick = the current tick + 1)
	 */
	void			examineDeltaUpdatesReceivedBeforeSync();

	///
	void			setNewTagOfRemoteMS( const CMTRTag& tag, NLNET::TServiceId msId );

	///
	void			decNbExpectedATAcknowledges( NLNET::TServiceId clientServiceId, uint whichEorP );

	///
	sint			getTotalNbExpectedATAcknowledges() const;

	/// Return the client services info (read-only)
	const TClientServices&		clientServices() { return _ClientServices; }

	//void			receiveDeclaredRange( NLNET::CMessage& msgin );

	/// Return the tag of a service
	const CMTRTag&				getTagOfService( NLNET::TServiceId serviceId )
	{
		return _AllServiceTags[serviceId.get()][0];
	}

	///
	const CMTRTag&				getPreviousTagOfService( NLNET::TServiceId serviceId )
	{
		return _AllServiceTags[serviceId.get()][1];
	}

	///
//	const CMTRTag&				getPreviousTagOfService( uint16 serviceId ) { return getPreviousTagOfService( (TServiceId)serviceId ); }

	///
//	const CMTRTag&				getTagOfService( uint16 serviceId ) { return getTagOfService( (TServiceId)serviceId ); }

	/// Return true if the tag changed during the last dataset entity subscription from the specified MS
	bool						hasTagOfServiceChanged( NLNET::TServiceId serviceId )
	{
		return (_AllServiceTags[serviceId.get()][0].rawTag() != _AllServiceTags[serviceId.get()][1].rawTag());
	}

	bool			rangeManagerReady() const { return _RangeManagerReady; }
	bool			mirrorsOnline() const { return _MirrorsOnline; }
	sint			nbOfOnlineMS() const { return (sint)_NumberOfOnlineMS; }
	float			getEmittedKBytesPerSecond() const { return _EmittedKBytesPerSecond; }
	CMTRTag&		mainTag() { return _AllServiceTags[getServiceId().get()][0]; }
	bool			isPureReceiver() const { return _IsPureReceiver; }
	
	/// The datasets
	TNDataSetsMS	NDataSets;

protected:

	/// Process an entity management subscription request from a client service, to forward it to all other mirror services
	void			broadcastSubscribeDataSetEntities( const std::string& dataSetName, NLNET::TServiceId clientServiceId );

	/// Process a property subscription request from a client service, to forward it to all other mirror services
	void			broadcastSubscribeProperty( NLNET::CMessage& msgin, const std::string& propName, NLNET::TServiceId serviceId );

	/// Allocate entity tracker
	void			allocateEntityTrackers( CDataSetMS& dataset, NLNET::TServiceId serviceId, NLNET::TServiceId clientServiceId, bool local, const CMTRTag& tagOfNewPeer );

	/// Delete a tracker from its vector
	void			deleteTracker( CChangeTrackerMS& tracker, std::vector<CChangeTrackerMS>& vect );

	/// Send RT to local services
	void			sendTrackersToRemoveToLocalServices( std::map< NLNET::TServiceId, std::vector<sint32> >& servicesForTrackerRemoval );

	/// Push the property changes on the local machine for a remote mirror service to its delta buffer
	template <class T>
	void			pushPropertyChanges( CDataSetMS& dataset, CChangeTrackerMS& tracker, TPropertyIndex propIndex, T *typ );
	
	/// Apply the property changes coming from a remote mirror service
	template <class T>
	void			applyPropertyChanges( NLNET::CMessage& msgin, CDataSetMS& dataset, TPropertyIndex propIndex, NLNET::TServiceId serviceId, T * )
	{
		//sint NbChangesRead;
		//NbChangesRead = 0;
		TDataSetRow datasetRow;
		msgin.fastRead( datasetRow );
		while ( datasetRow.isValid() )
		{
			// Read timestamp
			NLMISC::TGameCycle timestamp;
			msgin.fastRead( timestamp );
			
			// Check remCounter
			bool apply = true;
			if ( dataset.dataSetRowIsTooOld( datasetRow ) )
			{
				  	//nlwarning( "Discarding value set at %u (%u ticks ago) for old E%hu (new counter=%hu, transmitted=%hu), P%hd", 
					//timestamp, CTickProxy::getGameCycle()-timestamp, 
					//entityIndex.getIndex(), 
					//(uint16)dataset.getBindingCounter( entityIndex.getIndex() ), (uint16)entityIndex.counter(), propIndex );
				apply = false;
			}

			//++NbChangesRead;
			//MIRROR_DEBUG( "MIRROR: Reading propchange for %d", entityIndex );
			if ( dataset._PropIsList[propIndex] )
			{
				// Read values
				TSharedListRow size;
				msgin.fastRead( size );
				slist<T> tempList;
				for ( TSharedListRow i=0; i!=size; ++i )
				{
					T value;
					msgin.fastRead( value ); // the values are stored in reverse order
					tempList.push_front( value );
				}
				if ( apply )
					dataset.applyListPropChange( datasetRow.getIndex(), propIndex, serviceId, tempList, timestamp );
			}
			else
			{
				// Read value and timestamp
				T value;
				msgin.fastRead( value );
				if ( apply )
					dataset.applyPropChange( datasetRow.getIndex(), propIndex, serviceId, value, timestamp );
			}

			msgin.fastRead( datasetRow );
		}
		//nldebug( "Number of changes read for P%hd: %d", propIndex, NbChangesRead );
	}

	/// 
	void			setupDestEntityTrackersInterestedByDelta( CDataSetMS& dataset, const CMTRTag& sourceTag, TEntityTrackerIndex addRem, const std::vector<NLNET::TServiceId8>& listOfExcludedServices );

	///
	void			setupDestEntityTrackersInterestedBySyncDelta( CDataSetMS& dataset, const CMTRTag& sourceTag, std::vector<NLNET::TServiceId8> listOfInterestedServices );

	/// 
	void			setupDestPropTrackersInterestedByDelta( CDataSetMS& dataset, const CMTRTag& sourceTag, NLNET::TServiceId sourceMSId, TPropertyIndex propIndex );

	///
	void			applyQuittingOf( NLNET::TServiceId clientServiceId );

	///
	void			computeNewMainTag();
	
public:
	/// Send the delta buffers to the remote mirror services
	void			sendAllDeltas();
protected:

	/// If all the expected MS and the range manager service are ready, send a message to all the local clients
	void			testAndSendMirrorsOnline();

	/// Send a synchronize message to a new remote MS
	void			synchronizeSubscriptionsToNewMS( NLNET::TServiceId newRemoteMSId );

	/// Compute output rate stats if needed (delta + messages)
	void			computeDeltaStats();

	/// Return true if there are no Add Tracker acknowledges that we have to wait
	bool			checkIfAllATAcknowledgesReceived()
	{
		if ( getTotalNbExpectedATAcknowledges() == 0 )
			return true;
		else
		{
			_BlockedAwaitingATAck = true;
			return false;
		}
	}

	/// Set the new tag
	void						setNewTag( NLNET::TServiceId serviceId, const CMTRTag& newTag )
	{
		_AllServiceTags[serviceId.get()][1] = _AllServiceTags[serviceId.get()][0];
		_AllServiceTags[serviceId.get()][0] = newTag;
	}

	///
//	void						setNewTag( uint16 serviceId, const CMTRTag& newTag ) { setNewTag( (TServiceId)serviceId, newTag ); }

	/// See doc in .cpp
	void			syncOnlineLocallyManagedEntitiesIntoDeltaAndOpenEntityTrackersToRemoteMS( CDeltaToMS *delta, CDataSetMS& dataset, CChangeTrackerMS& entityTrackerAdding, uint entityTrackerIndexInVector );

private:

	typedef std::pair<NLMISC::TGameCycle,NLNET::TServiceId> TServiceAndTimeLeft;

	/// Properties in the local mirror
	TPropertiesInMirrorMS			_PropertiesInMirror;

	/// Shared memory id pool, to generate new ids
	CSMIdPool						_SMIdPool;

	/// Mutex id pool (different from _SMIdPool because a mutex creation may fail if it was not properly released (for instance after a crash))
	CSMIdPool						_MutIdPool;

	/// List of all remote mirror services
	CRemoteMSList					_RemoteMSList;

	/// List of all local client services
	TClientServices					_ClientServices;

	/// List of quitting services, to defer the tracker removals and unsubscription
	std::list<TServiceAndTimeLeft>	_QuittingServices;

	/// List of client services that have their rescan pending
	std::list<TServiceAndTimeLeft>	_ServicesWaitingForRescan;

	/// Map of datasets with keys as CSheetId
	TSDataSetsMS					_SDataSets;

	/// Delta buffers received but not applied yet
	std::vector< std::pair<NLNET::TServiceId,NLNET::CMessage> > _PendingDeltas;

	/// Messages to broadcast to local services at next cycle
	std::vector<TMessageCarrier>	_MessagesToBroadcastLocally;

	/// Tags of MS, remote client services, local client services. 0: New tag (since the latest SDSE or SYNC_MS); 1: Previous tag
	CMTRTag							_AllServiceTags [256] [2];

	/// Output rate statistics
	float							_EmittedKBytesPerSecond;

	/// Output rate statistics helper
	uint32							_EmittedBytesPartialSum;

	/// Output rate statistics helper
	NLMISC::TTime					_TimeOfLatestEmittedBytesAvg;

	// This value indicates if we have received a delta update from all of the other services (we check it before sending master tock)
	sint16							_NbDeltaUpdatesReceived;

	//
	sint16							_NbExpectedDeltaUpdates;

	// Only MS the delta updates of which we must wait for, and that have already sent delta updates
	std::list<NLNET::TServiceId>				_RemoteMSToWaitForDelta;

	// If we receive a delta update before sync, store the gamecycle (it will be used to increment or not _NbDeltaUpdatesReceived)
	std::vector<NLMISC::TGameCycle> _GameCyclesOfDeltaReceivedBeforeSync;

	/// Number of FWD_ACKATE, FWD_ACKATP awaited, per client service. Can always use ++_NbExpectedATAcknowledges[] directly on it, as POD types are automatically initialized to 0 according to STL standard
	std::map<NLNET::TServiceId,sint>			_NbExpectedATAcknowledges;

	/// Current number of mirror services online in the shard
	sint16							_NumberOfOnlineMS;

	/// Status of range manager service
	bool							_RangeManagerReady;

	/// Main status of mirrors online
	bool							_MirrorsOnline;

	/// True if the delta of the current tick has been sent (or if there is no client service)
	bool							_DeltaSent;

	/// True if the automaton is blocked while all Add Tracker acks are not received
	bool							_BlockedAwaitingATAck;

	/// This MS is a pure receiver if all client services are pure receivers
	bool							_IsPureReceiver;
};


extern CMirrorService *MSInstance;

#endif

