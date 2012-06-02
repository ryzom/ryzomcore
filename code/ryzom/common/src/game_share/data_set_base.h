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



#ifndef NL_DATA_SET_BASE_H
#define NL_DATA_SET_BASE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/entity_id.h>
#include <nel/misc/sheet_id.h>
#include <nel/misc/bit_set.h>
#include <nel/misc/mem_stream.h>
#include <nel/net/service.h>
#include <nel/georges/u_form.h>

#include "tick_event_handler.h"
#include "base_types.h"

#include <vector>
#include <map>


extern NLMISC::CMemDisplayer *TmpDebugDisplayer;


// Known types of property value
enum TTypeOfProp { TypeUint8, TypeSint8, TypeUint16, TypeSint16, TypeUint32, TypeSint32, TypeUint64, TypeSint64, TypeFloat, TypeDouble, TypeBool, TypeCEntityId, TypeUnknown };


// Possibles sizes of row
enum TSizeOfRow { Row32, Row16, Row8 };


const std::string ENTITYID_PREFIX = "_EId_";
const sint LENGTH_OF_ENTITYID_PREFIX = 5;


/**
 * Exception class for the mirror system
 */
class EMirror : public NLMISC::Exception
{
};


const sint InvalidSMId = -1;


// Get dataset in sheetid map
#define GET_SDATASET(it) ((*it).second)

// Get dataset in name map
#define GET_NDATASET(it) (*((*it).second))


/**
 * Structure of a dataset for loading
 */
class TDataSetSheet
{
public:

	/// Read the sheet
	void			readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	/// Serial (for fast binary sheet loading)
	void			serial( NLMISC::IStream& s );

	/// Return the version of this class, increments this value when the content of this class changed
	static uint		getVersion () { return 3; }

	/// Void
	void			removed() {}

	/// Add a property which is not listed in .dataset files
	void			addACustomProperty( std::string& name, TTypeOfProp typeOfProp/*, sint8 weight*/, bool propIsList )
	{
		PropertyNames.push_back( name );
		PropIndexToType.push_back( typeOfProp );
		//Weights.push_back( weight );
		PropIsList.push_back( propIsList );
	}

	/// Return the dataset size to use. If found in the .cfg, use the value of the property DataSetSize + DataSetName., otherwise use the value in the sheet.
	TDataSetIndex	getConfigDataSetSize() const;

	std::string					DataSetName;

	TPropertyIndex				NbProperties;

	uint32						MaxNbRows;

	std::vector<std::string>	PropertyNames;

	std::vector<TTypeOfProp>	PropIndexToType;

	//std::vector<sint8>		Weights; // not used: currently all property values are always sent

	std::vector<bool>			PropIsList;

	std::vector<uint8>			EntityTypesFilter;
};


/*
 * TSDataSetSheets
 */
typedef std::map<NLMISC::CSheetId, TDataSetSheet> TSDataSetSheets;

// TServiceId (8 bits)
//typedef uint8 TServiceId8;


/**
 * Pointers for accessing the values and the change timestamps of a specific property
 */
struct TPropertyValueArrayHeader
{
	void				reset()
	{
		Values = NULL;
		ChangeTimestamps = NULL;
#ifdef STORE_CHANGE_SERVICEIDS
		ChangeServiceIds = NULL;
#endif
		ListCellContainer = NULL;
		DataTypeSize = 0;
//#ifdef NL_DEBUG
		IsReadOnly = false;
		IsMonitored = false;
//#endif
	}

	/// Pointer on the values(points into shared memory)
	void				*Values;

	/// Pointer on the timestamps (points into shared memory)
	NLMISC::TGameCycle	*ChangeTimestamps;

#ifdef STORE_CHANGE_SERVICEIDS
	/// Pointer on the change service ids (points into shared memory)
	NLNET::TServiceId8			*ChangeServiceIds;
#endif

	/// Pointer on the list container (points into shared memory if the property is a list property)
	void				*ListCellContainer;

