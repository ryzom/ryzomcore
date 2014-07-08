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



#ifndef NL_DATA_SET_MS_H
#define NL_DATA_SET_MS_H

#include "nel/misc/types_nl.h"
#include "game_share/data_set_base.h"
#include "game_share/change_tracker_base.h"
#include "game_share/mirrored_data_set_types.h"
#include "tick_proxy.h"
#include <nel/net/service.h>

//#include <slist>
#define slist std::list

#undef DISPLAY_DELTAS

class CDataSetMS;


/*
 *
 */
struct TMessageCarrier
{
	TMessageCarrier( bool inputStream ) : Msg( inputStream ) {}

	NLMISC::CMemStream		Msg;
	NLNET::TServiceId		SenderId;
};


/*
 *
 */
struct TBindingCountersToApply
{
	TDataSetIndex				RangeFirst;
	std::vector<uint8>			Counters;
	std::vector<TDataSetIndex>	EntitiesToRemove;
};


/*
 *
 */
struct TClientServiceInfoOwnedRange
{
	TClientServiceInfoOwnedRange( CDataSetMS *dataset=NULL ) : DataSet(dataset), First(INVALID_DATASET_INDEX), Last(INVALID_DATASET_INDEX) {}

	CDataSetMS*		DataSet;

	TDataSetIndex	First;
	TDataSetIndex	Last;

	///
	void			fillFromEntityRange( const TEntityRange& src )
	{
		First = src.first();
		Last = src.last();
	}

	///
	/*void			fillToDeclaredEntityRange( TDeclaredEntityRange decl )
	{
		decl._First = First;
		decl._Size = Last - First + 1;
	}*/
};


/**
 * For each local client service, store the datasets in which the service owns a range.
 * This is necessary to release the range when the service shuts down.
 */
struct TClientServiceInfo
{
	/// Queued messages
	std::vector<TMessageCarrier>				Messages;

	/// Ranges owned by the client service
	std::vector<TClientServiceInfoOwnedRange>	OwnedRanges;
};


typedef std::map<NLNET::TServiceId, TClientServiceInfo> TClientServices;
#define GET_CLIENT_SERVICE_INFO(ics) ((*ics).second)


/*
 * Return the service id string with the format "MS/128"
 */
std::string servStr( NLNET::TServiceId serviceId );


const sint MaxNbSMIds = 4096;
const sint FirstSMId = 1;


/**
 *
 */
class CMTRTag
{
public:

	/// Constructor
	CMTRTag( TMTRTag rawTag = IniTag) : _RawTag(rawTag) {}
	
	/// Reset
	void			reset() { _RawTag = IniTag; }

	/// Update the main tag.
	void			addTag( TMTRTag tag )
	{
		// IniTag + X    -> X
		// All    + X    -> All
		// TAG    + TAG  -> TAG
		// TAG    + All  -> All
		if ( _RawTag == IniTag )
			_RawTag = tag;
		else if ( _RawTag != tag )
			_RawTag = AllTag;
	}

	/// Return true if the destination service(s) wants to receive changes from services with the specified tag
	bool			doesAcceptTag( const CMTRTag& tag ) const
	{
		return (_RawTag == AllTag) || ((_RawTag>AllTag) && (_RawTag!=tag._RawTag)); // assumes IniTag<AllTag=0
		// Note: IniTag does accept any tag
	}

	/// Serial
	void			serial( NLMISC::IStream& s ) { s.serial( _RawTag ); }

	/// Assignment operator
	CMTRTag&		operator= ( const CMTRTag& other ) { _RawTag = other._RawTag; return *this; }

	/// Return the raw tag
	TMTRTag			rawTag() const { return _RawTag; }

	friend bool		operator== ( const CMTRTag& tag1, CMTRTag tag2 );
	
private:

	TMTRTag			_RawTag;
};


/// Comparison
inline bool	operator== ( const CMTRTag& tag1, CMTRTag tag2 ) { return tag1._RawTag == tag2._RawTag; }


/**
 * CSMIdPool
 */
class CSMIdPool
{
public:

	/// Constructor
	CSMIdPool()
	{
		sint i;
		for ( i=0; i<MaxNbSMIds; ++i )
		{
			_UsedPool[i] = false;
		}
	}

	/// Get a free Id
	sint	getNewId()
	{
		sint i;
		for ( i=FirstSMId; i<MaxNbSMIds; ++i )
		{
			if ( ! _UsedPool[i] )
			{
				_UsedPool[i] = true;
				return i;
			}
		}
		return InvalidSMId;
	}

	/// Get a particular Id
	bool	reacquireId( sint id )
	{
		if ( ! _UsedPool[id] )
		{
			_UsedPool[id] = true;
			return true;
		}
		else
		{
			nlwarning( "POOL: Cannot reacquire id %d", id );
			return false;
		}
	}

	/// Release an Id
	void		releaseId( sint id )
	{
		_UsedPool[id] = false;
	}

private:

	bool		_UsedPool [MaxNbSMIds];
};


class CDataSetMS;

template <class T>
class CMirrorPropValueListMS;


template <class T>
class CMirrorPropValueItemMS
{
public:

	/// Constructor
	CMirrorPropValueItemMS( CMirrorPropValueListMS<T> *parentlist, T *pt ) : _ParentList(parentlist), _Pt(pt) {}
	
	/// Get the value
	const T&					operator() () const;

	/// Set the value
	CMirrorPropValueItemMS&		operator= ( const T& srcValue );

protected:

	CMirrorPropValueListMS<T>	*_ParentList;
	T							*_Pt;
};


#if NL_ISO_SYNTAX
template <>
class CMirrorPropValueItemMS<NLMISC::CEntityId>
{
public:

	/// Constructor for NLMISC::CEntityId
	CMirrorPropValueItemMS( CMirrorPropValueListMS<NLMISC::CEntityId> *parentlist, uint64 *pt ) : _ParentList(parentlist), _Pt(pt) {}

	/// Get the value
	const NLMISC::CEntityId&					operator() () const;

	/// Set the value
	CMirrorPropValueItemMS&		operator= ( const NLMISC::CEntityId& srcValue );

protected:

	CMirrorPropValueListMS<NLMISC::CEntityId>	*_ParentList;
	uint64		                                *_Pt;
};
#endif


