// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std3d.h"

#include "nel/3d/skeleton_weight.h"
#include "nel/misc/stream.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************

uint CSkeletonWeight::getNumNode () const
{
	return (uint)_Elements.size();
}

// ***************************************************************************

const std::string& CSkeletonWeight::getNodeName (uint node) const
{
	// Return the name of the n-th node
	return _Elements[node].Name;
}

// ***************************************************************************

float CSkeletonWeight::getNodeWeight (uint node) const
{
	// Return the name of the n-th node
	return _Elements[node].Weight;
}

// ***************************************************************************

void CSkeletonWeight::build (const TNodeArray& array)
{
	// Copy the array
	_Elements=array;
}

// ***************************************************************************

void CSkeletonWeight::serial (NLMISC::IStream& f)
{
	// Serial a header
	f.serialCheck (NELID("TWKS"));

	// Serial a version number
	(void)f.serialVersion (0);

	// Serial the array
	f.serialCont (_Elements);
}

// ***************************************************************************

void CSkeletonWeight::CNode::serial (NLMISC::IStream& f)
{
	// Serial a version number
	(void)f.serialVersion (0);

	// Serial the name
	f.serial (Name);

	// Serial the weight
	f.serial (Weight);
}

// ***************************************************************************

} // NL3D
