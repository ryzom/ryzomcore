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



#ifndef NL_CHANGE_TRACKER_CLIENT_H
#define NL_CHANGE_TRACKER_CLIENT_H

#include "nel/misc/types_nl.h"
#include "change_tracker_base.h"


const uint32 INVALID_TRACKER = ~0;


/**
 * Change tracker specialized for the client part of the mirror system
 * (accessing shared memory but not creating it).
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CChangeTrackerClient : public CChangeTrackerBase
{
public:

	CChangeTrackerClient() : CChangeTrackerBase(), _CurrentNotifiedRow(), _LastNotifiedRow(), _PointingToGroupTracker(false) {}

	/// Access the tracker header and item array (shared memory)
	void					access( sint32 shmid );

	/// Release the tracker (unaccess shared memory + release mutex)
	void					release();

	/// Instead of accessing an smid, use another tracker's data (set it after operator=())
	void					setPointingToGroupTracker() { _PointingToGroupTracker = true; }

	/// Return true if the tracker is not allocating for itself, using the data of another tracker
	bool					isPointingToGroupTracker() const { return _PointingToGroupTracker; }

	/// Assignment operator
	CChangeTrackerClient& operator= ( const CChangeTrackerClient& src )
	{
		CChangeTrackerBase::operator=( src );
		_CurrentNotifiedRow = src._CurrentNotifiedRow;
		_LastNotifiedRow = src._LastNotifiedRow;
		return *this;
	}

	/// Serial ids
	void					serial( NLMISC::IStream& s )
	{
		CChangeTrackerBase::serial( s );
		s.serial( _PointingToGroupTracker );
	}

private:

	/// The current row that is being notified
	TDataSetRow				_CurrentNotifiedRow;

	/// The last row that has been notified (acknowledged)
	TDataSetRow				_LastNotifiedRow;

	/// True if the tracker is not allocating for itself, using the data of another tracker
	bool					_PointingToGroupTracker;
};



/**
 * Change tracker with property index
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CChangeTrackerClientProp : public CChangeTrackerClient
{
public:

	CChangeTrackerClientProp() : CChangeTrackerClient(), _PropIndex( INVALID_PROPERTY_INDEX ) {}

	CChangeTrackerClientProp( TPropertyIndex propIndex ) : CChangeTrackerClient(), _PropIndex( propIndex ) {}

	TPropertyIndex			propIndex() const { return _PropIndex; }

	void					serial( NLMISC::IStream& s )
	{
		CChangeTrackerClient::serial( s );
		s.serial( _PropIndex );
	}

private:

	TPropertyIndex			_PropIndex;
};


#endif // NL_CHANGE_TRACKER_CLIENT_H

/* End of change_tracker_client.h */