template <class T>
struct _CMirrorPropValueListIteratorMS
{
	CMirrorPropValueItemMS<T>			operator*() const												{ return CMirrorPropValueItemMS<T>( _ParentList, &(_ParentList->_Container[_Index].Value) ); }
	_CMirrorPropValueListIteratorMS&	operator++()													{ _Index = _ParentList->_Container[_Index].Next; return *this; }
	_CMirrorPropValueListIteratorMS		operator++(int)													{ _CMirrorPropValueListIteratorMS tmp = *this;	_Index = _ParentList->_Container[_Index].Next; return tmp; }
	bool								operator==( const _CMirrorPropValueListIteratorMS& other ) const	{ return (_Index == other._Index) && (_ParentList == other._ParentList); }
	bool								operator!=( const _CMirrorPropValueListIteratorMS& other ) const	{ return (_Index != other._Index) || (_ParentList != other._ParentList); }
	_CMirrorPropValueListIteratorMS&	operator=( const _CMirrorPropValueListIteratorMS& other )			{ _ParentList = other._ParentList; _Index = other._Index; return *this; }

	_CMirrorPropValueListIteratorMS()	: _ParentList(NULL), _Index(INVALID_SHAREDLIST_ROW) {}
	CMirrorPropValueListMS<T>			*_ParentList;
	TSharedListRow						_Index;
};


/**
 * MS version of the shard list.
 * Unlike the normal (client service) version, this one never notifies the changes to a tracker.
 * This is done by the method that receives the data from another mirror service.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
template <class T>
class CMirrorPropValueListMS
{
public:

	typedef uint32									size_type;
	typedef _CMirrorPropValueListIteratorMS<T>		iterator;

#ifdef NL_OS_WINDOWS
	friend											iterator; // MSVC
#else
	template <class U> friend						class _CMirrorPropValueListIteratorMS; // GCC3
#endif


	/// Constructor
	CMirrorPropValueListMS( CDataSetMS& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow constructor
	//CMirrorPropValueListMS( CDataSetMS& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	bool						empty() const;
	void						clear();
	CMirrorPropValueItemMS<T>	front();
	const T&					front() const;
	void						push_front( const T& value );
	void						pop_front();
	size_type					size();
	iterator					begin();
	iterator					end();

#pragma pack(push, PACK_SHAREDLIST_CELL_MS, 1)
	struct TSharedListCell
	{
		// If changing this struct, don't forget to change the places where it is initialized
		TSharedListRow				Next;
		T							Value;
#ifdef NL_COMP_VC
	};
#else
	} __attribute__((packed)); 
#endif
#pragma pack(pop, PACK_SHAREDLIST_CELL_MS)

protected:

	/// Default constructor
	CMirrorPropValueListMS() : _PtFront(NULL), _Container(NULL) {}

	TSharedListRow		allocateNewCell();

	void				releaseCell( TSharedListRow row );

	/// Pointer to the list container in shared memory
	TSharedListCell		*_Container;

	/// Pointer to the 'first' index in shared memory
	TSharedListRow		*_PtFront;
};

#if NL_ISO_SYNTAX
/**
 * MS version of the shard list - specialization for CEntityId (non-POD types can't be packed from gcc 3.4)
 * Unlike the normal (client service) version, this one never notifies the changes to a tracker.
 * This is done by the method that receives the data from another mirror service.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002-2005
 */
template <>
class CMirrorPropValueListMS<class NLMISC::CEntityId>
{
public:

	typedef uint32									size_type;
	typedef _CMirrorPropValueListIteratorMS<NLMISC::CEntityId>		iterator;

#ifdef NL_OS_WINDOWS
	friend											iterator; // MSVC
#else
	template <class U> friend						class _CMirrorPropValueListIteratorMS; // GCC3
#endif


	/// Constructor
	CMirrorPropValueListMS( CDataSetMS& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow constructor
	//CMirrorPropValueListMS( CDataSetMS& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	bool						empty() const;
	void						clear();
	CMirrorPropValueItemMS<NLMISC::CEntityId>	front();
	const NLMISC::CEntityId&					front() const;
void						push_front( const NLMISC::CEntityId& value );

        void						pop_front();
	size_type					size();
	iterator					begin();
	iterator					end();

#pragma pack(push, PACK_SHAREDLIST_CELL_MS, 1)
	struct TSharedListCell
	{
		// If changing this struct, don't forget to change the places where it is initialized
		TSharedListRow				Next;
	        uint64                                  Value;
#ifdef NL_COMP_VC
	};
#else
	} __attribute__((packed)); 
#endif
#pragma pack(pop, PACK_SHAREDLIST_CELL_MS)

protected:

	/// Default constructor
	CMirrorPropValueListMS() : _PtFront(NULL), _Container(NULL) {}

	TSharedListRow		allocateNewCell();

	void				releaseCell( TSharedListRow row );

	/// Pointer to the list container in shared memory
	TSharedListCell		*_Container;

	/// Pointer to the 'first' index in shared memory
	TSharedListRow		*_PtFront;
};
#endif


enum TGroupTrackerState { Normal, IsGroupTracker, PointingToGroupTracker };


/**
 *
 */
class CTrackerBlockingInfo
{
public:

	/// Constructor
	CTrackerBlockingInfo() : _WeHaveWriters(false), _TagGotWider(false) {}

	/// Push
	void					pushAckExpected( NLNET::TServiceId newRemoteClientService, const std::vector<NLNET::TServiceId>& entityTrackerPairsSentTo, bool tagGetsWider )
	{
		_RemoteClientServicesToSync.push_back( newRemoteClientService );
		_LocalClientsDueToSendATEAck.insert( _LocalClientsDueToSendATEAck.end(), entityTrackerPairsSentTo.begin(), entityTrackerPairsSentTo.end() );
		_TagGotWider |= tagGetsWider;
	}

	/// Pop (255 is used for "no serviceIdFrom")
	void					popAckExpected( NLNET::TServiceId serviceIdFrom, bool wasTrackerAdded )
	{
		std::list<NLNET::TServiceId8>::iterator it = getServiceDueToSendATEAck( serviceIdFrom );
		if ( it != _LocalClientsDueToSendATEAck.end() )
		{
			_LocalClientsDueToSendATEAck.erase( it );
		}
		_WeHaveWriters |= wasTrackerAdded; // we will sync if at least one tracker was added by one of our clients
	}

