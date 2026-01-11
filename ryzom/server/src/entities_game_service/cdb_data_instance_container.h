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



#ifndef NL_CDB_DATA_INSTANCE_CONTAINER_H
#define NL_CDB_DATA_INSTANCE_CONTAINER_H

#include "server_share/fixed_size_int_vector.h"
#include "cdb_struct_banks.h"


/**
 *
 */
class CCDBChangeTracker
{
protected:

	friend class CCDBDataInstanceContainer;

	/// Constructor
	CCDBChangeTracker() : _FirstChanged(CDB_LAST_CHANGED), _LastChanged( CDB_INVALID_DATA_INDEX), _ChangedCount(0) {}

	/// Init
	void	init( TCDBDataIndex size ) { _TrackVec.init( size, CDB_INVALID_DATA_INDEX ); }

	/// Record a change (push)
	inline void		recordChange( TCDBDataIndex index );

	/// Record a change (push) in the backward-linnked-list which entry-point is subtrackerLast
	inline void		recordChangeSublist( TCDBDataIndex& subtrackerLast, TCDBDataIndex index );
	
	/// Get the entity index of the first changed. Returns CDB_LAST_CHANGED if there is no change.
	TCDBDataIndex	getFirstChanged() const { return _FirstChanged; }

	/// Get the entity index of the next changed (assumes dataIndex is valid). Returns LAST_CHANGED if there is no more change.
	TCDBDataIndex	getNextChanged( TCDBDataIndex index ) const { return _TrackVec[index]; }

	/// Pop the first change out of the tracker. Do not call if getFirstChanged() returned CDB_LAST_CHANGED.
	void			popFirstChanged()
	{
#ifdef NL_DEBUG
		nlassert( _FirstChanged != CDB_LAST_CHANGED );
#endif
		TCDBDataIndex *queueFront = &(_TrackVec[_FirstChanged]);
		_FirstChanged = *queueFront;
		*queueFront = CDB_INVALID_DATA_INDEX;

		--_ChangedCount;
	}

	/// Pop the last change out of the subtracker, or return CDB_INVALID_DATA_INDEX if no more changes
	TCDBDataIndex	popChangedIndexInAtomSubstrackerLast( TCDBDataIndex& subtrackerLast )
	{ 
		if ( subtrackerLast == CDB_LAST_CHANGED )
		{
			return subtrackerLast;
		}
		else
		{
			TCDBDataIndex lastChange = subtrackerLast;
			subtrackerLast = _TrackVec[lastChange]; // set the new queue end to the previous of 'last'
			_TrackVec[lastChange] = CDB_INVALID_DATA_INDEX;
			return lastChange;
		}
	}

	/// Finish cleaning the change tracker to allow adding new changes when all changes have been popped by popFirstChanged()
	void			quickCleanChanges()
	{
		if ( _FirstChanged == CDB_LAST_CHANGED )
		{
			_LastChanged = CDB_INVALID_DATA_INDEX;
		}
	}

	/// Return true if the specified property is marked as changed (no bound check)
	bool			isChanged( TCDBDataIndex index ) const { return (_TrackVec[index] != CDB_INVALID_DATA_INDEX); }

	/// Return the number of changes
	uint			getChangedPropertyCount() const { return _ChangedCount; }

private:

	/**
	 * Storage for the main tracker and all subtrackers, which are
	 * single-linked list implemented by this array of the same size as _DataArray.
	 *
	 * For the main tracker (forward-linking):
	 * Each changed item either points to the next changed or is the value CDB_LAST_CHANGED.
	 * If the item state is 'unchanged', the value is CDB_INVALID_DATA_INDEX.
	 *
	 * For a subtracker (backward-linking):
	 * Each changed item either points to the previous changed or is the value CDB_INVALID_DATA_INDEX.
	 * If the item state is 'unchanged', the value is CDB_INVALID_DATA_INDEX.
	 */
	CFixedSizeIntVector<TCDBDataIndex>	_TrackVec;

	/** First changed item (useful to read the changed from the beginning).
	 * When there is no change to read, First equals CDB_LAST_CHANGED.
	 */
	TCDBDataIndex				_FirstChanged;

	/** Last changed item (useful to link the last changed to a newly changed item).
	 * When there is no change yet, Last equals CDB_INVALID_DATA_INDEX.
	 */
	TCDBDataIndex				_LastChanged;

	/// Number of changes still to pop
	sint						_ChangedCount;
};


