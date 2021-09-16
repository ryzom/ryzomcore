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
#include "property_allocator.h"


/*
 * Return a pointer to the property, or NULL if it is not ready yet (try again)
 */
void		*CPropertyAllocator::getPropertySegment( const std::string& propName ) const
{
	TPropertiesInMirror::const_iterator ipim = _PropertiesInMirror.find( propName );
	if ( ipim != _PropertiesInMirror.end() )
	{
		return GET_PROPERTY_INFO(ipim).Segment;
	}
	else
	{
		return NULL;
	}
}


/*
 *
 */
std::string TPropertyInfo::getFlagsString() const
{
	std::string s;
	if ( flagReadOnly() )
		s = "RO";
	else if ( flagWriteOnly() )
		s = "WO";
	else
		s = "RW";
	if ( flagNotifyChanges() )
		s += " Ntf";
	if ( flagGroupNotifiedByOtherProp() )
		s += "-byOtherOfGrp";
	if ( flagMonitorAssignment() )
		s += " Mon";
	return s;
}
