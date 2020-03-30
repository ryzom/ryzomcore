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



#ifndef NL_MIRROR_PROP_VALUE_INLINE_H
#define NL_MIRROR_PROP_VALUE_INLINE_H

#include "nel/misc/types_nl.h"

//#ifdef NL_DEBUG
#include "nel/misc/common.h"
//#endif

#include <limits>

/*
 * Activate this define if you want to set a value into the mirror only if it is different from the previous value
 * (more comparisons but less transfers if you ofter set a property to the same value)
 */
#define MIRROR_ASSIGN_ONLY_IF_CHANGED
//#undef MIRROR_ASSIGN_ONLY_IF_CHANGED


/* -------------------------------------------------------------------------------------------------------
 * CPropLocation...
 * ----------------------------------------------------------------------------------------------------- */

#ifndef CHECK_DATASETROW_VALIDITY

/// Constructor
inline CPropLocationPacked1DS::CPropLocationPacked1DS( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex )
{
	init( dataSet, propIndex, entityIndex );
}

/// Initialize
inline void				CPropLocationPacked1DS::init( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex )
{
	_DataSetRow16 = (TDataSetRow16)entityIndex.getIndex();
	_PropIndexAndDSR4 = propIndex | (uint16)((((uint32)entityIndex.getIndex()) & 0xF0000) >> 4);
}

/// Go back to default state (deinit)
inline void				CPropLocationPacked1DS::reset()
{
	_DataSetRow16 = 0;
	_PropIndexAndDSR4 = 0;
}

/// Get dataset
inline CMirroredDataSet	*CPropLocationPacked1DS::dataSet() const
{
	return firstDataset();
}

/// Get entity index
inline TDataSetRow		CPropLocationPacked1DS::dataSetRow() const { return (TDataSetRow)((uint32)_DataSetRow16 | (((uint32)_PropIndexAndDSR4) & 0xF000) << 4); }

/// Get property index
inline TPropertyIndex	CPropLocationPacked1DS::propIndex() const
{
	return _PropIndexAndDSR4 & 0x0FFF;
}

#ifdef NL_DEBUG

/*
 * Return true if the specified property must be read-only (debug feature)
 */
inline bool				CPropLocationPacked1DS::isReadOnly() const
{
	return dataSet()->isReadOnly( propIndex() );
}

/*
 * Return true if the row is valid for writing/reading
 */
inline bool				CPropLocationPacked1DS::rowIsUsed() const
{
	if ( dataSet()->getEntityId( dataSetRow() ).isUnknownId() )
	{
		nlwarning( "MIRROR/%s: Writing value of prop %hd for row %d which is empty (no CEntityId)", dataSet()->name().c_str(), propIndex(), dataSetRow().getIndex() );
		return false;
	}
	else
		return true;
}

/*
 * Display the new value when it has changed, if monitoring is enabled for the property
 */
inline void				CPropLocationPacked1DS::monitorValue( const char *valueStr ) const
{
	if ( dataSet()->isMonitored( propIndex() ) )
	{
		MIRROR_INFO( "MIRROR:MON: Value of %s/E%d/P%hd changed to %s", dataSet()->name().c_str(), dataSetRow().getIndex(), propIndex(), valueStr );
	}
}


/*
 * Set changed, test if row is valid, and do monitoring
 */
inline void				CPropLocationPacked1DS::setChanged( const char *valueStr )
{
	if ( rowIsUsed() )
	{
		dataSet()->setChanged( dataSetRow(), propIndex() );
		monitorValue( valueStr );
	}
}


#else

/// Set changed
inline void				CPropLocationPacked1DS::setChanged() { dataSet()->setChanged( dataSetRow(), propIndex() ); }

#endif


/// Constructor
template <int nbits>
inline CPropLocationPacked<nbits>::CPropLocationPacked( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex )
{
	init( dataSet, propIndex, entityIndex );
}

/// Initialize
template <int nbits>
inline void				CPropLocationPacked<nbits>::init( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex )
{
	_DataSetRow16 = (TDataSetRow16)entityIndex.getIndex();
	_PropIndexAndDataSetAndDSR4 = (uint16)propIndex | ( (datasetToBitIndex(&dataSet)) << (12-nbits) ) | (uint16)((((uint32)entityIndex.getIndex()) & 0xF0000) >> 4);
}

/// Go back to default state (deinit)
template <int nbits>
inline void				CPropLocationPacked<nbits>::reset()
{
	_DataSetRow16 = 0;
	_PropIndexAndDataSetAndDSR4 = (datasetToBitIndex(NULL)) << (12-nbBitsForDataset);
}

/// Get dataset
template <int nbits>
inline CMirroredDataSet	*CPropLocationPacked<nbits>::dataSet() const
{
	return bitIndexToDataset((_PropIndexAndDataSetAndDSR4 & 0x0FFF) >> (12-nbits));
}

/// Get entity index
template <int nbits>
inline TDataSetRow		CPropLocationPacked<nbits>::dataSetRow() const { return	dataSet()->getCurrentDataSetRow(((uint32)_DataSetRow16 | (((uint32)_PropIndexAndDataSetAndDSR4) & 0xF000) << 4)); }

/// Get property index
template <int nbits>
inline TPropertyIndex	CPropLocationPacked<nbits>::propIndex() const
{
	return (TPropertyIndex)(_PropIndexAndDataSetAndDSR4 & ((1<<(12-nbits)) - 1));
}

#ifdef NL_DEBUG

/*
 * Return true if the specified property must be read-only (debug feature)
 */
template <int nbits>
inline bool				CPropLocationPacked<nbits>::isReadOnly() const
{
	return dataSet()->isReadOnly( propIndex() );
}

/*
 * Return true if the row is valid for writing/reading
 */
