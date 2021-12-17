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



#ifndef NL_PROPERTY_ALLOCATOR_H
#define NL_PROPERTY_ALLOCATOR_H

#include "nel/misc/types_nl.h"
#include "base_types.h"

const std::string RANGE_MANAGER_SERVICE = "TICKS";

class CMirroredDataSet;


/**
 * Base class for allocation information about a property
 */
template <class TDS>
struct TPropertyInfoBase
{
	/// Constructor
	TPropertyInfoBase( TDS *ds, TPropertyIndex propindex ) : Segment(NULL), DataSet(ds), PropertyIndex(propindex), SMId(-1) {}

	/// Where the property array is allocated
	void				*Segment;

	/// To which dataset it belongs
	TDS					*DataSet;

	/// With which property index
	TPropertyIndex		PropertyIndex;

	/// Shared memory id
	sint32				SMId;

	/// Return true if the property is allocated
	bool				allocated() const { return Segment != NULL; }

	/// Serial ids
	void				serial( NLMISC::IStream& s )
	{
		s.serial( PropertyIndex );
		s.serial( SMId );
	}
};


#define GET_PROPERTY_INFO(it) ((*it).second)


/**
 * Map of CPropertyInfoBase
 */
template <class TPI, class TDS>
class TMapOfPropertyInfoBase : public CHashMap< std::string, TPI>
{
public:

	/// Return the dataset corresponding to a property (or NULL if not found)
	TDS		*getDataSetByPropName( const std::string& propName, TPropertyIndex& propIndex )
	{
		typename CHashMap< std::string, TPI>::iterator ipim = this->find( propName );
		if ( ipim != this->end() )
		{
			propIndex = GET_PROPERTY_INFO(ipim).PropertyIndex;
			return GET_PROPERTY_INFO(ipim).DataSet;
		}
		else
		{
			return NULL;
		}
	}
};


enum TPropertyDebugFlag { FlagReadOnly=0, FlagWriteOnly=1, FlagNotifyChanges=3, FlagMonitorAssignment=4, FlagGroupNotifiedByOtherProp=5 };


/**
 * Property info for client-side mirror system
 */
struct TPropertyInfo : public TPropertyInfoBase< CMirroredDataSet >
{
	/// Constructor
	TPropertyInfo( CMirroredDataSet *ds=NULL, TPropertyIndex propindex=INVALID_PROPERTY_INDEX ) : TPropertyInfoBase<CMirroredDataSet>(ds, propindex), Pending(false), PropTrackersPending(false), _PropertyDebugFlags(0), _PropNotifyingTheGroup(INVALID_PROPERTY_INDEX) {}

	// True if allocation of the property is being processed (property not allocated yet)
	bool				Pending;

	// True if the property trackers list has not been received yet
	bool				PropTrackersPending;

	bool				flagReadOnly() const { return (_PropertyDebugFlags & PSOReadOnly)!=0; }
	bool				flagWriteOnly() const { return (_PropertyDebugFlags & PSOWriteOnly)!=0; }
	bool				flagNotifyChanges() const { return (_PropertyDebugFlags & PSONotifyChanges)!=0; }
	bool				flagMonitorAssignment() const { return (_PropertyDebugFlags & PSOMonitorAssignment)!=0; }
	bool				flagGroupNotifiedByOtherProp() const { return (_PropertyDebugFlags & (1<<FlagGroupNotifiedByOtherProp))!=0; }
	TPropertyIndex		propNotifyingTheGroup() const { return _PropNotifyingTheGroup; }
	std::string			getFlagsString() const;
	void				setFlags( TPropSubscribingOptions options, bool groupNotif ) { _PropertyDebugFlags = options | (groupNotif<<FlagGroupNotifiedByOtherProp); }
	void				setFlagReadOnly( bool b ) { _PropertyDebugFlags |= (b<<FlagReadOnly); }
	void				setFlagNotifyChanges( bool b ) { _PropertyDebugFlags |= (b<<FlagNotifyChanges); }
	void				setFlagMonitorAssignment( bool b ) { _PropertyDebugFlags |= (b<<FlagMonitorAssignment); }
	void				setFlagGroupNotifiedByOtherProp( bool b, TPropertyIndex otherProp ) { _PropertyDebugFlags |= (b<<FlagGroupNotifiedByOtherProp); _PropNotifyingTheGroup = otherProp; }

	// Serial
	void				serial( NLMISC::IStream& s )
	{
		TPropertyInfoBase<CMirroredDataSet>::serial( s );
		s.serial( _PropertyDebugFlags );
		s.serial( _PropNotifyingTheGroup );
	}

private:

	/// Flags used especially in debug mode (user options + group notification)
	TPropSubscribingOptions	_PropertyDebugFlags;

	/// Index of another prop notifying this prop, if flagGroupNotifiedByOtherProp() (note: in the same dataset)
	TPropertyIndex			_PropNotifyingTheGroup;
};

typedef TMapOfPropertyInfoBase< TPropertyInfo, CMirroredDataSet > TPropertiesInMirror;


/**
 * Base class for property array allocation
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CPropertyAllocator
{
public:

	/** Ask to allocate, if not done yet, a segment for the specified property.
	 * The pointer will be soon returned by getPropertySegment(), but not always
	 * immediately (getPropertySegment() will return NULL when the information
	 * is not ready yet).
	 */
	void		allocProperty( const std::string& propName ) {}

	/// Unallocate property
	void		unallocProperty( const std::string& propName ) {}

	/// Return a pointer to the property, or NULL if it is not ready yet (try again)
	void		*getPropertySegment( const std::string& propName ) const;

protected:

	/// Properties in the local _mirror
	TPropertiesInMirror		_PropertiesInMirror;

};


#endif // NL_PROPERTY_ALLOCATOR_H

/* End of property_allocator.h */
