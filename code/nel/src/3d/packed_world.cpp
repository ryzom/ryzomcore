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
//
#include "nel/3d/packed_world.h"
//
#include "nel/misc/grid_traversal.h"


using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// *************************************************************************************************
void CPackedWorld::build(std::vector<TPackedZoneBaseSPtr> &packedZones)
{
	_ZoneGrid.clear();
	_Zones.clear();
	if (packedZones.empty()) return;
	CAABBox box;
	nlassert(packedZones[0]);
	box = packedZones[0]->Box;
	for(uint k = 1; k < packedZones.size(); ++k)
	{
		nlassert(packedZones[k]);
		box.extend(packedZones[k]->Box.getMin());
		box.extend(packedZones[k]->Box.getMax());
	}

	_ZoneMinX = (sint32) floorf(box.getMin().x / 160.f);
	_ZoneMinY = (sint32) floorf(box.getMin().y / 160.f);
	//
	sint32 zoneMaxX = (sint32) floorf(box.getMax().x / 160.f);
	sint32 zoneMaxY = (sint32) floorf(box.getMax().y / 160.f);
	//
	_ZoneGrid.init(zoneMaxX - _ZoneMinX + 1, zoneMaxY - _ZoneMinY + 1);
	_Zones.resize(packedZones.size());
	//
	for(uint k = 0; k < packedZones.size(); ++k)
	{
		CZoneInfo zi;
		zi.Zone = packedZones[k];
		zi.RaytraceCounter = 0;
		_Zones[k] = zi;
		sint zoneMinX = (sint) floorf(packedZones[k]->Box.getMin().x / 160.f) - (sint) _ZoneMinX;
		sint zoneMinY = (sint) floorf(packedZones[k]->Box.getMin().y / 160.f) - (sint) _ZoneMinY;
		sint zoneMaxX = (sint) floorf(packedZones[k]->Box.getMax().x / 160.f)  - (sint) _ZoneMinX;
		sint zoneMaxY = (sint) floorf(packedZones[k]->Box.getMax().y / 160.f)  - (sint) _ZoneMinY;
		for (sint y = zoneMinY; y <= zoneMaxY; ++y)
		{
			if (y < 0) continue;
			if (y >= (sint) _ZoneGrid.getHeight()) break;
			for (sint x = zoneMinX; x <= zoneMaxX; ++x)
			{
				if (x < 0) continue;
				if (x >= (sint) _ZoneGrid.getWidth()) break;
				_ZoneGrid(x, y).IDs.push_back((uint32) k);
			}
		}
	}
	_RaytraceCounter = std::numeric_limits<uint32>::max();
}

// *************************************************************************************************
bool CPackedWorld::raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, CVector &inter, std::vector<NLMISC::CTriangle> *testedTriangles /*= NULL*/, NLMISC::CVector *normal)
{
	if (_ZoneGrid.empty()) return false;
	++_RaytraceCounter;
	float bestDist = FLT_MAX;
	CVector bestNormal(CVector::Null);
	CVector currEnd = end;
	CVector currInter;
	if (_RaytraceCounter == std::numeric_limits<uint32>::max())
	{
		for(uint k = 0; k < _Zones.size(); ++k)
		{
			_Zones[k].RaytraceCounter = 0;
		}
	}
	sint currX, currY;
	CVector2f start2f(start.x / 160.f, start.y / 160.f);
	CVector2f dir2f((end.x - start.x) / 160.f, (end.y - start.y) / 160.f);
	CGridTraversal::startTraverse(start2f, currX, currY);
	do
	{
		sint x = currX - (sint) _ZoneMinX;
		sint y = currY - (sint) _ZoneMinY;
		if (x < 0) continue;
		if (y < 0) continue;
		if (x >= (sint) _ZoneGrid.getWidth()) continue;
		if (y >= (sint) _ZoneGrid.getHeight()) continue;
		std::vector<uint32> &currZoneList = _ZoneGrid(x, y).IDs;
		for(uint k = 0; k < currZoneList.size(); ++k)
		{
			if (_Zones[currZoneList[k]].RaytraceCounter != _RaytraceCounter) // already visited
			{
				NLMISC::CVector normalTmp;
				if (_Zones[currZoneList[k]].Zone->raytrace(start, currEnd, currInter, testedTriangles, &normalTmp))
				{
					float dist = (currInter - start).norm();
					if (dist < bestDist)
					{
						bestNormal = normalTmp;
						bestDist = dist;
						inter = currInter;
						currEnd = currInter; // during search, just seek hit that are nearest
					}
				}
				_Zones[currZoneList[k]].RaytraceCounter = _RaytraceCounter;
			}
		}
		if (bestDist != FLT_MAX)
		{
			if (normal)
			{
				*normal = bestNormal;
			}
			return true;
		}
	}
	while (CGridTraversal::traverse(start2f, dir2f, currX, currY));
	return false;
}

// *************************************************************************************************
void CPackedWorld::getZones(std::vector<TPackedZoneBaseSPtr> &zones)
{
	zones.clear();
	for(uint k = 0; k < _Zones.size(); ++k)
	{
		zones.push_back(_Zones[k].Zone);
	}
}

// *************************************************************************************************
void CPackedWorld::serialZoneNames(NLMISC::IStream &f)
{
	f.serialVersion(1);
	f.serialCheck(NELID("OWPA"));
	f.serialCont(ZoneNames);
}

// *************************************************************************************************
void CPackedWorld::serial(NLMISC::IStream &f)
{
	serialZoneNames(f);
	f.serialCont(_Zones);
	f.serial(_ZoneGrid);
	f.serial(_ZoneMinX);
	f.serial(_ZoneMinY);
}

// *************************************************************************************************
void CPackedWorld::select(const NLMISC::CPolygon2D &poly, std::vector<NLMISC::CTriangle> &selectedTriangles) const
{
	selectedTriangles.clear();
	// compute covered zones
	NLMISC::CPolygon2D zonePoly = poly;
	for (uint k = 0; k < zonePoly.Vertices.size(); ++k)
	{
		zonePoly.Vertices[k].x = zonePoly.Vertices[k].x / 160.f - (float) _ZoneMinX;
		zonePoly.Vertices[k].y = zonePoly.Vertices[k].y / 160.f - (float) _ZoneMinY;
	}
	NLMISC::CPolygon2D::TRasterVect borders;
	sint minY;
	zonePoly.computeOuterBorders(borders, minY);
	for (sint y = minY; y < (sint) (minY + borders.size()); ++y)
	{
		if (y < 0 || y >= (sint) _ZoneGrid.getHeight()) continue;
		for (sint x = borders[y - minY].first; x <= borders[y - minY].second; ++x)
		{
			if (x < 0 || x >= (sint) _ZoneGrid.getWidth()) continue;
			{
				const CZoneIndexList &zil = _ZoneGrid(x, y);
				for (uint k = 0; k < zil.IDs.size(); ++k)
				{
					_Zones[zil.IDs[k]].Zone->appendSelection(poly, selectedTriangles);
				}
			}
		}
	}
}


} // Nl3D