template <int nbits>
inline bool				CPropLocationPacked<nbits>::rowIsUsed() const
{
	if ( dataSet()->getEntityId( dataSetRow() ).isUnknownId() )
	{
		nlwarning( "MIRROR/%s: Writing value of prop %hd for row %d which is empty (no CEntityId)", dataSet()->name().c_str(), propIndex(), dataSetRow().getIndex() );
		return false;
	}
	else
		return true;
}

/*
 * Display the new value when it has changed, if monitoring is enabled for the property
 */
template <int nbits>
inline void				CPropLocationPacked<nbits>::monitorValue( const char *valueStr ) const
{
	if ( dataSet()->isMonitored( propIndex() ) )
	{
		MIRROR_INFO( "MIRROR:MON: Value of %s/E%d/P%hd changed to %s", dataSet()->name().c_str(), dataSetRow().getIndex(), propIndex(), valueStr );
	}
}


/*
 * Set changed, test if row is valid, and do monitoring
 */
template <int nbits>
inline void				CPropLocationPacked<nbits>::setChanged( const char *valueStr )
{
	if ( rowIsUsed() )
	{
		dataSet()->setChanged( dataSetRow(), propIndex() );
		monitorValue( valueStr );
	}
}


#else

/// Set changed
template <int nbits>
inline void				CPropLocationPacked<nbits>::setChanged() { dataSet()->setChanged( dataSetRow(), propIndex() ); }

#endif

#endif // CHECK_DATASETROW_VALIDITY


/// Initialize
inline void				CPropLocationUnpacked::init( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex )
{
	_DataSet = &dataSet; _PropIndex = propIndex; _DataSetRow = entityIndex;
}

/// Go back to default state (deinit)
inline void				CPropLocationUnpacked::reset()
{
	_DataSet = NULL;
	/*_PropIndex = 0; // no need to reset other things than _DataSet
	_DataSetRow = 0;*/ // this would not compile anyway
}

/// Get dataset
inline CMirroredDataSet	*CPropLocationUnpacked::dataSet() const { return _DataSet; }

/// Get entity index
inline TDataSetRow		CPropLocationUnpacked::dataSetRow() const { return _DataSetRow; }

/// Get property index
inline TPropertyIndex	CPropLocationUnpacked::propIndex() const { return _PropIndex; }

#ifdef NL_DEBUG

/*
 * Return true if the specified property must be read-only (debug feature)
 */
inline bool				CPropLocationUnpacked::isReadOnly() const
{
	return _DataSet->isReadOnly( _PropIndex );
}

/*
 * Return true if the row is valid for writing/reading
 */
inline bool				CPropLocationUnpacked::rowIsUsed() const
{
	if ( _DataSet->getEntityId( _DataSetRow ).isUnknownId() )
	{
		nlwarning( "MIRROR/%s: Writing value of prop %hd for row %u which is empty (no CEntityId)", _DataSet->name().c_str(), _PropIndex, _DataSetRow.getIndex() );
		return false;
	}
	else
		return true;
}

/*
 * Display the new value when it has changed, if monitoring is enabled for the property
 */
inline void				CPropLocationUnpacked::monitorValue( const char *valueStr ) const
{
	if ( _DataSet->isMonitored( _PropIndex ) )
	{
		MIRROR_INFO( "MIRROR:MON: Value of %s/E%u/P%hd changed to %s", _DataSet->name().c_str(), _DataSetRow.getIndex(), _PropIndex, valueStr );
	}
}

#else

/// Set changed
inline void				CPropLocationUnpacked::setChanged() { _DataSet->setChanged( _DataSetRow, _PropIndex ); }

#endif



/* -------------------------------------------------------------------------------------------------------
 * CMirrorPropValueBase and CMirrorPropValue
 * ----------------------------------------------------------------------------------------------------- */

/*
 * Constructor
 */
template <class T>
CMirrorPropValueBase<T>::CMirrorPropValueBase NL_TMPL_PARAM_ON_METHOD_1(T)( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex )
{
	init( dataSet, entityIndex, propIndex );
}


/*
 * Constructor
 */
template <class T, class CPropLocationClass >
CMirrorPropValue<T,CPropLocationClass>::CMirrorPropValue NL_TMPL_PARAM_ON_METHOD_2(T,CPropLocationClass)( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) :
	CMirrorPropValueBase<T>( dataSet, entityIndex, propIndex ),
	_PropLocation( dataSet, propIndex, entityIndex )
{
}


/*
 * Slow constructor
 */
template <class T>
CMirrorPropValueBase<T>::CMirrorPropValueBase NL_TMPL_PARAM_ON_METHOD_1(T)( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
	init( dataSet, entityId, propName );
}


/*
 * Slow constructor
 */
template <class T, class CPropLocationClass>
CMirrorPropValue<T,CPropLocationClass>::CMirrorPropValue NL_TMPL_PARAM_ON_METHOD_2(T,CPropLocationClass)( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
	init( dataSet, entityId, propName );
}


/*
 * Init (after default construction)
 */
template <class T>
void					CMirrorPropValueBase<T>::init( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex )
{
#ifndef FAST_MIRROR
	nlassertex( entityIndex.getIndex() < (uint32)dataSet.maxNbRows(), ("E%d", entityIndex.getIndex()) );
	/*if ( dataSet.getEntityId( entityIndex ).isUnknownId() )
	{
		nlwarning( "MIRROR: Accessing a value of prop %hu for row %u which is empty (no CEntityId)", propIndex, entityIndex.getIndex() );
	}*/
#endif
	dataSet.getPropPointer( &_Pt, propIndex, entityIndex );
}


/*
 * Init (after default construction)
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::init( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex )
{
	_PropLocation.init( dataSet, propIndex, entityIndex );
	CMirrorPropValueBase<T>::init( dataSet, entityIndex, propIndex );
}


/*
 * Slow init (after default construction)
 */
template <class T>
void						CMirrorPropValueBase<T>::init( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
	dataSet.getPropPointer( &_Pt, dataSet.getPropertyIndex( propName ), dataSet.getDataSetRow( entityId ) );
}