	/// Return the location of the index of the first free cell (beginning the list of free cells)
	static TSharedListRow *	getFreeCellsFront( void *listCellContainer ) { return (TSharedListRow*)(((uint8*)listCellContainer) - sizeof(TSharedListRow)); }

	/// Data type size of the property for type checking in debug mode
	uint32				DataTypeSize;

//#ifdef NL_DEBUG
	/// Flag to check if the property is read-only
	bool				IsReadOnly;

	/// Flag to monitor the assignment of the values
	bool				IsMonitored;
//#endif
};


/**
 * Pointers for accessing the entity ids and the booleans to check if there are still online or if they have been removed
 */
struct TEntityIdArrayHeader
{
	void				reset()
	{
		EntityIds = NULL;
		OnlineBitfieldPt = NULL;
		OnlineTimestamps = NULL;
		Counts = NULL;
		SpawnerServiceIds = NULL;
	}

	/// Set the service responsible for managing the entity (spawning/despawning)
	void				setSpawnerServiceId( TDataSetIndex entityIndex, NLNET::TServiceId serviceId )
	{
		SpawnerServiceIds[entityIndex] = NLNET::TServiceId8(serviceId);
	}

	/// Test if online or removed
	bool				isOnline( TDataSetIndex entityIndex ) const
	{
		//nlassert(OnlineBitfieldPt);
		//nlassert(entityIndex>=0 && entityIndex<NumBits);
		uint32 mask = entityIndex & (NL_BITLEN-1);
		mask = 1 << mask;
		return (OnlineBitfieldPt[entityIndex >> NL_BITLEN_SHIFT] & mask) != 0;
	}

	/// Mark as online or removed, and update the online timestamp
	void				setOnline( TDataSetIndex entityIndex, bool value )
	{
		//nlassert(OnlineBitfieldPt);
		//nlassert(entityIndex>=0 && entityIndex<NumBits);
		uint32 mask= entityIndex & (NL_BITLEN-1);
		mask = 1 << mask;
		if ( value )
			OnlineBitfieldPt[entityIndex >> NL_BITLEN_SHIFT] |= mask ;
		else
			OnlineBitfieldPt[entityIndex >> NL_BITLEN_SHIFT] &= ~mask;

		// Timestamp the changing of online state
		updateOnlineTimestamp( entityIndex );
	}

	// Check that the same service created and set online/offline the entity
	void				checkSpawnerId( TDataSetIndex entityIndex, NLNET::TServiceId serviceId ) const
	{
		nlassert( SpawnerServiceIds[entityIndex] == NLNET::TServiceId8(serviceId) );
	}

	/// Update the online timestamp of an entity. Done when creating an entity, when setting/unsetting it online
	void				updateOnlineTimestamp( TDataSetIndex entityIndex )
	{
		OnlineTimestamps[entityIndex] = CTickEventHandler::getGameCycle();
	}

#ifdef CHECK_DATASETROW_VALIDITY
	/// Check if the row has not been reassigned and invalidated the dataset row object
	bool				isDataSetRowStillValid( const TDataSetRow& dataSetRow ) const
	{
		return (dataSetRow.counter() == 0) // no removal counter in the object
#ifdef NL_DEBUG
			// check if the row has not been reassigned to another entity;
			// if the entity has left, check if the entity timestamp is not too old
			|| (Counts[dataSetRow.getIndex()] == dataSetRow.counter())
			|| ( (Counts[dataSetRow.getIndex()] == dataSetRow.counter()+1) &&
			     (CTickEventHandler::getGameCycle() < OnlineTimestamps[dataSetRow.getIndex()]+15) )
			|| ( ((Counts[dataSetRow.getIndex()] == 1) && (dataSetRow.counter()==255)) && // 0 is skipped
			     (CTickEventHandler::getGameCycle() < OnlineTimestamps[dataSetRow.getIndex()]+15) );
#else
			// check if the row has not been reassigned to another entity (but it can have been deleted)
			|| (Counts[dataSetRow.getIndex()] == dataSetRow.counter())
			|| (Counts[dataSetRow.getIndex()] == dataSetRow.counter()+1)
			|| ( (Counts[dataSetRow.getIndex()] == 1) && (dataSetRow.counter()==255) ); // 0 is skipped
#endif
	}
#endif

