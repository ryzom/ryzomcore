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



#ifndef NL_PROPERTY_ALLOCATOR_CLIENT_H
#define NL_PROPERTY_ALLOCATOR_CLIENT_H

#include "nel/misc/types_nl.h"

#include "property_allocator.h"

namespace NLNET
{
	class CMessage;
};

/**
 * Client-side property array allocator for shared memory
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CPropertyAllocatorClient : public CPropertyAllocator
{
public:

	/// Constructor
	CPropertyAllocatorClient() : _LocalMSId((uint16)~0) {}

	/** Ask to allocate, if not done yet, a segment for the specified property.
	 * The pointer will be soon returned by getPropertySegment(), but not always
	 * immediately (getPropertySegment() will return NULL when the information
	 * is not ready yet).
	 */
	void		allocProperty( const std::string& propName, TPropSubscribingOptions options, const std::string notifyGroupByPropName );

	/// Unallocate property
	void		unallocProperty( const std::string& propName );

	/// Set the local mirror service id (~0 for none)
	void		setLocalMirrorServiceId( NLNET::TServiceId serviceId )
				{ _LocalMSId = serviceId; }

	/// Return the local mirror service id
	NLNET::TServiceId		mirrorServiceId() const { return _LocalMSId; }

	///
	void		serialOutMirrorInfo( NLNET::CMessage& msgout );

protected:

	friend class CMirror;
	friend class CMirroredDataSet;

	/// Access a shared property segment
	void		*accessPropertySegment( const std::string& propName, sint32 smid );

	/// Access a shared property segment if this service did not subscribed to it (return NULL if already accessed)
	void		*accessOtherPropertySegment( sint32 smid );

	friend void cbRecvSMIdToAccessPropertySegment( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

private:

	NLNET::TServiceId		_LocalMSId;
};


#endif // NL_PROPERTY_ALLOCATOR_CLIENT_H

/* End of property_allocator_client.h */