/*
 * Slow init (after default construction)
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::init( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
	_PropLocation.init( dataSet, dataSet.getPropertyIndex( propName ), dataSet.getDataSetRow( entityId ) );
	CMirrorPropValueBase<T>::init( dataSet, _PropLocation.dataSetRow(), _PropLocation.propIndex() );
}


/*
 * Return true if the CMirrorPropValue<T,CPropLocationClass> object is initialized or contains a temporary value (i.e. operator () can be called)
 */
template <class T>
bool						CMirrorPropValueBase<T>::isReadable() const
{
	return (_Pt != NULL);
}

template <class T, class CPropLocationClass> bool CMirrorPropValue<T,CPropLocationClass>::isReadable() const
{ return CMirrorPropValueBase<T>::isReadable(); }



/*
 * Return true if the CMirrorPropValue<T,CPropLocationClass> object is initialized in the mirror
 */
template <class T, class CPropLocationClass>
bool						CMirrorPropValue<T,CPropLocationClass>::isInitialized() const
{
#ifndef FAST_MIRROR
	if ( _PropLocation.dataSet() != NULL )
	{
		nlassert( CMirrorPropValueBase<T>::_Pt != NULL );
		return true;
	}
	else
		return false;
#else
	return (_PropLocation.dataSet() != NULL);
#endif
}


/*
 * Get the value
 */
template <class T>
const T&					CMirrorPropValueBase<T>::operator() () const
{
#ifndef FAST_MIRROR
	nlassert( _Pt ); // not initialized yet?
#endif
	return *_Pt;
}


/*
 * Allocate and store a temporary value, NOT in the mirror. The CMirrorPropValue<T,CPropLocationClass>
 * object must be UNitialized (i.e. the default constructor has been called but
 * not any other constructor or init()).
 */
template <class T>
void						CMirrorPropValueBase<T>::tempStore( const T& srcValue )
{
#ifndef FAST_MIRROR
	nlassert( ! _Pt );
#endif
	_Pt = new T();
	*_Pt = srcValue;
}


/*
 * Unaccess mirror (use only after init() or tempMirrorize()) and store a temporary value
 */
template <class T>
void						CMirrorPropValueBase<T>::tempRestore( const T& srcValue )
{
#ifdef NL_DEBUG
	nlassert( _Pt );
#endif
	_Pt = NULL;
	tempStore( srcValue );
}


/*
 * Same as tempStore but no assignment is done
 */
template <class T>
void						CMirrorPropValueBase<T>::tempAllocate()
{
#ifndef FAST_MIRROR
	nlassert( ! _Pt );
#endif
	_Pt = new T();
}


/*
 * Return the temporary value stored
 */
template <class T>
const T&					CMirrorPropValueBase<T>::tempValue() const
{
#ifndef FAST_MIRROR
	nlassert( _Pt );
#endif
	return *_Pt;
}


/*
 * Delete the temporary value stored if it becomes useless
 */
template <class T>
void						CMirrorPropValueBase<T>::tempDelete()
{
#ifndef FAST_MIRROR
	nlassert( _Pt );
#endif
	delete _Pt;
	_Pt = NULL;
}



template <class T, class CPropLocationClass> const T& CMirrorPropValue<T,CPropLocationClass>::operator() () const
{ return CMirrorPropValueBase<T>::operator()(); }


/*
 * Return the timestamp of the property change
 */
template <class T, class CPropLocationClass>
NLMISC::TGameCycle			CMirrorPropValue<T,CPropLocationClass>::getTimestamp() const
{
#ifndef FAST_MIRROR
	nlassert( _PropLocation.dataSet() );
#endif
	return _PropLocation.dataSet()->getChangeTimestamp( _PropLocation.propIndex(), _PropLocation.dataSetRow() );
}


#ifdef STORE_CHANGE_SERVICEIDS
/*
 * Return the id of the latest service who changed the property value
 */
template <class T, class CPropLocationClass>
NLNET::TServiceId8					CMirrorPropValue<T,CPropLocationClass>::getWriterServiceId() const
{
#ifndef FAST_MIRROR
	nlassert( _PropLocation.dataSet() );
#endif
	return _PropLocation.dataSet()->getWriterServiceId( _PropLocation.propIndex(), _PropLocation.dataSetRow() );
}
#endif


/*
 * Set the value
 */
template <class T, class CPropLocationClass>
CMirrorPropValue<T,CPropLocationClass>&		CMirrorPropValue<T,CPropLocationClass>::operator= ( const T& srcValue )
{
	// Debug checks
#ifndef FAST_MIRROR
	nlassert( CMirrorPropValueBase<T>::_Pt ); // not initialized yet?
	nlassert( _PropLocation.dataSet() ); // not initialized in the mirror yet?
#endif
#ifdef NL_DEBUG
	if (_PropLocation.dataSet())
	{
		nlassert( (_PropLocation.dataSetRow().isValid()) && (_PropLocation.dataSetRow().getIndex() < (uint32)_PropLocation.dataSet()->maxNbRows()) );
		if ( _PropLocation.isReadOnly() )
			nlwarning( "MIRROR: Overwriting a value of read-only property %s/%hd", _PropLocation.dataSet()->name().c_str(), _PropLocation.propIndex() );
	}
#endif

	// Copy the value (if MIRROR_ASSIGN_ONLY_IF_CHANGED: only if the new value is different)
#ifdef MIRROR_ASSIGN_ONLY_IF_CHANGED
	if ( srcValue != *CMirrorPropValueBase<T>::_Pt )
	{
#endif

	// Atomicity is not ensured (e.g. for int64) but setChanged() is called after complete assigment
#ifdef NL_DEBUG
		*CMirrorPropValueBase<T>::_Pt = srcValue;
		_PropLocation.setChanged( NLMISC::toString( srcValue ).c_str() );
#else
		*CMirrorPropValueBase<T>::_Pt = srcValue;
		_PropLocation.setChanged();
#endif
	// And set as changed

#ifdef MIRROR_ASSIGN_ONLY_IF_CHANGED
	}
#endif
	return *this;
}