	/// The entity ids
	NLMISC::CEntityId	*EntityIds;

	/// Theses bits tell whether a row is currently bound to an entity or not
	uint32				*OnlineBitfieldPt;

	/// Timestamp for entity binding (adding/removing)
	NLMISC::TGameCycle	*OnlineTimestamps;

	/// These counters are there to check if we don't forget to remove our row references when entities leave
	uint8				*Counts;

	/// These service ids tell which service is responsible for managing each entity.
	NLNET::TServiceId8			*SpawnerServiceIds;
};


/**
 * Pointers to entity ids and property values of one container, corresponding to one dataset.
 * Data pointed to may be in shared memory.
 *
 * Note: of course, pointers can't be placed in shared memory.
 */
struct TPropertyContainerHeader
{
	/// Constructor
	TPropertyContainerHeader() {}

	/// Reset
	void				init( TPropertyIndex length )
	{
		PropertyValueArrays.resize( length );
		for ( sint i=0; i!=length; ++i )
		{
			PropertyValueArrays[i].reset();
		}
		EntityIdArray.reset();
	}

	/// Array (indexed by TPropertyIndex) of property value arrays (indexed by TDataSetRow)
	std::vector< TPropertyValueArrayHeader >	PropertyValueArrays;

	/// Array (indexed by TDataSetRow) of entity ids
	TEntityIdArrayHeader						EntityIdArray;
};

typedef CHashMap< NLMISC::CEntityId, TDataSetIndex, NLMISC::CEntityIdHashMapTraits > TEntityIdToEntityIndexMap;
#define GET_ENTITY_INDEX(it) ((*it).second)


enum TEntityTrackerIndex { ADDING = 0, REMOVING = 1 };


/**
 * Base class for CMirroredDataSet and CDataSetMS.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CDataSetBase
{
public:

	/// Constructor
	CDataSetBase();

	/// Initialize
	void						init( const NLMISC::CSheetId& sheetId, const TDataSetSheet& properties );

	/// Return the name of the dataset
	const std::string&			name() const { return _DataSetName; }

	/// Return the sheet id of the dataset
	const NLMISC::CSheetId&		sheetId() const { return _SheetId; }

	/// Return the maximum number of rows (or entities)
	TDataSetIndex				maxNbRows() const { return _MaxNbRows; }

	/// Return the number of properties in the dataset
	TPropertyIndex				nbProperties() const { return (TPropertyIndex)_PropIndexToType.size(); }

	/// Return the type of the specified property
	TTypeOfProp					getPropType( TPropertyIndex propIndex ) const
	{
		if ( ((uint32)propIndex) < _PropIndexToType.size() )
		{
			return _PropIndexToType[propIndex];
		}
		else
		{
//#ifdef NL_DEBUG
			nlwarning( "MIRROR: getPropType(): Property index outside of dataset" );
//#endif
			return TypeUnknown;
		}
	}

	/// Return the size of a value of the specified property (in bytes)
	uint32						getDataSizeOfProp( TPropertyIndex propIndex ) const
	{
#ifndef FAST_MIRROR
		nlassert( (uint)propIndex < (uint)nbProperties() );
#endif
		return _DataSizeByProp[propIndex];
	}

//	/// Serialize out a datasetrow, depending on the dataset
//	void						serialOutRow( NLMISC::CMemStream& msgout, const TDataSetRow& entityIndex ) const
//	{
//		msgout.fastWrite( entityIndex );
//
//		//switch( _SizeOfRow )
//		//{
//		//case Row32 : msgout.fastWrite( entityIndex ); break;
//		//case Row16 : { TDataSetRow16 row16 = (TDataSetRow16)entityIndex.getIndex(); msgout.fastWrite( row16 ); /*nldebug( "Written %hu", row16 );*/ break; }
//		//case Row8  : { TDataSetRow8 row8 = (TDataSetRow8)entityIndex.getIndex(); msgout.fastWrite( row8 ); break; }
//		//default  : ;
//		//}
//	}

