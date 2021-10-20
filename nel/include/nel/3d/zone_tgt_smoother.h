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

#ifndef NL_ZONE_TGT_SMOOTHER_H
#define NL_ZONE_TGT_SMOOTHER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/zone.h"
#include "nel/misc/common.h"
#include <map>
#include <list>
#include <vector>


namespace NL3D
{


// ***************************************************************************
/**
 * A class used to make Vertices coplanar IN or/and across zones.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CZoneTgtSmoother
{
public:

	/// Constructor
	CZoneTgtSmoother() {}

	/// Doit method. zones are modified. zones[0] is the center zones. Other are border zones.
	void		makeVerticesCoplanar(std::vector<CZoneInfo>  &zones);


// *************************
private:
	struct	CPatchId
	{
		// The unique Id of the patch.
		uint16			ZoneId;
		uint16			PatchId;
		// a ptr on the patch.
		CPatchInfo*		Patch;
		// [0,3]. which vertex of this patch points on the vertex.
		sint			IdVert;

		// local index on tangentes, around the vertex.
		sint			Tangents[2];
	};
	struct	CTangentId
	{
		uint16			ZoneId;
		uint16			PatchId;
		sint			EdgeId;
		// The two patchs which share the tangent.
		CPatchInfo		*Patchs[2];
		// The value of this tangent.
		CVector			Tangent;

		bool			isOppositeOf(const CTangentId &tgt)
		{
			// 4x4 configuartion only.
			// The opposite tangent do not have the same patchs which share this tangent.
			if(Patchs[0]==tgt.Patchs[0])	return false;
			if(Patchs[0]==tgt.Patchs[1])	return false;
			if(Patchs[1]==tgt.Patchs[0])	return false;
			if(Patchs[1]==tgt.Patchs[1])	return false;
			return true;
		}
	};

	struct	CVertexInfo
	{
		// neighbors patchs.
		std::list<CPatchId>	Patchs;
		bool				OnBorder;

		CVertexInfo()
		{
			OnBorder= false;
		}
	};

	typedef	std::map<sint, CVertexInfo>		TVertexMap;
	typedef	TVertexMap::iterator			ItVertexMap;

private:
	TVertexMap		VertexMap;

};


} // NL3D


#endif // NL_ZONE_TGT_SMOOTHER_H

/* End of zone_tgt_smoother.h */
