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



#ifndef CL_CONTINENT_MANAGER_BUILD_H
#define CL_CONTINENT_MANAGER_BUILD_H

#include "nel/misc/vector_2f.h"
#include "nel/ligo/primitive.h"

// Continent landmark
class CContLandMark
{

public:

	enum TContLMType
	{
		Capital,	// A city		(   icon and    text)
		Village,	// A city		(   icon and    text)
		Outpost,	// An outpost	(   icon and    text)
		Stable,		// A stable		(   icon and no text)
		Region,		// A region		(no icon and    text)
		Place,		// (lieu-dit)	(no icon and	text)
		Street,		// A Street		(no icon and	text)
		Unknown
	};

	TContLMType			Type;
	NLMISC::CVector2f	Pos;			// Center of the zone
	NLLIGO::CPrimZone	Zone;			// Region & Place
	std::string			TitleTextID;	// should be converted with CStringManagerClient::getPlaceLocalizedName() to get the actual title in ucstring

	CContLandMark()
	{
		Type = Unknown;
		Pos.x = 0.0f;
		Pos.y = 0.0f;
	}

	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1);
		f.serialEnum(Type);
		f.serial(Pos);
		f.serial(Zone);
		f.serial(TitleTextID);
	}

	static NLMISC::CVector2f getZoneCenter(const NLLIGO::CPrimZone &z)
	{
		NLMISC::CVector2f vMin, vMax;
		if (z.VPoints.size() == 0)
			return NLMISC::CVector2f(0,0);
		vMin = vMax = z.VPoints[0];
		for (uint32 i = 1; i < z.VPoints.size(); ++i)
		{
			if (z.VPoints[i].x < vMin.x) vMin.x = z.VPoints[i].x;
			if (z.VPoints[i].y < vMin.y) vMin.y = z.VPoints[i].y;
			if (z.VPoints[i].x > vMax.x) vMax.x = z.VPoints[i].x;
			if (z.VPoints[i].y > vMax.y) vMax.y = z.VPoints[i].y;
		}
		return (vMax+vMin)/2;
	}

	NLMISC::CVector2f getZoneCenter()
	{
		return getZoneCenter(Zone);
	}
};

extern bool buildLMConts(const std::string &worldSheet, const std::string &primitivesPath, const std::string &dataPath);

#define LM_PACKED_FILE "lmconts.packed"
#define WORLD_MAP_FILE "global_world_map.primitive"

#endif // CL_CONTINENT_MANAGER_BUILD_H

/* End of continent_manager_build.h */