//	/// Serialize in a datasetrow, depending on the dataset
//	void						serialInRow( NLMISC::CMemStream& msgin, TDataSetRow& entityIndex ) const
//	{
//		msgin.fastRead( entityIndex );
//
//		switch( _SizeOfRow )
//		{
//		case Row32 : msgin.fastRead( entityIndex ); break;
//		case Row16 :
//			{
//				TDataSetRow16 row16;
//				msgin.fastRead( row16 );
//				//nldebug( "Read %hu", row16 );
//				if ( row16 == (TDataSetRow16)~0 )
//					entityIndex	=	TDataSetRow();
//				else
//					entityIndex = (TDataSetRow)row16;
//				break;
//			}
//		case Row8 :
//			{
//				TDataSetRow8 row8;
//				msgin.fastRead( row8 );
//				if ( row8 == (TDataSetRow8)~0 )
//					entityIndex	=	TDataSetRow();
//				else
//					entityIndex = (TDataSetRow)row8;
//				break;
//			}
//		//default : ;
//		}
//	}

    // PUBLIC because template friend classes are not supported
	/// Return the pointer to the property value
	template <class T>
	void						getPropPointer( T **ppt, TPropertyIndex propIndex, const TDataSetRow& entityIndex ) const
	{
#ifndef FAST_MIRROR
		nlassert( (propIndex != INVALID_PROPERTY_INDEX) && (propIndex < (TPropertyIndex)_PropertyContainer.PropertyValueArrays.size()) ); // Wrong property
		nlassert( _PropertyContainer.PropertyValueArrays[propIndex].Values ); // Pointer not initialized
		nlassertex( entityIndex.getIndex() < (uint32)maxNbRows(), ("E%d",entityIndex.getIndex()) );  // Wrong entity (including INVALID_ENTITY_INDEX and LAST_CHANGED)
		checkTemplateSize( sizeof(T), propIndex, entityIndex ); // Wrong template type
#endif
#ifdef CHECK_DATASETROW_VALIDITY
		//nlassertex( isDataSetRowStillValid( entityIndex ), ("E%u GIVEN:%hu_%u REAL:%hu_%u", entityIndex.getIndex(), entityIndex.counter(), CTickEventHandler::getGameCycle(), _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()], _PropertyContainer.EntityIdArray.OnlineTimestamps[entityIndex.getIndex()] ) ); // entity not removed
		/*if ( ! isDataSetRowStillValid( entityIndex ) ) // entity not removed
		{
			nlwarning( "isDataSetRowStillValid E%u GIVEN:%hu_%u REAL:%hu_%u", entityIndex.getIndex(), entityIndex.counter(), CTickEventHandler::getGameCycle(), _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()], _PropertyContainer.EntityIdArray.OnlineTimestamps[entityIndex.getIndex()] );
			if ( TmpDebugDisplayer )
				TmpDebugDisplayer->write( NLMISC::DebugLog );
			nlstop;
		}*/
#endif
		*ppt = &(((T*)(_PropertyContainer.PropertyValueArrays[propIndex].Values))[entityIndex.getIndex()]);
	}

    // PUBLIC because template friend classes are not supported
	/// Return the pointer to the property value
	template <class T>
	void						getPropPointerForList( TSharedListRow **ppt, TPropertyIndex propIndex, const TDataSetRow& entityIndex, T* /* typeHint */ ) const
	{
#ifndef FAST_MIRROR
		nlassert( (propIndex != INVALID_PROPERTY_INDEX) && (propIndex < (TPropertyIndex)_PropertyContainer.PropertyValueArrays.size()) ); // Wrong property
		nlassert( _PropertyContainer.PropertyValueArrays[propIndex].Values ); // Pointer not initialized
		nlassertex( entityIndex.getIndex() < (uint32)maxNbRows(), ("E%d",entityIndex.getIndex()) );  // Wrong entity (including INVALID_ENTITY_INDEX and LAST_CHANGED)
		// Following test commented out, because property actually in row is not the size of the property in list!
		checkTemplateSize( sizeof(T), propIndex, entityIndex ); // Wrong template type
#endif
#ifdef CHECK_DATASETROW_VALIDITY
		//nlassertex( isDataSetRowStillValid( entityIndex ), ("E%u GIVEN:%hu_%u REAL:%hu_%u", entityIndex.getIndex(), entityIndex.counter(), CTickEventHandler::getGameCycle(), _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()], _PropertyContainer.EntityIdArray.OnlineTimestamps[entityIndex.getIndex()] ) ); // entity not removed
		/*if ( ! isDataSetRowStillValid( entityIndex ) ) // entity not removed
		{
			nlwarning( "isDataSetRowStillValid E%u GIVEN:%hu_%u REAL:%hu_%u", entityIndex.getIndex(), entityIndex.counter(), CTickEventHandler::getGameCycle(), _PropertyContainer.EntityIdArray.Counts[entityIndex.getIndex()], _PropertyContainer.EntityIdArray.OnlineTimestamps[entityIndex.getIndex()] );
			if ( TmpDebugDisplayer )
				TmpDebugDisplayer->write( NLMISC::DebugLog );
			nlstop;
		}*/
#endif
		*ppt = &(((TSharedListRow*)(_PropertyContainer.PropertyValueArrays[propIndex].Values))[entityIndex.getIndex()]);
	}

