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

/*#ifndef _VILLAGE_H
#define _VILLAGE_H

#include "nel/misc/array_2d.h"
#include "3d/shape_info.h"
//
#include "3d/scene_group.h"
//
#include "nel/misc/smart_ptr.h"
//
#include <vector>


namespace NLGEORGES
{
	class UFormElm;
}

class CIGInfo
{
public:
	std::string								Path;
	NLMISC::CSmartPtr<NL3D::CInstanceGroup> IG; // NULL if not loaded yet	
	bool									Loaded;
public:
	CIGInfo() : Loaded(false) {}
	// load data & complete shape cache. no-op if datas where already loaded
	void load(NL3D::TShapeCache &shapeCache);
};


class CVillage
{
public:	
	std::vector<CIGInfo> IG;	
	uint32				 FileModificationDate;
public:		
	void swap(CVillage &other)
	{
		IG.swap(other.IG);
		std::swap(other.FileModificationDate, FileModificationDate);
	}
	// load data & complete shape cache. no-op if datas where already loaded
	void load(NL3D::TShapeCache &shapeCache);
};




class CVillageGrid
{
public:
	std::vector<CVillage>				Villages;
	NLMISC::CArray2D<std::list<uint> >	VillageGrid; // each grid cells gives the list of villages that overlap that cell (identified by their indices into 'Villages')
public:
	CVillageGrid();
	//
	void init(uint gridWidth, uint gridHright, sint zoneMinX, sint zoneMinY);
	void reset();
	// Load & add all villages of a continent to the map
	void addVillagesFromContinent(const std::string &continentSheetName);
private:	
	sint _ZoneMinX;
	sint _ZoneMinY;
private:
	bool loadVillageSheet(const NLGEORGES::UFormElm *villageItem, const std::string &continentSheetName, uint villageIndex, CVillage &dest);
};



#endif
  */