	/// Reset (but keep blocked)
	void					resetInfo()
	{
		_RemoteClientServicesToSync.clear();
		_LocalClientsDueToSendATEAck.clear();
		_TagGotWider = false;
		_WeHaveWriters = false;
	}

	/// Return true if we have writers
	bool					weHaveWriters() const { return _WeHaveWriters; }

	/// Return true if the tag changed into "more information requested"
	bool					hasRemoteTagGotWider() const { return _TagGotWider; }

	/// Return true if a sync should be done now (when building the delta buffer).
	bool					canSyncNow() const { return _LocalClientsDueToSendATEAck.empty(); }

	/// Return true if we are expecting an ATE acknowledge from the specified service
	bool					isExpectingATEAckFrom( NLNET::TServiceId serviceIdFrom ) { return (getServiceDueToSendATEAck( serviceIdFrom ) != _LocalClientsDueToSendATEAck.end()); }

	/// Return the list of remote client services to sync
	const std::vector<NLNET::TServiceId8>& remoteClientServicesToSync() const { return _RemoteClientServicesToSync; }

	/// Return the number of acknowledges still expected
	NLNET::TServiceId8::size_type		nbAcksExpected() const { return (NLNET::TServiceId8::size_type)_LocalClientsDueToSendATEAck.size(); }

protected:

	///
	std::list<NLNET::TServiceId8>::iterator getServiceDueToSendATEAck( NLNET::TServiceId serviceIdFrom )
	{
		for ( std::list<NLNET::TServiceId8>::iterator it=_LocalClientsDueToSendATEAck.begin(); it!=_LocalClientsDueToSendATEAck.end(); ++it )
		{
			if ( (*it) == serviceIdFrom )
			{
				return it;
			}
		}
		return _LocalClientsDueToSendATEAck.end();
	}

private:

	/// List of remote client services that need a sync delta
	std::vector<NLNET::TServiceId8>		_RemoteClientServicesToSync;

	/// List of local client service from which we're expecting an ATE acknowledge for this tracker
	std::list<NLNET::TServiceId8>		_LocalClientsDueToSendATEAck;

	/// True if the sync will be needed when _NbAcksExpectedBeforeUnblocking gets back to 0, false if only unblocking is needed
	bool					_WeHaveWriters;

	/// True if one of the tag changes correspond to an "increasing" requested information
	bool					_TagGotWider;
};


class CDeltaToMS;


