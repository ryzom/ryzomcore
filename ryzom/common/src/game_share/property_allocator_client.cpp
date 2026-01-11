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
#include "property_allocator_client.h"
#include "mirrored_data_set.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


/* Ask to allocate, if not done yet, a segment for the specified property.
 * The pointer will be soon returned by getPropertySegment(), but not always
 * immediately (getPropertySegment() will return NULL when the information
 * is not ready yet).
 */
void		CPropertyAllocatorClient::allocProperty(
				const std::string& propName, TPropSubscribingOptions options,
				const std::string notifyGroupByPropName )
{
	// TODO: check if simple property or STL container (special case with no shared memory)

	// Check if the property must be allocated
	TPropertiesInMirror::iterator ipim = _PropertiesInMirror.find( propName );
	if ( ipim != _PropertiesInMirror.end() )
	{
		TPropertyInfo& propinfo = GET_PROPERTY_INFO(ipim);
		if ( ! (propinfo.allocated() || propinfo.Pending) )
		{
			// Send alloc request to the local Mirror Service
			if ( _LocalMSId != TServiceId(std::numeric_limits<uint16>::max()) )
			{
				// Check options
				if ( (options & PSOReadOnly) && (options & PSOWriteOnly) )
				{
					nlwarning( "MIRROR: Property %s declared with both RO and WO options, keeping RO", propName.c_str() );
					options &= ~PSOWriteOnly;
				}
				if ( (options & PSOWriteOnly) && (options & PSONotifyChanges) )
				{
					nlwarning( "MIRROR: Property %s declared with both WO and notification, keeping notification", propName.c_str() );
					options &= ~PSOWriteOnly;
				}

				// Send msg to the MS
				CMessage msgout( "AP" );
				msgout.serial( const_cast<std::string&>(propName) );
				msgout.serial( options );
				msgout.serial( const_cast<std::string&>(notifyGroupByPropName) );
				CUnifiedNetwork::getInstance()->send( _LocalMSId, msgout );

				// Set pending state in properties map
				propinfo.Pending = true;
				if ( (! ((options & PSOReadOnly) || (options && PSOWriteOnly))) || (options & PSONotifyChanges)) // we don't expect to receive a prop trackers list if readonly && !notify
					propinfo.PropTrackersPending = true;

				// Set debug flags
				propinfo.setFlags( options, false );
				if ( ! notifyGroupByPropName.empty() )
				{
					TPropertiesInMirror::iterator ipimGroup = _PropertiesInMirror.find( notifyGroupByPropName );
					if ( ipimGroup != _PropertiesInMirror.end() )
					{
						if ( GET_PROPERTY_INFO(ipimGroup).DataSet != propinfo.DataSet )
						{
							nlwarning( "MIRROR: The group property %s does not belong to the same dataset as the declared property %s!", notifyGroupByPropName.c_str(), propName.c_str() );
						}
						else if ( notifyGroupByPropName != propName )
						{
							propinfo.setFlagGroupNotifiedByOtherProp( true, GET_PROPERTY_INFO(ipimGroup).PropertyIndex );
						}
					}
					else
					{
						nlwarning( "MIRROR: Invalid group property name %s", notifyGroupByPropName.c_str() );
					}
				}
			}
		}
	}
	else
	{
		nlwarning( "MIRROR: Invalid property name %s", propName.c_str() );
	}
}


/*
 * Access a shared property segment
 */
void		*CPropertyAllocatorClient::accessPropertySegment( const std::string& propName, sint32 smid )
{
	// Find the property in the local map
	TPropertiesInMirror::iterator ipim = _PropertiesInMirror.find(propName);
	if ( ipim == _PropertiesInMirror.end() )
	{
		nlwarning( "MIRROR: Invalid property name %s", propName.c_str() );
		return NULL;
	}

	TPropertyInfo& propinfo = GET_PROPERTY_INFO(ipim);
	if ( propinfo.Segment != NULL )
	{
		nlwarning( "MIRROR: Cannot receive twice the same shared mem info" );
		return propinfo.Segment;
	}

	// Set the segment pointer
	propinfo.Segment = CSharedMemory::accessSharedMemory( toSharedMemId(smid) );
	if ( propinfo.Segment != NULL )
	{
		propinfo.Pending = false;
		propinfo.SMId = smid;
	}
	else
	{
		nlwarning( "MIRROR: Cannot access shared memory" );
	}
	return propinfo.Segment;
}


/*
 * Access a shared property segment if this service did not subscribed to it (return NULL if already accessed + warning)
 */
void		*CPropertyAllocatorClient::accessOtherPropertySegment( sint32 smid )
{
	for ( TPropertiesInMirror::iterator ipim=_PropertiesInMirror.begin(); ipim!=_PropertiesInMirror.end(); ++ipim )
	{
		TPropertyInfo& propinfo = GET_PROPERTY_INFO(ipim);
		if ( propinfo.SMId == smid )
		{
			// We have it already
			nlwarning( "MIRROR: Segment already accessed smid %d", smid );
			return NULL;
		}
	}
	void *segment = CSharedMemory::accessSharedMemory( toSharedMemId(smid) );
	if ( ! segment )
		nlwarning( "MIRROR: Cannot access shared memory" );
	return segment;
}


/*
 * Unallocate property
 */
void		CPropertyAllocatorClient::unallocProperty( const std::string& propName )
{
	TPropertiesInMirror::iterator ipim = _PropertiesInMirror.find( propName );
	if ( ipim != _PropertiesInMirror.end() )
	{
		if ( GET_PROPERTY_INFO(ipim).Segment != NULL )
		{
			CSharedMemory::closeSharedMemory( GET_PROPERTY_INFO(ipim).Segment );
			GET_PROPERTY_INFO(ipim).Segment = NULL;
		}
		else
			nlwarning( "MIRROR: Cannot unalloc property" );
	}

	// TODO: tell the MS?
}


/*
 *
 */
void		CPropertyAllocatorClient::serialOutMirrorInfo( NLNET::CMessage& msgout )
{
	// Count the number of property allocated
	uint16 nbPropsAlloc = 0;
	TPropertiesInMirror::iterator itr;
	for ( itr=_PropertiesInMirror.begin(); itr!=_PropertiesInMirror.end(); ++itr )
	{
		if ( GET_PROPERTY_INFO(itr).allocated() )
			++nbPropsAlloc;
	}
	msgout.serial( nbPropsAlloc );

	// Serialize the property info
	for ( itr=_PropertiesInMirror.begin(); itr!=_PropertiesInMirror.end(); ++itr )
	{
		if ( GET_PROPERTY_INFO(itr).allocated() )
		{
			msgout.serial( const_cast<string&>((*itr).first) );
			msgout.serial( const_cast<TPropertyInfo&>((*itr).second) );
		}
	}
	MIRROR_INFO( "MIRROR: Resyncing %u properties to MS", nbPropsAlloc );
}

