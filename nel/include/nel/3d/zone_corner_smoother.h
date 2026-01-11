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

#ifndef NL_ZONE_CORNER_SMOOTHER_H
#define NL_ZONE_CORNER_SMOOTHER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/zone.h"
#include <map>
#include <vector>


namespace NL3D
{


class	CZone;


// ***************************************************************************
/**
 * A class used to setup CornerSmoothFlag in CPatch. NB: it works with a compiled CZone (ie added in a landscape, and
 *	got with landscape->getZone()). NB: it changes values directly in CPatch (which it should not be permitted...),
 *	but don't except see the changes in realtime.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CZoneCornerSmoother
{
public:

	/// Constructor
	CZoneCornerSmoother();

	/** the doit method. this Zone and his 8 neighbors should be added into a landscape.
	 *	this method call setCornerSmoothFlag() on all CPatch of the zone.
	 *	All corner smooth are set to false but if:
	 *		- the corner is a vertex on a bind 1/X OR the vertex has exaclty 4 patch around him.
	 *		AND
	 *		- All edges sharing this corner must be smoothed getSmoothFlag()==true.
	 *		AND
	 *		- All edges sharing this corner must be "SameEdgeOrder", ie the patchs sharing this edge must have
	 *		same number of tiles along this edge.
	 *
	 *	This is important for noise computing, and lighting.
	 */
	void	computeAllCornerSmoothFlags(CZone *zone, std::vector<CZone*> neighborZones);


// ***************
private:
	struct	CVertexSmoothInfo
	{
		uint	NPatchShared;
		bool	VertexOnBind;
		bool	Smoothed;

		CVertexSmoothInfo()
		{
			NPatchShared= 0;
			VertexOnBind= false;
			// by default smoothed is true. any edge which don't agree will set it to false.
			Smoothed= true;
		}
	};


	typedef	std::map<sint, CVertexSmoothInfo>	TVertexMap;
	typedef	TVertexMap::iterator				ItVertexMap;

	typedef	std::map<const CTessVertex *, sint>	TIdVertexMap;
	typedef	TIdVertexMap::iterator				ItIdVertexMap;


private:
	TVertexMap		VertexMap;
	TIdVertexMap	IdVertexMap;

	void	buildPatchBindInfo(CPatch &pa, const CZone::CPatchConnect &pc, bool smoothEdge[4], bool cornerOnBind[4]);
	void	updateVertex(uint idVert, uint corner, bool smoothEdge[4], bool cornerOnBind[4]);


};


} // NL3D


#endif // NL_ZONE_CORNER_SMOOTHER_H

/* End of zone_corner_smoother.h */
