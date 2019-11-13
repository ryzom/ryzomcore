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

#include "nel/3d/mrm_mesh.h"
#include <algorithm>

using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
uint CMRMMeshFinal::CWedge::NumAttributesToCompare= 0;
bool CMRMMeshFinal::CWedge::CompareSkinning= false;


// ***************************************************************************
CMRMMesh::CMRMMesh()
{
	NumAttributes= 0;
}



// ***************************************************************************
sint	CMRMMeshFinal::findInsertWedge(const CWedge &w)
{
	sint	ret;
	TWedgeMap::iterator		it;
	it= _WedgeMap.find(w);

	// if not found, must add it.
	if(it==_WedgeMap.end())
	{
		ret= (sint)Wedges.size();
		// insert into the map, with good id.
		_WedgeMap.insert(make_pair(w, ret));
		// add it to the array.
		Wedges.push_back(w);
	}
	else
	{
		ret= it->second;
	}

	return ret;
}

// ***************************************************************************
CMRMMeshFinal::CWedge::CWedge()
{
	Vertex = NLMISC::CVector::Null;
	NSkinMatUsed = 0;
	for (uint k = 0; k < NL3D_MRM_MAX_ATTRIB; ++k)
	{
		Attributes[k].set(0, 0, 0, 0);
	}
}

} // NL3D