/*
 * Return the location
 */
template <class T, class CPropLocationClass>
const CPropLocationClass&		CMirrorPropValue<T,CPropLocationClass>::location() const { return _PropLocation; }


/*
 * Serial
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::serial( NLMISC::IStream& s )
{
	if ( s.isReading() )
	{
		T temp;
		s.serial( temp );
		(*this) = temp;
	}
	else
	{
		s.serial( const_cast<T&>((*this)()) );
	}
}


/*
 * Special serial: if Reading, reassign in Temp storage (off-mirror), if Writing, get from Mirror or temp storage.
 * Precondition: if reading, tempStore() must have been called before and not tempMirrorize() yet.
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::serialRTWM( NLMISC::IStream& s )
{
	if ( s.isReading() )
	{
		T temp;
		s.serial( temp );
		tempReassign( temp );
	}
	else
	{
		s.serial( const_cast<T&>((*this)()) );
	}
}


/*
 * Return an human-readable string (callable if provided by class T)
 */
template <class T>
std::string					CMirrorPropValueBase<T>::toString() const
{
	return NLMISC::toString((*this)());
}

template <class T, class CPropLocationClass> std::string CMirrorPropValue<T,CPropLocationClass>::toString() const
{ return CMirrorPropValueBase<T>::toString(); }


/*
 * Allocate and store a temporary value, NOT in the mirror. The CMirrorPropValue<T,CPropLocationClass>
 * object must be UNitialized (i.e. the default constructor has been called but
 * not any other constructor or init()).
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::tempStore( const T& srcValue )
{
#ifndef FAST_MIRROR
	nlassertex( !isInitialized(), ("tempStore used on CMirrorPropValue initialized in the mirror (E%u P%hd)!", _PropLocation.dataSetRow().getIndex(), _PropLocation.propIndex()) );
#endif
	CMirrorPropValueBase<T>::tempStore( srcValue );
}


/*
 * Unaccess mirror (use only after init() or tempMirrorize()) and store a temporary value
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::tempRestore( const T& srcValue )
{
#ifndef FAST_MIRROR
	nlassertex( isInitialized(), ("tempRestore used on CMirrorPropValue not initialized in the mirror (E%u P%hd)!", _PropLocation.dataSetRow().getIndex(), _PropLocation.propIndex()) );
#endif
	_PropLocation.reset();
	CMirrorPropValueBase<T>::tempRestore( srcValue );
}

/*
 * Same as tempStore but no assignment is done
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::tempAllocate()
{
#ifndef FAST_MIRROR
	nlassertex( !isInitialized(), ("tempAllocate used on CMirrorPropValue initialized in the mirror (E%u P%hd)!", _PropLocation.dataSetRow().getIndex(), _PropLocation.propIndex()) );
#endif
	CMirrorPropValueBase<T>::tempAllocate();
}


/*
 * Assign a new value to the temporary value already allocated by tempStore()
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::tempReassign( const T& srcValue )
{
#ifndef FAST_MIRROR
	nlassert( CMirrorPropValueBase<T>::_Pt );
	nlassertex( !isInitialized(), ("tempReassign used on CMirrorPropValue initialized in the mirror (E%u P%hd)!", _PropLocation.dataSetRow().getIndex(), _PropLocation.propIndex()) );
#endif
	*CMirrorPropValueBase<T>::_Pt = srcValue;
}


/*
 * Return the temporary value stored
 */
template <class T, class CPropLocationClass>
const T&					CMirrorPropValue<T,CPropLocationClass>::tempValue() const
{
	return CMirrorPropValueBase<T>::tempValue();
}


/*
 * Delete the temporary value stored if it becomes useless
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::tempDelete()
{
	CMirrorPropValueBase<T>::tempDelete();
}


/*
 * Init the CMirrorPropValue<T,CPropLocationClass> object
 *  and move the temporary value stored to the new place in the mirror
 *  and delete the temporary value
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::tempMirrorize( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex )
{
#ifndef FAST_MIRROR
	nlassert( CMirrorPropValueBase<T>::_Pt );
#endif
	T *tempPt = CMirrorPropValueBase<T>::_Pt;
	init( dataSet, entityIndex, propIndex );
	*this = *tempPt;
	delete tempPt;
}


/* Init the CMirrorPropValue<T,CPropLocationClass> object (slow version)
 *  and move the temporary value stored to the new place in the mirror
 *  and delete the temporary value
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::tempMirrorize( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
#ifndef FAST_MIRROR
	nlassert( CMirrorPropValueBase<T>::_Pt );
#endif
	T *tempPt = CMirrorPropValueBase<T>::_Pt;
	init( dataSet, entityId, propName );
	*this = *tempPt;
	delete tempPt;
}



/*
 *
 */
/*template<class T, class CPropLocationClass>
T&	__dafs( CMirrorPropValue<T,CPropLocationClass>& mpv )
{
	return mpv.directAccessForStructMembers();
}*/


/*
 *
 */
/*template<class T, class CPropLocationClass>
T&	__dafs( CMirrorPropValueAlice<T,CPropLocationClass>& mpv )
{
	return mpv.directAccessForStructMembers();
}*/


/*
 *
 */
/*template<class T, class CPropLocationClass>
void __sc( CMirrorPropValue<T,CPropLocationClass>& mpv )
{
	mpv.setChanged();
}*/


/*
 *
 */
/*template<class T, class CPropLocationClass>
void __sc( CMirrorPropValueAlice<T,CPropLocationClass>& mpv )
{
	mpv.setChanged();
}
*/

/*
 * Method provided for setting the members of a struct using the macro SET_STRUCT_MEMBER (setChanged() must be called after!)
 */
