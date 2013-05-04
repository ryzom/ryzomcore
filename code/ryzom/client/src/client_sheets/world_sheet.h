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



#ifndef RY_WORLD_SHEET_H
#define RY_WORLD_SHEET_H

#include <vector>
#include <string>
#include "entity_sheet.h"

/// Continent location
struct SContLoc
{
	std::string SelectionName;			// name exported that we can use with select
	std::string ContinentName;			// .continent file
	float MinX, MinY, MaxX, MaxY;	    // Bounding box

	SContLoc ();
	void build (const NLGEORGES::UFormElm *pItem);
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

// Hierarchical map node
struct SMap
{
	std::string Name;				// Map Name
	std::string ContinentName;		// Name of the continent where the map is (empty if the map is world)
	std::string BitmapName;			// Name of the bitmap to load
	float MinX, MinY, MaxX, MaxY;	// Bounding Box
	struct SChild
	{
		std::string Name;		// Name to a child map
		std::string ZoneName;	// Click Zone to be found in the region_*.primitive
		// -----------------------
		void build (const NLGEORGES::UFormElm *pItem);
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	};
	std::vector<SChild>	Children;
	// ---------------------------------------------------------------------------------
	SMap();
	void build (const NLGEORGES::UFormElm *pItem);
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

/** Class that manage .world sheets
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  *
  */
class CWorldSheet : public CEntitySheet
{
public:
	std::string				Name;
	std::vector<SContLoc>	ContLocs;
	std::vector<SMap>		Maps;
public:
	CWorldSheet();
	// from CEntitySheet;
	virtual void build(const NLGEORGES::UFormElm &item);
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

#endif