/**
 * Change tracker specialized for the server part (Mirror Service) of the mirror system
 * (creating shared memory, and reading the changes to send them to another mirror service).
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CChangeTrackerMS : public CChangeTrackerBase
{
public:

#ifdef COUNT_MIRROR_PROP_CHANGES
	/// Constructor
	CChangeTrackerMS( NLNET::TServiceId destServiceId, bool local, bool readOnly=false, bool writeOnly=false, TGroupTrackerState state=Normal ) : CChangeTrackerBase(), _DestServiceId( destServiceId ), _Local(local), _GroupTrackerState(state), _NbInterestedServices(0), _StoredNbChanges(0), _BlockingInfo(NULL), _LinkToDeltaBuffer(NULL) { resetFlags( readOnly, writeOnly ); if ( ! local ) allowSendingToRemoteMS( false ); }
#else
	/// Constructor
	CChangeTrackerMS( NLNET::TServiceId destServiceId, bool local, bool readOnly=false, bool writeOnly=false, TGroupTrackerState state=Normal ) : CChangeTrackerBase(), _DestServiceId( destServiceId ), _Local(local), _GroupTrackerState(state), _NbInterestedServices(0), _BlockingInfo(NULL), _LinkToDeltaBuffer(NULL) { resetFlags( readOnly, writeOnly ); if ( ! local ) allowSendingToRemoteMS( false ); }
#endif

	CChangeTrackerMS() { _BlockingInfo = NULL; }

	/// Copy constructor
	CChangeTrackerMS( const CChangeTrackerMS& src )
	{
		operator= ( src );
	}

	/// Assignment operator (don't forget to update if adding class members)
	CChangeTrackerMS&		operator = ( const CChangeTrackerMS& src )
	{
		if ( &src == this )
			return *this;

		CChangeTrackerBase::operator=( src );
		_DestServiceId = src._DestServiceId;
		_Local = src._Local;
		_ReadOrWriteOnly = src._ReadOrWriteOnly;
		_GroupTrackerState = src._GroupTrackerState; // see also useGroupTracker()
		_NbInterestedServices = src._NbInterestedServices;
#ifdef COUNT_MIRROR_PROP_CHANGES
		_StoredNbChanges = src._StoredNbChanges;
#endif
		_BlockingInfo = src._BlockingInfo ? new CTrackerBlockingInfo( *src._BlockingInfo ) : NULL;
		_LinkToDeltaBuffer = src._LinkToDeltaBuffer;
		return *this;
	}

	/// Destructor
	~CChangeTrackerMS()
	{
		if ( _BlockingInfo )
		{
			delete _BlockingInfo;
			_BlockingInfo = NULL;
		}
	}

	/// Allocate the tracker header and item array (shared memory)
	bool					allocate( sint32 shmid, sint32 nbRowsToAllocate );

	/// Reaccess a previously allocated tracker
	void					reaccess( sint32 shmid );

	/// If param not null: instead of allocating, use another tracker's data. If null: set as group tracker
	void					useGroupTracker( const CChangeTrackerMS *groupTracker );

	/// Unallocate
	bool					destroy();

	/// Return the service id of the destination service (remote mirror service or local client service)
	NLNET::TServiceId		destServiceId() const { return _DestServiceId; }

	/// [Remote tracker only] Set the link to the delta buffer
	void					setLinkToDeltaBuffer( CDeltaToMS *delta ) { _LinkToDeltaBuffer = delta; }

	/// [Remote tracker only] Return the link to the delta buffer
	CDeltaToMS*				getLinkToDeltaBuffer() { return _LinkToDeltaBuffer; }

	/// [Remote tracker only] Allow/block sending to remote MS. Precondition: isLocal(); Postcondition: b == (_BlockingInfo != NULL)
	void					allowSendingToRemoteMS( bool b );

	/// [Remote tracker only] Can the tracker changes be sent to a remote MS?
	bool					isAllowedToSendToRemoteMS() const { return (_BlockingInfo == NULL); }

	/// [Remote ADDING entity tracker only] Set the counter of acks to receive before unblocking the tracker
	void					pushATEAcknowledgesExpectedBeforeUnblocking( NLNET::TServiceId remoteClientServiceId, const std::vector<NLNET::TServiceId>& entityTrackerPairsSentTo );

	/// [Blocked remote ADDING entity tracker only] Called when an ack is received
	void					popATEAcknowledgesExpected( NLNET::TServiceId serviceIdFrom, bool wasTrackerAdded );

	/// [ADDING entity tracker only] Return true if we are expecting an ATE acknowledge from the specified service
	bool					isExpectingATEAckFrom( NLNET::TServiceId serviceIdFrom ) const { return (! isLocal()) && _BlockingInfo && _BlockingInfo->isExpectingATEAckFrom( serviceIdFrom ); }

	/// [Blocked remote ADDING entity tracker only] Return the number of ATE acknowledges still expected, before unblocking and filling the tracker for sync to a 'tag-changed' remote MS
	uint					nbATEAcksExpectedBeforeUnblocking() const { nlassert( _BlockingInfo ); return _BlockingInfo->nbAcksExpected(); }

	/// [Blocked remote ADDING entity tracker only] Return true if all acks have been received and a sync is needed. If true, you should unblock the tracker.
	bool					needsSyncAfterAllATEAcknowledgesReceived();

	/// [Blocked remote ADDING entity tracker only] Return the list of remote client services to sync (valid when needsSyncAfterAllATEAcknowledgesReceived() returns true, until the tracker is unblocked)
	const std::vector<NLNET::TServiceId8>& remoteClientServicesToSync() { nlassert( _BlockingInfo ); return _BlockingInfo->remoteClientServicesToSync(); }

	/// Return true if the destination service is a local client service
	bool					isLocal() const { return _Local; }

	/// Return true if the destination service cannot write values for the concerned property
	bool					isReadOnly() const { return (_ReadOrWriteOnly & 0x1) != 0; }

	/// Return true if the destination service cannot read values for the concerned property
	bool					isWriteOnly() const { return (_ReadOrWriteOnly & 0x2) != 0; }

	/// Return true if the tracker is used by several properties for group notification
	bool					isGroupTracker() const { return _GroupTrackerState==IsGroupTracker; }

	/// Increment the number of interested services
	void					addInterestedService() { ++_NbInterestedServices; }

	/// Decrement the number of interested services
	void					remInterestedService() { --_NbInterestedServices; }

	/// Return the number of interested services (set only for property trackers)
	uint16					nbInterstedServices() const { return _NbInterestedServices; }

	/// Display debug info (1 line)
	void					displayTrackerInfo( const char *headerStr="", const char *footerStrNotAllocd="", NLMISC::CLog *log=NLMISC::InfoLog ) const;

	/// Reset RO/WO flags
	void					resetFlags( bool readOnly, bool writeOnly ) { _ReadOrWriteOnly = (((uint8)readOnly) | ((uint8)writeOnly << 1)); }

#ifdef COUNT_MIRROR_PROP_CHANGES
	/// Store the number of changes
	void					storeNbChanges() { _StoredNbChanges = _Header->NbDistinctChanges; }

	/// Return the stored number of changes
	sint32					getStoredNbChanges() const { return _StoredNbChanges; }
#endif

private:

	/// Service id of the destination service (local service or remote MS)
	NLNET::TServiceId		_DestServiceId;

	/// Local client service, or remote mirror service
	bool					_Local;

	/// Can the source or destination service write values for the concerned property
	uint8					_ReadOrWriteOnly;

	/// Is the tracker used by several properties for group notification, or using another group tracker
	TGroupTrackerState		_GroupTrackerState;

	/// Number of services aimed by the tracker (useful for a tracker to a remote MS )
	uint					_NbInterestedServices; // set only for property trackers

#ifdef COUNT_MIRROR_PROP_CHANGES
	/// Fixed value of the number of changes (stat)
	sint32					_StoredNbChanges;
#endif

	/// When this is NULL, the tracker is open (always NULL for local trackers). Additional info for ADDING entity trackers is stored when blocked.
	CTrackerBlockingInfo	*_BlockingInfo;

	/// Pointer to the delta buffer (always NULL for local trackers) (the delta does not move in memory, allocated by 'new')
	CDeltaToMS				*_LinkToDeltaBuffer;
};


// List of subscribers
typedef std::vector< CChangeTrackerMS > TSubscriberListForProp;
typedef std::vector< CChangeTrackerMS > TSubscriberListForEnt;
typedef std::vector< CChangeTrackerMS* > TDestTrackersForDelta; // must not be kept when subscriber lists changed (pointers invalidated by a possible reallocation)

// Indexed by TPropertyIndex
typedef std::vector< TSubscriberListForProp > TSubscriberLists;

// List of owned entity ranges, sorted (r1<r2 => r1.baseIndex()+r1.size() should be lower than r2.baseIndex())
typedef std::set<TDeclaredEntityRange> TEntityRangeList;

class TPropertiesInMirrorMS;


/**
 * Dataset specialized for the server part (Mirror Service) of the mirror system.
 *
 * The method applyPropChange() is the core function that processes a modification
 * coming from a remote mirror service.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CDataSetMS : public CDataSetBase
{
public:
	
	/// Constructor
	CDataSetMS() : _MaxOutBandwidth(1024*1024), _NbLocalWriterProps(0), _BindingCountsToSet(NULL) /*, NbOnlineEntities(0)*/ {}

	/// Initialize
	void							init( const NLMISC::CSheetId& sheetId, const TDataSetSheet& properties, TPropertiesInMirrorMS& propertyMap );

	/// Add a tracker for a property
	CChangeTrackerMS&				addPropTracker( TPropertyIndex propIndex, NLNET::TServiceId destId, bool local, bool readOnly, bool allocate, bool writeOnly=false );

	/// Add a tracker for adding/removing entities
	CChangeTrackerMS&				addEntityTracker( TEntityTrackerIndex addOrRemove, NLNET::TServiceId destId, bool local, sint32 smid );

	/// Return the list of property trackers
	const TSubscriberListForProp&	getPropTrackers( TPropertyIndex propIndex ) const
	{
		return _SubscribersByProperty[propIndex];
	}

	/// Return the list of entity (row management) trackers
	const TSubscriberListForEnt&	getEntityTrackers( TEntityTrackerIndex addOrRemove )
	{
		return _EntityTrackers[addOrRemove];
	}

	/// Find a specific property tracker if it exists
	CChangeTrackerMS				*findPropTracker( TPropertyIndex propIndex, NLNET::TServiceId destId )
	{
		TSubscriberListForProp& propTrackers = _SubscribersByProperty[propIndex];
		TSubscriberListForProp::iterator it;
		for ( it=propTrackers.begin(); it!=propTrackers.end(); ++it )
		{
			if ( (*it).destServiceId() == destId )
			{
				return &(*it);
			}
		}
		return NULL;
	}

	/// Find a specific entity (row management) tracker if it exists
	CChangeTrackerMS				*findPropTrackerBySmid( TPropertyIndex propIndex, sint32 smid )
	{
		TSubscriberListForProp& propTrackers = _SubscribersByProperty[propIndex];
		TSubscriberListForProp::iterator it;
		for ( it=propTrackers.begin(); it!=propTrackers.end(); ++it )
		{
			if ( (*it).smid() == smid )
			{
				return &(*it);
			}
		}
		return NULL;
	}

	/// Find a specific entity (row management) tracker if it exists
	CChangeTrackerMS				*findEntityTracker( TEntityTrackerIndex addOrRemove, NLNET::TServiceId destId )
	{
		TSubscriberListForEnt& entityTrackers = _EntityTrackers[addOrRemove];
		TSubscriberListForEnt::iterator it;
		for ( it=entityTrackers.begin(); it!=entityTrackers.end(); ++it )
		{
			if ( (*it).destServiceId() == destId )
			{
				return &(*it);
			}
		}
		return NULL;
	}

	/// Find a specific entity (row management) tracker if it exists
	CChangeTrackerMS				*findEntityTrackerBySmid( TEntityTrackerIndex addOrRemove, sint32 smid )
	{
		TSubscriberListForEnt& entityTrackers = _EntityTrackers[addOrRemove];
		TSubscriberListForEnt::iterator it;
		for ( it=entityTrackers.begin(); it!=entityTrackers.end(); ++it )
		{
			if ( (*it).smid() == smid )
			{
				return &(*it);
			}
		}
		return NULL;
	}

	/// Return the entity id corresponding to a row
	const NLMISC::CEntityId&		getEntityId( const TDataSetRow& entityIndex ) const;

	/** Get a valid datasetrow usable with the current entity at entity index, if the entity is online.
	 * If the entity is offline, the method returns an invalid datasetrow (offline means the row is not used;
	 * warning: after despawning an entity by a service A, and before notifying the entity removal in a
	 * service B, the entity will be reported as offline).
	 */
	TDataSetRow					getCurrentDataSetRow( TDataSetIndex entityIndex ) const
	{
		if ( entityIndex != INVALID_DATASET_INDEX )
		{
			TDataSetRow datasetrow( entityIndex, getBindingCounter( entityIndex ) );
			if ( _PropertyContainer.EntityIdArray.isOnline( entityIndex ) )
				return datasetrow;
		}
		return TDataSetRow();
	}

	/** Get a valid datasetrow usable with the current entity at entity index, be the entity online or not.
	 * warning: after despawning an entity by a service A, and before notifying the entity removal in a
	 * service B, the entity will be reported as offline).
	 */
	TDataSetRow					forceCurrentDataSetRow( TDataSetIndex entityIndex ) const
	{
		if ( entityIndex != INVALID_DATASET_INDEX )
			return TDataSetRow( entityIndex, getBindingCounter( entityIndex ) );
		return TDataSetRow();
	}



	/// Get a property value
	template <class T>
	void							getValue( const TDataSetRow& entityIndex, TPropertyIndex propIndex, T& returnedValue, NLMISC::TGameCycle& timestamp ) const
	{
		T *ptvalue;
		getPropPointer( &ptvalue, propIndex, entityIndex );
		returnedValue = *ptvalue;
		timestamp = getChangeTimestamp( propIndex, entityIndex );
	}
	
	/// Get a property value
	template <class T>
	void							getHeaderOfList( const TDataSetRow& entityIndex, TPropertyIndex propIndex, TSharedListRow& returnedValue, NLMISC::TGameCycle& timestamp, T *typeHint ) const
	{
		TSharedListRow *ptvalue;
		getPropPointerForList( &ptvalue, propIndex, entityIndex, typeHint );
		returnedValue = *ptvalue;
		timestamp = getChangeTimestamp( propIndex, entityIndex );
	}

	// Clear all the property values of the row
	void							clearValuesOfRow( TDataSetIndex entityIndex );

	/// Add an entity. Must be preceded by setupDestEntityTrackersInterestedByDelta()
	void							addEntityToDataSet( const TDataSetRow& datasetRow, const NLMISC::CEntityId& entityId, NLNET::TServiceId spawnerId )
	{
		//nldebug( "ROWMGT: Adding entity E%d from remote to local ADDING trackers", entityIndex );
		//++NbOnlineEntities;
		TDataSetIndex entityIndex = datasetRow.getIndex();

		// Init entry in shared memory
		_PropertyContainer.EntityIdArray.EntityIds[entityIndex] = entityId;
		_PropertyContainer.EntityIdArray.Counts[entityIndex] = datasetRow.counter();
	
		// Clear all the properties values of the row (otherwise a change could fail to setChange() if the value was the same, with MIRROR_ASSIGN_ONLY_IF_CHANGED defined, and even without some services may test if a value is non-zero)
		clearValuesOfRow( entityIndex );
		
		// Declare and push to local services' add lists
		_PropertyContainer.EntityIdArray.setOnline( entityIndex, true );
		_PropertyContainer.EntityIdArray.setSpawnerServiceId( entityIndex, spawnerId );
		for ( TDestTrackersForDelta::iterator it=_DestTrackersForDelta.begin(); it!=_DestTrackersForDelta.end(); ++it )
		{
			(*it)->recordChange( entityIndex );
		}
	}

	/// Add an entity with the whole datasetRow, including assignment counter
	void							syncEntityToDataSet( const TDataSetRow& datasetRow, const NLMISC::CEntityId& entityId, NLNET::TServiceId spawnerId )
	{
		//nldebug( "ROWMGT: Syncing entity E%d from remote to local ADDING trackers", entityIndex );
		//++NbOnlineEntities;
		TDataSetIndex entityIndex = datasetRow.getIndex();

		// Init entry in shared memory
		_PropertyContainer.EntityIdArray.EntityIds[entityIndex] = entityId;
		_PropertyContainer.EntityIdArray.Counts[entityIndex] = datasetRow.counter();
		
		// Clear all the properties values of the row (e.g. killList if it was not cleared in a previous session)
		//clearValuesOfRow( entityIndex );
		
		// Declare and push to the add lists of the local services' that are destination of the sync delta
		_PropertyContainer.EntityIdArray.setOnline( entityIndex, true );
		_PropertyContainer.EntityIdArray.setSpawnerServiceId( entityIndex, spawnerId );
		for ( TDestTrackersForDelta::iterator it=_DestTrackersForDelta.begin(); it!=_DestTrackersForDelta.end(); ++it )
		{
			(*it)->recordChange( entityIndex );
		}
	}

	/// Remove an entity. Must be preceded by setupDestEntityTrackersInterestedByDelta()
	void							removeEntityFromDataSet( const TDataSetRow& datasetRow )
	{
		//nldebug( "ROWMGT: Adding entity E%d from remote to local REMOVING trackers", entityIndex );
		//--NbOnlineEntities;
		TDataSetIndex entityIndex = datasetRow.getIndex();

#ifdef NL_DEBUG
		if ( ! _PropertyContainer.EntityIdArray.isOnline( entityIndex ) )
			nlwarning( "ROWMGT: Removing entity: row E%d already offline", entityIndex );
#endif
		// We don't reset the entry in shard memory now, it will be done just when reusing the row,
		// because the notification system for removed entities NEEDS the entityId in the array
		_PropertyContainer.EntityIdArray.setOnline( entityIndex, false );
		_PropertyContainer.EntityIdArray.Counts[entityIndex] = datasetRow.counter();

		// Clear the timestamp of all properties for the entity
		// to prevent a connecting service to receive values corresponding to no longer entity
		for ( uint propIndex=0; propIndex!=_PropertyContainer.PropertyValueArrays.size(); ++propIndex )
		{
			if ( _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps )
				_PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex] = 0;
		}

		// Push to local services' remove lists
		for ( TDestTrackersForDelta::iterator it=_DestTrackersForDelta.begin(); it!=_DestTrackersForDelta.end(); ++it )
		{
			(*it)->recordChange( entityIndex );
		}
	}

	/// Set a property value. Must be preceded by setupDestPropTrackersInterestedByDelta()
	template <class T>
	void							applyPropChange( TDataSetIndex entityIndex, TPropertyIndex propIndex, NLNET::TServiceId srcServiceId, const T& value, NLMISC::TGameCycle timestamp )
	{
		// Set if the received timestamp is newer than the stored timestamp
		NLMISC::TGameCycle& localTimestamp = _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex];
		//if ( timestamp > localTimestamp )
		{
			// Set value
			T *ptvalue;
			getPropPointer( &ptvalue, propIndex, TDataSetRow(entityIndex) );
			*ptvalue = value;

			// Set timestamp
			localTimestamp = timestamp;

#ifdef STORE_CHANGE_SERVICEIDS
			_PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex] = NLNET::IService::getInstance()->getServiceId();