template <class T, class CPropLocationClass>
T&							CMirrorPropValue<T,CPropLocationClass>::directAccessForStructMembers()
{
#ifndef FAST_MIRROR
	nlassert( CMirrorPropValueBase<T>::_Pt ); // not initialized yet?
#endif
	return *CMirrorPropValueBase<T>::_Pt;
}


/*
 * Method provided for setting the members of a struct using the macro SET_STRUCT_MEMBER
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValue<T,CPropLocationClass>::setChanged()
{
	if ( ! _PropLocation.dataSet() )
		return; // allow to use things like SET_STRUCT_MEMBER even for temp storage!

#ifdef NL_DEBUG
	nlassert( _PropLocation.dataSetRow().getIndex() < _PropLocation.dataSet()->maxNbRows() );
	if ( _PropLocation.dataSet()->getEntityId( _PropLocation.dataSetRow() ).isUnknownId() )
	{
		nlwarning( "MIRROR: Writing value of prop %hd for row %d which is empty (no CEntityId)", _PropLocation.propIndex(), _PropLocation.dataSetRow().getIndex() );
	}
	else
	{
		_PropLocation.setChanged( "<struct>" );
	}
#else
	_PropLocation.setChanged();
#endif
}


/* -------------------------------------------------------------------------------------------------------
 * CMirrorPropValueAlice
 * ----------------------------------------------------------------------------------------------------- */


/*
 * Constructor
 */
template <class T, class CPropLocationClass>
CMirrorPropValueAlice<T,CPropLocationClass>::CMirrorPropValueAlice NL_TMPL_PARAM_ON_METHOD_2(T,CPropLocationClass)( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) :
	CMirrorPropValue<T,CPropLocationClass>( dataSet, entityIndex, propIndex ), _InMirror( true )
{
	//nlinfo( "Alice %p constructed IN mirror", this );
}


/*
 * Slow constructor
 */
template <class T, class CPropLocationClass>
CMirrorPropValueAlice<T,CPropLocationClass>::CMirrorPropValueAlice NL_TMPL_PARAM_ON_METHOD_2(T,CPropLocationClass)( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
	//nlinfo( "Alice %p constructed OFF mirror then IN...", this );
	tempStore();
	init( dataSet, entityId, propName );
}


/*
 * Init (after default construction)
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValueAlice<T,CPropLocationClass>::init( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex )
{
#ifndef FAST_MIRROR
	nlassert( ! _InMirror );
#endif
	CMirrorPropValue<T,CPropLocationClass>::tempDelete();
	CMirrorPropValue<T,CPropLocationClass>::init( dataSet, entityIndex, propIndex );
	_InMirror = true;
	//nlinfo( "Alice %p now IN mirror because init", this );
}


/*
 * Change the location of a property *that is already in mirror*
 * Precondition: _InMirror
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValueAlice<T,CPropLocationClass>::changeLocation( CMirroredDataSet& dataSet, const TDataSetRow& newEntityIndex, TPropertyIndex newPropIndex )
{
#ifndef FAST_MIRROR
	nlassert( _InMirror );
#endif
	CMirrorPropValue<T,CPropLocationClass>::init( dataSet, newEntityIndex, newPropIndex );
}


/*
 * Slow init (after default construction)
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValueAlice<T,CPropLocationClass>::init( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
	CMirrorPropValue<T,CPropLocationClass>::tempDelete();
	CMirrorPropValue<T,CPropLocationClass>::init( dataSet, entityId, propName );
	_InMirror = true;
	//nlinfo( "Alice %p now IN mirror because init", this );
}


/*
 * Return true if the CMirrorPropValue<T,CPropLocationClass> object is initialized in the mirror
 */
template <class T, class CPropLocationClass>
bool						CMirrorPropValueAlice<T,CPropLocationClass>::isInitialized() const
{
	return _InMirror;

}


template <class T, class CPropLocationClass> const T& CMirrorPropValueAlice<T,CPropLocationClass>::operator() () const
{ return CMirrorPropValue<T,CPropLocationClass>::operator()(); }


/*
 * Set the value
 */
template <class T, class CPropLocationClass>
CMirrorPropValueAlice<T,CPropLocationClass>&	CMirrorPropValueAlice<T,CPropLocationClass>::operator= ( const T& srcValue )
{
	if ( _InMirror )
	{
		CMirrorPropValue<T,CPropLocationClass>::operator=( srcValue );
	}
	else
	{
		this->tempReassign( srcValue );
	}
	return *this;
}


/*
 * Serial
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValueAlice<T,CPropLocationClass>::serial( NLMISC::IStream& s )
{
	if ( s.isReading() )
	{
		T temp;
		s.serial( temp );
		(*this) = temp; // CMirrorPropValueAlice::operator= and not the one from CMirrorPropValue
	}
	else
	{
		s.serial( const_cast<T&>((*this)()) );
	}
}


/*
 * Allocate the local storage (no value assigned)
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValueAlice<T,CPropLocationClass>::tempStore()
{
#ifndef FAST_MIRROR
	nlassert( ! CMirrorPropValueBase<T>::_Pt );
#endif
	CMirrorPropValueBase<T>::_Pt = new T();
}


template <class T, class CPropLocationClass> void CMirrorPropValueAlice<T,CPropLocationClass>::tempMirrorize( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex )
{ CMirrorPropValue<T,CPropLocationClass>::tempMirrorize( dataSet, entityIndex, propIndex ); _InMirror = true; /*nlinfo( "Alice %p now IN mirror because mirrorize", this );*/ }