#ifdef CHECK_DATASETROW_VALIDITY
	/// Return false if the row has been invalidated (reassigned)
	bool						isDataSetRowStillValid( const TDataSetRow& dataSetRow ) const
	{
		return _PropertyContainer.EntityIdArray.isDataSetRowStillValid( dataSetRow );
	}
#endif

	/// Return the current removal counter of a row
	uint8						getBindingCounter( TDataSetIndex entityIndex ) const
	{
		return _PropertyContainer.EntityIdArray.Counts[entityIndex];
	}

	/// Check that the template argument T passed to CMirrorPropValue<T> has the right size (debug feature)
	void						checkTemplateSize( uint32 passedSize, TPropertyIndex propIndex, const TDataSetRow& entityIndex ) const;

//#ifdef NL_DEBUG
	/// Return true if the specified property must be read-only (debug feature)
	bool						isReadOnly( TPropertyIndex propIndex ) const
	{
		return _PropertyContainer.PropertyValueArrays[propIndex].IsReadOnly;
	}

	/// Return true if the specified property must be monitored (debug feature)
	bool						isMonitored( TPropertyIndex propIndex ) const
	{
		return _PropertyContainer.PropertyValueArrays[propIndex].IsMonitored;
	}
//#endif

	/// Return the pointer of the cell container for a list property
	void						*getListCellContainer( TPropertyIndex propIndex ) const
	{
#ifndef FAST_MIRROR
		nlassert( (propIndex != INVALID_PROPERTY_INDEX) && (propIndex < (TPropertyIndex)_PropertyContainer.PropertyValueArrays.size()) ); // Wrong property
#endif
		return _PropertyContainer.PropertyValueArrays[propIndex].ListCellContainer;
	}

	/// Return the timestamp of a property
	NLMISC::TGameCycle			getChangeTimestamp( TPropertyIndex propIndex, const TDataSetRow& entityIndex ) const
	{
#ifndef FAST_MIRROR
		nlassert( (propIndex != INVALID_PROPERTY_INDEX) && (propIndex < (TPropertyIndex)_PropertyContainer.PropertyValueArrays.size()) ); // Wrong property
		nlassert( _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps ); // Pointer not initialized
		nlassertex( entityIndex.getIndex() < (uint32)maxNbRows(), ("E%d",entityIndex.getIndex()) );  // Wrong entity (including INVALID_ENTITY_INDEX and LAST_CHANGED)
#endif
		return _PropertyContainer.PropertyValueArrays[propIndex].ChangeTimestamps[entityIndex.getIndex()];
	}

