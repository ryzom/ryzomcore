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



#ifndef CL_FLORA_SHEET_H
#define CL_FLORA_SHEET_H

#include "plant_sheet.h"

/** Infos about a .plant contained in a .flora
  *
  */
class CPlantInfo
{
public:
	std::string SheetName;
	uint32		Weight;
	uint64      CumulatedWeight; // for sorting by weights
public:
	virtual void build(const NLGEORGES::UFormElm &item);
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};
inline bool operator < (const CPlantInfo &lhs, const CPlantInfo &rhs)
{
	return lhs.CumulatedWeight < rhs.CumulatedWeight;
}


/** Info about flora, read from a .flora sheet
  *
  */
class CFloraSheet : public CEntitySheet
{
public:
	float			MicroLifeThreshold; // 0 -> every tile has micro-life > 1 -> no tile has micro-life
public:
	/// ctor
	CFloraSheet();
	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);
	/// Serialize plant sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	// Get total weight of plant infos
	uint64		 getPlantInfoTotalWeight() const { return _TotalWeight; }
	/** Get plant info from weighted index
	  *
	  * e.g : we have 3 .plant in the .flora:
	  * .plant a.plant, weighted 4
	  * .plant b.plant, weighted 2
	  * .plant c.plant, weighted 1
	  *
	  * getPlantInfoWeight(0) -> a.plant
	  * getPlantInfoWeight(1) -> a.plant
	  * getPlantInfoWeight(2) -> a.plant
	  * getPlantInfoWeight(3) -> a.plant (4 occurences)
	  * getPlantInfoWeight(4) -> b.plant
	  * getPlantInfoWeight(5) -> b.plant (2 occurences)
	  * getPlantInfoWeight(6) -> c.plant (1 occurences)
	  *
	  */
	const CPlantInfo *getPlantInfoFromWeightedIndex(uint64 index) const;
	// Plant info access
	uint getNumPlantInfos() const { return (uint)_Plants.size(); }
	const CPlantInfo &getPlantInfo(uint index) const { return _Plants[index]; }
private:
	std::vector<CPlantInfo>  _Plants;
	uint64					 _TotalWeight;
};


#endif