template <class T, class CPropLocationClass> void CMirrorPropValueAlice<T,CPropLocationClass>::tempMirrorize( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{ CMirrorPropValue<T,CPropLocationClass>::tempMirrorize( dataSet, entityId, propName ); _InMirror = true; /*nlinfo( "Alice %p now IN mirror because mirrorize", this );*/ }


/*
 * Return the timestamp of the property change
 */
template <class T, class CPropLocationClass>
NLMISC::TGameCycle			CMirrorPropValueAlice<T,CPropLocationClass>::getTimestamp() const
{
	if ( _InMirror )
		return CMirrorPropValue<T,CPropLocationClass>::getTimestamp();
	else
		return 0;
}


#ifdef STORE_CHANGE_SERVICEIDS
/*
 * Return the timestamp of the property change
 */
template <class T, class CPropLocationClass>
NLNET::TServiceId8					CMirrorPropValueAlice<T,CPropLocationClass>::getWriterServiceId() const
{
	if ( _InMirror )
		return CMirrorPropValue<T,CPropLocationClass>::getWriterServiceId();
	else
		return NLNET::TServiceId8(std::numeric_limits<uint8>::max());
}
#endif


/*
 * Destructor
 */
template <class T, class CPropLocationClass>
CMirrorPropValueAlice<T,CPropLocationClass>::~CMirrorPropValueAlice NL_TMPL_PARAM_ON_METHOD_2(T,CPropLocationClass)()
{
	//nlinfo( "Alice %p destructing (%s)", this, _InMirror?"IN":"OFF" );
	if ( ! _InMirror )
	{
		CMirrorPropValue<T,CPropLocationClass>::tempDelete();
	}
}


/* -------------------------------------------------------------------------------------------------------
 * CMirrorPropValue*CF
 * ----------------------------------------------------------------------------------------------------- */


/*
 * Constructor
 */
template <class T>
CMirrorPropValueBaseCF<T>::CMirrorPropValueBaseCF NL_TMPL_PARAM_ON_METHOD_1(T)( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) :
	CMirrorPropValueBase<T>( dataSet, entityIndex, propIndex ), _PreviousValue()
{}


/*
 * Constructor
 */
template <class T, class CPropLocationClass>
CMirrorPropValueCF<T,CPropLocationClass>::CMirrorPropValueCF NL_TMPL_PARAM_ON_METHOD_2(T,CPropLocationClass)( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) :
	CMirrorPropValue<T,CPropLocationClass>( dataSet, entityIndex, propIndex ), _PreviousValue()
{}


/*
 * Slow constructor
 */
template <class T>
CMirrorPropValueBaseCF<T>::CMirrorPropValueBaseCF NL_TMPL_PARAM_ON_METHOD_1(T)( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName ) :
	CMirrorPropValueBase<T>( dataSet, entityId, propName ), _PreviousValue()
{}


/*
 * Slow constructor
 */
template <class T, class CPropLocationClass>
CMirrorPropValueCF<T,CPropLocationClass>::CMirrorPropValueCF NL_TMPL_PARAM_ON_METHOD_2(T,CPropLocationClass)( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName ) :
	CMirrorPropValue<T,CPropLocationClass>( dataSet, entityId, propName ), _PreviousValue()
{}


/*
 * Return true if the value has been set since the latest call to resetChangeFlag()
 */
template <class T>
bool						CMirrorPropValueBaseCF<T>::hasChanged() const
{
	return ( _PreviousValue != *CMirrorPropValueBase<T>::_Pt );
}

/*
 * Return true if the value has been set since the latest call to resetChangeFlag()
 */
template <class T, class CPropLocationClass>
bool						CMirrorPropValueCF<T,CPropLocationClass>::hasChanged() const
{
	return ( _PreviousValue != *CMirrorPropValueBase<T>::_Pt );
}


/*
 * Reset the change flag
 */
template <class T>
void						CMirrorPropValueBaseCF<T>::resetChangeFlag()
{
	_PreviousValue = *CMirrorPropValueBase<T>::_Pt;
}

/*
 * Reset the change flag
 */
template <class T, class CPropLocationClass>
void						CMirrorPropValueCF<T,CPropLocationClass>::resetChangeFlag()
{
	_PreviousValue = *CMirrorPropValueBase<T>::_Pt;
}


/*
 * Return the previous value (call this method before resetChangeFlag())
 */
template <class T>
T							CMirrorPropValueBaseCF<T>::previousValue() const
{
	return _PreviousValue;
}

/*
 * Return the previous value (call this method before resetChangeFlag())
 */
template <class T, class CPropLocationClass>
T							CMirrorPropValueCF<T,CPropLocationClass>::previousValue() const
{
	return _PreviousValue;
}


/* -------------------------------------------------------------------------------------------------------
 * CMirrorPropValueList
 * ----------------------------------------------------------------------------------------------------- */


/*
 * Constructor
 */
template <class T, class CPropLocationClass>
CMirrorPropValueList<T,CPropLocationClass>::CMirrorPropValueList NL_TMPL_PARAM_ON_METHOD_2(T,CPropLocationClass)( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) :
	_PropLocation( dataSet, propIndex, entityIndex )
{
#ifdef NL_DEBUG
	nlassert( dataSet.propIsList( propIndex ) );
#endif
#ifndef FAST_MIRROR
	// This warning is disabled, because it occurs when called by killList() or clear() when clearing orphan lists.
	//if ( _PropLocation.dataSet()->getEntityId( _PropLocation.dataSetRow() ).isUnknownId() )
	//{
	//	nlwarning( "MIRROR: Accessing a value of prop %hd for row %d which is empty (no CEntityId)", _PropLocation.propIndex(), _PropLocation.dataSetRow().getIndex() );
	//}
	nlassert( (uint32)_PropLocation.dataSetRow().getIndex() < dataSet.maxNbRows() );
#endif
	dataSet.getPropPointerForList( &_PtFront, propIndex, entityIndex, (T*)NULL );
	_Container = (TSharedListCell*)(dataSet.getListCellContainer( propIndex ));
}


/*
 * Slow constructor
 */
template <class T, class CPropLocationClass>
CMirrorPropValueList<T,CPropLocationClass>::CMirrorPropValueList NL_TMPL_PARAM_ON_METHOD_2(T,CPropLocationClass)( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
	_PropLocation.init( dataSet, dataSet.getPropertyIndex( propName ), dataSet.getDataSetRow( entityId ) );
#ifdef NL_DEBUG
	nlassert( dataSet.propIsList( _PropLocation.propIndex() ) );
#endif
	dataSet.getPropPointerForList( &_PtFront, _PropLocation.propIndex(), _PropLocation.dataSetRow(), (T*)NULL );
	_Container = (TSharedListCell*)(dataSet.getListCellContainer( _PropLocation.propIndex() ));
}


template <class T, class CPropLocationClass>
bool						CMirrorPropValueList<T,CPropLocationClass>::empty() const
{
	return ((*_PtFront) == INVALID_SHAREDLIST_ROW);
}

template <class T, class CPropLocationClass>
CMirrorPropValueItem<T,CPropLocationClass>		CMirrorPropValueList<T,CPropLocationClass>::front()
{
	return CMirrorPropValueItem<T,CPropLocationClass>( this, &(_Container[*_PtFront].Value) );
}

template <class T, class CPropLocationClass>
const T&					CMirrorPropValueList<T,CPropLocationClass>::front() const
{
	return _Container[*_PtFront].Value;
}

template <class T, class CPropLocationClass>
void						CMirrorPropValueList<T,CPropLocationClass>::push_front( const T& value )
{
	// Ensure the TSharedListCell struct has 1-byte packing
	nlctassert( sizeof(TSharedListRow) + sizeof(T) == sizeof(TSharedListCell) );

#ifdef NL_DEBUG
	nlassert( ! _PropLocation.isReadOnly() );
#endif
	TSharedListRow oldfront = *_PtFront;
	TSharedListRow newfront = allocateNewCell();
	if ( newfront != INVALID_SHAREDLIST_ROW )
	{
		_Container[newfront].Next = oldfront;
		_Container[newfront].Value = value;
		*_PtFront = newfront;

#ifdef NL_DEBUG
		_PropLocation.setChanged( NLMISC::toString( "pushed %s", NLMISC::toString(value).c_str() ).c_str() );
#else
		_PropLocation.setChanged();
#endif
	}
	else
		nlwarning( "No shared list storage left (prop %hd)", _PropLocation.propIndex() );
}

template <class T, class CPropLocationClass>
void						CMirrorPropValueList<T,CPropLocationClass>::pop_front()
{
	TSharedListRow oldfront = *_PtFront;
#ifdef NL_DEBUG
	nlassert( ! _PropLocation.isReadOnly() );
#endif
#ifndef FAST_MIRROR
	nlassertex( oldfront != INVALID_SHAREDLIST_ROW, ("pop_front() on empty list") );
#endif
	_Container[oldfront].Value.~T();
	*_PtFront = _Container[*_PtFront].Next;
	releaseCell( oldfront );

#ifdef NL_DEBUG
	_PropLocation.setChanged( "popped" );
#else
	_PropLocation.setChanged();
#endif
}

extern sint32 NbAllocdListCells;
extern sint32 MaxNbAllocdListCells;

template <class T, class CPropLocationClass>
TSharedListRow										CMirrorPropValueList<T,CPropLocationClass>::allocateNewCell()
{
	/*// Old trivial linear browsing code
	TSharedListRow row = 0;
	while ( (_Container[row].Next != UNUSED_SHAREDLIST_CELL) && (row < INVALID_SHAREDLIST_ROW) )
	{
		++row;
	ListCellAllocRow = row;
	}*/

	++NbAllocdListCells; // not correct if there's no free cell (but it's just a simple debugging feature)
	if ( NbAllocdListCells > MaxNbAllocdListCells )
		MaxNbAllocdListCells = NbAllocdListCells;

	// New linked-list code
	TSharedListRow *firstFreeCell = TPropertyValueArrayHeader::getFreeCellsFront( (void*)_Container );
	TSharedListRow cellToAllocate = *firstFreeCell;
	if ( cellToAllocate != INVALID_SHAREDLIST_ROW )
		*firstFreeCell = _Container[cellToAllocate].Next;
	return cellToAllocate;
}

template <class T, class CPropLocationClass>
void												CMirrorPropValueList<T,CPropLocationClass>::releaseCell( TSharedListRow row )
{
	/*// Old trivial linear browsing code
	_Container[row].Next = UNUSED_SHAREDLIST_CELL;
	*/

	--NbAllocdListCells;
#ifdef NL_DEBUG
	nlassert( NbAllocdListCells >= 0 );
#endif

	// New linked-list code
	TSharedListRow *firstFreeCell = TPropertyValueArrayHeader::getFreeCellsFront( (void*)_Container );
	_Container[row].Next = *firstFreeCell;
	*firstFreeCell = row;
}

template <class T, class CPropLocationClass>
typename CMirrorPropValueList<T,CPropLocationClass>::size_type					CMirrorPropValueList<T,CPropLocationClass>::size() const
{
	size_type s=0;
	const_iterator it;
	for ( it=const_begin(); it!=const_end(); ++it )
	{
		++s;
	}
	return s;
}

/*
 * Return the timestamp of the property change
 */
template <class T, class CPropLocationClass>
NLMISC::TGameCycle			CMirrorPropValueList<T,CPropLocationClass>::getTimestamp() const
{
#ifndef FAST_MIRROR
	nlassert( _PropLocation.dataSet() );
#endif
	return _PropLocation.dataSet()->getChangeTimestamp( _PropLocation.propIndex(), _PropLocation.dataSetRow() );
}



template <class T, class CPropLocationClass>
typename CMirrorPropValueList<T,CPropLocationClass>::size_type					CMirrorPropValueList<T,CPropLocationClass>::max_size() const
{
	return INVALID_SHAREDLIST_ROW; //TEMP (less the size of other lists!)
}


/*
 * Ensures the size of the list is n:
 * - If the requested size is greater than the previous size, new elements are pushed at front
 * - If the requested size is smaller than the previous size, elements are popped at front
 */
template <class T, class CPropLocationClass>
void												CMirrorPropValueList<T,CPropLocationClass>::resize( size_type n, T t )
{
	size_type prevSize = size();
	if ( n > prevSize )
	{
		for ( size_type i=0; i!=n-prevSize; ++i )
			push_front( t );
	}
	else
	{
		for ( size_type i=0; i!=prevSize-n; ++i )
			pop_front();
	}
}


/*
 *
 */
template <class T, class CPropLocationClass>
void												CMirrorPropValueList<T,CPropLocationClass>::testList( uint expectedSize ) const
{
	size_type s=0;
	const_iterator it;
	for ( it=const_begin(); it!=const_end(); ++it )
	{
		++s;
		if ( s > expectedSize )
			nlwarning( "List size > expected size" );
	}
	if ( s < expectedSize )
		nlwarning( "List size < expected size" );
}


template <class T, class CPropLocationClass>
void												CMirrorPropValueList<T,CPropLocationClass>::clear( bool hidePropChange )
{
#ifdef NL_DEBUG
	nlassert( ! _PropLocation.isReadOnly() );
#endif
	TSharedListRow row = *_PtFront;
	*_PtFront = INVALID_SHAREDLIST_ROW;
	while ( row != INVALID_SHAREDLIST_ROW )
	{
		TSharedListRow oldrow = row;
		row = _Container[oldrow].Next;
		_Container[oldrow].Value.~T();
		releaseCell( oldrow );
	}

	if ( ! hidePropChange )
	{
#ifdef NL_DEBUG
		_PropLocation.setChanged( "cleared" );
#else
		_PropLocation.setChanged();
#endif
	}
}

template <class T, class CPropLocationClass>
typename CMirrorPropValueList<T,CPropLocationClass>::iterator					CMirrorPropValueList<T,CPropLocationClass>::erase( typename CMirrorPropValueList<T,CPropLocationClass>::iterator pos )
{
#ifdef NL_DEBUG
	nlassert( ! _PropLocation.isReadOnly() );
#endif
	TSharedListRow oldrow = pos._Index;
	_Container[oldrow].Value.~T();

	if ( oldrow == *_PtFront )
	{
		*_PtFront = _Container[*_PtFront].Next;
		pos._Index = *_PtFront;
	}
	else
	{
		TSharedListRow row = *_PtFront;
		while ( _Container[row].Next != oldrow )
			row = _Container[row].Next;
		_Container[row].Next = _Container[oldrow].Next;
		pos._Index = _Container[row].Next;
	}

	releaseCell( oldrow );
#ifdef NL_DEBUG
	_PropLocation.setChanged( "erased" );
#else
	_PropLocation.setChanged();
#endif
	return pos; // returns the element following the one that was erased
}

template <class T, class CPropLocationClass>
typename CMirrorPropValueList<T,CPropLocationClass>::iterator					CMirrorPropValueList<T,CPropLocationClass>::erase_after( typename CMirrorPropValueList<T,CPropLocationClass>::iterator pos )
{
	TSharedListRow after = _Container[pos._Index].Next;
	_Container[after].Value.~T();
	_Container[pos._Index].Next = _Container[after].Next;
	releaseCell( after );
	pos._Index = _Container[pos._Index].Next;
#ifdef NL_DEBUG
	_PropLocation.setChanged( "erased after" );
#else
	_PropLocation.setChanged();
#endif
	return pos; // returns the element following the one that was erased
}

template <class T, class CPropLocationClass>
typename CMirrorPropValueList<T,CPropLocationClass>::iterator					CMirrorPropValueList<T,CPropLocationClass>::begin()
{
	iterator it;
	it._ParentList = this;
	it._Index = *_PtFront;
	return it;
}

template <class T, class CPropLocationClass>
typename CMirrorPropValueList<T,CPropLocationClass>::iterator					CMirrorPropValueList<T,CPropLocationClass>::end()
{
	iterator it;
	it._ParentList = this;
	it._Index = INVALID_SHAREDLIST_ROW;
	return it;
}

template <class T, class CPropLocationClass>
typename CMirrorPropValueList<T,CPropLocationClass>::const_iterator				CMirrorPropValueList<T,CPropLocationClass>::const_begin() const
{
	const_iterator it;
	it._ParentList = const_cast<CMirrorPropValueList<T,CPropLocationClass>*>(this);
	it._Index = *_PtFront;
	return it;
}

template <class T, class CPropLocationClass>
typename CMirrorPropValueList<T,CPropLocationClass>::const_iterator				CMirrorPropValueList<T,CPropLocationClass>::const_end() const
{
	const_iterator it;
	it._ParentList = const_cast<CMirrorPropValueList<T,CPropLocationClass>*>(this);
	it._Index = INVALID_SHAREDLIST_ROW;
	return it;
}

/// Get the value
template <class T, class CPropLocationClass>
const T&											CMirrorPropValueItem<T,CPropLocationClass>::operator() () const
{
	return *_Pt;
}

/// Set the value
template <class T, class CPropLocationClass>
CMirrorPropValueItem<T,CPropLocationClass>&			CMirrorPropValueItem<T,CPropLocationClass>::operator= ( const T& srcValue )
{
#ifdef NL_DEBUG
	nlassert( ! _ParentList->_PropLocation.isReadOnly() );
#endif

	*_Pt = srcValue;
#ifdef NL_DEBUG
	_ParentList->_PropLocation.setChanged( NLMISC::toString( "assigned an item to %s", NLMISC::toString( srcValue ).c_str() ) );
#else
	_ParentList->_PropLocation.setChanged();
#endif
	return *this;
}



#endif // NL_MIRROR_PROP_VALUE_INLINE_H

/* End of mirror_prop_value.h */
