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



#ifndef RYAI_AI_INSTANCE_INLINE_H
#define RYAI_AI_INSTANCE_INLINE_H

inline
CContinent* CAIInstance::locateContinentForPos(CAIVector const& pos)
{
	FOREACH(it, CCont<CContinent>, _Continents)
	{
		if (it->_BoundingBox.isInside(pos))
			return	*it;
	}
	return NULL;
}

inline
CRegion* CAIInstance::locateRegionForPos(CAIVector const& pos)
{
	CContinent* cont = locateContinentForPos(pos);
	if	(!cont)
		return	NULL;
	FOREACH(it, CCont<CRegion>, cont->regions())
	{
		if (it->_BoundingBox.isInside(pos))
			return *it;
	}
	return NULL;
}

inline
CCellZone* CAIInstance::locateCellZoneForPos(CAIVector const& pos)
{
	CRegion* region = locateRegionForPos(pos);
	if (!region)
		return	NULL;
	FOREACH(it, CCont<CCellZone>, region->cellZones())
	{
		if (it->_BoundingBox.isInside(pos))
			return *it;
	}
	return NULL;
}

inline
CCell* CAIInstance::locateCellForPos(CAIVector const& pos)
{
	CCellZone* cz = locateCellZoneForPos(pos);
	if (!cz)
		return NULL;
	FOREACH(it, CCont<CCell>, cz->cells())
	{
		if	(it->_BoundingBox.isInside(pos))
			return *it;
	}
	return NULL;
}

#endif
