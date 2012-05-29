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



#include "stdpch.h"
#include "world_sheet.h"
//
#include "nel/georges/u_form_elm.h"

using namespace NLGEORGES;

//===============================================================================
CWorldSheet::CWorldSheet()
{
	Type = CEntitySheet::WORLD;
}

//===============================================================================
void CWorldSheet::build(const NLGEORGES::UFormElm &item)
{
	const UFormElm *pElt;
	uint size;
	nlverify (item.getNodeByName (&pElt, "continents list"));
	if(!pElt)
	{
		nlwarning("node 'continents list' not found in a .world");
	}
	else
	{
		nlverify (pElt->getArraySize (size));
		ContLocs.reserve(size);
		for (uint32 i = 0; i <size; ++i)
		{
			const UFormElm *pEltOfList;

			// Get the continent
			if (pElt->getArrayNode (&pEltOfList, i) && pEltOfList)
			{
				SContLoc clTmp;
				clTmp.build (pEltOfList);
				ContLocs.push_back (clTmp);
			}
		}
		item.getValueByName (Name, "name");
	}

	// Maps loading
	nlverify (item.getNodeByName (&pElt, "maps list"));
	if(!pElt)
	{
		nlwarning("node 'maps list' is not found in a .world");
	}
	else
	{
		nlverify (pElt->getArraySize (size));
		Maps.reserve(size);
		for (uint32 i = 0; i < size; ++i)
		{
			const UFormElm *pEltOfList;

			// Get the continent
			if (pElt->getArrayNode (&pEltOfList, i) && pEltOfList)
			{
				SMap mapTmp;
				mapTmp.build (pEltOfList);
				Maps.push_back (mapTmp);
			}
		}
	}
}

//===============================================================================
void CWorldSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Name);
	f.serialCont(ContLocs);
	f.serialCont(Maps);
}


//-----------------------------------------------
SContLoc::SContLoc()
{
	SelectionName = "unknown";
	ContinentName = "unknown";
	MinX = MinY = MaxX = MaxY = 0.0;
}

//-----------------------------------------------
void SContLoc::build (const UFormElm *pItem)
{
	pItem->getValueByName (SelectionName, "selection_name");
	pItem->getValueByName (ContinentName, "continent_name");
	pItem->getValueByName (MinX, "minx");
	pItem->getValueByName (MinY, "miny");
	pItem->getValueByName (MaxX, "maxx");
	pItem->getValueByName (MaxY, "maxy");
}

//-----------------------------------------------
void SContLoc::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(SelectionName);
	f.serial(ContinentName);
	f.serial(MinX, MinY, MaxX, MaxY);
}

//-----------------------------------------------
SMap::SMap()
{
	MinX = MinY = MaxX = MaxY = 0.0;
}

//-----------------------------------------------
void SMap::build(const NLGEORGES::UFormElm *pItem)
{
	pItem->getValueByName (Name, "name");
	pItem->getValueByName (ContinentName, "contname");
	pItem->getValueByName (BitmapName, "bitmap");
	pItem->getValueByName (MinX, "xmin");
	pItem->getValueByName (MinY, "ymin");
	pItem->getValueByName (MaxX, "xmax");
	pItem->getValueByName (MaxY, "ymax");
	// Read child
	const UFormElm *pElt;
	nlverify (pItem->getNodeByName (&pElt, "children"));
	uint size;
	if (pElt == NULL) return;
	nlverify (pElt->getArraySize (size));
	Children.reserve(size);
	for (uint32 i = 0; i < size; ++i)
	{
		const UFormElm *pEltOfList;
		// Get the continent
		if (pElt->getArrayNode (&pEltOfList, i) && pEltOfList)
		{
			SMap::SChild childTmp;
			childTmp.build (pEltOfList);
			Children.push_back (childTmp);
		}
	}
}

//-----------------------------------------------
void SMap::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Name);
	f.serial(ContinentName);
	f.serial(BitmapName);
	f.serial(MinX);
	f.serial(MinY);
	f.serial(MaxX);
	f.serial(MaxY);
	f.serialCont(Children);
}

//-----------------------------------------------
void SMap::SChild::build(const NLGEORGES::UFormElm *pItem)
{
	pItem->getValueByName (Name, "name");
	pItem->getValueByName (ZoneName, "click zone name");
}

//-----------------------------------------------
void SMap::SChild::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Name);
	f.serial(ZoneName);
}