#endif

			// Fill local trackers
			setChangedLocal( entityIndex, propIndex );
		}
		// Obsolete: now we don't handle the case anymore when a service can overwrite a property value in the same
		// game cycle as another service.
		/*else if ( timestamp == localTimestamp )
		{
			// Compare source serviceid with the current mirror serviceid (priority rule)
			if ( srcServiceId < NLNET::IService::getInstance()->getServiceId() )
			{
				// Set new value 
				T *ptvalue;
				getPropPointer( &ptvalue, propIndex, TDataSetRow(entityIndex) );
				*ptvalue = value;

				// No need to change the timestamp, it's the same

				// Fill local trackers
				setChangedLocal( entityIndex, propIndex );
			}
			else
			{
				// Bounce value to all other services (reemit the property value to avoid wrong consistency)
				setChangedAll( entityIndex, propIndex );
			}
		}*/
	}

	/// Set a property list. Must be preceded by setupDestPropTrackersInterestedByDelta()
	template <class T>
	void							applyListPropChange( TDataSetIndex entityIndex, TPropertyIndex propIndex, NLNET::TServiceId srcServiceId, slist<T>& tempList, NLMISC::TGameCycle timestamp )
	{
		// Set if the received timestamp is newer than the stored timestamp
		NLMISC::TGameCycle& localTimestamp = _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex];
		//if ( timestamp > localTimestamp )
		{
			// Set list
			replaceList( entityIndex, propIndex, tempList );

			// Set timestamp
			localTimestamp = timestamp;

#ifdef STORE_CHANGE_SERVICEIDS
			_PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex] = NLNET::IService::getInstance()->getServiceId();