#ifdef STORE_CHANGE_SERVICEIDS
	/// Return the id of the latest service who changed the property value
	NLNET::TServiceId8					getWriterServiceId( TPropertyIndex propIndex, const TDataSetRow& entityIndex ) const
	{
#ifndef FAST_MIRROR
		nlassert( (propIndex != INVALID_PROPERTY_INDEX) && (propIndex < (TPropertyIndex)_PropertyContainer.PropertyValueArrays.size()) ); // Wrong property
		nlassert( _PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds ); // Pointer not initialized
		nlassertex( entityIndex.getIndex() < (uint32)maxNbRows(), ("E%d",entityIndex.getIndex()) );  // Wrong entity (including INVALID_ENTITY_INDEX and LAST_CHANGED)
#endif
		return _PropertyContainer.PropertyValueArrays[propIndex].ChangeServiceIds[entityIndex.getIndex()];
	}
#endif

	/// Return true if the row is still used
	bool						isOnline( const TDataSetRow& entityIndex ) const
	{
#ifndef FAST_MIRROR
		nlassert( entityIndex.isValid() );
#endif
		return _PropertyContainer.EntityIdArray.isOnline( entityIndex.getIndex() );
	}

	/// Return the gamecycle of adding or removing
	NLMISC::TGameCycle			getOnlineTimestamp( const TDataSetRow& entityIndex ) const
	{
#ifndef FAST_MIRROR
		nlassert( entityIndex.isValid() );
#endif
		return _PropertyContainer.EntityIdArray.OnlineTimestamps[entityIndex.getIndex()];
	}

protected:

	/** Fill property info.
	 * When the flag initValues is set:
	 * If the counts must be initialised from a remote MS, use the vector ptCountsToSet (it will be read
	 * then deleted) and timestamp, otherwise set NULL.
	 * segmentSize is used only if initValues is true.
	 * If propName is the special entityId property, the behaviour is different (propName need not exist)
	 */
	void						setPropertyPointer( std::string& propName, TPropertyIndex propIndex, void *segmentPt, bool initValues, uint32 dataTypeSize, bool isReadOnly, bool mustMonitorAssignment, std::vector<uint8> *ptCountsToSet=NULL, NLMISC::TGameCycle timestamp=0, uint32 segmentSize=~0 );

	/// Get the property index corresponding to the specified property name in the dataset
	TPropertyIndex				getPropertyIndex( const std::string& propName ) const;

	/// Init _DataSizeByProp
	void						fillDataSizeByProp();

	/// Return the id of the service who spawned the entity (even after it was despawned, while the row is not reassigned)
	NLNET::TServiceId8					getSpawnerServiceId( TDataSetIndex entityIndex ) const
	{
		return _PropertyContainer.EntityIdArray.SpawnerServiceIds[entityIndex];
	}

	TPropertyContainerHeader	_PropertyContainer;

	// Name of the dataset
	std::string					_DataSetName;

	// Sheet id of the data
	NLMISC::CSheetId			_SheetId;

	/// Maximum number of rows
	TDataSetIndex				_MaxNbRows;

	// Size of datasetrow needed
	//TSizeOfRow					_SizeOfRow;

	/// Indexed by TPropertyIndex
	std::vector< TTypeOfProp >	_PropIndexToType;

	/// Indexed by TPropertyIndex
	std::vector< uint32	>		_DataSizeByProp;

	/// Indexed by TPropertyIndex
	std::vector<bool>			_PropIsList;

	/// Entity types allowed in this dataset
	std::vector<uint8>			_EntityTypesFilter;

};


/**
 * List of entity types accepted in a tracker.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2004
 */
/*class CEntityTrackerFilter
{
public:

	/// Add an entity type into the list of accepted entity types (no duplicate check)
	void						addAcceptedEntityType( uint8 entityType ) { _AcceptedTypes.push_back( entityType ); }

	/// Test if an entity type is in the list
	void						isAccepted( uint8 entityType ) { return (find( _AcceptedTypes.begin(), _AcceptedTypes.end(), entityType ) != _AcceptedTypes.end()); }

private:

	/// List of types (a vector for fast browsing with a few elements)
	std::vector<uint8>			_AcceptedTypes;
};*/


#endif // NL_DATA_SET_BASE_H

/* End of data_set_base.h */
