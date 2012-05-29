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
#include "mirror_prop_value.h"


sint32 NbAllocdListCells = 0;
sint32 MaxNbAllocdListCells = 0;
NLMISC_CATEGORISED_VARIABLE(mirror, sint32, NbAllocdListCells, "Current number of allocated rows of mirror lists by this service" );
NLMISC_CATEGORISED_VARIABLE(mirror, sint32, MaxNbAllocdListCells, "Max number of allocated rows of mirror lists by this service simultaneously" );



/// Constructor
CPropLocationUnpacked::CPropLocationUnpacked( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex )
{
	init( dataSet, propIndex, entityIndex );
}


#ifdef NL_DEBUG

/*
 * Set changed, test if row is valid, and do monitoring
 */
void				CPropLocationUnpacked::setChanged( const char *valueStr )
{
	// Always done, because can be called even before the entityId is set (e.g. for an initial property from another machine)
	_DataSet->setChanged( _DataSetRow, _PropIndex );
	monitorValue( valueStr );
}

#endif


/*
 * Return the location as a string
 */
std::string			CPropLocationUnpacked::toString() const
{
	return NLMISC::toString( "%s/E%u P%hd", _DataSet ? _DataSet->name().c_str() : "NULL", _DataSetRow.getIndex(), _PropIndex );
}


/*
 * Return a string representing the datasetrow
 */
std::string			TDataSetRow::toString() const
{
	return NLMISC::toString( "E%u_%hu", getIndex(), counter() );
}


// ---------------------------------------------------------------------------------------------------------------------------------------

#if NL_ISO_SYNTAX

/*
 * Constructor
 */
CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::CMirrorPropValueList( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) :
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
	dataSet.getPropPointerForList( &_PtFront, propIndex, entityIndex, (NLMISC::CEntityId*)NULL );
	_Container = (TSharedListCell*)(dataSet.getListCellContainer( propIndex ));
}

/*
 * Slow constructor
 */

CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::CMirrorPropValueList( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName )
{
	_PropLocation.init( dataSet, dataSet.getPropertyIndex( propName ), dataSet.getDataSetRow( entityId ) );
#ifdef NL_DEBUG
	nlassert( dataSet.propIsList( _PropLocation.propIndex() ) );
#endif
	dataSet.getPropPointerForList( &_PtFront, _PropLocation.propIndex(), _PropLocation.dataSetRow(), (NLMISC::CEntityId*)NULL );
	_Container = (TSharedListCell*)(dataSet.getListCellContainer( _PropLocation.propIndex() ));
}

bool						CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::empty() const
{
	return ((*_PtFront) == INVALID_SHAREDLIST_ROW);
}


CMirrorPropValueItem<NLMISC::CEntityId,CPropLocationUnpacked>		CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::front()
{
	return CMirrorPropValueItem<NLMISC::CEntityId,CPropLocationUnpacked>( this, &(_Container[*_PtFront].Value) );
}

NLMISC::CEntityId					CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::front() const
{
	return NLMISC::CEntityId(_Container[*_PtFront].Value);
}

void						CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::push_front( const NLMISC::CEntityId& value )
{
	// Ensure the TSharedListCell struct has 1-byte packing
	nlctassert( sizeof(TSharedListRow) + sizeof(uint64) == sizeof(TSharedListCell) );

#ifdef NL_DEBUG
	nlassert( ! _PropLocation.isReadOnly() );
#endif
	TSharedListRow oldfront = *_PtFront;
	TSharedListRow newfront = allocateNewCell();
	if ( newfront != INVALID_SHAREDLIST_ROW )
	{
		_Container[newfront].Next = oldfront;
		_Container[newfront].Value = value.getRawId();
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

void						CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::pop_front()
{
	TSharedListRow oldfront = *_PtFront;
#ifdef NL_DEBUG
	nlassert( ! _PropLocation.isReadOnly() );
#endif
#ifndef FAST_MIRROR
	nlassertex( oldfront != INVALID_SHAREDLIST_ROW, ("pop_front() on empty list") );
#endif
	(*((NLMISC::CEntityId*)(&_Container[oldfront].Value))).~CEntityId();
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


TSharedListRow										CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::allocateNewCell()
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

void												CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::releaseCell( TSharedListRow row )
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

CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::size_type					CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::size() const
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
NLMISC::TGameCycle			CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::getTimestamp() const
{
#ifndef FAST_MIRROR
	nlassert( _PropLocation.dataSet() );
#endif
	return _PropLocation.dataSet()->getChangeTimestamp( _PropLocation.propIndex(), _PropLocation.dataSetRow() );
}


CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::size_type					CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::max_size() const
{
	return INVALID_SHAREDLIST_ROW; //TEMP (less the size of other lists!)
}


/*
 * Ensures the size of the list is n:
 * - If the requested size is greater than the previous size, new elements are pushed at front
 * - If the requested size is smaller than the previous size, elements are popped at front
 */
void												CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::resize( size_type n, NLMISC::CEntityId t )
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
void												CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::testList( uint expectedSize ) const
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


void												CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::clear( bool hidePropChange )
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
		(*((NLMISC::CEntityId*)(&_Container[oldrow].Value))).~CEntityId();
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

CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::iterator					CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::erase( CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::iterator pos )
{
#ifdef NL_DEBUG
	nlassert( ! _PropLocation.isReadOnly() );
#endif
	TSharedListRow oldrow = pos._Index;
	(*((NLMISC::CEntityId*)(&_Container[oldrow].Value))).~CEntityId();

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

CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::iterator					CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::erase_after( CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::iterator pos )
{
	TSharedListRow after = _Container[pos._Index].Next;
	(*((NLMISC::CEntityId*)(&_Container[after].Value))).~CEntityId();
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

CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::iterator					CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::begin()
{
	iterator it;
	it._ParentList = this;
	it._Index = *_PtFront;
	return it;
}

CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::iterator					CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::end()
{
	iterator it;
	it._ParentList = this;
	it._Index = INVALID_SHAREDLIST_ROW;
	return it;
}

CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::const_iterator				CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::const_begin() const
{
	const_iterator it;
	it._ParentList = const_cast<CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>*>(this);
	it._Index = *_PtFront;
	return it;
}

CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::const_iterator				CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>::const_end() const
{
	const_iterator it;
	it._ParentList = const_cast<CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>*>(this);
	it._Index = INVALID_SHAREDLIST_ROW;
	return it;
}

/// Get the value (ugly workaround to the packing limitation of GCC 3.4)
const NLMISC::CEntityId&											CMirrorPropValueItem<NLMISC::CEntityId,CPropLocationUnpacked>::operator() () const
{
  uint64 rawId = *_Pt;
  nlctassert( sizeof(NLMISC::CEntityId) == sizeof(uint64) );
  const NLMISC::CEntityId *placementEid = new (_Pt) NLMISC::CEntityId(rawId);
  return *placementEid;
}

/// Set the value
CMirrorPropValueItem<NLMISC::CEntityId,CPropLocationUnpacked>&			CMirrorPropValueItem<NLMISC::CEntityId,CPropLocationUnpacked>::operator= ( const NLMISC::CEntityId& srcValue )
{
#ifdef NL_DEBUG
	nlassert( ! _ParentList->_PropLocation.isReadOnly() );
#endif

	*_Pt = srcValue.getRawId();
#ifdef NL_DEBUG
	_ParentList->_PropLocation.setChanged( NLMISC::toString( "assigned an item to %s", NLMISC::toString( srcValue ).c_str() ).c_str() );
#else
	_ParentList->_PropLocation.setChanged();
#endif
	return *this;
}

#endif