#endif

			// Fill local trackers
			setChangedLocal( entityIndex, propIndex );
		}
		// Obsolete: now we don't handle the case anymore when a service can overwrite a property value in the same
		// game cycle as another service.
		/*else if ( timestamp == localTimestamp )
		{
			// Compare source serviceid with the current mirror serviceid (priority rule)
			if ( srcServiceId < NLNET::IService::getInstance()->getServiceId() )
			{
				// Set new list 
				replaceList( entityIndex, propIndex, tempList );

				// No need to change the timestamp, it's the same

				// Fill local trackers
				setChangedLocal( entityIndex, propIndex );
			}
			else
			{
				// Bounce value to all other services (reemit the property value to avoid wrong consistency)
				setChangedAll( entityIndex, propIndex );
			}
		}*/
	}

	/// Erase a list and set the new one instead
	template <class T>
	void							replaceList( TDataSetIndex entityIndex, TPropertyIndex propIndex, slist<T>& tempList )
	{
		// Clear old list
		CMirrorPropValueListMS<T> theList( *this, TDataSetRow(entityIndex), propIndex );
		theList.clear();

		// Paste new list
		typename slist<T>::iterator it;
		for ( it=tempList.begin(); it!=tempList.end(); ++it )
		{
			theList.push_front( *it ); // re-reverse ordering
		}
	}

	/// Add the specified property into the local trackers
	void							setChangedLocal( TDataSetIndex entityIndex, TPropertyIndex propIndex )
	{
#ifdef NL_DEBUG
		nlassert( (uint32)propIndex < _SubscribersByProperty.size() );
#endif
		for ( TDestTrackersForDelta::iterator it=_DestTrackersForDelta.begin(); it!=_DestTrackersForDelta.end(); ++it )
		{
			(*it)->recordChange( entityIndex );
		}
	}

	/*/// Force the property to be reemitted to the remote mirror service
	void							setChangedAll( TDataSetIndex entityIndex, TPropertyIndex propIndex )
	{
		// Fill all trackers, including external ones
#ifdef NL_DEBUG
		nlassert( (uint32)propIndex < _SubscribersByProperty.size() );
#endif
		for ( TDestTrackersForDelta::iterator it=_DestTrackersForDeltaBounce.begin(); it!=_DestTrackersForDeltaBounce.end(); ++it )
		{
			(*it)->recordChange( entityIndex );
		}
	}*/

	/// Empty a list without knowing the type of elements
	void							killList( TDataSetRow entityIndex, TPropertyIndex propIndex );

	/// Increment the number of props for which local client services are able to write something
	void							incNbLocalWriterProps() { ++_NbLocalWriterProps; }

	/// Decrement the number of props for which local client services are able to write something
	void							decNbLocalWriterProps() { --_NbLocalWriterProps; }

	/// Return the number of props for which local client services are able to write something
	uint32							nbLocalWriterProps() const { return _NbLocalWriterProps; }

	/// Change a weight
	void							changeWeightOfProperty( TPropertyIndex propIndex, sint32 weight ) { _Weights[propIndex] = weight; calculateChangesBudgets(); }

	/// Return the weight of a property
	sint							getWeight( TPropertyIndex propIndex ) const { return _Weights[propIndex]; }

	/// Display weights
	void							displayWeights( NLMISC::CLog& log = *NLMISC::InfoLog ) const;

	/// Change the maximum size to send across MS
	void							setMaxOutBandwidth( uint32 mob ) { _MaxOutBandwidth = mob; calculateChangesBudgets(); }

	/// Return the maximum size to send across MS
	uint32							maxOutBandwidth() const { return _MaxOutBandwidth; }

	/// Return true if the specified property represents a list and not a single value
	bool							propIsList( TPropertyIndex propIndex ) const { return _PropIsList[propIndex]; }

	/// Display the trackers
	void							displayTrackers( NLMISC::CLog& log=*NLMISC::InfoLog) const;

	/// Calculate the changes budgets with the current bandwidth limit and property weights
	void							calculateChangesBudgets();

	/// Return a budget
	sint32							getChangesBudget( TPropertyIndex propIndex ) const { return _ChangesBudgets[propIndex]; }