/**
 * There are two levels of trackers. The main tracker is accessed with getFirstChanged(),
 * popFirstChanged(). It is a forward-linked-list, to ensure the changes are popped in
 * the same order as they were recorded.
 * You can have several subtrackers (one for each atom group), they are backward-linked-lists
 * (because all changes in them are always popped in one shot for atomic sending). The last
 * recorded index is stored in the data array at the position of the atom group index.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CCDBDataInstanceContainer
{
public:

	/// Initialization
	void			init( TCDBDataIndex size, bool usePermanentTracker  );

	/// Init/reset the atom group subtracker at index (must be called for every index which corresponding node is an atom branch)
	void			resetSubTracker( TCDBDataIndex index ) { _DataArray[index] = CDB_LAST_CHANGED; }

	/// Get the value of the property (no bound check)
	inline sint64	getValue64( TCDBDataIndex index ) const { return _DataArray[index]; }

	/// Set the value of the property (no bound check, sets the change flag if value different than previous or forceSending is true).
	bool			setValue64( TCDBDataIndex index, sint64 value, bool forceSending );
	
	/// Set the value of the property (no bound check, does NOT modify the change flag).
	void			setValue64NoChange( TCDBDataIndex index, sint64 value ) { _DataArray[index] = value; }

	/// Set the value of the property (no bound check, set the change flag in an atom group if value different than previous or forceSending is true)
	bool			setValue64InAtom( TCDBDataIndex index, sint64 value, TCDBDataIndex atomGroupIndex, bool forceSending );

	/// Copy the current value to the last sent value of the property (no bound check)
	//void			archiveCurrentValue( TCDBDataIndex index ) { _LastSentDataArray[index] = _DataArray[index]; }

	/// Get the difference (delta) between the last sent value and the current one (no bound check)
	//sint64		getDeltaFromArchive( TCDBDataIndex index ) { return _DataArray[index] - _LastSentDataArray[index]; }

	/// Set the value of the property (no bound check, does NOT modify the change flag).
	/*inline sint32	getValue32( TCDBDataIndex index ) const { return *((sint32*)&(_DataArray[index])); }
	void			setValue32( TCDBDataIndex index, sint32 value ) { setValue64( index, (sint64)value ); }
	inline sint16	getValue16( TCDBDataIndex index ) const { return *((sint16*)&(_DataArray[index])); }
	void			setValue16( TCDBDataIndex index, sint16 value ) { setValue64( index, (sint64)value ); }
	inline sint8	getValue8( TCDBDataIndex index ) const { return *((sint8*)&(_DataArray[index])); }
	void			setValue8( TCDBDataIndex index, sint8 value ) { setValue64( index, (sint64)value ); }
	inline bool		getValueBool( TCDBDataIndex index ) const { return ((_DataArray[index])!=(sint64)0 ); }
	void			setValueBool( TCDBDataIndex index, bool value ) { setValue64( index, (sint64)value ); }*/

	/// Return true if the specified index is valid in the container
	bool			checkIndex( TCDBDataIndex index ) const { return (index >= 0) && (index < (TCDBDataIndex)_DataArray.size()); }

	/// Get the entity index of the first changed. Returns CDB_LAST_CHANGED if there is no change.
	TCDBDataIndex	getFirstChanged() const { return _ChangeTracker.getFirstChanged(); }

	/// Get the entity index of the next changed (assumes dataIndex is valid). Returns LAST_CHANGED if there is no more change.
	TCDBDataIndex	getNextChanged( TCDBDataIndex index ) const { return _ChangeTracker.getNextChanged( index ); }

	/// Pop the first change out of the tracker. Do not call if getFirstChanged() returned CDB_LAST_CHANGED.
	void			popFirstChanged() { _ChangeTracker.popFirstChanged(); }
	
	/// Pop the last change out of the subtracker referenced at the specified index in the data array, or return CDB_INVALID_DATA_INDEX if no more changes
	TCDBDataIndex	popChangedIndexInAtom( TCDBDataIndex atomGroupIndex )
	{
		TCDBDataIndex& subtrackerLast = (TCDBDataIndex&)(_DataArray[atomGroupIndex]);
		return _ChangeTracker.popChangedIndexInAtomSubstrackerLast( subtrackerLast );
	}

	/// Debug
	void			displayAtomChanges( TCDBDataIndex atomGroupIndex );

	/// Finish cleaning the change tracker to allow adding new changes when all changes have been popped by popFirstChanged()
	void			quickCleanChanges() { _ChangeTracker.quickCleanChanges(); }

	/// Return true if the specified property is marked as changed (no bound check)
	bool			isChanged( TCDBDataIndex index ) const { return _ChangeTracker.isChanged( index ); }

	/// Return the number of changes
	uint			getChangedPropertyCount() const { return _ChangeTracker.getChangedPropertyCount(); }

	/// Get the entity index of the first changed. Returns CDB_LAST_CHANGED if there is no change.
	TCDBDataIndex	getPermanentFirstChanged() const { return _PermanentTracker.getFirstChanged(); }

	/// Get the entity index of the next changed (assumes dataIndex is valid). Returns LAST_CHANGED if there is no more change.
	TCDBDataIndex	getPermanentNextChanged( TCDBDataIndex index ) const { return _PermanentTracker.getNextChanged( index ); }

	/// Return true if the specified property is marked as changed (no bound check)
	bool			isPermanentChanged( TCDBDataIndex index ) const { return _PermanentTracker.isChanged( index ); }

	/// Return the number of changes
	uint			getPermanentChangedPropertyCount() const { return _PermanentTracker.getChangedPropertyCount(); }

private:

	/// Array of property values, indexed by TCDBDataIndex
	CFixedSizeIntVector<sint64>	_DataArray;

	/// Array of the last sent property values, indexed by TCDBDataIndex
	//std::vector<sint64>		_LastSentDataArray;

	/// Regular change tracker
	CCDBChangeTracker			_ChangeTracker;

	/// Additional tracker for changes since the beginning (optional)
	CCDBChangeTracker			_PermanentTracker;

	/// Enable additionnal tracker
	bool						_UsePermanentTracker;
};


#endif // NL_CDB_DATA_INSTANCE_CONTAINER_H

/* End of cdb_data_instance_container.h */
