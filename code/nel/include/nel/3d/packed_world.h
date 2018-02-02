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

#ifndef NL_PACKED_WORLD
#define NL_PACKED_WORLD


#include "nel/3d/packed_zone.h"
//
#include "nel/misc/array_2d.h"
#include "nel/misc/stream.h"
#include "nel/misc/triangle.h"
//
#include <vector>



namespace NLMISC
{
	class CVector;
}

namespace NL3D
{

/** A set of packed zones
  * Allows for world scale collision test with a packed format
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2005
  */
class CPackedWorld
{
public:
	// list of zones that goes into this packed world (used by incremental build to know if some
	// zones need to be rebuilt)
	std::vector<std::string> ZoneNames;
public:
	// build world from a set of packed zones
	void build(std::vector<TPackedZoneBaseSPtr> &packesZones);
	bool raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<NLMISC::CTriangle> *testedTriangles = NULL, NLMISC::CVector *normal = NULL);
	void getZones(std::vector<TPackedZoneBaseSPtr> &zones);
	void serial(NLMISC::IStream &f);
	// just serialize the header, containing name of the zones this CPackedWorld was built from
	void serialZoneNames(NLMISC::IStream &f);
	/** Roughly select triangles that are within a convex 2D polygon (world coordinates)
	  * Selection is not exact, because limited to the resolution of the grid into which is packed each zone.
	  * Triangle that are within are guaranteed to be selected, however.
	  */
	void select(const NLMISC::CPolygon2D &poly, std::vector<NLMISC::CTriangle> &selectedTriangles) const;
private:
	class CZoneInfo
	{
	public:
		TPackedZoneBaseSPtr Zone;
		uint32				RaytraceCounter;
		void serial(NLMISC::IStream &f)
		{
			f.serialVersion(1);
			CPackedZoneBase *pz = Zone;
			f.serialPolyPtr(pz);
			Zone = pz;
		}
	};
	std::vector<CZoneInfo>				 _Zones;
	class CZoneIndexList
	{
	public:
		std::vector<uint32> IDs; // make a class from this vector just for serialization
		void serial(NLMISC::IStream &f)
		{
			f.serialCont(IDs);
		}
	};
	NLMISC::CArray2D<CZoneIndexList>	   _ZoneGrid;
	sint32								   _ZoneMinX;
	sint32								   _ZoneMinY;
	uint32								   _RaytraceCounter;
};

} // NL3D


#endif