#ifdef COUNT_MIRROR_PROP_CHANGES
	/// Return the number of changes for add/remove
	sint32							getNbChangesForRowManagement() const;

	/// Return the number of changes for props
	sint32							getNbChangesForPropChanges() const;
#endif

	/// Return the name of a property
	const std::string&				getPropertyName( TPropertyIndex propIndex ) const
	{
#ifdef NL_DEBUG
		nlassert( (uint)propIndex < (uint)nbProperties() ); // Wrong property
#endif
		return _PropertyNames[propIndex];
	}

	///
	void							readdAllocatedTrackers( NLNET::CMessage& msgin, NLNET::TServiceId serviceId, CSMIdPool& smidpool, CSMIdPool& mutidpool );

	///
	void							readdTracker( NLNET::TServiceId serviceId, TPropertyIndex propIndex, bool readOnly, bool writeOnly, bool pointingToGroup, TPropertyIndex propNotifyingTheGroup );
	
	/// Tell if a dataSetRow is too old (using its counter) (handles modulo issue)
	bool							dataSetRowIsTooOld( const TDataSetRow& dataSetRow ) const
	{
		uint8 currentCounter = getBindingCounter( dataSetRow.getIndex() );
		return (dataSetRow.counter() < currentCounter)
			|| ((dataSetRow.counter() > 230) && (currentCounter < 25)); // 50 ticks vs 205 ticks
	}

	///
	bool							transmitAllBindingCounters( NLMISC::CMemStream& deltaBuffer, const TClientServices& clientServices, bool forceRemovals, bool inDelta ) const;

	///
	void							transmitAllBindingCounters( CDeltaToMS *delta, const TClientServices& clientServices ) const;

	///
	void							transmitBindingCountersOfRange( NLMISC::CMemStream& deltaBuffer, TDataSetIndex first, TDataSetIndex end, NLNET::TServiceId serviceId ) const;

	///
	void							receiveAllBindingCounters( NLNET::CMessage& msgin );

	///
	void							processReceivedBindingCounters();

	///
	void							applyAllBindingCounters( NLNET::CMessage& msgin );

	///
	void							applyAllBindingCounters( const TBindingCountersToApply& bc );

	//uint32						NbOnlineEntities;

	friend class CMirrorService;
	friend class CDeltaToMS;

private:

	/// Indexed by TPropertyIndex
	TSubscriberLists		_SubscribersByProperty;

	/// Indexed by TEntityTrackerIndex
	TSubscriberListForEnt	_EntityTrackers[2];

	/// Local trackers on which the delta being received must be applied
	TDestTrackersForDelta	_DestTrackersForDelta;

	// Trackers on which the delta being received must be applied when bouncing a value changed by multiple services
	//TDestTrackersForDelta	_DestTrackersForDeltaBounce;

	/// Weights for inter-MS throttles
	std::vector< sint >		_Weights;

	/// The maximum size to send across MS (can be exceeded during one push, but with only one change)
	uint32					_MaxOutBandwidth;

	/// Number of props for which local client services are able to write something, in this dataset
	uint					_NbLocalWriterProps;

	/// The maximum numbers of changes that can be sent, by property
	std::vector< sint >		_ChangesBudgets;

	/// Property names
	std::vector<std::string> _PropertyNames;

	/// Counts received from a remote MS (at time of receiving them, we don't know if the dataset will be allocated used, so store them)
	std::vector<uint8>		*_BindingCountsToSet;

	/// Binding counter messages stored for application in applyDeltas() (for proper synchronization)
	std::vector<TBindingCountersToApply>	_ReceivedBindingCounters;

	// Owned entity ranges
	//TEntityRangeList		_SortedOwnedEntityRanges;

};


/*
 * Get the entity id corresponding to entityIndex in the dataset
 */
inline const NLMISC::CEntityId& CDataSetMS::getEntityId( const TDataSetRow& entityIndex ) const
{
#ifdef NL_DEBUG
	nlassertex( entityIndex.getIndex() < (uint32)maxNbRows(), ("E%d",entityIndex.getIndex()) );  // Wrong entity (including INVALID_ENTITY_INDEX and LAST_CHANGED)
	nlassert( _PropertyContainer.EntityIdArray.EntityIds );
#endif
	return _PropertyContainer.EntityIdArray.EntityIds[entityIndex.getIndex()];
}



/*
 * Constructor
 */
template <class T>
CMirrorPropValueListMS<T>::CMirrorPropValueListMS NL_TMPL_PARAM_ON_METHOD_1(T)( CDataSetMS& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex )
{
	dataSet.getPropPointerForList( &_PtFront, propIndex, entityIndex, (T*)NULL );
	_Container = (TSharedListCell*)(dataSet.getListCellContainer( propIndex ));
}


/*
 * Slow constructor
 */
/*template <class T>
CMirrorPropValueListMS<T>::CMirrorPropValueListMS<T>( CDataSetMS& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
	dataSet.getPropPointerForList( &_PtFront, _PropLocation.propIndex(), _PropLocation.dataSetRow(), (T*)NULL );
	_Container = (TSharedListCell*)(dataSet.getListCellContainer( _PropLocation.propIndex() ));
}*/


template <class T>
bool						CMirrorPropValueListMS<T>::empty() const
{
	return ((*_PtFront) == INVALID_SHAREDLIST_ROW);
}

template <class T>
CMirrorPropValueItemMS<T>		CMirrorPropValueListMS<T>::front()
{
	return CMirrorPropValueItemMS<T>( this, &(_Container[*_PtFront].Value) );
}

template <class T>
const T&					CMirrorPropValueListMS<T>::front() const
{
	return _Container[*_PtFront].Value;
}


template <class T>
void						CMirrorPropValueListMS<T>::push_front( const T& value )
{
	// Ensure the TSharedListCell struct has 1-byte packing
	nlctassert( sizeof(TSharedListRow) + sizeof(T) == sizeof(TSharedListCell) );

	TSharedListRow oldfront = *_PtFront;
	TSharedListRow newfront = allocateNewCell();
	if ( newfront != INVALID_SHAREDLIST_ROW )
	{
		_Container[newfront].Next = oldfront;
		_Container[newfront].Value = value;
		*_PtFront = newfront;
	}
	else
		nlwarning( "No shared list storage left" );
}

template <class T>
void						CMirrorPropValueListMS<T>::pop_front()
{
	TSharedListRow oldfront = *_PtFront;
#ifdef NL_DEBUG
	nlassertex( oldfront != INVALID_SHAREDLIST_ROW, ("pop_front() on empty list") );
#endif
	_Container[oldfront].Value.~T();
	*_PtFront = _Container[*_PtFront].Next;
	releaseCell( oldfront );
}

template <class T>
TSharedListRow										CMirrorPropValueListMS<T>::allocateNewCell()
{
	/*// Old trivial linear browsing code
	TSharedListRow row = 0;
	while ( (_Container[row].Next != UNUSED_SHAREDLIST_CELL) && (row < INVALID_SHAREDLIST_ROW) )
	{
		++row;
	ListCellAllocRow = row;
	}*/

	// New linked-list code
	TSharedListRow *firstFreeCell = TPropertyValueArrayHeader::getFreeCellsFront( (void*)_Container );
	TSharedListRow cellToAllocate = *firstFreeCell;
	if ( cellToAllocate != INVALID_SHAREDLIST_ROW )
		*firstFreeCell = _Container[cellToAllocate].Next;
	return cellToAllocate;
}

template <class T>
void												CMirrorPropValueListMS<T>::releaseCell( TSharedListRow row )
{
	/*// Old trivial linear browsing code
	_Container[row].Next = UNUSED_SHAREDLIST_CELL;
	*/

	// New linked-list code
	TSharedListRow *firstFreeCell = TPropertyValueArrayHeader::getFreeCellsFront( (void*)_Container );
	_Container[row].Next = *firstFreeCell;
	*firstFreeCell = row;
}

template <class T>
typename CMirrorPropValueListMS<T>::size_type				CMirrorPropValueListMS<T>::size()
{
	size_type s = 0;
	for ( iterator it=begin(); it!=end(); ++it )
	{
		++s;
		if ( s > 100 ) // TEMP workaround
			return ~0;
	}
	return s;
}

template <class T>
void												CMirrorPropValueListMS<T>::clear()
{
	TSharedListRow row = *_PtFront;
	*_PtFront = INVALID_SHAREDLIST_ROW;
	while ( row != INVALID_SHAREDLIST_ROW )
	{
		TSharedListRow oldrow = row;
		row = _Container[oldrow].Next;
		_Container[oldrow].Value.~T();
		releaseCell( oldrow );
		//nldebug( "Release cell" );
	}
}

template <class T>
typename CMirrorPropValueListMS<T>::iterator					CMirrorPropValueListMS<T>::begin()
{
	iterator it;
	it._ParentList = this;
	it._Index = *_PtFront;
	return it;
}
	
template <class T>
typename CMirrorPropValueListMS<T>::iterator					CMirrorPropValueListMS<T>::end()
{
	iterator it;
	it._ParentList = this;
	it._Index = INVALID_SHAREDLIST_ROW;
	return it;
}
	
/// Get the value
template <class T>
const T&											CMirrorPropValueItemMS<T>::operator() () const
{
	return *_Pt;
}

/// Set the value
template <class T>
CMirrorPropValueItemMS<T>&							CMirrorPropValueItemMS<T>::operator= ( const T& srcValue )
{
	*_Pt = srcValue;
	return *this;
}



#endif // NL_DATA_SET_MS_H

/* End of data_set_ms.h */
